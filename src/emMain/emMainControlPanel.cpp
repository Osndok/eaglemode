//------------------------------------------------------------------------------
// emMainControlPanel.cpp
//
// Copyright (C) 2014-2015 Oliver Hamann.
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

#include <emMain/emMainControlPanel.h>
#include <emCore/emRes.h>
#include <emMain/emCoreConfigPanel.h>
#include <emMain/emMainWindow.h>


emMainControlPanel::emMainControlPanel(
	ParentArg parent, const emString & name, emMainWindow & mainWin,
	emView & contentView
)
	: emLinearGroup(parent,name),
	MainWin(mainWin),
	ContentView(contentView)
{
	const char * aboutTextFormat=
		"This is Eagle Mode version %s\n"
		"\n"
		"Copyright (C) 2001-2015 Oliver Hamann.\n"
		"\n"
		"Homepage: http://eaglemode.sourceforge.net/\n"
		"\n"
		"This program is free software: you can redistribute it and/or modify it under\n"
		"the terms of the GNU General Public License version 3 as published by the\n"
		"Free Software Foundation.\n"
		"\n"
		"This program is distributed in the hope that it will be useful, but WITHOUT\n"
		"ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
		"FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for\n"
		"more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License version 3\n"
		"along with this program. If not, see <http://www.gnu.org/licenses/>.\n"
	;
	emLinearGroup * grMain, * grAbout;
	emRasterGroup * grCommands, * grFullscreenPrefs;
	emLinearLayout * lLeft, * lTop, * lClose;
	emCoreConfigPanel * coreConfigPanel;
	emLabel * iconLabel, * textLabel;
	emLook look;

	ContentControlPanel=NULL;

	MainConfig=emMainConfig::Acquire(GetRootContext());
	BookmarksModel=emBookmarksModel::Acquire(GetRootContext());

	SetOuterBorderType(emBorder::OBT_POPUP_ROOT);
	SetInnerBorderType(emBorder::IBT_NONE);
	SetMinCellCount(2);
	SetOrientationThresholdTallness(1.0);
	SetChildWeight(0,3.0);
	SetChildWeight(1,7.0);

	grMain=new emLinearGroup(this,"general","General");
	grMain->SetOrientationThresholdTallness(1.0);
	grMain->SetChildWeight(0,0.9);
	grMain->SetChildWeight(1,2.2);
		lLeft=new emLinearLayout(grMain,"l");
		lLeft->SetOrientationThresholdTallness(0.5);
		lLeft->SetChildWeight(0,0.2);
		lLeft->SetChildWeight(1,0.9);
		new emBookmarksPanel(
			grMain,"bookmarks",
			&MainWin.GetContentView(),
			BookmarksModel
		);

	lTop=new emLinearLayout(lLeft,"t");
		lTop->SetOrientationThresholdTallness(0.5);
		lTop->SetChildWeight(0,1.2);
		lTop->SetChildWeight(1,1.0);
		grAbout=new emLinearGroup(lTop,"about","About");
		grAbout->SetOrientationThresholdTallness(1.0);
		grAbout->SetBorderScaling(4.0);
		grAbout->SetChildWeight(0,1.0);
		grAbout->SetChildWeight(1,2.0);
			iconLabel=new emLabel(grAbout,"icon",
				emString(),
				emString(),
				emGetInsResImage(GetRootContext(),"icons","eaglemode.tga")
			);
			iconLabel->SetLabelAlignment(EM_ALIGN_CENTER);
			textLabel=new emLabel(grAbout,"text",
				emString::Format(aboutTextFormat,emGetVersion())
			);
		coreConfigPanel=new emCoreConfigPanel(
			lTop,"core config"
		);
		coreConfigPanel->SetBorderScaling(4.0);

	grCommands=new emRasterGroup(lLeft,"commands","Commands");
		grCommands->SetPrefChildTallness(0.2);
		BtNewWindow=new emButton(
			grCommands,"new window",
			"New Window",
			"Create a new window showing the same location.\n"
			"\n"
			"Hotkey: F4"
		);
		BtFullscreen=new emCheckButton(
			grCommands,"fullscreen",
			"Fullscreen",
			"Switch between fullscreen mode and normal window mode.\n"
			"\n"
			"Hotkey: F11"
		);
		BtFullscreen->SetChecked((MainWin.GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0);
		BtFullscreen->HaveAux("aux",0.4);
			grFullscreenPrefs=new emRasterGroup(
				BtFullscreen,"aux",
				"Preferences For Fullscreen Mode"
			);
			grFullscreenPrefs->SetBorderScaling(4.5);
			grFullscreenPrefs->SetChildTallness(0.11);
				BtAutoHideControlView=new emCheckBox(
					grFullscreenPrefs,
					"auto-hide_control_view",
					"Auto-Hide Control View",
					"Whether the control view shall automatically\n"
					"be minimized when in fullscreen mode."
				);
				BtAutoHideControlView->SetNoEOI();
				BtAutoHideControlView->SetChecked(MainConfig->AutoHideControlView);
				BtAutoHideSlider=new emCheckBox(
					grFullscreenPrefs,
					"auto-hide_slider",
					"Auto-Hide Slider",
					"Whether the control view sizing slider shall get\n"
					"invisible when in fullscreen mode and when the\n"
					"control view is minimized and when the mouse has\n"
					"not been used for some time."
				);
				BtAutoHideSlider->SetNoEOI();
				BtAutoHideSlider->SetChecked(MainConfig->AutoHideSlider);
		BtReload=new emButton(
			grCommands,"reload",
			"Reload Files",
			"Reload files and directories which are currently shown by this program. You\n"
			"should trigger this function after you have modified files or directories\n"
			"with another program. This function does not really reload all files, but it\n"
			"checks file modification time stamps to see which files are to be reloaded.\n"
			"\n"
			"Hotkey: F5"
		);
		lClose=new emLinearLayout(grCommands,"close");
			lClose->SetOrientationThresholdTallness(0.4);
			lClose->SetChildWeight(0,1.0);
			lClose->SetChildWeight(1,0.66);
			BtClose=new emButton(
				lClose,"close",
				"Close",
				"Close this window.\n"
				"\n"
				"Hotkey: Alt+F4"
			);
			BtQuit=new emButton(
				lClose,"quit",
				"Quit",
				"Close all windows of this process (and terminate this process).\n"
				"\n"
				"Hotkey: Shift+Alt+F4"
			);

	look=BtQuit->GetLook();
	look.SetButtonBgColor(0x99CC99FF);
	look.SetButtonFgColor(0x000000FF);
	BtNewWindow->SetLook(look,false);
	look.SetButtonBgColor(0xAAAADDFF);
	look.SetButtonFgColor(0x000000FF);
	BtFullscreen->SetLook(look,false);
	look.SetButtonBgColor(0x99CCCCFF);
	look.SetButtonFgColor(0x000000FF);
	BtReload->SetLook(look,false);
	look.SetButtonBgColor(0xCCCC99FF);
	look.SetButtonFgColor(0x000000FF);
	BtClose->SetLook(look,false);
	look.SetButtonBgColor(0xCC9999FF);
	look.SetButtonFgColor(0x000000FF);
	BtQuit->SetLook(look,false);

	AddWakeUpSignal(ContentView.GetControlPanelSignal());
	AddWakeUpSignal(MainWin.GetWindowFlagsSignal());
	AddWakeUpSignal(MainConfig->GetChangeSignal());
	AddWakeUpSignal(BtNewWindow->GetClickSignal());
	AddWakeUpSignal(BtFullscreen->GetClickSignal());
	AddWakeUpSignal(BtAutoHideControlView->GetClickSignal());
	AddWakeUpSignal(BtAutoHideSlider->GetClickSignal());
	AddWakeUpSignal(BtReload->GetClickSignal());
	AddWakeUpSignal(BtClose->GetClickSignal());
	AddWakeUpSignal(BtQuit->GetClickSignal());

	RecreateContentControlPanel();
}


emMainControlPanel::~emMainControlPanel()
{
	if (ContentControlPanel) {
		delete ContentControlPanel;
		ContentControlPanel=NULL;
	}
}


emString emMainControlPanel::GetTitle()
{
	return "emMainControl";
}


bool emMainControlPanel::Cycle()
{
	if (IsSignaled(ContentView.GetControlPanelSignal())) {
		RecreateContentControlPanel();
	}

	if (IsSignaled(MainWin.GetWindowFlagsSignal())) {
		BtFullscreen->SetChecked((MainWin.GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0);
	}

	if (IsSignaled(MainConfig->GetChangeSignal())) {
		BtAutoHideControlView->SetChecked(MainConfig->AutoHideControlView);
		BtAutoHideSlider->SetChecked(MainConfig->AutoHideSlider);
	}

	if (IsSignaled(BtNewWindow->GetClickSignal())) {
		MainWin.Duplicate();
	}

	if (IsSignaled(BtFullscreen->GetClickSignal())) {
		MainWin.ToggleFullscreen();
	}

	if (IsSignaled(BtAutoHideControlView->GetClickSignal())) {
		MainConfig->AutoHideControlView.Invert();
		MainConfig->Save();
	}

	if (IsSignaled(BtAutoHideSlider->GetClickSignal())) {
		MainConfig->AutoHideSlider.Invert();
		MainConfig->Save();
	}

	if (IsSignaled(BtReload->GetClickSignal())) {
		MainWin.ReloadFiles();
	}

	if (IsSignaled(BtClose->GetClickSignal())) {
		MainWin.Close();
	}

	if (IsSignaled(BtQuit->GetClickSignal())) {
		MainWin.Quit();
	}

	return false;
}


void emMainControlPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	switch (event.GetKey()) {
	case EM_KEY_ESCAPE:
#if defined(ANDROID)
	case EM_KEY_MENU:
#endif
		if (state.IsNoMod()) {
			MainWin.ToggleControlView();
			event.Eat();
		}
		break;
	default:
		break;
	}
	emLinearGroup::Input(event,state,mx,my);
}


void emMainControlPanel::RecreateContentControlPanel()
{
	if (ContentControlPanel) delete ContentControlPanel;
	ContentControlPanel=ContentView.CreateControlPanel(
		*this,
		"context"
	);
}
