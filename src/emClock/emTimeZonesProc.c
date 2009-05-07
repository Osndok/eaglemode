/*------------------------------------------------------------------------------
// emTimeZonesProc.c
//
// Copyright (C) 2008-2009 Oliver Hamann.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


/* This is a child process because setenv("TZ",name) is not thread-safe. */


int main(int argc, char * argv[])
{
	char * rBuf, * wBuf, * name;
	int rBufSize, wBufSize, rBufFill, wBufFill, i, j;
	time_t currentTime;
	struct tm tm;
	struct tm * ptm;

	rBufSize=16384;
	wBufSize=16384;
	rBuf=(char*)malloc(rBufSize);
	wBuf=(char*)malloc(wBufSize);
	rBufFill=0;
	wBufFill=0;
	for (;;) {
		if (rBufFill<rBufSize) {
			i=(int)read(STDIN_FILENO,rBuf,rBufSize-rBufFill);
			if (i<=0) break;
			rBufFill+=i;
		}
		if (rBufFill>=rBufSize) {
			rBufSize*=2;
			rBuf=(char*)realloc(rBuf,rBufSize);
		}
		currentTime=time(NULL);
		for (i=0, j=0; i<rBufFill; i++) {
			if (rBuf[i]!=0x0a && rBuf[i]!=0x0d) continue;
			if (i==j) { j++; continue; }
			rBuf[i]=0;
			name=rBuf+j;
			j=i+1;
			if (
				setenv("TZ",name,1)!=0 ||
				(ptm=localtime(&currentTime))==NULL
			) {
				memset(&tm,0,sizeof(tm));
				ptm=&tm;
			}
			if (wBufSize-wBufFill<100) {
				wBufSize*=2;
				wBuf=(char*)realloc(wBuf,wBufSize);
			}
			wBufFill+=sprintf(
				wBuf+wBufFill,
				"%d-%d-%d %d %d:%d:%d\n",
				(int)ptm->tm_year+1900,
				(int)ptm->tm_mon+1,
				(int)ptm->tm_mday,
				(int)ptm->tm_wday,
				(int)ptm->tm_hour,
				(int)ptm->tm_min,
				(int)ptm->tm_sec
			);
		}
		rBufFill-=i;
		if (rBufFill>0) memmove(rBuf,rBuf+i,rBufFill);
		if (wBufFill>0) {
			i=(int)write(STDOUT_FILENO,wBuf,wBufFill);
			if (i<=0) break;
			wBufFill-=i;
			if (wBufFill>0) memmove(wBuf,wBuf+i,wBufFill);
		}
	}
	free(rBuf);
	free(wBuf);
	return 0;
}
