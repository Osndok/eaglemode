/*------------------------------------------------------------------------------
// emWndsAdapterProc.c
//
// Copyright (C) 2017-2018,2022 Oliver Hamann.
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


static void addToCmd(char * * pCmd, const char * cmdEnd, char c)
{
	char * cmd = *pCmd;
	if (cmd>=cmdEnd) {
			fprintf(stderr,"Command line too long\n");
			ExitProcess(255);
	}
	*cmd++=c;
	*pCmd=cmd;
}


static const char * getLastErrorText()
{
	static char text[512];
	DWORD errorNumber;

	errorNumber=GetLastError();
	if (!FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNumber,
		0,
		text,
		sizeof(text)-1,
		NULL
	)) {
		sprintf(text,"error #%d",errorNumber);
	}
	return text;
}


int main(int argc, char * argv[])
{
	char * command, * p, * e;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	MSG msg;
	DWORD d,exitCode;
	BOOL b,gotWMQuit,childProcExited;
	int cmdMemSize,i,j,k;
	char c;

	setmode(fileno(stdout),O_BINARY);
	setmode(fileno(stdin),O_BINARY);
	setbuf(stderr,NULL);

	if (argc<2) {
		fprintf(stderr,"%s:\nInvalid arguments.\n",argv[0]);
		ExitProcess(255);
	}

	cmdMemSize=65536;
	command=malloc(cmdMemSize);
	p=command;
	e=command+cmdMemSize;

	for (i=1; i<argc; i++) {
		if (i>1) {
			addToCmd(&p,e,' ');
			for (j=0; argv[i][j]; j++) {
				c=argv[i][j];
				if ((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') &&
				    c!='\\' && c!='/' && c!='.' && c!=':' && c!='-') break;
			}
			if (!argv[i][j] && argv[i][0]) {
				for (j=0; argv[i][j]; j++) addToCmd(&p,e,argv[i][j]);
				continue;
			}
		}
		addToCmd(&p,e,'"');
		for (j=0, k=0; argv[i][j]; j++) {
			c=argv[i][j];
			if (c=='"') {
				while (k>0) { addToCmd(&p,e,'\\'); k--; }
				addToCmd(&p,e,'\\');
			}
			else if (c=='\\') k++;
			else k=0;
			addToCmd(&p,e,c);
		}
		while (k>0) { addToCmd(&p,e,'\\'); k--; }
		addToCmd(&p,e,'"');
	}
	addToCmd(&p,e,'\0');
	addToCmd(&p,e,'\0');

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
			"%s:\nFailed to execute %s:\n%s\n",
			argv[0],command,getLastErrorText()
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
				"%s:\nWait failed (d=%ld):\n%s\n",
				argv[0],d,getLastErrorText()
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
				"%s:\nFailed to send Ctrl+Break to child process:\n%s\n",
				argv[0],getLastErrorText()
			);
			ExitProcess(255);
		}
		SetLastError(ERROR_SUCCESS);
		d=WaitForSingleObject(pi.hProcess,INFINITE);
		if (d!=WAIT_OBJECT_0) {
			fprintf(
				stderr,
				"%s:\nFailed to wait for child process (d=%ld):\n%s\n",
				argv[0],d,getLastErrorText()
			);
			ExitProcess(255);
		}
	}

	if (!GetExitCodeProcess(pi.hProcess,&exitCode)) {
		fprintf(
			stderr,
			"%s:\nFailed to get exit code from child process:\n%s\n",
			argv[0],getLastErrorText()
		);
		ExitProcess(255);
	}

	ExitProcess(exitCode);
	return 0;
}
