//------------------------------------------------------------------------------
// emFileSelectionBox.cpp
//
// Copyright (C) 2015-2016 Oliver Hamann.
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

#include <emCore/emFileSelectionBox.h>
#include <emCore/emAvlTreeMap.h>
#include <emCore/emFpPlugin.h>
#include <ctype.h>
#if defined(_WIN32)
#	include <windows.h>
#endif


emFileSelectionBox::emFileSelectionBox(
	ParentArg parent, const emString & name,
	const emString & caption,
	const emString & description,
	const emImage & icon
) :
	emBorder(parent,name,caption,description,icon)
{
	MultiSelectionEnabled=false;
	ParentDir=emGetCurrentDirectory();
	SelectedNames.SetTuningLevel(1);
	Filters.SetTuningLevel(1);
	SelectedFilterIndex=-1;
	HiddenFilesShown=false;

	ParentDirFieldHidden=false;
	HiddenCheckBoxHidden=false;
	NameFieldHidden=false;
	FilterHidden=false;

	ParentDirField=NULL;
	HiddenCheckBox=NULL;
	FilesLB=NULL;
	NameField=NULL;
	FiltersLB=NULL;

	ListingInvalid=true;

	SetBorderType(OBT_GROUP,IBT_GROUP);
}


emFileSelectionBox::~emFileSelectionBox()
{
}


void emFileSelectionBox::SetMultiSelectionEnabled(bool enabled)
{
	if (MultiSelectionEnabled!=enabled) {
		if (!enabled && SelectedNames.GetCount()>1) {
			SetSelectedName(SelectedNames[0]);
		}
		MultiSelectionEnabled=enabled;
		if (FilesLB) {
			FilesLB->SetSelectionType(
				MultiSelectionEnabled ?
				emListBox::MULTI_SELECTION :
				emListBox::SINGLE_SELECTION
			);
		}
	}
}


void emFileSelectionBox::SetParentDirectory(const emString & parentDirectory)
{
	emString absPath;

#if defined(_WIN32)
	if (
		parentDirectory.IsEmpty() || (
			parentDirectory.GetLen() == 5 &&
			parentDirectory[1]==':' &&
			(parentDirectory[2]=='\\' || parentDirectory[2]=='/') &&
			parentDirectory[3]=='.' &&
			parentDirectory[4]=='.'
		)
	) {
		absPath.Clear();
	}
	else {
		absPath=emGetAbsolutePath(parentDirectory);
	}
#else
	absPath=emGetAbsolutePath(parentDirectory);
#endif

	if (ParentDir!=absPath) {
		ParentDir=absPath;
		if (ParentDirField) {
			ParentDirField->SetText(ParentDir);
		}
		TriggeredFileName.Clear();
		InvalidateListing();
		Signal(SelectionSignal);
	}
}


emString emFileSelectionBox::GetSelectedName() const
{
	if (!SelectedNames.IsEmpty()) return SelectedNames[0];
	else return emString();
}


void emFileSelectionBox::SetSelectedName(const emString & selectedName)
{
	if (selectedName.IsEmpty()) {
		if (!SelectedNames.IsEmpty()) {
			emArray<emString> array;
			array.SetTuningLevel(1);
			SetSelectedNames(array);
		}
	}
	else {
		if (SelectedNames.GetCount()!=1 || SelectedNames[0]!=selectedName) {
			emArray<emString> array;
			array.SetTuningLevel(1);
			array.Add(selectedName);
			SetSelectedNames(array);
		}
	}
}


void emFileSelectionBox::SetSelectedNames(const emArray<emString> & selectedNames)
{
	emArray<emString> sortedSelectedNames;
	bool changed;
	int i;

	sortedSelectedNames = selectedNames;
	if (sortedSelectedNames.GetCount() > 1) {
		sortedSelectedNames.Sort(CompareNames,this);
	}

	if (sortedSelectedNames.GetCount()!=SelectedNames.GetCount()) {
		changed=true;
	}
	else {
		for (i=sortedSelectedNames.GetCount()-1; i>=0; i--) {
			if (sortedSelectedNames[i]!=SelectedNames[i]) {
				break;
			}
		}
		changed = (i>=0);
	}

	if (changed) {
		SelectedNames=sortedSelectedNames;
		if (NameField) {
			if (SelectedNames.GetCount()==1) {
				NameField->SetText(SelectedNames[0]);
			}
			else {
				NameField->SetText(emString());
			}
		}
		SelectionToListBox();
		Signal(SelectionSignal);
	}
}


