//------------------------------------------------------------------------------
// emStd1.cpp
//
// Copyright (C) 2004-2011 Oliver Hamann.
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

#if defined(_WIN32)
#	include <windows.h>
#elif defined(ANDROID)
#	include <unistd.h>
#	include <android/log.h>
#else
#	include <unistd.h>
#	include <langinfo.h>
#endif
#include <locale.h>
#include <emCore/emStd1.h>
#include <emCore/emThread.h>
#include <emCore/emProcess.h>
#include <emCore/emInstallInfo.h>


//==============================================================================
//================================== Version ===================================
//==============================================================================

const char * emGetVersion()
{
#	define emGV_STR_OF_ARG(x) #x
#	define emGV_STR_OF(x) emGV_STR_OF_ARG(x)
	return
		emGV_STR_OF(EM_MAJOR_VERSION) "."
		emGV_STR_OF(EM_MINOR_VERSION) "."
		emGV_STR_OF(EM_MICRO_VERSION)
		EM_VERSION_POSTFIX
	;
}


emCompatibilityCheckerClass::emCompatibilityCheckerClass(
	int maj, int min, int mic, const char * postfix
)
{
	if (maj!=EM_MAJOR_VERSION || min!=EM_MINOR_VERSION) {
		emFatalError(
			"Some linked object is binary incompatible with emCore (=> try recompilation).\n"
		);
	}
}


//==============================================================================
//============ Some adaptations to compilers and operating systems =============
//==============================================================================

#if defined(_WIN32)

	static emThreadMiniMutex em_time_func_mutex;


	char * em_asctime_r(const struct tm * ptm, char * buf)
	{
		char * p;

		em_time_func_mutex.Lock();
		p=asctime(ptm);
		if (p) {
			strcpy(buf,p);
			p=buf;
		}
		em_time_func_mutex.Unlock();
		return p;
	}


	char * em_ctime_r(const time_t * ptime, char * buf)
	{
		char * p;

		em_time_func_mutex.Lock();
		p=ctime(ptime);
		if (p) {
			strcpy(buf,p);
			p=buf;
		}
		em_time_func_mutex.Unlock();
		return p;
	}


	struct tm * em_gmtime_r(const time_t * ptime, struct tm * buf)
	{
		struct tm * p;

		em_time_func_mutex.Lock();
		p=gmtime(ptime);
		if (p) {
			memcpy(buf,p,sizeof(struct tm));
			p=buf;
		}
		em_time_func_mutex.Unlock();
		return p;
	}


	struct tm * em_localtime_r(const time_t * ptime, struct tm * buf)
	{
		struct tm * p;

		em_time_func_mutex.Lock();
		p=localtime(ptime);
		if (p) {
			memcpy(buf,p,sizeof(struct tm));
			p=buf;
		}
		em_time_func_mutex.Unlock();
		return p;
	}

#endif


//==============================================================================
//============================= Integer data types =============================
//==============================================================================

int emStrToInt64(const char * str, int strLen, emInt64 * pVal)
{
	emInt64 val;
	int l;

	if (strLen>=1 && str[0]=='-') {
		l=emStrToUInt64(str+1,strLen-1,(emUInt64*)&val);
		val=-val;
		if (l>0) {
			if (val>=0) { val=EM_INT64_MIN; l=0; }
			else l++;
		}
	}
	else {
		l=emStrToUInt64(str,strLen,(emUInt64*)&val);
		if (l>0 && val<0) { val=EM_INT64_MAX; l=0; }
	}
	*pVal=val;
	return l;
}


int emStrToUInt64(const char * str, int strLen, emUInt64 * pVal)
{
	emUInt64 v,w;
	int i;

	i=0;
	if (i<strLen && str[i]=='+') i++;
	if (i>=strLen || str[i]<'0' || str[i]>'9') {
		*pVal=0;
		return 0;
	}
	for (w=0;;) {
		v=w+(str[i]-'0');
		if (v<w) { v=EM_UINT64_MAX; i=0; break; }
		i++;
		if (i>=strLen || str[i]<'0' || str[i]>'9') break;
		w=v*10;
		if (w<v) { v=EM_UINT64_MAX; i=0; break; }
	}
	*pVal=v;
	return i;
}


int emInt64ToStr(char * buf, int bufLen, emInt64 val)
{
	int l;

	if (val<0) {
		if (bufLen<1) return 0;
		buf[0]='-';
		l=emUInt64ToStr(buf+1,bufLen-1,(emUInt64)(-val));
		if (l>0) l++;
	}
	else {
		l=emUInt64ToStr(buf,bufLen,(emUInt64)val);
	}
	return l;
}


