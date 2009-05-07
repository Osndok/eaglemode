//------------------------------------------------------------------------------
// emProcess.cpp
//
// Copyright (C) 2006-2009 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emCore/emProcess.h>


#if defined(_WIN32)
//==============================================================================
//==================== Windows implementation of emProcess =====================
//==============================================================================

#include <windows.h>
#ifndef FILE_FLAG_FIRST_PIPE_INSTANCE
#	define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000
#endif


struct emProcessPrivate {

	emString Arg0;
	HANDLE HProcess;
	HANDLE HThread;
	DWORD ProcessId;
	DWORD ThreadId;
	DWORD ExitStatus;

	struct PipeStruct {
		HANDLE Handle;
		HANDLE EventHandle;
		OVERLAPPED Overlapped;
		DWORD Status;
		char Buf[16384];
		DWORD Len;
		DWORD Pos;
	};
	PipeStruct Pipe[3]; // stdin, stdout and stderr in that order.

	HANDLE PreparePipe(int index);
	void DoPipeIO(int index);
	void ClosePipe(int index);
};


emProcess::emProcess()
{
	int i;

	P=new emProcessPrivate;
	P->HProcess=INVALID_HANDLE_VALUE;
	P->HThread=INVALID_HANDLE_VALUE;
	P->ProcessId=(DWORD)(-1);
	P->ThreadId=(DWORD)(-1);
	P->ExitStatus=0;
	for (i=0; i<3; i++) {
		P->Pipe[i].Handle=INVALID_HANDLE_VALUE;
		P->Pipe[i].EventHandle=INVALID_HANDLE_VALUE;
	}
}


emProcess::~emProcess()
{
	Terminate();
	delete P;
}


void emProcess::TryStart(
	const emArray<emString> & args, const emArray<emString> & extraEnv,
	const char * dirPath, int flags
) throw(emString)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	emString str,msg,useDir;
	emArray<emString> env;
	emArray<char> envBlock;
	emArray<char> cmd;
	const char * p;
	char * * pp;
	HANDLE tmpHandle[10];
	HANDLE h;
	DWORD d;
	int i,j,k,tmpHandleCount;
	char c;

	if (args.IsEmpty()) {
		emFatalError("emProcess: No arguments.");
	}

	if (P->HProcess!=INVALID_HANDLE_VALUE) {
		emFatalError(
			"emProcess: TryStart called while still managing another process."
		);
	}

	if ((flags&SF_PIPE_STDIN )!=0) flags&=~SF_SHARE_STDIN ;
	if ((flags&SF_PIPE_STDOUT)!=0) flags&=~SF_SHARE_STDOUT;
	if ((flags&SF_PIPE_STDERR)!=0) flags&=~SF_SHARE_STDERR;

	for (i=0; i<args.GetCount(); i++) {
		if (i) {
			cmd+=' ';
			for (j=args[i].GetCount()-1; j>=0; j--) {
				c=args[i][j];
				if ((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') &&
				    c!='\\' && c!='/' && c!='.' && c!=':' && c!='-') break;
			}
			if (j<0) {
				cmd.Add(args[i].Get(),args[i].GetLen());
				continue;
			}
		}
		cmd+='"';
		for (j=0, k=0; j<args[i].GetCount(); j++) {
			c=args[i][j];
			if (c=='"') {
				while (k>0) { cmd+='\\'; k--; }
				cmd+='\\';
			}
			else if (c=='\\') k++;
			else k=0;
			cmd+=c;
		}
		while (k>0) { cmd+='\\'; k--; }
		cmd+='"';
	}
	cmd+='\0';
	cmd+='\0';

	pp=environ; if (!pp) { getenv("X"); pp=environ; }
	while (*pp) { env.Add(*pp); pp++; }
	for (i=0; i<extraEnv.GetCount(); i++) {
		str=extraEnv[i];
		p=strchr(str.Get(),'=');
		if (p) k=p-str.Get();
		else k=strlen(p);
		for (j=0; j<env.GetCount(); j++) {
			if (
				env[j].GetLen()>=k &&
				memcmp(env[j].Get(),str.Get(),k)==0 &&
				(env[j][k]==0 || env[j][k]=='=')
			) {
				env.Remove(j);
				break;
			}
		}
		if (str[k]=='=') env.Add(str);
	}
	env.Sort(emStdComparer<emString>::Compare);
	for (i=0; i<env.GetCount(); i++) envBlock.Add(env[i].Get(),env[i].GetLen()+1);
	envBlock.Add('\0');

	if (dirPath) useDir=emGetAbsolutePath(dirPath);
	else useDir=emGetCurrentDirectory();

	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESTDHANDLES;
	si.hStdInput=INVALID_HANDLE_VALUE;
	si.hStdOutput=INVALID_HANDLE_VALUE;
	si.hStdError=INVALID_HANDLE_VALUE;

	tmpHandleCount=0;

	if ((flags&SF_SHARE_STDIN)!=0) {
		si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
	}
	if ((flags&SF_PIPE_STDIN)!=0) {
		h=P->PreparePipe(0);
		si.hStdInput=h;
		tmpHandle[tmpHandleCount++]=h;
	}

	if ((flags&SF_SHARE_STDOUT)!=0) {
		si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
	}
	if ((flags&SF_PIPE_STDOUT)!=0) {
		h=P->PreparePipe(1);
		si.hStdOutput=h;
		tmpHandle[tmpHandleCount++]=h;
	}

	if ((flags&SF_SHARE_STDERR)!=0) {
		si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
	}
	if ((flags&SF_PIPE_STDERR)!=0) {
		h=P->PreparePipe(2);
		si.hStdError=h;
		tmpHandle[tmpHandleCount++]=h;
	}

	if (
		!CreateProcess(
			NULL,
			cmd.GetWritable(),
			NULL,
			NULL,
			TRUE,
			0,
			(LPVOID)envBlock.Get(),
			useDir.Get(),
			&si,
			&pi
		)
	) {
		d=GetLastError();
		while (tmpHandleCount>0) CloseHandle(tmpHandle[--tmpHandleCount]);
		CloseWriting();
		CloseReading();
		CloseReadingErr();
		throw emString::Format(
			"Failed to start process \"%s\": %s",
			args[0].Get(),
			emGetErrorText(d).Get()
		);
	}

	while (tmpHandleCount>0) CloseHandle(tmpHandle[--tmpHandleCount]);
	P->Arg0=args[0];
	P->HProcess=pi.hProcess;
	P->HThread=pi.hThread;
	P->ProcessId=pi.dwProcessId;
	P->ThreadId=pi.dwThreadId;
	P->ExitStatus=0;
}


