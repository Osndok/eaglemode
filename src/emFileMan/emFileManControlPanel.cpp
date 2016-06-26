//------------------------------------------------------------------------------
// emFileManControlPanel.cpp
//
// Copyright (C) 2006-2008,2010,2014-2016 Oliver Hamann.
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
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emFileMan/emFileManSelInfoPanel.h>
#include <emFileMan/emDirPanel.h>


emFileManControlPanel::emFileManControlPanel(
	ParentArg parent, const emString & name, emView & contentView
)
	: emLinearLayout(parent,name),
	ContentView(contentView)
{
	emLinearLayout * vsLayout, * saveGroup;
	emRasterLayout * themeLayout, * themeLayout2, * sortLayout, * sortLayout2;
	emRasterGroup * nameSortingStyleGroup;
	emRadioButton * rb;
	emTunnel * tunnel;
	emImage image;
	emLook look;
	int i;

	FMModel=emFileManModel::Acquire(GetRootContext());
	FMVConfig=emFileManViewConfig::Acquire(contentView);
	FMThemeNames=emFileManThemeNames::Acquire(GetRootContext());

	SetOrientationThresholdTallness(0.43);
	SetChildWeight(0, 8.16);
	SetChildWeight(1, 13.0);
	SetInnerSpace(0.015,0.015);

	vsLayout=new emLinearLayout(this,"vs");
	vsLayout->SetOrientationThresholdTallness(0.8);
	vsLayout->SetChildWeight(0, 5.0);
	vsLayout->SetChildWeight(1, 3.0);
	vsLayout->SetInnerSpace(0.04,0.04);

	GrView=new emPackGroup(vsLayout,"view","File View Settings");
	GrView->SetPrefChildTallness(0, 0.7);
	GrView->SetPrefChildTallness(1, 0.7);
	GrView->SetPrefChildTallness(2, 0.7);
	GrView->SetPrefChildTallness(3, 0.35);
	GrView->SetPrefChildTallness(4, 0.35);
	GrView->SetChildWeight(0, 4.0);
	GrView->SetChildWeight(1, 4.0);
	GrView->SetChildWeight(2, 1.0);
	GrView->SetChildWeight(3, 0.5);
	GrView->SetChildWeight(4, 0.5);
		themeLayout=new emRasterLayout(GrView,"theme");
		themeLayout->SetPrefChildTallness(0.7);
		rb=new emRadioButton(
			themeLayout,"aspect ratio 1",emString(),
			"Set the aspect ratio of file panels to the highest value.\n"
			"This may be best for managing files, because the file\n"
			"names get a bigger proportion of the display area, than\n"
			"with the other selectable aspect ratios.",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/aspect_ratio_1.tga")
		);
		rb->SetIconAboveCaption();
		rb->SetBorderScaling(0.5);
		RbmAspect.Add(rb);
		rb=new emRadioButton(
			themeLayout,"aspect ratio 2",emString(),
			"Set the aspect ratio of file panels to a low value.\n"
			"This may be best for viewing the contents of many files\n"
			"(e.g. photos), which have similar aspect ratios.",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/aspect_ratio_2.tga")
		);
		rb->SetIconAboveCaption();
		rb->SetBorderScaling(0.5);
		RbmAspect.Add(rb);
		rb=new emRadioButton(
			themeLayout,"aspect ratio 3",emString(),
			"Set the aspect ratio of file panels to the lowest value.\n"
			"This may be best for viewing the contents of many files\n"
			"(e.g. photos), which have similar aspect ratios.",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/aspect_ratio_3.tga")
		);
		rb->SetIconAboveCaption();
		rb->SetBorderScaling(0.5);
		RbmAspect.Add(rb);
		themeLayout2=new emRasterGroup(
			themeLayout,"theme","Theme",
			"Here you can choose the look of the directory entry panels."
		);
		themeLayout2->SetPrefChildTallness(0.7);
		for (i=0; i<FMThemeNames->GetThemeStyleCount(); i++) {
			image.Clear();
			if (!FMThemeNames->GetThemeStyleDisplayIcon(i).IsEmpty()) {
				image=emGetResImage(
					GetRootContext(),
					emGetChildPath(
						emGetInstallPath(EM_IDT_RES,"emFileMan","icons"),
						FMThemeNames->GetThemeStyleDisplayIcon(i)
					)
				);
			}
			rb=new emRadioButton(
				themeLayout2,
				FMThemeNames->GetThemeStyleDisplayName(i),
				FMThemeNames->GetThemeStyleDisplayName(i),
				emString(),
				image
			);
			rb->SetIconAboveCaption();
			rb->SetBorderScaling(0.5);
			RbmTheme.Add(rb);
		}
		sortLayout=new emRasterLayout(GrView,"sort");
		sortLayout->SetPrefChildTallness(0.7);
			RbSortByName=new emRadioButton(
				sortLayout,"sort by name",
				"Sort By Name",
				"Sort by file name.\n"
				"\n"
				"Hotkey: Shift+Alt+N",
				emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_name.tga")
			);
			RbSortByName->SetIconAboveCaption();
			RbSortByName->SetBorderScaling(0.5);
			RbSortByDate=new emRadioButton(
				sortLayout,"sort by date",
				"Sort By Date",
				"Sort primarily by date and time of last modification,\n"
				"secondarily by name.\n"
				"\n"
				"Hotkey: Shift+Alt+D",
				emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_date.tga")
			);
			RbSortByDate->SetIconAboveCaption();
			RbSortByDate->SetBorderScaling(0.5);
			RbSortBySize=new emRadioButton(
				sortLayout,"sort by size",
				"Sort By Size",
				"Sort primarily by file size, secondarily by name.\n"
				"\n"
				"Hotkey: Shift+Alt+S",
				emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_size.tga")
			);
			RbSortBySize->SetIconAboveCaption();
			RbSortBySize->SetBorderScaling(0.5);
			sortLayout2=new emRasterLayout(sortLayout,"more");
			sortLayout2->SetPrefChildTallness(0.7);
				RbSortByEnding=new emRadioButton(
					sortLayout2,"sort by ending",
					"Sort By Ending",
					"Sort by file name ending. It's the part after the last dot.\n"
					"Files with equal endings are sorted by name.\n"
					"\n"
					"Hotkey: Shift+Alt+E",
					emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_ending.tga")
				);
				RbSortByEnding->SetIconAboveCaption();
				RbSortByEnding->SetBorderScaling(0.5);
				RbSortByClass=new emRadioButton(
					sortLayout2,"sort by class",
					"Sort By Class",
					"Sort by class names in the file names. This sorting algorithm\n"
					"splits each file name into words and sorts primarily by the\n"
					"last word, secondarily by the next-to-last word and so on.\n"
					"\n"
					"Hotkey: Shift+Alt+C",
					emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_class.tga")
				);
				RbSortByClass->SetIconAboveCaption();
				RbSortByClass->SetBorderScaling(0.5);
				RbSortByVersion=new emRadioButton(
					sortLayout2,"sort by version",
					"Sort By Version",
					"Sort by version number. This is like sorting by name, but\n"
					"decimal numbers are recognized and compared. For example,\n"
					"\"a-2.10.xy\" comes after \"a-2.9.xy\".\n"
					"\n"
					"Hotkey: Shift+Alt+V",
					emGetInsResImage(GetRootContext(),"emFileMan","icons/sort_by_version.tga")
				);
				RbSortByVersion->SetIconAboveCaption();
				RbSortByVersion->SetBorderScaling(0.5);
				CbSortDirectoriesFirst=new emCheckButton(
					sortLayout2,"sort directories first",
					"Directories First",
					"Always have directories at the beginning, regardless of\n"
					"the other sort criterions.",
					emGetInsResImage(GetRootContext(),"emFileMan","icons/directories_first.tga")
				);
				CbSortDirectoriesFirst->SetIconAboveCaption();
				CbSortDirectoriesFirst->SetBorderScaling(0.5);
		CbShowHiddenFiles=new emCheckButton(
			GrView,"show hidden files",
			"Show Hidden Files",
			"Hotkey: Shift+Alt+H",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/show_hidden_files.tga")
		);
		CbShowHiddenFiles->SetIconAboveCaption();
		CbShowHiddenFiles->SetBorderScaling(0.5);
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
		tunnel->SetBorderScaling(0.57);
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

	GrSelection=new emRasterGroup(vsLayout,"selection","File Selection");
	GrSelection->SetPrefChildTallness(0.7);
		BtSelectAll=new emButton(
			GrSelection,"select all",
			"Select All",
			"Select all entries in the directory content panel which is active\n"
			"(focused) itself, or which is the parent of an active directory\n"
			"entry panel. As usual, the entries are selected as the target and\n"
			"the source selection is set from the previous target selection.\n"
			"\n"
			"Hotkey: Alt+A",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/select_all.tga")
		);
		BtSelectAll->SetIconAboveCaption();
		BtSelectAll->SetBorderScaling(0.5);
		BtClearSelection=new emButton(
			GrSelection,"clear selection",
			"Clear Selection",
			"Clear the source and target selections.\n"
			"\n"
			"Hotkey: Alt+E",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/clear_selection.tga")
		);
		BtClearSelection->SetIconAboveCaption();
		BtClearSelection->SetBorderScaling(0.5);
		BtSwapSelection=new emButton(
			GrSelection,"swap selection",
			"Swap Selection",
			"Exchange the source selection for the target selection.\n"
			"\n"
			"Hotkey: Alt+Z",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/swap_selection.tga")
		);
		BtSwapSelection->SetIconAboveCaption();
		BtSwapSelection->SetBorderScaling(0.5);
		GrSelInfo=new emLinearGroup(GrSelection,"stat","Selection Statistics");
		GrSelInfo->SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
		look=GrSelInfo->GetLook();
		look.SetOutputBgColor(look.GetButtonBgColor());
		GrSelInfo->SetLook(look);
		GrSelInfo->SetBorderScaling(0.5);
			SelInfo=new emFileManSelInfoPanel(GrSelInfo,"info");
			SelInfo->SetFocusable(false);
		BtPaths2Clipboard=new emButton(
			GrSelection,"paths2clipboard",
			"Copy Paths",
			"Copy the full file path(s) of the target selection to the clipboard.\n"
			"Multiple entries are separated by line feeds.\n"
			"\n"
			"Hotkey: Alt+P",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/copy_paths.tga")
		);
		BtPaths2Clipboard->SetIconAboveCaption();
		BtPaths2Clipboard->SetBorderScaling(0.5);
		BtNames2Clipboard=new emButton(
			GrSelection,"names2clipboard",
			"Copy Names",
			"Copy the file name(s) of the target selection to the clipboard.\n"
			"Multiple entries are separated by line feeds.\n"
			"\n"
			"Hotkey: Alt+N",
			emGetInsResImage(GetRootContext(),"emFileMan","icons/copy_names.tga")
		);
		BtNames2Clipboard->SetIconAboveCaption();
		BtNames2Clipboard->SetBorderScaling(0.5);

	GrCommand=new Group(this,"commands",contentView,FMModel,FMModel->GetCommandRoot());
	GrCommand->SetCaption("File Manager Commands");

	AddWakeUpSignal(FMModel->GetSelectionSignal());
	AddWakeUpSignal(FMVConfig->GetChangeSignal());
	AddWakeUpSignal(RbmAspect.GetCheckSignal());
	AddWakeUpSignal(RbmTheme.GetCheckSignal());
	AddWakeUpSignal(RbSortByName->GetClickSignal());
	AddWakeUpSignal(RbSortByDate->GetClickSignal());
	AddWakeUpSignal(RbSortBySize->GetClickSignal());
	AddWakeUpSignal(RbSortByEnding->GetClickSignal());
	AddWakeUpSignal(RbSortByClass->GetClickSignal());
	AddWakeUpSignal(RbSortByVersion->GetClickSignal());
	AddWakeUpSignal(CbSortDirectoriesFirst->GetCheckSignal());
	AddWakeUpSignal(CbShowHiddenFiles->GetCheckSignal());
	AddWakeUpSignal(RbPerLocale->GetClickSignal());
	AddWakeUpSignal(RbCaseSensitive->GetClickSignal());
	AddWakeUpSignal(RbCaseInsensitive->GetClickSignal());
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
	emScreen * screen;
	emDirPanel * dp;
	emPanel * p;
	int i,j;

	if (
		IsSignaled(FMModel->GetSelectionSignal()) ||
		IsSignaled(FMVConfig->GetChangeSignal())
	) {
		UpdateButtonStates();
	}
	if (
		IsSignaled(RbmAspect.GetCheckSignal()) ||
		IsSignaled(RbmTheme.GetCheckSignal())
	) {
		i=RbmTheme.GetCheckIndex();
		j=RbmAspect.GetCheckIndex();
		if (i<0 || i>=FMThemeNames->GetThemeStyleCount()) i=0;
		if (j<0 || j>=FMThemeNames->GetThemeAspectRatioCount(i)) j=0;
		FMVConfig->SetThemeName(FMThemeNames->GetThemeName(i,j));
	}
	if (IsSignaled(RbSortByName->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_NAME);
	}
	if (IsSignaled(RbSortByDate->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_DATE);
	}
	if (IsSignaled(RbSortBySize->GetClickSignal())) {
		FMVConfig->SetSortCriterion(emFileManViewConfig::SORT_BY_SIZE);
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
	if (IsSignaled(CbSortDirectoriesFirst->GetCheckSignal())) {
		FMVConfig->SetSortDirectoriesFirst(CbSortDirectoriesFirst->IsChecked());
	}
	if (IsSignaled(CbShowHiddenFiles->GetCheckSignal())) {
		FMVConfig->SetShowHiddenFiles(CbShowHiddenFiles->IsChecked());
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
	return emLinearLayout::Cycle();
}


void emFileManControlPanel::UpdateButtonStates()
{
	emRadioButton * rb;
	int si,ai,ac,i;

	si=FMThemeNames->GetThemeStyleIndex(FMVConfig->GetThemeName());
	ai=FMThemeNames->GetThemeAspectRatioIndex(FMVConfig->GetThemeName());
	ac=FMThemeNames->GetThemeAspectRatioCount(si);
	for (i=0; i<RbmAspect.GetCount(); i++) {
		rb=RbmAspect.GetButton(i);
		if (i<ac) {
			rb->SetEnableSwitch(true);
			rb->SetCaption(FMThemeNames->GetThemeAspectRatio(si,i));
		}
		else {
			rb->SetEnableSwitch(false);
		}
	}
	RbmAspect.SetCheckIndex(ai);
	RbmTheme.SetCheckIndex(si);

	RbSortByName->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_NAME);
	RbSortByDate->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_DATE);
	RbSortBySize->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_SIZE);
	RbSortByEnding->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_ENDING);
	RbSortByClass->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_CLASS);
	RbSortByVersion->SetChecked(FMVConfig->GetSortCriterion()==emFileManViewConfig::SORT_BY_VERSION);

	CbSortDirectoriesFirst->SetChecked(FMVConfig->GetSortDirectoriesFirst());
	CbShowHiddenFiles->SetChecked(FMVConfig->GetShowHiddenFiles());

	RbPerLocale->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_PER_LOCALE);
	RbCaseSensitive->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_CASE_SENSITIVE);
	RbCaseInsensitive->SetChecked(FMVConfig->GetNameSortingStyle()==emFileManViewConfig::NSS_CASE_INSENSITIVE);

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
	SetMinChildTallness(cmd->PrefChildTallness*0.65);
	SetMaxChildTallness(1.0);
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
	SetIconAboveCaption();
	SetMaxIconAreaTallness(9.0/16.0);
	SetBorderScaling(cmd->BorderScaling * 0.5);
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
