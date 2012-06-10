//------------------------------------------------------------------------------
// emView.cpp
//
// Copyright (C) 2004-2011 Oliver Hamann.
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

#include <emCore/emPanel.h>
#include <emCore/emViewInputFilter.h>


//==============================================================================
//=================================== emView ===================================
//==============================================================================

emView::emView(emContext & parentContext, ViewFlags viewFlags)
	: emContext(parentContext)
{
	CoreConfig=emCoreConfig::Acquire(GetRootContext());
	DummyViewPort=new emViewPort();
	DummyViewPort->CurrentView=this;
	DummyViewPort->HomeView=this;
	HomeViewPort=DummyViewPort;
	CurrentViewPort=DummyViewPort;
	WindowPtrCache=NULL;
	ScreenRefCache=NULL;
	WindowPtrValid=false;
	ScreenRefValid=false;
	PopupWindow=NULL;
	FirstVIF=NULL;
	LastVIF=NULL;
	RootPanel=NULL;
	SupremeViewedPanel=NULL;
	MinSVP=NULL;
	MaxSVP=NULL;
	ActivePanel=NULL;
	ActivationCandidate=NULL;
	VisitedPanel=NULL;
	PanelCreationNumber=0;
	HomeX=0.0;
	HomeY=0.0;
	HomeWidth=1.0;
	HomeHeight=1.0;
	HomePixelTallness=1.0;
	CurrentX=0.0;
	CurrentY=0.0;
	CurrentWidth=1.0;
	CurrentHeight=1.0;
	CurrentPixelTallness=1.0;
	LastMouseX=0.0;
	LastMouseY=0.0;
	Title="";
	Cursor=emCursor::NORMAL;
	BackgroundColor=0x808080FF;
	VFlags=0;
	Focused=false;
	VisitAdherent=false;
	TitleInvalid=false;
	CursorInvalid=false;
	SVPChoiceInvalid=false;
	SVPChoiceByOpacityInvalid=false;
	GotPopupWindowCloseSignal=false;
	RestartInputRecursion=false;
	ZoomedOutBeforeSG=true;
	SettingGeometry=0;
	SVPUpdCount=0;
	SVPUpdSlice=0;
	NoticeList.Prev=&NoticeList;
	NoticeList.Next=&NoticeList;
	UpdateEngine=new UpdateEngineClass(*this);
	ActivationEngine=NULL;
	EOIEngine=NULL;
	SeekEngine=NULL;
	ProtectSeeking=0;
	SeekPosPanel=NULL;
	StressTest=NULL;

	UpdateEngine->WakeUp();

	SetViewFlags(viewFlags);

	new emDefaultTouchVIF(*this);
	new emCheatVIF(*this);
	new emKeyboardZoomScrollVIF(*this);
	new emMouseZoomScrollVIF(*this);
}


emView::~emView()
{
	AbortSeeking();
	CrossPtrList.BreakCrossPtrs();
	//??? Should we delete child views here like emWindow deletes its child
	//??? windows? If so, remember to adapt emSubViewPanel. (No panic,
	//??? child views are deleted at least by the context destructor)
	if (RootPanel) delete RootPanel;
	if (StressTest) delete StressTest;
	while (LastVIF) delete LastVIF;
	if (EOIEngine) delete EOIEngine;
	if (ActivationEngine) delete ActivationEngine;
	delete UpdateEngine;
	if (HomeViewPort!=DummyViewPort) {
		emFatalError("emView::~emView: View port must be destructed first.");
	}
	DummyViewPort->HomeView=NULL;
	DummyViewPort->CurrentView=NULL;
	delete DummyViewPort;
}


void emView::SetViewFlags(ViewFlags viewFlags)
{
	ViewFlags oldFlags;

	if ((viewFlags&VF_NO_ZOOM)!=0) {
		viewFlags&=~(VF_POPUP_ZOOM|VF_EGO_MODE);
		viewFlags|=VF_NO_USER_NAVIGATION;
	}
	if (VFlags!=viewFlags) {
		oldFlags=VFlags;
		if (
			(viewFlags&VF_POPUP_ZOOM)!=0 &&
			(oldFlags&VF_POPUP_ZOOM)==0
		) {
			ZoomOut();
		}
		VFlags=viewFlags;
		if (
			(viewFlags&VF_ROOT_SAME_TALLNESS)!=0 &&
			(oldFlags&VF_ROOT_SAME_TALLNESS)==0 &&
			RootPanel
		) {
			RootPanel->Layout(0,0,1,GetHomeTallness());
		}
		if (
			(viewFlags&VF_NO_ZOOM)!=0 &&
			(oldFlags&VF_NO_ZOOM)==0
		) {
			ZoomOut();
		}
		if ((viewFlags&VF_EGO_MODE)!=(oldFlags&VF_EGO_MODE)) {
			CursorInvalid=true;
		}
		if ((viewFlags&VF_STRESS_TEST)!=0) {
			if (!StressTest) {
				StressTest=new StressTestClass(*this);
			}
		}
		else {
			if (StressTest) {
				delete StressTest;
				StressTest=NULL;
				InvalidatePainting();
			}
		}
		SVPChoiceInvalid=true;
		Signal(ViewFlagsSignal);
		UpdateEngine->WakeUp();
	}
}


void emView::SetBackgroundColor(emColor c)
{
	if (BackgroundColor!=c) {
		BackgroundColor=c;
		InvalidatePainting();
	}
}


emString emView::GetTitle()
{
	return Title;
}


emPanel * emView::CreateControlPanel(
	emPanel & parent, const emString & name
)
{
	if (!ActivePanel) return NULL;
	return ActivePanel->CreateControlPanel(parent,name);
}


void emView::Focus()
{
	if (!Focused) CurrentViewPort->RequestFocus();
}


emWindow * emView::GetWindow()
{
	emContext * c;

	if (!WindowPtrValid) {
		// Unfortunately this cannot be done in the constructor.
		for (c=this; c!=NULL; c=c->GetParentContext()) {
			WindowPtrCache=dynamic_cast<emWindow*>(c);
			if (WindowPtrCache) break;
		}
		WindowPtrValid=true;
	}
	return WindowPtrCache;
}


emScreen * emView::GetScreen()
{
	emWindow * win;

	if (!ScreenRefValid) {
		win=GetWindow();
		if (win) ScreenRefCache=&win->GetScreen();
		else ScreenRefCache=emScreen::LookupInherited(*this);
		ScreenRefValid=true;
	}
	return (emScreen*)ScreenRefCache.Get();
}


emPanel * emView::GetPanelByIdentity(const char * identity)
{
	emArray<emString> a;
	emPanel * p;
	int i;

	p=RootPanel;
	if (p) {
		a=emPanel::DecodeIdentity(identity);
		if (a.GetCount() && a[0]==p->GetName()) {
			for (i=1; ; i++) {
				if (i>=a.GetCount()) return p;
				p=p->GetChild(a[i]);
				if (!p) break;
			}
		}
	}
	return NULL;
}


emPanel * emView::GetPanelAt(double x, double y)
{
	emPanel * p, * c;

	p=SupremeViewedPanel;
	if (p && p->ClipX1<=x && p->ClipX2>x && p->ClipY1<=y && p->ClipY2>y) {
		c=p->GetLastChild();
		while (c) {
			if (c->Viewed && c->ClipX1<=x && c->ClipX2>x && c->ClipY1<=y &&
			    c->ClipY2>y) {
				p=c;
				c=p->GetLastChild();
			}
			else {
				c=c->GetPrev();
			}
		}
		return p;
	}
	else {
		return NULL;
	}
}


emPanel * emView::GetFocusablePanelAt(double x, double y)
{
	emPanel * p, * c;

	p=SupremeViewedPanel;
	if (p && p->ClipX1<=x && p->ClipX2>x && p->ClipY1<=y && p->ClipY2>y) {
		c=p->GetFocusableLastChild();
		while (c) {
			if (c->Viewed && c->ClipX1<=x && c->ClipX2>x && c->ClipY1<=y &&
			    c->ClipY2>y) {
				p=c;
				c=p->GetFocusableLastChild();
			}
			else {
				c=c->GetFocusablePrev();
			}
		}
		if (!p->IsFocusable()) p=p->GetFocusableParent();
		return p;
	}
	else {
		return NULL;
	}
}


emPanel * emView::GetVisitedPanel(
	double * pRelX, double * pRelY, double * pRelA, bool * pAdherent
)
{
	emPanel * p;

	p=VisitedPanel;
	if (p) {
		if (pRelX) *pRelX=(HomeX+HomeWidth*0.5-p->ViewedX)/p->ViewedWidth-0.5;
		if (pRelY) *pRelY=(HomeY+HomeHeight*0.5-p->ViewedY)/p->ViewedHeight-0.5;
		if (pRelA) *pRelA=(HomeWidth*HomeHeight)/(p->ViewedWidth*p->ViewedHeight);
		if (pAdherent) *pAdherent=VisitAdherent;
	}
	else {
		if (pRelX) *pRelX=0.0;
		if (pRelY) *pRelY=0.0;
		if (pRelA) *pRelA=0.0;
		if (pAdherent) *pAdherent=false;
	}
	return p;
}