void emProcess::TryStartUnmanaged(
	const emArray<emString> & args, const emArray<emString> & extraEnv,
	const char * dirPath, int flags
) throw(emString)
{
	emProcess p;

	flags&=~(SF_PIPE_STDIN|SF_PIPE_STDOUT|SF_PIPE_STDERR);
	p.TryStart(args,extraEnv,dirPath,flags);
	CloseHandle(p.P->HThread);
	CloseHandle(p.P->HProcess);
	p.P->HProcess=INVALID_HANDLE_VALUE;
	p.P->HThread=INVALID_HANDLE_VALUE;
	p.P->ProcessId=(DWORD)(-1);
	p.P->ThreadId=(DWORD)(-1);
	p.P->ClosePipe(0);
	p.P->ClosePipe(1);
	p.P->ClosePipe(2);
}


int emProcess::TryWrite(const char * buf, int len) throw(emString)
{
	emProcessPrivate::PipeStruct * pipe;
	int done,l;
	DWORD s;

	pipe=&P->Pipe[0];
	if (pipe->Handle==INVALID_HANDLE_VALUE) return -1;
	done=0;
	for (;;) {
		P->DoPipeIO(0);
		if (pipe->Status!=0) break;
		l=sizeof(pipe->Buf)-pipe->Len;
		if (l>len-done) l=len-done;
		if (l<=0) break;
		memcpy(pipe->Buf+pipe->Len,buf+done,l);
		pipe->Len+=l;
		done+=l;
	}
	if (done>0) return done;
	s=pipe->Status;
	if (s==0 || s==ERROR_IO_PENDING || s==ERROR_IO_INCOMPLETE) return 0;
	CloseWriting();
	if (s==ERROR_HANDLE_EOF || s==ERROR_BROKEN_PIPE) return -1;
	throw emString::Format(
		"Failed to write to stdin pipe of child process \"%s\": %s",
		P->Arg0.Get(),
		emGetErrorText(s).Get()
	);
}


int emProcess::TryRead(char * buf, int maxLen) throw(emString)
{
	emProcessPrivate::PipeStruct * pipe;
	int done,l;
	DWORD s;

	pipe=&P->Pipe[1];
	if (pipe->Handle==INVALID_HANDLE_VALUE) return -1;
	done=0;
	for (;;) {
		P->DoPipeIO(1);
		if (pipe->Status!=0) break;
		l=pipe->Len-pipe->Pos;
		if (l>maxLen-done) l=maxLen-done;
		if (l<=0) break;
		memcpy(buf+done,pipe->Buf+pipe->Pos,l);
		pipe->Pos+=l;
		done+=l;
	}
	if (done>0) return done;
	s=pipe->Status;
	if (s==0 || s==ERROR_IO_PENDING || s==ERROR_IO_INCOMPLETE) return 0;
	CloseReading();
	if (s==ERROR_HANDLE_EOF || s==ERROR_BROKEN_PIPE) return -1;
	throw emString::Format(
		"Failed to read stdout pipe of child process \"%s\": %s",
		P->Arg0.Get(),
		emGetErrorText(s).Get()
	);
}


