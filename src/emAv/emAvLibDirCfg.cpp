//------------------------------------------------------------------------------
// emAvLibDirCfg.cpp
//
// Copyright (C) 2020 Oliver Hamann.
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

#include <emAv/emAvLibDirCfg.h>
#include <emCore/emInstallInfo.h>
#if defined(_WIN32)
#	include <windows.h>
#endif


emAvLibDirCfg::emAvLibDirCfg(const emString & serverProcPath)
	: LibDirNecessary(false),
	LibDirValid(false)
{
#	if defined(_WIN32)
		LibDirNecessary=
			CheckProcName(serverProcPath,"emAvServerProc_vlc") &&
			IsMatchingBinary(serverProcPath+".exe")
		;
#	endif

	if (LibDirNecessary) {
		LoadConfigFile();
		LibDirValid=CheckLibDir(LibDir,&LibDirError);
	}
}


emAvLibDirCfg::~emAvLibDirCfg()
{
}


void emAvLibDirCfg::SetSaveAndSignalLibDir(const emString & libDir, emScheduler & scheduler)
{
	LibDir=libDir;
	SaveConfigFile();
	LibDirValid=CheckLibDir(LibDir,&LibDirError);
	ChangeSignal.Signal(scheduler);
}


emArray<emString> emAvLibDirCfg::GetExtraEnv() const
{
	if (!LibDirNecessary || !LibDirValid || LibDir.IsEmpty()) {
		return emArray<emString>();
	}

#	if defined(_WIN32)
		static const char * name="PATH";
		static char sep=';';
#	elif defined(__APPLE__)
		static const char * name="DYLD_LIBRARY_PATH";
		static char sep=':';
#	else
		static const char * name="LD_LIBRARY_PATH";
		static char sep=':';
#	endif

	emString str=emString(name)+'='+LibDir;
	const char * p=getenv(name);
	if (p && *p) {
		str+=sep;
		str+=p;
	}

	return emArray<emString>(str);
}


emPanel * emAvLibDirCfg::CreateFilePanelElement(emPanel * parent, const emString & name)
{
	if (!LibDirNecessary) return NULL;
	emLinearLayout * ll = new emLinearLayout(parent,name);
	ll->SetOuterSpace(0.3,0.3);
	ll->SetChildTallness(0.57);
	new CfgPanel(ll,"cfg",*this);
	return ll;
}


emPanel * emAvLibDirCfg::CreateControlPanelElement(emPanel * parent, const emString & name)
{
	if (!LibDirNecessary) return NULL;
	CfgPanel * p=new CfgPanel(parent,name,*this);
	p->SetOuterBorderType(emBorder::OBT_INSTRUMENT);
	return p;
}


void emAvLibDirCfg::LoadConfigFile()
{
	emString path;
	emArray<char> array;
	const char * p, * e;

	path=emGetInstallPath(EM_IDT_USER_CONFIG,"emAv","libdir.cfg");
	if (emIsExistingPath(path)) {
		try {
			array=emTryLoadFile(path);
		}
		catch (const emException & exception) {
			emFatalError("%s",exception.GetText().Get());
		}
	}

	p=array.Get();
	e=array.Get()+array.GetCount();
	while (p<e && (unsigned char)p[0]<=32) p++;
	while (p<e && (unsigned char)e[-1]<=32) e--;

	LibDir=emString(p,e-p);
}


void emAvLibDirCfg::SaveConfigFile() const
{
	emString path;

	path=emGetInstallPath(EM_IDT_USER_CONFIG,"emAv","libdir.cfg");
	try {
		emTrySaveFile(path,LibDir.Get(),LibDir.GetCount());
	}
	catch (const emException & exception) {
		emFatalError("%s",exception.GetText().Get());
	}
}


