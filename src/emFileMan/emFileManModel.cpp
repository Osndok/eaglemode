//------------------------------------------------------------------------------
// emFileManModel.cpp
//
// Copyright (C) 2004-2009,2011 Oliver Hamann.
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
#endif
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emCore/emProcess.h>
#include <emCore/emClipboard.h>
#include <emCore/emToolkit.h>
#include <emFileMan/emFileManModel.h>
#include <emFileMan/emFileManViewConfig.h>


emRef<emFileManModel> emFileManModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emFileManModel,rootContext,"")
}


void emFileManModel::ClearSourceSelection()
{
	if (!Sel[0].IsEmpty()) {
		Sel[0].Empty();
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::ClearTargetSelection()
{
	if (!Sel[1].IsEmpty()) {
		Sel[1].Empty();
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::SelectAsSource(const emString & path)
{
	int i,hc;

	hc=emCalcHashCode(path);
	i=SearchSelection(Sel[0],hc,path);
	if (i<0) {
		i=~i;
		Sel[0].InsertNew(i);
		Sel[0].GetWritable(i).HashCode=hc;
		Sel[0].GetWritable(i).Path=path;
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::SelectAsTarget(const emString & path)
{
	int i,hc;

	hc=emCalcHashCode(path);
	i=SearchSelection(Sel[1],hc,path);
	if (i<0) {
		i=~i;
		Sel[1].InsertNew(i);
		Sel[1].GetWritable(i).HashCode=hc;
		Sel[1].GetWritable(i).Path=path;
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::DeselectAsSource(const char * path)
{
	int i;

	i=SearchSelection(Sel[0],emCalcHashCode(path),path);
	if (i>=0) {
		Sel[0].Remove(i);
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::DeselectAsTarget(const char * path)
{
	int i;

	i=SearchSelection(Sel[1],emCalcHashCode(path),path);
	if (i>=0) {
		Sel[1].Remove(i);
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


bool emFileManModel::IsAnySelectionInDirTree(const char * dirPath) const
{
	const SelEntry * psel, * pend;
	const char * path;
	char separator;
	int s,len,len2;

	separator=emGetChildPath("a","b").Get(1);
	len=strlen(dirPath);
	if (len>0 && dirPath[len-1]==separator) len--;
	for (s=0; s<2; s++) {
		psel=Sel[s].Get();
		pend=psel+Sel[s].GetCount();
		while (psel<pend) {
			path=psel->Path;
			len2=strlen(path);
			if (
				len2>=len &&
				(len2==len || path[len]==separator) &&
				memcmp(dirPath,path,len)==0
			) {
				return true;
			}
			psel++;
		}
	}
	return false;
}


void emFileManModel::SetShiftTgtSelPath(const emString & path)
{
	ShiftTgtSelPath=path;
}


void emFileManModel::SwapSelection()
{
	emArray<SelEntry> t;

	if (!Sel[0].IsEmpty() || !Sel[1].IsEmpty()) {
		t=Sel[0];
		Sel[0]=Sel[1];
		Sel[1]=t;
		Signal(SelectionSignal);
	}
	SetShiftTgtSelPath(emString());
	SelCmdCounter++;
}


void emFileManModel::UpdateSelection()
{
	int s,i;

	for (s=0; s<2; s++) {
		for (i=0; i<Sel[s].GetCount(); ) {
			if (emIsExistingPath(Sel[s][i].Path)) {
				i++;
			}
			else {
				if (ShiftTgtSelPath==Sel[s][i].Path) {
					SetShiftTgtSelPath(emString());
				}
				Sel[s].Remove(i);
				Signal(SelectionSignal);
				SelCmdCounter++;
			}
		}
	}
}


void emFileManModel::SelectionToClipboard(
	emView & contentView, bool source, bool namesOnly
)
{
	emRef<emClipboard> clipboard;
	emArray<emDirEntry> entries;
	emArray<char> text;
	emString str;
	int i;

	clipboard=emClipboard::LookupInherited(contentView);
	if (!clipboard) {
		emTkDialog::ShowMessage(contentView,"Error","No clipboard available.");
		return;
	}
	if (source) entries=CreateSortedSrcSelDirEntries(contentView);
	else entries=CreateSortedTgtSelDirEntries(contentView);
	text.SetTuningLevel(4);
	for (i=0; i<entries.GetCount(); i++) {
		if (namesOnly) str=entries[i].GetName();
		else str=entries[i].GetPath();
		if (i>0) text.Add('\n');
		text.Add(str.Get(),str.GetLen());
	}
	str=emString(text,text.GetCount());
	clipboard->PutText(str);
	clipboard->PutText(str,true);
}


emFileManModel::CommandNode::CommandNode()
{
	Type=CT_GROUP;
	Order=0.0;
	BorderScaling=1.0;
	PrefChildTallness=0.2;
	Children.SetTuningLevel(4);
	DirCRC=0;
}


emFileManModel::CommandNode::~CommandNode()
{
}


const emFileManModel::CommandNode * emFileManModel::GetCommand(
	const emString & cmdPath
) const
{
	int i;

	i=SearchCommand(emCalcHashCode(cmdPath),cmdPath);
	if (i>=0) return Cmds[i].Node;
	else return NULL;
}


const emFileManModel::CommandNode * emFileManModel::SearchDefaultCommandFor(
	const emString & filePath
) const
{
	return SearchDefaultCommandFor(CmdRoot,filePath);
}


const emFileManModel::CommandNode * emFileManModel::SearchHotkeyCommand(
	const emInputHotkey & hotkey
) const
{
	return SearchHotkeyCommand(CmdRoot,hotkey);
}


void emFileManModel::RunCommand(const CommandNode * cmd, emView & contentView)
{
	emArray<emDirEntry> src,tgt;
	emArray<emString> args,extraEnv;
	emWindow * wnd;
	emScreen * screen;
	emString commandRunId;
	emString str;
	double l,t,r,b;
	int i,scnt,tcnt,winX,winY,winW,winH;

	if (!cmd || cmd->Type!=CT_COMMAND) return;

	screen=contentView.GetScreen();
	if (screen) screen->LeaveFullscreenModes();

	SelCmdCounter++;
	commandRunId=GetCommandRunId();

	wnd=contentView.GetWindow();
	if (wnd) {
		wnd->GetBorderSizes(&l,&t,&r,&b);
		winX=(int)(wnd->GetHomeX()-l+0.5);
		winY=(int)(wnd->GetHomeY()-t+0.5);
		winW=(int)(wnd->GetHomeWidth()+l+r+0.5);
		winH=(int)(wnd->GetHomeHeight()+t+b+0.5);
	}
	else {
		winX=0;
		winY=0;
		winW=800;
		winH=600;
	}

	extraEnv.Add(emString::Format(
		"EM_FM_SERVER_NAME=%s",
		GetMiniIpcServerName().Get()
	));
	extraEnv.Add(emString::Format(
		"EM_COMMAND_RUN_ID=%s",
		commandRunId.Get()
	));
	extraEnv.Add(emString::Format("EM_X=%d",winX));
	extraEnv.Add(emString::Format("EM_Y=%d",winY));
	extraEnv.Add(emString::Format("EM_WIDTH=%d",winW));
	extraEnv.Add(emString::Format("EM_HEIGHT=%d",winH));

#if defined(_WIN32)
	extraEnv.Add(emString::Format("EM_ACP=%u",::GetACP()));
#endif

	// Prepare arguments:
	//  [<interpreter>] <path> <src-count> <tgt-count> <src>... <tgt>... NULL
	src=CreateSortedSrcSelDirEntries(contentView);
	tgt=CreateSortedTgtSelDirEntries(contentView);
	scnt=src.GetCount();
	tcnt=tgt.GetCount();
	if (!cmd->Interpreter.IsEmpty()) args.Add(cmd->Interpreter);
	args.Add(cmd->CmdPath);
	args.Add(emString::Format("%d",scnt));
	args.Add(emString::Format("%d",tcnt));
	for (i=0; i<scnt; i++) args.Add(src[i].GetPath());
	for (i=0; i<tcnt; i++) args.Add(tgt[i].GetPath());

#if defined(_WIN32)
	for (i=0; i<args.GetCount(); i++) {
		if (strchr(args[i].Get(),'?') || strchr(args[i].Get(),'*')) {
			emTkDialog::ShowMessage(
				contentView,"Error",
				"A selected path contains a wild card character (? or *).\n"
				"This cannot be handled safely."
			);
			return;
		}
	}
#endif

	try {
		emProcess::TryStartUnmanaged(args,extraEnv);
	}
	catch (emString errorMessage) {
		emTkDialog::ShowMessage(contentView,"Error",errorMessage);
	}
}


void emFileManModel::HotkeyInput(
	emView & contentView, emInputEvent & event, const emInputState & state
)
{
	const CommandNode * cmd;
	emRef<emFileManViewConfig> viewConfig;

	switch (event.GetKey()) {
	case EM_KEY_C:
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_CLASS);
			event.Eat();
		}
		break;
	case EM_KEY_D:
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_DATE);
			event.Eat();
		}
		break;
	case EM_KEY_E:
		if (state.IsAltMod()) {
			ClearSourceSelection();
			ClearTargetSelection();
			event.Eat();
		}
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_ENDING);
			event.Eat();
		}
		break;
	case EM_KEY_H:
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetShowHiddenFiles(!viewConfig->GetShowHiddenFiles());
			event.Eat();
		}
		break;
	case EM_KEY_N:
		if (state.IsAltMod()) {
			SelectionToClipboard(contentView,false,true);
			event.Eat();
		}
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_NAME);
			event.Eat();
		}
		break;
	case EM_KEY_P:
		if (state.IsAltMod()) {
			SelectionToClipboard(contentView,false,false);
			event.Eat();
		}
		break;
	case EM_KEY_S:
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_SIZE);
			event.Eat();
		}
		break;
	case EM_KEY_V:
		if (state.IsShiftAltMod()) {
			viewConfig=emFileManViewConfig::Acquire(contentView);
			viewConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_VERSION);
			event.Eat();
		}
		break;
	case EM_KEY_Z:
		if (state.IsAltMod()) {
			SwapSelection();
			event.Eat();
		}
		break;
	default:
		break;
	}
	if (event.IsKeyboardEvent()) {
		cmd=SearchHotkeyCommand(emInputHotkey(event,state));
		if (cmd) {
			RunCommand(cmd,contentView);
			event.Eat();
		}
	}
}