int emProcess::TryReadErr(char * buf, int maxLen) throw(emString)
{
	emProcessPrivate::PipeStruct * pipe;
	int done,l;
	DWORD s;

	pipe=&P->Pipe[2];
	if (pipe->Handle==INVALID_HANDLE_VALUE) return -1;
	done=0;
	for (;;) {
		P->DoPipeIO(2);
		if (pipe->Status!=0) break;
		l=pipe->Len-pipe->Pos;
		if (l>maxLen-done) l=maxLen-done;
		if (l<=0) break;
		memcpy(buf+done,pipe->Buf+pipe->Pos,l);
		pipe->Pos+=l;
		done+=l;
	}
	if (done>0) return done;
	s=pipe->Status;
	if (s==0 || s==ERROR_IO_PENDING || s==ERROR_IO_INCOMPLETE) return 0;
	CloseReadingErr();
	if (s==ERROR_HANDLE_EOF || s==ERROR_BROKEN_PIPE) return -1;
	throw emString::Format(
		"Failed to read stderr pipe of child process \"%s\": %s",
		P->Arg0.Get(),
		emGetErrorText(s).Get()
	);
}


void emProcess::WaitPipes(int waitFlags, unsigned timeoutMS)
{
	static const int flag[3] = { WF_WAIT_STDIN, WF_WAIT_STDOUT, WF_WAIT_STDERR };
	HANDLE handles[3];
	DWORD i,n;

	if (timeoutMS==0) return;
	n=0;
	for (i=0; i<3; i++) {
		if ((waitFlags&flag[i])==0) continue;
		if (P->Pipe[i].Handle==INVALID_HANDLE_VALUE) continue;
		P->DoPipeIO(i);
		if (
			P->Pipe[i].Status!=ERROR_IO_PENDING &&
			P->Pipe[i].Status!=ERROR_IO_INCOMPLETE
		) return;
		handles[n++]=P->Pipe[i].EventHandle;
	}
	if (n<=0) return;
	if (WaitForMultipleObjects(
		n,
		handles,
		FALSE,
		timeoutMS==UINT_MAX ? INFINITE : (DWORD)timeoutMS
	)==WAIT_FAILED) {
		emFatalError(
			"emProcess: WaitForMultipleObjects failed: %s\n",
			emGetErrorText(GetLastError()).Get()
		);
	}
}


void emProcess::CloseWriting()
{
	P->ClosePipe(0);
}


void emProcess::CloseReading()
{
	P->ClosePipe(1);
}


void emProcess::CloseReadingErr()
{
	P->ClosePipe(2);
}


void emProcess::SendTerminationSignal()
{
	if (IsRunning()) PostThreadMessage(P->ThreadId,WM_QUIT,0,0);
}


void emProcess::SendKillSignal()
{
	if (IsRunning()) TerminateProcess(P->HProcess,128+9);
}


