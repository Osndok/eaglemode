//------------------------------------------------------------------------------
// emView.cpp
//
// Copyright (C) 2004-2011,2014,2016 Oliver Hamann.
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
	emContext * c;
	emWindow * win;

	CoreConfig=emCoreConfig::Acquire(GetRootContext());
	DummyViewPort=new emViewPort();
	DummyViewPort->CurrentView=this;
	DummyViewPort->HomeView=this;
	HomeViewPort=DummyViewPort;
	CurrentViewPort=DummyViewPort;
	Window=NULL;
	ScreenRef=NULL;
	PopupWindow=NULL;
	FirstVIF=NULL;
	LastVIF=NULL;
	ActiveAnimator=NULL;
	MagneticVA=NULL;
	VisitingVA=NULL;
	RootPanel=NULL;
	SupremeViewedPanel=NULL;
	MinSVP=NULL;
	MaxSVP=NULL;
	ActivePanel=NULL;
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
	ActivationAdherent=false;
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
	EOIEngine=NULL;
	SeekPosPanel=NULL;
	StressTest=NULL;

	UpdateEngine->WakeUp();

	SetViewFlags(viewFlags);

	MagneticVA=new emMagneticViewAnimator(*this);
	VisitingVA=new emVisitingViewAnimator(*this);

	new emDefaultTouchVIF(*this);
	new emCheatVIF(*this);
	new emKeyboardZoomScrollVIF(*this);
	new emMouseZoomScrollVIF(*this);

	for (win=NULL, c=GetParentContext(); c!=NULL; c=c->GetParentContext()) {
		win=dynamic_cast<emWindow*>(c);
		if (win) break;
	}
	SetWindowAndScreen(win); // emWindow constructor also calls that...
}


emView::~emView()
{
	AbortActiveAnimator();
	CrossPtrList.BreakCrossPtrs();
	//??? Should we delete child views here like emWindow deletes its child
	//??? windows? If so, remember to adapt emSubViewPanel. (No panic,
	//??? child views are deleted at least by the context destructor)
	if (RootPanel) delete RootPanel;
	if (StressTest) delete StressTest;
	while (LastVIF) delete LastVIF;
	if (EOIEngine) delete EOIEngine;
	delete UpdateEngine;
	if (VisitingVA) { delete VisitingVA; VisitingVA=NULL; }
	if (MagneticVA) { delete MagneticVA; MagneticVA=NULL; }
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
			RawZoomOut();
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
			RawZoomOut();
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


emString emView::GetTitle() const
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


emScreen * emView::GetScreen() const
{
	return (emScreen*)ScreenRef.Get();
}


void emView::GetMaxPopupViewRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	double x,y,w,h,mx,my,mw,mh,cx,cy;
	const emScreen * screen;
	bool found;
	int i,n;

	x=CurrentX;
	y=CurrentY;
	w=CurrentWidth;
	h=CurrentHeight;
	screen=GetScreen();
	if (screen) {
		n=screen->GetMonitorCount();
		found=false;
		for (i=n-1; i>=0; i--) {
			screen->GetMonitorRect(i,&mx,&my,&mw,&mh);
			if (
				(!found && i==0) || (
					mx<HomeX+HomeWidth  && mx+mw>HomeX &&
					my<HomeY+HomeHeight && my+mh>HomeY
				)
			) {
				if (!found) {
					x=mx;
					y=my;
					w=mw;
					h=mh;
					found=true;
				}
				else {
					if (x>mx) { w+=x-mx; x=mx; }
					if (w<mx+mw-x) w=mx+mw-x;
					if (y>my) { h+=y-my; y=my; }
					if (h<my+mh-y) h=my+mh-y;
				}
			}
		}
		if (found) {
			// This is just for that the users sees more than
			// nothing even when the monitor rectangles are
			// completely wrong.
			cx=HomeX+HomeWidth*0.5;
			cy=HomeY+HomeHeight*0.5;
			if (x>cx) { w+=x-cx; x=cx; }
			if (w<cx-x) w=cx-x;
			if (y>cy) { h+=y-cy; y=cy; }
			if (h<cy-y) h=cy-y;
		}
	}
	if (pX) *pX=x;
	if (pY) *pY=y;
	if (pW) *pW=w;
	if (pH) *pH=h;
}


void emView::SetActivePanel(emPanel * panel, bool adherent)
{
	emPanel::NoticeFlags flags;
	emPanel * p;

	if (!panel) return;

	while (!panel->Focusable) panel=panel->Parent;

	if (ActivePanel!=panel) {
		if (emIsDLogEnabled()) {
			emDLog("emView %p: Active=\"%s\"",this,panel->GetIdentity().Get());
		}
		if (ActivePanel) InvalidateHighlight();
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
		p=panel;
		p->Active=1;
		do {
			p->InActivePath=1;
			p->AddPendingNotice(flags);
			p=p->Parent;
		} while (p);
		ActivePanel=panel;
		ActivationAdherent=adherent;
		InvalidateHighlight();
		TitleInvalid=true;
		UpdateEngine->WakeUp();
		Signal(ControlPanelSignal);
	}
	else if (ActivationAdherent!=adherent) {
		ActivationAdherent=adherent;
		InvalidateHighlight();
	}
}