void emView::Visit(emPanel * panel, bool adherent)
{
	static const double MIN_REL_DISTANCE=0.03;
	static const double MIN_REL_CIRCUMFERENCE=0.05;
	emScreen * screen;
	emPanel * p, * cp;
	double ph,dx,dy,sx,sy,sw,sh,minvw,maxvw,vx,vy,vw,vh;
	double ctx,cty,ctw,cth,csx,csy,csw,csh;

	if (!panel) return;

	if (!ProtectSeeking) AbortSeeking();

	ph=panel->GetHeight();

	sx=CurrentX;
	sy=CurrentY;
	sw=CurrentWidth;
	sh=CurrentHeight;
	if ((VFlags&VF_POPUP_ZOOM)!=0) {
		screen=GetScreen();
		if (screen) {
			screen->GetVisibleRect(&sx,&sy,&sw,&sh);
		}
	}

	dx=emMin(
		CurrentWidth*MIN_REL_DISTANCE,
		CurrentHeight*MIN_REL_DISTANCE*CurrentPixelTallness
	);
	dy=dx/CurrentPixelTallness;
	sx+=dx;
	sy+=dy;
	sw-=2*dx;
	sh-=2*dy;

	maxvw=emMin(sw,sh/ph*CurrentPixelTallness);
	minvw=emMin(
		(CurrentWidth+CurrentHeight)*MIN_REL_CIRCUMFERENCE/
			(1.0+ph/CurrentPixelTallness),
		maxvw*0.999
	);

	if (
		panel->Viewed &&
		panel->ViewedWidth>=minvw &&
		panel->ViewedWidth<=maxvw &&
		panel->ViewedX>=sx &&
		panel->ViewedX+panel->ViewedWidth<=sx+sw &&
		panel->ViewedY>=sy &&
		panel->ViewedY+panel->ViewedHeight<=sy+sh
	) {
		VisitImmobile(panel,adherent);
		return;
	}

	cp=panel;
	ctx=0.0;
	cty=0.0;
	ctw=1.0;
	cth=ph;
	while (cp!=SupremeViewedPanel && (cp->Viewed || !cp->InViewedPath)) {
		ctx=cp->LayoutX+ctx*cp->LayoutWidth;
		cty=cp->LayoutY+cty*cp->LayoutWidth;
		ctw*=cp->LayoutWidth;
		cth*=cp->LayoutWidth;
		cp=cp->Parent;
	}

	p=SupremeViewedPanel;
	csx=(sx-p->ViewedX)/p->ViewedWidth;
	csy=(sy-p->ViewedY)*CurrentPixelTallness/p->ViewedWidth;
	csw=sw/p->ViewedWidth;
	csh=sh*CurrentPixelTallness/p->ViewedWidth;
	while (p!=cp) {
		csx=p->LayoutX+csx*p->LayoutWidth;
		csy=p->LayoutY+csy*p->LayoutWidth;
		csw*=p->LayoutWidth;
		csh*=p->LayoutWidth;
		p=p->Parent;
	}

	if (ctw*sw>=maxvw*csw) vw=maxvw;
	else if (ctw*sw<=minvw*csw) vw=minvw;
	else vw=ctw/csw*sw;
	vh=vw*ph/CurrentPixelTallness;

	if (ctw>csw) {
		vx=-(csx+csw*0.5-ctx)*vw;
		if (vx<=(-sw*0.5)*ctw) vx=sx;
		else if (vx>=(sw*0.5-vw)*ctw) vx=sx+sw-vw;
		else vx=vx/ctw+sx+sw*0.5;
	}
	else {
		vx=(ctx+ctw*0.5-csx)*sw;
		if (vx<=vw*0.5*csw) vx=sx;
		else if (vx>=(sw-vw*0.5)*csw) vx=sx+sw-vw;
		else vx=vx/csw+sx-vw*0.5;
	}

	if (cth>csh) {
		vy=-(csy+csh*0.5-cty)*vh;
		if (vy<=(-sh*0.5)*cth) vy=sy;
		else if (vy>=(sh*0.5-vh)*cth) vy=sy+sh-vh;
		else vy=vy/cth+sy+sh*0.5;
	}
	else {
		vy=(cty+cth*0.5-csy)*sh;
		if (vy<=vh*0.5*csh) vy=sy;
		else if (vy>=(sh-vh*0.5)*csh) vy=sy+sh-vh;
		else vy=vy/csh+sy-vh*0.5;
	}

	VisitAbs(panel,vx,vy,vw,adherent,false);
}


void emView::Visit(
	emPanel * panel, double relX, double relY, double relA, bool adherent
)
{
	if (!ProtectSeeking) AbortSeeking();
	VisitRel(panel,relX,relY,relA,adherent,false);
}


void emView::VisitBy(emPanel * panel, double relX, double relY, double relA)
{
	if (!ProtectSeeking) AbortSeeking();
	VisitRelBy(panel,relX,relY,relA,false);
}


void emView::VisitLazy(emPanel * panel, bool adherent)
{
	if (!panel) return;
	while (!panel->Focusable) panel=panel->Parent;
	if (
		panel->Viewed || (
			!SupremeViewedPanel->Focusable &&
			SupremeViewedPanel->GetFocusableParent()==panel
		)
	) {
		if (!panel->Active || (adherent && !VisitAdherent)) {
			VisitImmobile(panel,adherent);
		}
	}
	else {
		if (!ProtectSeeking) AbortSeeking();
		Visit(panel,adherent);
	}
}


void emView::VisitFullsized(emPanel * panel, bool adherent, bool utilizeView)
{
	if (!ProtectSeeking) AbortSeeking();
	VisitRel(panel,0.0,0.0,utilizeView?-1.0:0.0,adherent,false);
}


void emView::VisitByFullsized(emPanel * panel)
{
	if (!ProtectSeeking) AbortSeeking();
	VisitRelBy(panel,0.0,0.0,0.0,false);
}


void emView::VisitNext()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	p=ActivePanel;
	if (p) {
		p=p->GetFocusableNext();
		if (!p) {
			p=ActivePanel->GetFocusableParent();
			if (!p) p=RootPanel;
			if (p!=ActivePanel) p=p->GetFocusableFirstChild();
		}
		Visit(p,true);
	}
}


void emView::VisitPrev()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	p=ActivePanel;
	if (p) {
		p=p->GetFocusablePrev();
		if (!p) {
			p=ActivePanel->GetFocusableParent();
			if (!p) p=RootPanel;
			if (p!=ActivePanel) p=p->GetFocusableLastChild();
		}
		Visit(p,true);
	}
}


void emView::VisitFirst()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (ActivePanel) {
		p=ActivePanel->GetFocusableParent();
		if (p) p=p->GetFocusableFirstChild();
		if (!p) p=ActivePanel;
		Visit(p,true);
	}
}


void emView::VisitLast()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (ActivePanel) {
		p=ActivePanel->GetFocusableParent();
		if (p) p=p->GetFocusableLastChild();
		if (!p) p=ActivePanel;
		Visit(p,true);
	}
}


void emView::VisitLeft()
{
	VisitNeighbour(2);
}


void emView::VisitRight()
{
	VisitNeighbour(0);
}


void emView::VisitUp()
{
	VisitNeighbour(3);
}


void emView::VisitDown()
{
	VisitNeighbour(1);
}


void emView::VisitNeighbour(int direction)
{
	emPanel * p, * n, * current, * parent, * best;
	double cx1,cy1,cx2,cy2,nx1,ny1,nx2,ny2,dx,dy,d,e,fx,fy,f,bestVal,val,defdx;

	if (!ProtectSeeking) AbortSeeking();
	direction&=3;
	current=ActivePanel;
	if (!current) return;
	parent=current->GetFocusableParent();
	if (!parent) parent=RootPanel;
	if (parent!=current) {
		cx1=0; cy1=0; cx2=1.0; cy2=current->GetHeight();
		for (p=current; p!=parent; p=p->GetParent()) {
			f=p->GetLayoutWidth();
			fx=p->GetLayoutX();
			fy=p->GetLayoutY();
			cx1=cx1*f+fx;
			cy1=cy1*f+fy;
			cx2=cx2*f+fx;
			cy2=cy2*f+fy;
		}
		best=NULL;
		bestVal=0.0;
		defdx=-1.0;
		for (n=parent->GetFocusableFirstChild(); n; n=n->GetFocusableNext()) {
			if (n==current) { defdx=-defdx; continue; }
			nx1=0; ny1=0; nx2=1.0; ny2=n->GetHeight();
			for (p=n; p!=parent; p=p->GetParent()) {
				f=p->GetLayoutWidth();
				fx=p->GetLayoutX();
				fy=p->GetLayoutY();
				nx1=nx1*f+fx;
				ny1=ny1*f+fy;
				nx2=nx2*f+fx;
				ny2=ny2*f+fy;
			}
			dx=0.0;
			dy=0.0;
			fx=nx1-cx1;
			fy=ny1-cy1;
			f=sqrt(fx*fx+fy*fy);
			if (f>1E-30) { dx+=fx/f; dy+=fy/f; }
			fx=nx2-cx2;
			fy=ny1-cy1;
			f=sqrt(fx*fx+fy*fy);
			if (f>1E-30) { dx+=fx/f; dy+=fy/f; }
			fx=nx1-cx1;
			fy=ny2-cy2;
			f=sqrt(fx*fx+fy*fy);
			if (f>1E-30) { dx+=fx/f; dy+=fy/f; }
			fx=nx2-cx2;
			fy=ny2-cy2;
			f=sqrt(fx*fx+fy*fy);
			if (f>1E-30) { dx+=fx/f; dy+=fy/f; }
			f=sqrt(dx*dx+dy*dy);
			if (f>1E-30) { dx/=f; dy/=f; }
			else { dx=defdx; dy=0.0; }
			fx=(nx1+nx2-cx1-cx2)*0.5;
			fy=(ny1+ny2-cy1-cy2)*0.5;
			d=sqrt(fx*fx+fy*fy);
			if (nx2<cx1) fx=nx2-cx1;
			else if (nx1>cx2) fx=nx1-cx2;
			else fx=0.0;
			if (ny2<cy1) fy=ny2-cy1;
			else if (ny1>cy2) fy=ny1-cy2;
			else fy=0.0;
			e=sqrt(fx*fx+fy*fy);
			if ((direction&1)!=0) {
				f=dx;
				dx=dy;
				dy=-f;
			}
			if ((direction&2)!=0) {
				dx=-dx;
				dy=-dy;
			}
			if (dx<=1E-12) continue;
			val=(e*10+d)*(1+2*dy*dy);
			if (fabs(dy)>0.707) val*=1000*dy*dy*dy*dy;
			if (!best || val<bestVal) {
				best=n;
				bestVal=val;
			}
		}
		if (best) current=best;
	}
	Visit(current,true);
}


void emView::VisitIn()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (!ActivePanel) return;
	p=ActivePanel->GetFocusableFirstChild();
	if (p) Visit(p,true);
	else VisitFullsized(ActivePanel,true);
}