bool emProcess::WaitForTermination(unsigned timeoutMS)
{
	DWORD res;

	if (P->HProcess!=INVALID_HANDLE_VALUE) {
		res=WaitForSingleObject(
			P->HProcess,
			timeoutMS==UINT_MAX ? INFINITE : (DWORD)timeoutMS
		);
		if (res!=WAIT_OBJECT_0) {
			if (res!=WAIT_TIMEOUT) {
				emFatalError(
					"emProcess: WaitForSingleObject failed: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
			return false;
		}
		GetExitCodeProcess(P->HProcess,&P->ExitStatus);
		CloseHandle(P->HThread);
		CloseHandle(P->HProcess);
		P->HProcess=INVALID_HANDLE_VALUE;
		P->HThread=INVALID_HANDLE_VALUE;
		P->ProcessId=(DWORD)(-1);
		P->ThreadId=(DWORD)(-1);
		P->ClosePipe(0);
		P->ClosePipe(1);
		P->ClosePipe(2);
	}
	return true;
}


bool emProcess::IsRunning()
{
	return !WaitForTermination(0);
}


void emProcess::Terminate(unsigned fatalTimeoutMS)
{
	if (IsRunning()) {
		SendTerminationSignal();
		if (!WaitForTermination(fatalTimeoutMS)) {
			emFatalError(
				"Child process \"%s\" not willing to terminate.",
				P->Arg0.Get()
			);
		}
	}
}


int emProcess::GetExitStatus() const
{
	return P->ExitStatus;
}


HANDLE emProcessPrivate::PreparePipe(int index)
{
	static int counter=0;
	SECURITY_ATTRIBUTES sec;
	HANDLE h1,h2,he;
	emString name;
	DWORD i,openMode;

	for (i=0; ; i++) {
		name=emString::Format(
			"%d:%p:%d:%d:%d",
			(int)GetCurrentProcessId(),
			(char*)this,
			index,
			(int)emGetClockMS(),
			counter++
		);
		name=
			emString("\\\\.\\pipe\\emProcess-") +
			emCalcHashName(name.Get(),name.GetCount(),40)
		;
		openMode=FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE;
		if (index==0) openMode|=PIPE_ACCESS_OUTBOUND;
		else          openMode|=PIPE_ACCESS_INBOUND;
		h1=CreateNamedPipe(
			name.Get(),
			openMode,
			0,
			1,
			16384,
			16384,
			1000,
			NULL
		);
		if (h1!=INVALID_HANDLE_VALUE) break;
		if (i>1000) {
			emFatalError(
				"emProcess: CreateNamedPipe failed: %s\n",
				emGetErrorText(GetLastError()).Get()
			);
		}
	}

	if (index==0) openMode=GENERIC_READ;
	else          openMode=GENERIC_WRITE;
	memset(&sec,0,sizeof(sec));
	sec.nLength=sizeof(sec);
	sec.bInheritHandle=TRUE;
	h2=CreateFile(
		name.Get(),
		openMode,
		0,
		&sec,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (h2==INVALID_HANDLE_VALUE) {
		emFatalError(
			"emProcess: CreateFile on named pipe failed: %s\n",
			emGetErrorText(GetLastError()).Get()
		);
	}

	he=CreateEvent(NULL,TRUE,FALSE,NULL);
	if (he==NULL) {
		emFatalError(
			"emProcess: CreateEvent failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

	Pipe[index].Handle=h1;
	Pipe[index].EventHandle=he;
	Pipe[index].Status=0;
	Pipe[index].Len=0;
	Pipe[index].Pos=0;
	return h2;
}


void emProcessPrivate::DoPipeIO(int index)
{
	PipeStruct * pipe;
	BOOL res;

	pipe=&Pipe[index];
	if (pipe->Handle==INVALID_HANDLE_VALUE) return;
	for (;;) {
		if (pipe->Status==0) {
			if (index==0) {
				if (pipe->Len==0) break;
			}
			else {
				if (pipe->Pos<pipe->Len) break;
			}
			ResetEvent(pipe->EventHandle);
			memset(&pipe->Overlapped,0,sizeof(pipe->Overlapped));
			pipe->Overlapped.hEvent=pipe->EventHandle;
			pipe->Pos=0;
			if (index==0) {
				res=WriteFile(
					pipe->Handle,
					pipe->Buf,
					pipe->Len,
					&pipe->Pos,
					&pipe->Overlapped
				);
			}
			else {
				res=ReadFile(
					pipe->Handle,
					pipe->Buf,
					sizeof(pipe->Buf),
					&pipe->Pos,
					&pipe->Overlapped
				);
			}
		}
		else if (
			pipe->Status==ERROR_IO_PENDING ||
			pipe->Status==ERROR_IO_INCOMPLETE
		) {
			res=GetOverlappedResult(
				pipe->Handle,
				&pipe->Overlapped,
				&pipe->Pos,
				FALSE
			);
		}
		else {
			break;
		}
		if (!res) {
			pipe->Status=GetLastError();
			break;
		}
		if (index==0 && pipe->Pos==0) {
			pipe->Status=ERROR_BROKEN_PIPE;
			break;
		}
		pipe->Status=0;
		if (index==0) {
			if (pipe->Pos<pipe->Len) {
				memmove(pipe->Buf,pipe->Buf+pipe->Pos,pipe->Len-pipe->Pos);
				pipe->Len-=pipe->Pos;
			}
			else {
				pipe->Len=0;
			}
		}
		else {
			pipe->Len=pipe->Pos;
		}
		pipe->Pos=0;
	}
}


void emProcessPrivate::ClosePipe(int index)
{
	PipeStruct * pipe;

	pipe=&Pipe[index];
	if (pipe->Handle!=INVALID_HANDLE_VALUE) {
		CancelIo(pipe->Handle);
		CloseHandle(pipe->Handle);
		pipe->Handle=INVALID_HANDLE_VALUE;
		if (pipe->EventHandle!=INVALID_HANDLE_VALUE) {
			CloseHandle(pipe->EventHandle);
			pipe->EventHandle=INVALID_HANDLE_VALUE;
		}
	}
}


#else
//==============================================================================
//====================== UNIX implementation of emProcess ======================
//==============================================================================

#include <emCore/emThread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>


struct emProcessPrivate {

	static void EmptySigHandler(int signum);

	static void TryStart(
		const emArray<emString> & args,
		const emArray<emString> & extraEnv,
		const char * dirPath,
		int flags,
		emProcessPrivate * managed
	) throw(emString);

	emString Arg0;
	pid_t Pid;
	int FdIn;
	int FdOut;
	int FdErr;
	int ExitStatus;
};


emProcess::emProcess()
{
	P=new emProcessPrivate;
	P->Pid=-1;
	P->FdIn=-1;
	P->FdOut=-1;
	P->FdErr=-1;
	P->ExitStatus=0;
}


emProcess::~emProcess()
{
	Terminate();
	delete P;
}


void emProcess::TryStart(
	const emArray<emString> & args, const emArray<emString> & extraEnv,
	const char * dirPath, int flags
) throw(emString)
{
	emProcessPrivate::TryStart(args,extraEnv,dirPath,flags,P);
}


void emProcess::TryStartUnmanaged(
	const emArray<emString> & args, const emArray<emString> & extraEnv,
	const char * dirPath, int flags
) throw(emString)
{
	flags&=~(SF_PIPE_STDIN|SF_PIPE_STDOUT|SF_PIPE_STDERR);
	emProcessPrivate::TryStart(args,extraEnv,dirPath,flags,NULL);
}


int emProcess::TryWrite(const char * buf, int len) throw(emString)
{
	ssize_t r;
	int e;

	if (P->FdIn==-1) return -1;
	if (len<=0) return 0;
	r=write(P->FdIn,buf,len);
	if (r>=0) return (int)r;
	if (errno==EAGAIN) return 0;
	if (errno==EPIPE) {
		CloseWriting();
		return -1;
	}
	e=errno;
	CloseWriting();
	throw emString::Format(
		"Failed to write to stdin pipe of child process \"%s\" (pid %d): %s",
		P->Arg0.Get(),
		(int)P->Pid,
		emGetErrorText(e).Get()
	);
}


int emProcess::TryRead(char * buf, int maxLen) throw(emString)
{
	ssize_t r;
	int e;

	if (P->FdOut==-1) return -1;
	if (maxLen<=0) return 0;
	r=read(P->FdOut,buf,maxLen);
	if (r>0) return (int)r;
	if (r==0) {
		CloseReading();
		return -1;
	}
	if (errno==EAGAIN) return 0;
	e=errno;
	CloseReading();
	throw emString::Format(
		"Failed to read stdout pipe of child process \"%s\" (pid %d): %s",
		P->Arg0.Get(),
		(int)P->Pid,
		emGetErrorText(e).Get()
	);
}


int emProcess::TryReadErr(char * buf, int maxLen) throw(emString)
{
	ssize_t r;
	int e;

	if (P->FdErr==-1) return -1;
	if (maxLen<=0) return 0;
	r=read(P->FdErr,buf,maxLen);
	if (r>0) return (int)r;
	if (r==0) {
		CloseReadingErr();
		return -1;
	}
	if (errno==EAGAIN) return 0;
	e=errno;
	CloseReadingErr();
	throw emString::Format(
		"Failed to read stderr pipe of child process \"%s\" (pid %d): %s",
		P->Arg0.Get(),
		(int)P->Pid,
		emGetErrorText(e).Get()
	);
}


void emProcess::WaitPipes(int waitFlags, unsigned timeoutMS)
{
	timeval tv;
	timeval * ptv;
	fd_set rset;
	fd_set wset;
	int fdMax;

	if (timeoutMS==0) return;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	fdMax=-1;
	if ((waitFlags&WF_WAIT_STDIN)!=0 && P->FdIn!=-1) {
		FD_SET(P->FdIn,&wset);
		if (fdMax<P->FdIn) fdMax=P->FdIn;
	}
	if ((waitFlags&WF_WAIT_STDOUT)!=0 && P->FdOut!=-1) {
		FD_SET(P->FdOut,&rset);
		if (fdMax<P->FdOut) fdMax=P->FdOut;
	}
	if ((waitFlags&WF_WAIT_STDERR)!=0 && P->FdErr!=-1) {
		FD_SET(P->FdErr,&rset);
		if (fdMax<P->FdErr) fdMax=P->FdErr;
	}
	if (fdMax==-1) return;
	if (timeoutMS==UINT_MAX) {
		ptv=NULL;
	}
	else {
		tv.tv_sec=(time_t)(timeoutMS/1000);
		tv.tv_usec=(time_t)((timeoutMS%1000)*1000);
		ptv=&tv;
	}
	if (select(fdMax+1,&rset,&wset,NULL,ptv)<0) {
		if (errno!=EINTR) {
			emFatalError(
				"emProcess: select failed: %s\n",
				emGetErrorText(errno).Get()
			);
		}
	}
}


void emProcess::CloseWriting()
{
	if (P->FdIn!=-1) {
		close(P->FdIn);
		P->FdIn=-1;
	}
}


void emProcess::CloseReading()
{
	if (P->FdOut!=-1) {
		close(P->FdOut);
		P->FdOut=-1;
	}
}


void emProcess::CloseReadingErr()
{
	if (P->FdErr!=-1) {
		close(P->FdErr);
		P->FdErr=-1;
	}
}


void emProcess::SendTerminationSignal()
{
	if (IsRunning()) kill(P->Pid,SIGTERM);
	// If the process would be a group leader, kill(-P->Pid,SIGTERM) could
	// be used to send the signal to all processes in the group.
}


void emProcess::SendKillSignal()
{
	if (IsRunning()) kill(P->Pid,SIGKILL);
}


bool emProcess::WaitForTermination(unsigned timeoutMS)
{
	pid_t pr;
	unsigned t, u;

	if (P->Pid==-1) return true;
	for (t=0;;) {
		pr=waitpid(P->Pid,&P->ExitStatus,WNOHANG);
		if (pr) break;
		if (timeoutMS==0) return false;
		u=t;
		if (u>timeoutMS) u=timeoutMS;
		emSleepMS(u);
		if (timeoutMS!=UINT_MAX) timeoutMS-=u;
		if (t<10) t++;
	}
	if (pr!=P->Pid) {
		if (pr<0) {
			emFatalError("emProcess: waitpid failed: %s",emGetErrorText(errno).Get());
		}
		else {
			emFatalError("emProcess: unexpected return value from waitpid.");
		}
	}
	P->Pid=-1;
	if (WIFEXITED(P->ExitStatus)) P->ExitStatus=WEXITSTATUS(P->ExitStatus);
	else P->ExitStatus=128+WTERMSIG(P->ExitStatus);
	CloseWriting();
	CloseReading();
	CloseReadingErr();
	return true;
}


bool emProcess::IsRunning()
{
	return !WaitForTermination(0);
}


void emProcess::Terminate(unsigned fatalTimeoutMS)
{
	if (IsRunning()) {
		SendTerminationSignal();
		if (!WaitForTermination(fatalTimeoutMS)) {
			emFatalError(
				"Child process \"%s\" (pid %d) not willing to terminate.",
				P->Arg0.Get(),
				(int)P->Pid
			);
		}
	}
}


int emProcess::GetExitStatus() const
{
	return P->ExitStatus;
}


void emProcessPrivate::EmptySigHandler(int signum)
{
}


void emProcessPrivate::TryStart(
	const emArray<emString> & args, const emArray<emString> & extraEnv,
	const char * dirPath, int flags, emProcessPrivate * managed
) throw(emString)
{
	static emThreadInitMutex sigHandlersInitMutex;
	struct sigaction sa;
	char buf[1024];
	emString msg;
	int pipeIn[2],pipeOut[2],pipeErr[2],pipeTmp[2];
	char * * cargs;
	int i,n,f,len,r;
	pid_t pr,pid;

	if (args.IsEmpty()) {
		emFatalError("emProcess: No arguments.");
	}

	if (managed && managed->Pid!=-1) {
		emFatalError(
			"emProcess: TryStart called while still managing another process."
		);
	}

	if (!managed) {
		flags&=~emProcess::SF_PIPE_STDIN;
		flags&=~emProcess::SF_PIPE_STDOUT;
		flags&=~emProcess::SF_PIPE_STDERR;
	}
	if ((flags&emProcess::SF_PIPE_STDIN)!=0) {
		flags&=~emProcess::SF_SHARE_STDIN;
	}
	if ((flags&emProcess::SF_PIPE_STDOUT)!=0) {
		flags&=~emProcess::SF_SHARE_STDOUT;
	}
	if ((flags&emProcess::SF_PIPE_STDERR)!=0) {
		flags&=~emProcess::SF_SHARE_STDERR;
	}

	if (sigHandlersInitMutex.Begin()) {
		memset(&sa,0,sizeof(sa));
		sa.sa_handler=EmptySigHandler;
		sa.sa_flags=SA_RESTART;
		if (sigaction(SIGCHLD,&sa,NULL)) {
			emFatalError(
				"emProcess: Failed to install handler for SIGCHLD: %s",
				emGetErrorText(errno).Get()
			);
		}
		memset(&sa,0,sizeof(sa));
		sa.sa_handler=EmptySigHandler;
		sa.sa_flags=SA_RESTART;
		if (sigaction(SIGPIPE,&sa,NULL)) {
			emFatalError(
				"emProcess: Failed to install handler for SIGPIPE: %s",
				emGetErrorText(errno).Get()
			);
		}
		sigHandlersInitMutex.End();
	}

	pid=-1;
	pipeIn[0]=-1;
	pipeIn[1]=-1;
	pipeOut[0]=-1;
	pipeOut[1]=-1;
	pipeErr[0]=-1;
	pipeErr[1]=-1;
	pipeTmp[0]=-1;
	pipeTmp[1]=-1;

	if ((flags&emProcess::SF_PIPE_STDIN)!=0) {
		if (pipe(pipeIn)) goto L_PipeErr;
		if ((f=fcntl(pipeIn[1],F_GETFL))<0) goto L_GetFlErr;
		if (fcntl(pipeIn[1],F_SETFL,f|O_NONBLOCK)<0) goto L_SetFlErr;
	}
	if ((flags&emProcess::SF_PIPE_STDOUT)!=0) {
		if (pipe(pipeOut)) goto L_PipeErr;
		if ((f=fcntl(pipeOut[0],F_GETFL))<0) goto L_GetFlErr;
		if (fcntl(pipeOut[0],F_SETFL,f|O_NONBLOCK)<0) goto L_SetFlErr;
	}
	if ((flags&emProcess::SF_PIPE_STDERR)!=0) {
		if (pipe(pipeErr)) goto L_PipeErr;
		if ((f=fcntl(pipeErr[0],F_GETFL))<0) goto L_GetFlErr;
		if (fcntl(pipeErr[0],F_SETFL,f|O_NONBLOCK)<0) goto L_SetFlErr;
	}
	if (pipe(pipeTmp)) goto L_PipeErr;

	pid=fork();
	if (pid<0) goto L_ForkErr;
	// If we ever want to support an option for making the managed child
	// process a group leader, insert code like this here:
	//   if (managed && wantToMakeItAGroupLeader) setpgid(pid,pid);
	// Yes, insert it exactly at this point for both, child and parent!
	// For the unmanaged case, look around next fork more below.
	if (pid==0) {

		//--------- Child process ---------

		for (i=0; i<extraEnv.GetCount(); i++) {
			if (putenv((char*)extraEnv[i].Get())<0) {
				msg=emString::Format(
					"Failed to set environment variable: %s",
					emGetErrorText(errno).Get()
				);
				goto L_ChildErr;
			}
		}

		if (dirPath) {
			if (chdir(dirPath)<0) {
				msg=emString::Format(
					"Failed to set working directory to \"%s\": %s",
					dirPath,
					emGetErrorText(errno).Get()
				);
				goto L_ChildErr;
			}
		}

		if (pipeIn[0]!=-1) {
			if (dup2(pipeIn[0],STDIN_FILENO)!=STDIN_FILENO) {
				msg=emString::Format("dup2 failed: %s",emGetErrorText(errno).Get());
				goto L_ChildErr;
			}
		}
		if (pipeOut[1]!=-1) {
			if (dup2(pipeOut[1],STDOUT_FILENO)!=STDOUT_FILENO) {
				msg=emString::Format("dup2 failed: %s",emGetErrorText(errno).Get());
				goto L_ChildErr;
			}
		}
		if (pipeErr[1]!=-1) {
			if (dup2(pipeErr[1],STDERR_FILENO)!=STDERR_FILENO) {
				msg=emString::Format("dup2 failed: %s",emGetErrorText(errno).Get());
				goto L_ChildErr;
			}
		}

		n=sysconf(_SC_OPEN_MAX);
		if (n<=0) {
			msg=emString::Format(
				"sysconf(_SC_OPEN_MAX) failed: %s",
				emGetErrorText(errno).Get()
			);
			goto L_ChildErr;
		}
		for (i=0; i<n; i++) {
			if (
				i==STDIN_FILENO &&
				(flags&(emProcess::SF_SHARE_STDIN|emProcess::SF_PIPE_STDIN))!=0
			) continue;
			if (
				i==STDOUT_FILENO &&
				(flags&(emProcess::SF_SHARE_STDOUT|emProcess::SF_PIPE_STDOUT))!=0
			) continue;
			if (
				i==STDERR_FILENO &&
				(flags&(emProcess::SF_SHARE_STDERR|emProcess::SF_PIPE_STDERR))!=0
			) continue;
			if (i==pipeTmp[1]) continue;
			close(i);
		}

		if (fcntl(pipeTmp[1],F_SETFD,FD_CLOEXEC)<0) {
			msg=emString::Format(
				"fcntl(pipeTmp[1],F_SETFD,FD_CLOEXEC) failed: %s",
				emGetErrorText(errno).Get()
			);
			goto L_ChildErr;
		}

		cargs=new char*[args.GetCount()+1];
		for (i=0; i<args.GetCount(); i++) {
			cargs[i]=(char*)args[i].Get();
		}
		cargs[i]=NULL;

		if (!managed) {
			pid=fork();
			if (pid<0) {
				msg=emString::Format(
					"fork failed: %s",
					emGetErrorText(errno).Get()
				);
				goto L_ChildErr;
			}
			if (pid!=0) _exit(0);
			setsid();
				// setsid() makes this unmanaged process a group
				// and session leader. Should we make it just a
				// group leader with setpgid(0,0) instead?
		}

		execvp(cargs[0],cargs);
		msg=emGetErrorText(errno);
L_ChildErr:
		i=0;
		len=msg.GetLen();
		while (i<len) {
			r=(int)write(pipeTmp[1],msg.Get()+i,len-i);
			if (r<=0) break;
			i+=r;
		}
		_exit(-1);

		//--------- End of child process ---------
	}

	close(pipeTmp[1]);
	pipeTmp[1]=-1;
	len=0;
	do {
		r=(int)read(pipeTmp[0],buf+len,sizeof(buf)-1-len);
		if (r<=0) break;
		len+=r;
	} while(len<(int)sizeof(buf)-1);
	if (len>0) {
		msg=emString(buf,len);
		goto L_Err;
	}
	close(pipeTmp[0]);
	pipeTmp[0]=-1;

	if (managed) {
		if (pipeIn[0]!=-1) close(pipeIn[0]);
		if (pipeOut[1]!=-1) close(pipeOut[1]);
		if (pipeErr[1]!=-1) close(pipeErr[1]);
		managed->Pid=pid;
		managed->Arg0=args[0];
		managed->FdIn=pipeIn[1];
		managed->FdOut=pipeOut[0];
		managed->FdErr=pipeErr[0];
		managed->ExitStatus=0;
	}
	else {
		for (;;) {
			pr=waitpid(pid,NULL,0);
			if (pr==pid) break;
			if (pr<0) {
				if (errno==EINTR) continue;
				emFatalError("emProcess: waitpid failed: %s",emGetErrorText(errno).Get());
			}
			else {
				emFatalError("emProcess: unexpected return value from waitpid.");
			}
		}
	}
	return;

L_PipeErr:
	msg=emString::Format(
		"Pipe creation failed: %s",
		emGetErrorText(errno).Get()
	);
	goto L_Err;
L_GetFlErr:
	msg=emString::Format(
		"fcntl(...,F_GETFL) failed: %s",
		emGetErrorText(errno).Get()
	);
	goto L_Err;
L_SetFlErr:
	msg=emString::Format(
		"fcntl(...,F_SETFL) failed: %s",
		emGetErrorText(errno).Get()
	);
	goto L_Err;
L_ForkErr:
	msg=emString::Format(
		"fork() failed: %s",
		emGetErrorText(errno).Get()
	);
	goto L_Err;
L_Err:
	if (pid!=-1) {
		for (;;) {
			pr=waitpid(pid,NULL,0);
			if (pr==pid) break;
			if (pr<0) {
				if (errno==EINTR) continue;
				emFatalError("emProcess: waitpid failed: %s",emGetErrorText(errno).Get());
			}
			else {
				emFatalError("emProcess: unexpected return value from waitpid.");
			}
		}
	}
	if (pipeIn[0]!=-1) close(pipeIn[0]);
	if (pipeIn[1]!=-1) close(pipeIn[1]);
	if (pipeOut[0]!=-1) close(pipeOut[0]);
	if (pipeOut[1]!=-1) close(pipeOut[1]);
	if (pipeErr[0]!=-1) close(pipeErr[0]);
	if (pipeErr[1]!=-1) close(pipeErr[1]);
	if (pipeTmp[0]!=-1) close(pipeTmp[0]);
	if (pipeTmp[1]!=-1) close(pipeTmp[1]);
	throw emString::Format(
		"Failed to start process \"%s\": %s",
		args[0].Get(),
		msg.Get()
	);
}


#endif
