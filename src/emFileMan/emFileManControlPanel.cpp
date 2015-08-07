//------------------------------------------------------------------------------
// emFileManControlPanel.cpp
//
// Copyright (C) 2006-2008,2010,2014-2015 Oliver Hamann.
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

#include <emFileMan/emFileManControlPanel.h>
#include <emFileMan/emFileManSelInfoPanel.h>
#include <emFileMan/emDirPanel.h>


emFileManControlPanel::emFileManControlPanel(
	ParentArg parent, const emString & name, emView & contentView
)
	: emPackLayout(parent,name),
	ContentView(contentView)
{
	emRasterGroup * sortByGroup, * nameSortingStyleGroup;
	emLinearGroup * saveGroup;
	emRasterLayout * sortOptLayout;
	emTunnel * tunnel;
	emRef<emFileManThemeNames> themeNames;
	int i;

	FMModel=emFileManModel::Acquire(GetRootContext());
	FMVConfig=emFileManViewConfig::Acquire(contentView);

	SetPrefChildTallness(0, 0.66);
	SetPrefChildTallness(1, 0.97);
	SetPrefChildTallness(2, 0.195);
	SetChildWeight(0, 1.0/0.66);
	SetChildWeight(1, 1.0/0.97);
	SetChildWeight(2, 1.0/0.195);

	GrView=new emPackGroup(this,"view","File View Settings");
	GrView->SetPrefChildTallness(0, 0.4);
	GrView->SetPrefChildTallness(1, 0.8);
	GrView->SetPrefChildTallness(2, 0.3);
	GrView->SetPrefChildTallness(3, 0.4);
	GrView->SetPrefChildTallness(4, 0.4);
	GrView->SetChildWeight(0, 70.0);
	GrView->SetChildWeight(1,  6.0);
	GrView->SetChildWeight(2, 16.0);
	GrView->SetChildWeight(3,  3.0);
	GrView->SetChildWeight(4,  3.0);
		sortByGroup=new emRasterGroup(GrView,"sort","Sort Files By");
		sortByGroup->SetBorderScaling(1.7);
		sortByGroup->SetPrefChildTallness(0.2);
			RbSortByName=new emRadioButton(
				sortByGroup,"sort by name",
				"Name",
				"Sort by file name.\n"
				"\n"
				"Hotkey: Shift+Alt+N"
			);
			RbSortByEnding=new emRadioButton(
				sortByGroup,"sort by ending",
				"Ending",
				"Sort by file name ending. It's the part after the last dot.\n"
				"Files with equal endings are sorted by name.\n"
				"\n"
				"Hotkey: Shift+Alt+E"
			);
			RbSortByClass=new emRadioButton(
				sortByGroup,"sort by class",
				"Class",
				"Sort by class names in the file names. This sorting algorithm\n"
				"splits each file name into words and sorts primarily by the\n"
				"last word, secondarily by the next-to-last word and so on.\n"
				"\n"
				"Hotkey: Shift+Alt+C"
			);
			RbSortByVersion=new emRadioButton(
				sortByGroup,"sort by version",
				"Version",
				"Sort by version number. This is like sorting by name, but\n"
				"decimal numbers are recognized and compared. For example,\n"
				"\"a-2.10.xy\" comes after \"a-2.9.xy\".\n"
				"\n"
				"Hotkey: Shift+Alt+V"
			);
			RbSortByDate=new emRadioButton(
				sortByGroup,"sort by date",
				"Date",
				"Sort primarily by date and time of last modification,\n"
				"secondarily by name.\n"
				"\n"
				"Hotkey: Shift+Alt+D"
			);
			RbSortBySize=new emRadioButton(
				sortByGroup,"sort by size",
				"Size",
				"Sort primarily by file size, secondarily by name.\n"
				"\n"
				"Hotkey: Shift+Alt+S"
			);
		RbgTheme=new emRadioButton::RasterGroup(
			GrView,"theme","Theme",
			"Here you can choose the look of directory entry panels."
		);
		RbgTheme->SetPrefChildTallness(0.2);
		themeNames=emFileManThemeNames::Acquire(GetRootContext());
		for (i=0; i<themeNames->GetThemeCount(); i++) {
			new emRadioButton(
				RbgTheme,
				themeNames->GetThemeName(i),
				themeNames->GetThemeDisplayName(i)
			);
		}
		sortOptLayout=new emRasterLayout(GrView,"left");
		sortOptLayout->SetPrefChildTallness(0.15);
			CbSortDirectoriesFirst=new emCheckButton(
				sortOptLayout,"sort directories first",
				"Directories First",
				"Always have directories at the beginning, regardless of\n"
				"the other sort criterions."
			);
			CbShowHiddenFiles=new emCheckButton(
				sortOptLayout,"show hidden files",
				"Show Hidden Files",
				"Hotkey: Shift+Alt+H"
			);
		nameSortingStyleGroup=new emRasterGroup(
			GrView,"nameSortingStyle",
			"File Name Sorting Style (Order of Characters)"
		);
		nameSortingStyleGroup->SetBorderScaling(2.0);
		nameSortingStyleGroup->SetPrefChildTallness(0.05);
			RbPerLocale=new emRadioButton(
				nameSortingStyleGroup,"per locale",
				"Per Locale",
				"Sort names depending on the user's current locale.\n"
				"Technically, names are compared through \"strcoll\"."
			);
			RbCaseSensitive=new emRadioButton(
				nameSortingStyleGroup,"case-sensitive",
				"Classic Case-Sensitive",
				"Sort names by the ASCII code of characters.\n"
				"Technically, names are compared through \"strcmp\"."
			);
			RbCaseInsensitive=new emRadioButton(
				nameSortingStyleGroup,"case-insensitive",
				"Classic Case-Insensitive",
				"Sort names by the ASCII code of characters, but ignore the\n"
				"letter case. This may be correct for English letters only.\n"
				"Technically, names are compared through \"strcasecmp\"."
			);
		tunnel=new emTunnel(GrView,"tunnel","Save File View Settings");
			saveGroup=new emLinearGroup(tunnel,"save");
			saveGroup->SetVertical();
				CbAutosave=new emCheckBox(
					saveGroup,"autosave",
					"Save Automatically",
					"Automatically save changes of the file view settings as the default for new windows."
				);
				CbAutosave->SetNoEOI();
				BtSaveAsDefault=new emButton(
					saveGroup,"save",
					"Save",
					"Save the current file view settings as the default for new windows."
				);

	GrSelection=new emLinearGroup(this,"selection","File Selection");
	GrSelection->SetVertical();
	GrSelection->SetChildWeight(0.17);
	GrSelection->SetChildWeight(5,0.28);
		BtSelectAll=new emButton(
			GrSelection,"select all",
			"Select All",
			"Select all entries in the directory content panel which is active\n"
			"(focused) itself, or which is the parent of an active directory\n"
			"entry panel. As usual, the entries are selected as the target and\n"
			"the source selection is set from the previous target selection.\n"
			"\n"
			"Hotkey: Alt+A"
		);
		BtClearSelection=new emButton(
			GrSelection,"clear selection",
			"Clear Selection",
			"Clear the source and target selections.\n"
			"\n"
			"Hotkey: Alt+E"
		);
		BtSwapSelection=new emButton(
			GrSelection,"swap selection",
			"Swap Selection",
			"Exchange the source selection for the target selection.\n"
			"\n"
			"Hotkey: Alt+Z"
		);
		BtPaths2Clipboard=new emButton(
			GrSelection,"paths2clipboard",
			"Copy Paths",
			"Copy the full file path(s) of the target selection to the clipboard.\n"
			"Multiple entries are separated by line feeds.\n"
			"\n"
			"Hotkey: Alt+P"
		);
		BtNames2Clipboard=new emButton(
			GrSelection,"names2clipboard",
			"Copy Names",
			"Copy the file name(s) of the target selection to the clipboard.\n"
			"Multiple entries are separated by line feeds.\n"
			"\n"
			"Hotkey: Alt+N"
		);
		GrSelInfo=new emLinearGroup(GrSelection,"stat","Selection Statistics");
		GrSelInfo->SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
			SelInfo=new emFileManSelInfoPanel(GrSelInfo,"info");
			SelInfo->SetFocusable(false);

	GrCommand=new Group(this,"commands",contentView,FMModel,FMModel->GetCommandRoot());
	GrCommand->SetCaption("File Manager Commands");

	AddWakeUpSignal(FMModel->GetSelectionSignal());
	AddWakeUpSignal(FMVConfig->GetChangeSignal());
	AddWakeUpSignal(RbSortByName->GetClickSignal());
	AddWakeUpSignal(RbSortByEnding->GetClickSignal());
	AddWakeUpSignal(RbSortByClass->GetClickSignal());
	AddWakeUpSignal(RbSortByVersion->GetClickSignal());
	AddWakeUpSignal(RbSortByDate->GetClickSignal());
	AddWakeUpSignal(RbSortBySize->GetClickSignal());
	AddWakeUpSignal(RbPerLocale->GetClickSignal());
	AddWakeUpSignal(RbCaseSensitive->GetClickSignal());
	AddWakeUpSignal(RbCaseInsensitive->GetClickSignal());
	AddWakeUpSignal(CbSortDirectoriesFirst->GetCheckSignal());
	AddWakeUpSignal(CbShowHiddenFiles->GetCheckSignal());
	AddWakeUpSignal(RbgTheme->GetCheckSignal());
	AddWakeUpSignal(CbAutosave->GetCheckSignal());
	AddWakeUpSignal(BtSaveAsDefault->GetClickSignal());
	AddWakeUpSignal(BtSelectAll->GetClickSignal());
	AddWakeUpSignal(BtClearSelection->GetClickSignal());
	AddWakeUpSignal(BtSwapSelection->GetClickSignal());
	AddWakeUpSignal(BtPaths2Clipboard->GetClickSignal());
	AddWakeUpSignal(BtNames2Clipboard->GetClickSignal());

	UpdateButtonStates();
}