int emUInt64ToStr(char * buf, int bufLen, emUInt64 val)
{
	char tmp[32];
	int l;

	l=0;
	do {
		tmp[31-l]=(char)(((int)(val%10))+'0');
		val/=10;
		l++;
	} while(val);
	if (l>bufLen) return 0;
	memcpy(buf,tmp+32-l,l);
	return l;
}


//==============================================================================
//========================== Locale and UTF-8 support ==========================
//==============================================================================

static bool emUtf8System = false;


void emInitLocale()
{
	const char * p;
	bool latin1;

	setlocale(LC_ALL,"C");
	setlocale(LC_COLLATE,"");
	setlocale(LC_CTYPE,"");

	emUtf8System=false;
	latin1=false;
#if defined(_WIN32)
	p=setlocale(LC_CTYPE,NULL);
	if (p) p=strrchr(p,'.');
	int cp=(p?atoi(p+1):0);
	if (cp==65001) emUtf8System=true;
	else if (cp==850 || cp==858 || cp==1252 || cp==28591) latin1=true;
#elif defined(ANDROID)
	emUtf8System=true;
#else
	p=nl_langinfo(CODESET);
	if (strcmp(p,"UTF-8")==0) emUtf8System=true;
	else if (strcmp(p,"ISO-8859-1")==0) latin1=true;
#endif

	if (emUtf8System || latin1) {
		// ??? setlocale(LC_MESSAGES,"");
		// ??? setlocale(LC_MONETARY,"");
		// ??? setlocale(LC_TIME,"");
		// never: setlocale(LC_NUMERIC,"");
	}
}


bool emIsUtf8System()
{
	return emUtf8System;
}


int emEncodeUtf8Char(char * utf8, int ucs4)
{
	if (ucs4<0x00000080) {
		utf8[0]=(char)ucs4;
		return 1;
	}
	if (ucs4<0x00000800) {
		utf8[0]=(char)(0xc0|(ucs4>>6));
		utf8[1]=(char)(0x80|(ucs4&0x3f));
		return 2;
	}
	if (ucs4<0x00010000) {
		utf8[0]=(char)(0xe0|(ucs4>>12));
		utf8[1]=(char)(0x80|((ucs4>>6)&0x3f));
		utf8[2]=(char)(0x80|(ucs4&0x3f));
		return 3;
	}
	if (ucs4<0x00200000) {
		utf8[0]=(char)(0xf0|(ucs4>>18));
		utf8[1]=(char)(0x80|((ucs4>>12)&0x3f));
		utf8[2]=(char)(0x80|((ucs4>>6)&0x3f));
		utf8[3]=(char)(0x80|(ucs4&0x3f));
		return 4;
	}
	if (ucs4<0x04000000) {
		utf8[0]=(char)(0xf8|(ucs4>>24));
		utf8[1]=(char)(0x80|((ucs4>>18)&0x3f));
		utf8[2]=(char)(0x80|((ucs4>>12)&0x3f));
		utf8[3]=(char)(0x80|((ucs4>>6)&0x3f));
		utf8[4]=(char)(0x80|(ucs4&0x3f));
		return 5;
	}
	utf8[0]=(char)(0xfc|(ucs4>>30));
	utf8[1]=(char)(0x80|((ucs4>>24)&0x3f));
	utf8[2]=(char)(0x80|((ucs4>>18)&0x3f));
	utf8[3]=(char)(0x80|((ucs4>>12)&0x3f));
	utf8[4]=(char)(0x80|((ucs4>>6)&0x3f));
	utf8[5]=(char)(0x80|(ucs4&0x3f));
	return 6;
}


