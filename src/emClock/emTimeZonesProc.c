/*------------------------------------------------------------------------------
// emTimeZonesProc.c
//
// Copyright (C) 2008-2009,2017-2018 Oliver Hamann.
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
#include <errno.h>
#if defined(_WIN32)
#	include <fcntl.h>
#	include <io.h>
#	include <windows.h>
#else
#	include <time.h>
#	include <unistd.h>
#	include <sys/stat.h>
#endif


/* This is a child process because setenv("TZ",name) is not thread-safe. */


#if defined(_WIN32)
struct TzMapEntry {
	char * tz;
	char * winTz;
};
static struct TzMapEntry * tzMap=NULL;
static int tzMapElems=0;
#endif


#if defined(_WIN32)
static int compareTzMapEntries(const void * a, const void * b)
{
	return strcmp(
		((const struct TzMapEntry*)a)->tz,
		((const struct TzMapEntry*)b)->tz
	);
}
#endif


#if defined(_WIN32)
static int loadZone2WinTab(
	const char * zoneInfoDir, char * errBuf, int errBufSize
)
{
	char * path;
	char * p, * tz, * winTz;
	char buf[512];
	FILE * f;
	int tzMapSize;

	path=malloc(strlen(zoneInfoDir)+256);
	sprintf(path,"%s\\zone2win.tab",zoneInfoDir);
	f=fopen(path,"r");
	if (!f) {
		snprintf(errBuf,errBufSize,"Failed to open %s: %s",path,strerror(errno));
		errBuf[errBufSize-1]=0;
		free(path);
		return 0;
	}
	free(path);

	tzMapSize=256;
	tzMapElems=0;
	tzMap=malloc(sizeof(struct TzMapEntry)*tzMapSize);

	while (fgets(buf,sizeof(buf),f)) {
		tz=NULL;
		winTz=NULL;

		p=strchr(buf,'#');
		if (!p) p=buf+strlen(buf);
		while (p>buf && (unsigned char)p[-1]<=32) p--;
		*p=0;
		p=buf;
		while (*p && (unsigned char)*p<=32) p++;
		if (*p) {
			tz=p;
			do { p++; } while ((unsigned char)*p>32);
			if (*p) {
				*p=0;
				do { p++; } while (*p && (unsigned char)*p<=32);
				if (*p) winTz=p;
			}
		}

		if (tz && winTz) {
			if (tzMapElems>=tzMapSize) {
				tzMapSize+=256;
				tzMap=realloc(tzMap,sizeof(struct TzMapEntry)*tzMapSize);
			}
			tzMap[tzMapElems].tz=strdup(tz);
			tzMap[tzMapElems].winTz=strdup(winTz);
			tzMapElems++;
		}
	}

	fclose(f);

	qsort(tzMap,tzMapElems,sizeof(struct TzMapEntry),compareTzMapEntries);

	return 1;
}
#endif


#if defined(_WIN32)
static const char * getWinTz(
	const char * zoneInfoDir, const char * tz, char * errBuf, int errBufSize
)
{
	int i,j,k,l;

	if (!tzMap) {
		if (!loadZone2WinTab(zoneInfoDir,errBuf,errBufSize)) {
			return 0;
		}
	}

	i=0;
	j=tzMapElems;
	while (i<j) {
		k=(i+j)/2;
		l=strcmp(tzMap[k].tz,tz);
		if (l<0) i=k+1;
		else if (l>0) j=k;
		else return tzMap[k].winTz;
	}

	snprintf(errBuf,errBufSize,"Windows time zone not configured");
	errBuf[errBufSize-1]=0;
	return NULL;
}
#endif


