//------------------------------------------------------------------------------
// emMainWindow.cpp
//
// Copyright (C) 2006-2012,2014-2017 Oliver Hamann.
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
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emMain/emMainContentPanel.h>
#include <emMain/emMainControlPanel.h>


emMainWindow::emMainWindow(
	emContext & parentContext,
	const char * visitIdentity,
	double visitRelX,
	double visitRelY,
	double visitRelA,
	bool visitAdherent,
	const char * visitSubject,
	emColor ceColor
)
	: emWindow(parentContext,0,0,"emMainWindow"),
	WindowStateSaver(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","winstate.rec")
	)
{
	ToClose=false;
	MainPanel=NULL;
	ControlPanel=NULL;
	ContentPanel=NULL;
	StartupEngine=NULL;

	SetViewFlags(
		VF_ROOT_SAME_TALLNESS |
		VF_NO_ZOOM |
		VF_NO_FOCUS_HIGHLIGHT |
		VF_NO_ACTIVE_HIGHLIGHT
	);

	SetWindowIcon(emGetInsResImage(GetRootContext(),"icons","eaglemode48.tga"));

	StartupEngine=new StartupEngineClass(
		*this,
		visitIdentity,
		visitRelX,
		visitRelY,
		visitRelA,
		visitAdherent,
		visitSubject,
		ceColor
	);

	AddWakeUpSignal(GetCloseSignal());
}


emMainWindow::~emMainWindow()
{
	AutoplayViewModel=NULL;
	if (StartupEngine) delete StartupEngine;
	if (ControlWindow.Get()) delete ControlWindow.Get();
	if (ControlPanel) delete ControlPanel;
	if (ContentPanel) delete ContentPanel;
	if (MainPanel) delete MainPanel;
}


emString emMainWindow::GetTitle() const
{
	if (MainPanel && !StartupEngine) {
		return "Eagle Mode - " + MainPanel->GetContentView().GetTitle();
	}
	else {
		return "Eagle Mode";
	}
}