void emFileSelectionBox::ClearSelection()
{
	SetSelectedName(emString());
}


emString emFileSelectionBox::GetSelectedPath() const
{
	if (!SelectedNames.IsEmpty()) return emGetChildPath(ParentDir,SelectedNames[0]);
	else return ParentDir;
}


void emFileSelectionBox::SetSelectedPath(const emString & selectedPath)
{
	emString absPath;
	bool isDir;

#if defined(_WIN32)
	if (
		selectedPath.IsEmpty() || (
			selectedPath.GetLen() == 5 &&
			selectedPath[1]==':' &&
			(selectedPath[2]=='\\' || selectedPath[2]=='/') &&
			selectedPath[3]=='.' &&
			selectedPath[4]=='.'
		)
	) {
		absPath.Clear();
		isDir=true;
	}
	else {
		absPath=emGetAbsolutePath(selectedPath);
		isDir=emIsDirectory(absPath);
	}
#else
	absPath=emGetAbsolutePath(selectedPath);
	isDir=emIsDirectory(absPath);
#endif

	if (isDir) {
		SetParentDirectory(absPath);
		ClearSelection();
	}
	else {
		SetParentDirectory(emGetParentPath(absPath));
		SetSelectedName(emGetNameInPath(absPath));
	}
}


void emFileSelectionBox::SetFilters(const emArray<emString> & filters)
{
	int i;

	if (Filters.GetCount()==filters.GetCount()) {
		for (i=Filters.GetCount()-1; i>=0; i--) {
			if (Filters[i]!=filters[i]) {
				break;
			}
		}
		if (i<0) return;
	}

	Filters=filters;
	if (SelectedFilterIndex>=Filters.GetCount()) {
		SelectedFilterIndex=Filters.GetCount()-1;
	}
	else if (SelectedFilterIndex<0 && Filters.GetCount()>0) {
		SelectedFilterIndex=0;
	}

	if (FiltersLB) {
		FiltersLB->ClearItems();
		for (i=0; i<Filters.GetCount(); i++) {
			FiltersLB->AddItem(Filters[i]);
		}
		FiltersLB->SetSelectedIndex(SelectedFilterIndex);
	}
	InvalidateListing();
}


void emFileSelectionBox::SetSelectedFilterIndex(int selectedFilterIndex)
{
	if (selectedFilterIndex<0 || selectedFilterIndex>=Filters.GetCount()) {
		selectedFilterIndex=-1;
	}
	if (SelectedFilterIndex!=selectedFilterIndex) {
		SelectedFilterIndex=selectedFilterIndex;
		if (FiltersLB) {
			FiltersLB->SetSelectedIndex(SelectedFilterIndex);
		}
		InvalidateListing();
	}
}


void emFileSelectionBox::SetHiddenFilesShown(bool hiddenFilesShown)
{
	if (HiddenFilesShown!=hiddenFilesShown) {
		HiddenFilesShown=hiddenFilesShown;
		if (HiddenCheckBox) {
			HiddenCheckBox->SetChecked(HiddenFilesShown);
		}
		InvalidateListing();
	}
}


void emFileSelectionBox::TriggerFile(const emString & name)
{
	emDLog("emFileSelectionBox::TriggerFile: %s",name.Get());
	TriggeredFileName=name;
	Signal(FileTriggerSignal);
}


