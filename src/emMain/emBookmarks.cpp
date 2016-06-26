//------------------------------------------------------------------------------
// emBookmarks.cpp
//
// Copyright (C) 2007-2008,2011,2014-2016 Oliver Hamann.
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
#include <emCore/emRes.h>
#include <emCore/emProcess.h>
#include <emMain/emBookmarks.h>



//==============================================================================
//=============================== emBookmarksRec ===============================
//==============================================================================

emBookmarksRec::emBookmarksRec()
	: emArrayRec(&AllocateUnion)
{
}


emBookmarksRec::emBookmarksRec(emStructRec * parent, const char * varIdentifier)
	: emArrayRec(parent,varIdentifier,&AllocateUnion)
{
}


emBookmarksRec::~emBookmarksRec()
{
}


void emBookmarksRec::InsertNewBookmark(int index, emView * contentView)
{
	emPanel * p;
	emUnionRec * u;
	emBookmarkRec * bm;
	emString title,name,desc,icon;
	double x,y,a;

	if (index<0) index=0;
	if (index>GetCount()) index=GetCount();
	Insert(index);
	u=(emUnionRec*)&Get(index);
	u->SetVariant(BOOKMARK);
	if (!contentView) return;
	p=contentView->GetVisitedPanel(&x,&y,&a);
	if (!p) return;

	title=p->GetTitle();
	name=BookmarkNameFromPanelTitle(p->GetTitle());
	desc=(name!=title ? title : emString());

	icon=emBookmarksModel::GetNormalizedIconFileName(p->GetIconFileName());

	bm=(emBookmarkRec*)&u->Get();
	bm->Name=name;
	bm->Description=desc;
	bm->Icon=icon;
	bm->LocationIdentity=p->GetIdentity();
	bm->LocationRelX=x;
	bm->LocationRelY=y;
	bm->LocationRelA=a;
}


void emBookmarksRec::InsertNewGroup(int index)
{
	emUnionRec * u;

	if (index<0) index=0;
	if (index>GetCount()) index=GetCount();
	Insert(index);
	u=(emUnionRec*)&Get(index);
	u->SetVariant(GROUP);
}


void emBookmarksRec::CopyToClipboard(int index, emClipboard & clipboard)
{
	emArray<char> buf;
	emBookmarksRec tmpRec;
	emString str;

	buf.SetTuningLevel(4);
	tmpRec.SetCount(1);
	tmpRec.Get(0).Copy(Get(index));
	tmpRec.SaveToMem(buf);
	str=emString(buf,buf.GetCount());
	clipboard.PutText(str);
	clipboard.PutText(str,true);
}


void emBookmarksRec::TryInsertFromClipboard(
	int index, emClipboard & clipboard
) throw(emException)
{
	emString txt;
	emBookmarksRec tmpRec;
	int i,count;

	txt=clipboard.GetText();

	try {
		tmpRec.TryLoadFromMem(txt.Get(),txt.GetLen());
	}
	catch (emException & exception) {
		throw emException(
			"No valid bookmarks in clipboard (%s)",
			exception.GetText()
		);
	}

	tmpRec.ClearStartLocation();

	if (index<0) index=0;
	if (index>GetCount()) index=GetCount();
	count=tmpRec.GetCount();
	if (count<=0) return;
	Insert(index,count);
	for (i=0; i<count; i++) {
		Get(index+i).Copy(tmpRec.Get(i));
	}
}


emBookmarkRec * emBookmarksRec::SearchBookmarkByHotkey(const emInputHotkey & hotkey)
{
	emBookmarkGroupRec * grp;
	emBookmarkRec * bm;
	emInputHotkey bmhk;
	emUnionRec * u;
	int i;

	if (!hotkey.IsValid()) return NULL;
	for (i=0; i<GetCount(); i++) {
		u=(emUnionRec*)&Get(i);
		switch (u->GetVariant()) {
		case BOOKMARK:
			bm=(emBookmarkRec*)&u->Get();
			if (!bm->Hotkey.Get().IsEmpty()) {
				try {
					bmhk.TryParse(bm->Hotkey.Get().Get());
				}
				catch (emException &) {
					bmhk=emInputHotkey();
				}
				if (bmhk==hotkey) return bm;
			}
			break;
		case GROUP:
			grp=(emBookmarkGroupRec*)&u->Get();
			bm=grp->Bookmarks.SearchBookmarkByHotkey(hotkey);
			if (bm) return bm;
			break;
		}
	}
	return NULL;
}


emBookmarkRec * emBookmarksRec::SearchStartLocation()
{
	emBookmarkGroupRec * grp;
	emBookmarkRec * bm;
	emUnionRec * u;
	int i;

	for (i=0; i<GetCount(); i++) {
		u=(emUnionRec*)&Get(i);
		switch (u->GetVariant()) {
		case BOOKMARK:
			bm=(emBookmarkRec*)&u->Get();
			if (bm->VisitAtProgramStart.Get()) return bm;
			break;
		case GROUP:
			grp=(emBookmarkGroupRec*)&u->Get();
			bm=grp->Bookmarks.SearchStartLocation();
			if (bm) return bm;
			break;
		}
	}
	return NULL;
}


void emBookmarksRec::SetStartLocation(emBookmarkRec * rec)
{
	emBookmarkRec * bm;

	bm=SearchStartLocation();
	if (bm==rec) return;
	ClearStartLocation();
	if (!rec) return;
	rec->VisitAtProgramStart.Set(true);
}


void emBookmarksRec::ClearStartLocation()
{
	emBookmarkGroupRec * grp;
	emBookmarkRec * bm;
	emUnionRec * u;
	int i;

	for (i=0; i<GetCount(); i++) {
		u=(emUnionRec*)&Get(i);
		switch (u->GetVariant()) {
		case BOOKMARK:
			bm=(emBookmarkRec*)&u->Get();
			if (bm->VisitAtProgramStart.Get()) {
				bm->VisitAtProgramStart.Set(false);
			}
			break;
		case GROUP:
			grp=(emBookmarkGroupRec*)&u->Get();
			grp->Bookmarks.ClearStartLocation();
			break;
		}
	}
}


const char * emBookmarksRec::GetFormatName() const
{
	return "emBookmarks";
}


