//------------------------------------------------------------------------------
// emStd2.cpp
//
// Copyright (C) 2004-2012,2014-2020 Oliver Hamann.
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
#	include <ctype.h>
#	include <direct.h>
#	include <io.h>
#	include <windows.h>
#	ifndef F_OK
#		define F_OK 0
#	endif
#	ifndef W_OK
#		define W_OK 2
#	endif
#	ifndef R_OK
#		define R_OK 4
#	endif
#else
#	include <dirent.h>
#	include <fcntl.h>
#	include <pwd.h>
#	include <sched.h>
#	include <signal.h>
#	include <sys/times.h>
#	include <sys/wait.h>
#	include <unistd.h>
#	include <dlfcn.h>
#endif
#if defined(_MSC_VER)
#	include <isa_availability.h>
	extern "C" int __isa_available;
#endif
#include <emCore/emStd2.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emThread.h>


//==============================================================================
//================================ emException =================================
//==============================================================================

emException::emException(const char * format, ...)
{
	va_list args;

	va_start(args,format);
	Text=emString::VFormat(format,args);
	va_end(args);
}


emException::~emException()
{
}


//==============================================================================
//=========================== Host, user, process id ===========================
//==============================================================================

emString emGetHostName()
{
#if defined(_WIN32)
	char tmp[512];
	DWORD sz;

	sz=sizeof(tmp)-1;
	if (!GetComputerName(tmp,&sz)) {
		emFatalError(
			"emGetHostName: GetComputerName failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
	tmp[sizeof(tmp)-1]=0;
	return emString(tmp);
#else
	char tmp[512];

	if (gethostname(tmp,sizeof(tmp))!=0) {
		emFatalError(
			"emGetHostName: gethostname failed: %s",
			emGetErrorText(errno).Get()
		);
	}
	tmp[sizeof(tmp)-1]=0;
	return emString(tmp);
#endif
}


emString emGetUserName()
{
#if defined(_WIN32)
	char tmp[512];
	DWORD sz;

	sz=sizeof(tmp)-1;
	if (!GetUserName(tmp,&sz)) {
		emFatalError(
			"emGetUserName: GetUserName failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
	tmp[sizeof(tmp)-1]=0;
	return emString(tmp);
#elif defined(ANDROID)
	struct passwd * pw;
	int i;

	errno=0;
	pw=getpwuid(getuid());
	if (!pw || !pw->pw_name) {
		emFatalError(
			"emGetUserName: getpwuid failed: %s",
			emGetErrorText(errno).Get()
		);
	}
	return emString(pw->pw_name);
#else
	char tmp[1024];
	struct passwd pwbuf;
	struct passwd * pw;
	int i;

	errno=0;
	i=getpwuid_r(getuid(),&pwbuf,tmp,sizeof(tmp),&pw);
	if (i!=0 || !pw || !pw->pw_name) {
		emFatalError(
			"emGetUserName: getpwuid_r failed: %s",
			emGetErrorText(errno).Get()
		);
	}
	return emString(pw->pw_name);
#endif
}


int emGetProcessId()
{
#if defined(_WIN32)
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}


//==============================================================================
//================================ Error Texts =================================
//==============================================================================

#if !defined(_WIN32)
const char * emGetErrorText_strerror_r_helper(int res, const char * buf)
{
	return res==0 ? buf : NULL;
}
const char * emGetErrorText_strerror_r_helper(const char * res, const char * buf)
{
	return res;
}
#endif


emString emGetErrorText(int errorNumber)
{
#if defined(_WIN32)
	char tmp[512];

	if (!FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(DWORD)errorNumber,
		0,
		tmp,
		sizeof(tmp)-1,
		NULL
	)) {
		sprintf(tmp,"error #%d",errorNumber);
	}
	return emString(tmp);
#else
	char tmp[512];
	const char * p;

	memset(tmp,0,sizeof(tmp));
	p=emGetErrorText_strerror_r_helper(strerror_r(errorNumber,tmp,sizeof(tmp)),tmp);
	tmp[sizeof(tmp)-1]=0;
	if (!p) {
		sprintf(tmp,"error #%d",errorNumber);
		p=tmp;
	}
	return emString(p);
#endif
}


//==============================================================================
//================================ SIMD support ================================
//==============================================================================

bool emCanCpuDoAvx2()
{
	static const struct Detector {

		bool canCpuDoAvx2;

		Detector()
		{
#			if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#				if !defined(__clang__)
					__builtin_cpu_init();
#				endif
				canCpuDoAvx2=
					__builtin_cpu_supports("avx2") &&
					__builtin_cpu_supports("avx") &&
					__builtin_cpu_supports("sse4.1") &&
#					if !defined(__clang__)
						__builtin_cpu_supports("ssse3") &&
#					endif
					__builtin_cpu_supports("sse3") &&
					__builtin_cpu_supports("sse2") &&
					__builtin_cpu_supports("sse") &&
					__builtin_cpu_supports("mmx")
				;
#			elif defined(_MSC_VER)
				canCpuDoAvx2=(__isa_available >= __ISA_AVAILABLE_AVX2);
#			else
				canCpuDoAvx2=false;
#			endif
		}

	} detector;

	return detector.canCpuDoAvx2;
}


//==============================================================================
//==================================== Time ====================================
//==============================================================================

void emSleepMS(int millisecs)
{
#if defined(_WIN32)
	if (millisecs<0) millisecs=0;
	Sleep((DWORD)millisecs);
#else
	// usleep(0) is unlike sched_yield() on some systems.
	if (millisecs<=0) sched_yield();
	else if ((unsigned long)millisecs>ULONG_MAX/1000) sleep(millisecs/1000);
	else usleep(((unsigned long)millisecs)*1000);
#endif
}


emUInt64 emGetClockMS()
{
#if defined(_WIN32)
	static emThreadMiniMutex mutex;
	static emUInt64 ms64=0;
	static DWORD tcks=0;
	DWORD t;
	emUInt64 res;

	mutex.Lock();
	t=GetTickCount();
	ms64+=(DWORD)(t-tcks);
	tcks=t;
	res=ms64;
	mutex.Unlock();
	return res;
#else
	static emThreadMiniMutex mutex;
	static clock_t tcks=0;
	static unsigned long tps=0;
	static unsigned long rem=0;
	static emUInt64 ms64=0;
	emUInt64 t,res;
	clock_t d;
	tms tb;

	mutex.Lock();
	d=times(&tb)-tcks;
	if (d) {
		tcks+=d;
		if (tps<=0) {
			tps=(unsigned long)sysconf(_SC_CLK_TCK);
			if (((long)tps)<=0) {
				emFatalError("sysconf(_SC_CLK_TCK) failed");
			}
		}
		t=((emUInt64)d)*1000+rem;
		rem=(unsigned long)(t%tps);
		ms64+=(emUInt64)(t/tps);
	}
	res=ms64;
	mutex.Unlock();
	return res;
#endif
}


emUInt64 emGetCPUTSC()
{
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
	static const struct Detector {

		bool haveRDTSC;

		Detector()
		{
			emUInt32 d;
			asm volatile (
#				if defined(__x86_64__)
					"push %%rbx\n"
					"pushfq\n"
					"movl (%%rsp),%%eax\n"
					"movl %%eax,%%ebx\n"
					"xorl $0x00200000,%%eax\n"
					"movl %%eax,(%%rsp)\n"
					"popfq\n"
					"pushfq\n"
					"movl (%%rsp),%%eax\n"
					"xorl %%ebx,%%eax\n"
					"movl %%ebx,(%%rsp)\n"
					"popfq\n"
#				else
					"push %%ebx\n"
					"pushfl\n"
					"popl %%eax\n"
					"movl %%eax,%%ebx\n"
					"xorl $0x00200000,%%eax\n"
					"pushl %%eax\n"
					"popfl\n"
					"pushfl\n"
					"popl %%eax\n"
					"xorl %%ebx,%%eax\n"
					"pushl %%ebx\n"
					"popfl\n"
#				endif
				"testl $0x00200000,%%eax\n"
				"je 3f\n"
				"xorl %%eax,%%eax\n"
				"cpuid\n"
				"cmpl $0x756e6547,%%ebx\n"
				"jne 1f\n"
				"cmpl $0x49656e69,%%edx\n"
				"jne 1f\n"
				"cmpl $0x6c65746e,%%ecx\n"
				"je 2f\n"
				"1:\n"
				"cmpl $0x68747541,%%ebx\n"
				"jne 3f\n"
				"cmpl $0x69746e65,%%edx\n"
				"jne 3f\n"
				"cmpl $0x444d4163,%%ecx\n"
				"jne 3f\n"
				"2:\n"
				"movl $1,%%eax\n"
				"cpuid\n"
				"jmp 4f\n"
				"3:\n"
				"xorl %%edx,%%edx\n"
				"4:\n"
#				if defined(__x86_64__)
					"pop %%rbx\n"
#				else
					"pop %%ebx\n"
#				endif
				: "=d"(d) : : "eax","ecx","cc"
			);
			haveRDTSC=((d>>4)&1)!=0;
		}

	} detector;

	emUInt32 a,d;

	if (detector.haveRDTSC) {
		asm volatile (
			"rdtsc\n"
			: "=a"(a),"=d"(d) : : "cc"
		);
		return (((emUInt64)d)<<32)|a;
	}
#endif
	return 0;
}


//==============================================================================
//============================ Files & Directories =============================
//==============================================================================

emString emGetParentPath(const char * path)
{
	int i;

	i=strlen(path);
#if defined(_WIN32)
	while (i>0 && (path[i-1]=='\\' || path[i-1]=='/' || path[i-1]==':')) i--;
	while (i>0 && path[i-1]!='\\' && path[i-1]!='/' &&  path[i-1]!=':') i--;
	while (i>0 && (path[i-1]=='\\' || path[i-1]=='/' || path[i-1]==':')) i--;
	if (i<=1) {
		if (path[0]=='\\') {
			if (path[1]=='\\') i=2; else i=1;
		}
		else if (path[0] && path[1]==':') {
			if (path[2]=='\\' || path[2]=='/') i=3; else i=2;
		}
	}
#else
	while (i>0 && path[i-1]=='/') i--;
	while (i>0 && path[i-1]!='/') i--;
	while (i>0 && path[i-1]=='/') i--;
	if (i==0 && path[0]=='/') i++;
#endif
	return emString(path,i);
}


emString emGetChildPath(const char * path, const char * name)
{
	emString subPath;
	char * subPathPtr;
	int pathLen,nameLen;

	pathLen=strlen(path);
#if defined(_WIN32)
	if (pathLen>0 && (path[pathLen-1]=='\\' || path[pathLen-1]=='/')) pathLen--;
	if (name[0]=='\\' || name[0]=='/') name++;
	if (pathLen==0 && name[0]!=0 && name[1]==':') return emString(name);
#else
	if (pathLen>0 && path[pathLen-1]=='/') pathLen--;
	if (name[0]=='/') name++;
#endif
	nameLen=strlen(name);
	subPathPtr=subPath.SetLenGetWritable(pathLen+1+nameLen);
	memcpy(subPathPtr,path,pathLen);
#if defined(_WIN32)
	subPathPtr[pathLen]='\\';
#else
	subPathPtr[pathLen]='/';
#endif
	memcpy(subPathPtr+pathLen+1,name,nameLen);
	return subPath;
}


emString emGetAbsolutePath(const emString & path, const char * cwd)
{
	emString absPath;
	const char * p;
	int i, j;
	bool cool;

	p=path;
#if defined(_WIN32)
	if (p[0] && p[1]==':') {
		if (p[2]=='\\' || p[2]=='/') {
			absPath=path;
			cool=true;
			i=3;
		}
		else {
			if (cwd) absPath=cwd;
			else absPath=emGetCurrentDirectory();
			if (tolower(absPath[0])!=tolower(p[0]) || absPath[1]!=':') {
				emFatalError("emGetAbsolutePath impossible for: %s", path.Get());
			}
			cool=false;
			i=2;
		}
	}
	else if (p[0]=='\\' || p[0]=='/') {
		if (p[1]=='\\' || p[1]=='/') {
			absPath=path;
			cool=true;
			i=2;
		}
		else {
			if (cwd) {
				if (cwd[0] && cwd[1]==':') {
					absPath=emString(cwd,2);
					absPath+="\\";
				}
				else {
					emFatalError("emGetAbsolutePath for %s impossible with cwd %s", path.Get(), cwd);
				}
			}
			else {
				absPath=emGetCurrentDirectory().GetSubString(0,3);
			}
			cool=false;
			i=1;
		}
	}
#else
	if (p[0]=='/') {
		absPath=path;
		cool=true;
		i=1;
	}
#endif
	else {
		if (cwd) absPath=cwd;
		else absPath=emGetCurrentDirectory();
		cool=false;
		i=0;
	}
	while (p[i]) {
#if defined(_WIN32)
		for (j=i; p[j]!=0 && p[j]!='\\' && p[j]!='/'; j++);
#else
		for (j=i; p[j]!=0 && p[j]!='/'; j++);
#endif
		if (j==i || (j==i+1 && p[i]=='.')) {
			if (cool) {
				absPath=emString(p,i);
				cool=false;
			}
		}
		else if (j==i+2 && p[i]=='.' && p[i+1]=='.') {
			if (cool) {
				absPath=emString(p,i);
				cool=false;
			}
			absPath=emGetParentPath(absPath);
		}
		else if (!cool) {
			absPath=emGetChildPath(absPath,emString(p+i,j-i));
		}
		if (!p[j]) break;
		i=j+1;
	}

	return absPath;
}


const char * emGetNameInPath(const char * path)
{
	int i;

	i=strlen(path);
#if defined(_WIN32)
	while (i>0 && (path[i-1]=='\\' || path[i-1]=='/' || path[i-1]==':')) i--;
	while (i>0 && path[i-1]!='\\' && path[i-1]!='/' &&  path[i-1]!=':') i--;
#else
	while (i>0 && path[i-1]=='/') i--;
	while (i>0 && path[i-1]!='/') i--;
#endif
	return path+i;
}


const char * emGetExtensionInPath(const char * path)
{
	const char * name, * end, * p;

	name=emGetNameInPath(path);
	end=name+strlen(name);
	p=end;
	while (p>name && *p!='.') p--;
	if (*p!='.') p=end;
	return p;
}


bool emIsExistingPath(const char * path)
{
	return access(path,F_OK)==0;
}


bool emIsReadablePath(const char * path)
{
	return access(path,R_OK)==0;
}


bool emIsWritablePath(const char * path)
{
	return access(path,W_OK)==0;
}


bool emIsDirectory(const char * path)
{
	struct em_stat st;

#if defined(_WIN32)
	int i=strlen(path);
	while (i>0 && (path[i-1]=='\\' || path[i-1]=='/')) i--;
	if (
		i==2 && path[1]==':' && (
			(path[0]>='A' && path[0]<='Z') ||
			(path[0]>='a' && path[0]<='z')
		)
	) return true;
#endif

	if (em_stat(path,&st)!=0) return false;
	if ((st.st_mode&S_IFMT)!=S_IFDIR)  return false;
	return true;
}


bool emIsRegularFile(const char * path)
{
	struct em_stat st;

	if (em_stat(path,&st)!=0) return false;
	if ((st.st_mode&S_IFMT)!=S_IFREG)  return false;
	return true;
}


bool emIsSymLinkPath(const char * path)
{
#if defined(_WIN32)
	return false;
#else
	struct em_stat st;

	if (em_lstat(path,&st)!=0) return false;
	if ((st.st_mode&S_IFMT)!=S_IFLNK)  return false;
	return true;
#endif
}


bool emIsHiddenPath(const char * path)
{
#if defined(_WIN32)
	DWORD d;

	d=GetFileAttributes(path);
	return d!=0xFFFFFFFF && (d&FILE_ATTRIBUTE_HIDDEN)!=0;
#else
	return emGetNameInPath(path)[0] == '.';
#endif
}


emUInt64 emTryGetFileSize(const char * path)
{
	struct em_stat st;

	if (em_stat(path,&st)!=0) {
		throw emException(
			"Failed to get size of \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
	return st.st_size;
}


time_t emTryGetFileTime(const char * path)
{
	struct em_stat st;

	if (em_stat(path,&st)!=0) {
		throw emException(
			"Failed to get modification time of \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
	return st.st_mtime;
}


emString emGetCurrentDirectory()
{
	char tmp[1024];

	if (getcwd(tmp,sizeof(tmp))==NULL) {
		emFatalError("getcwd failed: %s",emGetErrorText(errno).Get());
	}
	return emString(tmp);
}


#if defined(_WIN32)
struct emDirHandleContent {
	HANDLE handle;
	WIN32_FIND_DATA data;
	bool first;
};
#endif


emDirHandle emTryOpenDir(const char * path)
{
#if defined(_WIN32)
	emDirHandleContent * hc;
	DWORD d;

	hc=new emDirHandleContent;
	hc->handle=FindFirstFile(
		emGetChildPath(path,"*.*"),
		&hc->data
	);
	if (hc->handle==INVALID_HANDLE_VALUE) {
		d=GetLastError();
		if (d!=ERROR_NO_MORE_FILES) {
			delete hc;
			throw emException(
				"Failed to read directory \"%s\": %s",
				path,
				emGetErrorText(d).Get()
			);
		}
	}
	hc->first=true;
	return hc;
#else
	DIR * dir;

	dir=opendir(path);
	if (!dir) {
		throw emException(
			"Failed to read directory \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}

	return (emDirHandle)dir;
#endif
}


emString emTryReadDir(emDirHandle dirHandle)
{
#if defined(_WIN32)
	emDirHandleContent * hc;

	hc=(emDirHandleContent*)dirHandle;
	for (;;) {
		if (hc->handle==INVALID_HANDLE_VALUE) return emString();
		if (hc->first) {
			hc->first=false;
		}
		else {
			if (!FindNextFile(hc->handle,&hc->data)) {
				if (GetLastError()!=ERROR_NO_MORE_FILES) {
					throw emException(
						"Failed to read directory: %s",
						emGetErrorText(GetLastError()).Get()
					);
				}
				FindClose(hc->handle);
				hc->handle=INVALID_HANDLE_VALUE;
				return emString();
			}
		}
		if (
			hc->data.cFileName[0] &&
			strcmp(hc->data.cFileName,".")!=0 &&
			strcmp(hc->data.cFileName,"..")!=0
		) return emString(hc->data.cFileName);
	}
#else
	struct dirent * de;
	emString res;

	for (;;) {
		errno=0;
		de=readdir((DIR*)dirHandle);
		if (!de) {
			if (errno) {
				throw emException(
					"Failed to read directory: %s",
					emGetErrorText(errno).Get()
				);
			}
			break;
		}
		if (
			de->d_name[0] &&
			strcmp(de->d_name,".")!=0 &&
			strcmp(de->d_name,"..")!=0
		) {
			res=emString(de->d_name);
			break;
		}
	}
	return res;
#endif
}


void emCloseDir(emDirHandle dirHandle)
{
#if defined(_WIN32)
	emDirHandleContent * hc;

	hc=(emDirHandleContent*)dirHandle;
	if (hc->handle!=INVALID_HANDLE_VALUE) {
		FindClose(hc->handle);
	}
	delete hc;
#else
	closedir((DIR*)dirHandle);
#endif
}


emArray<emString> emTryLoadDir(const char * path)
{
	emDirHandle dirHandle;
	emArray<emString> names;
	emString name;

	names.SetTuningLevel(1);
	dirHandle=emTryOpenDir(path);
	for (;;) {
		try {
			name=emTryReadDir(dirHandle);
		}
		catch (const emException & exception) {
			emCloseDir(dirHandle);
			throw exception;
		}
		if (name.IsEmpty()) break;
		names+=name;
	}
	emCloseDir(dirHandle);
	names.Compact();
	return names;
}


emArray<char> emTryLoadFile(const char * path)
{
	emArray<char> buf;
	FILE * f;
	emInt64 l;
	int i,j;

	buf.SetTuningLevel(4);
	f=fopen(path,"rb");
	if (!f) goto L_Err;
	if (fseek(f,0,SEEK_END)!=0) goto L_Err;
	l=ftell(f);
	if (l<0) goto L_Err;
	if (l>INT_MAX) { errno=EFBIG; goto L_Err; }
	buf.SetCount((int)l,true);
	if (fseek(f,0,SEEK_SET)!=0) goto L_Err;
	for (i=0; i<buf.GetCount(); i+=j) {
		j=fread(buf.GetWritable()+i,1,buf.GetCount()-i,f);
		if (j<=0) goto L_Err;
	}
	fclose(f);
	return buf;
L_Err:
	if (f) fclose(f);
	throw emException(
		"Failed to read file \"%s\": %s",
		path,
		emGetErrorText(errno).Get()
	);
}


void emTrySaveFile(const char * path, const char * data, int len)
{
	FILE * f;
	int i;

	f=fopen(path,"wb");
	if (!f) goto L_Err;
	while (len>0) {
		i=fwrite(data,1,len,f);
		if (i<=0) goto L_Err;
		data+=i;
		len-=i;
	}
	fclose(f);
	return;
L_Err:
	if (f) fclose(f);
	throw emException(
		"Failed to write file \"%s\": %s",
		path,
		emGetErrorText(errno).Get()
	);
}


void emTrySaveFile(const char * path, const emArray<char> & data)
{
	emTrySaveFile(path,data,data.GetCount());
}


void emTryMakeDirectories(const char * path, int mode)
{
	emString parentPath;

	if (*path && access(path,F_OK)!=0) {
		parentPath=emGetParentPath(path);
		if (parentPath!=path) emTryMakeDirectories(parentPath,mode);
#if defined(_WIN32)
		if (mkdir(path)!=0) {
#else
		if (mkdir(path,mode)!=0) {
#endif
			throw emException(
				"Failed to create directory \"%s\": %s",
				path,
				emGetErrorText(errno).Get()
			);
		}
	}
}


void emTryRemoveFile(const char * path)
{
	if (!*path) {
		throw emException("Cannot to remove file: empty path");
	}
	if (unlink(path)!=0) {
		throw emException(
			"Failed to remove \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
}


void emTryRemoveDirectory(const char * path)
{
	if (!*path) {
		throw emException("Cannot to remove directory: empty path");
	}
	if (rmdir(path)!=0) {
		throw emException(
			"Failed to remove directory \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
}


#ifdef _WIN32
static emString emW2A(const wchar_t * w)
{
	emString str;
	char * p;
	int i,l;

	l=WideCharToMultiByte(CP_ACP,0,w,-1,NULL,0,NULL,NULL);
	if (l>0) {
		p=str.SetLenGetWritable(l);
		l=WideCharToMultiByte(CP_ACP,0,w,-1,p,l,NULL,NULL);
	}
	if (l<=0) {
		str.Clear();
		for (i=0; w[i]; i++) str.Add(w[i]>=32 && w[i]<=126 ? (char)w[i] : '?');
	}
	return str;
}
#endif


#ifdef _WIN32
static void emTryRemoveFileOrTreeW(const wchar_t * path, bool force)
{
	emArray<wchar_t> subPath;
	WIN32_FIND_DATAW data;
	HANDLE h;
	DWORD d;

	if (!*path) {
		throw emException("Cannot remove file or directory: empty path");
	}

	d=GetFileAttributesW(path);
	if (d==INVALID_FILE_ATTRIBUTES) {
		// Yes, even do not simply return successful on ERROR_FILE_NOT_FOUND,
		// because we don't want to miss path conversion errors.
		throw emException(
			"Failed to get file attributes of \"%s\": %s",
			emW2A(path).Get(),
			emGetErrorText(GetLastError()).Get()
		);
	}
	if (force && (d&FILE_ATTRIBUTE_READONLY)!=0) {
		SetFileAttributesW(path,d&~FILE_ATTRIBUTE_READONLY);
	}
	if (d&FILE_ATTRIBUTE_DIRECTORY) {
		subPath=emArray<wchar_t>(path,lstrlenW(path));
		subPath.Add(L"\\*.*\0",5);
		h=FindFirstFileW(subPath.Get(),&data);
		if (h==INVALID_HANDLE_VALUE) {
			d=GetLastError();
			if (d!=ERROR_NO_MORE_FILES) {
				throw emException(
					"Failed to read directory \"%s\": %s",
					emW2A(path).Get(),
					emGetErrorText(d).Get()
				);
			}
		}
		else {
			do {
				if (
					data.cFileName[0] &&
					lstrcmpW(data.cFileName,L".")!=0 &&
					lstrcmpW(data.cFileName,L"..")!=0
				) {
					subPath=emArray<wchar_t>(path,lstrlenW(path));
					subPath.Add(L"\\",1);
					subPath.Add(data.cFileName,lstrlenW(data.cFileName)+1);
					emTryRemoveFileOrTreeW(subPath,force);
				}
			} while(FindNextFileW(h,&data));
			if (GetLastError()!=ERROR_NO_MORE_FILES) {
				FindClose(h);
				throw emException(
					"Failed to read directory: %s",
					emGetErrorText(GetLastError()).Get()
				);
			}
			FindClose(h);
		}
		if (!RemoveDirectoryW(path)) {
			throw emException(
				"Failed to remove directory \"%s\": %s",
				emW2A(path).Get(),
				emGetErrorText(GetLastError()).Get()
			);
		}
	}
	else {
		if (!DeleteFileW(path)) {
			throw emException(
				"Failed to remove \"%s\": %s",
				emW2A(path).Get(),
				emGetErrorText(GetLastError()).Get()
			);
		}
	}
}
#endif


void emTryRemoveFileOrTree(const char * path, bool force)
{
#ifdef _WIN32
	emArray<wchar_t> wPath;
	emString absPath;
	int i,l;

	// For the (indirect) use by emTmpConv, removal of trees must be quite
	// reliable. But on Windows, "normal" removal fails if:
	//  - A path is longer than 260 bytes.
	//  - A path contains Unicode characters not in the active code page.
	//  - A file name equals a device name (aux, con, com1, com2, ...).
	// All these problems can be solved by using wide char functions and
	// prefixing the absolute path with "\\?\". But care must be taken
	// because "\\?\" brings some certain behavior:
	//  - Slashes are not implicitly converted to backslashes.
	//  - "." and ".." are losing their meaning.
	//  - File names can contain characters like "<", ">" and "|"
	// (Hint: There is the alternative prefix "\\.\", but that one does not
	// help with long paths.)

	static const struct VersionTester {
		bool result;
		VersionTester()
		{
			// From which Windows version on is the \\?\ path prefix
			// supported? I could not find it out, but I could test
			// XP (5.1).
			static const unsigned long minWinVer[2] = { 5, 1 };
			OSVERSIONINFO osvi;
			memset(&osvi,0,sizeof(osvi));
			osvi.dwOSVersionInfoSize=sizeof(osvi);
			GetVersionEx(&osvi);
			result=
				osvi.dwMajorVersion > minWinVer[0] || (
					osvi.dwMajorVersion == minWinVer[0] &&
					osvi.dwMinorVersion >= minWinVer[1]
				)
			;
		}
	} versionTester;

	absPath=emGetAbsolutePath(path);

	if (versionTester.result) {
		if (absPath[0] && absPath[1]==':' && (absPath[2]=='\\' || absPath[2]=='/')) {
			for (i=0; absPath[i]; i++) {
				if (absPath[i]=='/') absPath.Replace(i,1,"\\");
			}
			absPath.Insert(0,"\\\\?\\");
		}
	}

	SetLastError(ERROR_SUCCESS);
	l=MultiByteToWideChar(CP_ACP,0,absPath.Get(),-1,NULL,0);
	if (l>0) {
		wPath.SetCount(l+1);
		wPath.Set(l,0);
		l=MultiByteToWideChar(CP_ACP,0,absPath.Get(),-1,wPath.GetWritable(),l);
	}
	if (l<=0 || GetLastError()!=ERROR_SUCCESS) {
		throw emException(
			"Failed to convert path \"%s\" to wide char: %s",
			absPath.Get(),emGetErrorText(GetLastError()).Get()
		);
	}

	emTryRemoveFileOrTreeW(wPath,force);

#else

	emArray<emString> list;
	struct em_stat st;
	int i;

	if (!*path) {
		throw emException("Cannot remove file or directory: empty path");
	}

	if (em_lstat(path,&st)!=0) {
		throw emException(
			"Failed to get file information of \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}

	if ((st.st_mode&S_IFMT)==S_IFDIR) {
		if (force && (st.st_mode&0700)!=0700) {
			chmod(path,(st.st_mode&07777)|0700);
		}
		list=emTryLoadDir(path);
		for (i=0; i<list.GetCount(); i++) {
			emTryRemoveFileOrTree(emGetChildPath(path,list[i]),force);
		}
		if (rmdir(path)!=0) {
			throw emException(
				"Failed to remove directory \"%s\": %s",
				path,
				emGetErrorText(errno).Get()
			);
		}
	}
	else {
		if (unlink(path)!=0) {
			throw emException(
				"Failed to remove \"%s\": %s",
				path,
				emGetErrorText(errno).Get()
			);
		}
	}
#endif
}


void emTryCopyFileOrTree(const char * targetPath, const char * sourcePath)
{
	emDirHandle dh;
	emString nm;
	FILE * sf, * tf;
	char buf[8192];
	int bufFill,len;

	if (emIsDirectory(sourcePath)) {
		emTryMakeDirectories(targetPath);
		dh=emTryOpenDir(sourcePath);
		try {
			for (;;) {
				nm=emTryReadDir(dh);
				if (nm.IsEmpty()) break;
				emTryCopyFileOrTree(
					emGetChildPath(targetPath,nm),
					emGetChildPath(sourcePath,nm)
				);
			}
		}
		catch (const emException & exception) {
			emCloseDir(dh);
			throw exception;
		}
		emCloseDir(dh);
	}
	else if (emIsExistingPath(targetPath)) {
		errno=EEXIST;
		goto L_Throw_per_errno;
	}
	else {
		sf=fopen(sourcePath,"rb");
		if (!sf) {
			goto L_Throw_per_errno;
		}
		tf=fopen(targetPath,"wb");
		if (!tf) {
			fclose(sf);
			goto L_Throw_per_errno;
		}
		bufFill=0;
		do {
			if (sf) {
				len=sizeof(buf)-bufFill;
				if (len>0) {
					len=fread(buf+bufFill,1,len,sf);
					if (ferror(sf)) {
						fclose(sf);
						fclose(tf);
						goto L_Throw_per_errno;
					}
					if (feof(sf)) {
						fclose(sf);
						sf=NULL;
					}
					bufFill+=len;
				}
			}
			if (bufFill>0) {
				len=fwrite(buf,1,bufFill,tf);
				if (ferror(tf)) {
					if (sf) fclose(sf);
					fclose(tf);
					goto L_Throw_per_errno;
				}
				bufFill-=len;
				if (bufFill>0) memmove(buf,buf+len,bufFill);
			}
		} while (sf || bufFill>0);
		fclose(tf);
	}
	return;

L_Throw_per_errno:
	throw emException(
		"Failed to copy \"%s\" to \"%s\": %s",
		sourcePath,
		targetPath,
		emGetErrorText(errno).Get()
	);
}


//==============================================================================
//======================== Accessing Dynamic Libraries =========================
//==============================================================================

struct emLibTableEntry {
	emString Filename;
	emUInt64 RefCount; // Zero means infinite.
#if defined(_WIN32)
	HMODULE HModule;
#else
	void * DLHandle;
#endif
};

static emThreadMiniMutex emLibTableMutex;
static emArray<emLibTableEntry*> emLibTable;


static int emCompareLibEntryFilename(
	emLibTableEntry * const * entry, void * filename, void * context
)
{
	return strcmp((*entry)->Filename.Get(),(const char*)filename);
}


emLibHandle emTryOpenLib(const char * libName, bool isFilename)
{
	emLibTableEntry * e;
	emString filename;
	int idx;
#if defined(_WIN32)
	HMODULE hModule;
#else
	void * dlHandle;
#endif

	if (isFilename) {
		filename=libName;
	}
	else {
#if defined(_WIN32) || defined(__CYGWIN__)
		filename=emString::Format("%s.dll",libName);
#elif defined(__APPLE__)
		filename=emString::Format("lib%s.dylib",libName);
#else
		filename=emString::Format("lib%s.so",libName);
#endif
	}

	emLibTableMutex.Lock();

	idx=emLibTable.BinarySearchByKey(
		(void*)filename.Get(),
		emCompareLibEntryFilename
	);
	if (idx>=0) {
		e=emLibTable[idx];
		if (e->RefCount) e->RefCount++;
		emLibTableMutex.Unlock();
		return e;
	}
#if defined(_WIN32)
	hModule=LoadLibrary(filename.Get());
	if (!hModule) {
		emLibTableMutex.Unlock();
		throw emException(
			"Failed to load library \"%s\": %s",
			filename.Get(),
			emGetErrorText(GetLastError()).Get()
		);
	}
#else
	dlHandle=dlopen(filename,RTLD_NOW|RTLD_GLOBAL);
	if (!dlHandle) {
		emLibTableMutex.Unlock();
#		if defined(__linux__) || defined(__sun__) || defined(__FreeBSD__)
			throw emException("%s",dlerror());
#		else
			throw emException(
				"Failed to load library \"%s\": %s",
				filename.Get(),
				dlerror()
			);
#		endif
	}
#endif
	e=new emLibTableEntry;
	e->Filename=filename;
	e->RefCount=1;
#if defined(_WIN32)
	e->HModule=hModule;
#else
	e->DLHandle=dlHandle;
#endif
	emLibTable.Insert(~idx,e);

	filename.Clear();
	e->Filename.MakeNonShared();

	emLibTableMutex.Unlock();
	return e;
}


void * emTryResolveSymbolFromLib(emLibHandle handle, const char * symbol)
{
	emLibTableEntry * e;
	void * r;
#if defined(_WIN32)
#else
	const char * err;
#endif

	e=(emLibTableEntry*)handle;
#if defined(_WIN32)
	r=(void*)GetProcAddress(e->HModule,symbol);
	if (!r) {
		throw emException(
			"Failed to get address of \"%s\" in \"%s\": %s",
			symbol,
			e->Filename.Get(),
			emGetErrorText(GetLastError()).Get()
		);
	}
#else
	dlerror();
	r=dlsym(e->DLHandle,symbol);
	err=dlerror();
	if (err) {
#		if defined(ANDROID)
			throw emException(
				"Failed to get address of \"%s\" in \"%s\".",
				symbol,
				e->Filename.Get()
			);
#		elif defined(__linux__) || defined(__sun__) || defined(__FreeBSD__)
			throw emException("%s",err);
#		else
			throw emException(
				"Failed to get address of \"%s\" in \"%s\": %s",
				symbol,
				e->Filename.Get(),
				err
			);
#		endif
	}
#endif
	return r;
}


void emCloseLib(emLibHandle handle)
{
	emLibTableEntry * e;

	emLibTableMutex.Lock();
	e=(emLibTableEntry*)handle;
	if (e->RefCount && !e->RefCount--) {
#if defined(_WIN32)
		FreeLibrary(e->HModule);
		e->HModule=NULL;
#else
		dlclose(e->DLHandle);
		e->DLHandle=NULL;
#endif
		emLibTable.BinaryRemoveByKey(
			(void*)e->Filename.Get(),
			emCompareLibEntryFilename
		);
		delete e;
	}
	emLibTableMutex.Unlock();
}


void * emTryResolveSymbol(
	const char * libName, bool isFilename, const char * symbol
)
{
	emLibTableEntry * e;
	void * r;

	e=(emLibTableEntry*)emTryOpenLib(libName,isFilename);
	r=emTryResolveSymbolFromLib(e,symbol);
	emLibTableMutex.Lock();
	e->RefCount=0;
	emLibTableMutex.Unlock();
	return r;
}


//==============================================================================
//=========================== Pseudo Random Numbers ============================
//==============================================================================

int emGetIntRandom(int minimum, int maximum)
{
	return (int)emGetInt64Random(minimum,maximum);
}


unsigned int emGetUIntRandom(unsigned int minimum, unsigned int maximum)
{
	return (unsigned int)emGetUInt64Random(minimum,maximum);
}


emInt64 emGetInt64Random(emInt64 minimum, emInt64 maximum)
{
	return (emInt64)(emGetUInt64Random(
		((emUInt64)minimum)^(((emUInt64)1)<<63),
		((emUInt64)maximum)^(((emUInt64)1)<<63)
	)^(((emUInt64)1)<<63));
}


emUInt64 emGetUInt64Random(emUInt64 minimum, emUInt64 maximum)
{
	static emUInt32 seedLo=0x302D9934U;
	static emUInt32 seedHi=0xD5441C6EU;
	static emUInt32 count=0;
	emUInt32 a,b,c;
	emUInt64 r;

	if (!count) {
		a=(emUInt32)time(NULL);
		b=(emUInt32)emGetClockMS();
		c=(emUInt32)emGetProcessId();
		seedLo^=(a+b*1321+c*1231277)*0x106F68F6U+0x0723BF76U;
		seedHi^=(a*9601769+b*5099+c)*0xA0ECFAC5U+0x1840E54BU;
	}
	count++;
	seedLo=seedLo*0xC78D632DU+0xBDFAAE3BU;
	seedHi=seedHi*0xAC7D7A21U+0x2FF59947U;
	r=maximum-minimum+1;
	if (r>(emUInt64)0xffffffff) r=((((emUInt64)seedHi)<<32)|seedLo)%r;
	else if (r) r=((seedLo>>16)^seedHi)%((emUInt32)r);
	else r=(((emUInt64)seedHi)<<32)|seedLo;
	return r+minimum;
}


double emGetDblRandom(double minimum, double maximum)
{
	return emGetUInt64Random(
		0,(emUInt64)(emInt64)-1
	)/((double)(emUInt64)(emInt64)-1)*(maximum-minimum)+minimum;
}


//==============================================================================
//========================== Checksums and hash codes ==========================
//==============================================================================

emUInt32 emCalcAdler32(const char * src, int srcLen, emUInt32 start)
{
	const char * brk, * end;
	emUInt32 lo, hi;

	lo=start&0xffff;
	hi=start>>16;
	end=src+srcLen;
	while (src<end) {
		if (end-src<=5552) brk=end;
		else brk=src+5552;
		do {
			lo+=(emUInt8)*src++;
			hi+=lo;
		} while(src<brk);
		lo%=65521;
		hi%=65521;
	}
	return (hi<<16)|lo;
}


emUInt32 emCalcCRC32(const char * src, int srcLen, emUInt32 start)
{
	static const struct CRC32Table {

		emUInt32 tab[256];

		CRC32Table()
		{
			emUInt32 r;
			int i,j;
			for (i=0; i<256; i++) {
				for (r=i, j=0; j<8; j++) {
					if ((r&1)!=0) r=(r>>1)^0xEDB88320; else r>>=1;
				}
				tab[i]=r;
			}
		}

	} crc32Table;

	const char * end;
	emUInt32 r;

	r=start;
	if (srcLen>0) {
		r=~r;
		end=src+srcLen;
		do {
			r=crc32Table.tab[((emUInt8)*src++)^((emUInt8)r)]^(r>>8);
		} while (src<end);
		r=~r;
	}
	return r;
}


emUInt64 emCalcCRC64(const char * src, int srcLen, emUInt64 start)
{
	static const struct CRC64Table {

		emUInt64 tab[256];

		CRC64Table()
		{
			emUInt64 r;
			int i,j;
			for (i=0; i<256; i++) {
				for (r=i, j=0; j<8; j++) {
					if ((r&1)!=0) r=(r>>1)^(((emUInt64)0xD8000000)<<32);
					else r>>=1;
				}
				tab[i]=r;
			}
		}

	} crc64Table;

	const char * end;
	emUInt64 r;

	r=start;
	if (srcLen>0) {
		r=~r;
		end=src+srcLen;
		do {
			r=crc64Table.tab[((emUInt8)*src++)^((emUInt8)r)]^(r>>8);
		} while (src<end);
		r=~r;
	}
	return r;
}


int emCalcHashCode(const char * str, int start)
{
	unsigned int r,c;

	r=(unsigned int)start;
	c=(unsigned char)*str++;
	if (c) {
		do {
			r=r*335171+c;
			c=(unsigned char)*str++;
		} while (c);
	}
	return (int)r;
}


emString emCalcHashName(const char * src, int srcLen, int hashLen)
{
	emString hash;
	char * hashPtr;
	unsigned int a;
	emUInt64 b;
	int i,j,k;

	// Part 1: Prepare the hash name consisting of digits and non-capital
	// letters only.
	//
	// For this algorithm, it has been proved with a test program, that the
	// prime number 6795413 produces the theoretical minimum possible number
	// of collisions for all possible sources with all combinations of
	// srcLen = 1 to 3 (256 combinations per source byte) and hashLen = 1 to
	// 5 (36 combinations per hash byte). About 50 different prime numbers
	// have been tested, three of them resulted best, 6795413 is one of
	// those three. But it is still not proved that the results are even an
	// optimum for greater srcLen and hashLen.
	hashPtr=hash.SetLenGetWritable(hashLen);
	memset(hashPtr,0,hashLen);
	for (i=0; i<srcLen; i++) {
		for (j=0; j<hashLen; j++) {
			a=(unsigned char)hashPtr[j];
			if (j==hashLen-1) a+=(unsigned char)src[i];
			a*=6795413;
			hashPtr[j]=(char)(a%36);
			a/=36;
			for (k=j-1; k>=0 && a!=0; k--) {
				a+=hashPtr[k];
				hashPtr[k]=(char)(a%36);
				a/=36;
			}
		}
	}
	for (i=0; i<hashLen; i++) {
		a=(unsigned char)hashPtr[i];
		if (a<10) a+='0';
		else a+='a'-10;
		hashPtr[i]=(char)a;
	}

	// Part 2: Improve the hash name a little bit through capitalization.
	// This is an extra algorithm because it could happen that the user
	// performs comparisons ignoring the capitalization, for example when
	// using the name as a DOS file name.
	for (i=0, j=0; i<hashLen; i++) {
		if (hashPtr[i]>='a' && hashPtr[i]<='z') j++;
	}
	if (j<=32) b=emCalcCRC32(src,srcLen);
	else b=emCalcCRC64(src,srcLen);
	if (j<=16) b^=b>>16;
	if (j<=8) b^=b>>8;
	if (j<=4) b^=b>>4;
	if (j<=2) b^=b>>2;
	if (j<=1) b^=b>>1;
	for (i=0; i<hashLen; i++) {
		if (hashPtr[i]>='a' && hashPtr[i]<='z') {
			if ((b&1)!=0) hashPtr[i]+=(char)('A'-'a');
			b>>=1;
		}
	}

	return hash;
}