void emFileSelectionBox::EnterSubDir(const emString & name)
{
	emString path;
	bool readable;

	emDLog("emFileSelectionBox::EnterSubDir: %s",name.Get());
	path=emGetChildPath(ParentDir,name);
	if (emIsDirectory(path)) {
		readable=emIsReadablePath(path);
#if defined(_WIN32)
		// emIsReadablePath does not care about NTFS permissions...
		if (readable) {
			WIN32_FIND_DATA d;
			HANDLE h=FindFirstFile(emGetChildPath(path,"*.*"),&d);
			if (h==INVALID_HANDLE_VALUE) {
				if (GetLastError()!=ERROR_NO_MORE_FILES) {
					readable=false;
				}
			}
			else {
				FindClose(h);
			}
		}
#endif
		if (readable) SetSelectedPath(path);
	}
}


void emFileSelectionBox::SetParentDirFieldHidden(bool parentDirFieldHidden)
{
	if (ParentDirFieldHidden!=parentDirFieldHidden) {
		ParentDirFieldHidden=parentDirFieldHidden;
		InvalidateAutoExpansion();
	}
}


void emFileSelectionBox::SetHiddenCheckBoxHidden(bool hiddenCheckBoxHidden)
{
	if (HiddenCheckBoxHidden!=hiddenCheckBoxHidden) {
		HiddenCheckBoxHidden=hiddenCheckBoxHidden;
		InvalidateAutoExpansion();
	}
}


void emFileSelectionBox::SetNameFieldHidden(bool nameFieldHidden)
{
	if (NameFieldHidden!=nameFieldHidden) {
		NameFieldHidden=nameFieldHidden;
		InvalidateAutoExpansion();
	}
}


void emFileSelectionBox::SetFilterHidden(bool filterHidden)
{
	if (FilterHidden!=filterHidden) {
		FilterHidden=filterHidden;
		InvalidateAutoExpansion();
	}
}


bool emFileSelectionBox::Cycle()
{
	emString name;
	bool busy;

	busy=emBorder::Cycle();


	if (ParentDirField && IsSignaled(ParentDirField->GetTextSignal())) {
		if (ParentDir!=ParentDirField->GetText()) {
			ParentDir=ParentDirField->GetText();
			TriggeredFileName.Clear();
			InvalidateListing();
			Signal(SelectionSignal);
		}
	}

	if (HiddenCheckBox && IsSignaled(HiddenCheckBox->GetCheckSignal())) {
		SetHiddenFilesShown(HiddenCheckBox->IsChecked());
	}

	if (ListingInvalid && FilesLB) {
		ReloadListing();
	}

	if (FilesLB && IsSignaled(FilesLB->GetSelectionSignal())) {
		if (!ListingInvalid) {
			SelectionFromListBox();
		}
	}

	if (FilesLB && IsSignaled(FilesLB->GetItemTriggerSignal())) {
		if (!ListingInvalid) {
			SelectionFromListBox();
			if (FilesLB->GetTriggeredItemIndex()>=0) {
				name=FilesLB->GetItemText(FilesLB->GetTriggeredItemIndex());
				if (emIsDirectory(emGetChildPath(ParentDir,name))) {
					EnterSubDir(name);
				}
				else {
					TriggerFile(name);
				}
			}
		}
	}

	if (NameField && IsSignaled(NameField->GetTextSignal())) {
		if (NameField->GetText().IsEmpty()) {
			if (SelectedNames.GetCount() == 1) {
				SetSelectedName("");
			}
		}
		else {
			if (
#if defined(_WIN32)
				(
					strchr(NameField->GetText().Get(),'\\') ||
					strchr(NameField->GetText().Get(),'/')
				) && (
					!ParentDir.IsEmpty() ||
					NameField->GetText().GetLen()!=3 ||
					NameField->GetText()[1]!=':'
				)
#else
				strchr(NameField->GetText().Get(),'/')
#endif
			) {
				SetSelectedPath(emGetAbsolutePath(NameField->GetText(),ParentDir));
				if (SelectedNames.GetCount()==1) {
					NameField->SetText(SelectedNames[0]);
				}
				else {
					NameField->SetText(emString());
				}
			}
			else {
				SetSelectedName(NameField->GetText());
			}
		}
	}

	if (FiltersLB && IsSignaled(FiltersLB->GetSelectionSignal())) {
		SetSelectedFilterIndex(FiltersLB->GetSelectedIndex());
	}

	return busy;
}