#if defined(_WIN32)
static int getWinTzInfo(
	const char * winTz, TIME_ZONE_INFORMATION * info,
	char * errBuf, int errBufSize
)
{
	struct Tzi {
		LONG Bias;
		LONG StandardBias;
		LONG DaylightBias;
		SYSTEMTIME StandardDate;
		SYSTEMTIME DaylightDate;
	};

	char * key;
	HKEY hk;
	struct Tzi tzi;
	DWORD d,tziSize;

	key=malloc(strlen(winTz)+256);
	sprintf(key,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\%s",winTz);

	d=RegOpenKeyEx(HKEY_LOCAL_MACHINE,key,0,KEY_QUERY_VALUE,&hk);
	if (d != ERROR_SUCCESS) {
		snprintf(errBuf,errBufSize,"RegOpenKeyEx failed for \"%s\": 0x%lX",winTz,(long)d);
		errBuf[errBufSize-1]=0;
		free(key);
		return 0;
	}

	tziSize=sizeof(tzi);
	d=RegQueryValueEx(hk,"TZI",NULL,NULL,(BYTE*)&tzi,&tziSize);
	if (d!=ERROR_SUCCESS) {
		snprintf(errBuf,errBufSize,"RegQueryValueEx failed for TZI in \"%s\": 0x%lX",winTz,(long)d);
		errBuf[errBufSize-1]=0;
		RegCloseKey(hk);
		free(key);
		return 0;
	}
	if (tziSize!=sizeof(tzi)) {
		snprintf(errBuf,errBufSize,"Reg TZI value too short for %s",winTz);
		errBuf[errBufSize-1]=0;
		RegCloseKey(hk);
		free(key);
		return 0;
	}

	memset(info,0,sizeof(TIME_ZONE_INFORMATION));
	info->Bias=tzi.Bias;
	info->DaylightBias=tzi.DaylightBias;
	info->DaylightDate=tzi.DaylightDate;
	info->StandardBias=tzi.StandardBias;
	info->StandardDate=tzi.StandardDate;

	RegCloseKey(hk);
	free(key);

	return 1;
}
#endif


#if !defined(_WIN32)
static int checkTzFile(const char * zoneInfoDir, const char * tz)
{
	char * path;
	struct stat st;
	int sr;

	path=malloc(strlen(zoneInfoDir)+strlen(tz)+2);
	strcpy(path,zoneInfoDir);
	strcat(path,"/");
	strcat(path,tz);
	sr=stat(path,&st);
	free(path);
	return sr==0 && (st.st_mode&S_IFMT)==S_IFREG;
}
#endif


#if !defined(_WIN32)
static int checkTzFileCached(const char * zoneInfoDir, const char * tz)
{
	struct cacheEntry {
		char * tz;
		int result;
	};
	static struct cacheEntry * * cache = NULL;
	static int cacheElems = 0;
	static int cacheSize = 0;
	int i,j,k,l;

	i=0;
	j=cacheElems;
	while (i<j) {
		k=(i+j)/2;
		l=strcmp(cache[k]->tz,tz);
		if (l<0) i=k+1;
		else if (l>0) j=k;
		else return cache[k]->result;
	}
	if (cacheElems>=cacheSize) {
		cacheSize+=256;
		if (cache) cache=realloc(cache,sizeof(struct cacheEntry*)*cacheSize);
		else cache=malloc(sizeof(struct cacheEntry*)*cacheSize);
	}
	if (i<cacheElems) {
		memmove(cache+i+1,cache+i,sizeof(struct cacheEntry*)*(cacheElems-i));
	}
	cacheElems++;
	cache[i]=malloc(sizeof(struct cacheEntry));
	cache[i]->tz=strdup(tz);
	cache[i]->result=checkTzFile(zoneInfoDir,tz);
	return cache[i]->result;
}
#endif


#if defined(_WIN32)
static SYSTEMTIME tzBaseTime;
#else
static time_t tzBaseTime;
#endif


void setTzBaseTimeNow()
{
#if defined(_WIN32)
	GetSystemTime(&tzBaseTime);
#else
	tzBaseTime=time(NULL);
#endif
}


void setTzBaseTime(int year, int month, int day, int hour, int minute, int second)
{
#if defined(_WIN32)
	memset(&tzBaseTime,0,sizeof(tzBaseTime));
	tzBaseTime.wYear=(WORD)year;
	tzBaseTime.wMonth=(WORD)month;
	tzBaseTime.wDay=(WORD)day;
	tzBaseTime.wHour=(WORD)hour;
	tzBaseTime.wMinute=(WORD)minute;
	tzBaseTime.wSecond=(WORD)second;
#else
	struct tm tm;

	memset(&tm,0,sizeof(struct tm));
	tm.tm_year=year-1900;
	tm.tm_mon=month-1;
	tm.tm_mday=day;
	tm.tm_hour=hour;
	tm.tm_min=minute;
	tm.tm_sec=second;

	if (setenv("TZ","",1)!=0) {
		fprintf(stderr,"setenv failed: %s\n",strerror(errno));
		exit(255);
	}

	tzBaseTime=mktime(&tm);
#endif
}


int getTzTime(
	const char * zoneInfoDir, const char * tz,
	int * pYear, int * pMonth, int * pDay, int * pWDay,
	int * pHour, int * pMinute, int * pSecond,
	char * errBuf, int errBufSize
)
{
#if defined(_WIN32)
	TIME_ZONE_INFORMATION tzInfo;
	const char * winTz;
	SYSTEMTIME st;

	winTz=getWinTz(zoneInfoDir,tz,errBuf,errBufSize);
	if (!winTz) {
		return 0;
	}

	if (!getWinTzInfo(winTz,&tzInfo,errBuf,errBufSize)) {
		return 0;
	}

	if (!SystemTimeToTzSpecificLocalTime(&tzInfo,&tzBaseTime,&st)) {
		snprintf(errBuf,errBufSize,"Time conversion failure (0x%lX)",(long)GetLastError());
		errBuf[errBufSize-1]=0;
		return 0;
	}

	*pYear  =st.wYear;
	*pMonth =st.wMonth;
	*pDay   =st.wDay;
	*pWDay  =st.wDayOfWeek;
	*pHour  =st.wHour;
	*pMinute=st.wMinute;
	*pSecond=st.wSecond;
	return 1;
#else
	struct tm * ptm;

	if (!checkTzFileCached(zoneInfoDir,tz)) {
		snprintf(errBuf,errBufSize,"Could not find %s/%s",zoneInfoDir,tz);
		errBuf[errBufSize-1]=0;
		return 0;
	}
	if (setenv("TZ",tz,1)!=0) {
		snprintf(errBuf,errBufSize,"setenv failed: %s",strerror(errno));
		errBuf[errBufSize-1]=0;
		return 0;
	}
	ptm=localtime(&tzBaseTime);
	if (!ptm) {
		snprintf(errBuf,errBufSize,"Time conversion failure");
		errBuf[errBufSize-1]=0;
		return 0;
	}
	*pYear  =ptm->tm_year+1900;
	*pMonth =ptm->tm_mon+1;
	*pDay   =ptm->tm_mday;
	*pWDay  =ptm->tm_wday;
	*pHour  =ptm->tm_hour;
	*pMinute=ptm->tm_min;
	*pSecond=ptm->tm_sec;
	return 1;
#endif
}


void handleRequest(
	const char * zoneInfoDir, const char * request,
	char * replyBuf, int replyBufSize
)
{
	int year, month, day, wDay, hour, minute, second, n;
	const char * p;
	char errBuf[256];

	if (strncmp(request,"test#",5)==0 && (p=strchr(request+5,'#'))!=NULL) {
		n=sscanf(request+5,"%d-%d-%d %d:%d:%d",&year,&month,&day,&hour,&minute,&second);
		if (n!=6) {
			snprintf(replyBuf,replyBufSize,"ERROR: Failed to parse test command\n");
			replyBuf[replyBufSize-1]=0;
			return;
		}
		setTzBaseTime(year,month,day,hour,minute,second);
		n=snprintf(replyBuf,replyBufSize,"%s#",request+5);
		if (n>replyBufSize) n=replyBufSize;
		replyBuf+=n;
		replyBufSize-=n;
		request=p+1;
	}
	else {
		setTzBaseTimeNow();
	}

	if (
		getTzTime(
			zoneInfoDir,request,
			&year,&month,&day,&wDay,&hour,&minute,&second,
			errBuf,sizeof(errBuf)
		)
	) {
		snprintf(
			replyBuf,replyBufSize,
			"%d-%d-%d %d %d:%d:%d\n",
			year,month,day,wDay,hour,minute,second
		);
		replyBuf[replyBufSize-1]=0;
	}
	else {
		snprintf(replyBuf,replyBufSize,"ERROR: %s\n",errBuf);
		replyBuf[replyBufSize-1]=0;
	}
}


int tzServe(int argc, char * argv[])
{
	static const int maxReplySize=256;
	const char * zoneInfoDir;
	char * rBuf, * wBuf, * request;
	int fdIn, fdOut, rBufSize, wBufSize, rBufFill, wBufFill, i, j;

	if (argc!=2) {
		fprintf(stderr,"%s: Illegal arguments\n",argv[0]);
		return 1;
	}
	zoneInfoDir=argv[1];

	fdIn=fileno(stdin);
	fdOut=fileno(stdout);
	rBufSize=65536;
	wBufSize=65536;
	rBuf=(char*)malloc(rBufSize);
	wBuf=(char*)malloc(wBufSize);
	rBufFill=0;
	wBufFill=0;
	for (;;) {
		if (rBufFill<rBufSize) {
			i=(int)read(fdIn,rBuf+rBufFill,rBufSize-rBufFill);
			if (i<=0) break;
			rBufFill+=i;
		}
		for (i=0, j=0; i<rBufFill; i++) {
			if (rBuf[i]!=0x0a && rBuf[i]!=0x0d) continue;
			if (i==j) { j++; continue; }
			rBuf[i]=0;
			request=rBuf+j;
			j=i+1;
			while (wBufSize-wBufFill<maxReplySize) {
				wBufSize*=2;
				wBuf=(char*)realloc(wBuf,wBufSize);
			}
			handleRequest(zoneInfoDir,request,wBuf+wBufFill,maxReplySize);
			wBufFill+=strlen(wBuf+wBufFill);
		}
		if (j==0 && rBufFill>=rBufSize) {
			rBufSize*=2;
			rBuf=(char*)realloc(rBuf,rBufSize);
			continue;
		}
		rBufFill-=j;
		if (rBufFill>0) memmove(rBuf,rBuf+j,rBufFill);
		if (wBufFill>0) {
			i=(int)write(fdOut,wBuf,wBufFill);
			if (i<=0) break;
			wBufFill-=i;
			if (wBufFill>0) memmove(wBuf,wBuf+i,wBufFill);
		}
	}
	free(rBuf);
	free(wBuf);
	return 0;
}


#ifdef _WIN32
static DWORD WINAPI tzServeThreadProc(LPVOID data)
{
	return tzServe(__argc,__argv);
}
#endif


int main(int argc, char * argv[])
{
#ifdef _WIN32
	HANDLE hdl;
	DWORD d;
	MSG msg;

	setmode(fileno(stdout),O_BINARY);
	setmode(fileno(stdin),O_BINARY);
	setbuf(stderr,NULL);

	hdl=CreateThread(NULL,0,tzServeThreadProc,NULL,0,&d);
	do {
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT) ExitProcess(143);
		}
		d=MsgWaitForMultipleObjects(1,&hdl,FALSE,INFINITE,QS_ALLPOSTMESSAGE);
	} while(d==WAIT_OBJECT_0+1);
	WaitForSingleObject(hdl,INFINITE);
	GetExitCodeThread(hdl,&d);
	return d;
#else
	return tzServe(argc,argv);
#endif
}
