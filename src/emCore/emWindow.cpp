//------------------------------------------------------------------------------
// emWindow.cpp
//
// Copyright (C) 2005-2010 Oliver Hamann.
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

#include <emCore/emWindow.h>
#include <emCore/emPanel.h>


//==============================================================================
//================================== emWindow ==================================
//==============================================================================

emWindow::emWindow(
	emContext & parentContext, ViewFlags viewFlags, WindowFlags windowFlags,
	const emString & wmResName
)
	: emView(parentContext,viewFlags),
	CloseSignal(),
	AutoDeleteEngine(this)
{
	emContext * con;
	emWindow * win;

	Screen=emScreen::LookupInherited(parentContext);
	if (!Screen) emFatalError("emWindow: No emScreen found.");
	WFlags=windowFlags;
	WMResName=wmResName;
	for (con=GetParentContext(); con; con=con->GetParentContext()) {
		win=dynamic_cast<emWindow*>(con);
		if (win) {
			WindowIcon=win->WindowIcon;
			break;
		}
	}
	WindowPort=NULL;
	PrevVPValid=false;
	WindowPort=Screen->CreateWindowPort(*this);
	Screen->Windows.Add(this);
	Signal(Screen->WindowsSignal);
}


emWindow::~emWindow()
{
	emContext * con, * nextCon;
	emWindow * win;
	emView * vw;
	int i;

	// Get rid of cross pointers.
	CrossPtrList.BreakCrossPtrs();

	// Delete child windows of same screen.
	for (;;) {
		// Search for a child window.
		win=NULL;
		for (con=GetFirstChildContext(); con; con=nextCon) {
			win=dynamic_cast<emWindow*>(con);
			if (win) {
				if (win->Screen==Screen) break;
				win=NULL;
			}
			nextCon=con->GetFirstChildContext();
			if (!nextCon) {
				for (;;) {
					nextCon=con->GetNextContext();
					if (nextCon) break;
					con=con->GetParentContext();
					if (con==this) break;
				}
			}
		}
		if (!win) break;
		// Found a child window to delete, but popup windows of views
		// must not be deleted directly (??? still a problem of emView).
		vw=dynamic_cast<emView*>(win->GetParentContext());
		if (vw && vw->IsPoppedUp()) vw->ZoomOut();
		else delete win;
	}

	// Delete panels.
	if (GetRootPanel()) delete GetRootPanel();

	// Remove from screen.
	for (i=Screen->Windows.GetCount()-1; i>=0; i--) {
		if (Screen->Windows[i]==this) {
			Screen->Windows.Remove(i);
			Signal(Screen->WindowsSignal);
			break;
		}
	}
	delete WindowPort;
	WindowPort=NULL;
}


void emWindow::SetWindowFlags(WindowFlags windowFlags)
{
	if (WFlags!=windowFlags) {
		if ((WFlags&WF_FULLSCREEN)==0) {
			PrevVPX=GetHomeX();
			PrevVPY=GetHomeY();
			PrevVPW=GetHomeWidth();
			PrevVPH=GetHomeHeight();
			PrevVPValid=true;
		}
		WFlags=windowFlags;
		WindowPort->WindowFlagsChanged();
		if ((WFlags&WF_FULLSCREEN)==0 && PrevVPValid) {
			SetViewPosSize(PrevVPX,PrevVPY,PrevVPW,PrevVPH);
		}
		Signal(WindowFlagsSignal);
	}
}


bool emWindow::SetWinPosViewSize(const char * geometry)
{
	emWindowPort::PosSizeArgSpec posSpec,sizeSpec;
	double x,y,w,h,l,t,r,b;
	char sx,sy;

	GetBorderSizes(&l,&t,&r,&b);
	posSpec=sizeSpec=emWindowPort::PSAS_IGNORE;
	x=y=w=h=0.0;
	if (sscanf(geometry,"%lfx%lf%c%lf%c%lf",&w,&h,&sx,&x,&sy,&y)==6) {
		posSpec=emWindowPort::PSAS_WINDOW;
		sizeSpec=emWindowPort::PSAS_VIEW;
		if (sx=='-') x=Screen->GetWidth()-w-l-r-x;
		else if (sx!='+') return false;
		if (sy=='-') y=Screen->GetHeight()-h-t-b-y;
		else if (sy!='+') return false;
	}
	else if (sscanf(geometry,"%lfx%lf",&w,&h)==2) {
		sizeSpec=emWindowPort::PSAS_VIEW;
	}
	else if (sscanf(geometry,"%c%lf%c%lf",&sx,&x,&sy,&y)==4) {
		posSpec=emWindowPort::PSAS_WINDOW;
		if (sx=='-') x=Screen->GetWidth()-GetHomeWidth()-l-r-x;
		else if (sx!='+') return false;
		if (sy=='-') y=Screen->GetHeight()-GetHomeHeight()-t-b-y;
		else if (sy!='+') return false;
	}
	else {
		return false;
	}
	WindowPort->SetPosSize(x,y,posSpec,w,h,sizeSpec);
	return true;
}


void emWindow::SetWindowIcon(const emImage & windowIcon)
{
	WindowIcon=windowIcon;
	WindowPort->InvalidateIcon();
}


void emWindow::InvalidateTitle()
{
	emView::InvalidateTitle();
	WindowPort->InvalidateTitle();
}


emWindow::AutoDeleteEngineClass::AutoDeleteEngineClass(emWindow * window)
	: emEngine(window->GetScheduler())
{
	Window=window;
	CountDown=-1;
	AddWakeUpSignal(Window->GetCloseSignal());
}


bool emWindow::AutoDeleteEngineClass::Cycle()
{
	if (
		IsSignaled(Window->GetCloseSignal()) &&
		(Window->GetWindowFlags()&emWindow::WF_AUTO_DELETE)!=0
	) {
		RemoveWakeUpSignal(Window->GetCloseSignal());
		CountDown=3;
	}
	if (CountDown>0) { CountDown--; return true; }
	if (CountDown==0) delete Window;
	return false;
}


//==============================================================================
//================================ emWindowPort ================================
//==============================================================================

emWindowPort::emWindowPort(emWindow & window)
	: emViewPort(window), Window(window)
{
	if (Window.WindowPort) {
		emFatalError("Illegal use of emWindowPort.");
	}
}
