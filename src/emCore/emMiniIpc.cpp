//------------------------------------------------------------------------------
// emMiniIpc.cpp - Minimalistic support for interprocess communication
//
// Copyright (C) 2004-2009,2011 Oliver Hamann.
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

#include <emCore/emMiniIpc.h>


//==============================================================================
//==================== System dependent stuff for emMiniIpc ====================
//==============================================================================

// Required functions:
//  emMiniIpc_OpenServer    - Open server by name, return instance stuff on
//                            success, or NULL if another server of that name
//                            already exists.
//  emMiniIpc_Receive       - Receive as much data as available.
//  emMiniIpc_CloseServer   - Close server instance.
//  emMiniIpc_TrySendAtomic - Send data to a server (atomic operation), throw
//                            message on error.
//  emMiniIpc_CleanUpFiles  - Delete any files left by an abnormal termination.


#if defined(_WIN32) || defined(__CYGWIN__)
//------------------------------ Windows variant -------------------------------

#include <windows.h>
#ifndef FILE_FLAG_FIRST_PIPE_INSTANCE
#	define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000
#endif


static emString emMiniIpc_CalcPipePath(const char * serverName)
{
	emArray<char> idBuf;
	emString hostName,userName,idHash;

	hostName=emGetHostName();
	userName=emGetUserName();
	idBuf.SetTuningLevel(4);
	idBuf.Add(hostName.Get(),hostName.GetLen()+1);
	idBuf.Add(userName.Get(),userName.GetLen()+1);
	idBuf.Add(serverName,strlen(serverName));
	idHash=emCalcHashName(idBuf.Get(),idBuf.GetCount(),40);
	return emString::Format(
		"\\\\.\\pipe\\emMiniIpc-%s",
		idHash.Get()
	);
}


struct emMiniIpc_ServerInstance {
	emString PipePath;
	HANDLE PipeHandle;
	HANDLE EventHandle;
	OVERLAPPED Overlapped;
	int State;
	char InBuf[256];
	DWORD InBufFill;
};