emFileManModel::emFileManModel(emContext & context, const emString & name)
	: emModel(context,name)
{
	SetMinCommonLifetime(UINT_MAX);
	Sel[0].SetTuningLevel(1);
	Sel[1].SetTuningLevel(1);
	SelCmdCounter=0;
	IpcServer=new IpcServerClass(*this);
	FileUpdateSignalModel=emFileModel::AcquireUpdateSignalModel(GetRootContext());
	AddWakeUpSignal(FileUpdateSignalModel->Sig);
	LoadCommands(emGetConfigDirOverloadable(GetRootContext(),"emFileMan","Commands"));
}


emFileManModel::~emFileManModel()
{
	delete IpcServer;
	ClearCommands();
}


bool emFileManModel::Cycle()
{
	if (IsSignaled(FileUpdateSignalModel->Sig)) {
		UpdateSelection();
		UpdateCommands();
	}
	return false;
}


emFileManModel::IpcServerClass::IpcServerClass(emFileManModel & fmModel)
	: emMiniIpcServer(fmModel.GetScheduler()), FmModel(fmModel)
{
	StartServing();
}


void emFileManModel::IpcServerClass::OnReception(
	int argc, const char * const argv[]
)
{
	FmModel.OnIpcReception(argc,argv);
}


void emFileManModel::OnIpcReception(int argc, const char * const argv[])
{
	emString str;
	int i;

	if (argc==1 && strcmp(argv[0],"update")==0) {
		Signal(FileUpdateSignalModel->Sig);
	}
	else if (argc>=2 && strcmp(argv[0],"select")==0) {
		if (GetCommandRunId()==argv[1]) {
			SwapSelection();
			ClearTargetSelection();
			for (i=2; i<argc; i++) {
				DeselectAsSource(argv[i]);
				SelectAsTarget(argv[i]);
			}
		}
		Signal(FileUpdateSignalModel->Sig);
	}
	else if (argc>=2 && strcmp(argv[0],"selectks")==0) {
		if (GetCommandRunId()==argv[1]) {
			ClearTargetSelection();
			for (i=2; i<argc; i++) {
				DeselectAsSource(argv[i]);
				SelectAsTarget(argv[i]);
			}
		}
		Signal(FileUpdateSignalModel->Sig);
	}
	else if (argc>=2 && strcmp(argv[0],"selectcs")==0) {
		if (GetCommandRunId()==argv[1]) {
			ClearSourceSelection();
			ClearTargetSelection();
			for (i=2; i<argc; i++) SelectAsTarget(argv[i]);
		}
		Signal(FileUpdateSignalModel->Sig);
	}
	else {
		str.Empty();
		for (i=0; i<argc; i++) { str+=" "; str+=argv[i]; }
		emWarning("emFileManModel: Illegal MiniIpc request:%s",str.Get());
	}
}


