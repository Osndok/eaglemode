/*------------------------------------------------------------------------------
// emPsWinAdapterProc.c
//
// Copyright (C) 2017-2018 Oliver Hamann.
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

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>


int main(int argc, char * argv[])
{
	char * command;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	MSG msg;
	DWORD d,exitCode;
	BOOL b,gotWMQuit,childProcExited;
	int cmdMemSize,cmdLen,len,i;

	setmode(fileno(stdout),O_BINARY);
	setmode(fileno(stdin),O_BINARY);
	setbuf(stderr,NULL);

	if (argc<2) {
		fprintf(stderr,"%s: Invalid arguments.\n",argv[0]);
		ExitProcess(255);
	}

	cmdMemSize=65536;
	command=malloc(cmdMemSize);
	cmdLen=0;
	for (i=1; i<argc; i++) {
		len=strlen(argv[i]);
		if (cmdMemSize<cmdLen+len+4) {
			fprintf(stderr,"%s: Command line too long\n",argv[0]);
			ExitProcess(255);
		}
		if (cmdLen>0) command[cmdLen++]=' ';
		if (strchr(argv[i],'"')) {
			fprintf(stderr,"%s: Quotes in arguments not supported\n",argv[0]);
			ExitProcess(255);
		}
		command[cmdLen++]='"';
		memcpy(command+cmdLen,argv[i],len);
		cmdLen+=len;
		command[cmdLen++]='"';
	}
	command[cmdLen]=0;

	memset(&si,0,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags=STARTF_USESTDHANDLES;
	si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
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

	childProcExited=FALSE;
	for (;;) {
		gotWMQuit=FALSE;
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT) gotWMQuit=TRUE;
		}
		if (gotWMQuit) break;
		SetLastError(ERROR_SUCCESS);
		d=MsgWaitForMultipleObjects(1,&pi.hProcess,FALSE,INFINITE,QS_ALLPOSTMESSAGE);
		if (d==WAIT_OBJECT_0) {
			childProcExited=TRUE;
			break;
		}
		else if (d!=WAIT_OBJECT_0+1) {
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