bool emAvLibDirCfg::CheckLibDir(const char * libDir, emString * pErr)
{
	emString dllPath,regDir,regVer;

	if (!*libDir) {
		if (pErr) *pErr="VLC directory not set";
		return false;
	}

	if (!emIsDirectory(libDir)) {
		if (pErr) *pErr="VLC directory not found";
		return false;
	}

	dllPath=emGetChildPath(libDir,"libvlc.dll");
	if (!emIsRegularFile(dllPath)) {
		if (pErr) *pErr="libvlc.dll not found in VLC directory";
		return false;
	}
	if (!emIsReadablePath(dllPath)) {
		if (pErr) *pErr="libvlc.dll not readable in VLC directory";
		return false;
	}
	if (!IsMatchingBinary(dllPath,pErr)) {
		return false;
	}

	dllPath=emGetChildPath(libDir,"libvlccore.dll");
	if (!emIsRegularFile(dllPath)) {
		if (pErr) *pErr="libvlccore.dll not found in VLC directory";
		return false;
	}
	if (!emIsReadablePath(dllPath)) {
		if (pErr) *pErr="libvlccore.dll not readable in VLC directory";
		return false;
	}
	if (!IsMatchingBinary(dllPath,pErr)) {
		return false;
	}

	if (GetVlcInfoFromRegistry(&regDir,&regVer)) {
		if (strcasecmp(regDir.Get(),libDir)==0) {
			int hi,med,lo,n;
			n=sscanf(regVer.Get(),"%d.%d.%d",&hi,&med,&lo);
			if (n<3) lo=0;
			if (n<2) med=0;
			if (n<1) hi=0;
			if (
				hi!=RequiredVlcVersion ||
				med<MinRequiredVlcSubVersion ||
				med>MaxRequiredVlcSubVersion
			) {
				if (pErr) *pErr="VLC found but version not matching";
				return false;
			}
		}
	}

	if (pErr) pErr->Clear();
	return true;
}


bool emAvLibDirCfg::CheckProcName(const emString & serverProcPath, const char * name)
{
	const char * n=emGetNameInPath(serverProcPath);
	const char * e=emGetExtensionInPath(n);
	emString procName(n,e-n);
	return strcasecmp(procName.Get(),name)==0;
}


bool emAvLibDirCfg::IsMatchingBinary(const char * filePath, emString * pErr)
{
	int arch;

	try {
		arch=TryGetWinBinArch(filePath);
	}
	catch (const emException & e) {
		if (pErr) *pErr=e.GetText();
		return false;
	}

	if (arch!=RequiredVlcArch) {
		if (pErr) {
			*pErr=emString::Format(
				"%s is not %s",
				emGetNameInPath(filePath),
				RequiredVlcArchString
			);
		}
		return false;
	}

	if (pErr) pErr->Clear();
	return true;
}


int emAvLibDirCfg::TryGetWinBinArch(const char * filePath)
{
	FILE * f;
	int i;

	f=fopen(filePath,"rb");
	if (!f) goto L_Err;
	if (fseek(f,60,SEEK_SET)!=0) goto L_Err;
	i=(unsigned char)fgetc(f);
	i|=((unsigned char)fgetc(f))<<8;
	i|=((unsigned char)fgetc(f))<<16;
	i|=((unsigned char)fgetc(f))<<24;
	if (fseek(f,i,SEEK_SET)!=0) goto L_FormatErr;
	if ((unsigned char)fgetc(f)!=0x50) goto L_FormatErr;
	if ((unsigned char)fgetc(f)!=0x45) goto L_FormatErr;
	if ((unsigned char)fgetc(f)!=0x00) goto L_FormatErr;
	if ((unsigned char)fgetc(f)!=0x00) goto L_FormatErr;
	i=(unsigned char)fgetc(f);
	i|=((unsigned char)fgetc(f))<<8;
	fclose(f);
	return i;
L_FormatErr:
	if (f) fclose(f);
	throw emException(
		"Cannot read %s: unexpected binary format",
		emGetNameInPath(filePath)
	);
L_Err:
	if (f) fclose(f);
	throw emException(
		"Cannot read %s: %s",
		emGetNameInPath(filePath),
		emGetErrorText(errno).Get()
	);
}


