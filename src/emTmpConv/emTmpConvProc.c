/*------------------------------------------------------------------------------
// emTmpConvProc.c
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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


#if defined(_WIN32)
/*------------------------------ Windows variant -----------------------------*/

#include <stdio.h>

int main(int argc, char * argv[])
{
	/**
	 * Hint: If this is ever developed, it should become a windowed
	 * executable without window. Otherwise a console pops up on each run.
	 * This also concerns all descendant processes like interpreters and
	 * converters. (Or is it possible to run a console app without console?)
	 */

	fprintf(stderr,"%s: Not functioning on Windows.\n",argv[0]);
	return 255;
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


/**
 * This slave process manages a child process which performs the file
 * conversion. The child process is a /bin/sh running the command. That child
 * process gets its own process group for that we are able to terminate all
 * processes in that group.
 *
 * This process even listens to several signals for terminating itself and the
 * process group properly.
 *
 * Finally, the parent process should have created a pipe whose read end is
 * STDIN for this process. We never get any data through that pipe, but we
 * detect the closing of the write end. This way, this process automatically
 * performs the termination as soon as the parent process closes the write end -
 * and this even works when the parent process dies abnormally.
 */


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