void emView::VisitOut()
{
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (!ActivePanel) return;
	p=ActivePanel->GetFocusableParent();
	if (p) Visit(p,true);
	else {
		ZoomOut();
		VisitImmobile(RootPanel,true);
	}
}


void emView::Seek(const char * identity, bool adherent, const char * subject)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		Visit(p,adherent);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,1,identity,0.0,0.0,0.0,adherent,subject);
}


void emView::Seek(
	const char * identity, double relX, double relY, double relA,
	bool adherent, const char * subject
)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		Visit(p,relX,relY,relA,adherent);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,2,identity,relX,relY,relA,adherent,subject);
}


void emView::SeekBy(
	const char * identity, double relX, double relY, double relA,
	const char * subject
)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		VisitBy(p,relX,relY,relA);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,3,identity,relX,relY,relA,false,subject);
}


void emView::SeekLazy(
	const char * identity, bool adherent, const char * subject
)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		VisitLazy(p,adherent);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,4,identity,0.0,0.0,0.0,adherent,subject);
}


void emView::SeekFullsized(
	const char * identity, bool adherent, const char * subject
)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		VisitFullsized(p,adherent);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,5,identity,0.0,0.0,0.0,adherent,subject);
}


void emView::SeekByFullsized(const char * identity, const char * subject)
{
	emPanel * p;

	AbortSeeking();
	p=GetPanelByIdentity(identity);
	if (p) {
		VisitByFullsized(p);
		return;
	}
	if (!subject) subject="";
	SeekEngine=new SeekEngineClass(*this,6,identity,0.0,0.0,0.0,false,subject);
}


void emView::AbortSeeking()
{
	if (SeekEngine) {
		delete SeekEngine;
		SeekEngine=NULL;
		ProtectSeeking=0;
		SetSeekPos(NULL,NULL);
		InvalidatePainting();
	}
}


void emView::Zoom(double fixX, double fixY, double factor)
{
	double rx,ry,ra,reFac;
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (factor!=1.0 && factor>0.0) {
		p=GetVisitedPanel(&rx,&ry,&ra);
		if (p) {
			reFac=1.0/factor;
			rx+=(fixX-(HomeX+HomeWidth*0.5))*(1.0-reFac)/p->ViewedWidth;
			ry+=(fixY-(HomeY+HomeHeight*0.5))*(1.0-reFac)/p->ViewedHeight;
			ra*=reFac*reFac;
			VisitRelBy(p,rx,ry,ra,true);
		}
	}
}


void emView::Scroll(double deltaX, double deltaY)
{
	double rx,ry,ra;
	emPanel * p;

	if (!ProtectSeeking) AbortSeeking();
	if (deltaX!=0.0 || deltaY!=0.0) {
		p=GetVisitedPanel(&rx,&ry,&ra);
		if (p) {
			rx+=deltaX/p->ViewedWidth;
			ry+=deltaY/p->ViewedHeight;
			VisitRelBy(p,rx,ry,ra,true);
		}
	}
}


void emView::ZoomOut()
{
	double relA,relA2;

	if (!ProtectSeeking) AbortSeeking();

	if (RootPanel) {
		relA=HomeWidth*RootPanel->GetHeight()/HomePixelTallness/HomeHeight;
		relA2=HomeHeight/RootPanel->GetHeight()*HomePixelTallness/HomeWidth;
		if (relA<relA2) relA=relA2;
		VisitRelBy(RootPanel,0.0,0.0,relA,true);
	}

	if (IsPoppedUp()) {
		emFatalError("emView::ZoomOut: Inconsistent algorithms.");
	}
}


bool emView::IsZoomedOut()
{
	double x,y,w,h;
	emPanel * p;

	if (SettingGeometry) return ZoomedOutBeforeSG;
	if (VFlags&VF_POPUP_ZOOM) return PopupWindow==NULL;
	p=SupremeViewedPanel;
	if (!p) return true;
	x=(HomeX-p->ViewedX)/p->ViewedWidth;
	y=(HomeY-p->ViewedY)*HomePixelTallness/p->ViewedWidth;
	w=HomeWidth/p->ViewedWidth;
	h=HomeHeight*HomePixelTallness/p->ViewedWidth;
	while (p->Parent) {
		x=p->LayoutX+x*p->LayoutWidth;
		y=p->LayoutY+y*p->LayoutWidth;
		w*=p->LayoutWidth;
		h*=p->LayoutWidth;
		p=p->Parent;
	}
	return
		x<=0.001 &&
		y<=0.001 &&
		x+w>=1.0-0.001 &&
		y+h>=p->GetHeight()-0.001
	;
}


void emView::SignalEOIDelayed()
{
	if (!EOIEngine) EOIEngine=new EOIEngineClass(*this);
}


double emView::GetTouchEventPriority(
	double touchX, double touchY, bool afterVIFs
)
{
	emPanel * p;
	double pri,t;

	if (!afterVIFs && FirstVIF) {
		return FirstVIF->GetTouchEventPriority(touchX,touchY);
	}
	pri=-1E30;
	p=RootPanel;
	if (p) {
		for (;;) {
			if (
				p->InViewedPath && (
					!p->Viewed || (
						p->ClipX1<=touchX &&
						p->ClipY1<=touchY &&
						p->ClipX2>touchX &&
						p->ClipY2>touchY
					)
				)
			) {
				t=p->GetTouchEventPriority(touchX,touchY);
				if (pri<t) pri=t;
			}
			if (p->FirstChild) p=p->FirstChild;
			else if (p->Next) p=p->Next;
			else {
				do {
					p=p->Parent;
				} while (p && !p->Next);
				if (!p) break;
				p=p->Next;
			}
		}
	}
	return pri;
}


void emView::Input(emInputEvent & event, const emInputState & state)
{
	emPanel * p;

	if (SeekEngine && !event.IsEmpty()) {
		event.Eat();
		AbortSeeking();
	}

	if (
		fabs(state.GetMouseX()-LastMouseX)>0.1 ||
		fabs(state.GetMouseY()-LastMouseY)>0.1
	) {
		LastMouseX=state.GetMouseX();
		LastMouseY=state.GetMouseY();
		CursorInvalid=true;
		UpdateEngine->WakeUp();
	}

	p=RootPanel;
	if (p) {
		for (;;) {
			p->PendingInput=p->InViewedPath;
			if (p->FirstChild) p=p->FirstChild;
			else if (p->Next) p=p->Next;
			else {
				do {
					p=p->Parent;
				} while (p && !p->Next);
				if (!p) break;
				p=p->Next;
			}
		}
	}

	do {
		RestartInputRecursion=false;
		RecurseInput(event,state);
		if (RestartInputRecursion) {
			emDLog("emView %p: Restarting input recursion.",this);
		}
	} while (RestartInputRecursion);
}


emCursor emView::GetCursor()
{
	return Cursor;
}


void emView::Paint(const emPainter & painter, emColor canvasColor)
{
	emColor ncc;
	emPainter pnt;
	emPanel * p;
	double rx1,ry1,rx2,ry2,ox,oy,cx1,cy1,cx2,cy2;

	if (painter.GetScaleX()!=1.0 || painter.GetScaleY()!=1.0) {
		emFatalError("emView::Paint: Scaling not possible.");
	}

	if (!SupremeViewedPanel) {
		painter.Clear(BackgroundColor,canvasColor);
	}
	else {
		ox=painter.GetOriginX();
		oy=painter.GetOriginY();
		rx1=painter.GetClipX1()-ox;
		ry1=painter.GetClipY1()-oy;
		rx2=painter.GetClipX2()-ox;
		ry2=painter.GetClipY2()-oy;
		p=SupremeViewedPanel;
		if (
			!p->IsOpaque() ||
			p->ViewedX                >rx1 ||
			p->ViewedX+p->ViewedWidth <rx2 ||
			p->ViewedY                >ry1 ||
			p->ViewedY+p->ViewedHeight<ry2
		) {
			ncc=p->CanvasColor;
			if (!ncc.IsOpaque()) ncc=BackgroundColor;
			painter.Clear(ncc,canvasColor);
			canvasColor=ncc;
		}
		cx1=p->ClipX1; if (cx1<rx1) cx1=rx1;
		cx2=p->ClipX2; if (cx2>rx2) cx2=rx2;
		cy1=p->ClipY1; if (cy1<ry1) cy1=ry1;
		cy2=p->ClipY2; if (cy2>ry2) cy2=ry2;
		if (cx1<cx2 && cy1<cy2) {
			pnt=painter;
			pnt.SetClipping(cx1+ox,cy1+oy,cx2+ox,cy2+oy);
			pnt.SetTransformation(
				p->ViewedX+ox,
				p->ViewedY+oy,
				p->ViewedWidth,
				p->ViewedWidth/CurrentPixelTallness
			);
			p->Paint(pnt,canvasColor);
			p=p->FirstChild;
			if (p) {
				for (;;) {
					if (p->Viewed) {
						cx1=p->ClipX1; if (cx1<rx1) cx1=rx1;
						cx2=p->ClipX2; if (cx2>rx2) cx2=rx2;
						if (cx1<cx2) {
							cy1=p->ClipY1; if (cy1<ry1) cy1=ry1;
							cy2=p->ClipY2; if (cy2>ry2) cy2=ry2;
							if (cy1<cy2) {
								pnt.SetClipping(cx1+ox,cy1+oy,cx2+ox,cy2+oy);
								pnt.SetTransformation(
									p->ViewedX+ox,
									p->ViewedY+oy,
									p->ViewedWidth,
									p->ViewedWidth/CurrentPixelTallness
								);
								p->Paint(pnt,p->CanvasColor);
								if (p->FirstChild) {
									p=p->FirstChild;
									continue;
								}
							}
						}
					}
					if (p->Next) p=p->Next;
					else {
						do {
							p=p->Parent;
						} while (p!=SupremeViewedPanel && !p->Next);
						if (p==SupremeViewedPanel) break;
						p=p->Next;
					}
				}
			}
		}
		PaintHighlight(painter);
	}

	if (SeekEngine) SeekEngine->Paint(painter);
	if (StressTest) StressTest->PaintInfo(painter);
}