bool emAvLibDirCfg::GetVlcInfoFromRegistry(emString * pDir, emString * pVersion)
{
#if defined(_WIN32)
	char buf[512];
	HKEY hk;
	DWORD d,sz;

	d=RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\VideoLAN\\VLC",0,KEY_QUERY_VALUE,&hk);
	if (d!=ERROR_SUCCESS) {
		return false;
	}

	if (pDir) {
		sz=sizeof(buf);
		d=RegQueryValueEx(hk,"InstallDir",NULL,NULL,(LPBYTE)buf,&sz);
		if (d!=ERROR_SUCCESS) {
			RegCloseKey(hk);
			return false;
		}
		*pDir=emString(buf,sz);
	}

	if (pVersion) {
		sz=sizeof(buf);
		d=RegQueryValueEx(hk,"Version",NULL,NULL,(LPBYTE)buf,&sz);
		if (d!=ERROR_SUCCESS) {
			RegCloseKey(hk);
			return false;
		}
		*pVersion=emString(buf,sz);
	}

	RegCloseKey(hk);
	return true;
#else
	return false;
#endif
}


emAvLibDirCfg::CfgPanel::CfgPanel(
	emPanel * parent, const emString & name, emAvLibDirCfg & cfg
)
	: emLinearGroup(parent,name,"Configure VLC Directory"),
	Cfg(cfg),
	TfDesc(NULL),
	FsBox(NULL),
	BtAutoDetect(NULL),
	TfStatus(NULL),
	BtSave(NULL),
	LibDirValid(false)
{
	AddWakeUpSignal(Cfg.GetChangeSignal());
}


emAvLibDirCfg::CfgPanel::~CfgPanel()
{
}


bool emAvLibDirCfg::CfgPanel::Cycle()
{
	emString str;

	if (IsSignaled(Cfg.GetChangeSignal())) {
		UpdateFromCfg();
	}

	if (FsBox && IsSignaled(FsBox->GetSelectionSignal())) {
		if (LibDir!=FsBox->GetParentDirectory()) {
			LibDir=FsBox->GetParentDirectory();
			LibDirValid=emAvLibDirCfg::CheckLibDir(LibDir,&LibDirError);
			UpdateStatusLabel();
		}
	}

	if (BtAutoDetect && IsSignaled(BtAutoDetect->GetClickSignal())) {
		if (emAvLibDirCfg::GetVlcInfoFromRegistry(&str)) {
			LibDir=str;
			LibDirValid=emAvLibDirCfg::CheckLibDir(LibDir,&LibDirError);
			if (FsBox) {
				FsBox->SetParentDirectory(LibDir);
				FsBox->ClearSelection();
			}
			UpdateStatusLabel();
		}
		else {
			UpdateStatusLabel(true);
		}
	}

	if (BtSave && IsSignaled(BtSave->GetClickSignal())) {
		Cfg.SetSaveAndSignalLibDir(LibDir,GetScheduler());
	}

	return emLinearGroup::Cycle();
}