emString emBookmarksRec::BookmarkNameFromPanelTitle(const emString & title)
{
	int i,l;

	for (i=title.GetLen()-1; i>0; i--) {
		if (title[i-1]=='/' || title[i-1]=='\\') break;
	}
	if (i<=0) return title;
	for (l=1; i+l<title.GetLen(); l++) {
		if (title[i+l]=='.') break;
	}
	return title.GetSubString(i,l);
}


emRec * emBookmarksRec::AllocateUnion()
{
	return new emUnionRec(
		BOOKMARK,
		"Bookmark",EM_DEFAULT_REC_ALLOCATOR(emBookmarkRec),
		"Group",EM_DEFAULT_REC_ALLOCATOR(emBookmarkGroupRec),
		NULL
	);
}


//==============================================================================
//============================= emBookmarkEntryRec =============================
//==============================================================================

emBookmarkEntryRec::emBookmarkEntryRec(
		const emColor & defaultBgColor,
		const emColor & defaultFgColor
)
	: emStructRec(),
	Name(this,"Name"),
	Description(this,"Description"),
	Icon(this,"Icon"),
	BgColor(this,"BgColor",defaultBgColor),
	FgColor(this,"FgColor",defaultFgColor)
{
}


emBookmarkEntryRec::~emBookmarkEntryRec()
{
}


emBookmarksRec * emBookmarkEntryRec::GetBookmarksRec()
{
	emRec * p;

	p=GetParent();
	if (!p) return NULL;
	p=p->GetParent();
	if (!p) return NULL;
	return dynamic_cast<emBookmarksRec*>(p);
}


int emBookmarkEntryRec::GetIndexInBookmarksRec()
{
	emBookmarksRec * a;
	emRec * p, * u;
	int i;

	u=GetParent();
	if (!u) return -1;
	p=u->GetParent();
	if (!p) return -1;
	a=dynamic_cast<emBookmarksRec*>(p);
	if (!a) return -1;
	for (i=a->GetCount()-1; i>=0; i--) {
		if (u==&a->Get(i)) break;
	}
	return i;
}


void emBookmarkEntryRec::TryPasteColorsFromClipboard(
	emClipboard & clipboard
) throw(emException)
{
	emBookmarksRec tmpRec;
	emBookmarkEntryRec * e;
	emUnionRec * u;
	emString txt;

	txt=clipboard.GetText();

	try {
		tmpRec.TryLoadFromMem(txt.Get(),txt.GetLen());
	}
	catch (emException & exception) {
		throw emException(
			"No valid bookmarks in clipboard (%s)",
			exception.GetText()
		);
	}

	if (tmpRec.GetCount()<=0) return;
	u=(emUnionRec*)&tmpRec.Get(0);
	e=(emBookmarkEntryRec*)&u->Get();
	BgColor.Set(e->BgColor.Get());
	FgColor.Set(e->FgColor.Get());
}


//==============================================================================
//============================= emBookmarkGroupRec =============================
//==============================================================================

emBookmarkGroupRec::emBookmarkGroupRec()
	:
	emBookmarkEntryRec(
		emLook().GetBgColor(),
		emLook().GetFgColor()
	),
	Bookmarks(this,"Bookmarks")
{
}


emBookmarkGroupRec::~emBookmarkGroupRec()
{
}


//==============================================================================
//=============================== emBookmarkRec ================================
//==============================================================================

emBookmarkRec::emBookmarkRec()
	:
	emBookmarkEntryRec(
		emLook().GetButtonBgColor(),
		emLook().GetButtonFgColor()
	),
	Hotkey(this,"Hotkey"),
	LocationIdentity(this,"LocationIdentity"),
	LocationRelX(this,"LocationRelX"),
	LocationRelY(this,"LocationRelY"),
	LocationRelA(this,"LocationRelA",0.0,0.0),
	VisitAtProgramStart(this,"VisitAtProgramStart")
{
}


emBookmarkRec::~emBookmarkRec()
{
}


//==============================================================================
//============================== emBookmarksModel ==============================
//==============================================================================

emRef<emBookmarksModel> emBookmarksModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emBookmarksModel,rootContext,"")
}


emString emBookmarksModel::GetDefaultIconDir()
{
	return emGetInstallPath(EM_IDT_RES,"icons");
}


emString emBookmarksModel::GetNormalizedIconFileName(const emString & iconFile)
{
	emString iconDir;
	int dLen, fLen;

	if (!iconFile.IsEmpty()) {
		iconDir=GetDefaultIconDir();
		dLen=iconDir.GetLen();
		fLen=iconFile.GetLen();
		if (
			dLen+1<fLen &&
#if defined(_WIN32)
			(iconFile[dLen]=='\\' || iconFile[dLen]=='/') &&
#else
			iconFile[dLen]=='/' &&
#endif
			iconFile.GetSubString(0,dLen)==iconDir
		) {
			return iconFile.GetSubString(dLen+1,fLen-dLen-1);
		}
	}
	return iconFile;
}


emBookmarksModel::emBookmarksModel(emContext & context, const emString & name)
	: emConfigModel(context,name)
{
	emString recPath,srcPath,interpreter;
	emArray<emString> args;
	emProcess process;

	recPath=emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","bookmarks.rec");

	if (!emIsExistingPath(recPath)) {

		interpreter.Clear();
		srcPath=emGetInstallPath(EM_IDT_HOST_CONFIG,"emMain","InitialBookmarks.rec");

		if (!emIsExistingPath(srcPath)) {
#if defined(_WIN32)
			interpreter="wscript";
			srcPath=emGetInstallPath(EM_IDT_HOST_CONFIG,"emMain","CreateInitialBookmarks.js");
#else
			interpreter="perl";
			srcPath=emGetInstallPath(EM_IDT_HOST_CONFIG,"emMain","CreateInitialBookmarks.pl");
#endif
		}

		if (emIsExistingPath(srcPath)) {
			if (interpreter.IsEmpty()) {
				try {
					emTryMakeDirectories(emGetParentPath(recPath));
					emTryCopyFileOrTree(recPath,srcPath);
				}
				catch (emException & exception) {
					emWarning("%s",exception.GetText());
				}
			}
			else {
				args.Clear();
				args.Add(interpreter);
				args.Add(srcPath);
				args.Add(recPath);
				try {
					emTryMakeDirectories(emGetParentPath(recPath));
					process.TryStart(args);
					while (!process.WaitForTermination(60000)) {}
				}
				catch (emException & exception) {
					emWarning("%s",exception.GetText());
				}
			}
		}
	}

	PostConstruct(*this,recPath);
	LoadOrInstall(NULL);

	Update_0_90_0();
	Update_0_91_0();
}


