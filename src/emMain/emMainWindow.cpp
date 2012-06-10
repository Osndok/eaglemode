//------------------------------------------------------------------------------
// emMainWindow.cpp
//
// Copyright (C) 2006-2012 Oliver Hamann.
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

#include <emCore/emFileModel.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emMain/emMainContentPanel.h>
#include <emMain/emCoreConfigPanel.h>
#include <emMain/emMainWindow.h>


emMainWindow::emMainWindow(emContext & parentContext)
	: emWindow(parentContext,0,0,"emMainWindow")
{
	const char * aboutTextFormat=
		"This is Eagle Mode version %s\n"
		"\n"
		"Copyright (C) 2001-2012 Oliver Hamann.\n"
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
	emTkGroup * grMain, * grCommands, * grAbout, * grFullscreenPrefs;
	emTkTiling * tlLeft, * tlTop, * tlClose;
	emCoreConfigPanel * coreConfigPanel;
	emTkLabel * iconLabel, * textLabel;
	emTkLook look;
	emBookmarkRec * bmRec;

	MainConfig=emMainConfig::Acquire(GetRootContext());
	BookmarksModel=emBookmarksModel::Acquire(GetRootContext());

	SetViewFlags(
		VF_ROOT_SAME_TALLNESS |
		VF_NO_ZOOM |
		VF_NO_FOCUS_HIGHLIGHT |
		VF_NO_ACTIVE_HIGHLIGHT
	);

	SetWindowIcon(emGetInsResImage(GetRootContext(),"icons","eaglemode48.tga"));

	MainPanel=new emMainPanel(this,"main",1.0/10.0);

	ControlPanel=new ControlPanelClass(GetControlView(),"ctrl",*this);
	ControlPanel->SetOuterBorderType(emTkBorder::OBT_POPUP_ROOT);
	ControlPanel->SetInnerBorderType(emTkBorder::IBT_NONE);
	ControlPanel->SetMinCellCount(2);
	ControlPanel->SetFixedRowCount(1);
	ControlPanel->SetPrefChildTallness(1.0/3.0);
	ControlPanel->SetPrefChildTallness(1.0/7.0,1);

	ContentPanel=new emMainContentPanel(GetContentView(),"");

	grMain=new emTkGroup(ControlPanel,"general","General");
	grMain->SetPrefChildTallness(1.0/0.9);
	grMain->SetPrefChildTallness(1.0/2.2,1);
		tlLeft=new emTkTiling(grMain,"l");
		tlLeft->SetPrefChildTallness(0.2);
		tlLeft->SetPrefChildTallness(0.9,-1);
		new emBookmarksPanel(
			grMain,"bookmarks",
			&GetContentView(),
			BookmarksModel
		);

	tlTop=new emTkTiling(tlLeft,"t");
		tlTop->SetPrefChildTallness(0.5);
		tlTop->SetPrefChildTallness(0.6,1);
		grAbout=new emTkGroup(tlTop,"about","About");
		grAbout->SetBorderScaling(4.0);
		grAbout->SetPrefChildTallness(1.0);
		grAbout->SetPrefChildTallness(0.5,1);
			iconLabel=new emTkLabel(grAbout,"icon",
				emString(),
				emString(),
				emGetInsResImage(GetRootContext(),"icons","eaglemode.tga")
			);
			iconLabel->SetLabelAlignment(EM_ALIGN_CENTER);
			textLabel=new emTkLabel(grAbout,"text",
				emString::Format(aboutTextFormat,emGetVersion())
			);
		coreConfigPanel=new emCoreConfigPanel(
			tlTop,"core config"
		);
		coreConfigPanel->SetBorderScaling(4.0);

	grCommands=new emTkGroup(tlLeft,"commands","Commands");
		BtNewWindow=new emTkButton(
			grCommands,"new window",
			"New Window",
			"Create a new window showing the same location.\n"
			"\n"
			"Hotkey: F4"
		);
		BtFullscreen=new emTkCheckButton(
			grCommands,"fullscreen",
			"Fullscreen",
			"Switch between fullscreen mode and normal window mode.\n"
			"\n"
			"Hotkey: F11"
		);
		BtFullscreen->SetChecked((GetWindowFlags()&WF_FULLSCREEN)!=0);
		BtFullscreen->HaveAux("aux",0.4);
			grFullscreenPrefs=new emTkGroup(
				BtFullscreen,"aux",
				"Preferences For Fullscreen Mode"
			);
			grFullscreenPrefs->SetBorderScaling(4.5);
			grFullscreenPrefs->SetChildTallness(0.11);
				BtAutoHideControlView=new emTkCheckBox(
					grFullscreenPrefs,
					"auto-hide_control_view",
					"Auto-Hide Control View",
					"Whether the control view shall automatically\n"
					"be minimized when in fullscreen mode."
				);
				BtAutoHideControlView->SetNoEOI();
				BtAutoHideControlView->SetChecked(MainConfig->AutoHideControlView);
				BtAutoHideSlider=new emTkCheckBox(
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
		BtReload=new emTkButton(
			grCommands,"reload",
			"Reload Files",
			"Reload files and directories which are currently shown by this program. You\n"
			"should trigger this function after you have modified files or directories\n"
			"with another program. This function does not really reload all files, but it\n"
			"checks file modification time stamps to see which files are to be reloaded.\n"
			"\n"
			"Hotkey: F5"
		);
		tlClose=new emTkTiling(grCommands,"close");
			tlClose->SetPrefChildTallness(0.33);
			tlClose->SetPrefChildTallness(0.5,1);
			BtClose=new emTkButton(
				tlClose,"close",
				"Close",
				"Close this window.\n"
				"\n"
				"Hotkey: Alt+F4"
			);
			BtQuit=new emTkButton(
				tlClose,"quit",
				"Quit",
				"Close all windows of this process (and terminate this process).\n"
				"\n"
				"Hotkey: Shift+Alt+F4"
			);

	ContentControlPanel=NULL;

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

	AddWakeUpSignal(GetContentView().GetTitleSignal());
	AddWakeUpSignal(GetContentView().GetControlPanelSignal());
	AddWakeUpSignal(GetCloseSignal());
	AddWakeUpSignal(GetWindowFlagsSignal());
	AddWakeUpSignal(MainConfig->GetChangeSignal());
	AddWakeUpSignal(BtNewWindow->GetClickSignal());
	AddWakeUpSignal(BtFullscreen->GetClickSignal());
	AddWakeUpSignal(BtAutoHideControlView->GetClickSignal());
	AddWakeUpSignal(BtAutoHideSlider->GetClickSignal());
	AddWakeUpSignal(BtReload->GetClickSignal());
	AddWakeUpSignal(BtClose->GetClickSignal());
	AddWakeUpSignal(BtQuit->GetClickSignal());

	bmRec=BookmarksModel->SearchStartLocation();
	if (bmRec) {
		GetContentView().Seek(
			bmRec->LocationIdentity.Get(),
			bmRec->LocationRelX.Get(),
			bmRec->LocationRelY.Get(),
			bmRec->LocationRelA.Get(),
			true,
			bmRec->Name.Get()
		);
	}
}


emMainWindow::~emMainWindow()
{
	if (ContentControlPanel) {
		delete ContentControlPanel;
		ContentControlPanel=NULL;
	}
}


emString emMainWindow::GetTitle()
{
	return "Eagle Mode - " + GetContentView().GetTitle();
}


emMainWindow * emMainWindow::Duplicate()
{
	emMainWindow * w;
	emPanel * p;
	double relX,relY,relA;
	bool adherent;

	w=new emMainWindow(*GetParentContext());
	w->GetMainPanel().SetControlEdgesColor(GetMainPanel().GetControlEdgesColor());
	p=GetContentView().GetVisitedPanel(&relX,&relY,&relA,&adherent);
	if (p) w->GetContentView().Seek(p->GetIdentity(),relX,relY,relA,adherent,GetContentView().GetTitle());
	return w;
}


bool emMainWindow::Cycle()
{
	if (IsSignaled(GetContentView().GetTitleSignal())) {
		InvalidateTitle();
	}

	if (IsSignaled(GetContentView().GetControlPanelSignal())) {
		if (ContentControlPanel) delete ContentControlPanel;
		ContentControlPanel=GetContentView().CreateControlPanel(
			*ControlPanel,
			"context"
		);
	}

	if (IsSignaled(GetWindowFlagsSignal())) {
		BtFullscreen->SetChecked((GetWindowFlags()&WF_FULLSCREEN)!=0);
	}

	if (IsSignaled(MainConfig->GetChangeSignal())) {
		BtAutoHideControlView->SetChecked(MainConfig->AutoHideControlView);
		BtAutoHideSlider->SetChecked(MainConfig->AutoHideSlider);
	}

	if (IsSignaled(BtNewWindow->GetClickSignal())) {
		Duplicate();
	}
	if (IsSignaled(BtFullscreen->GetClickSignal())) {
		SetWindowFlags(GetWindowFlags()^WF_FULLSCREEN);
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
		Signal(emFileModel::AcquireUpdateSignalModel(GetRootContext())->Sig);
	}
	if (
		IsSignaled(BtClose->GetClickSignal()) ||
		IsSignaled(GetCloseSignal())
	) {
		delete this;
		return false;
	}
	if (IsSignaled(BtQuit->GetClickSignal())) {
		GetScheduler().InitiateTermination(0);
		return false;
	}

	return false;
}


void emMainWindow::Input(emInputEvent & event, const emInputState & state)
{
	emBookmarkRec * bmRec;

	switch (event.GetKey()) {
	case EM_KEY_F4:
		if (state.IsNoMod()) {
			BtNewWindow->Click();
			event.Eat();
		}
		else if (state.IsAltMod()) {
			BtClose->Click();
			event.Eat();
		}
		else if (state.IsShiftAltMod()) {
			BtQuit->Click();
			event.Eat();
		}
		break;
	case EM_KEY_F5:
		if (state.IsNoMod()) {
			BtReload->Click();
			event.Eat();
		}
		break;
	case EM_KEY_F11:
		if (state.IsNoMod()) {
			BtFullscreen->Click();
			event.Eat();
		}
		break;
	case EM_KEY_ESCAPE:
#if defined(ANDROID)
	case EM_KEY_MENU:
#endif
		if (state.IsNoMod()) {
			ToggleControlView();
			event.Eat();
		}
		break;
	default:
		break;
	}

	bmRec=BookmarksModel->SearchBookmarkByHotkey(emInputHotkey(event,state));
	if (bmRec) {
		event.Eat();
		GetContentView().Seek(
			bmRec->LocationIdentity.Get(),
			bmRec->LocationRelX.Get(),
			bmRec->LocationRelY.Get(),
			bmRec->LocationRelA.Get(),
			true,
			bmRec->Name.Get()
		);
	}

	emWindow::Input(event,state);
}


void emMainWindow::DoCustomCheat(const char * func)
{
	if (strcmp(func,"rcp")==0) RecreateContentPanels(GetScreen());
	else emWindow::DoCustomCheat(func);
}


void emMainWindow::RecreateContentPanels(emScreen & screen)
{
	emArray<emWindow*> windows;
	emMainWindow * mw;
	emString id,title;
	emPanel * p;
	double rx,ry,ra;
	bool adherent;
	int i;

	windows=screen.GetWindows();
	for (i=0; i<windows.GetCount(); i++) {
		mw=dynamic_cast<emMainWindow *>(windows[i]);
		if (!mw) continue;
		title=mw->GetContentView().GetTitle();
		p=mw->GetContentView().GetVisitedPanel(&rx,&ry,&ra,&adherent);
		if (!p) continue;
		id=p->GetIdentity();
		delete mw->ContentPanel;
		mw->ContentPanel=new emMainContentPanel(mw->GetContentView(),"");
		mw->GetContentView().Seek(id,rx,ry,ra,adherent,title);
	}
}


void emMainWindow::ToggleControlView()
{
	if (GetContentView().IsFocused()) {
		GetControlView().Focus();
		GetControlView().VisitFullsized(ControlPanel,false);
	}
	else {
		GetControlView().ZoomOut();
		GetContentView().Focus();
	}
}


emMainWindow::ControlPanelClass::ControlPanelClass(
	ParentArg parent, const emString & name, emMainWindow & mainWin
)
	: emTkGroup(parent,name), MainWin(mainWin)
{
}


void emMainWindow::ControlPanelClass::Input(
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
	emTkGroup::Input(event,state,mx,my);
}