void emView::InvalidateTitle()
{
	Signal(TitleSignal);
}


void emView::DoCustomCheat(const char * func)
{
	emContext * c;
	emView * v;

	for (c=GetParentContext(); c; c=c->GetParentContext()) {
		v=dynamic_cast<emView*>(c);
		if (v) {
			v->DoCustomCheat(func);
			break;
		}
	}
}


void emView::SetFocused(bool focused)
{
	emPanel * p;
	emPanel::NoticeFlags flags;

	if (Focused==focused) return;
	if (Focused) InvalidateHighlight();
	Focused=focused;
	if (Focused) InvalidateHighlight();
	Signal(FocusSignal);
	p=RootPanel;
	if (p) {
		for (;;) {
			flags=
				emPanel::NF_VIEW_FOCUS_CHANGED |
				emPanel::NF_UPDATE_PRIORITY_CHANGED
			;
			if (p->InActivePath) flags|=emPanel::NF_FOCUS_CHANGED;
			p->AddPendingNotice(flags);
			if (p->FirstChild) p=p->FirstChild;
			else if (p->Next) p=p->Next;
			else {
				do {
					p=p->Parent;
				} while (p && !p->Next);
				if (!p) break;
				p=p->Next;
			}
		}
	}
}


void emView::SetGeometry(
	double x, double y, double width, double height, double pixelTallness
)
{
	double rx,ry,ra,ra2;
	emPanel * p;
	bool adherent;

	if (width<0.0001) width=0.0001;
	if (height<0.0001) height=0.0001;
	if (pixelTallness<0.0001) pixelTallness=0.0001;
	if (
		CurrentX==x && CurrentY==y &&
		CurrentWidth==width && CurrentHeight==height &&
		CurrentPixelTallness==pixelTallness
	) return;

	ZoomedOutBeforeSG=IsZoomedOut();
	SettingGeometry++;
	p=GetVisitedPanel(&rx,&ry,&ra,&adherent);
	CurrentViewPort->HomeView->HomeX=x;
	CurrentViewPort->HomeView->HomeY=y;
	CurrentViewPort->HomeView->HomeWidth=width;
	CurrentViewPort->HomeView->HomeHeight=height;
	CurrentViewPort->HomeView->HomePixelTallness=pixelTallness;
	CurrentX=x;
	CurrentY=y;
	CurrentWidth=width;
	CurrentHeight=height;
	CurrentPixelTallness=pixelTallness;
	CurrentViewPort->HomeView->Signal(GeometrySignal);
	Signal(GeometrySignal);
	if ((VFlags&VF_ROOT_SAME_TALLNESS)!=0 && RootPanel) {
		RootPanel->Layout(0,0,1,GetHomeTallness());
	}
	if (ZoomedOutBeforeSG) {
		if (RootPanel) {
			ra=HomeWidth*RootPanel->GetHeight()/HomePixelTallness/HomeHeight;
			ra2=HomeHeight/RootPanel->GetHeight()*HomePixelTallness/HomeWidth;
			if (ra<ra2) ra=ra2;
			VisitRelBy(RootPanel,0.0,0.0,ra,true);
		}
	}
	else if (p) {
		VisitRel(p,rx,ry,ra,adherent,true);
	}
	SettingGeometry--;
}


void emView::AddToNoticeList(PanelRingNode * node)
{
	node->Next=&NoticeList;
	node->Prev=NoticeList.Prev;
	node->Prev->Next=node;
	NoticeList.Prev=node;
	UpdateEngine->WakeUp();
}


void emView::Update()
{
	PanelRingNode * n;
	emPanel * p;
	emString tmp;
	emCursor cur;

	if (PopupWindow && IsSignaled(PopupWindow->GetCloseSignal())) {
		GotPopupWindowCloseSignal=true;
		ZoomOut();
	}

	for (;;) {
		n=NoticeList.Next;
		if (n!=&NoticeList) {
			do {
				NoticeList.Next=n->Next;
				NoticeList.Next->Prev=&NoticeList;
				n->Prev=NULL;
				n->Next=NULL;
				p=(emPanel*)(((char*)n)-offsetof(emPanel,NoticeNode));
				p->HandleNotice();
				n=NoticeList.Next;
			} while (n!=&NoticeList);
		}
		else if (SVPChoiceByOpacityInvalid) {
			SVPChoiceByOpacityInvalid=false;
			if (!SVPChoiceInvalid && MinSVP!=MaxSVP) {
				for (p=MinSVP; p!=MaxSVP; p=p->Parent) {
					if (p->CanvasColor.IsOpaque() || p->IsOpaque()) break;
				}
				if (SupremeViewedPanel!=p) {
					emDLog("emView %p: SVP choice invalid by opacity.",this);
					SVPChoiceInvalid=true;
				}
			}
		}
		else if (SVPChoiceInvalid) {
			SVPChoiceInvalid=false;
			if (VisitedPanel) {
				VisitAbs(
					VisitedPanel,
					VisitedPanel->ViewedX,
					VisitedPanel->ViewedY,
					VisitedPanel->ViewedWidth,
					VisitAdherent,
					false
				);
			}
		}
		else if (TitleInvalid) {
			TitleInvalid=false;
			if (ActivePanel) tmp=ActivePanel->GetTitle();
			else tmp="";
			if (Title!=tmp) {
				Title=tmp;
				InvalidateTitle();
			}
		}
		else if (CursorInvalid) {
			CursorInvalid=false;
			p=GetPanelAt(LastMouseX,LastMouseY);
			if (p) cur=p->GetCursor();
			else cur=emCursor::NORMAL;
			if ((VFlags&VF_EGO_MODE)!=0) {
				if (cur==emCursor::NORMAL) cur=emCursor::CROSSHAIR;
			}
			if (Cursor!=cur) {
				Cursor=cur;
				InvalidateCursor();
			}
		}
		else {
			break;
		}
	}
}


void emView::CalcVisitFullsizedCoords(
	emPanel * panel, double * pRelX, double * pRelY, double * pRelA,
	bool utilizeView
)
{
	double fx,fy,fw,fh,ph,vx,vy,vw,vh,ex,ey,ew,eh;
	emScreen * screen;

	fx=HomeX;
	fy=HomeY;
	fw=HomeWidth;
	fh=HomeHeight;
	if ((VFlags&VF_POPUP_ZOOM)!=0) {
		screen=GetScreen();
		if (screen) screen->GetVisibleRect(&fx,&fy,&fw,&fh);
	}

	panel->GetEssenceRect(&ex,&ey,&ew,&eh);
	ph=panel->GetHeight();
	if ((ew*fh*HomePixelTallness>=eh*fw) != utilizeView) {
		vw=fw/ew;
		vh=vw*ph/HomePixelTallness;
	}
	else {
		vh=fh/eh*ph;
		vw=vh/ph*HomePixelTallness;
	}
	vx=fx+fw*0.5-(ex+ew*0.5)*vw;
	vy=fy+fh*0.5-(ey+eh*0.5)/ph*vh;

	*pRelX=(HomeX+HomeWidth*0.5-vx)/vw-0.5;
	*pRelY=(HomeY+HomeHeight*0.5-vy)/vh-0.5;
	*pRelA=(HomeWidth*HomeHeight)/(vw*vh);
}


void emView::VisitRelBy(
	emPanel * panel, double relX, double relY, double relA,
	bool forceViewingUpdate
)
{
	emPanel * best, * p, * oldActive;
	double cx,cy,cw,ch,ex,ey,ew,eh,minW,minH,minA;
	bool adherent,oldAdherent;

	if (!panel) return;

	oldActive=ActivePanel;
	oldAdherent=VisitAdherent;
	VisitRel(panel,relX,relY,relA,false,forceViewingUpdate);

	cx=CurrentX;
	cy=CurrentY;
	cw=CurrentWidth;
	ch=CurrentHeight;
	if (PopupWindow) {
		PopupWindow->GetScreen().GetVisibleRect(&ex,&ey,&ew,&eh);
		if (ex<cx) { ew-=cx-ex; ex=cx; }
		if (ey<cy) { eh-=cy-ey; ey=cy; }
		if (ew>cx+cw-ex) { ew=cx+cw-ex; }
		if (eh>cy+ch-ey) { eh=cy+ch-ey; }
		if (ew>=10.0 && eh>=10.0) {
			cx=ex; cy=ey;
			cw=ew; ch=eh;
		}
	}
	minW=cw*0.99;
	minH=ch*0.99;
	minA=cw*ch*0.33;
	cx+=cw*0.5;
	cy+=ch*0.5;
	best=SupremeViewedPanel;
	for (;;) {
		p=best->GetFocusableLastChild();
		if (!p) break;
		do {
			if (
				p->Viewed &&
				p->ClipX1<=cx && p->ClipX2>cx &&
				p->ClipY1<=cy && p->ClipY2>cy
			) break;
			p=p->GetFocusablePrev();
		} while(p);
		if (!p) break;
		if (
			p->ClipX2-p->ClipX1<minW &&
			p->ClipY2-p->ClipY1<minH &&
			(p->ClipX2-p->ClipX1)*(p->ClipY2-p->ClipY1)<minA
		) break;
		best=p;
	}
	while (!best->Focusable) best=best->Parent;
	adherent=false;
	if (
		oldAdherent &&
		oldActive &&
		oldActive->Viewed &&
		oldActive->ViewedWidth>=4 &&
		oldActive->ViewedHeight>=4
	) {
		for (p=oldActive; p; p=p->Parent) {
			if (p==best) {
				best=oldActive;
				adherent=true;
				break;
			}
		}
	}

	VisitImmobile(best,adherent);
}


void emView::VisitRel(
	emPanel * panel, double relX, double relY, double relA, bool adherent,
	bool forceViewingUpdate
)
{
	double vx,vy,vw,vh;

	if (!panel) return;
	if (relA<=0.0) CalcVisitFullsizedCoords(panel,&relX,&relY,&relA,relA<-0.9);
	vw=sqrt(HomeWidth*HomeHeight*HomePixelTallness/(relA*panel->GetHeight()));
	vh=vw*panel->GetHeight()/HomePixelTallness;
	vx=HomeX+HomeWidth*0.5-(relX+0.5)*vw;
	vy=HomeY+HomeHeight*0.5-(relY+0.5)*vh;
	VisitAbs(panel,vx,vy,vw,adherent,forceViewingUpdate);
}