int emFileManModel::SearchSelection(
	const emArray<SelEntry> & sel, int hashCode, const char * path
)
{
	int i,i1,i2,d;
	const SelEntry * arr;

	i2=sel.GetCount();
	arr=sel;
	if (i2) {
		i1=0;
		for (;;) {
			i=(i1+i2)>>1;
			if (arr[i].HashCode>hashCode) {
				i2=i;
				if (i1<i2) continue;
				break;
			}
			if (arr[i].HashCode<hashCode) {
				i1=i+1;
				if (i1<i2) continue;
				break;
			}
			d=strcmp(arr[i].Path.Get(),path);
			if (d>0) {
				i2=i;
				if (i1<i2) continue;
				break;
			}
			if (d<0) {
				i1=i+1;
				if (i1<i2) continue;
				break;
			}
			return i;
		}
	}
	return ~i2;
}


emArray<emDirEntry> emFileManModel::CreateSortedSelDirEntries(
	emView & contentView, const emArray<SelEntry> & sel
) const
{
	emArray<emDirEntry> res;
	emRef<emFileManViewConfig> vc;
	int i;

	res.SetCount(sel.GetCount());
	res.SetTuningLevel(1);
	for (i=0; i<sel.GetCount(); i++) {
		res.GetWritable(i)=emDirEntry(sel[i].Path);
	}

	vc=emFileManViewConfig::Acquire(contentView);
	res.Sort(CmpDEs,vc.Get());
	return res;
}


