//------------------------------------------------------------------------------
// emStd1.cpp
//
// Copyright (C) 2004-2012,2014,2016,2018-2019 Oliver Hamann.
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
			"Some linked object is binary incompatible with emCore (=> try recompilation)."
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
static bool emLatin1System = false;


void emInitLocale()
{
	const char * p;

	setlocale(LC_ALL,"C");
	setlocale(LC_COLLATE,"");
	setlocale(LC_CTYPE,"");

	emUtf8System=false;
	emLatin1System=false;
#if defined(_WIN32)
	p=setlocale(LC_CTYPE,NULL);
	if (p) p=strrchr(p,'.');
	unsigned ccp=(p?atoi(p+1):0);
	unsigned acp=GetACP();
	if (ccp!=0 && acp!=65001 && ccp!=acp) {
		// Workaround for a problem seen with MinGW-w64 (probably
		// because it uses a very old msvcrt.dll), where functions like
		// mbrtowc are not using the active code page. Maybe the problem
		// only exists if the active code page is not equal to the
		// default code page of the active language - I don't know. In a
		// test, the fix failed for 65001 (UTF-8), but that case is
		// covered by our own conversion functions.
		char locale[64];
		sprintf(locale,".%u",acp);
		setlocale(LC_CTYPE,locale);
		setlocale(LC_COLLATE,locale);
	}
	if (acp==65001) emUtf8System=true;
	else if (acp==1252 || acp==28591) emLatin1System=true;
#elif defined(ANDROID)
	emUtf8System=true;
#else
	p=nl_langinfo(CODESET);
	if (strcmp(p,"UTF-8")==0) emUtf8System=true;
	else if (strcmp(p,"ISO-8859-1")==0) emLatin1System=true;
	else if (strcmp(p,"ANSI_X3.4-1968")==0) {
		// This is 7-Bit ASCII (LANG="POSIX" or "C").
		// Here we show 8-Bit characters as Latin1.
		emLatin1System=true;
	}
#endif

	// ??? setlocale(LC_MESSAGES,"");
	// ??? setlocale(LC_MONETARY,"");
	// ??? setlocale(LC_TIME,"");
	// never: setlocale(LC_NUMERIC,"");
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


int emEncodeChar(char * str, int ucs4, emMBState * state)
{
	int r,r2,i,i1,i2;
	wchar_t w,w2;

	if (ucs4<0x00000080) {
		*str=(char)ucs4;
		return 1;
	}

	if (emUtf8System) {
		return emEncodeUtf8Char(str,ucs4);
	}

	if (emLatin1System) {
		if (ucs4>0xFF) {
			struct Latin1ExtraMapElement {
				int ucs4, latin1;
			};
			static const Latin1ExtraMapElement latin1ExtraMap[]={
				{0x0152,0x8C},{0x0153,0x9C},{0x0160,0x8A},{0x0161,0x9A},{0x0164,0x8D},
				{0x0165,0x9D},{0x0178,0x9F},{0x0179,0x8F},{0x017D,0x8E},{0x017E,0x9E},
				{0x0192,0x83},{0x02C6,0x88},{0x02DC,0x98},{0x2013,0x96},{0x2014,0x97},
				{0x201A,0x82},{0x201E,0x84},{0x2020,0x86},{0x2021,0x87},{0x2022,0x95},
				{0x2026,0x85},{0x2030,0x89},{0x2032,0x92},{0x2033,0x94},{0x2035,0x91},
				{0x2036,0x93},{0x2039,0x8B},{0x203A,0x9B},{0x20AC,0x80},{0x2122,0x99}
			};
			i1=0;
			i2=sizeof(latin1ExtraMap)/sizeof(Latin1ExtraMapElement);
			for (;;) {
				i=(i1+i2)>>1;
				if (latin1ExtraMap[i].ucs4>ucs4) {
					i2=i;
				}
				else if (latin1ExtraMap[i].ucs4<ucs4) {
					i1=i+1;
				}
				else {
					*str=(char)latin1ExtraMap[i].latin1;
					break;
				}
				if (i1>=i2) {
					*str='?';
					break;
				}
			}
		}
		else {
			*str=(char)ucs4;
		}
		return 1;
	}

	// Hint on Windows: See my hints on mbrtowc in emDecodeChar. wcrtomb has
	// similar problems. The surrogate conversion below is probably useless.

	if (sizeof(wchar_t)<4 && ucs4>0xFFFF) {
		// Assuming UTF-16
		w=(wchar_t)((((ucs4-0x10000)>>10)&0x3FF)|0xD800);
		w2=(wchar_t)(((ucs4-0x10000)&0x3FF)|0xDC00);
	}
	else {
		w=(wchar_t)ucs4;
		w2=0;
	}
	r=(int)wcrtomb(str,w,state?&state->State:NULL);
	if (r<1) {
		*str='?';
		return 1;
	}
	if (w2) {
		r2=(int)wcrtomb(str+r,w2,state?&state->State:NULL);
		if (r2>0) {
			r+=r2;
		}
	}
	return r;
}


int emDecodeChar(int * pUcs4, const char * str, int strLen, emMBState * state)
{
	int c,c2,r,r2;
	wchar_t w;

	if (strLen<=0) {
		*pUcs4=0;
		return 0;
	}

	c=(unsigned char)str[0];
	if (c<128) {
		if (!c) {
			*pUcs4=0;
			return 0;
		}
		*pUcs4=c;
		return 1;
	}

	if (emUtf8System) {
		r=emDecodeUtf8Char(pUcs4,str,strLen);
		if (r<0) {
			*pUcs4=c;
			r=1;
		}
		return r;
	}

	if (emLatin1System) {
		if (c>=0x80 && c<=0x9F) {
			static const int latin1ExtraTab[32]={
				0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
				0x02C6,0x2030,0x0160,0x2039,0x0152,0x0164,0x017D,0x0179,
				0x0090,0x2035,0x2032,0x2036,0x2033,0x2022,0x2013,0x2014,
				0x02DC,0x2122,0x0161,0x203A,0x0153,0x0165,0x017E,0x0178
			};
			c=latin1ExtraTab[c-0x80];
		}
		*pUcs4=c;
		return 1;
	}

	// Hint on Windows: mbrtowc is based on MultiByteToWideChar (in the
	// implementations I inspected: SDK 10 and MinGW-w64, 2018). Thereby the
	// multibyte state is not used for shift states. It is only used to
	// restart within a multibyte char after that char has not been provided
	// completely - a feature we don't need. But decoding strings with shift
	// states may fail! The MinGW-w64 implementation even does not handle
	// UTF-8 (okay no problem, that case is covered by our own function more
	// above). Finally, UTF-16 surrogates are not generated. This means
	// unicode greater 16 bit is not supported, and the surrogates decoding
	// below is useless. Hopefully future versions of mbrtowc are better. I
	// also tried mbrtoc32 and c32rtomb, but on Windows, these seem to use
	// UTF-8 always, instead of active code page.

	r=(int)mbrtowc(&w,str,strLen,state?&state->State:NULL);
	if (r<=0) {
		if (r==0) {
			*pUcs4=0;
			return 0;
		}
		*pUcs4=c;
		return 1;
	}
	c=(int)w;
	if (sizeof(wchar_t)<4) {
		// Assuming UTF-16
		c&=0xFFFF;
	 	if (c>=0xD800 && c<=0xDBFF && r<strLen) {
			r2=(int)mbrtowc(&w,str+r,strLen-r,state?&state->State:NULL);
			if (r2>0) {
				c2=(emUInt16)w;
				if (c2>=0xDC00 && c2<=0xDFFF) {
					r+=r2;
					c=0x10000+((c&0x03FF)<<10)+(c2&0x03FF);
				}
			}
		}
	}
	*pUcs4=c;
	return r;
}


int emGetDecodedCharCount(const char * str, int strLen)
{
	int i,c,d,n;

	emMBState mbState;
	for (i=0, d=0; i<strLen; i++) {
		c=(unsigned char)str[i];
		if (!c) break;
		if (c>=128) {
			n=emDecodeChar(&c,str+i,strLen-i,&mbState);
			n--;
			if (n>=0) {
				i+=n;
				d+=n;
			}
		}
	}
	return i-d;
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
		if (em_stat(logFilePath,&st)==0 && st.st_size>512*1024) {
			unlink(backupFilePath);
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
	__android_log_write(ANDROID_LOG_INFO,"eaglemode",buf);
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
				catch (const emException &) {
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
//================================ emException =================================
//==============================================================================

emException::emException(const char * format, ...)
{
	va_list args;

	va_start(args,format);
	vsnprintf(Text,sizeof(Text),format,args);
	Text[sizeof(Text)-1]=0;
	va_end(args);
}


emException::emException(const emException & exception)
{
	strcpy(Text,exception.Text);
}


emException::~emException()
{
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