void emFileSelectionBox::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emBorder::Input(event,state,mx,my);

	if (
		event.GetKey()==EM_KEY_ENTER &&
		state.IsNoMod() &&
		NameField &&
		NameField->IsFocused() &&
		!NameField->GetText().IsEmpty()
	) {
		if (emIsDirectory(emGetChildPath(ParentDir,NameField->GetText()))) {
			EnterSubDir(NameField->GetText());
		}
		else {
			TriggerFile(NameField->GetText());
		}
		event.Eat();
	}
}


void emFileSelectionBox::AutoExpand()
{
	int i;

	emBorder::AutoExpand();

	if (!ParentDirFieldHidden) {
		ParentDirField=new emTextField(this,"directory","Directory");
		ParentDirField->SetEditable();
		ParentDirField->SetText(ParentDir);
		AddWakeUpSignal(ParentDirField->GetTextSignal());
	}

	if (!HiddenCheckBoxHidden) {
		HiddenCheckBox=new emCheckBox(this,"showHiddenFiles","Show\nHidden\nFiles");
		HiddenCheckBox->SetChecked(HiddenFilesShown);
		HiddenCheckBox->SetNoEOI();
		AddWakeUpSignal(HiddenCheckBox->GetCheckSignal());
	}

	FilesLB=new FilesListBox(*this,"files");
	FilesLB->SetCaption("Files");
	FilesLB->SetSelectionType(
		MultiSelectionEnabled ?
		emListBox::MULTI_SELECTION :
		emListBox::SINGLE_SELECTION
	);
	AddWakeUpSignal(FilesLB->GetSelectionSignal());
	AddWakeUpSignal(FilesLB->GetItemTriggerSignal());

	if (!NameFieldHidden) {
		NameField=new emTextField(this,"name","Name");
		NameField->SetEditable();
		if (SelectedNames.GetCount()==1) {
			NameField->SetText(SelectedNames[0]);
		}
		AddWakeUpSignal(NameField->GetTextSignal());
	}

	if (!FilterHidden) {
		FiltersLB=new emListBox(this,"filter","Filter");
		FiltersLB->SetMaxChildTallness(0.1);
		for (i=0; i<Filters.GetCount(); i++) {
			FiltersLB->AddItem(Filters[i]);
		}
		FiltersLB->SetSelectedIndex(SelectedFilterIndex);
		AddWakeUpSignal(FiltersLB->GetSelectionSignal());
	}

	InvalidateListing();
}


void emFileSelectionBox::AutoShrink()
{
	ParentDirField=NULL;
	HiddenCheckBox=NULL;
	FilesLB=NULL;
	NameField=NULL;
	FiltersLB=NULL;

	emBorder::AutoShrink();
}


void emFileSelectionBox::LayoutChildren()
{
	double x,y,w,w1,w2,h,hs,h1,h2,h3;
	emColor cc;

	emBorder::LayoutChildren();

	GetContentRectUnobscured(&x,&y,&w,&h,&cc);
	if (w<1E-100) w=1E-100;
	if (h<1E-100) h=1E-100;

	hs=emMin(w*0.05,h*0.15);
	if (ParentDirField || HiddenCheckBox) h1=hs; else h1=0.0;
	if (NameField || FiltersLB) h3=hs; else h3=0.0;
	h2=h-h1-h3;

	if (HiddenCheckBox) w2=emMin(w*0.5,h1*2.0); else w2=0.0;
	w1=w-w2;
	if (ParentDirField) ParentDirField->Layout(x,y,w1,h1,cc);
	if (HiddenCheckBox) HiddenCheckBox->Layout(x+w1,y,w2,h1,cc);

	if (FilesLB) {
		FilesLB->Layout(x,y+h1,w,h2,cc);
		FilesLB->SetBorderScaling(hs/h2);
	}

	if (FiltersLB) w2=emMin(w*0.5,h3*10.0); else w2=0.0;
	w1=w-w2;
	if (NameField) NameField->Layout(x,y+h1+h2,w1,h3,cc);
	if (FiltersLB) FiltersLB->Layout(x+w1,y+h1+h2,w2,h3,cc);
}