int emFileManModel::CmpDEs(
	const emDirEntry * de1, const emDirEntry * de2,
	void * context
)
{
	emFileManViewConfig * vc;

	vc=(emFileManViewConfig*)context;
	return vc->CompareDirEntries(*de1,*de2);
}


void emFileManModel::UpdateCommands()
{
	emString rootDir;

	rootDir=emGetConfigDirOverloadable(GetRootContext(),"emFileMan","Commands");
	if (CmdRoot->Dir!=rootDir || !CheckCRCs(CmdRoot)) {
		emDLog("emFileManModel: Reloading commands.");
		LoadCommands(rootDir);
		Signal(CommandsSignal);
	}
}


bool emFileManModel::CheckCRCs(const CommandNode * parent)
{
	emArray<emString> names;
	const CommandNode * child;
	int i;

	try {
		names=emTryLoadDir(parent->Dir);
	}
	catch (emString) {
	}
	names.Sort(emStdComparer<emString>::Compare);
	if (parent->DirCRC!=CalcDirCRC(parent->Dir,names)) {
		return false;
	}
	for (i=0; i<parent->Children.GetCount(); i++) {
		child=parent->Children[i];
		if (child->Type==CT_GROUP) {
			if (!CheckCRCs(child)) return false;
		}
	}
	return true;
}