static emMiniIpc_ServerInstance * emMiniIpc_OpenServer(const char * serverName)
{
	emMiniIpc_ServerInstance * inst;

	inst=new emMiniIpc_ServerInstance;
	inst->PipeHandle=NULL;
	inst->EventHandle=NULL;
	memset(&inst->Overlapped,0,sizeof(inst->Overlapped));
	inst->State=0;
	inst->InBufFill=0;

	inst->PipePath=emMiniIpc_CalcPipePath(serverName);

	inst->PipeHandle=CreateNamedPipe(
		inst->PipePath.Get(),
		PIPE_ACCESS_INBOUND|FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE,
		0,
		1,
		4096,
		4096,
		2000,
		NULL
	);
	if (inst->PipeHandle==INVALID_HANDLE_VALUE) {
		delete inst;
		return NULL;
	}

	inst->EventHandle=CreateEvent(NULL,TRUE,FALSE,NULL);
	if (inst->EventHandle==NULL) {
		emFatalError(
			"emMiniIpc: CreateEvent failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

	return inst;
}


static void emMiniIpc_Receive(
	emMiniIpc_ServerInstance * inst, emArray<char> * buf
)
{
	DWORD d;

	for (;;) {
		if (inst->State==0) {
			ResetEvent(inst->EventHandle);
			memset(&inst->Overlapped,0,sizeof(inst->Overlapped));
			inst->Overlapped.hEvent=inst->EventHandle;
			if (ConnectNamedPipe(inst->PipeHandle,&inst->Overlapped)) {
				inst->State=2;
			}
			else if (GetLastError()==ERROR_IO_PENDING) {
				inst->State=1;
			}
			else {
				emFatalError(
					"emMiniIpc: ConnectNamedPipe failed: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
		}
		else if (inst->State==1) {
			d=WaitForSingleObject(inst->EventHandle,0);
			if (d==WAIT_TIMEOUT) break;
			if (d!=WAIT_OBJECT_0) {
				emFatalError(
					"emMiniIpc: WaitForSingleObject failed: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
			if (!GetOverlappedResult(inst->PipeHandle,&inst->Overlapped,&d,FALSE)) {
				emFatalError(
					"emMiniIpc: ConnectNamedPipe failed: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
			inst->State=2;
		}
		else if (inst->State==2) {
			ResetEvent(inst->EventHandle);
			memset(&inst->Overlapped,0,sizeof(inst->Overlapped));
			inst->Overlapped.hEvent=inst->EventHandle;
			if (ReadFile(inst->PipeHandle,inst->InBuf,sizeof(inst->InBuf),&inst->InBufFill,&inst->Overlapped)) {
				if (inst->InBufFill==0) break;
				buf->Add(inst->InBuf,inst->InBufFill);
			}
			else if (GetLastError()==ERROR_IO_PENDING) {
				inst->State=3;
			}
			else {
				DisconnectNamedPipe(inst->PipeHandle);
				inst->State=0;
			}
		}
		else if (inst->State==3) {
			d=WaitForSingleObject(inst->EventHandle,0);
			if (d==WAIT_TIMEOUT) break;
			if (d!=WAIT_OBJECT_0) {
				emFatalError(
					"emMiniIpc: WaitForSingleObject failed: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
			if (!GetOverlappedResult(inst->PipeHandle,&inst->Overlapped,&d,FALSE)) {
				DisconnectNamedPipe(inst->PipeHandle);
				inst->State=0;
			}
			buf->Add(inst->InBuf,d);
			inst->State=2;
		}
	}
}


static void emMiniIpc_CloseServer(emMiniIpc_ServerInstance * inst)
{
	CancelIo(inst->PipeHandle);
	DisconnectNamedPipe(inst->PipeHandle);
	CloseHandle(inst->PipeHandle);
	CloseHandle(inst->EventHandle);
	delete inst;
}


static void emMiniIpc_TrySendAtomic(
	const char * serverName, const char * data, int len
) throw(emString)
{
	emString pipePath;
	HANDLE handle;
	DWORD d;

	pipePath=emMiniIpc_CalcPipePath(serverName);

	for (;;) {
		handle=CreateFile(
			pipePath.Get(),
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);
		if (handle!=INVALID_HANDLE_VALUE) break;
		if (GetLastError()!=ERROR_PIPE_BUSY) {
			throw emString::Format(
				"CreateFile with OPEN_EXISTING on \"%s\" failed: %s",
				pipePath.Get(),
				emGetErrorText(GetLastError()).Get()
			);
		}
		WaitNamedPipe(pipePath.Get(),1000);
	}

	while (len>0) {
		if (!WriteFile(handle,data,len,&d,NULL)) {
			d=GetLastError();
			CloseHandle(handle);
			throw emString::Format(
				"Failed to write to \"%s\": %s",
				pipePath.Get(),
				emGetErrorText(d).Get()
			);
		}
		if (d==0) {
			CloseHandle(handle);
			throw emString::Format(
				"Failed to write to \"%s\": end of file",
				pipePath.Get()
			);
		}
		data+=d;
		len-=d;
	}
	CloseHandle(handle);
}


static void emMiniIpc_CleanUpFiles()
{
}


#else
//-------------------------------- UNIX variant --------------------------------

#	include <fcntl.h>
#	include <unistd.h>
#	include <emCore/emInstallInfo.h>


static int emMiniIpc_Lock(const char * lockFilePath)
{
	struct flock sfl;
	int handle;

	// Yes, with flock instead of fcntl we could even lock the fifo files
	// (no extra lock files required). But flock is not so portable.

	handle=open(lockFilePath,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if (handle==-1) {
		emFatalError(
			"emMiniIpc_Lock: Failed to open or create \"%s\": %s",
			lockFilePath,
			emGetErrorText(errno).Get()
		);
	}

	for (;;) {
		memset(&sfl,0,sizeof(sfl));
		sfl.l_type=F_WRLCK;
		sfl.l_whence=SEEK_SET;
		if (fcntl(handle,F_SETLKW,&sfl)==0) break;
		if (errno!=EINTR) {
			emFatalError(
				"emMiniIpc_Lock: Failed to lock \"%s\": %s",
				lockFilePath,
				emGetErrorText(errno).Get()
			);
		}
	}

	return handle;
}


static void emMiniIpc_Unlock(int lockFileHandle)
{
	struct flock sfl;

	memset(&sfl,0,sizeof(sfl));
	sfl.l_type=F_UNLCK;
	sfl.l_whence=SEEK_SET;
	fcntl(lockFileHandle,F_SETLK,&sfl);
	close(lockFileHandle);
}


static emString emMiniIpc_CalcFifoDir()
{
	return emGetChildPath(
		emGetInstallPath(EM_IDT_TMP,"emCore"),
		emString::Format("emMiniIpc-%s",emGetUserName().Get())
	);
}


static emString emMiniIpc_CalcFifoBaseName(const char * serverName)
{
	emArray<char> idBuf;
	emString hostName,userName;

	hostName=emGetHostName();
	userName=emGetUserName();
	idBuf.SetTuningLevel(4);
	idBuf.Add(hostName.Get(),hostName.GetLen()+1);
	idBuf.Add(userName.Get(),userName.GetLen()+1);
	idBuf.Add(serverName,strlen(serverName));
	return emCalcHashName(idBuf.Get(),idBuf.GetCount(),40);
}


static const char * const emMiniIpc_FifoEnding=".f.autoremoved";
static const char * const emMiniIpc_FifoLockEnding=".l.autoremoved";
static const char * const emMiniIpc_FifoCreationLockFileName="fifo-creation.lock";
	// Remember: Files with FifoEnding and FifoLockEnding are auto-removed
	// by CleanUpFiles. The ending of FifoCreationLockFileName must differ.


struct emMiniIpc_ServerInstance {
	emString FifoDir;
	emString FifoBaseName;
	emString FifoPath;
	emString FifoLockPath;
	emString FifoCreationLockPath;
	int FifoHandle;
};


static emMiniIpc_ServerInstance * emMiniIpc_OpenServer(const char * serverName)
{
	emMiniIpc_ServerInstance * inst;
	struct stat statbuf;
	int lockHandle,fd;

	inst=new emMiniIpc_ServerInstance;

	inst->FifoDir=emMiniIpc_CalcFifoDir();

	inst->FifoBaseName=emMiniIpc_CalcFifoBaseName(serverName);

	inst->FifoPath=emString::Format(
		"%s/%s%s",
		inst->FifoDir.Get(),
		inst->FifoBaseName.Get(),
		emMiniIpc_FifoEnding
	);

	inst->FifoLockPath=emString::Format(
		"%s/%s%s",
		inst->FifoDir.Get(),
		inst->FifoBaseName.Get(),
		emMiniIpc_FifoLockEnding
	);

	inst->FifoCreationLockPath=emString::Format(
		"%s/%s",
		inst->FifoDir.Get(),
		emMiniIpc_FifoCreationLockFileName
	);

	inst->FifoHandle=-1;

	try {
		emTryMakeDirectories(inst->FifoDir,0700);
	}
	catch (emString errorMessage) {
		emFatalError("emMiniIpc_OpenServer: %s",errorMessage.Get());
	}

	lockHandle=emMiniIpc_Lock(inst->FifoCreationLockPath);

	for (;;) {
		if (stat(inst->FifoPath,&statbuf)==0) {
			if (!S_ISFIFO(statbuf.st_mode)) break;
			fd=open(inst->FifoPath,O_WRONLY|O_NONBLOCK);
			if (fd!=-1) {
				close(fd);
				break;
			}
			try {
				emTryRemoveFileOrTree(inst->FifoPath);
			}
			catch (emString) {
				break;
			}
		}
		if (mkfifo(inst->FifoPath,S_IRUSR|S_IWUSR)!=0) {
			if (errno==EEXIST) break;
			emFatalError(
				"emMiniIpc_OpenServer: Failed to create fifo file \"%s\": %s",
				inst->FifoPath.Get(),
				emGetErrorText(errno).Get()
			);
		}
		inst->FifoHandle=open(inst->FifoPath,O_RDONLY|O_NONBLOCK);
		if (inst->FifoHandle==-1) {
			emFatalError(
				"emMiniIpc_OpenServer: Failed to open created fifo file \"%s\": %s",
				inst->FifoPath.Get(),
				emGetErrorText(errno).Get()
			);
		}
		break;
	}

	emMiniIpc_Unlock(lockHandle);

	if (inst->FifoHandle==-1) {
		delete inst;
		inst=NULL;
	}

	return inst;
}


static void emMiniIpc_Receive(
	emMiniIpc_ServerInstance * inst, emArray<char> * buf
)
{
	char tmp[256];
	int len;

	for (;;) {
		len=read(inst->FifoHandle,tmp,sizeof(tmp));
		if (len<=0) break;
		buf->Add(tmp,len);
	}
}


static void emMiniIpc_CloseServer(emMiniIpc_ServerInstance * inst)
{
	int lockHandle;

	lockHandle=emMiniIpc_Lock(inst->FifoCreationLockPath);

	close(inst->FifoHandle);

	try {
		emTryRemoveFileOrTree(inst->FifoPath);
		emTryRemoveFileOrTree(inst->FifoLockPath);
	}
	catch (emString) {
	}

	emMiniIpc_Unlock(lockHandle);

	delete inst;
}


static void emMiniIpc_TrySendAtomic(
	const char * serverName, const char * data, int len
) throw(emString)
{
	emString fifoDir,fifoBaseName,fifoPath,fifoLockPath;
	struct stat statbuf;
	int flags,handle,lockHandle,e,l;

	fifoDir=emMiniIpc_CalcFifoDir();

	fifoBaseName=emMiniIpc_CalcFifoBaseName(serverName);

	fifoPath=emString::Format(
		"%s/%s%s",
		fifoDir.Get(),
		fifoBaseName.Get(),
		emMiniIpc_FifoEnding
	);

	fifoLockPath=emString::Format(
		"%s/%s%s",
		fifoDir.Get(),
		fifoBaseName.Get(),
		emMiniIpc_FifoLockEnding
	);

	handle=open(fifoPath.Get(),O_WRONLY|O_NONBLOCK);
	if (handle==-1) {
		throw emString::Format(
			"Failed to open \"%s\": %s",
			fifoPath.Get(),
			emGetErrorText(errno).Get()
		);
	}

	if (
		(flags=fcntl(handle,F_GETFL))==-1 ||
		fcntl(handle,F_SETFL,flags&~O_NONBLOCK)==-1
	) {
		e=errno;
		close(handle);
		throw emString::Format(
			"Failed to fcntl \"%s\": %s",
			fifoPath.Get(),
			emGetErrorText(e).Get()
		);
	}

	if (fstat(handle,&statbuf)!=0) {
		e=errno;
		close(handle);
		throw emString::Format(
			"Failed to stat \"%s\": %s",
			fifoPath.Get(),
			emGetErrorText(e).Get()
		);
	}

	if (!S_ISFIFO(statbuf.st_mode)) {
		close(handle);
		throw emString::Format(
			"\"%s\" is not a fifo",
			fifoPath.Get()
		);
	}

	lockHandle=emMiniIpc_Lock(fifoLockPath);

	while (len) {
		l=write(handle,data,len);
		if (l<=0) {
			if (l==0) e=EINVAL; else e=errno;
			close(handle);
			emMiniIpc_Unlock(lockHandle);
			throw emString::Format(
				"Failed to write to \"%s\": %s",
				fifoPath.Get(),
				emGetErrorText(e).Get()
			);
		}
		data+=l;
		len-=l;
	}

	close(handle);

	emMiniIpc_Unlock(lockHandle);
}


static void emMiniIpc_CleanUpFiles()
{
	emString fifoDir,name,fifoPath,fifoLockPath,fifoCreationLockPath;
	emArray<emString> list;
	struct stat statbuf;
	int lockHandle,fd,i,le,lb;

	fifoDir=emMiniIpc_CalcFifoDir();

	fifoCreationLockPath=emString::Format(
		"%s/%s",
		fifoDir.Get(),
		emMiniIpc_FifoCreationLockFileName
	);

	try {
		list=emTryLoadDir(fifoDir);
	}
	catch (emString) {
		return;
	}

	lockHandle=emMiniIpc_Lock(fifoCreationLockPath);

	for (i=0; i<list.GetCount(); i++) {
		name=list[i];
		le=strlen(emMiniIpc_FifoEnding);
		lb=name.GetLen()-le;
		if (lb<=0 || strcmp(name.Get()+lb,emMiniIpc_FifoEnding)!=0) continue;
		fifoPath=emGetChildPath(fifoDir,name);
		if (stat(fifoPath,&statbuf)!=0) continue;
		if (!S_ISFIFO(statbuf.st_mode)) continue;
		fd=open(fifoPath,O_WRONLY|O_NONBLOCK);
		if (fd!=-1) {
			close(fd);
			continue;
		}
		try {
			emTryRemoveFileOrTree(fifoPath);
		}
		catch (emString) {
		}
	}

	for (i=0; i<list.GetCount(); i++) {
		name=list[i];
		le=strlen(emMiniIpc_FifoLockEnding);
		lb=name.GetLen()-le;
		if (lb<=0 || strcmp(name.Get()+lb,emMiniIpc_FifoLockEnding)!=0) continue;
		fifoPath=emGetChildPath(
			fifoDir,
			name.GetSubString(0,lb) + emMiniIpc_FifoEnding
		);
		if (emIsExistingPath(fifoPath)) continue;
		fifoLockPath=emGetChildPath(fifoDir,name);
		try {
			emTryRemoveFileOrTree(fifoLockPath);
		}
		catch (emString) {
		}
	}

	emMiniIpc_Unlock(lockHandle);
}


#endif


//==============================================================================
//============================== emMiniIpcClient ===============================
//==============================================================================

void emMiniIpcClient::TrySend(
	const char * serverName, int argc, const char * const argv[]
) throw(emString)
{
	emArray<char> data;
	char tmp[256];
	int i;

	data.SetTuningLevel(4);
	sprintf(tmp,"%d",argc);
	data.Add(tmp,strlen(tmp)+1);
	for (i=0; i<argc; i++) data.Add(argv[i],strlen(argv[i])+1);

	try {
		emMiniIpc_TrySendAtomic(serverName,data.Get(),data.GetCount());
	}
	catch (emString errorMessage) {
		throw emString::Format(
			"Failed to find or access emMiniIpc server \"%s\" (%s)",
			serverName,
			errorMessage.Get()
		);
	}
}


//==============================================================================
//============================== emMiniIpcServer ===============================
//==============================================================================

emMiniIpcServer::emMiniIpcServer(emScheduler & scheduler)
	: Scheduler(scheduler)
{
	Instance=NULL;
	Buffer.SetTuningLevel(4);
	ServerEngine=NULL;
	PtrStoppedOrDestructed=NULL;
}


emMiniIpcServer::~emMiniIpcServer()
{
	StopServing();
}


void emMiniIpcServer::StartServing(const char * userDefinedServerName)
{
	static int counter=0;
	emUInt64 codes[5];
	int i;

	StopServing();

	emMiniIpc_CleanUpFiles();

	ServerEngine=new SEClass(*this);

	if (userDefinedServerName) {
		ServerName=userDefinedServerName;
		Instance=emMiniIpc_OpenServer(ServerName);
		if (!Instance) {
			// This is okay (another server of same name exists).
			// It will be tried again later through Poll.
		}
	}
	else {
		for (i=0; ; i++) {
			codes[0]=emGetProcessId();
			codes[1]=counter++;
			codes[2]=((char*)this)-((char*)NULL);
			codes[3]=emGetClockMS();
			codes[4]=i;
			ServerName=emString::Format(
				"%x.generic",
				emCalcAdler32((const char*)codes,sizeof(codes))
			);
			Instance=emMiniIpc_OpenServer(ServerName);
			if (Instance) break;
			if (i>=1000) {
				emFatalError("emMiniIpcServer::StartServing: Giving up.");
			}
		}
	}
}


void emMiniIpcServer::StopServing()
{
	if (PtrStoppedOrDestructed) {
		*PtrStoppedOrDestructed=true;
		PtrStoppedOrDestructed=NULL;
	}
	if (ServerEngine) {
		delete ServerEngine;
		ServerEngine=NULL;
	}
	Buffer.Empty();
	if (Instance) {
		emMiniIpc_CloseServer((emMiniIpc_ServerInstance*)Instance);
		Instance=NULL;
	}
	ServerName.Empty();
}


void emMiniIpcServer::Poll()
{
	emArray<const char *> argv;
	const char * p, * p2, * pe;
	int i,argc,oldLen;
	bool stoppedOrDestructed;

	if (!Instance) {
		Instance=emMiniIpc_OpenServer(ServerName);
		if (!Instance) return;
	}

	oldLen=Buffer.GetCount();
	emMiniIpc_Receive((emMiniIpc_ServerInstance*)Instance,&Buffer);
	if (Buffer.GetCount()==oldLen) return;

	while (!Buffer.IsEmpty()) {
		p=Buffer;
		pe=p+Buffer.GetCount();
		p2=(const char*)memchr(p,0,pe-p);
		if (!p2) return;
		argc=atoi(p);
		p=p2+1;
		argv.SetTuningLevel(4);
		argv.SetCount(argc);
		for (i=0; i<argc; i++) {
			p2=(const char*)memchr(p,0,pe-p);
			if (!p2) return;
			argv.Set(i,p);
			p=p2+1;
		}
		stoppedOrDestructed=false;
		PtrStoppedOrDestructed=&stoppedOrDestructed;
		OnReception(argc,argv);
		if (stoppedOrDestructed) return;
		PtrStoppedOrDestructed=NULL;
		Buffer.Remove(0,p-Buffer.Get());
	}
}


emMiniIpcServer::SEClass::SEClass(emMiniIpcServer & server)
	: emEngine(server.GetScheduler()),
	Server(server),
	Timer(server.GetScheduler())
{
	AddWakeUpSignal(Timer.GetSignal());
	Timer.Start(0);
}


emMiniIpcServer::SEClass::~SEClass()
{
}


bool emMiniIpcServer::SEClass::Cycle()
{
	Server.Poll();
	Timer.Start(200);
	return false;
}