void emFileSelectionBox::InvalidateListing()
{
	ListingInvalid=true;
	WakeUp();
}


void emFileSelectionBox::ReloadListing()
{
	emArray<emString> names;
	FileItemData data;
	emString path;
	bool dirValid;
	int i;

	if (!FilesLB) return;

#if defined(_WIN32)
	if (ParentDir.IsEmpty()) {
		DWORD logicalDrives=::GetLogicalDrives();
		for (i=0; i<26; i++) {
			if ((1<<i)&logicalDrives) {
				names.Add(emString::Format("%c:\\",'A'+i));
			}
		}
	}
	else {
		try {
			names=emTryLoadDir(ParentDir);
			dirValid=true;
		}
		catch (emException &) {
			names.Clear();
			dirValid=false;
		}
		names.Sort(CompareNames,this);
		if (dirValid) names.Insert(0,"..");
	}
#else
	try {
		names=emTryLoadDir(ParentDir);
		dirValid=true;
	}
	catch (emException &) {
		names.Clear();
		dirValid=false;
	}
	names.Sort(CompareNames,this);
	if (ParentDir != "/" && dirValid) names.Insert(0,"..");
#endif

	for (i=0; i<names.GetCount(); ) {
		path=emGetChildPath(ParentDir,names[i]);
		if (names[i] == "..") {
			data.IsDirectory=true;
			data.IsReadable=true;
			data.IsHidden=false;
		}
		else {
			data.IsDirectory=emIsDirectory(path);
			data.IsReadable=emIsReadablePath(path);
			data.IsHidden=emIsHiddenPath(path);
		}

#if defined(_WIN32)
		if (ParentDir.IsEmpty()) {
			data.IsHidden=false;
		}
#endif

		if (!HiddenFilesShown && data.IsHidden) {
			names.Remove(i);
			continue;
		}

		if (
			SelectedFilterIndex>=0 &&
			SelectedFilterIndex<Filters.GetCount() &&
			!data.IsDirectory &&
			!MatchFileNameFilter(names[i],Filters[SelectedFilterIndex])
		) {
			names.Remove(i);
			continue;
		}

		if (i<FilesLB->GetItemCount()) {
			FilesLB->SetItemText(i,names[i]);
			FilesLB->SetItemData(
				i,
				emCastAnything<emFileSelectionBox::FileItemData>(data)
			);
		}
		else {
			FilesLB->AddItem(
				names[i],
				emCastAnything<emFileSelectionBox::FileItemData>(data)
			);
		}

		i++;
	}

	while (FilesLB->GetItemCount()>names.GetCount()) {
		FilesLB->RemoveItem(FilesLB->GetItemCount()-1);
	}

	ListingInvalid=false;

	SelectionToListBox();
}


void emFileSelectionBox::SelectionToListBox()
{
	emAvlTreeMap<emString,int> map;
	const emAvlTreeMap<emString,int>::Element * elem;
	bool selectionChanged;
	int i,j;

	if (!FilesLB || ListingInvalid) return;

	if (SelectedNames.GetCount()==FilesLB->GetSelectionCount()) {
		for (i=SelectedNames.GetCount()-1; i>=0; i--) {
			j=FilesLB->GetSelectedIndices()[i];
			if (SelectedNames[i]!=FilesLB->GetItemText(j)) {
				break;
			}
		}
		if (i<0) return;
	}

	if (SelectedNames.GetCount()==0) {
		FilesLB->ClearSelection();
	}
	else if (SelectedNames.GetCount()==1) {
		FilesLB->ClearSelection();
		for (i=0; i<FilesLB->GetItemCount(); i++) {
			if (FilesLB->GetItemText(i)==SelectedNames[0]) {
				FilesLB->Select(i);
			}
		}
	}
	else {
		for (i=0; i<FilesLB->GetItemCount(); i++) {
			map[FilesLB->GetItemText(i)]=i;
		}
		selectionChanged=false;
		FilesLB->ClearSelection();
		for (i=0; i<SelectedNames.GetCount(); ) {
			elem=map.Get(SelectedNames[i]);
			if (elem) {
				FilesLB->Select(elem->Value);
				i++;
			}
			else {
				SelectedNames.Remove(i);
				selectionChanged=true;
			}
		}
		if (selectionChanged) {
			if (NameField) {
				if (SelectedNames.GetCount()==1) {
					NameField->SetText(SelectedNames[0]);
				}
				else {
					NameField->SetText(emString());
				}
			}
			Signal(SelectionSignal);
		}
	}
}