emUInt64 emFileManModel::CalcDirCRC(
	const emString & dir, const emArray<emString> & names
)
{
	emString name;
	emUInt64 crc;
	time_t t;
	int i;

	crc=0;
	for (i=0; i<names.GetCount(); i++) {
		name=names[i];
		try {
			t=emTryGetFileTime(emGetChildPath(dir,name));
		}
		catch (emString) {
			t=0;
		}
		crc=emCalcCRC64(name.Get(),name.GetLen(),crc);
		crc=emCalcCRC64((char*)&t,sizeof(t),crc);
	}
	return crc;
}


void emFileManModel::ClearCommands()
{
	int i;

	for (i=0; i<Cmds.GetCount(); i++) {
		delete Cmds[i].Node;
	}
	CmdRoot=NULL;
	Cmds.Empty();
}


void emFileManModel::LoadCommands(const emString & rootDir)
{
	ClearCommands();
	CmdRoot=new CommandNode;
	CmdRoot->Type=CT_GROUP;
	CmdRoot->Dir=rootDir;
	CmdRoot->Caption="Commands";
	Cmds.AddNew();
	Cmds.GetWritable(0).HashCode=emCalcHashCode(CmdRoot->CmdPath);
	Cmds.GetWritable(0).Node=CmdRoot;
	LoadChildCommands(CmdRoot);
}


void emFileManModel::LoadChildCommands(CommandNode * parent)
{
	emArray<emString> names;
	emString path;
	int i,l;

	try {
		names=emTryLoadDir(parent->Dir);
	}
	catch (emString errorMessage) {
		emWarning("%s",errorMessage.Get());
	}
	names.Sort(emStdComparer<emString>::Compare);
	for (i=0; i<names.GetCount(); i++) {
		path=emGetChildPath(parent->Dir,names[i]);
		if (!emIsRegularFile(path)) continue;
		if ((l=names[i].GetLen())>0 && names[i][l-1]=='~') continue;
		LoadCommand(parent,path);
	}
	parent->DirCRC=CalcDirCRC(parent->Dir,names);
	parent->Children.Sort(CompareCmds);
}