emFileManControlPanel::~emFileManControlPanel()
{
}


bool emFileManControlPanel::Cycle()
{
	const emRadioButton * rb;
	emScreen * screen;
	emDirPanel * dp;
	emPanel * p;

	if (
		IsSignaled(FMModel->GetSelectionSignal()) ||
		IsSignaled(FMVConfig->GetChangeSignal())
	) {
		UpdateButtonStates();
	}
	if (IsSignaled(RbSortByName->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_NAME);
	}
	if (IsSignaled(RbSortByEnding->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_ENDING);
	}
	if (IsSignaled(RbSortByClass->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_CLASS);
	}
	if (IsSignaled(RbSortByVersion->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_VERSION);
	}
	if (IsSignaled(RbSortByDate->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_DATE);
	}
	if (IsSignaled(RbSortBySize->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_SIZE);
	}
	if (IsSignaled(RbPerLocale->GetClickSignal())) {
		FMVConfig->SetNameSortingStyle(emFileManViewConfig::NSS_PER_LOCALE);
	}
	if (IsSignaled(RbCaseSensitive->GetClickSignal())) {
		FMVConfig->SetNameSortingStyle(emFileManViewConfig::NSS_CASE_SENSITIVE);
	}
	if (IsSignaled(RbCaseInsensitive->GetClickSignal())) {
		FMVConfig->SetNameSortingStyle(emFileManViewConfig::NSS_CASE_INSENSITIVE);
	}
	if (IsSignaled(CbSortDirectoriesFirst->GetCheckSignal())) {
		FMVConfig->SetSortDirectoriesFirst(CbSortDirectoriesFirst->IsChecked());
	}
	if (IsSignaled(CbShowHiddenFiles->GetCheckSignal())) {
		FMVConfig->SetShowHiddenFiles(CbShowHiddenFiles->IsChecked());
	}
	if (IsSignaled(RbgTheme->GetCheckSignal())) {
		rb=RbgTheme->GetChecked();
		if (rb) FMVConfig->SetThemeName(rb->GetName());
	}
	if (IsSignaled(CbAutosave->GetCheckSignal())) {
		FMVConfig->SetAutosave(CbAutosave->IsChecked());
	}
	if (IsSignaled(BtSaveAsDefault->GetClickSignal())) {
		FMVConfig->SaveAsDefault();
	}
	if (IsSignaled(BtSelectAll->GetClickSignal())) {
		dp=NULL;
		for (p=ContentView.GetActivePanel(); p; p=p->GetParent()) {
			dp=dynamic_cast<emDirPanel*>(p);
			if (dp) break;
		}
		if (dp && dp->IsContentComplete()) {
			dp->SelectAll();
		}
		else {
			screen=GetScreen();
			if (screen) screen->Beep();
		}
	}
	if (IsSignaled(BtClearSelection->GetClickSignal())) {
		FMModel->ClearSourceSelection();
		FMModel->ClearTargetSelection();
	}
	if (IsSignaled(BtSwapSelection->GetClickSignal())) {
		FMModel->SwapSelection();
	}
	if (IsSignaled(BtPaths2Clipboard->GetClickSignal())) {
		FMModel->SelectionToClipboard(ContentView,false,false);
	}
	if (IsSignaled(BtNames2Clipboard->GetClickSignal())) {
		FMModel->SelectionToClipboard(ContentView,false,true);
	}
	return emPackLayout::Cycle();
}