emBookmarksModel::~emBookmarksModel()
{
}


void emBookmarksModel::Update_0_90_0()
{
	emBookmarkRec * bm;
	emUnionRec * u;
	bool anyChange;
	int i;

	// This is a hack which updates the colors and icons of the default
	// bookmarks from version <= 0.89.x to version 0.90.0.

	anyChange=false;
	for (i=0; i<GetCount(); i++) {
		u=(emUnionRec*)&Get(i);
		if (u->GetVariant() != BOOKMARK) continue;
		bm=(emBookmarkRec*)&u->Get();

		if (
			bm->BgColor.Get()==0xEEEEEEFFU &&
			bm->FgColor.Get()==0x333333FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Help" &&
			bm->Hotkey.Get()=="F1"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("help.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0xAAAAAAFFU &&
			bm->FgColor.Get()==0x000088FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Home" &&
			bm->Hotkey.Get()=="F6"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("home.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x777777FFU &&
			bm->FgColor.Get()==0x000066FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Root" &&
			bm->Hotkey.Get()=="F7"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("root.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x111122FFU &&
			bm->FgColor.Get()==0xDDDD99FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Virtual Cosmos" &&
			bm->LocationIdentity.Get()==":"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("virtual_cosmos.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x886644FFU &&
			bm->FgColor.Get()==0xDDCCBBFFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Chess" &&
			bm->LocationIdentity.Get()=="::Chess1:"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("silchess.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x221111FFU &&
			bm->FgColor.Get()==0xCC4444FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Mines" &&
			bm->LocationIdentity.Get()=="::Mines1:"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("mines.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x447799FFU &&
			(bm->FgColor.Get()==0xAADDCCFFU || bm->FgColor.Get()==0x66DDAAFFU) &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Netwalk" &&
			bm->LocationIdentity.Get()=="::Netwalk1:"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("netwalk.tga");
			anyChange=true;
		}

		if (
			bm->BgColor.Get()==0x002288FFU &&
			bm->FgColor.Get()==0x99CC66FFU &&
			bm->Icon.Get().IsEmpty() &&
			bm->Name.Get()=="Clock" &&
			bm->LocationIdentity.Get()=="::Clock1:"
		) {
			bm->BgColor.Set(emLook().GetButtonBgColor());
			bm->FgColor.Set(emLook().GetButtonFgColor());
			bm->Icon.Set("clock.tga");
			anyChange=true;
		}
	}

	if (anyChange) {
		Save();
	}
}


void emBookmarksModel::Update_0_91_0()
{
#if defined(_WIN32)
	emArray<emString> names;
	emString str;
	emBookmarkRec * bm;
	emUnionRec * u;
	bool anyChange;
	int i;

	// This is a hack which updates some locations of the default
	// bookmarks from version <= 0.90.0 to version 0.91.0 on Windows.

	anyChange=false;
	for (i=0; i<GetCount(); i++) {
		u=(emUnionRec*)&Get(i);
		if (u->GetVariant() != BOOKMARK) continue;
		bm=(emBookmarkRec*)&u->Get();

		if (
			bm->Name.Get()=="Home" &&
			bm->Hotkey.Get()=="F6" &&
			bm->LocationIdentity.Get()=="::HomeDir:::" &&
			fabs(bm->LocationRelX.Get() - 0.0)<0.000001 &&
			fabs(bm->LocationRelY.Get() - 0.0)<0.000001 &&
			fabs(bm->LocationRelA.Get() - 0.0)<0.000001
		) {
			str=emGetInstallPath(EM_IDT_HOME,"unknown");
			names.Clear();
			while (str!=emGetParentPath(str)) {
				names.Insert(0,emGetNameInPath(str));
				names.Insert(0,"");
				str=emGetParentPath(str);
			}
			names.Insert(0,str);
			names.Insert(0,"");
			names.Insert(0,"FS");
			names.Insert(0,"");
			names.Insert(0,"");
			bm->LocationIdentity.Set(emPanel::EncodeIdentity(names));
			anyChange=true;
		}

		if (
			bm->Name.Get()=="Root" &&
			bm->Hotkey.Get()=="F7" &&
			bm->LocationIdentity.Get()==":" &&
			fabs(bm->LocationRelX.Get() - 0.0014)<0.000001 &&
			fabs(bm->LocationRelY.Get() - 0.0031)<0.000001 &&
			fabs(bm->LocationRelA.Get() - 0.0271)<0.000001
		) {
			bm->LocationIdentity.Set("::FS:");
			bm->LocationRelX.Set(0.0);
			bm->LocationRelY.Set(0.0);
			bm->LocationRelA.Set(0.0);
			anyChange=true;
		}
	}

	if (anyChange) {
		Save();
	}
#endif
}


//==============================================================================
//========================== emBookmarkEntryAuxPanel ===========================
//==============================================================================

emBookmarkEntryAuxPanel::emBookmarkEntryAuxPanel(
	ParentArg parent, const emString & name, emView * contentView,
	emBookmarksModel * model, emBookmarkEntryRec * rec
)
	: emLinearGroup(parent,name)
{
	emLook look;

	ContentView=contentView;
	Model=model;
	UpToDate=false;
	BtNewBookmarkBefore=NULL;
	BtNewGroupBefore=NULL;
	BtPasteBefore=NULL;
	BtCut=NULL;
	BtCopy=NULL;
	TfName=NULL;
	TfDescription=NULL;
	FlbIcon=NULL;
	CfBgColor=NULL;
	CfFgColor=NULL;
	BtPasteColors=NULL;
	TfLocationIdentity=NULL;
	TfLocationRelX=NULL;
	TfLocationRelY=NULL;
	TfLocationRelA=NULL;
	BtSetLocation=NULL;
	TfHotkey=NULL;
	RbVisitAtProgramStart=NULL;
	BtNewBookmarkAfter=NULL;
	BtNewGroupAfter=NULL;
	BtPasteAfter=NULL;
	SetListenedRec(rec);
	SetAutoExpansionThreshold(5,VCT_MIN_EXT);
	SetBorderType(OBT_NONE,IBT_NONE);
	SetVertical();
	SetChildTallness(0,0.6);
	SetChildTallness(1,2.2);
	SetChildTallness(2,0.6);
	SetAlignment(EM_ALIGN_CENTER);
	SetInnerSpace(0.0,1.2);
	SetFocusable(false);
	emLinearGroup::SetLook(look);
	WakeUp();
}


emBookmarkEntryAuxPanel::~emBookmarkEntryAuxPanel()
{
}


void emBookmarkEntryAuxPanel::SetLook(const emLook & look, bool recursively)
{
	// Simply don't do it.
}


void emBookmarkEntryAuxPanel::OnRecChanged()
{
	UpToDate=false;
	WakeUp();
}


bool emBookmarkEntryAuxPanel::Cycle()
{
	emBookmarkEntryRec * entryRec;
	emBookmarkRec * bmRec;
	emBookmarkGroupRec * grpRec;
	emBookmarksRec * bmsParentRec;
	emClipboard * clipboard;
	emPanel * p;
	emString icon;
	double x,y,a;
	bool busy,modified,zoomOut;
	int index;

	busy=emLinearGroup::Cycle();

	if (!UpToDate) {
		Update();
		UpToDate=true;
	}

	modified=false;
	zoomOut=false;

	bmRec=NULL;
	grpRec=NULL;
	bmsParentRec=NULL;
	index=0;
	entryRec=(emBookmarkEntryRec*)GetListenedRec();
	if (entryRec) {
		grpRec=dynamic_cast<emBookmarkGroupRec*>(entryRec);
		if (!grpRec) bmRec=dynamic_cast<emBookmarkRec*>(entryRec);
		bmsParentRec=entryRec->GetBookmarksRec();
		index=entryRec->GetIndexInBookmarksRec();
	}
	clipboard=emClipboard::LookupInherited(GetView());

	if (BtNewBookmarkBefore && IsSignaled(BtNewBookmarkBefore->GetClickSignal())) {
		if (bmsParentRec && ContentView) {
			bmsParentRec->InsertNewBookmark(index,ContentView);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtNewGroupBefore && IsSignaled(BtNewGroupBefore->GetClickSignal())) {
		if (bmsParentRec) {
			bmsParentRec->InsertNewGroup(index);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtPasteBefore && IsSignaled(BtPasteBefore->GetClickSignal())) {
		if (bmsParentRec && clipboard) {
			try {
				bmsParentRec->TryInsertFromClipboard(index,*clipboard);
				modified=true;
				zoomOut=true;
			}
			catch (emException &) {
				if (GetScreen()) GetScreen()->Beep();
			}
		}
	}

	if (BtCut && IsSignaled(BtCut->GetClickSignal())) {
		if (bmsParentRec && clipboard) {
			bmsParentRec->CopyToClipboard(index,*clipboard);
			bmsParentRec->Remove(index);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtCopy && IsSignaled(BtCopy->GetClickSignal())) {
		if (bmsParentRec && clipboard) {
			bmsParentRec->CopyToClipboard(index,*clipboard);
		}
	}
	if (TfName && IsSignaled(TfName->GetTextSignal())) {
		if (entryRec) {
			if (entryRec->Name.Get()!=TfName->GetText()) {
				entryRec->Name.Set(TfName->GetText());
				modified=true;
			}
		}
	}
	if (TfDescription && IsSignaled(TfDescription->GetTextSignal())) {
		if (entryRec) {
			if (entryRec->Description.Get()!=TfDescription->GetText()) {
				entryRec->Description.Set(TfDescription->GetText());
				modified=true;
			}
		}
	}
	if (FlbIcon && IsSignaled(FlbIcon->GetSelectionSignal())) {
		if (entryRec) {
			if (FlbIcon->GetSelectedNames().IsEmpty()) {
				if (!entryRec->Icon.Get().IsEmpty()) {
					entryRec->Icon.Set(emString());
					modified=true;
				}
			}
			else {
				icon=emBookmarksModel::GetNormalizedIconFileName(
					FlbIcon->GetSelectedPath()
				);
				if (entryRec->Icon.Get()!=icon) {
					if (emIsDirectory(icon)) {
						icon.Clear();
					}
					if (entryRec->Icon.Get()!=icon) {
						entryRec->Icon.Set(icon);
						modified=true;
					}
				}
			}
		}
	}
	if (FlbIcon && IsSignaled(FlbIcon->GetFileTriggerSignal())) {
		p=GetParent();
		if (grpRec && p) p=p->GetParent();
		if (p) GetView().VisitFullsized(p,false);
	}
	if (CfBgColor && IsSignaled(CfBgColor->GetColorSignal())) {
		if (entryRec) {
			if (entryRec->BgColor.Get()!=CfBgColor->GetColor()) {
				entryRec->BgColor.Set(CfBgColor->GetColor());
				modified=true;
			}
		}
	}
	if (CfFgColor && IsSignaled(CfFgColor->GetColorSignal())) {
		if (entryRec) {
			if (entryRec->FgColor.Get()!=CfFgColor->GetColor()) {
				entryRec->FgColor.Set(CfFgColor->GetColor());
				modified=true;
			}
		}
	}
	if (BtPasteColors && IsSignaled(BtPasteColors->GetClickSignal())) {
		if (entryRec && clipboard) {
			try {
				entryRec->TryPasteColorsFromClipboard(*clipboard);
				modified=true;
			}
			catch (emException &) {
				if (GetScreen()) GetScreen()->Beep();
			}
		}
	}
	if (BtSetLocation && IsSignaled(BtSetLocation->GetClickSignal())) {
		if (bmRec && ContentView) {
			p=ContentView->GetVisitedPanel(&x,&y,&a);
			if (p) {
				bmRec->LocationIdentity=p->GetIdentity();
				bmRec->LocationRelX=x;
				bmRec->LocationRelY=y;
				bmRec->LocationRelA=a;
				modified=true;
			}
		}
	}
	if (TfHotkey && IsSignaled(TfHotkey->GetTextSignal())) {
		if (bmRec) {
			if (bmRec->Hotkey.Get()!=TfHotkey->GetText()) {
				bmRec->Hotkey.Set(TfHotkey->GetText());
				modified=true;
			}
		}
	}
	if (RbVisitAtProgramStart && IsSignaled(RbVisitAtProgramStart->GetClickSignal())) {
		if (bmRec && Model) {
			if (!bmRec->VisitAtProgramStart.Get()) {
				Model->SetStartLocation(bmRec);
				modified=true;
			}
		}
	}
	if (BtNewBookmarkAfter && IsSignaled(BtNewBookmarkAfter->GetClickSignal())) {
		if (bmsParentRec && ContentView) {
			bmsParentRec->InsertNewBookmark(index+1,ContentView);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtNewGroupAfter && IsSignaled(BtNewGroupAfter->GetClickSignal())) {
		if (bmsParentRec) {
			bmsParentRec->InsertNewGroup(index+1);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtPasteAfter && IsSignaled(BtPasteAfter->GetClickSignal())) {
		if (bmsParentRec && clipboard) {
			try {
				bmsParentRec->TryInsertFromClipboard(index+1,*clipboard);
				modified=true;
				zoomOut=true;
			}
			catch (emException &) {
				if (GetScreen()) GetScreen()->Beep();
			}
		}
	}

	if (modified && Model && Model->IsUnsaved()) Model->Save();
	if (zoomOut) {
		p=GetParent();
		if (p) p=p->GetParent();
		if (grpRec && p) p=p->GetParent();
		if (p) GetView().VisitFullsized(p,false);
	}

	return busy;
}


void emBookmarkEntryAuxPanel::AutoExpand()
{
	emBookmarkEntryRec * entryRec;
	emBookmarkRec * bmRec;
	emBookmarkGroupRec * grpRec;
	emBookmarksRec * bmsParentRec;
	emLinearGroup * g;
	emRasterGroup * g2;
	emArray<emString> filters;

	emLinearGroup::AutoExpand();

	bmRec=NULL;
	bmsParentRec=NULL;
	entryRec=(emBookmarkEntryRec*)GetListenedRec();
	if (entryRec) {
		grpRec=dynamic_cast<emBookmarkGroupRec*>(entryRec);
		if (!grpRec) bmRec=dynamic_cast<emBookmarkRec*>(entryRec);
		bmsParentRec=entryRec->GetBookmarksRec();
	}

	if (bmsParentRec) {
		g=new emLinearGroup(
			this,
			"before",
			bmRec ? "Before This Bookmark" : "Before This Group"
		);
		g->SetVertical();
		BtNewBookmarkBefore=new emButton(
			g,
			"nb",
			"New Bookmark",
			"Insert a new bookmark form the current location."
		);
		BtNewBookmarkBefore->SetNoEOI();
		AddWakeUpSignal(BtNewBookmarkBefore->GetClickSignal());
		BtNewGroupBefore=new emButton(
			g,
			"ng",
			"New Group",
			"Insert a new empty group."
		);
		BtNewGroupBefore->SetNoEOI();
		AddWakeUpSignal(BtNewGroupBefore->GetClickSignal());
		BtPasteBefore=new emButton(
			g,
			"p",
			"Paste",
			"Insert a bookmark or group from the clipboard."
		);
		BtPasteBefore->SetNoEOI();
		AddWakeUpSignal(BtPasteBefore->GetClickSignal());
	}

	if (entryRec) {
		g=new emLinearGroup(
			this,
			"entry",
			bmRec ? "This Bookmark" : "This Group"
		);
		g->SetVertical();
		g->SetMinCellCount(12);
		BtCut=new emButton(
			g,
			"cut",
			"Cut",
			bmRec ?
				"Remove this bookmark and put it to the clipboard."
			:
				"Remove this group and put it to the clipboard."
		);
		BtCut->SetNoEOI();
		AddWakeUpSignal(BtCut->GetClickSignal());
		BtCopy=new emButton(
			g,
			"copy",
			"Copy",
			bmRec ?
				"Copy this bookmark to the clipboard."
			:
				"Copy this group to the clipboard."
		);
		BtCopy->SetNoEOI();
		AddWakeUpSignal(BtCopy->GetClickSignal());
		TfName=new emTextField(
			g,
			"name",
			"Name",
			bmRec ?
				"A name to be shown in this bookmark button."
			:
				"A name to be shown in this bookmark group border."
		);
		TfName->SetEditable();
		TfName->SetMultiLineMode();
		AddWakeUpSignal(TfName->GetTextSignal());
		TfDescription=new emTextField(
			g,
			"desc",
			"Description",
			bmRec ?
				"A description to be shown in this bookmark button."
			:
				"A description to be shown in this bookmark group border."
		);
		TfDescription->SetEditable();
		TfDescription->SetMultiLineMode();
		AddWakeUpSignal(TfDescription->GetTextSignal());
		FlbIcon=new emFileSelectionBox(
			g,
			"icon",
			"Icon",
			emString::Format(
				"An icon to be shown in this bookmark %s.\n"
				"It must be the name of a TGA file.",
				bmRec ? "button" : "group border"
			)
		);
		FlbIcon->SetBorderType(OBT_INSTRUMENT,IBT_NONE);
		filters.Clear();
		filters.Add("Targa Files (*.tga)");
		FlbIcon->SetFilters(filters);
		FlbIcon->SetParentDirectory(emBookmarksModel::GetDefaultIconDir());
		AddWakeUpSignal(FlbIcon->GetSelectionSignal());
		AddWakeUpSignal(FlbIcon->GetFileTriggerSignal());
		CfBgColor=new emColorField(
			g,
			"bgcol",
			"Background Color",
			bmRec ?
				"Background color of this bookmark button."
			:
				"Background color of this bookmark group."
		);
		CfBgColor->SetEditable();
		AddWakeUpSignal(CfBgColor->GetColorSignal());
		CfFgColor=new emColorField(
			g,
			"fgcol",
			"Foreground Color",
			bmRec ?
				"Foreground color of this bookmark button."
			:
				"Foreground color of this bookmark group."
		);
		CfFgColor->SetEditable();
		AddWakeUpSignal(CfFgColor->GetColorSignal());
		BtPasteColors=new emButton(
			g,
			"pastecolors",
			"Paste Colors",
			bmRec ?
				"Replace the colors of this bookmark button by the\n"
				"colors of a bookmark or group in the clipboard."
			:
				"Replace the colors of this bookmark group by the\n"
				"colors of a bookmark or group in the clipboard."
		);
		BtPasteColors->SetNoEOI();
		AddWakeUpSignal(BtPasteColors->GetClickSignal());
		if (bmRec) {
			g2=new emRasterGroup(g,"loc","Location");
			g2->SetPrefChildTallness(0.2);
			g2->SetOuterBorderType(OBT_INSTRUMENT);
			TfLocationIdentity=new emTextField(g2,"Identity","Panel Identity");
			TfLocationRelX=new emTextField(g2,"RelX","Relative X-Position");
			TfLocationRelY=new emTextField(g2,"RelY","Relative Y-Position");
			TfLocationRelA=new emTextField(g2,"RelA","Relative Area Size");
			BtSetLocation=new emButton(
				g,
				"setlocation",
				"Set Location",
				"Replace the location of this bookmark by the current location."
			);
			BtSetLocation->SetNoEOI();
			AddWakeUpSignal(BtSetLocation->GetClickSignal());
			TfHotkey=new emTextField(
				g,"hotkey",
				"Hotkey",
				"A hotkey for this bookmark, or leave blank.\n"
				"Examples: F7, Ctrl+F8, Alt+Meta+X\n"
				"Possible modifiers: Shift, Ctrl, Alt and Meta\n"
				"Possible keys: A-Z, 0-9, F1-F12, Print, Pause, ...\n"
			);
			TfHotkey->SetEditable(true);
			AddWakeUpSignal(TfHotkey->GetTextSignal());
			RbVisitAtProgramStart=new emRadioBox(
				g,
				"visitatprogramstart",
				"Visit At Program Start",
				"Whether this is the bookmark whose location shall be visited\n"
				"automatically when Eagle Mode starts."
			);
			RbVisitAtProgramStart->SetNoEOI();
			AddWakeUpSignal(RbVisitAtProgramStart->GetClickSignal());
		}
	}

	if (bmsParentRec) {
		g=new emLinearGroup(
			this,
			"after",
			bmRec ? "After This Bookmark" : "After This Group"
		);
		g->SetVertical();
		BtNewBookmarkAfter=new emButton(
			g,
			"nb",
			"New Bookmark",
			"Insert a new bookmark form the current location."
		);
		BtNewBookmarkAfter->SetNoEOI();
		AddWakeUpSignal(BtNewBookmarkAfter->GetClickSignal());
		BtNewGroupAfter=new emButton(
			g,
			"ng",
			"New Group",
			"Insert a new empty group."
		);
		BtNewGroupAfter->SetNoEOI();
		AddWakeUpSignal(BtNewGroupAfter->GetClickSignal());
		BtPasteAfter=new emButton(
			g,
			"p",
			"Paste",
			"Insert a bookmark or group from the clipboard."
		);
		BtPasteAfter->SetNoEOI();
		AddWakeUpSignal(BtPasteAfter->GetClickSignal());
	}

	UpToDate=false;
	WakeUp();
}


void emBookmarkEntryAuxPanel::AutoShrink()
{
	emLinearGroup::AutoShrink();

	BtNewBookmarkBefore=NULL;
	BtNewGroupBefore=NULL;
	BtPasteBefore=NULL;
	BtCut=NULL;
	BtCopy=NULL;
	TfName=NULL;
	TfDescription=NULL;
	FlbIcon=NULL;
	CfBgColor=NULL;
	CfFgColor=NULL;
	BtPasteColors=NULL;
	TfLocationIdentity=NULL;
	TfLocationRelX=NULL;
	TfLocationRelY=NULL;
	TfLocationRelA=NULL;
	BtSetLocation=NULL;
	TfHotkey=NULL;
	RbVisitAtProgramStart=NULL;
	BtNewBookmarkAfter=NULL;
	BtNewGroupAfter=NULL;
	BtPasteAfter=NULL;
}


void emBookmarkEntryAuxPanel::Update()
{
	emBookmarkEntryRec * entryRec;
	emBookmarkRec * bmRec;

	entryRec=(emBookmarkEntryRec*)GetListenedRec();
	if (!entryRec) return;

	if (TfName) TfName->SetText(entryRec->Name.Get());
	if (TfDescription) TfDescription->SetText(entryRec->Description.Get());

	if (FlbIcon) {
		if (entryRec->Icon.Get().IsEmpty()) {
			if (
				!FlbIcon->GetSelectedNames().IsEmpty() &&
				!emIsDirectory(FlbIcon->GetSelectedPath())
			) {
				FlbIcon->SetParentDirectory(emBookmarksModel::GetDefaultIconDir());
				FlbIcon->ClearSelection();
			}
		}
		else {
			FlbIcon->SetSelectedPath(
				emGetAbsolutePath(
					entryRec->Icon.Get(),
					emBookmarksModel::GetDefaultIconDir()
				)
			);
		}
	}

	if (CfBgColor) CfBgColor->SetColor(entryRec->BgColor.Get());
	if (CfFgColor) CfFgColor->SetColor(entryRec->FgColor.Get());

	bmRec=dynamic_cast<emBookmarkRec*>(entryRec);
	if (bmRec) {
		if (TfLocationIdentity) {
			TfLocationIdentity->SetText(bmRec->LocationIdentity.Get());
		}
		if (TfLocationRelX) {
			TfLocationRelX->SetText(
				emString::Format("%.9G",bmRec->LocationRelX.Get())
			);
		}
		if (TfLocationRelY) {
			TfLocationRelY->SetText(
				emString::Format("%.9G",bmRec->LocationRelY.Get())
			);
		}
		if (TfLocationRelA) {
			TfLocationRelA->SetText(
				emString::Format("%.9G",bmRec->LocationRelA.Get())
			);
		}
		if (TfHotkey) {
			TfHotkey->SetText(bmRec->Hotkey.Get());
		}
		if (RbVisitAtProgramStart) {
			RbVisitAtProgramStart->SetChecked(bmRec->VisitAtProgramStart.Get());
		}
	}
}


//==============================================================================
//============================ emBookmarksAuxPanel =============================
//==============================================================================

emBookmarksAuxPanel::emBookmarksAuxPanel(
	ParentArg parent, const emString & name, emView * contentView,
	emBookmarksModel * model, emBookmarksRec * rec
)
	: emLinearGroup(parent,name)
{
	ContentView=contentView;
	Model=model;
	BtNewBookmark=NULL;
	BtNewGroup=NULL;
	BtPaste=NULL;
	SetListenedRec(rec);
	SetAutoExpansionThreshold(5,VCT_MIN_EXT);
	SetVertical();
	emLinearGroup::SetLook(emLook());
	SetCaption("In This Empty Group");
}


emBookmarksAuxPanel::~emBookmarksAuxPanel()
{
}


void emBookmarksAuxPanel::SetLook(const emLook & look, bool recursively)
{
	// Simply don't do it.
}


void emBookmarksAuxPanel::OnRecChanged()
{
}


bool emBookmarksAuxPanel::Cycle()
{
	emBookmarksRec * bms;
	emClipboard * clipboard;
	emPanel * p;
	bool busy,modified,zoomOut;

	busy=emLinearGroup::Cycle();

	modified=false;
	zoomOut=false;

	if (BtNewBookmark && IsSignaled(BtNewBookmark->GetClickSignal())) {
		bms=(emBookmarksRec*)GetListenedRec();
		if (bms && ContentView) {
			bms->InsertNewBookmark(bms->GetCount(),ContentView);
			modified=true;
			zoomOut=true;
		}
	}
	if (BtNewGroup && IsSignaled(BtNewGroup->GetClickSignal())) {
		bms=(emBookmarksRec*)GetListenedRec();
		if (bms) {
			bms->InsertNewGroup(bms->GetCount());
			modified=true;
			zoomOut=true;
		}
	}
	if (BtPaste && IsSignaled(BtPaste->GetClickSignal())) {
		bms=(emBookmarksRec*)GetListenedRec();
		clipboard=emClipboard::LookupInherited(GetView());
		if (bms && clipboard) {
			try {
				bms->TryInsertFromClipboard(bms->GetCount(),*clipboard);
				modified=true;
				zoomOut=true;
			}
			catch (emException &) {
				if (GetScreen()) GetScreen()->Beep();
			}
		}
	}

	if (modified && Model && Model->IsUnsaved()) Model->Save();
	if (zoomOut) {
		p=GetParent();
		if (p) GetView().VisitFullsized(p,false);
	}

	return busy;
}


void emBookmarksAuxPanel::AutoExpand()
{
	emLinearGroup::AutoExpand();

	BtNewBookmark=new emButton(
		this,
		"nb",
		"New Bookmark",
		"Insert a new bookmark form the current location."
	);
	BtNewBookmark->SetNoEOI();
	AddWakeUpSignal(BtNewBookmark->GetClickSignal());
	BtNewGroup=new emButton(
		this,
		"ng",
		"New Group",
		"Insert a new empty group."
	);
	BtNewGroup->SetNoEOI();
	AddWakeUpSignal(BtNewGroup->GetClickSignal());
	BtPaste=new emButton(
		this,
		"p",
		"Paste",
		"Insert a bookmark or group from the clipboard."
	);
	BtPaste->SetNoEOI();
	AddWakeUpSignal(BtPaste->GetClickSignal());
}


void emBookmarksAuxPanel::AutoShrink()
{
	emLinearGroup::AutoShrink();
	BtNewBookmark=NULL;
	BtNewGroup=NULL;
	BtPaste=NULL;
}


//==============================================================================
//============================== emBookmarkButton ==============================
//==============================================================================

emBookmarkButton::emBookmarkButton(
	ParentArg parent, const emString & name, emView * contentView,
	emBookmarksModel * model, emBookmarkRec * rec
)
	: emButton(parent,name)
{
	ContentView=contentView;
	Model=model;
	UpToDate=false;
	SetListenedRec(rec);
	HaveAux("aux",6.0);
	SetIconAboveCaption();
	SetMaxIconAreaTallness(9.0/16.0);
	SetBorderScaling(0.5);
	SetAutoExpansionThreshold(5,VCT_MIN_EXT);
	AddWakeUpSignal(GetClickSignal());
	WakeUp();
}


emBookmarkButton::~emBookmarkButton()
{
}


void emBookmarkButton::SetLook(const emLook & look, bool recursively)
{
	emBookmarkRec * bmRec;
	emLook newLook;

	newLook=look;
	bmRec=(emBookmarkRec*)GetListenedRec();
	if (bmRec) {
		newLook.SetButtonBgColor(bmRec->BgColor);
		newLook.SetButtonFgColor(bmRec->FgColor);
	}
	emButton::SetLook(newLook,recursively);
}


void emBookmarkButton::OnRecChanged()
{
	UpToDate=false;
	WakeUp();
}


bool emBookmarkButton::Cycle()
{
	emBookmarkRec * bmRec;
	bool busy;

	busy=emButton::Cycle();

	if (!UpToDate) {
		Update();
		UpToDate=true;
	}

	if (IsSignaled(GetClickSignal()) && ContentView!=NULL) {
		bmRec=(emBookmarkRec*)GetListenedRec();
		if (bmRec) {
			ContentView->Visit(
				bmRec->LocationIdentity.Get(),
				bmRec->LocationRelX.Get(),
				bmRec->LocationRelY.Get(),
				bmRec->LocationRelA.Get(),
				true,
				bmRec->Name.Get()
			);
		}
	}

	return busy;
}


void emBookmarkButton::AutoExpand()
{
	new emBookmarkEntryAuxPanel(this,"aux",ContentView,Model,GetRec());
}


void emBookmarkButton::Update()
{
	emBookmarkRec * bmRec;
	emString str;
	emImage img;
	emLook look;

	bmRec=(emBookmarkRec*)GetListenedRec();
	if (!bmRec) return;

	SetCaption(bmRec->Name);

	str=bmRec->Description;
	if (!bmRec->Hotkey.Get().IsEmpty()) {
		if (!str.IsEmpty()) str+="\n\n";
		str+="Hotkey: ";
		str+=bmRec->Hotkey.Get();
	}
	SetDescription(str);

	if (!bmRec->Icon.Get().IsEmpty()) {
		try {
			img=emTryGetResImage(
				GetRootContext(),
				emGetAbsolutePath(
					bmRec->Icon.Get(),
					emBookmarksModel::GetDefaultIconDir()
				)
			);
		}
		catch (emException &) {
			try {
				img=emTryGetInsResImage(
					GetRootContext(),"icons","error_unknown_icon.tga"
				);
			}
			catch (emException &) {
				img.Clear();
			}
		}
	}
	SetIcon(img);
	look=GetLook();
	look.SetButtonBgColor(bmRec->BgColor);
	look.SetButtonFgColor(bmRec->FgColor);
	emButton::SetLook(look);
}


//==============================================================================
//============================== emBookmarksPanel ==============================
//==============================================================================

emBookmarksPanel::emBookmarksPanel(
	ParentArg parent, const emString & name, emView * contentView,
	emBookmarksModel * model, emRec * rec
)
	: emRasterGroup(parent,name)
{
	if (!rec) rec=model;
	ContentView=contentView;
	Model=model;
	UpToDate=false;
	RasterLayout=NULL;
	SetListenedRec(rec);
	SetAutoExpansionThreshold(5,VCT_MIN_EXT);
	WakeUp();
	SetCaption("Bookmarks");
	SetDescription(
		"The bookmark buttons herein can bring you quickly to certain locations.\n"
		"For adding new bookmarks or editing existing ones, please refer to the\n"
		"small auxiliary panels in the right borders of the buttons."
	);
}


emBookmarksPanel::~emBookmarksPanel()
{
}


void emBookmarksPanel::SetLook(const emLook & look, bool recursively)
{
	emRec * rec;
	emBookmarkGroupRec * grpRec;
	emLook newLook;

	newLook=look;
	rec=GetListenedRec();
	if (rec) {
		grpRec=dynamic_cast<emBookmarkGroupRec*>(rec);
		if (grpRec) {
			newLook.SetBgColor(grpRec->BgColor);
			newLook.SetFgColor(grpRec->FgColor);
		}
	}
	emRasterGroup::SetLook(newLook,recursively);
}


void emBookmarksPanel::OnRecChanged()
{
	UpToDate=false;
	WakeUp();
}


bool emBookmarksPanel::Cycle()
{
	bool busy;

	busy=emRasterGroup::Cycle();

	if (!UpToDate) {
		Update();
		UpToDate=true;
	}

	return busy;
}


void emBookmarksPanel::AutoExpand()
{
	emRec * rec;
	emBookmarksRec * bmsRec;
	emBookmarkGroupRec * grpRec;
	emUnionRec * unionRec;
	char name[256];
	int idx,cnt;

	emRasterGroup::AutoExpand();

	rec=GetListenedRec();
	if (!rec) return;
	grpRec=dynamic_cast<emBookmarkGroupRec*>(rec);
	if (grpRec) bmsRec=&grpRec->Bookmarks;
	else bmsRec=dynamic_cast<emBookmarksRec*>(rec);

	if (!bmsRec) return;

	if (grpRec) {
		RasterLayout=new emRasterLayout(this,"t");
		RasterLayout->HaveAux("aux",11.0);
		new emBookmarkEntryAuxPanel(RasterLayout,"aux",ContentView,Model,grpRec);
		RasterLayout->SetBorderType(OBT_NONE,IBT_GROUP);
		SetInnerBorderType(IBT_NONE);
	}
	else {
		RasterLayout=this;
	}

	cnt=bmsRec->GetCount();
	if (cnt<=0) {
		RasterLayout->SetMinCellCount(1);
		RasterLayout->SetOuterSpace(1.0,1.0);
		RasterLayout->SetChildTallness(0.6);
		RasterLayout->SetStrictRaster(false);
		RasterLayout->SetAlignment(EM_ALIGN_CENTER);
		new emBookmarksAuxPanel(RasterLayout,"empty",ContentView,Model,bmsRec);
	}
	else {
		RasterLayout->SetMinCellCount(2);
		RasterLayout->SetOuterSpace(0.0,0.0);
		RasterLayout->SetPrefChildTallness(1.0);
		RasterLayout->SetMinChildTallness(0.45);
		RasterLayout->SetMaxChildTallness(0.9);
		RasterLayout->SetStrictRaster(true);
		RasterLayout->SetAlignment(EM_ALIGN_TOP_LEFT);
		for (idx=0; idx<cnt; idx++) {
			unionRec=(emUnionRec*)&bmsRec->Get(idx);
			sprintf(name,"%d",idx);
			switch (unionRec->GetVariant()) {
			case emBookmarksRec::BOOKMARK:
				new emBookmarkButton(
					RasterLayout,
					name,
					ContentView,
					Model,
					(emBookmarkRec*)&unionRec->Get()
				);
				break;
			case emBookmarksRec::GROUP:
				new emBookmarksPanel(
					RasterLayout,
					name,
					ContentView,
					Model,
					&unionRec->Get()
				);
				break;
			}
		}
	}
}


void emBookmarksPanel::AutoShrink()
{
	emRasterGroup::AutoShrink();
	RasterLayout=NULL;
}


void emBookmarksPanel::Update()
{
	emRec * rec;
	emBookmarksRec * bmsRec;
	emBookmarkGroupRec * grpRec;
	emImage img;
	emLook look;
	emPanel * p, * aux;
	emBookmarkButton * bmButton;
	emBookmarksPanel * bmsPanel;
	int idx;

	rec=GetListenedRec();
	if (!rec) return;
	grpRec=dynamic_cast<emBookmarkGroupRec*>(rec);
	if (grpRec) bmsRec=&grpRec->Bookmarks;
	else bmsRec=dynamic_cast<emBookmarksRec*>(rec);

	if (grpRec) {
		SetCaption(grpRec->Name);
		SetDescription(grpRec->Description);
		if (!grpRec->Icon.Get().IsEmpty()) {
			try {
				img=emTryGetResImage(
					GetRootContext(),
					emGetAbsolutePath(
						grpRec->Icon.Get(),
						emBookmarksModel::GetDefaultIconDir()
					)
				);
			}
			catch (emException &) {
				try {
					img=emTryGetInsResImage(
						GetRootContext(),"icons","error_unknown_icon.tga"
					);
				}
				catch (emException &) {
					img.Clear();
				}
			}
		}
		SetIcon(img);
		look=GetLook();
		look.SetBgColor(grpRec->BgColor);
		look.SetFgColor(grpRec->FgColor);
		emRasterGroup::SetLook(look,true);
	}

	if (bmsRec && RasterLayout) {
		aux=RasterLayout->GetAuxPanel();
		for (idx=0, p=RasterLayout->GetFirstChild(); ; p=p->GetNext()) {
			if (!p) {
				if (idx!=bmsRec->GetCount()) InvalidateAutoExpansion();
				break;
			}
			if (p==aux) continue;
			bmButton=dynamic_cast<emBookmarkButton*>(p);
			if (bmButton) {
				rec=bmButton->GetRec();
			}
			else {
				bmsPanel=dynamic_cast<emBookmarksPanel*>(p);
				if (bmsPanel) {
					rec=bmsPanel->GetRec();
				}
				else {
					continue;
				}
			}
			if (
				idx>=bmsRec->GetCount() ||
				rec!=&((emUnionRec*)&bmsRec->Get(idx))->Get()
			) {
				InvalidateAutoExpansion();
				break;
			}
			idx++;
		}
	}
}
