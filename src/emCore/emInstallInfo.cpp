//------------------------------------------------------------------------------
// emInstallInfo.cpp
//
// Copyright (C) 2006-2012 Oliver Hamann.
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
#	include <shlobj.h>
#	ifndef CSIDL_PROFILE
#		define CSIDL_PROFILE 0x0028
#	endif
#endif
#include <emCore/emInstallInfo.h>
#include <emCore/emToolkit.h>
#include <emCore/emThread.h>


static void emInitBaseInstallPaths(emString basePaths[EM_NUMBER_OF_IDTS])
{
#if defined(_WIN32)
	char buf[MAX_PATH+64];
	DWORD d;
	HRESULT r;
	LPMALLOC pMalloc;
	LPITEMIDLIST pidl;
	BOOL b;
#endif
	emString prefix;
	const char * p;

#if defined(_WIN32)
	r=SHGetMalloc(&pMalloc);
	if (r!=NOERROR) emFatalError("SHGetMalloc failed.");
#endif

	p=getenv("EM_DIR");
	if (!p) emFatalError("Environment variable EM_DIR not set.");
	prefix=emGetAbsolutePath(p);

	basePaths[EM_IDT_BIN]=emGetChildPath(prefix,"bin");

	basePaths[EM_IDT_INCLUDE]=emGetChildPath(prefix,"include");

	basePaths[EM_IDT_LIB]=emGetChildPath(prefix,"lib");

	basePaths[EM_IDT_HTML_DOC]=emGetChildPath(emGetChildPath(prefix,"doc"),"html");

	basePaths[EM_IDT_PS_DOC]=emGetChildPath(emGetChildPath(prefix,"doc"),"ps");

#if defined(_WIN32) && !defined(__CYGWIN__)
	basePaths[EM_IDT_HOST_CONFIG]=emGetChildPath(prefix,"etcw");
#elif defined(ANDROID)
	basePaths[EM_IDT_HOST_CONFIG]=emGetChildPath(prefix,"etca");
#else
	basePaths[EM_IDT_HOST_CONFIG]=emGetChildPath(prefix,"etc");
#endif

	basePaths[EM_IDT_RES]=emGetChildPath(prefix,"res");

#if defined(_WIN32)
	r=SHGetSpecialFolderLocation(NULL,CSIDL_PROFILE,&pidl);
	if (r!=NOERROR) emFatalError("SHGetSpecialFolderLocation failed.");
	b=SHGetPathFromIDList(pidl,buf);
	if (!b) emFatalError("SHGetPathFromIDList failed.");
	pMalloc->Free(pidl);
	basePaths[EM_IDT_HOME]=buf;
#else
	p=getenv("HOME");
	if (!p) emFatalError("Environment variable HOME not set.");
	basePaths[EM_IDT_HOME]=p;
#endif

	p=getenv("EM_USER_CONFIG_DIR");
	if (p) {
		basePaths[EM_IDT_USER_CONFIG]=p;
	}
	else {
#if defined(_WIN32)
		r=SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&pidl);
		if (r!=NOERROR) emFatalError("SHGetSpecialFolderLocation failed.");
		b=SHGetPathFromIDList(pidl,buf);
		if (!b) emFatalError("SHGetPathFromIDList failed.");
		pMalloc->Free(pidl);
		basePaths[EM_IDT_USER_CONFIG]=emGetChildPath(buf,"eaglemode");
#else
		basePaths[EM_IDT_USER_CONFIG]=emGetChildPath(basePaths[EM_IDT_HOME],".eaglemode");
#endif
	}

#if defined(_WIN32)
	d=GetTempPath(sizeof(buf),buf);
	if (d==0 || d>=sizeof(buf)) {
		emFatalError("GetTempPath failed.");
	}
	buf[d-1]=0;
	basePaths[EM_IDT_TMP]=buf;
#else
	p=getenv("TMPDIR");
	if (!p) p="/tmp";
	basePaths[EM_IDT_TMP]=p;
#endif
}


emString emGetInstallPath(
	emInstallDirType idt, const char * prj, const char * subPath
)
{
	static emThreadInitMutex initMutex;
	static emString basePaths[EM_NUMBER_OF_IDTS];
	emString str;

	if (initMutex.Begin()) {
		emInitBaseInstallPaths(basePaths);
		initMutex.End();
	}

	if (!prj || !*prj) {
		emFatalError("emGetInstallPath: Illegal prj argument.");
	}

	switch (idt) {
		case EM_IDT_BIN:
		case EM_IDT_LIB:
		case EM_IDT_HTML_DOC:
		case EM_IDT_PS_DOC:
		case EM_IDT_TMP:
		case EM_IDT_HOME:
			str=basePaths[idt];
			break;
		case EM_IDT_INCLUDE:
		case EM_IDT_USER_CONFIG:
		case EM_IDT_HOST_CONFIG:
		case EM_IDT_RES:
			str=emGetChildPath(basePaths[idt],prj);
			break;
		default:
			emFatalError("emGetInstallPath: Illegal idt argument.");
	}

	if (subPath && *subPath) {
		str=emGetChildPath(str,subPath);
	}

	return str;
}


emString emGetConfigDirOverloadable(
	emContext & context, const char * prj, const char * subDir
)
{
	emString hostDir,userDir,result,warning,varModelName;
	emIntRec hostVer,userVer;

	hostDir=emGetInstallPath(EM_IDT_HOST_CONFIG,prj,subDir);
	userDir=emGetInstallPath(EM_IDT_USER_CONFIG,prj,subDir);

	try {
		hostVer.TryLoad(emGetChildPath(hostDir,"version"));
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}

	result=hostDir;
	warning="";
	if (emIsExistingPath(userDir)) {
		try {
			userVer.TryLoad(emGetChildPath(userDir,"version"));
		}
		catch (emString errorMessage) {
			warning=emString::Format(
				"The directory \"%s\" is not used, because of this error: %s",
				userDir.Get(),
				errorMessage.Get()
			);
		}
		if (warning.IsEmpty() && hostVer.Get()!=userVer.Get()) {
			warning=emString::Format(
				"The directory \"%s\" is not used, because its version file indicates a wrong version.",
				userDir.Get()
			);
		}
		if (warning.IsEmpty()) result=userDir;
	}

	varModelName=emString::Format(
		"emGetConfigDirOverloadable.warning.%s",
		userDir.Get()
	);
	if (warning.IsEmpty()) {
		emVarModel<emString>::Remove(context.GetRootContext(),varModelName);
	}
	else if (
		warning != emVarModel<emString>::Get(
			context.GetRootContext(),
			varModelName,
			emString()
		)
	) {
		emVarModel<emString>::Set(
			context.GetRootContext(),
			varModelName,
			warning,
			UINT_MAX
		);
		if (emScreen::LookupInherited(context).Get()!=NULL) {
			emTkDialog::ShowMessage(context,"WARNING",warning);
		}
		else {
			emWarning("%s",warning.Get());
		}
	}

	return result;
}
