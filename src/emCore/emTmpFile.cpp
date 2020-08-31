//------------------------------------------------------------------------------
// emTmpFile.cpp
//
// Copyright (C) 2006-2008,2014,2019-2020 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emTmpFile.h>


//==============================================================================
//================================= emTmpFile ==================================
//==============================================================================

emTmpFile::emTmpFile()
{
}


emTmpFile::emTmpFile(emRootContext & rootContext, const char * postfix)
{
	Setup(rootContext,postfix);
}


emTmpFile::emTmpFile(const emString & customPath)
{
	SetupCustomPath(customPath);
}


emTmpFile::~emTmpFile()
{
	Discard();
}


void emTmpFile::TrySetup(emRootContext & rootContext, const char * postfix)
{
	emRef<emTmpFileMaster> m;

	Discard();
	m=emTmpFileMaster::Acquire(rootContext);
	Path=m->TryInventPath(postfix);
}


void emTmpFile::Setup(emRootContext & rootContext, const char * postfix)
{
	try {
		TrySetup(rootContext,postfix);
	}
	catch (const emException & exception) {
		emFatalError("%s",exception.GetText().Get());
	}
}


void emTmpFile::SetupCustomPath(const emString & customPath)
{
	Discard();
	Path=customPath;
}


void emTmpFile::Discard()
{
	if (!Path.IsEmpty()) {
		if (emIsExistingPath(Path) || emIsSymLinkPath(Path)) {
			try {
				emTryRemoveFileOrTree(Path,true);
			}
			catch (const emException &) {
			}
		}
		Path.Clear();
	}
}


//==============================================================================
//============================== emTmpFileMaster ===============================
//==============================================================================

emRef<emTmpFileMaster> emTmpFileMaster::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emTmpFileMaster,rootContext,"")
}


emString emTmpFileMaster::TryInventPath(const char * postfix)
{
	emString name,path;

	if (DirPath.IsEmpty()) {
		TryStartOwnDirectory();
	}
	for (;;) {
		FileNameCounter++;
		name=emString::Format("%x",FileNameCounter);
		if (postfix && *postfix) {
			if (*postfix!='.') name+=".";
			name+=postfix;
		}
		path=emGetChildPath(DirPath,name);
		if (!emIsExistingPath(path)) break;
	}
	return path;
}


emTmpFileMaster::emTmpFileMaster(emContext & context, const emString & name)
	: emModel(context,name), IpcServer(GetScheduler())
{
	FileNameCounter=0;
	SetMinCommonLifetime(UINT_MAX);
}


emTmpFileMaster::~emTmpFileMaster()
{
	if (!DirPath.IsEmpty()) {
		try {
			emTryRemoveFileOrTree(DirPath,true);
		}
		catch (const emException & exception) {
			emFatalError(
				"Failed to remove temporary file or directory:\n%s",
				exception.GetText().Get()
			);
		}
	}
}


emString emTmpFileMaster::GetCommonPath()
{
	emArray<char> idBuf;
	emString hostName,userName,hash;

	// It must be specific to host and user.

	hostName=emGetHostName();
	userName=emGetUserName();
	idBuf.SetTuningLevel(4);
	idBuf.Add(hostName.Get(),hostName.GetLen()+1);
	idBuf.Add(userName.Get(),userName.GetLen());
	hash=emCalcHashName(idBuf.Get(),idBuf.GetCount(),20);
	return emGetInstallPath(
		EM_IDT_TMP,
		"emCore",
		emString::Format("emTmp-%s",hash.Get())
	);
}


void emTmpFileMaster::TryDeleteDeadDirectories()
{
	const char * args[10];
	emString commonPath;
	emArray<emString> list;
	emString nm,srv,errors;
	int i,l1,l2,errCount;
	bool srvOkay;

	commonPath=GetCommonPath();
	try {
		list=emTryLoadDir(commonPath);
	}
	catch (const emException &) {
		return;
	}

	for (i=0, errCount=0; i<list.GetCount(); i++) {
		nm=list[i];
		l1=strlen(DirNameEnding);
		l2=nm.GetLen();
		if (l2<=l1 || strcmp(DirNameEnding,nm.Get()+l2-l1)!=0) continue;
		srv=nm.GetSubString(0,l2-l1);
		args[0]="ping";
		srvOkay=true;
		try {
			emMiniIpcClient::TrySend(srv,1,args);
		}
		catch (const emException &) {
			srvOkay=false;
		}
		if (srvOkay) continue;
		try {
			emTryRemoveFileOrTree(emGetChildPath(commonPath,nm),true);
		}
		catch (const emException & exception) {
			if (!errors.IsEmpty()) errors+='\n';
			errors+=exception.GetText();
			errCount++;
		}
	}

	if (errCount>=2) {
		throw emException(
			"Failed to remove old temporary directories:\n%s",
			errors.Get()
		);
	}
}


void emTmpFileMaster::TryStartOwnDirectory()
{
	int i;

	for (i=1; ; i++) {
		TryDeleteDeadDirectories();
		IpcServer.StartServing();
		DirPath=emGetChildPath(
			GetCommonPath(),
			IpcServer.GetServerName()+DirNameEnding
		);
		if (!emIsExistingPath(DirPath)) break;
		DirPath.Clear();
		IpcServer.StopServing();
		if (i>=3) {
			emFatalError("emTmpFileMaster::TryStartOwnDirectory: giving up");
		}
		emWarning("emTmpFileMaster::TryStartOwnDirectory: retry #%d",i);
		emSleepMS(500);
	}

	try {
		emTryMakeDirectories(DirPath,0700);
	}
	catch (const emException & exception) {
		DirPath.Clear();
		IpcServer.StopServing();
		throw emException("emTmpFileMaster: %s",exception.GetText().Get());
	}
}


emTmpFileMaster::IpcServerClass::IpcServerClass(emScheduler & scheduler)
	: emMiniIpcServer(scheduler)
{
}


void emTmpFileMaster::IpcServerClass::OnReception(
	int argc, const char * const argv[]
)
{
}


const char * emTmpFileMaster::DirNameEnding=".autoremoved";
