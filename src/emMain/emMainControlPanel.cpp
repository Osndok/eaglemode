//------------------------------------------------------------------------------
// emMainControlPanel.cpp
//
// Copyright (C) 2014-2017 Oliver Hamann.
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
#include <emCore/emCoreConfigPanel.h>
#include <emMain/emAutoplay.h>
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
		"Copyright (C) 2001-2017 Oliver Hamann.\n"
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
	emLinearGroup * grAbout;
	emPackGroup * grCommands;
	emRasterGroup * grFullscreenPrefs;
	emLinearLayout * lMain, * lAbtCfgCmd, * lAbtCfg, * lCloseQuit;
	emCoreConfigPanel * coreConfigPanel;
	emLabel * iconLabel, * textLabel;
	emAutoplayControlPanel * apc;
	emLook look;

	ContentControlPanel=NULL;

	MainConfig=emMainConfig::Acquire(GetRootContext());
	BookmarksModel=emBookmarksModel::Acquire(GetRootContext());

	SetOuterBorderType(emBorder::OBT_POPUP_ROOT);
	SetInnerBorderType(emBorder::IBT_NONE);
	SetMinCellCount(2);
	SetOrientationThresholdTallness(1.0);
	SetChildWeight(0,11.37);
	SetChildWeight(1,21.32);
	SetInnerSpace(0.0098,0.0098);

	lMain=new emLinearLayout(this,"general");
	lMain->SetOrientationThresholdTallness(0.8);
	lMain->SetChildWeight(0,4.71);
	lMain->SetChildWeight(1,6.5);
	lMain->SetInnerSpace(0.0281,0.0281);
		lAbtCfgCmd=new emLinearLayout(lMain,"l");
		lAbtCfgCmd->SetOrientationThresholdTallness(0.8);
		lAbtCfgCmd->SetChildWeight(0,1.5);
		lAbtCfgCmd->SetChildWeight(1,3.05);
		lAbtCfgCmd->SetInnerSpace(0.07,0.07);
		new emBookmarksPanel(
			lMain,"bookmarks",
			&ContentView,
			BookmarksModel
		);

	lAbtCfg=new emLinearLayout(lAbtCfgCmd,"t");
		lAbtCfg->SetOrientationThresholdTallness(0.5);
		lAbtCfg->SetChildWeight(0,1.15);
		lAbtCfg->SetChildWeight(1,1.85);
		lAbtCfg->SetInnerSpace(0.16,0.16);
		grAbout=new emLinearGroup(lAbtCfg,"about","About Eagle Mode");
		grAbout->SetOrientationThresholdTallness(1.0);
		grAbout->SetBorderScaling(2.9);
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
			lAbtCfg,"core config"
		);
		coreConfigPanel->SetBorderScaling(1.77);

	grCommands=new emPackGroup(lAbtCfgCmd,"commands","Main Commands");
		grCommands->SetPrefChildTallness(0.7);
		grCommands->SetChildWeight(0,1.0);
		grCommands->SetChildWeight(1,1.09);
		grCommands->SetChildWeight(2,1.0);
		grCommands->SetChildWeight(3,2.09);
		grCommands->SetChildWeight(4,1.0);
		BtNewWindow=new emButton(
			grCommands,"new window",
			"New Window",
			"Create a new window showing the same location.\n"
			"\n"
			"Hotkey: F4",
			emGetInsResImage(GetRootContext(),"emMain","NewWindow.tga")
		);
		BtNewWindow->SetIconAboveCaption();
		BtNewWindow->SetBorderScaling(0.5);
		BtFullscreen=new emCheckButton(
			grCommands,"fullscreen",
			"Fullscreen",
			"Switch between fullscreen mode and normal window mode.\n"
			"\n"
			"Hotkey: F11",
			emGetInsResImage(GetRootContext(),"emMain","Fullscreen.tga")
		);
		BtFullscreen->SetIconAboveCaption();
		BtFullscreen->SetBorderScaling(0.5);
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
			"Hotkey: F5",
			emGetInsResImage(GetRootContext(),"emMain","ReloadFiles.tga")
		);
		BtReload->SetIconAboveCaption();
		BtReload->SetBorderScaling(0.5);
		apc=new emAutoplayControlPanel(grCommands,"autoplay",ContentView);
		apc->SetBorderScaling(0.2);
		apc->SetFocusable(false);
		lCloseQuit=new emLinearLayout(grCommands,"close_quit");
		lCloseQuit->SetOrientationThresholdTallness(0.7);
		lCloseQuit->SetChildWeight(0,1.0);
		lCloseQuit->SetChildWeight(1,0.8);
			BtClose=new emButton(
				lCloseQuit,"close",
				"Close",
				"Close this window.\n"
				"\n"
				"Hotkey: Alt+F4",
				emGetInsResImage(GetRootContext(),"emMain","CloseWindow.tga")
			);
			BtClose->SetIconAboveCaption();
			BtClose->SetBorderScaling(0.5);
			BtQuit=new emButton(
				lCloseQuit,"quit",
				"Quit",
				"Close all windows of this process (and terminate this process).\n"
				"\n"
				"Hotkey: Shift+Alt+F4",
				emGetInsResImage(GetRootContext(),"emMain","Quit.tga")
			);
			BtQuit->SetIconAboveCaption();
			BtQuit->SetBorderScaling(0.5);


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


emString emMainControlPanel::GetTitle() const
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