emMainWindow * emMainWindow::Duplicate()
{
	emMainWindow * w;
	emPanel * p;
	double relX,relY,relA;
	bool adherent;
	emString identity, subject;
	emColor ceColor;

	p=NULL;
	relX=relY=relA=0.0;
	adherent=false;
	ceColor=0;

	if (MainPanel) {
		p=MainPanel->GetContentView().GetVisitedPanel(&relX,&relY,&relA);
		if (p) identity=p->GetIdentity();
		adherent=MainPanel->GetContentView().IsActivationAdherent();
		subject=MainPanel->GetContentView().GetTitle();
		ceColor=MainPanel->GetControlEdgesColor();
	}

	w=new emMainWindow(
		*GetParentContext(),
		p?identity.Get():NULL,
		relX,relY,relA,adherent,subject,
		ceColor
	);

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
	if (MainPanel && ControlPanel) {
		if (MainPanel->GetContentView().IsFocused()) {
			MainPanel->GetControlView().Focus();
			MainPanel->GetControlView().AbortActiveAnimator();
			MainPanel->GetControlView().RawVisitFullsized(ControlPanel);
			MainPanel->GetControlView().SetActivePanel(ControlPanel,false);
		}
		else {
			MainPanel->GetControlView().ZoomOut();
			MainPanel->GetContentView().Focus();
		}
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
	if (MainPanel && IsSignaled(MainPanel->GetContentView().GetTitleSignal())) {
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

	if (StartupEngine) {
		event.Eat();
		emWindow::Input(event,state);
		return;
	}

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

	if (AutoplayViewModel) {
		AutoplayViewModel->Input(event,state);
	}

	if (BookmarksModel && MainPanel) {
		bmRec=BookmarksModel->SearchBookmarkByHotkey(emInputHotkey(event,state));
		if (bmRec) {
			event.Eat();
			MainPanel->GetContentView().Visit(
				bmRec->LocationIdentity.Get(),
				bmRec->LocationRelX.Get(),
				bmRec->LocationRelY.Get(),
				bmRec->LocationRelA.Get(),
				true,
				bmRec->Name.Get()
			);
		}
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
		if (!mw->MainPanel) continue;
		if (!mw->ControlPanel) continue;
		if (!mw->ContentPanel) continue;
		title=mw->MainPanel->GetContentView().GetTitle();
		p=mw->MainPanel->GetContentView().GetVisitedPanel(&rx,&ry,&ra);
		if (!p) continue;
		id=p->GetIdentity();
		adherent=mw->MainPanel->GetContentView().IsActivationAdherent();
		delete mw->ContentPanel;
		mw->ContentPanel=new emMainContentPanel(mw->MainPanel->GetContentView(),"");
		mw->MainPanel->GetContentView().Visit(id,rx,ry,ra,adherent,title);
	}
}


void emMainWindow::CreateControlWindow()
{
	if (ControlWindow) {
		ControlWindow->Raise();
	}
	else {
		if (MainPanel) {
			ControlWindow = new emWindow(
				*this,
				VF_POPUP_ZOOM|VF_ROOT_SAME_TALLNESS,
				WF_AUTO_DELETE,
				"emMainControlWindow"
			);
			new emMainControlPanel(
				*ControlWindow,"ctrl",*this,MainPanel->GetContentView()
			);
		}
	}
}


emMainWindow::StartupEngineClass::StartupEngineClass(
	emMainWindow & mainWin,
	const char * visitIdentity,
	double visitRelX,
	double visitRelY,
	double visitRelA,
	bool visitAdherent,
	const char * visitSubject,
	emColor ceColor
) :
	emEngine(mainWin.GetScheduler()),
	MainWin(mainWin),
	VisitValid(visitIdentity!=NULL),
	VisitIdentity(visitIdentity),
	VisitRelX(visitRelX),
	VisitRelY(visitRelY),
	VisitRelA(visitRelA),
	VisitAdherent(visitAdherent),
	VisitSubject(visitSubject),
	CeColor(ceColor),
	VisitingVA(NULL),
	Clk(0)
{
	State=0;
	WakeUp();
}


emMainWindow::StartupEngineClass::~StartupEngineClass()
{
	if (VisitingVA) delete VisitingVA;
}


bool emMainWindow::StartupEngineClass::Cycle()
{
	emBookmarkRec * bmRec;

	switch (State) {
	case 0:
		State++;
		return true;
	case 1:
		State++;
		return true;
	case 2:
		State++;
		return true;
	case 3:
		MainWin.MainPanel=new emMainPanel(&MainWin,"main",0.0538);
		MainWin.AddWakeUpSignal(MainWin.MainPanel->GetContentView().GetTitleSignal());
		if (CeColor.GetAlpha()!=0) {
			MainWin.MainPanel->SetControlEdgesColor(CeColor);
		}
		MainWin.MainPanel->SetStartupOverlay(true);
		MainWin.AutoplayViewModel=emAutoplayViewModel::Acquire(
			MainWin.MainPanel->GetContentView()
		);
		MainWin.AutoplayViewModel->SetConfigFilePath(
			emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","autoplay.rec")
		);
		State++;
		return true;
	case 4:
		MainWin.BookmarksModel=emBookmarksModel::Acquire(MainWin.GetRootContext());
		if (!VisitValid) {
			bmRec=MainWin.BookmarksModel->SearchStartLocation();
			if (bmRec) {
				VisitValid=true;
				VisitIdentity=bmRec->LocationIdentity.Get();
				VisitRelX=bmRec->LocationRelX.Get();
				VisitRelY=bmRec->LocationRelY.Get();
				VisitRelA=bmRec->LocationRelA.Get();
				VisitAdherent=true;
				VisitSubject=bmRec->Name.Get();
			}
		}
		State++;
		if (IsTimeSliceAtEnd()) return true;
	case 5:
		MainWin.ControlPanel=new emMainControlPanel(
			MainWin.MainPanel->GetControlView(),
			"ctrl",
			MainWin,
			MainWin.MainPanel->GetContentView()
		);
		State++;
		if (IsTimeSliceAtEnd()) return true;
	case 6:
		MainWin.ContentPanel=new emMainContentPanel(
			MainWin.MainPanel->GetContentView(),
			""
		);
		State++;
		if (IsTimeSliceAtEnd()) return true;
	case 7:
		VisitingVA=new emVisitingViewAnimator(
			MainWin.MainPanel->GetContentView()
		);
		VisitingVA->SetAnimated(false);
		VisitingVA->SetGoalFullsized(":",false);
		VisitingVA->Activate();
		Clk=emGetClockMS();
		State++;
		if (IsTimeSliceAtEnd()) return true;
	case 8:
		if (emGetClockMS()<Clk+2000 && VisitingVA->IsActive()) {
			return true;
		}
		State++;
		return true;
	case 9:
		VisitingVA->Deactivate();
		if (VisitValid) {
			VisitingVA->SetGoal(
				VisitIdentity,
				VisitRelX,
				VisitRelY,
				VisitRelA,
				VisitAdherent,
				VisitSubject
			);
			VisitingVA->Activate();
		}
		Clk=emGetClockMS();
		State++;
		if (IsTimeSliceAtEnd()) return true;
	case 10:
		if (emGetClockMS()<Clk+2000 && VisitingVA->IsActive()) {
			return true;
		}
		delete VisitingVA;
		VisitingVA=NULL;
		MainWin.MainPanel->GetContentView().RawZoomOut();
		MainWin.MainPanel->GetContentView().SetActivePanel(MainWin.ContentPanel);
		MainWin.MainPanel->SetStartupOverlay(false);
		Clk=emGetClockMS();
		State++;
		return true;
	default:
		if (emGetClockMS()<Clk+100) {
			return true;
		}
		if (VisitValid) {
			MainWin.MainPanel->GetContentView().Visit(
				VisitIdentity,
				VisitRelX,
				VisitRelY,
				VisitRelA,
				VisitAdherent,
				VisitSubject
			);
		}
		MainWin.InvalidateTitle();
		MainWin.StartupEngine=NULL;
		delete this;
		return false;
	}
}