void emView::VisitAbs(
	emPanel * panel, double vx, double vy, double vw, bool adherent,
	bool forceViewingUpdate
)
{
	emPanel * vp, * p, * sp;
	double w,h,vh,x1,y1,x2,y2,sx,sy,sw,sh;
	bool wasFocused;

	if (!panel) return;

	SVPChoiceByOpacityInvalid=false;
	SVPChoiceInvalid=false;

	if (VFlags&VF_NO_ZOOM) {
		vp=RootPanel;
		h=vp->GetHeight();
		if (CurrentHeight*CurrentPixelTallness>=CurrentWidth*h) {
			vw=CurrentWidth;
			vx=CurrentX;
			vy=CurrentY+(CurrentHeight-vw*h/CurrentPixelTallness)*0.5;
		}
		else {
			vw=CurrentHeight*CurrentPixelTallness/h;
			vx=CurrentX+(CurrentWidth-vw)*0.5;
			vy=CurrentY;
		}
	}
	else {
		vp=panel;
	}

	for (;;) {
		p=vp->Parent;
		if (!p) break;
		w=vw/vp->LayoutWidth;
		if (w>MaxSVPSize || w*p->GetHeight()>MaxSVPSize) break;
		vx-=vp->LayoutX*w;
		vy-=vp->LayoutY*w/CurrentPixelTallness;
		vw=w;
		vp=p;
	}

	if (vp==RootPanel) {
		vh=vp->GetHeight()*vw/HomePixelTallness;
		if (vw<HomeWidth && vh<HomeHeight) {
			vx=(HomeX+HomeWidth*0.5-vx)/vw;
			vy=(HomeY+HomeHeight*0.5-vy)/vh;
			if (vh*HomeWidth<vw*HomeHeight) {
				vw=HomeWidth;
				vh=vw*vp->GetHeight()/HomePixelTallness;
			}
			else {
				vh=HomeHeight;
				vw=vh/vp->GetHeight()*HomePixelTallness;
			}
			vx=HomeX+HomeWidth*0.5-vx*vw;
			vy=HomeY+HomeHeight*0.5-vy*vh;
		}

		if ((VFlags&VF_EGO_MODE)!=0) {
			x1=x2=HomeX+HomeWidth*0.5;
			y1=y2=HomeY+HomeHeight*0.5;
		}
		else {
			if (vh*HomeWidth<vw*HomeHeight) {
				x1=HomeX;
				x2=HomeX+HomeWidth;
				y1=HomeY+HomeHeight*0.5-HomeWidth*vp->GetHeight()/HomePixelTallness*0.5;
				y2=HomeY+HomeHeight*0.5+HomeWidth*vp->GetHeight()/HomePixelTallness*0.5;
			}
			else {
				x1=HomeX+HomeWidth*0.5-HomeHeight/vp->GetHeight()*HomePixelTallness*0.5;
				x2=HomeX+HomeWidth*0.5+HomeHeight/vp->GetHeight()*HomePixelTallness*0.5;
				y1=HomeY;
				y2=HomeY+HomeHeight;
			}
		}
		if (vx>x1) vx=x1;
		if (vx<x2-vw) vx=x2-vw;
		if (vy>y1) vy=y1;
		if (vy<y2-vh) vy=y2-vh;

		if ((VFlags&VF_POPUP_ZOOM)!=0) {
			if (vx<HomeX-0.1 || vx+vw>HomeX+HomeWidth+0.1 ||
			    vy<HomeY-0.1 || vy+vh>HomeY+HomeHeight+0.1) {
				if (!PopupWindow) {
					wasFocused=Focused;
					PopupWindow=new emWindow(
						*this,
						0,
						emWindow::WF_POPUP,
						"emViewPopup"
					);
					GotPopupWindowCloseSignal=false;
					UpdateEngine->AddWakeUpSignal(PopupWindow->GetCloseSignal());
					PopupWindow->SetBackgroundColor(GetBackgroundColor());
					SwapViewPorts(true);
					if (wasFocused && !Focused) CurrentViewPort->RequestFocus();
				}
				sw=PopupWindow->GetScreen().GetWidth();
				sh=PopupWindow->GetScreen().GetHeight();
				x1=floor(vx);
				y1=floor(vy);
				x2=ceil(vx+vw);
				y2=ceil(vy+vh);
				if (y1<0.0) y1=0.0;
				if (x1<0.0) x1=0.0;
				if (x2>sw) x2=sw;
				if (y2>sh) y2=sh;
				if (x2<x1+1.0) x2=x1+1.0;
				if (y2<y1+1.0) y2=y1+1.0;
				if (fabs(x1-CurrentX)>0.01 || fabs(x2-CurrentX-CurrentWidth)>0.01 ||
				    fabs(y1-CurrentY)>0.01 || fabs(y2-CurrentY-CurrentHeight)>0.01) {
					SwapViewPorts(false);
					PopupWindow->SetViewPosSize(x1,y1,x2-x1,y2-y1);
					SwapViewPorts(false);
					forceViewingUpdate=true;
				}
			}
			else if (PopupWindow) {
				wasFocused=Focused;
				SwapViewPorts(true);
				delete PopupWindow;
				PopupWindow=NULL;
				Signal(GeometrySignal);
				forceViewingUpdate=true;
				if (wasFocused && !Focused && !GotPopupWindowCloseSignal) {
					CurrentViewPort->RequestFocus();
				}
			}
		}
	}

	FindBestSVP(&vp,&vx,&vy,&vw);

	p=vp;
	w=vw;
	for (;;) {
		sp=p;
		p=p->Parent;
		if (!p) break;
		w=w/sp->LayoutWidth;
		if (w>MaxSVPSize || w*p->GetHeight()>MaxSVPSize) break;
	}
	MaxSVP=sp;

	sp=vp;
	sx=vx;
	sy=vy;
	sw=vw;
	for (;;) {
		p=sp->LastChild;
		if (!p) break;
		x1=(CurrentX-sx)/sw;
		x2=x1+CurrentWidth/sw;
		y1=(CurrentY-sy)*(CurrentPixelTallness/sw);
		y2=y1+CurrentHeight*(CurrentPixelTallness/sw);
		do {
			if (
				p->LayoutX<x2 && p->LayoutX+p->LayoutWidth>x1 &&
				p->LayoutY<y2 && p->LayoutY+p->LayoutHeight>y1
			) break;
			p=p->Prev;
		} while (p);
		if (
			!p || p->LayoutX>x1 || p->LayoutX+p->LayoutWidth<x2 ||
			p->LayoutY>y1 || p->LayoutY+p->LayoutHeight<y2
		) break;
		sp=p;
		sx+=p->LayoutX*sw;
		sy+=p->LayoutY*sw/CurrentPixelTallness;
		sw*=p->LayoutWidth;
	}
	MinSVP=sp;

	if (
		forceViewingUpdate ||
		SupremeViewedPanel!=vp ||
		fabs(vp->ViewedX-vx)>=0.001 ||
		fabs(vp->ViewedY-vy)>=0.001 ||
		fabs(vp->ViewedWidth-vw)>=0.001
	) {
		if (SVPUpdSlice!=GetScheduler().GetTimeSliceCounter()) {
			SVPUpdSlice=GetScheduler().GetTimeSliceCounter();
			SVPUpdCount=0;
		}
		SVPUpdCount++;
		if (SVPUpdCount>1000) {
			// Get out of a very unlikely situation where we have an
			// end-less loop of choosing a different SVP and
			// creating/destroying panels through the notice
			// mechanism. SVP choice depends on floating-point
			// calculations and that can result different in a
			// repetition with the same input numbers...
			if (SVPUpdCount%1000==1 || SVPUpdCount>10000) {
				vx+=emGetDblRandom(-0.01,0.01);
				vy+=emGetDblRandom(-0.01,0.01);
				vw*=emGetDblRandom(0.9999999999,1.0000000001);
			}
		}
		if (emIsDLogEnabled()) {
			emDLog("emView %p: SVP=\"%s\"",this,vp->GetIdentity().Get());
		}
		p=SupremeViewedPanel;
		if (p) {
			p->InViewedPath=0;
			p->Viewed=0;
			p->AddPendingNotice(
				emPanel::NF_VIEWING_CHANGED |
				emPanel::NF_UPDATE_PRIORITY_CHANGED |
				emPanel::NF_MEMORY_LIMIT_CHANGED
			);
			p->UpdateChildrenViewing();
			for (;;) {
				p=p->Parent;
				if (!p) break;
				p->InViewedPath=0;
				p->AddPendingNotice(
					emPanel::NF_VIEWING_CHANGED |
					emPanel::NF_UPDATE_PRIORITY_CHANGED |
					emPanel::NF_MEMORY_LIMIT_CHANGED
				);
			}
		}
		SupremeViewedPanel=vp;
		vp->InViewedPath=1;
		vp->Viewed=1;
		vp->ViewedX=vx;
		vp->ViewedY=vy;
		vp->ViewedWidth=vw;
		vp->ViewedHeight=vw*vp->GetHeight()/CurrentPixelTallness;
		vp->ClipX1=vp->ViewedX;
		if (vp->ClipX1<CurrentX) vp->ClipX1=CurrentX;
		vp->ClipY1=vp->ViewedY;
		if (vp->ClipY1<CurrentY) vp->ClipY1=CurrentY;
		vp->ClipX2=vp->ViewedX+vp->ViewedWidth;
		if (vp->ClipX2>CurrentX+CurrentWidth) vp->ClipX2=CurrentX+CurrentWidth;
		vp->ClipY2=vp->ViewedY+vp->ViewedHeight;
		if (vp->ClipY2>CurrentY+CurrentHeight) vp->ClipY2=CurrentY+CurrentHeight;
		vp->AddPendingNotice(
			emPanel::NF_VIEWING_CHANGED |
			emPanel::NF_UPDATE_PRIORITY_CHANGED |
			emPanel::NF_MEMORY_LIMIT_CHANGED
		);
		vp->UpdateChildrenViewing();
		for (p=vp->Parent; p; p=p->Parent) {
			p->InViewedPath=1;
			p->AddPendingNotice(
				emPanel::NF_VIEWING_CHANGED |
				emPanel::NF_UPDATE_PRIORITY_CHANGED |
				emPanel::NF_MEMORY_LIMIT_CHANGED
			);
		}
		RestartInputRecursion=true;
		CursorInvalid=true;
		UpdateEngine->WakeUp();
		InvalidatePainting();
	}

	VisitImmobile(panel,adherent);
}