void emView::SetActivePanelBestPossible()
{
	emPanel * best, * p;
	double cx,cy,cw,ch,ex,ey,ew,eh,minW,minH,minA;
	bool adherent;

	cx=CurrentX;
	cy=CurrentY;
	cw=CurrentWidth;
	ch=CurrentHeight;
	if (PopupWindow) {
		GetMaxPopupViewRect(&ex,&ey,&ew,&eh);
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
	if (!best) {
		return;
	}
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
		ActivationAdherent &&
		ActivePanel &&
		ActivePanel->Viewed &&
		ActivePanel->ViewedWidth>=4 &&
		ActivePanel->ViewedHeight>=4 &&
		best->InActivePath
	) {
		best=ActivePanel;
		adherent=true;
	}

	SetActivePanel(best,adherent);
}


emPanel * emView::GetPanelByIdentity(const char * identity) const
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


emPanel * emView::GetPanelAt(double x, double y) const
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


emPanel * emView::GetFocusablePanelAt(double x, double y, bool checkSubstance) const
{
	emPanel * p, * c;

	p=SupremeViewedPanel;
	if (
		p && p->ClipX1<=x && p->ClipX2>x && p->ClipY1<=y && p->ClipY2>y && (
			!checkSubstance ||
			p->IsPointInSubstanceRect(p->ViewToPanelX(x),p->ViewToPanelY(y))
		)
	) {
		c=p->GetFocusableLastChild();
		while (c) {
			if (
				c->Viewed &&
				c->ClipX1<=x && c->ClipX2>x && c->ClipY1<=y && c->ClipY2>y && (
					!checkSubstance ||
					c->IsPointInSubstanceRect(c->ViewToPanelX(x),c->ViewToPanelY(y))
				)
			) {
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
	double * pRelX, double * pRelY, double * pRelA
) const
{
	emPanel * p;

	p=ActivePanel;
	while (p && !p->InViewedPath) p=p->Parent;
	if (!p || !p->Viewed) p=SupremeViewedPanel;

	if (p) {
		if (pRelX) *pRelX=(HomeX+HomeWidth*0.5-p->ViewedX)/p->ViewedWidth-0.5;
		if (pRelY) *pRelY=(HomeY+HomeHeight*0.5-p->ViewedY)/p->ViewedHeight-0.5;
		if (pRelA) *pRelA=(HomeWidth*HomeHeight)/(p->ViewedWidth*p->ViewedHeight);
	}
	else {
		if (pRelX) *pRelX=0.0;
		if (pRelY) *pRelY=0.0;
		if (pRelA) *pRelA=0.0;
	}
	return p;
}


void emView::Visit(
	emPanel * panel, double relX, double relY, double relA, bool adherent
)
{
	Visit(panel->GetIdentity(), relX, relY, relA, adherent, panel->GetTitle());
}


void emView::Visit(
	const char * identity, double relX, double relY, double relA,
	bool adherent, const char * subject
)
{
	VisitingVA->SetAnimParamsByCoreConfig(*CoreConfig);
	VisitingVA->SetGoal(identity, relX, relY, relA, adherent, subject);
	VisitingVA->Activate();
}


void emView::Visit(emPanel * panel, bool adherent)
{
	Visit(panel->GetIdentity(), adherent, panel->GetTitle());
}


void emView::Visit(const char * identity, bool adherent, const char * subject)
{
	VisitingVA->SetAnimParamsByCoreConfig(*CoreConfig);
	VisitingVA->SetGoal(identity, adherent, subject);
	VisitingVA->Activate();
}


void emView::VisitFullsized(emPanel * panel, bool adherent, bool utilizeView)
{
	VisitFullsized(panel->GetIdentity(), adherent, utilizeView, panel->GetTitle());
}


void emView::VisitFullsized(
	const char * identity, bool adherent, bool utilizeView,
	const char * subject
)
{
	VisitingVA->SetAnimParamsByCoreConfig(*CoreConfig);
	VisitingVA->SetGoalFullsized(identity, adherent, utilizeView, subject);
	VisitingVA->Activate();
}


void emView::RawVisit(emPanel * panel, double relX, double relY, double relA)
{
	RawVisit(panel,relX,relY,relA,false);
}


void emView::RawVisit(emPanel * panel)
{
	double relX,relY,relA;

	if (!panel) return;
	CalcVisitCoords(panel,&relX,&relY,&relA);
	RawVisit(panel,relX,relY,relA);
}


void emView::RawVisitFullsized(emPanel * panel, bool utilizeView)
{
	RawVisit(panel,0.0,0.0,utilizeView?-1.0:0.0);
}


void emView::VisitNext()
{
	emPanel * p;

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

	if (!ActivePanel) return;
	p=ActivePanel->GetFocusableFirstChild();
	if (p) Visit(p,true);
	else VisitFullsized(ActivePanel,true);
}


void emView::VisitOut()
{
	double relA,relA2;
	emPanel * p;

	if (!ActivePanel) return;
	p=ActivePanel->GetFocusableParent();
	if (p) Visit(p,true);
	else if (RootPanel) {
		relA=HomeWidth*RootPanel->GetHeight()/HomePixelTallness/HomeHeight;
		relA2=HomeHeight/RootPanel->GetHeight()*HomePixelTallness/HomeWidth;
		if (relA<relA2) relA=relA2;
		Visit(RootPanel,0.0,0.0,relA,true);
	}
}


void emView::Scroll(double deltaX, double deltaY)
{
	double rx,ry,ra;
	emPanel * p;

	AbortActiveAnimator();
	if (deltaX!=0.0 || deltaY!=0.0) {
		p=GetVisitedPanel(&rx,&ry,&ra);
		if (p) {
			rx+=deltaX/p->ViewedWidth;
			ry+=deltaY/p->ViewedHeight;
			RawVisit(p,rx,ry,ra,true);
		}
	}
	SetActivePanelBestPossible();
}


void emView::Zoom(double fixX, double fixY, double factor)
{
	double rx,ry,ra,reFac;
	emPanel * p;

	AbortActiveAnimator();
	if (factor!=1.0 && factor>0.0) {
		p=GetVisitedPanel(&rx,&ry,&ra);
		if (p) {
			reFac=1.0/factor;
			rx+=(fixX-(HomeX+HomeWidth*0.5))*(1.0-reFac)/p->ViewedWidth;
			ry+=(fixY-(HomeY+HomeHeight*0.5))*(1.0-reFac)/p->ViewedHeight;
			ra*=reFac*reFac;
			RawVisit(p,rx,ry,ra,true);
		}
	}
	SetActivePanelBestPossible();
}


void emView::RawScrollAndZoom(
	double fixX, double fixY,
	double deltaX, double deltaY, double deltaZ,
	emPanel * panel, double * pDeltaXDone,
	double * pDeltaYDone, double * pDeltaZDone
)
{
	double zflpp,hx,hy,hw,hh,hmx,hmy,pvx,pvy,pvw,pvh;
	double rx,ry,ra,rx2,ry2,ra2,reFac;

	zflpp=GetZoomFactorLogarithmPerPixel();

	hx=GetHomeX();
	hy=GetHomeY();
	hw=GetHomeWidth();
	hh=GetHomeHeight();
	hmx=hx+hw*0.5;
	hmy=hy+hh*0.5;

	if (panel && panel->IsViewed()) {
		pvx=panel->GetViewedX();
		pvy=panel->GetViewedY();
		pvw=panel->GetViewedWidth();
		pvh=panel->GetViewedHeight();
		rx = (hmx-pvx) / pvw - 0.5;
		ry = (hmy-pvy) / pvh - 0.5;
		ra = (hw*hh) / (pvw*pvh);
	}
	else {
		panel = GetVisitedPanel(&rx,&ry,&ra);
		if (!panel) {
			if (pDeltaXDone) *pDeltaXDone=0.0;
			if (pDeltaYDone) *pDeltaYDone=0.0;
			if (pDeltaZDone) *pDeltaZDone=0.0;
			return;
		}
		pvw=panel->GetViewedWidth();
		pvh=panel->GetViewedHeight();
	}

	reFac=exp(-deltaZ*zflpp);
	rx2 = rx + ((fixX-hmx)*(1.0-reFac) + deltaX)/pvw;
	ry2 = ry + ((fixY-hmy)*(1.0-reFac) + deltaY)/pvh;
	ra2 = ra * reFac*reFac;

	RawVisit(panel,rx2,ry2,ra2);

	if (panel->IsViewed()) {
		pvx=panel->GetViewedX();
		pvy=panel->GetViewedY();
		pvw=panel->GetViewedWidth();
		pvh=panel->GetViewedHeight();

		rx2 = (hmx-pvx) / pvw - 0.5;
		ry2 = (hmy-pvy) / pvh - 0.5;
		ra2 = (hw*hh) / (pvw*pvh);
		reFac = sqrt(ra2/ra);

		if (pDeltaXDone) {
			*pDeltaXDone = (rx2-rx)*pvw*reFac - (fixX-hmx)*(1.0-reFac);
		}
		if (pDeltaYDone) {
			*pDeltaYDone = (ry2-ry)*pvh*reFac - (fixY-hmy)*(1.0-reFac);
		}
		if (pDeltaZDone) {
			*pDeltaZDone = -log(reFac)/zflpp;
		}
	}
	else {
		if (pDeltaXDone) *pDeltaXDone = deltaX;
		if (pDeltaYDone) *pDeltaYDone = deltaY;
		if (pDeltaZDone) *pDeltaZDone = deltaZ;
	}
}


double emView::GetZoomFactorLogarithmPerPixel() const
{
	double w,h,r;

	if ((GetViewFlags()&emView::VF_POPUP_ZOOM)!=0) {
		GetMaxPopupViewRect(NULL,NULL,&w,&h);
	}
	else {
		w=GetCurrentWidth();
		h=GetCurrentHeight();
	}
	r=(w+h)*0.25;
	if (r<1.0) r=1.0;
	return 1.33/r;
}


void emView::ZoomOut()
{
	AbortActiveAnimator();
	RawZoomOut();
	SetActivePanelBestPossible();
}


void emView::RawZoomOut()
{
	RawZoomOut(false);
}


bool emView::IsZoomedOut() const
{
	double x,y,w,h;
	const emPanel * p;

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
) const
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


void emView::ActivateMagneticViewAnimator()
{
	if (MagneticVA) MagneticVA->Activate();
}


void emView::AbortActiveAnimator()
{
	if (ActiveAnimator) ActiveAnimator->Deactivate();
}


void emView::Input(emInputEvent & event, const emInputState & state)
{
	emPanel * p;

	if (ActiveAnimator) ActiveAnimator->Input(event,state);

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
			p->PendingInput=true;
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


emCursor emView::GetCursor() const
{
	return Cursor;
}


void emView::Paint(const emPainter & painter, emColor canvasColor) const
{
	emColor ncc;
	emPainter pnt;
	const emPanel * p;
	double rx1,ry1,rx2,ry2,ox,oy,cx1,cy1,cx2,cy2;
	bool wasNotInUserSpace;

	if (painter.GetScaleX()!=1.0 || painter.GetScaleY()!=1.0) {
		emFatalError("emView::Paint: Scaling not possible.");
	}

	wasNotInUserSpace=painter.EnterUserSpace();

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
			painter.LeaveUserSpace();
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
								painter.EnterUserSpace();
								p->Paint(pnt,p->CanvasColor);
								painter.LeaveUserSpace();
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
			painter.EnterUserSpace();
		}
		PaintHighlight(painter);
	}

	if (ActiveAnimator) ActiveAnimator->Paint(painter);
	if (StressTest) StressTest->PaintInfo(painter);

	if (wasNotInUserSpace) painter.LeaveUserSpace();
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


emString emView::GetTitle()
{
	return ((const emView*)this)->GetTitle();
}


double emView::GetTouchEventPriority(
	double touchX, double touchY, bool afterVIFs
)
{
	return ((const emView*)this)->GetTouchEventPriority(touchX,touchY,afterVIFs);
}


emCursor emView::GetCursor()
{
	return ((const emView*)this)->GetCursor();
}


void emView::Paint(const emPainter & painter, emColor canvasColor)
{
	((const emView*)this)->Paint(painter,canvasColor);
}


void emView::SetWindowAndScreen(emWindow * window)
{
	// Called by the constructors of emView and emWindow.
	Window=window;
	if (window) ScreenRef=&window->GetScreen();
	else ScreenRef=emScreen::LookupInherited(*this);
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
	double rx,ry,ra;
	emPanel * p;

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
	p=GetVisitedPanel(&rx,&ry,&ra);
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
		RawZoomOut(true);
	}
	else if (p) {
		RawVisit(p,rx,ry,ra,true);
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
					if (
						p->CanvasColor.IsOpaque() ||
						((const emPanel*)p)->IsOpaque()
					) break;
				}
				if (SupremeViewedPanel!=p) {
					emDLog("emView %p: SVP choice invalid by opacity.",this);
					SVPChoiceInvalid=true;
				}
			}
		}
		else if (SVPChoiceInvalid) {
			SVPChoiceInvalid=false;
			p=GetVisitedPanel();
			if (p) {
				RawVisitAbs(
					p,
					p->ViewedX,
					p->ViewedY,
					p->ViewedWidth,
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


void emView::CalcVisitCoords(
	const emPanel * panel, double * pRelX, double * pRelY, double * pRelA
) const
{
	static const double MIN_REL_DISTANCE=0.03;
	static const double MIN_REL_CIRCUMFERENCE=0.05;
	const emPanel * p, * cp;
	double ph,dx,dy,sx,sy,sw,sh,minvw,maxvw,vx,vy,vw,vh;
	double ctx,cty,ctw,cth,csx,csy,csw,csh;

	ph=panel->GetHeight();

	if ((VFlags&VF_POPUP_ZOOM)!=0) {
		GetMaxPopupViewRect(&sx,&sy,&sw,&sh);
	}
	else {
		sx=CurrentX;
		sy=CurrentY;
		sw=CurrentWidth;
		sh=CurrentHeight;
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
		if (pRelX) *pRelX=(HomeX+HomeWidth*0.5-panel->ViewedX)/panel->ViewedWidth-0.5;
		if (pRelY) *pRelY=(HomeY+HomeHeight*0.5-panel->ViewedY)/panel->ViewedHeight-0.5;
		if (pRelA) *pRelA=(HomeWidth*HomeHeight)/(panel->ViewedWidth*panel->ViewedHeight);
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

	if (pRelX) *pRelX=(HomeX+HomeWidth*0.5-vx)/vw-0.5;
	if (pRelY) *pRelY=(HomeY+HomeHeight*0.5-vy)/vh-0.5;
	if (pRelA) *pRelA=(HomeWidth*HomeHeight)/(vw*vh);
}


void emView::CalcVisitFullsizedCoords(
	const emPanel * panel, double * pRelX, double * pRelY, double * pRelA,
	bool utilizeView
) const
{
	double fx,fy,fw,fh,ph,vx,vy,vw,vh,ex,ey,ew,eh;

	if ((VFlags&VF_POPUP_ZOOM)!=0) {
		GetMaxPopupViewRect(&fx,&fy,&fw,&fh);
	}
	else {
		fx=HomeX;
		fy=HomeY;
		fw=HomeWidth;
		fh=HomeHeight;
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


void emView::RawVisit(
	emPanel * panel, double relX, double relY, double relA,
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
	RawVisitAbs(panel,vx,vy,vw,forceViewingUpdate);
}


void emView::RawVisitAbs(
	emPanel * panel, double vx, double vy, double vw,
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

	vh=vp->GetHeight()*vw/HomePixelTallness;

	if (vp==RootPanel) {
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
	}

	if ((VFlags&VF_POPUP_ZOOM)!=0) {
		if (
			vp!=RootPanel ||
			vx<HomeX-0.1 || vx+vw>HomeX+HomeWidth+0.1 ||
			vy<HomeY-0.1 || vy+vh>HomeY+HomeHeight+0.1
		) {
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
			GetMaxPopupViewRect(&sx,&sy,&sw,&sh);
			if (vp==RootPanel) {
				x1=floor(vx);
				y1=floor(vy);
				x2=ceil(vx+vw);
				y2=ceil(vy+vh);
				if (x1<sx) x1=sx;
				if (y1<sy) y1=sy;
				if (x2>sx+sw) x2=sx+sw;
				if (y2>sy+sh) y2=sy+sh;
				if (x2<x1+1.0) x2=x1+1.0;
				if (y2<y1+1.0) y2=y1+1.0;
			}
			else {
				x1=sx;
				y1=sy;
				x2=sx+sw;
				y2=sy+sh;
			}
			if (
				fabs(x1-CurrentX)>0.01 || fabs(x2-CurrentX-CurrentWidth)>0.01 ||
				fabs(y1-CurrentY)>0.01 || fabs(y2-CurrentY-CurrentHeight)>0.01
			) {
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
		x1=(CurrentX+1E-4-sx)/sw;
		x2=(CurrentX+CurrentWidth-1E-4-sx)/sw;
		y1=(CurrentY+1E-4-sy)*(CurrentPixelTallness/sw);
		y2=(CurrentY+CurrentHeight-1E-4-sy)*(CurrentPixelTallness/sw);
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
}


void emView::RawZoomOut(bool forceViewingUpdate)
{
	double relA,relA2;

	if (RootPanel) {
		relA=HomeWidth*RootPanel->GetHeight()/HomePixelTallness/HomeHeight;
		relA2=HomeHeight/RootPanel->GetHeight()*HomePixelTallness/HomeWidth;
		if (relA<relA2) relA=relA2;
		RawVisit(RootPanel,0.0,0.0,relA,forceViewingUpdate);
	}

	if (IsPoppedUp()) {
		emFatalError("emView::RawZoomOut: Inconsistent algorithms.");
	}
}


void emView::FindBestSVP(
	emPanel * * pPanel, double * pVx, double * pVy, double * pVw
) const
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
			vx<=CurrentX+1E-4 &&
			vx+vw>=CurrentX+CurrentWidth-1E-4 &&
			vy<=CurrentY+1E-4 &&
			vy+vp->GetHeight()*vw/CurrentPixelTallness>=CurrentY+CurrentHeight-1E-4
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
) const
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
	vc=(
		covering &&
		(p->CanvasColor.IsOpaque() || ((const emPanel*)p)->IsOpaque())
	);

	p=p->LastChild;
	if (!p) return vc;

	x1=(CurrentX+1E-4-vx)/vw;
	x2=(CurrentX+CurrentWidth-1E-4-vx)/vw;
	vwc=vw/CurrentPixelTallness;
	y1=(CurrentY+1E-4-vy)/vwc;
	y2=(CurrentY+CurrentHeight-1E-4-vy)/vwc;
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


void emView::RecurseInput(emInputEvent & event, const emInputState & state)
{
	double mx,my,tx,ty;
	emInputEvent * ebase, * e;
	emPanel * panel, * child;

	panel=SupremeViewedPanel;
	if (!panel) return;

	NoEvent.Eat();

	ebase=&event;

	mx=state.GetMouseX();
	my=state.GetMouseY();
	if (ebase->IsMouseEvent()) {
		if (mx<panel->ClipX1 || mx>=panel->ClipX2 ||
		    my<panel->ClipY1 || my>=panel->ClipY2) ebase=&NoEvent;
	}
	mx=(mx-panel->ViewedX)/panel->ViewedWidth;
	my=(my-panel->ViewedY)/panel->ViewedWidth*CurrentPixelTallness;

	if (state.GetTouchCount()>0) {
		tx=state.GetTouchX(0);
		ty=state.GetTouchY(0);
	}
	else {
		tx=state.GetMouseX();
		ty=state.GetMouseY();
	}
	if (ebase->IsTouchEvent()) {
		if (tx<panel->ClipX1 || tx>=panel->ClipX2 ||
		    ty<panel->ClipY1 || ty>=panel->ClipY2) ebase=&NoEvent;
	}
	tx=(tx-panel->ViewedX)/panel->ViewedWidth;
	ty=(ty-panel->ViewedY)/panel->ViewedWidth*CurrentPixelTallness;

	for (;;) {
		if (panel->PendingInput) {
			e=ebase;
			if (!e->IsEmpty()) {
				if (e->IsMouseEvent()) {
					if (!panel->IsPointInSubstanceRect(mx,my)) {
						e=&NoEvent;
					}
				}
				else if (e->IsTouchEvent()) {
					if (!panel->IsPointInSubstanceRect(tx,ty)) {
						e=&NoEvent;
					}
				}
				else if (e->IsKeyboardEvent()) {
					if (!panel->InActivePath) {
						e=&NoEvent;
					}
				}
			}
			for (child=panel->LastChild; child; child=child->Prev) {
				RecurseInput(child,*e,state);
				if (RestartInputRecursion) return;
			}
			panel->PendingInput=0;
			panel->Input(*e,state,mx,my);
			if (RestartInputRecursion) return;
		}
		if (!panel->Parent) break;
		mx=mx*panel->LayoutWidth+panel->LayoutX;
		my=my*panel->LayoutWidth+panel->LayoutY;
		tx=tx*panel->LayoutWidth+panel->LayoutX;
		ty=ty*panel->LayoutWidth+panel->LayoutY;
		panel=panel->Parent;
	}
}


void emView::RecurseInput(
	emPanel * panel, emInputEvent & event, const emInputState & state
)
{
	double mx,my,tx,ty;
	emInputEvent * e;
	emPanel * child;

	if (!panel->PendingInput) return;

	if (panel->Viewed) {
		mx=(state.GetMouseX()-panel->ViewedX)/panel->ViewedWidth;
		my=(state.GetMouseY()-panel->ViewedY)/panel->ViewedWidth*CurrentPixelTallness;
		if (state.GetTouchCount()>0) {
			tx=(state.GetTouchX(0)-panel->ViewedX)/panel->ViewedWidth;
			ty=(state.GetTouchY(0)-panel->ViewedY)/panel->ViewedWidth*CurrentPixelTallness;
		}
		else {
			tx=mx;
			ty=my;
		}
	}
	else {
		mx=-1.0;
		my=-1.0;
		tx=-1.0;
		ty=-1.0;
	}

	e=&event;
	if (!e->IsEmpty()) {
		if (e->IsMouseEvent()) {
			if (!panel->IsPointInSubstanceRect(mx,my)) {
				e=&NoEvent;
			}
		}
		else if (e->IsTouchEvent()) {
			if (!panel->IsPointInSubstanceRect(tx,ty)) {
				e=&NoEvent;
			}
		}
		else if (e->IsKeyboardEvent()) {
			if (!panel->InActivePath) {
				e=&NoEvent;
			}
		}
	}

	for (child=panel->LastChild; child; child=child->Prev) {
		RecurseInput(child,*e,state);
		if (RestartInputRecursion) return;
	}

	panel->PendingInput=0;
	panel->Input(*e,state,mx,my);
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


void emView::PaintHighlight(const emPainter & painter) const
{
	emColor shadowColor,arrowColor;
	emPainter pnt;
	double cx1,cy1,cx2,cy2,vx,vy,vw,vh,sx,sy,sw,sh,sr,x1,y1,x2,y2;
	double goalX,goalY,lc,lh,lv,l,len,pc,pc2,pos,delta;
	int i,j,n,m;

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

	pnt=painter;
	pnt.SetScaling(1.0,1.0/CurrentPixelTallness);

	vx=ActivePanel->GetViewedX();
	vy=ActivePanel->GetViewedY()*CurrentPixelTallness;
	vw=ActivePanel->GetViewedWidth();
	vh=ActivePanel->GetViewedHeight()*CurrentPixelTallness;

	((const emPanel*)ActivePanel)->GetSubstanceRect(&sx,&sy,&sw,&sh,&sr);
	sx=vx+sx*vw;
	sy=vy+sy*vw;
	sw*=vw;
	sh*=vw;
	sr*=vw;
	if (sw<0.0) sw=0.0; else if (sw>vw) sw=vw;
	if (sh<0.0) sh=0.0; else if (sh>vh) sh=vh;
	if (sx<vx) sx=vx; else if (sx>vx+vw-sw) sx=vx+vw-sw;
	if (sy<vy) sy=vy; else if (sy>vy+vh-sh) sy=vy+vh-sh;
	if (sr<0.0) sr=0.0;
	if (sr>sw*0.5) sr=sw*0.5;
	if (sr>sh*0.5) sr=sh*0.5;
	sx-=distanceFromPanel;
	sy-=distanceFromPanel;
	sw+=2*distanceFromPanel;
	sh+=2*distanceFromPanel;
	sr+=distanceFromPanel;

	cx1=pnt.GetUserClipX1()-arrowSize*2.0;
	cy1=pnt.GetUserClipY1()-arrowSize*2.0;
	cx2=pnt.GetUserClipX2()+arrowSize*2.0;
	cy2=pnt.GetUserClipY2()+arrowSize*2.0;
	if (sx>=cx2 || sx+sw<=cx1 || sy>=cy2 || sy+sh<=cy1) return;

	pnt.LeaveUserSpace();

	shadowColor=emColor(0,0,0,192);
	if (ActivationAdherent) arrowColor=adherentHighlightColor;
	else arrowColor=highlightColor;
	if (!Focused || (VFlags&VF_NO_FOCUS_HIGHLIGHT)!=0) {
		shadowColor.SetAlpha((emByte)(shadowColor.GetAlpha()/3));
		arrowColor.SetAlpha((emByte)(arrowColor.GetAlpha()/3));
	}

	x1=sx+sr;
	x2=sx+sw-sr;
	y1=sy+sr;
	y2=sy+sh-sr;

	goalX=(x1+x2)*0.5;
	goalY=(y1+y2)*0.5;

	lh=x2-x1;
	lv=y2-y1;
	lc=0.5*M_PI*sr;

	pc=lc*0.5;
	if (lc>1E-10) {
		pc2=(lv*0.5+lc+lh*0.5)*0.5-lv*0.5;
		if (pc2<0.0) pc2=0.0;
		if (pc2>lc) pc2=lc;
		l=lc/(lc+emMin(lh,lv));
		pc=pc*(1.0-l)+pc2*l;
	}

	for (i=0; i<4; i++) {
		switch (i) {
		case 0:
			pos=pc;
			len=(lc-pc)*2.0+lh;
			break;
		case 1:
			pos=lc+lh+lc-pc;
			len=pc*2.0+lv;
			break;
		case 2:
			pos=lc+lh+lc+lv+pc;
			len=(lc-pc)*2.0+lh;
			break;
		default:
			pos=lc+lh+lc+lv+lc+lh+lc-pc;
			len=pc*2.0+lv;
			break;
		}

		n=(int)(emMin(len/arrowDistance,1E9)+0.5);
		if (n<1) n=1;
		for (m=1; m<n; m*=2);
		n&=(m|(m>>1)|(m>>2));

		delta=len/n;

		for (j=0; n>0; j=(j+1)&7) {
			if (!(j&1)) l=lc;
			else if (j&2) l=lv;
			else l=lh;
			m=(int)floor(1.0+(l-pos)/delta);
			if (m>0) {
				if (m>n) m=n;
				if (j&1) {
					PaintHighlightArrowsOnLine(
						pnt,
						j==1 ? x2    : j==3 ? x1-sr : j==5 ? x1    : x2+sr,
						j==1 ? y2+sr : j==3 ? y2    : j==5 ? y1-sr : y1,
						j==1 ? -1.0 : j==5 ? 1.0 : 0.0,
						j==3 ? -1.0 : j==7 ? 1.0 : 0.0,
						pos,delta,m,goalX,goalY,
						arrowSize,shadowColor,arrowColor
					);
				}
				else {
					PaintHighlightArrowsOnBow(
						pnt,
						j==2 || j==4 ? x1 : x2,
						j>=4 ? y1 : y2,
						sr,j/2,pos,delta,m,goalX,goalY,
						arrowSize,shadowColor,arrowColor
					);
				}
				pos+=delta*m;
				n-=m;
			}
			pos-=l;
		}
	}

	pnt.EnterUserSpace();
}


void emView::PaintHighlightArrowsOnLine(
	const emPainter & painter, double x, double y,
	double dx, double dy, double pos, double delta,
	int count, double goalX, double goalY, double arrowSize,
	emColor shadowColor, emColor arrowColor
) const
{
	double cx1,cy1,cx2,cy2,minPos,maxPos,t;

	minPos=-1E100;
	maxPos=1E100;

	cx1=painter.GetUserClipX1()-arrowSize*2.0;
	cx2=painter.GetUserClipX2()+arrowSize*2.0;
	if (dx>1E-10) {
		t=(cx1-x)/dx; if (minPos<t) minPos=t;
		t=(cx2-x)/dx; if (maxPos>t) maxPos=t;
	}
	else if (dx<-1E-10) {
		t=(cx2-x)/dx; if (minPos<t) minPos=t;
		t=(cx1-x)/dx; if (maxPos>t) maxPos=t;
	}
	else if (x>=cx2 || x<=cx1) {
		return;
	}

	cy1=painter.GetUserClipY1()-arrowSize*2.0;
	cy2=painter.GetUserClipY2()+arrowSize*2.0;
	if (dy>1E-10) {
		t=(cy1-y)/dy; if (minPos<t) minPos=t;
		t=(cy2-y)/dy; if (maxPos>t) maxPos=t;
	}
	else if (dy<-1E-10) {
		t=(cy2-y)/dy; if (minPos<t) minPos=t;
		t=(cy1-y)/dy; if (maxPos>t) maxPos=t;
	}
	else if (y>=cy2 || y<=cy1) {
		return;
	}

	if (pos<minPos) {
		t=ceil((minPos-pos)/delta);
		if (t>=(double)count) return;
		count-=(int)(t+0.5);
		pos+=t*delta;
	}

	while (count>0 && pos<=maxPos) {
		PaintHighlightArrow(
			painter,x+dx*pos,y+dy*pos,
			goalX,goalY,arrowSize,
			shadowColor,arrowColor
		);
		pos+=delta;
		count--;
	}
}


void emView::PaintHighlightArrowsOnBow(
	const emPainter & painter, double x, double y, double radius,
	int quadrant, double pos, double delta, int count,
	double goalX, double goalY, double arrowSize,
	emColor shadowColor, emColor arrowColor
) const
{
	double cx1,cy1,cx2,cy2,minPos,maxPos,t,a;
	int i;

	minPos=-1E100;
	maxPos=1E100;

	cx1=painter.GetUserClipX1()-arrowSize*2.0;
	cy1=painter.GetUserClipY1()-arrowSize*2.0;
	cx2=painter.GetUserClipX2()+arrowSize*2.0;
	cy2=painter.GetUserClipY2()+arrowSize*2.0;

	cx1-=x;
	cy1-=y;
	cx2-=x;
	cy2-=y;

	quadrant&=3;
	for (i=0; i<quadrant; i++) {
		t=cx1;
		cx1=cy1;
		cy1=-cx2;
		cx2=cy2;
		cy2=-t;
	}

	if (cx1>=radius || cx2<=0.0) return;
	if (cy1>=radius || cy2<=0.0) return;

	if (cx1>0.0) {
		t=acos(cx1/radius)*radius;
		if (maxPos>t) maxPos=t;
	}
	if (cx2<radius) {
		t=acos(cx2/radius)*radius;
		if (minPos<t) minPos=t;
	}
	if (cy1>0.0) {
		t=asin(cy1/radius)*radius;
		if (minPos<t) minPos=t;
	}
	if (cy2<radius) {
		t=asin(cy2/radius)*radius;
		if (maxPos>t) maxPos=t;
	}

	if (pos<minPos) {
		t=ceil((minPos-pos)/delta);
		if (t>=(double)count) return;
		count-=(int)(t+0.5);
		pos+=t*delta;
	}

	while (count>0 && pos<=maxPos) {
		a=quadrant*M_PI*0.5+pos/radius;
		PaintHighlightArrow(
			painter,
			x+cos(a)*radius,
			y+sin(a)*radius,
			goalX,goalY,arrowSize,
			shadowColor,arrowColor
		);
		pos+=delta;
		count--;
	}
}


void emView::PaintHighlightArrow(
	const emPainter & painter, double x, double y,
	double goalX, double goalY, double arrowSize,
	emColor shadowColor, emColor arrowColor
) const
{
	double sxy[4*2],axy[4*2];
	double dx,dy,d,aw,ah,ag,sd;

	dx=x-goalX;
	dy=y-goalY;
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
	axy[3]=y+dy*ah+dx*aw*0.5;
	axy[4]=x+dx*ag;
	axy[5]=y+(dy*ag);
	axy[6]=x+dx*ah+dy*aw*0.5;
	axy[7]=y+(dy*ah-dx*aw*0.5);

	sxy[0]=axy[0];
	sxy[1]=axy[1];
	sxy[2]=axy[2]+sd;
	sxy[3]=axy[3]+sd;
	sxy[4]=axy[4]+sd*ag/ah;
	sxy[5]=axy[5]+sd*ag/ah;
	sxy[6]=axy[6]+sd;
	sxy[7]=axy[7]+sd;

	painter.PaintPolygon(sxy,4,shadowColor);
	painter.PaintPolygon(axy,4,arrowColor);
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


bool emView::IsHopeForSeeking() const
{
	return SeekPosPanel && SeekPosPanel->IsHopeForSeeking();
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
				HomeView->RawZoomOut();
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


bool emViewPort::IsSoftKeyboardShown() const
{
	return false;
}


void emViewPort::ShowSoftKeyboard(bool show)
{
}


emUInt64 emViewPort::GetInputClockMS() const
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


bool emViewPort::IsSoftKeyboardShown()
{
	return ((const emViewPort*)this)->IsSoftKeyboardShown();
}


emUInt64 emViewPort::GetInputClockMS()
{
	return ((const emViewPort*)this)->GetInputClockMS();
}


emViewPort::emViewPort()
{
	HomeView=NULL;
	CurrentView=NULL;
}