int emDecodeUtf8Char(int * pUcs4, const char * utf8, int utf8Len)
{
	int a,b,c;

	if (utf8Len<=0 || (a=(unsigned char)utf8[0])==0) {
		*pUcs4=0;
		return 0;
	}
	if (a<0x80) {
		*pUcs4=a;
		return 1;
	}

	if (utf8Len<2) goto Err;
	b=(unsigned char)utf8[1];
	if ((b&0xc0)!=0x80) goto Err;
	c=b&0x3f;
	if ((a&0xe0)==0xc0) {
		c|=(a&0x1f)<<6;
		if (c<0x00000080) goto Err;
		*pUcs4=c;
		return 2;
	}

	if (utf8Len<3) goto Err;
	b=(unsigned char)utf8[2];
	if ((b&0xc0)!=0x80) goto Err;
	c=(c<<6)|(b&0x3f);
	if ((a&0xf0)==0xe0) {
		c|=(a&0x0f)<<12;
		if (c<0x00000800) goto Err;
		*pUcs4=c;
		return 3;
	}

	if (utf8Len<4) goto Err;
	b=(unsigned char)utf8[3];
	if ((b&0xc0)!=0x80) goto Err;
	c=(c<<6)|(b&0x3f);
	if ((a&0xf8)==0xf0) {
		c|=(a&0x07)<<18;
		if (c<0x00010000) goto Err;
		*pUcs4=c;
		return 4;
	}

	if (utf8Len<5) goto Err;
	b=(unsigned char)utf8[4];
	if ((b&0xc0)!=0x80) goto Err;
	c=(c<<6)|(b&0x3f);
	if ((a&0xfc)==0xf8) {
		c|=(a&0x03)<<24;
		if (c<0x00200000) goto Err;
		*pUcs4=c;
		return 5;
	}

	if (utf8Len<6) goto Err;
	b=(unsigned char)utf8[5];
	if ((b&0xc0)!=0x80) goto Err;
	c=(c<<6)|(b&0x3f);
	if ((a&0xfe)==0xfc) {
		c|=(a&0x01)<<30;
		if (c<0x04000000) goto Err;
		*pUcs4=c;
		return 6;
	}
Err:
	*pUcs4=a;
	return -1;
}


int emEncodeChar(char * str, int ucs4)
{
	if (ucs4<0x00000080) {
		*str=(char)ucs4;
		return 1;
	}
	else if (emUtf8System) {
		return emEncodeUtf8Char(str,ucs4);
	}
	else {
		if ((unsigned)ucs4>255) ucs4='?';
		*str=(char)ucs4;
		return 1;
	}
}


int emDecodeChar(int * pUcs4, const char * str, int strLen)
{
	int r;

	if (!*str || strLen<=0)  {
		*pUcs4=0;
		return 0;
	}
	else if (((*str)&0x80)!=0 && emUtf8System) {
		r=emDecodeUtf8Char(pUcs4,str,strLen);
		if (r<0) {
			*pUcs4=(unsigned char)*str;
			r=1;
		}
		return r;
	}
	else {
		*pUcs4=(unsigned char)*str;
		return 1;
	}
}


int emGetDecodedCharCount(const char * str, int strLen)
{
	int i,cnt,n,ucs4;

	for (i=0, cnt=0; i<strLen && str[i]; cnt++) {
		if ((str[i]&0x80)!=0 && emUtf8System) {
			n=emDecodeUtf8Char(&ucs4,str+i,strLen-i);
			if (n<=0) {
				if (n<0) n=1;
				else break;
			}
			i+=n;
		}
		else {
			i++;
		}
	}
	return cnt;
}


//==============================================================================
//========================= Logs, Warnings and Errors ==========================
//==============================================================================

static void emRawLog(const char * pre, const char * format, va_list args)
{
#if defined(_WIN32)
	static const char * const logFileName   ="emCoreBasedAppLog.log";
	static const char * const backupFileName="emCoreBasedAppLog-old.log";
	static emThreadMiniMutex logFileMutex;
	char logFilePath[_MAX_PATH+1], backupFilePath[_MAX_PATH+1];
	struct em_stat st;
	char * buf;
	FILE * f;
	DWORD d;
	int r,bufSize,e;

	bufSize=100000;
	buf=(char*)malloc(bufSize);
	r=0;
	if (pre) {
		strcpy(buf,pre);
		r=strlen(buf);
	}
	vsnprintf(buf+r,bufSize-r,format,args);
	buf[bufSize-2]=0;
	strcat(buf,"\n");
	r=fputs(buf,stderr);
	if (r>=0) r=fflush(stderr);
	if (r<0) {
		// stderr failed and it seems to be a window mode application.
		// Therefore write to a log file (though, window mode
		// applications can have stderr, e.g. through WshShell.Exec)
		d=GetTempPath(sizeof(logFilePath)-1,logFilePath);
		if (!d) {
			emFatalError("emLog: GetTempPath failed.");
		}
		d+=emMax(strlen(logFileName),strlen(backupFileName));
		if (d>sizeof(logFilePath)-1) {
			emFatalError("emLog: Temp path too long.");
		}
		strcpy(backupFilePath,logFilePath);
		strcat(logFilePath,logFileName);
		strcat(backupFilePath,backupFileName);
		logFileMutex.Lock();
		if (em_stat(logFilePath,&st)==0 && st.st_size>64000) {
			remove(backupFilePath);
			rename(logFilePath,backupFilePath);
		}
		f=fopen(logFilePath,"a");
		if (f) {
			fputs(buf,f);
			fclose(f);
			logFileMutex.Unlock();
		}
		else {
			e=errno;
			logFileMutex.Unlock();
			emFatalError(
				"emLog: Failed to open \"%s\": %s",
				logFilePath,
				strerror(e)
			);
		}
	}
	free(buf);
#elif defined(ANDROID)
	char * buf;
	int r,bufSize;

	bufSize=10000;
	buf=(char*)malloc(bufSize);
	r=0;
	if (pre) {
		strcpy(buf,pre);
		r=strlen(buf);
	}
	vsnprintf(buf+r,bufSize-r,format,args);
	buf[bufSize-1]=0;
	__android_log_print(ANDROID_LOG_INFO,"eaglemode",buf);
	free(buf);
#else
	if (pre) fputs(pre,stderr);
	vfprintf(stderr,format,args);
	fputs("\n",stderr);
#endif
}