void emView::VisitImmobile(emPanel * panel, bool adherent)
{
	emPanel::NoticeFlags flags;
	emPanel * p, * vp, * ap;
	bool apChanged,vpChanged,adherentChanged;

	if (!panel) return;

	while (!panel->Focusable) panel=panel->Parent;
	if (panel->Viewed) {
		ap=panel;
		vp=panel;
	}
	else if (panel->InViewedPath) {
		vp=SupremeViewedPanel;
		ap=vp;
		while (!ap->Focusable) ap=ap->Parent;
		if (panel!=ap) adherent=false;
	}
	else {
		for (;;) {
			panel=panel->Parent;
			if (!panel) {
				vp=SupremeViewedPanel;
				ap=vp;
				while (!ap->Focusable) ap=ap->Parent;
				break;
			}
			if (panel->Viewed && panel->Focusable) {
				ap=panel;
				vp=panel;
				break;
			}
		}
		adherent=false;
	}

	vpChanged = (VisitedPanel!=vp);
	apChanged = (ActivePanel!=ap);
	adherentChanged = (VisitAdherent!=adherent);

	if (apChanged && ActivePanel) InvalidateHighlight();

	if (vpChanged) {
		if (VisitedPanel) {
			p=VisitedPanel;
			p->Visited=0;
			do {
				p->InVisitedPath=0;
				p->AddPendingNotice(emPanel::NF_VISIT_CHANGED);
				p=p->Parent;
			} while (p);
		}
		VisitedPanel=p=vp;
		p->Visited=1;
		do {
			p->InVisitedPath=1;
			p->AddPendingNotice(emPanel::NF_VISIT_CHANGED);
			p=p->Parent;
		} while (p);
	}

	if (apChanged) {
		if (emIsDLogEnabled()) {
			emDLog("emView %p: Active=\"%s\"",this,ap->GetIdentity().Get());
		}
		flags=emPanel::NF_ACTIVE_CHANGED;
		if (Focused) flags|=emPanel::NF_FOCUS_CHANGED;
		if (ActivePanel) {
			p=ActivePanel;
			p->Active=0;
			do {
				p->InActivePath=0;
				p->AddPendingNotice(flags);
				p=p->Parent;
			} while (p);
		}
		ActivePanel=p=ap;
		p->Active=1;
		do {
			p->InActivePath=1;
			p->AddPendingNotice(flags);
			p=p->Parent;
		} while (p);
	}

	if (adherentChanged) {
		VisitAdherent=adherent;
	}

	if (apChanged || adherentChanged) {
		InvalidateHighlight();
	}

	if (apChanged) {
		TitleInvalid=true;
		UpdateEngine->WakeUp();
		Signal(ControlPanelSignal);
	}
}


void emView::FindBestSVP(
	emPanel * * pPanel, double * pVx, double * pVy, double * pVw
)
{
	emPanel * vp, * p, * op;
	double vx,vy,vw,x,y,w,minS;
	bool b;
	int i;

	vp=*pPanel;
	vx=*pVx;
	vy=*pVy;
	vw=*pVw;
	for (i=0; i<2; i++) {
		minS = (i==0 ? MaxSVPSize : MaxSVPSearchSize);
		op=vp;
		for (;;) {
			p=vp->Parent;
			if (!p) break;
			w=vw/vp->LayoutWidth;
			if (w>minS || w*p->GetHeight()>minS) break;
			vx-=vp->LayoutX*w;
			vy-=vp->LayoutY*w/CurrentPixelTallness;
			vw=w;
			vp=p;
		}
		if (op==vp && i>0) break;
		b=
			vx<=CurrentX &&
			vx+vw>=CurrentX+CurrentWidth &&
			vy<=CurrentY &&
			vy+vp->GetHeight()*vw/CurrentPixelTallness>=CurrentY+CurrentHeight
		;
		p=vp; x=vx; y=vy; w=vw;
		b=FindBestSVPInTree(&p,&x,&y,&w,b);
		if (*pPanel!=p) {
			*pPanel=p;
			*pVx=x;
			*pVy=y;
			*pVw=w;
		}
		if (b) break;
	}
}


bool emView::FindBestSVPInTree(
	emPanel * * pPanel, double * pVx, double * pVy, double * pVw, bool covering
)
{
	double f,vx,vy,vw,vwc,vs,vd,x1,y1,x2,y2,x,y,cx,cy,cw,cs,d;
	emPanel * p, * cp;
	bool cc,vc,tooLarge,overlapped;

	p=*pPanel;
	vx=*pVx;
	vy=*pVy;
	vw=*pVw;

	vs=vw;
	f=p->GetHeight();
	if (f>1.0) vs*=f;

	tooLarge=(vs>MaxSVPSize);

	if (!covering && !tooLarge) return false;
	vc=(covering && (p->CanvasColor.IsOpaque() || p->IsOpaque()));

	p=p->LastChild;
	if (!p) return vc;

	x1=(CurrentX-vx)/vw;
	x2=x1+CurrentWidth/vw;
	vwc=vw/CurrentPixelTallness;
	y1=(CurrentY-vy)/vwc;
	y2=y1+CurrentHeight/vwc;
	vd=1E+30;
	overlapped=false;

	do {
		if (
			p->LayoutX<x2 && p->LayoutX+p->LayoutWidth>x1 &&
			p->LayoutY<y2 && p->LayoutY+p->LayoutHeight>y1
		) {
			cc=true;
			if (
				!covering ||
				p->LayoutX>x1 || p->LayoutX+p->LayoutWidth<x2 ||
				p->LayoutY>y1 || p->LayoutY+p->LayoutHeight<y2
			) {
				if (!tooLarge && vc) break;
				cc=false;
			}
			cp=p;
			cx=vx+p->LayoutX*vw;
			cy=vy+p->LayoutY*vwc;
			cw=p->LayoutWidth*vw;
			cc=FindBestSVPInTree(&cp,&cx,&cy,&cw,cc);
			if (!cc && !tooLarge && vc) break;
			cs=cw;
			f=cp->GetHeight();
			if (f>1.0) cs*=f;
			if (cc && cs<=MaxSVPSize) {
				if (tooLarge || !overlapped) {
					*pPanel=cp;
					*pVx=cx;
					*pVy=cy;
					*pVw=cw;
				}
				return true;
			}
			overlapped=true;
			if (tooLarge) {
				x=(x2+x1)*0.5;
				y=(y2+y1)*0.5;
				if (x<p->LayoutX) x-=p->LayoutX;
				else if (x>p->LayoutX+p->LayoutWidth) x-=p->LayoutX+p->LayoutWidth;
				else x=0.0;
				if (y<p->LayoutY) y-=p->LayoutY;
				else if (y>p->LayoutY+p->LayoutHeight) y-=p->LayoutY+p->LayoutHeight;
				else y=0.0;
				d=x*x+y*y;
				if (
					(cs<=MaxSVPSize && d-0.1<=vd) ||
					(vs>MaxSVPSize && cs<=vs)
				) {
					*pPanel=cp;
					*pVx=cx;
					*pVy=cy;
					*pVw=cw;
					vd=d;
					vs=cs;
					vc=cc;
				}
			}
		}
		p=p->Prev;
	} while (p);

	return vc;
}


void emView::SwapViewPorts(bool swapFocus)
{
	emView * w;
	emViewPort * vp;
	bool fcs;

	w=PopupWindow;
	vp=w->CurrentViewPort;
	w->CurrentViewPort=CurrentViewPort;
	CurrentViewPort=vp;
	CurrentViewPort->CurrentView=this;
	w->CurrentViewPort->CurrentView=w;
	CurrentX=CurrentViewPort->HomeView->HomeX;
	CurrentY=CurrentViewPort->HomeView->HomeY;
	CurrentWidth=CurrentViewPort->HomeView->HomeWidth;
	CurrentHeight=CurrentViewPort->HomeView->HomeHeight;
	CurrentPixelTallness=CurrentViewPort->HomeView->HomePixelTallness;
	w->CurrentX=w->CurrentViewPort->HomeView->HomeX;
	w->CurrentY=w->CurrentViewPort->HomeView->HomeY;
	w->CurrentWidth=w->CurrentViewPort->HomeView->HomeWidth;
	w->CurrentHeight=w->CurrentViewPort->HomeView->HomeHeight;
	w->CurrentPixelTallness=w->CurrentViewPort->HomeView->HomePixelTallness;
	if (swapFocus) {
		fcs=Focused;
		SetFocused(w->Focused);
		w->SetFocused(fcs);
	}
}