void emFileManModel::LoadCommand(CommandNode * parent, const emString & cmdPath)
{
	char * buf, * p, * name;
	const char * miss;
	CommandNode * cmd;
	FILE * file;
	emColor col;
	bool beginFound, endFound, typeFound, orderFound, interpreterFound, dirFound;
	bool defaultForFound, iconFound, bgColorFound, fgColorFound;
	bool buttonBgColorFound, buttonFgColorFound, hotkeyFound;
	int bufsize,i,hashCode,insertIndex,line;

	if (Cmds.GetCount()>=10000) {
		emFatalError("Too many file manager commands, or a cycle in the command tree.");
	}

	hashCode=emCalcHashCode(cmdPath);
	insertIndex=SearchCommand(hashCode,cmdPath);
	if (insertIndex>=0) {
		parent->Children.Add(Cmds[insertIndex].Node);
		return;
	}
	insertIndex=~insertIndex;

	cmd=new CommandNode;
	cmd->CmdPath=cmdPath;
	cmd->Look=parent->Look;

	buf=NULL;
	file=NULL;

	bufsize=65536;
	buf=new char[bufsize];

	file=fopen(cmdPath,"r");
	if (!file) goto L_ErrFile;

	beginFound=false;
	endFound=false;
	typeFound=false;
	orderFound=false;
	interpreterFound=false;
	dirFound=false;
	defaultForFound=false;
	iconFound=false;
	bgColorFound=false;
	fgColorFound=false;
	buttonBgColorFound=false;
	buttonFgColorFound=false;
	hotkeyFound=false;
	line=0;
	while (fgets(buf,bufsize,file)) {

		// Count lines.
		line++;

		// Trim.
		p=buf;
		while (*p && (unsigned char)*p<=32) p++;
		for (i=strlen(p); i>0 && (unsigned char)p[i-1]<=32; i--);
		p[i]=0;

		// Eat #[<whitespace>]
		if (beginFound && *p!='#' && *p!=0) goto L_ErrSyntax;
		if (*p!='#') continue;
		p++;
		while (*p && (unsigned char)*p<=32) p++;

		// Comment?
		if (*p=='#') continue;

		// Begin?
		if (!beginFound) {
			if (strcasecmp(p,"[[BEGIN PROPERTIES]]")==0) beginFound=true;
			continue;
		}

		// End?
		if (strcasecmp(p,"[[END PROPERTIES]]")==0) {
			endFound=true;
			break;
		}

		// Eat <name>[<whitespace>]=
		for (i=0; p[i]!=0 && p[i]!='='; i++);
		if (p[i]==0) goto L_ErrSyntax;
		name=p;
		p+=i+1;
		while (i>0 && (unsigned char)name[i-1]<=32) i--;
		name[i]=0;

		// Branch by name.
		if (strcasecmp(name,"Type")==0) {
			if (typeFound) goto L_ErrDoubleProperty;
			typeFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			if (strcasecmp(p,"Command")==0) cmd->Type=CT_COMMAND;
			else if (strcasecmp(p,"Group")==0) cmd->Type=CT_GROUP;
			else if (strcasecmp(p,"Separator")==0) cmd->Type=CT_SEPARATOR;
			else goto L_ErrSyntax;
		}
		else if (strcasecmp(name,"Order")==0) {
			if (orderFound) goto L_ErrDoubleProperty;
			orderFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			cmd->Order=atof(p);
		}
		else if (strcasecmp(name,"Interpreter")==0) {
			if (interpreterFound) goto L_ErrDoubleProperty;
			interpreterFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			cmd->Interpreter=p;
		}
		else if (strcasecmp(name,"Directory")==0 || strcasecmp(name,"Dir")==0) {
			if (dirFound) goto L_ErrDoubleProperty;
			dirFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			cmd->Dir=emGetAbsolutePath(emGetChildPath(emGetParentPath(cmdPath),p));
		}
		else if (strcasecmp(name,"DefaultFor")==0) {
			if (defaultForFound) goto L_ErrDoubleProperty;
			defaultForFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			cmd->DefaultFor=p;
		}
		else if (strcasecmp(name,"Caption")==0) {
			while (*p && (unsigned char)*p<=32) p++;
			if (!cmd->Caption.IsEmpty()) cmd->Caption+="\n";
			cmd->Caption+=p;
		}
		else if (strcasecmp(name,"Description")==0 || strcasecmp(name,"Descr")==0) {
			if (!cmd->Description.IsEmpty()) cmd->Description+="\n";
			cmd->Description+=p;
		}
		else if (strcasecmp(name,"Icon")==0) {
			if (iconFound) goto L_ErrDoubleProperty;
			iconFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			try {
				cmd->Icon=emTryGetInsResImage(GetRootContext(),"icons",p);
			}
			catch (emString) {
				try {
					cmd->Icon=emTryGetInsResImage(GetRootContext(),"icons","em-error-unknown-icon.tga");
				}
				catch (emString) {
				}
			}
		}
		else if (strcasecmp(name,"BgColor")==0) {
			if (bgColorFound) goto L_ErrDoubleProperty;
			bgColorFound=true;
			try {
				col.TryParse(p);
				cmd->Look.SetBgColor(col);
			}
			catch (emString) {
				emWarning("In file \"%s\": unknown color: %s",cmdPath.Get(),p);
			}
		}
		else if (strcasecmp(name,"FgColor")==0) {
			if (fgColorFound) goto L_ErrDoubleProperty;
			fgColorFound=true;
			try {
				col.TryParse(p);
				cmd->Look.SetFgColor(col);
			}
			catch (emString) {
				emWarning("In file \"%s\": unknown color: %s",cmdPath.Get(),p);
			}
		}
		else if (strcasecmp(name,"ButtonBgColor")==0) {
			if (buttonBgColorFound) goto L_ErrDoubleProperty;
			buttonBgColorFound=true;
			try {
				col.TryParse(p);
				cmd->Look.SetButtonBgColor(col);
			}
			catch (emString) {
				emWarning("In file \"%s\": unknown color: %s",cmdPath.Get(),p);
			}
		}
		else if (strcasecmp(name,"ButtonFgColor")==0) {
			if (buttonFgColorFound) goto L_ErrDoubleProperty;
			buttonFgColorFound=true;
			try {
				col.TryParse(p);
				cmd->Look.SetButtonFgColor(col);
			}
			catch (emString) {
				emWarning("In file \"%s\": unknown color: %s",cmdPath.Get(),p);
			}
		}
		else if (strcasecmp(name,"Hotkey")==0) {
			if (hotkeyFound) goto L_ErrDoubleProperty;
			hotkeyFound=true;
			while (*p && (unsigned char)*p<=32) p++;
			try {
				cmd->Hotkey.TryParse(p);
			}
			catch (emString) {
				emWarning("In file \"%s\": unknown hotkey: %s",cmdPath.Get(),p);
			}
		}
		else if (strcasecmp(name,"BorderScaling")==0) {
			while (*p && (unsigned char)*p<=32) p++;
			cmd->BorderScaling=atof(p);
		}
		else if (strcasecmp(name,"PrefChildTallness")==0) {
			while (*p && (unsigned char)*p<=32) p++;
			cmd->PrefChildTallness=atof(p);
		}
		else {
			goto L_ErrSyntax;
		}
	}

	if (ferror(file)) goto L_ErrFile;
	fclose(file);
	file=NULL;

	delete [] buf;
	buf=NULL;

	if (cmd->Hotkey.IsValid()) {
		if (!cmd->Description.IsEmpty()) cmd->Description+="\n\n";
		cmd->Description+="Hotkey: ";
		cmd->Description+=cmd->Hotkey.GetString();
	}

	if (!beginFound) {
		goto L_Err;
	}
	if (!endFound) {
		emWarning(
			"Syntax error in file \"%s\": properties not terminated.",
			cmdPath.Get()
		);
		goto L_Err;
	}
	if (!typeFound) {
		miss="Type";
		goto L_ErrMissingProperty;
	}
	if (!dirFound && cmd->Type==CT_GROUP) {
		miss="Directory";
		goto L_ErrMissingProperty;
	}

	Cmds.InsertNew(insertIndex);
	Cmds.GetWritable(insertIndex).HashCode=hashCode;
	Cmds.GetWritable(insertIndex).Node=cmd;
	parent->Children.Add(cmd);

	if (cmd->Type==CT_GROUP) {
		LoadChildCommands(cmd);
	}

	return;

L_ErrSyntax:
	emWarning(
		"Syntax error in file \"%s\" line %d.",
		cmdPath.Get(),
		line
	);
	goto L_Err;
L_ErrDoubleProperty:
	emWarning(
		"Error in file \"%s\" line %d: double property",
		cmdPath.Get(),
		line
	);
	goto L_Err;
L_ErrMissingProperty:
	emWarning(
		"Error in file \"%s\": missing property \"%s\".",
		cmdPath.Get(),
		miss
	);
	goto L_Err;
L_ErrFile:
	emWarning(
		"Failed to read file \"%s\": %s",
		cmdPath.Get(),
		emGetErrorText(errno).Get()
	);
	goto L_Err;
L_Err:
	if (file) fclose(file);
	if (buf) delete [] buf;
	if (cmd) delete cmd;
}


