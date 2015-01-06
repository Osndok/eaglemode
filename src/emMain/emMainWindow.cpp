//------------------------------------------------------------------------------
// emMainWindow.cpp
//
// Copyright (C) 2006-2012,2014 Oliver Hamann.
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

#include <emMain/emMainWindow.h>
#include <emCore/emFileModel.h>
#include <emCore/emRes.h>
#include <emMain/emMainContentPanel.h>
#include <emMain/emMainControlPanel.h>


emMainWindow::emMainWindow(emContext & parentContext)
	: emWindow(parentContext,0,0,"emMainWindow")
{
	emBookmarkRec * bmRec;

	BookmarksModel=emBookmarksModel::Acquire(GetRootContext());
	ToClose=false;

	SetViewFlags(
		VF_ROOT_SAME_TALLNESS |
		VF_NO_ZOOM |
		VF_NO_FOCUS_HIGHLIGHT |
		VF_NO_ACTIVE_HIGHLIGHT
	);

	SetWindowIcon(emGetInsResImage(GetRootContext(),"icons","eaglemode48.tga"));

	MainPanel=new emMainPanel(this,"main",1.0/10.0);

	ControlPanel=new emMainControlPanel(
		GetControlView(),"ctrl",*this,GetContentView()
	);

	ContentPanel=new emMainContentPanel(GetContentView(),"");

	AddWakeUpSignal(GetContentView().GetTitleSignal());
	AddWakeUpSignal(GetCloseSignal());

	bmRec=BookmarksModel->SearchStartLocation();
	if (bmRec) {
		GetContentView().Visit(
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
	if (ControlWindow) delete ControlWindow;
	delete ControlPanel;
	delete ContentPanel;
	delete MainPanel;
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

	w=new emMainWindow(*GetParentContext());
	w->GetMainPanel().SetControlEdgesColor(GetMainPanel().GetControlEdgesColor());
	p=GetContentView().GetVisitedPanel(&relX,&relY,&relA);
	if (p) {
		w->GetContentView().Visit(
			p->GetIdentity(),relX,relY,relA,
			GetContentView().IsActivationAdherent(),
			GetContentView().GetTitle()
		);
	}
	return w;
}


void emMainWindow::ToggleFullscreen()
{
	SetWindowFlags(GetWindowFlags()^WF_FULLSCREEN);
}


void emMainWindow::ReloadFiles()
{
	Signal(emFileModel::AcquireUpdateSignalModel(GetRootContext())->Sig);
}


void emMainWindow::ToggleControlView()
{
	if (GetContentView().IsFocused()) {
		GetControlView().Focus();
		GetControlView().AbortActiveAnimator();
		GetControlView().RawVisitFullsized(ControlPanel);
		GetControlView().SetActivePanel(ControlPanel,false);
	}
	else {
		GetControlView().ZoomOut();
		GetContentView().Focus();
	}
}


void emMainWindow::Close()
{
	ToClose=true;
	WakeUp();
}


void emMainWindow::Quit()
{
	GetScheduler().InitiateTermination(0);
}


bool emMainWindow::Cycle()
{
	if (IsSignaled(GetContentView().GetTitleSignal())) {
		InvalidateTitle();
	}

	if (IsSignaled(GetCloseSignal())) {
		Close();
	}

	if (ToClose) {
		delete this;
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
			Duplicate();
			event.Eat();
		}
		else if (state.IsAltMod()) {
			Close();
			event.Eat();
		}
		else if (state.IsShiftAltMod()) {
			Quit();
			event.Eat();
		}
		break;
	case EM_KEY_F5:
		if (state.IsNoMod()) {
			ReloadFiles();
			event.Eat();
		}
		break;
	case EM_KEY_F11:
		if (state.IsNoMod()) {
			ToggleFullscreen();
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
		GetContentView().Visit(
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
	if (strcmp(func,"rcp")==0) {
		RecreateContentPanels(GetScreen());
	}
	else if (strcmp(func,"ccw")==0) {
		CreateControlWindow();
	}
	else {
		emWindow::DoCustomCheat(func);
	}
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
		p=mw->GetContentView().GetVisitedPanel(&rx,&ry,&ra);
		if (!p) continue;
		id=p->GetIdentity();
		adherent=mw->GetContentView().IsActivationAdherent();
		delete mw->ContentPanel;
		mw->ContentPanel=new emMainContentPanel(mw->GetContentView(),"");
		mw->GetContentView().Visit(id,rx,ry,ra,adherent,title);
	}
}


void emMainWindow::CreateControlWindow()
{
	if (ControlWindow) {
		ControlWindow->Raise();
	}
	else {
		ControlWindow = new emWindow(
			*this,
			VF_POPUP_ZOOM|VF_ROOT_SAME_TALLNESS,
			WF_AUTO_DELETE,
			"emMainControlWindow"
		);
		new emMainControlPanel(
			*ControlWindow,"ctrl",*this,GetContentView()
		);
	}
}