void emView::RecurseInput(
	emInputEvent & event, const emInputState & state
)
{
	emPanel * p;
	emInputEvent * e;
	double mx, my, tx, ty;

	p=SupremeViewedPanel;
	if (!p) return;

	NoEvent.Eat();

	e=&event;

	mx=state.GetMouseX();
	my=state.GetMouseY();
	if (e->IsMouseEvent()) {
		if (mx<p->ClipX1 || mx>=p->ClipX2 ||
		    my<p->ClipY1 || my>=p->ClipY2) e=&NoEvent;
	}
	mx=(mx-p->ViewedX)/p->ViewedWidth;
	my=(my-p->ViewedY)/p->ViewedWidth*CurrentPixelTallness;

	if (state.GetTouchCount()>0) {
		tx=state.GetTouchX(0);
		ty=state.GetTouchY(0);
	}
	else {
		tx=state.GetMouseX();
		ty=state.GetMouseY();
	}
	if (e->IsTouchEvent()) {
		if (tx<p->ClipX1 || tx>=p->ClipX2 ||
		    ty<p->ClipY1 || ty>=p->ClipY2) e=&NoEvent;
	}
	tx=(tx-p->ViewedX)/p->ViewedWidth;
	ty=(ty-p->ViewedY)/p->ViewedWidth*CurrentPixelTallness;

	if (p->PendingInput && p->LastChild) {
		RecurseChildrenInput(p,mx,my,tx,ty,*e,state);
		if (RestartInputRecursion) return;
	}

	for (;;) {
		if (p->PendingInput) {
			p->PendingInput=0;
			if (
				(
					e->IsMouseEvent() &&
					mx>=0.0 && mx<1.0 && my>=0.0 && my<p->GetHeight()
				) ||
				(
					e->IsTouchEvent() &&
					tx>=0.0 && tx<1.0 && ty>=0.0 && ty<p->GetHeight()
				) ||
				(
					p->InActivePath && e->IsKeyboardEvent()
				)
			) {
				p->Input(*e,state,mx,my);
			}
			else {
				p->Input(NoEvent,state,mx,my);
			}
			if (RestartInputRecursion) return;
		}
		if (!p->Parent) break;
		mx=mx*p->LayoutWidth+p->LayoutX;
		my=my*p->LayoutWidth+p->LayoutY;
		tx=tx*p->LayoutWidth+p->LayoutX;
		ty=ty*p->LayoutWidth+p->LayoutY;
		p=p->Parent;
	}
}


void emView::RecurseChildrenInput(
	emPanel * parent, double mx, double my, double tx, double ty,
	emInputEvent & event, const emInputState & state
)
{
	emPanel * p;
	emInputEvent * e;
	double cmx,cmy,ctx,cty;

	for (p=parent->LastChild; p; p=p->Prev) {
		if (!p->PendingInput || !p->InViewedPath) continue;
		cmx=(mx-p->LayoutX)/p->LayoutWidth;
		cmy=(my-p->LayoutY)/p->LayoutWidth;
		ctx=(tx-p->LayoutX)/p->LayoutWidth;
		cty=(ty-p->LayoutY)/p->LayoutWidth;
		if (
			(
				event.IsMouseEvent() &&
				cmx>=0.0 && cmx<1.0 && cmy>=0.0 && cmy<p->GetHeight()
			) ||
			(
				event.IsTouchEvent() &&
				ctx>=0.0 && ctx<1.0 && cty>=0.0 && cty<p->GetHeight()
			) ||
			(
				p->InActivePath && event.IsKeyboardEvent()
			)
		) {
			e=&event;
		}
		else {
			e=&NoEvent;
		}
		if (p->LastChild) {
			RecurseChildrenInput(p,cmx,cmy,ctx,cty,*e,state);
			if (RestartInputRecursion) return;
		}
		p->PendingInput=0;
		p->Input(*e,state,cmx,cmy);
		if (RestartInputRecursion) return;
	}
}


void emView::InvalidateHighlight()
{
	if (
		!ActivePanel || !ActivePanel->Viewed || (
			(VFlags&VF_NO_ACTIVE_HIGHLIGHT)!=0 &&
			((VFlags&VF_NO_FOCUS_HIGHLIGHT)!=0 || !Focused)
		)
	) return;
	InvalidatePainting(); //??? too much
}


void emView::PaintHighlight(const emPainter & painter)
{
	emColor shadowColor,arrowColor;
	double sxy[4*2],axy[4*2];
	double x1,y1,x2,y2,cx1,cy1,cx2,cy2,edx,edy,ex,ey,dx,dy,d,x,y,aw,ah,ag,sd;
	int edge,n,i1,i2;

	//??? These things could be configurable.
	static const emColor highlightColor=emColor(255,255,255);
	static const emColor adherentHighlightColor=emColor(255,255,187);
	static const double arrowSize=11.0;
	static const double arrowDistance=55.0;
	static const double distanceFromPanel=2.0;

	if (
		!ActivePanel || !ActivePanel->Viewed || (
			(VFlags&VF_NO_ACTIVE_HIGHLIGHT)!=0 &&
			((VFlags&VF_NO_FOCUS_HIGHLIGHT)!=0 || !Focused)
		)
	) return;

	x1=ActivePanel->GetViewedX();
	y1=ActivePanel->GetViewedY();
	x2=x1+ActivePanel->GetViewedWidth();
	y2=y1+ActivePanel->GetViewedHeight();
	x1-=distanceFromPanel;
	x2+=distanceFromPanel;
	y1-=distanceFromPanel/CurrentPixelTallness;
	y2+=distanceFromPanel/CurrentPixelTallness;

	cx1=painter.GetUserClipX1()-arrowSize*2.0;
	cy1=painter.GetUserClipY1()-arrowSize*2.0/CurrentPixelTallness;
	cx2=painter.GetUserClipX2()+arrowSize*2.0;
	cy2=painter.GetUserClipY2()+arrowSize*2.0/CurrentPixelTallness;

	if (x1>=cx2 || x2<=cx1 || y1>=cy2 || y2<=cy1) return;

	shadowColor=emColor(0,0,0,192);
	if (VisitAdherent) arrowColor=adherentHighlightColor;
	else arrowColor=highlightColor;
	if (!Focused || (VFlags&VF_NO_FOCUS_HIGHLIGHT)!=0) {
		shadowColor.SetAlpha((emByte)(shadowColor.GetAlpha()/3));
		arrowColor.SetAlpha((emByte)(arrowColor.GetAlpha()/3));
	}

	for (edge=0; edge<4; edge++) {
		if ((edge&1)==0) {
			d=(x2-x1)/arrowDistance;
			n=emMax(1,(int)(emMin(d,1E9)+0.5));
			edx=(x2-x1)/n;
			edy=0.0;
			if ((edge&2)==0) {
				ex=x1;
				ey=y1;
			}
			else {
				ex=x1+edx;
				ey=y2;
			}
			if (ey>=cy2 || ey<=cy1) continue;
			i1=emMax(0,(int)ceil((cx1-ex)/edx));
			i2=emMin(n-1,(int)floor((cx2-ex)/edx));
		}
		else {
			d=(y2-y1)*CurrentPixelTallness/arrowDistance;
			n=emMax(1,(int)(emMin(d,1E9)+0.5));
			edx=0.0;
			edy=(y2-y1)/n;
			if ((edge&2)==0) {
				ex=x2;
				ey=y1;
			}
			else {
				ex=x1;
				ey=y1+edy;
			}
			if (ex>=cx2 || ex<=cx1) continue;
			i1=emMax(0,(int)ceil((cy1-ey)/edy));
			i2=emMin(n-1,(int)floor((cy2-ey)/edy));
		}

		for (; i1<=i2; i1++) {
			x=ex+edx*i1;
			y=ey+edy*i1;

			dx=x-(x1+x2)*0.5;
			dy=(y-(y1+y2)*0.5)*CurrentPixelTallness;
			d=sqrt(dx*dx+dy*dy);
			if (d<0.01) {
				dx=0;
				dy=1.0;
			}
			else {
				dx/=d;
				dy/=d;
			}

			ah=arrowSize;
			ag=ah*0.8;
			aw=ah*0.5;
			sd=ah*0.2;

			axy[0]=x;
			axy[1]=y;
			axy[2]=x+dx*ah-dy*aw*0.5;
			axy[3]=y+(dy*ah+dx*aw*0.5)/CurrentPixelTallness;
			axy[4]=x+dx*ag;
			axy[5]=y+(dy*ag)/CurrentPixelTallness;
			axy[6]=x+dx*ah+dy*aw*0.5;
			axy[7]=y+(dy*ah-dx*aw*0.5)/CurrentPixelTallness;

			sxy[0]=axy[0];
			sxy[1]=axy[1];
			sxy[2]=axy[2]+sd;
			sxy[3]=axy[3]+sd/CurrentPixelTallness;
			sxy[4]=axy[4]+sd*ag/ah;
			sxy[5]=axy[5]+sd*ag/ah/CurrentPixelTallness;
			sxy[6]=axy[6]+sd;
			sxy[7]=axy[7]+sd/CurrentPixelTallness;

			painter.PaintPolygon(sxy,4,shadowColor);
			painter.PaintPolygon(axy,4,arrowColor);
		}
	}
}


void emView::SetSeekPos(emPanel * panel, const char * childName)
{
	if (!panel || !childName) childName="";
	if (SeekPosPanel!=panel) {
		if (SeekPosPanel) {
			SeekPosPanel->AddPendingNotice(
				emPanel::NF_SOUGHT_NAME_CHANGED|
				emPanel::NF_MEMORY_LIMIT_CHANGED
			);
		}
		SeekPosPanel=panel;
		SeekPosChildName=childName;
		if (SeekPosPanel) {
			SeekPosPanel->AddPendingNotice(
				emPanel::NF_SOUGHT_NAME_CHANGED|
				emPanel::NF_MEMORY_LIMIT_CHANGED
			);
		}
	}
	else if (panel && SeekPosChildName!=childName) {
		SeekPosChildName=childName;
		SeekPosPanel->AddPendingNotice(emPanel::NF_SOUGHT_NAME_CHANGED);
	}
}


bool emView::IsHopeForSeeking()
{
	return SeekPosPanel && SeekPosPanel->IsHopeForSeeking();
}


void emView::SetActivationCandidate(emPanel * panel)
{
	if (ActivationCandidate==panel) return;
	ActivationCandidate=panel;
	if (!ActivationEngine) ActivationEngine=new ActivationEngineClass(*this);
	ActivationEngine->WakeUp();
}


emView::UpdateEngineClass::UpdateEngineClass(emView & view)
	: emEngine(view.GetScheduler()), View(view)
{
	SetEnginePriority(emEngine::HIGH_PRIORITY);
}


bool emView::UpdateEngineClass::Cycle()
{
	View.Update();
	return false;
}


emView::ActivationEngineClass::ActivationEngineClass(emView & view)
	: emEngine(view.GetScheduler()), View(view)
{
	SetEnginePriority(emEngine::VERY_LOW_PRIORITY);
}