int emFileManModel::CompareCmds(
	const CommandNode * const * n1, const CommandNode * const * n2, void * context
)
{
	double d;

	d=(*n1)->Order-(*n2)->Order;
	if (d<0.0) return -1;
	if (d>0.0) return 1;
	return strcmp((*n1)->Caption.Get(),(*n2)->Caption.Get());
}


int emFileManModel::SearchCommand(int hashCode, const char * path) const
{
	int i,i1,i2,d;
	const CmdEntry * arr;

	i2=Cmds.GetCount();
	arr=Cmds;
	if (i2) {
		i1=0;
		for (;;) {
			i=(i1+i2)>>1;
			if (arr[i].HashCode>hashCode) {
				i2=i;
				if (i1<i2) continue;
				break;
			}
			if (arr[i].HashCode<hashCode) {
				i1=i+1;
				if (i1<i2) continue;
				break;
			}
			d=strcmp(arr[i].Node->CmdPath.Get(),path);
			if (d>0) {
				i2=i;
				if (i1<i2) continue;
				break;
			}
			if (d<0) {
				i1=i+1;
				if (i1<i2) continue;
				break;
			}
			return i;
		}
	}
	return ~i2;
}


const emFileManModel::CommandNode * emFileManModel::SearchDefaultCommandFor(
	const CommandNode * parent, const emString & filePath, int * pPriority
) const
{
	const CommandNode * cmd, * bestCmd, * subCmd;
	int i,pri,bestPri;

	bestCmd=NULL;
	bestPri=0;
	for (i=0; i<parent->Children.GetCount(); i++) {
		cmd=parent->Children[i];
		if (cmd->Type==CT_COMMAND) {
			pri=CheckDefaultCommand(cmd,filePath);
			if (pri>bestPri) {
				bestCmd=cmd;
				bestPri=pri;
			}
		}
	}
	for (i=0; i<parent->Children.GetCount(); i++) {
		cmd=parent->Children[i];
		if (cmd->Type==CT_GROUP) {
			subCmd=SearchDefaultCommandFor(cmd,filePath,&pri);
			if (pri>bestPri) {
				bestCmd=subCmd;
				bestPri=pri;
			}
		}
	}
	if (pPriority) *pPriority=bestPri;
	return bestCmd;
}