void emFileSelectionBox::SelectionFromListBox()
{
	bool singleDeselected;
	int i,j;

	if (!FilesLB || ListingInvalid) return;

	if (SelectedNames.GetCount()==FilesLB->GetSelectionCount()) {
		for (i=SelectedNames.GetCount()-1; i>=0; i--) {
			j=FilesLB->GetSelectedIndices()[i];
			if (SelectedNames[i]!=FilesLB->GetItemText(j)) {
				break;
			}
		}
		if (i<0) return;
	}

	if (FilesLB->GetSelectionCount()==0 && SelectedNames.GetCount()==1) {
		singleDeselected=false;
		for (i=0; i<FilesLB->GetItemCount(); i++) {
			if (FilesLB->GetItemText(i)==SelectedNames[0]) {
				singleDeselected=true;
			}
		}
		if (!singleDeselected) return;
	}

	SelectedNames.SetCount(FilesLB->GetSelectionCount());
	for (i=0; i<FilesLB->GetSelectionCount(); i++) {
		j=FilesLB->GetSelectedIndices()[i];
		SelectedNames.Set(i,FilesLB->GetItemText(j));
	}

	if (NameField) {
		if (SelectedNames.GetCount()==1) {
			NameField->SetText(SelectedNames[0]);
		}
		else {
			NameField->SetText(emString());
		}
	}

	Signal(SelectionSignal);
}


int emFileSelectionBox::CompareNames(
	const emString * name1, const emString * name2, void * context
)
{
	return strcoll(name1->Get(), name2->Get());
}


bool emFileSelectionBox::MatchFileNameFilter(
	const char * fileName, const char * filter
)
{
	const char * s, * e, * m;

	s=strrchr(filter,'(');
	e=strrchr(filter,')');
	if (s && e && s<e) {
		s++;
	}
	else {
		s=filter;
		e=filter+strlen(filter);
	}

	while (s<e) {
		while (s<e && ((unsigned char)*s)<=32) s++;
		for (m=s; m<e && ((unsigned char)*m)>32 && *m!='|' && *m!=';' && *m!=','; m++);
		if (MatchFileNamePattern(fileName,s,m)) return true;
		s=m+1;
	}
	return false;
}


bool emFileSelectionBox::MatchFileNamePattern(
	const char * fileName, const char * pattern, const char * patternEnd
)
{
	for (;; fileName++, pattern++) {
		if (pattern>=patternEnd) {
			return fileName[0]==0;
		}
		if (pattern[0]=='*') {
			for (;; fileName++) {
				if (MatchFileNamePattern(fileName,pattern+1,patternEnd)) return true;
				if (fileName[0]==0) return false;
			}
		}
		if (fileName[0]!=pattern[0]) {
			if (tolower((unsigned char)fileName[0]) !=
			    tolower((unsigned char)pattern[0])) return false;
		}
		if (fileName[0]==0) return true;
	}
}


emFileSelectionBox::FilesListBox::FilesListBox(
	emFileSelectionBox & parent, const emString & name
) :
	emListBox(parent, name)
{
	SetMinCellCount(4);
	SetChildTallness(0.6);
	SetAlignment(EM_ALIGN_TOP_LEFT);
}


emFileSelectionBox::FilesListBox::~FilesListBox()
{
}


