/*------------------------------------------------------------------------------
// emTmpConvProc.c
//
// Copyright (C) 2006-2008,2011,2017,2019-2020 Oliver Hamann.
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
//----------------------------------------------------------------------------*/


/*
 * This slave process manages a child process which performs the file
 * conversion. The child process is a /bin/sh (or cmd on Windows) which runs the
 * command. That child process gets its own process group for that we are able
 * to terminate all processes in that group.
 *
 * This process also listens to several signals (or WM_QUIT on Windows) for
 * terminating itself and the process group properly.
 *
 * Finally, the parent process should have created a pipe whose read end is
 * STDIN for this process. We never get any data through that pipe, but we
 * detect the closing of the write end. This way, this process automatically
 * performs the termination as soon as the parent process closes the write end -
 * and this even works when the parent process dies abnormally.
 */


#if defined(_WIN32)
/*------------------------------ Windows variant -----------------------------*/

#include <stdio.h>
#include <windows.h>


static DWORD WINAPI readingThreadProc(LPVOID data)
{
	char buf[64];
	HANDLE h;
	DWORD d;
	BOOL b;

	h=GetStdHandle(STD_INPUT_HANDLE);

	do {
		b=ReadFile(h,buf,sizeof(buf),&d,NULL);
	} while(b);

	return 0;
}


int main(int argc, char * argv[])
{
	HANDLE hReadingThread;
	HANDLE handles[2];
	char * command;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	MSG msg;
	DWORD d,exitCode;
	BOOL b,gotWMQuit,childProcExited;

	setbuf(stderr,NULL);

	if (argc!=2) {
		fprintf(stderr,"%s: Invalid arguments.\n",argv[0]);
		ExitProcess(255);
	}

	/* Note: If we ever say /V:ON below, somehow a quoting of exclamation
	   marks in file names in %INFILE% and %OUTFILE% would be required. */
	command=malloc(strlen(argv[1])+64);
	sprintf(command,"cmd /E:ON /F:OFF /V:OFF /C %s",argv[1]);

	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESTDHANDLES;
	si.hStdInput=INVALID_HANDLE_VALUE;
	si.hStdOutput=INVALID_HANDLE_VALUE;
	si.hStdError=GetStdHandle(STD_ERROR_HANDLE);

	b=CreateProcess(
		NULL,
		command,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_PROCESS_GROUP,
		NULL,
		NULL,
		&si,
		&pi
	);
	if (!b) {
		fprintf(
			stderr,
			"%s: Failed to execute %s (err=0x%lX)\n",
			argv[0],command,(long)GetLastError()
		);
		ExitProcess(255);
	}

	hReadingThread=CreateThread(NULL,0,readingThreadProc,NULL,0,&d);

	childProcExited=FALSE;
	for (;;) {
		gotWMQuit=FALSE;
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT) gotWMQuit=TRUE;
		}
		if (gotWMQuit) break;
		handles[0]=pi.hProcess;
		handles[1]=hReadingThread;
		SetLastError(ERROR_SUCCESS);
		d=MsgWaitForMultipleObjects(2,handles,FALSE,INFINITE,QS_ALLPOSTMESSAGE);
		if (d==WAIT_OBJECT_0) {
			childProcExited=TRUE;
			break;
		}
		else if (d==WAIT_OBJECT_0+1) {
			break;
		}
		else if (d!=WAIT_OBJECT_0+2) {
			fprintf(
				stderr,
				"%s: Wait failed (d=%ld, err=0x%lX)\n",
				argv[0],d,(long)GetLastError()
			);
			GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,pi.dwProcessId);
			WaitForSingleObject(pi.hProcess,INFINITE);
			ExitProcess(255);
		}
	}

	if (!childProcExited) {
		b=GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,pi.dwProcessId);
		if (!b) {
			fprintf(
				stderr,
				"%s: Failed to send Ctrl+Break to child process (err=0x%lX)\n",
				argv[0],(long)GetLastError()
			);
			ExitProcess(255);
		}
		SetLastError(ERROR_SUCCESS);
		d=WaitForSingleObject(pi.hProcess,INFINITE);
		if (d!=WAIT_OBJECT_0) {
			fprintf(
				stderr,
				"%s: Failed to wait for child process (d=%ld, err=0x%lX)\n",
				argv[0],d,(long)GetLastError()
			);
			ExitProcess(255);
		}
	}

	if (!GetExitCodeProcess(pi.hProcess,&exitCode)) {
		fprintf(
			stderr,
			"%s: Failed to get exit code from child process (error=0x%lX)\n",
			argv[0],(long)GetLastError()
		);
		ExitProcess(255);
	}

	ExitProcess(exitCode);
	return 0;
}


#else
/*-------------------------------- UNIX variant ------------------------------*/

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


static void SigHandler(int signum)
{
	close(STDIN_FILENO);
}


int main(int argc, char * argv[])
{
	char buf[8];
	char * cargs[10];
	int i,n,status;
	pid_t pid,res;
	ssize_t s;

	if (argc!=2) {
		fprintf(stderr,"%s: Invalid arguments.\n",argv[0]);
		exit(255);
	}

	signal(SIGHUP,SigHandler);
	signal(SIGINT,SigHandler);
	signal(SIGQUIT,SigHandler);
	signal(SIGPIPE,SigHandler);
	signal(SIGTERM,SigHandler);
	signal(SIGCHLD,SigHandler);

	pid=fork();
	if (pid<0) {
		fprintf(stderr,"%s: fork failed: %s\n",argv[0],strerror(errno));
		exit(255);
	}
	if (pid==0) {
		setpgid(getpid(),getpid());

		n=sysconf(_SC_OPEN_MAX);
		if (n<=0) {
			fprintf(stderr,"%s: sysconf(_SC_OPEN_MAX) failed.\n",argv[0]);
			exit(255);
		}
		for (i=0; i<n; i++) {
			if (i!=STDERR_FILENO) {
				close(i);
			}
		}

		i=0;
#if defined(ANDROID)
		cargs[i++]=(char*)"/system/bin/sh";
#else
		cargs[i++]=(char*)"/bin/sh";
#endif
		/* cargs[i++]=(char*)"-e"; */
		/* Option -e is not portable! */
		cargs[i++]=(char*)"-c";
		cargs[i++]=argv[1];
		cargs[i++]=NULL;
		execvp(cargs[0],cargs);
		fprintf(stderr,"%s: exec %s failed failed: %s\n",argv[0],cargs[0],strerror(errno));
		exit(255);
	}
	setpgid(pid,pid);

	for (;;) {
		s=read(STDIN_FILENO,buf,sizeof(buf));
		if (s>0) continue;
		if (s==0) break;
		if (errno!=EINTR) break;
	}

	kill(-pid,SIGTERM);
	for (;;) {
		res=waitpid(pid,&status,0);
		if (res==pid) {
			if (WIFEXITED(status)) status=WEXITSTATUS(status);
			else status=128+WTERMSIG(status);
			exit(status);
		}
		if (res!=-1) {
			fprintf(stderr,"%s: Unexpected return value from waitpid: %d\n",argv[0],(int)res);
			exit(255);
		}
		if (errno!=EINTR) {
			fprintf(stderr,"%s: waitpid failed: %s\n",argv[0],strerror(errno));
			exit(255);
		}
	}
}


#endif
