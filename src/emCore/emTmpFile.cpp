//------------------------------------------------------------------------------
// emTmpFile.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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


void emTmpFile::Setup(emRootContext & rootContext, const char * postfix)
{
	emRef<emTmpFileMaster> m;

	Discard();
	m=emTmpFileMaster::Acquire(rootContext);
	Path=m->InventPath(postfix);
}


void emTmpFile::SetupCustomPath(const emString & customPath)
{
	Discard();
	Path=customPath;
}


void emTmpFile::Discard()
{
	if (!Path.IsEmpty()) {
		try {
			emTryRemoveFileOrTree(Path,true);
		}
		catch (emString) {
		}
		Path.Empty();
	}
}


//==============================================================================
//============================== emTmpFileMaster ===============================
//==============================================================================

emRef<emTmpFileMaster> emTmpFileMaster::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emTmpFileMaster,rootContext,"")
}


emString emTmpFileMaster::InventPath(const char * postfix)
{
	emString name,path;

	if (DirPath.IsEmpty()) {
		StartOwnDirectory();
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
	DeleteDeadDirectories();
}


emTmpFileMaster::~emTmpFileMaster()
{
	if (!DirPath.IsEmpty()) {
		try {
			emTryRemoveFileOrTree(DirPath,true);
		}
		catch (emString errorMessage) {
			emFatalError("emTmpFileMaster: %s",errorMessage.Get());
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


void emTmpFileMaster::DeleteDeadDirectories()
{
	const char * args[10];
	emString commonPath;
	emArray<emString> list;
	emString nm,srv;
	int i,l1,l2;
	bool srvOkay;

	commonPath=GetCommonPath();
	try {
		list=emTryLoadDir(commonPath);
	}
	catch (emString) {
		return;
	}

	for (i=0; i<list.GetCount(); i++) {
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
		catch (emString) {
			srvOkay=false;
		}
		if (srvOkay) continue;
		try {
			emTryRemoveFileOrTree(emGetChildPath(commonPath,nm),true);
		}
		catch (emString) {
		}
	}
}


void emTmpFileMaster::StartOwnDirectory()
{
	int i;

	for (i=1; ; i++) {
		IpcServer.StartServing();
		DirPath=emGetChildPath(
			GetCommonPath(),
			IpcServer.GetServerName()+DirNameEnding
		);
		if (!emIsExistingPath(DirPath)) break;
		if (i>=3) {
			emFatalError("emTmpFileMaster::StartOwnDirectory: giving up");
		}
		emWarning("emTmpFileMaster::StartOwnDirectory: retry #%d",i);
		IpcServer.StopServing();
		emSleepMS(500);
		DeleteDeadDirectories();
	}
	try {
		emTryMakeDirectories(DirPath,0700);
	}
	catch (emString errorMessage) {
		emFatalError("emTmpFileMaster: %s",errorMessage.Get());
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
