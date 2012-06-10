//------------------------------------------------------------------------------
// emStd2.cpp
//
// Copyright (C) 2004-2012 Oliver Hamann.
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
#include <emCore/emStd2.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emThread.h>


//==============================================================================
//================================== Who am I ==================================
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
	static volatile int haveRDTSC=-1;
	emUInt32 a,d;

	while (haveRDTSC<=0) {
		if (haveRDTSC==0) return 0;
		asm volatile (
#			if defined(__x86_64__)
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
#			else
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
#			endif
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
#			if defined(__x86_64__)
				"pop %%rbx\n"
#			else
				"pop %%ebx\n"
#			endif
			: "=d"(d) : : "eax","ecx","cc"
		);
		haveRDTSC=(d>>4)&1;
	}
	asm volatile (
		"rdtsc\n"
		: "=a"(a),"=d"(d) : : "cc"
	);
	return (((emUInt64)d)<<32)|a;
#else
	return 0;
#endif
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
			//??? Bug: could be the wrong drive:
			if (cwd) absPath=cwd;
			else absPath=emGetCurrentDirectory();
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
			if (cwd && cwd[0] && cwd[1]==':') {
				absPath=emString(cwd,2);
				absPath+="\\";
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


emUInt64 emTryGetFileSize(const char * path) throw(emString)
{
	struct em_stat st;

	if (em_stat(path,&st)!=0) {
		throw emString::Format(
			"Failed to get size of \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
	return st.st_size;
}


time_t emTryGetFileTime(const char * path) throw(emString)
{
	struct em_stat st;

	if (em_stat(path,&st)!=0) {
		throw emString::Format(
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
struct emWndsDirHandleContent {
	HANDLE handle;
	WIN32_FIND_DATA data;
	bool first;
};
#endif


emDirHandle emTryOpenDir(const char * path) throw(emString)
{
#if defined(_WIN32)
	emWndsDirHandleContent * hc;
	DWORD d;

	hc=new emWndsDirHandleContent;
	hc->handle=FindFirstFile(
		emGetChildPath(path,"*.*"),
		&hc->data
	);
	if (hc->handle==INVALID_HANDLE_VALUE) {
		d=GetLastError();
		if (d!=ERROR_NO_MORE_FILES) {
			delete hc;
			throw emString::Format(
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
		throw emString::Format(
			"Failed to read directory \"%s\": %s",
			path,
			emGetErrorText(errno).Get()
		);
	}
	return dir;
#endif
}


emString emTryReadDir(emDirHandle dirHandle) throw(emString)
{
#if defined(_WIN32)
	emWndsDirHandleContent * hc;

	hc=(emWndsDirHandleContent*)dirHandle;
	for (;;) {
		if (hc->handle==INVALID_HANDLE_VALUE) return emString();
		if (hc->first) {
			hc->first=false;
		}
		else {
			if (!FindNextFile(hc->handle,&hc->data)) {
				if (GetLastError()!=ERROR_NO_MORE_FILES) {
					throw emString::Format(
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
	struct dirent * buf, * de;
	emString res;
	DIR * dir;

	// Hint: On some systems, the size of dirent.d_name is 1 and must be
	// extended when creating a buffer for use with readdir_r.
	buf=(struct dirent *)malloc(sizeof(struct dirent)+513);
	dir=(DIR*)dirHandle;
	for (;;) {
		if (readdir_r(dir,buf,&de)!=0) {
			free(buf);
			throw emString::Format(
				"Failed to read directory: %s",
				emGetErrorText(errno).Get()
			);
		}
		if (!de) break;
		if (
			de->d_name[0] &&
			strcmp(de->d_name,".")!=0 &&
			strcmp(de->d_name,"..")!=0
		) {
			res=emString(de->d_name);
			break;
		}
	}
	free(buf);
	return res;
#endif
}


void emCloseDir(emDirHandle dirHandle)
{
#if defined(_WIN32)
	emWndsDirHandleContent * hc;

	hc=(emWndsDirHandleContent*)dirHandle;
	if (hc->handle!=INVALID_HANDLE_VALUE) {
		FindClose(hc->handle);
	}
	delete hc;
#else
	closedir((DIR*)dirHandle);
#endif
}


emArray<emString> emTryLoadDir(const char * path) throw(emString)
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
		catch (emString errorMessage) {
			emCloseDir(dirHandle);
			throw errorMessage;
		}
		if (name.IsEmpty()) break;
		names+=name;
	}
	emCloseDir(dirHandle);
	names.Compact();
	return names;
}


emArray<char> emTryLoadFile(const char * path) throw(emString)
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
	throw emString::Format(
		"Failed to read file \"%s\": %s",
		path,
		emGetErrorText(errno).Get()
	);
}


void emTrySaveFile(
	const char * path, const char * data, int len
) throw(emString)
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
	throw emString::Format(
		"Failed to write file \"%s\": %s",
		path,
		emGetErrorText(errno).Get()
	);
}


void emTrySaveFile(
	const char * path, const emArray<char> & data
) throw(emString)
{
	emTrySaveFile(path,data,data.GetCount());
}


void emTryMakeDirectories(const char * path, int mode) throw(emString)
{
	emString parentPath;

	if (access(path,F_OK)!=0) {
		parentPath=emGetParentPath(path);
		if (parentPath!=path) emTryMakeDirectories(parentPath,mode);
#if defined(_WIN32)
		if (mkdir(path)!=0) {
#else
		if (mkdir(path,mode)!=0) {
#endif
			throw emString::Format(
				"Failed to create directory \"%s\": %s",
				path,
				emGetErrorText(errno).Get()
			);
		}
	}
}


void emTryRemoveFileOrTree(const char * path, bool force) throw(emString)
{
	emArray<emString> list;
	struct em_stat st;
	bool chmodDone;
	int i;

	if (!emIsExistingPath(path) && !emIsSymLinkPath(path)) {
		return;
	}

	chmodDone=false;
L_TryIt:
	try {
		if (emIsDirectory(path) && !emIsSymLinkPath(path)) {
			list=emTryLoadDir(path);
			for (i=0; i<list.GetCount(); i++) {
				emTryRemoveFileOrTree(emGetChildPath(path,list[i]),force);
			}
			if (rmdir(path)!=0) {
				throw emString::Format(
					"Failed to remove directory \"%s\": %s",
					path,
					emGetErrorText(errno).Get()
				);
			}
		}
		else {
			if (remove(path)!=0) {
				throw emString::Format(
					"Failed to remove \"%s\": %s",
					path,
					emGetErrorText(errno).Get()
				);
			}
		}
	}
	catch (emString errorMessage) {
		if (
			!force ||
			chmodDone ||
			emIsSymLinkPath(path) ||
			em_stat(path,&st)!=0
		) {
			throw errorMessage;
		}
		chmod(path,(st.st_mode&07777)|0700);
		chmodDone=true;
		goto L_TryIt;
	}
}


void emTryCopyFileOrTree(
	const char * targetPath, const char * sourcePath
) throw(emString)
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
		catch (emString errorMessage) {
			emCloseDir(dh);
			throw errorMessage;
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
	throw emString::Format(
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


emLibHandle emTryOpenLib(const char * libName, bool isFilename) throw(emString)
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
		throw emString::Format(
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
			throw emString(dlerror());
#		else
			throw emString::Format(
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

	filename.Empty();
	e->Filename.MakeNonShared();

	emLibTableMutex.Unlock();
	return e;
}


void * emTryResolveSymbolFromLib(
	emLibHandle handle, const char * symbol
) throw(emString)
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
		throw emString::Format(
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
			throw emString::Format(
				"Failed to get address of \"%s\" in \"%s\".",
				symbol,
				e->Filename.Get()
			);
#		elif defined(__linux__) || defined(__sun__) || defined(__FreeBSD__)
			throw emString(err);
#		else
			throw emString::Format(
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
) throw(emString)
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
	emUInt64 r;

	if (!count) {
		seedLo^=((emUInt32)emGetClockMS())*0x106F68F6U+0x0723BF76U;
		seedHi^=((emUInt32)time(NULL))*0xA0ECFAC5U+0x1840E54BU;
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
	static emThreadInitMutex initMutex;
	static emUInt32 tab[256];
	const char * end;
	emUInt32 r;
	int i,j;

	if (initMutex.Begin()) {
		for (i=0; i<256; i++) {
			for (r=i, j=0; j<8; j++) {
				if ((r&1)!=0) r=(r>>1)^0xEDB88320; else r>>=1;
			}
			tab[i]=r;
		}
		initMutex.End();
	}

	r=start;
	if (srcLen>0) {
		r=~r;
		end=src+srcLen;
		do {
			r=tab[((emUInt8)*src++)^((emUInt8)r)]^(r>>8);
		} while (src<end);
		r=~r;
	}
	return r;
}


emUInt64 emCalcCRC64(const char * src, int srcLen, emUInt64 start)
{
	static emThreadInitMutex initMutex;
	static emUInt64 tab[256];
	const char * end;
	emUInt64 r;
	int i,j;

	if (initMutex.Begin()) {
		for (i=0; i<256; i++) {
			for (r=i, j=0; j<8; j++) {
				if ((r&1)!=0) r=(r>>1)^(((emUInt64)0xD8000000)<<32);
				else r>>=1;
			}
			tab[i]=r;
		}
		initMutex.End();
	}

	r=start;
	if (srcLen>0) {
		r=~r;
		end=src+srcLen;
		do {
			r=tab[((emUInt8)*src++)^((emUInt8)r)]^(r>>8);
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