void emAvLibDirCfg::CfgPanel::AutoExpand()
{
	emLinearGroup::AutoExpand();

	SetSpace(0.01,0.05,0.01,0.1);
	SetChildWeight(0,1.5);
	SetChildWeight(1,2.1);

	TfDesc=new emLabel(this,"desc",emString::Format(
		"For the audio/video playback to work, the VLC media player %s %s\n"
		"must be installed, and then its installation directory must be set here,\n"
		"so that Eagle Mode can find and use the VLC libraries and plugins (you may\n"
		"try the Auto-Detect button). If you are going to download and install the\n"
		"VLC media player now, please remember that it must be the %s variant!",
		emAvLibDirCfg::RequiredVlcVersionString,
		emAvLibDirCfg::RequiredVlcArchString,
		emAvLibDirCfg::RequiredVlcArchString
	));

	emLinearLayout * ll=new emLinearLayout(this,"dir");
	ll->SetOrientationThresholdTallness(1.0);
	ll->SetSpace(0.0,0.0,0.05,0.2);
	ll->SetChildTallness(1,0.7);

	FsBox=new emFileSelectionBox(ll,"fsb","VLC Directory");
	FsBox->SetNameFieldHidden();
	FsBox->SetFilterHidden();
	FsBox->SetBorderScaling(1.65);
	AddWakeUpSignal(FsBox->GetSelectionSignal());

	BtAutoDetect=new emButton(ll,"autodetect","Auto\nDetect");
	BtAutoDetect->SetCaptionAlignment(EM_ALIGN_CENTER);
	BtAutoDetect->SetBorderScaling(0.5);
	BtAutoDetect->SetNoEOI();
	AddWakeUpSignal(BtAutoDetect->GetClickSignal());

	TfStatus=new emTextField(this,"status","Status");
	TfStatus->SetMultiLineMode();

	BtSave=new emButton(this,"save","Save and Use VLC Directory");
	AddWakeUpSignal(BtSave->GetClickSignal());

	UpdateFromCfg();
}


void emAvLibDirCfg::CfgPanel::AutoShrink()
{
	TfDesc=NULL;
	FsBox=NULL;
	BtAutoDetect=NULL;
	TfStatus=NULL;
	BtSave=NULL;
	emLinearGroup::AutoShrink();
}


void emAvLibDirCfg::CfgPanel::UpdateFromCfg()
{
	LibDirValid=Cfg.LibDirValid;
	LibDirError=Cfg.LibDirError;
	LibDir=Cfg.LibDir;

	if (LibDir.IsEmpty()) {
		LibDir=emGetCurrentDirectory();
#		if defined(_WIN32)
			const char * p=getenv("ProgramFiles");
			if (p) {
				LibDir=p;
			}
#		endif
		// Don't call CheckLibDir(..) here, let the error remain "... not set".
	}

	if (FsBox) {
		FsBox->SetParentDirectory(LibDir);
		FsBox->ClearSelection();
	}

	UpdateStatusLabel();
}


void emAvLibDirCfg::CfgPanel::UpdateStatusLabel(bool autoDetectFailed)
{
	emString msg;
	emColor color;
	emLook look;

	if (!TfStatus) return;

	if (autoDetectFailed) {
		msg="Auto-detect failed to find VLC";
		color=emColor(255,0,128);
	}
	else if (!LibDirValid) {
		msg=LibDirError;
		color=emColor(255,0,0);
	}
	else if (LibDir==Cfg.LibDir) {
		msg="VLC found and used";
		color=GetLook().GetOutputFgColor();
	}
	else {
		msg="VLC found - press Save to use it";
		color=GetLook().GetOutputFgColor();
		color=emColor(0,255,0);
	}

	look=TfStatus->GetLook();
	look.SetOutputFgColor(color);
	TfStatus->SetLook(look);
	TfStatus->SetText(msg);
}

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
	const int emAvLibDirCfg::RequiredVlcArch          = 0x8664;
	const char * emAvLibDirCfg::RequiredVlcArchString = "64-bit";
#else
	const int emAvLibDirCfg::RequiredVlcArch          = 0x14c;
	const char * emAvLibDirCfg::RequiredVlcArchString = "32-bit";
#endif

const int emAvLibDirCfg::RequiredVlcVersion           = 3;
const int emAvLibDirCfg::MinRequiredVlcSubVersion     = 0;
const int emAvLibDirCfg::MaxRequiredVlcSubVersion     = 0;
const char * emAvLibDirCfg::RequiredVlcVersionString  = "3.0.x";