void emFileManControlPanel::UpdateButtonStates()
{
	emPanel * p;

	RbSortByName->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_NAME);
	RbSortByEnding->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_ENDING);
	RbSortByClass->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_CLASS);
	RbSortByVersion->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_VERSION);
	RbSortByDate->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_DATE);
	RbSortBySize->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_SIZE);

	RbPerLocale->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_PER_LOCALE);
	RbCaseSensitive->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_CASE_SENSITIVE);
	RbCaseInsensitive->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_CASE_INSENSITIVE);

	CbSortDirectoriesFirst->SetChecked(FMVConfig->GetSortDirectoriesFirst());
	CbShowHiddenFiles->SetChecked(FMVConfig->GetShowHiddenFiles());

	p=RbgTheme->GetChild(FMVConfig->GetThemeName());
	RbgTheme->SetChecked(p ? dynamic_cast<emRadioButton*>(p) : NULL);

	CbAutosave->SetChecked(FMVConfig->GetAutosave());
	BtSaveAsDefault->SetEnableSwitch(FMVConfig->IsUnsaved());

	BtClearSelection->SetEnableSwitch(
		FMModel->GetSourceSelectionCount()>0 ||
		FMModel->GetTargetSelectionCount()>0
	);
	BtSwapSelection->SetEnableSwitch(
		FMModel->GetSourceSelectionCount()>0 ||
		FMModel->GetTargetSelectionCount()>0
	);
	BtPaths2Clipboard->SetEnableSwitch(FMModel->GetTargetSelectionCount()>0);
	BtNames2Clipboard->SetEnableSwitch(FMModel->GetTargetSelectionCount()>0);
}


