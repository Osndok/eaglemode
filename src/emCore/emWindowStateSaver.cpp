//------------------------------------------------------------------------------
// emWindowStateSaver.cpp
//
// Copyright (C) 2016 Oliver Hamann.
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

#include <emCore/emWindowStateSaver.h>


emWindowStateSaver::emWindowStateSaver(
	emWindow & window, const emString & filePath,
	bool allowRestoreFullscreen
) :
	emEngine(window.GetScheduler()),
	Window(window),
	AllowRestoreFullscreen(allowRestoreFullscreen)
{
	Model=ModelClass::Acquire(Window.GetRootContext(),filePath);

	OwnNormalX=0.0;
	OwnNormalY=0.0;
	OwnNormalW=0.0;
	OwnNormalH=0.0;

	AddWakeUpSignal(Window.GetWindowFlagsSignal());
	AddWakeUpSignal(Window.GetGeometrySignal());
	AddWakeUpSignal(Window.GetFocusSignal());

	Restore();
}


emWindowStateSaver::~emWindowStateSaver()
{
}


bool emWindowStateSaver::Cycle()
{
	if (
		IsSignaled(Window.GetWindowFlagsSignal()) ||
		IsSignaled(Window.GetGeometrySignal()) ||
		IsSignaled(Window.GetFocusSignal())
	) {
		if (Window.IsFocused()) {
			Save();
		}
	}

	return false;
}


void emWindowStateSaver::Save()
{
	emWindow::WindowFlags flags;

	flags=Window.GetWindowFlags();

	if ((flags&(emWindow::WF_MAXIMIZED|emWindow::WF_FULLSCREEN))==0) {
		OwnNormalX=Window.GetHomeX();
		OwnNormalY=Window.GetHomeY();
		OwnNormalW=Window.GetHomeWidth();
		OwnNormalH=Window.GetHomeHeight();
	}

	Model->ViewX     =OwnNormalX;
	Model->ViewY     =OwnNormalY;
	Model->ViewWidth =OwnNormalW;
	Model->ViewHeight=OwnNormalH;

	Model->Maximized=(flags&emWindow::WF_MAXIMIZED)!=0;

	Model->Fullscreen=(flags&emWindow::WF_FULLSCREEN)!=0;
}


void emWindowStateSaver::Restore()
{
	double x,y,w,h,mx,my,mw,mh,a,cw,ch,bl,bt,br,bb;
	bool maximized,fullscreen,posValid,sizeValid;
	emWindow::WindowFlags flags;
	int monitor;
	emScreen * screen;

	x=Model->ViewX.Get();
	y=Model->ViewY.Get();
	w=Model->ViewWidth.Get();
	h=Model->ViewHeight.Get();

	maximized = Model->Maximized.Get();

	fullscreen = AllowRestoreFullscreen && Model->Fullscreen.Get();

	OwnNormalX=x;
	OwnNormalY=y;
	OwnNormalW=w;
	OwnNormalH=h;

	sizeValid=(w>=32.0 && h>=32.0);
	posValid=false;
	if (sizeValid) {
		screen=&Window.GetScreen();
		monitor=0;
		if (maximized || fullscreen) {
			monitor=screen->GetMonitorIndexOfRect(x,y,w,h);
		}
		screen->GetMonitorRect(monitor,&mx,&my,&mw,&mh);
		Window.GetBorderSizes(&bl,&bt,&br,&bb);
		if (w>mw-bl-br) w=mw-bl-br;
		if (h>mh-bt-bb) h=mh-bt-bb;
		if (w<32.0 || h<32.0) sizeValid=false;
		else {
			cw=emMin(x+w,mx+mw)-emMax(x,mx);
			ch=emMin(y+h,my+mh)-emMax(y,my);
			a=emMax(0.0,cw)*emMax(0.0,ch);
			posValid=(a>=w*h*0.95);
		}
	}

	if (posValid && (maximized || fullscreen)) Window.SetViewPos(x,y);

	if (sizeValid) Window.SetViewSize(w,h);

	flags=Window.GetWindowFlags();
	if (maximized) flags|=emWindow::WF_MAXIMIZED;
	else flags&=~emWindow::WF_MAXIMIZED;
	if (fullscreen) flags|=emWindow::WF_FULLSCREEN;
	else flags&=~emWindow::WF_FULLSCREEN;
	Window.SetWindowFlags(flags);
}


emRef<emWindowStateSaver::ModelClass> emWindowStateSaver::ModelClass::Acquire(
	emRootContext & rootContext, const emString & filePath
)
{
	EM_IMPL_ACQUIRE_COMMON(ModelClass,rootContext,filePath)
}


const char * emWindowStateSaver::ModelClass::GetFormatName() const
{
	return "emWindowState";
}


emWindowStateSaver::ModelClass::ModelClass(
	emContext & context, const emString & filePath
) :
	emConfigModel(context,filePath),
	emStructRec(),
	ViewX(this,"ViewX"),
	ViewY(this,"ViewY"),
	ViewWidth(this,"ViewWidth"),
	ViewHeight(this,"ViewHeight"),
	Maximized(this,"Maximized"),
	Fullscreen(this,"Fullscreen")
{
	PostConstruct(*this,filePath);
	SetMinCommonLifetime(20);
	SetAutoSaveDelaySeconds(10);
	LoadOrInstall();
}


emWindowStateSaver::ModelClass::~ModelClass()
{
	Save();
}