bool emView::ActivationEngineClass::Cycle()
{
	if (View.ActivationCandidate) {
		View.VisitLazy(View.ActivationCandidate,true);
	}
	return false;
}


emView::EOIEngineClass::EOIEngineClass(emView & view)
	: emEngine(view.GetScheduler()), View(view)
{
	CountDown=5;
	WakeUp();
}


bool emView::EOIEngineClass::Cycle()
{
	CountDown--;
	if (CountDown>0) return true;
	Signal(View.EOISignal);
	View.EOIEngine=NULL;
	delete this;
	return false;
}


emView::SeekEngineClass::SeekEngineClass(
	emView & view, int seekType, const emString & identity,
	double relX, double relY, double relA, bool adherent,
	const emString & subject
)
	: emEngine(view.GetScheduler()), View(view)
{
	SeekType=seekType;
	Identity=identity;
	RelX=relX;
	RelY=relY;
	RelA=relA;
	Adherent=adherent;
	Subject=subject;
	Names=emPanel::DecodeIdentity(Identity);
	TimeSlicesWithoutHope=0;
	GiveUp=false;
	GiveUpClock=0;
	WakeUp();
}


emView::SeekEngineClass::~SeekEngineClass()
{
}


void emView::SeekEngineClass::Paint(const emPainter & painter)
{
	double f,x,y,w,h,ws,tw,ch;
	emString str;
	int l1,l2;

	w=emMin(emMax(View.CurrentWidth,View.CurrentHeight)*0.6,View.CurrentWidth);
	h=w*0.25;
	f=View.CurrentHeight*0.8/h;
	if (f<1.0) { w*=f; h*=f; }
	x=View.CurrentX+(View.CurrentWidth-w)*0.5;
	y=emMax(View.CurrentY+View.CurrentHeight*0.5-h*1.25,View.CurrentY);

	painter.PaintRoundRect(
		x+w*0.03,y+w*0.03,
		w,h,
		h*0.2,h*0.2,
		emColor(0,0,0,160)
	);
	painter.PaintRoundRect(
		x,y,
		w,h,
		h*0.2,h*0.2,
		emColor(34,102,153,208)
	);
	painter.PaintRoundRectOutline(
		x+h*0.03,y+h*0.03,
		w-h*0.06,h-h*0.06,
		h*0.2-h*0.06*0.5,h*0.2-h*0.06*0.5,
		h*0.02,
		emColor(221,221,221)
	);

	x+=h*0.2;
	y+=h*0.1;
	w-=h*0.4;
	h-=h*0.2;

	if (GiveUp) {
		painter.PaintTextBoxed(
			x,y,w,h,
			"Not found",
			h*0.6,
			emColor(255,136,136),
			0,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			0.8
		);
		return;
	}

	str="Seeking";
	if (!Subject.IsEmpty()) {
		str+=" for ";
		str+=Subject;
	}
	painter.PaintTextBoxed(
		x,y,w,h*0.4,
		str,
		h,
		emColor(221,221,221),
		0,
		EM_ALIGN_CENTER,
		EM_ALIGN_LEFT,
		0.8
	);

	painter.PaintTextBoxed(
		x,y+h*0.8,w,h*0.2,
		"Press any keyboard key or mouse button to abort.",
		h,
		emColor(221,221,221),
		0,
		EM_ALIGN_CENTER,
		EM_ALIGN_LEFT,
		0.8
	);

	y+=h*0.5;
	h*=0.2;
	if (View.SeekPosPanel) str=View.SeekPosPanel->GetIdentity();
	else str="";
	l1=strlen(str);
	l2=strlen(Identity);
	if (l1>l2) l1=l2;
	tw=painter.GetTextSize(Identity,h,false);
	ws=1.0;
	if (tw>w) { ws=w/tw; tw=w; }
	ch=h;
	if (ws<0.5) { ch*=ws/0.5; ws=0.5; }
	painter.PaintRect(
		x+(w-tw)*0.5,y,tw*l1/l2,h,
		emColor(136,255,136,80)
	);
	painter.PaintRect(
		x+(w-tw)*0.5+tw*l1/l2,y,tw*(l2-l1)/l2,h,
		emColor(136,136,136,80)
	);
	painter.PaintText(
		x+(w-tw)*0.5,y+(h-ch)*0.5,
		Identity,ch,ws,emColor(136,255,136),0,l1
	);
	painter.PaintText(
		x+(w-tw)*0.5+tw*l1/l2,y+(h-ch)*0.5,
		Identity.Get()+l1,ch,ws,emColor(136,136,136),0,l2-l1
	);
}


bool emView::SeekEngineClass::Cycle()
{
	emPanel * p, * c;
	int i;

	if (GiveUp) {
		if (emGetClockMS()<GiveUpClock+1500) return true;
		View.AbortSeeking(); // deletes this
		return false;
	}
	p=View.RootPanel;
	if (!p || Names.GetCount()<1 || Names[0]!=p->GetName()) {
		GiveUp=true;
		GiveUpClock=emGetClockMS();
		View.InvalidatePainting();
		return true;
	}
	for (i=1; i<Names.GetCount(); i++) {
		c=p->GetChild(Names[i]);
		if (!c) break;
		p=c;
	}
	if (i>=Names.GetCount()) {
		View.ProtectSeeking++;
		switch (SeekType) {
		case 1:
			View.Visit(p,Adherent);
			break;
		case 2:
			View.Visit(p,RelX,RelY,RelA,Adherent);
			break;
		case 3:
			View.VisitBy(p,RelX,RelY,RelA);
			break;
		case 4:
			View.VisitLazy(p,Adherent);
			break;
		case 5:
			View.VisitFullsized(p,Adherent);
			break;
		case 6:
			View.VisitByFullsized(p);
			break;
		}
		View.ProtectSeeking--;
		View.AbortSeeking(); // deletes this
		return false;
	}
	else if (View.SeekPosPanel!=p) {
		View.ProtectSeeking++;
		View.SetSeekPos(p,Names[i]);
		View.VisitFullsized(p,false);
		View.InvalidatePainting();
		View.ProtectSeeking--;
		TimeSlicesWithoutHope=4;
	}
	else if (View.IsHopeForSeeking()) {
		TimeSlicesWithoutHope=0;
	}
	else {
		TimeSlicesWithoutHope++;
		if (TimeSlicesWithoutHope>10) {
			GiveUp=true;
			GiveUpClock=emGetClockMS();
			View.InvalidatePainting();
		}
	}
	return true;
}


emView::StressTestClass::StressTestClass(emView & view)
	: emEngine(view.GetScheduler()), View(view)
{
	TCnt=128;
	TPos=0;
	TValid=0;
	T=new emUInt64[TCnt];
	FrameRate=0.0;
	FRUpdate=0;
	WakeUp();
}


emView::StressTestClass::~StressTestClass()
{
	delete [] T;
}


void emView::StressTestClass::PaintInfo(const emPainter & painter)
{
	char tmp[256];
	double x,y,w,h,ch;

	sprintf(tmp,"Stress Test\n%5.1f Hz",FrameRate);
	x=View.CurrentX;
	y=View.CurrentY;
	ch=View.CurrentHeight/45;
	if (ch<10.0) ch=10.0;
	w=painter.GetTextSize(tmp,ch,true,0.0,&h);
	painter.PaintRect(x,y,w,h,emColor(255,0,255,128));
	painter.PaintTextBoxed(
		x,y,w,h,
		tmp,
		ch,
		emColor(255,255,0,192),
		0,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);
}


bool emView::StressTestClass::Cycle()
{
	emUInt64 clk,dt;
	int i;

	clk=emGetClockMS();
	TPos=(TPos+1)%TCnt;
	T[TPos]=clk;
	if (TValid<TCnt) TValid++;
	if (clk-FRUpdate>100) {
		FrameRate=0.0;
		FRUpdate=clk;
		for (i=1; i<TValid; i++) {
			dt=clk-T[(TPos-i+TCnt)%TCnt];
			if (dt>1000 && i>0) break;
			FrameRate=(i)*1000.0/dt;
		}
	}

	View.InvalidatePainting();

	return true;
}


const double emView::MaxSVPSize=1E+12;
const double emView::MaxSVPSearchSize=1E+14;


//==============================================================================
//================================= emViewPort =================================
//==============================================================================

emViewPort::emViewPort(emView & homeView)
{
	HomeView=&homeView;
	CurrentView=&homeView;
	if (HomeView->DummyViewPort!=HomeView->HomeViewPort) {
		emFatalError("emViewPort: The view has already a view port.");
	}
	HomeView->HomeViewPort=this;
	HomeView->CurrentViewPort=this;
}


emViewPort::~emViewPort()
{
	if (HomeView) {
		if (HomeView->DummyViewPort==this) {
			emFatalError(
				"emViewPort::~emViewPort: Illegal destruction of dummy view port."
			);
		}
		if (HomeView!=CurrentView) {
			if (HomeView->PopupWindow) {
				HomeView->ZoomOut();
			}
			else {
				emFatalError(
					"emViewPort::~emViewPort: Illegal destruction of popup view port."
				);
			}
		}
		HomeView->HomeViewPort=HomeView->DummyViewPort;
		HomeView->CurrentViewPort=HomeView->DummyViewPort;
		HomeView=NULL;
		CurrentView=NULL;
	}
}


void emViewPort::RequestFocus()
{
	SetViewFocused(true);
}


bool emViewPort::IsSoftKeyboardShown()
{
	return false;
}


void emViewPort::ShowSoftKeyboard(bool show)
{
}


emUInt64 emViewPort::GetInputClockMS()
{
	return emGetClockMS();
}


void emViewPort::InputToView(
	emInputEvent & event, const emInputState & state
)
{
	if (!CurrentView->FirstVIF) CurrentView->Input(event,state);
	else CurrentView->FirstVIF->Input(event,state);
}


void emViewPort::InvalidateCursor()
{
}


void emViewPort::InvalidatePainting(double x, double y, double w, double h)
{
}


emViewPort::emViewPort()
{
	HomeView=NULL;
	CurrentView=NULL;
}