const emFileManModel::CommandNode * emFileManModel::SearchHotkeyCommand(
	const CommandNode * parent, const emInputHotkey & hotkey
) const
{
	const CommandNode * cmd;
	int i;

	if (hotkey.IsValid()) {
		for (i=0; i<parent->Children.GetCount(); i++) {
			cmd=parent->Children[i];
			if (cmd->Type==CT_COMMAND) {
				if (cmd->Hotkey==hotkey) {
					return cmd;
				}
			}
		}
		for (i=0; i<parent->Children.GetCount(); i++) {
			cmd=parent->Children[i];
			if (cmd->Type==CT_GROUP) {
				cmd=SearchHotkeyCommand(cmd,hotkey);
				if (cmd) return cmd;
			}
		}
	}
	return NULL;
}


int emFileManModel::CheckDefaultCommand(
	const CommandNode * cmd, const emString & filePath
) const
{
	const char * p;
	int len,pathlen,bestlen;

	if (cmd->Type!=CT_COMMAND) return 0;

	if (cmd->DefaultFor.GetLen()==0) return 0;

	if (cmd->DefaultFor=="file") {
		if (!emIsRegularFile(filePath)) return 0;
		return 1;
	}

	if (cmd->DefaultFor=="directory") {
		if (!emIsDirectory(filePath)) return 0;
		return 1;
	}

	if (!emIsRegularFile(filePath)) return 0;
	pathlen=filePath.GetLen();
	bestlen=0;
	for (p=cmd->DefaultFor.Get();;) {
		len=0;
		while (p[len]!=0 && p[len]!=':') len++;
		if (len>bestlen && len<=pathlen) {
			if (strncasecmp(p,filePath.Get()+pathlen-len,len)==0) bestlen=len;
		}
		p+=len;
		if (!*p) break;
		p++;
	}
	if (bestlen>0) return bestlen+1;
	return 0;
}


emString emFileManModel::GetCommandRunId() const
{
	return emString::Format("%p-%u",this,SelCmdCounter);
}