void emLog(const char * format, ...)
{
	va_list args;

	va_start(args,format);
	emRawLog(NULL,format,args);
	va_end(args);
}


static bool emDevLogEnabled=false;


void emEnableDLog(bool devLogEnabled)
{
	emDevLogEnabled=devLogEnabled;
}


bool emIsDLogEnabled()
{
	return emDevLogEnabled;
}


void emDLog(const char * format, ...)
{
	va_list args;

	if (emDevLogEnabled) {
		va_start(args,format);
		emRawLog(NULL,format,args);
		va_end(args);
	}
}


void emWarning(const char * format, ...)
{
	va_list args;

	va_start(args,format);
	emRawLog("WARNING: ",format,args);
	va_end(args);
}


static bool emFatalErrorGraphical=false;


void emFatalError(const char * format, ...)
{
#if defined(ANDROID)
	va_list args;

	va_start(args,format);
	emRawLog("FATAL ERROR: ",format,args);
	va_end(args);
#else
	va_list args;
	char tmp[512];

	fprintf(stderr,"FATAL ERROR: ");
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fprintf(stderr,"\n");
	if (emFatalErrorGraphical) {
		va_start(args,format);
		vsnprintf(tmp,sizeof(tmp),format,args);
		tmp[sizeof(tmp)-1]=0;
		va_end(args);
#		if defined(_WIN32)
			//??? Maybe we should call FatalAppExit instead.
			MessageBox(NULL,tmp,"Fatal Error",MB_OK|MB_ICONERROR);
#		else
			if (
				getenv("EM_FATAL_ERROR_LOCK")==NULL &&
				putenv((char*)"EM_FATAL_ERROR_LOCK=1")==0
			) {
				emArray<emString> cmd;
				cmd+=emGetInstallPath(EM_IDT_BIN,"emShowStdDlg","emShowStdDlg");
				cmd+="message";
				cmd+="Fatal Error";
				cmd+=tmp;
				try {
					emProcess::TryStartUnmanaged(cmd);
				}
				catch (emString) {
				}
			}
#		endif
	}
#endif
	_exit(255);
}


void emSetFatalErrorGraphical(bool graphical)
{
	emFatalErrorGraphical=graphical;
}


//==============================================================================
//================================ emAlignment =================================
//==============================================================================

const char * emAlignmentToString(emAlignment alignment)
{
	static const char * tab[16]={
		"center",
		"top",
		"bottom",
		"top-bottom",
		"left",
		"top-left",
		"bottom-left",
		"top-bottom-left",
		"right",
		"top-right",
		"bottom-right",
		"top-bottom-right",
		"left-right",
		"top-left-right",
		"bottom-left-right",
		"top-bottom-left-right"
	};
	return tab[alignment&15];
}


emAlignment emStringToAlignment(const char * str)
{
	emAlignment a;

	a=0;
	if (!str) return a;
	while (*str) {
		if ((*str<'a' || *str>'z') && (*str<'A' || *str>'Z')) {
			str++;
		}
		else if (strncasecmp(str,"top",3)==0) {
			str+=3;
			a|=EM_ALIGN_TOP;
		}
		else if (strncasecmp(str,"bottom",6)==0) {
			str+=6;
			a|=EM_ALIGN_BOTTOM;
		}
		else if (strncasecmp(str,"left",4)==0) {
			str+=4;
			a|=EM_ALIGN_LEFT;
		}
		else if (strncasecmp(str,"right",5)==0) {
			str+=5;
			a|=EM_ALIGN_RIGHT;
		}
		else if (strncasecmp(str,"center",6)==0) {
			str+=6;
		}
		else {
			break;
		}
	}
	return a;
}