void emFileSelectionBox::FilesListBox::CreateItemPanel(
	const emString & name, int itemIndex
)
{
	new FileItemPanel(*this,name,itemIndex);
}


emFileSelectionBox::FileItemPanel::FileItemPanel(
	FilesListBox & listBox, const emString & name, int itemIndex
) :
	emPanel(listBox,name),
	emListBox::ItemPanelInterface(listBox,itemIndex),
	ListBox(listBox)
{
	FilePanel=NULL;
	OverlayPanel=NULL;
}


emFileSelectionBox::FileItemPanel::~FileItemPanel()
{
}


void emFileSelectionBox::FileItemPanel::Notice(NoticeFlags flags)
{
	emPanel::Notice(flags);

	if (flags&(NF_ACTIVE_CHANGED|NF_VIEWING_CHANGED)) {
		// This is a hack or to be defined as a design pattern.
		if (IsInActivePath() && !IsActive() && IsViewed()) {
			emDLog("emFileSelectionBox::FileItemPanel::Notice: Stealing activation...");
			Activate(GetView().IsActivationAdherent());
		}
	}

	if (flags&NF_ENABLE_CHANGED) {
		InvalidateAutoExpansion();
		InvalidatePainting();
	}
}


void emFileSelectionBox::FileItemPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	ProcessItemInput(this,event,state);
	emPanel::Input(event,state,mx,my);
}


bool emFileSelectionBox::FileItemPanel::IsOpaque() const
{
	return false;
}


void emFileSelectionBox::FileItemPanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double h,fx,fy,fw,fh,r,t;
	const FileItemData * data;
	const emImage * img;
	emColor bgCol, fgCol;

	data=emCastAnything<emFileSelectionBox::FileItemData>(GetItemData());

	h=emMax(1E-3,GetHeight());

	fgCol=GetFgColor();

	if (IsItemSelected()) {
		fx=emMin(1.0,h)*0.015;
		fw=1.0-2.0*fx;
		fy=fx;
		fh=h-2.0*fy;
		r=emMin(1.0,h)*0.1;
		bgCol=GetBgColor();
		painter.PaintRoundRect(
			fx,fy,fw,fh,r,r,
			bgCol,canvasColor
		);
		canvasColor=bgCol;
	}

	fx=0.06;
	fw=1.0-2.0*fx;
	fy=h*0.77;
	fh=h-fy-h*0.05;
	painter.PaintTextBoxed(
		fx,fy,fw,fh,
		GetItemText(),
		h,
		fgCol,
		canvasColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);

	if (data->IsDirectory) {
		if (GetItemText()=="..") {
			img=&ListBox.GetDirUpImage();
		}
		else {
			img=&ListBox.GetDirImage();
		}
	}
	else {
		img=NULL;
	}

	if (img) {
		fx=0.06;
		fw=1.0-2.0*fx;
		fy=h*0.1;
		fh=h*0.62;
		t=img->GetHeight()/(double)img->GetWidth();
		if (fh/fw < t) {
			fw=fh/t;
			fx=(1.0-fw)*0.5;
		}
		else {
			fy+=(fh-fw*t)*0.5;
			fh=fw*t;
		}
		painter.PaintShape(
			fx,fy,fw,fh,
			*img, 0, fgCol, canvasColor
		);
		if (!data->IsReadable) {
			r=emMin(fw,fh)*0.35;
			fx=fx+fw*0.5;
			fy=fy+fh*0.5;
			painter.PaintEllipseOutline(
				fx-r,fy-r,r*2,r*2,
				r*0.26,
				fgCol
			);
			t=r*sqrt(0.5);
			painter.PaintLine(
				fx-t,fy-t,fx+t,fy+t,
				r*0.22,
				emPainter::LC_FLAT,
				emPainter::LC_FLAT,
				fgCol
			);
		}
	}
}