emFileManControlPanel::Group::Group(
	ParentArg parent, const emString & name, emView & contentView,
	emFileManModel * fmModel, const emFileManModel::CommandNode * cmd
)
	: emRasterGroup(parent,name,cmd->Caption,cmd->Description,cmd->Icon),
	ContentView(contentView)
{
	SetLook(cmd->Look);
	SetBorderScaling(cmd->BorderScaling);
	if (cmd->BorderScaling<=0.0) {
		SetBorderType(OBT_NONE,IBT_NONE);
		SetFocusable(false);
	}
	SetPrefChildTallness(cmd->PrefChildTallness);
	SetStrictRaster(true);
	SetAlignment(EM_ALIGN_TOP_LEFT);
	FMModel=fmModel;
	CmdPath=cmd->CmdPath;
	AddWakeUpSignal(FMModel->GetCommandsSignal());
}


emFileManControlPanel::Group::~Group()
{
}


bool emFileManControlPanel::Group::Cycle()
{
	if (IsSignaled(FMModel->GetCommandsSignal())) {
		InvalidateAutoExpansion();
	}
	return emRasterGroup::Cycle();
}


void emFileManControlPanel::Group::AutoExpand()
{
	const emFileManModel::CommandNode * cmd;
	emArray<const emFileManModel::CommandNode *> cmds;
	char chName[256];
	int i;

	cmd=FMModel->GetCommand(CmdPath);
	if (!cmd) return;
	cmds=cmd->Children;
	for (i=0; i<cmds.GetCount(); i++) {
		cmd=cmds[i];
		sprintf(chName,"%d",i);
		switch (cmd->Type) {
		case emFileManModel::CT_COMMAND:
			new Button(this,chName,ContentView,FMModel,cmd);
			break;
		case emFileManModel::CT_GROUP:
			new Group(this,chName,ContentView,FMModel,cmd);
			break;
		case emFileManModel::CT_SEPARATOR:
			new emPanel(this,chName);
			break;
		}
	}
}


emFileManControlPanel::Group::Button::Button(
	ParentArg parent, const emString & name, emView & contentView,
	emFileManModel * fmModel, const emFileManModel::CommandNode * cmd
)
	: emButton(parent,name,cmd->Caption,cmd->Description,cmd->Icon),
	ContentView(contentView)
{
	SetLook(cmd->Look);
	SetBorderScaling(cmd->BorderScaling);
	FMModel=fmModel;
	CmdPath=cmd->CmdPath;
}


emFileManControlPanel::Group::Button::~Button()
{
}


void emFileManControlPanel::Group::Button::Clicked()
{
	const emFileManModel::CommandNode * cmd;

	cmd=FMModel->GetCommand(CmdPath);
	if (cmd) FMModel->RunCommand(cmd,ContentView);
}