void emFileSelectionBox::FileItemPanel::AutoExpand()
{
	const FileItemData * data;
	emString name,path;

	emPanel::AutoExpand();

	if (!IsEnabled()) return;

	data=emCastAnything<emFileSelectionBox::FileItemData>(GetItemData());
	if (data->IsDirectory) return;

	name=GetItemText();
	path=emGetChildPath(ListBox.GetFileSelectionBox().GetParentDirectory(), name);

	emRef<emFpPluginList> fppl=emFpPluginList::Acquire(GetRootContext());
	FilePanel=fppl->CreateFilePanel(this,"content",path);

	OverlayPanel=new FileOverlayPanel(*this,"overlay");
}


void emFileSelectionBox::FileItemPanel::AutoShrink()
{
	FilePanel=NULL;
	OverlayPanel=NULL;
	emPanel::AutoShrink();
}


void emFileSelectionBox::FileItemPanel::LayoutChildren()
{
	double h,fx,fy,fw,fh;

	if (FilePanel) {
		h=emMax(1E-3,GetHeight());
		fx=0.06;
		fw=1.0-2.0*fx;
		fy=h*0.1;
		fh=h*0.62;
		fw=emMin(fw,fh*16.0/9.0);
		fx=(1.0-fw)*0.5;
		FilePanel->Layout(fx,fy,fw,fh,GetBgColor());
	}
	if (OverlayPanel) {
		OverlayPanel->Layout(0.0,0.0,1.0,GetHeight());
	}
}


void emFileSelectionBox::FileItemPanel::ItemTextChanged()
{
	InvalidateAutoExpansion();
	InvalidatePainting();
}


void emFileSelectionBox::FileItemPanel::ItemDataChanged()
{
	InvalidateAutoExpansion();
	InvalidatePainting();
}


void emFileSelectionBox::FileItemPanel::ItemSelectionChanged()
{
	InvalidateChildrenLayout();
	InvalidatePainting();
}


emColor emFileSelectionBox::FileItemPanel::GetBgColor() const
{
	emColor bgColor;

	if (IsItemSelected()) {
		if (GetListBox().GetSelectionType()==emListBox::READY_ONLY_SELECTION) {
			bgColor=GetListBox().GetLook().GetOutputHlColor();
		}
		else {
			bgColor=GetListBox().GetLook().GetInputHlColor();
		}
		if (!IsEnabled()) {
			bgColor=bgColor.GetBlended(GetListBox().GetLook().GetBgColor(),80.0F);
		}
	}
	else {
		bgColor=GetCanvasColor();
	}
	return bgColor;
}


emColor emFileSelectionBox::FileItemPanel::GetFgColor() const
{
	const FileItemData * data;
	emColor fgColor;

	if (GetListBox().GetSelectionType()==emListBox::READY_ONLY_SELECTION) {
		if (IsItemSelected()) {
			fgColor=GetListBox().GetLook().GetOutputBgColor();
		}
		else {
			fgColor=GetListBox().GetLook().GetOutputFgColor();
		}
	}
	else {
		if (IsItemSelected()) {
			fgColor=GetListBox().GetLook().GetInputBgColor();
		}
		else {
			fgColor=GetListBox().GetLook().GetInputFgColor();
		}
	}

	if (!IsEnabled()) {
		fgColor=fgColor.GetBlended(GetListBox().GetLook().GetBgColor(),80.0F);
	}

	data=emCastAnything<emFileSelectionBox::FileItemData>(GetItemData());
	if (data->IsHidden) {
		fgColor=fgColor.GetTransparented(27.0F);
	}

	return fgColor;
}


emFileSelectionBox::FileOverlayPanel::FileOverlayPanel(
	FileItemPanel & parent, const emString & name
) :
	emPanel(parent,name)
{
	SetFocusable(false);
}


emFileSelectionBox::FileOverlayPanel::~FileOverlayPanel()
{
}


void emFileSelectionBox::FileOverlayPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	((FileItemPanel*)GetParent())->ProcessItemInput(this,event,state);
	if (event.IsMouseEvent() || event.IsTouchEvent()) {
		Focus();
		event.Eat();
	}
	emPanel::Input(event,state,mx,my);
}
