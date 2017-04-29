//------------------------------------------------------------------------------
// emViewAnimator.cpp
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

#include <emCore/emViewAnimator.h>
#include <emCore/emPanel.h>


//==============================================================================
//=============================== emViewAnimator ===============================
//==============================================================================

emViewAnimator::emViewAnimator(emView & view)
	: emEngine(view.GetScheduler()),
	View(view)
{
	Master=NULL;
	ActiveSlave=NULL;
	UpperActivePtr=&View.ActiveAnimator;
	LastTSC=0;
	LastClk=0;
	DeactivateWhenIdle=false;
	SetEnginePriority(emEngine::HIGH_PRIORITY);
}


emViewAnimator::~emViewAnimator()
{
	Deactivate();
}


void emViewAnimator::SetMaster(emViewAnimator * master)
{
	emViewAnimator * va;

	if (Master!=master) {
		if (IsActive()) Deactivate();
		if (Master) {
			Master=NULL;
			UpperActivePtr=&View.ActiveAnimator;
		}
		if (master) {
			for (va=master; va; va=va->Master) if (va==this) return;
			Master=master;
			UpperActivePtr=&Master->ActiveSlave;
		}
	}
}


void emViewAnimator::Activate()
{
	if (!IsActive() && (!Master || Master->IsActive())) {
		if (*UpperActivePtr) {
			LastTSC=(*UpperActivePtr)->LastTSC;
			LastClk=(*UpperActivePtr)->LastClk;
			(*UpperActivePtr)->Deactivate();
		}
		else if (Master) {
			LastTSC=Master->LastTSC;
			LastClk=Master->LastClk;
		}
		*UpperActivePtr=this;
		WakeUp();
		emDLog("emViewAnimator::Activate: class = %s",typeid(*this).name());
	}
}


void emViewAnimator::Deactivate()
{
	if (ActiveSlave) {
		ActiveSlave->Deactivate();
	}

	if (*UpperActivePtr==this) {
		*UpperActivePtr=NULL;
		emDLog("emViewAnimator::Deactivate: class = %s",typeid(*this).name());
	}
}


void emViewAnimator::SetDeactivateWhenIdle(bool deactivateWhenIdle)
{
	if (DeactivateWhenIdle!=deactivateWhenIdle) {
		DeactivateWhenIdle=deactivateWhenIdle;
		if (DeactivateWhenIdle && IsActive()) {
			WakeUp(); // To be sure to deactivate soon if already idle.
		}
	}
}


void emViewAnimator::Input(emInputEvent & event, const emInputState & state)
{
	if (ActiveSlave) ActiveSlave->Input(event,state);
}


void emViewAnimator::Paint(const emPainter & painter) const
{
	if (ActiveSlave) ActiveSlave->Paint(painter);
}


bool emViewAnimator::Cycle()
{
	emUInt64 clk,tsc;
	double dt;
	bool busy;

	if (IsActive()) {
		tsc=GetScheduler().GetTimeSliceCounter();
		if (tsc!=LastTSC) {
			clk=GetView().GetInputClockMS();
			if (tsc==LastTSC+1) {
				dt=(clk-LastClk)*0.001;
				if (dt>0.33) dt=0.33;
			}
			else {
				dt=0.01;
			}
			LastTSC=tsc;
			LastClk=clk;
			if (dt>0.0) {
				busy=CycleAnimation(dt);
			}
			else {
				busy=true;
			}
		}
		else {
			busy=true;
		}
		if (!busy && DeactivateWhenIdle) {
			Deactivate();
		}
	}
	else {
		busy=false;
	}

	return busy;
}


//==============================================================================
//=========================== emKineticViewAnimator ============================
//==============================================================================

emKineticViewAnimator::emKineticViewAnimator(emView & view)
	: emViewAnimator(view)
{
	Velocity[0]=0.0;
	Velocity[1]=0.0;
	Velocity[2]=0.0;
	ZoomFixPointCentered=true;
	ZoomFixX=0.0;
	ZoomFixY=0.0;
	FrictionEnabled=false;
	Friction=1000.0;
	Busy=false;
}


emKineticViewAnimator::~emKineticViewAnimator()
{
}


void emKineticViewAnimator::Activate()
{
	emKineticViewAnimator * oldKVA;
	emViewAnimator * va;
	double fixX,fixY;
	bool fixCentered;

	if (!IsActive()) {
		oldKVA=NULL;
		for (va=GetView().GetActiveAnimator(); va; va=va->GetActiveSlave()) {
			oldKVA=dynamic_cast<emKineticViewAnimator*>(va);
			if (oldKVA) break;
		}
		if (oldKVA) {
			fixCentered=ZoomFixPointCentered;
			fixX=ZoomFixX;
			fixY=ZoomFixY;
			Velocity[0]=oldKVA->Velocity[0];
			Velocity[1]=oldKVA->Velocity[1];
			Velocity[2]=oldKVA->Velocity[2];
			ZoomFixPointCentered=oldKVA->ZoomFixPointCentered;
			ZoomFixX=oldKVA->ZoomFixX;
			ZoomFixY=oldKVA->ZoomFixY;
			if (fixCentered) {
				CenterZoomFixPoint();
			}
			else {
				SetZoomFixPoint(fixX,fixY);
			}
		}
		else {
			Velocity[0]=0.0;
			Velocity[1]=0.0;
			Velocity[2]=0.0;
		}
		emViewAnimator::Activate();
		UpdateBusyState();
	}
}


void emKineticViewAnimator::Deactivate()
{
	emViewAnimator::Deactivate();
}


double emKineticViewAnimator::GetAbsVelocity() const
{
	return sqrt(
		Velocity[0]*Velocity[0] +
		Velocity[1]*Velocity[1] +
		Velocity[2]*Velocity[2]
	);
}


void emKineticViewAnimator::SetVelocity(int dimension, double velocity)
{
	Velocity[dimension]=velocity;
	UpdateBusyState();
}


void emKineticViewAnimator::CenterZoomFixPoint()
{
	double oldFixX,oldFixY,f,q,dt;

	if (!ZoomFixPointCentered) {
		oldFixX=ZoomFixX;
		oldFixY=ZoomFixY;
		ZoomFixPointCentered=true;
		UpdateZoomFixPoint();
		f=GetView().GetZoomFactorLogarithmPerPixel();
		dt=0.01;
		q=(1.0-exp(-Velocity[2]*dt*f))/dt;
		Velocity[0]+=(oldFixX-ZoomFixX)*q;
		Velocity[1]+=(oldFixY-ZoomFixY)*q;
	}
}


void emKineticViewAnimator::SetZoomFixPoint(double zoomFixX, double zoomFixY)
{
	double oldFixX,oldFixY,f,q,dt;

	if (
		ZoomFixPointCentered ||
		ZoomFixX!=zoomFixX ||
		ZoomFixY!=zoomFixY
	) {
		UpdateZoomFixPoint();
		oldFixX=ZoomFixX;
		oldFixY=ZoomFixY;
		ZoomFixPointCentered=false;
		ZoomFixX=zoomFixX;
		ZoomFixY=zoomFixY;
		f=GetView().GetZoomFactorLogarithmPerPixel();
		dt=0.01;
		q=(1.0-exp(-Velocity[2]*dt*f))/dt;
		Velocity[0]+=(oldFixX-ZoomFixX)*q;
		Velocity[1]+=(oldFixY-ZoomFixY)*q;
	}
}


void emKineticViewAnimator::SetFrictionEnabled(bool enabled)
{
	FrictionEnabled=enabled;
}


void emKineticViewAnimator::SetFriction(double friction)
{
	Friction=friction;
}


bool emKineticViewAnimator::CycleAnimation(double dt)
{
	double v,v1,v2,f,a;
	double dist[3],done[3];
	int i;

	if (Busy) {

		if (IsFrictionEnabled()) {
			v=GetAbsVelocity();
			a=GetFriction();
			if (v-a*dt>0.0) {
				f=(v-a*dt)/v;
			}
			else if (v+a*dt<0.0) {
				f=(v+a*dt)/v;
			}
			else {
				f=0.0;
			}
		}
		else {
			f=1.0;
		}

		for (i=0; i<3; i++) {
			v1=Velocity[i];
			v2=v1*f;
			Velocity[i]=v2;
			dist[i]=(v1+v2)*0.5*dt;
			done[i]=0.0;
		}

		if (fabs(dist[0])>=0.01 || fabs(dist[1])>=0.01 || fabs(dist[2])>=0.01) {
			UpdateZoomFixPoint();
			GetView().RawScrollAndZoom(
				ZoomFixX,ZoomFixY,
				dist[0],dist[1],dist[2],
				NULL,
				&done[0],&done[1],&done[2]
			);
			GetView().SetActivePanelBestPossible();
		}

		for (i=0; i<3; i++) {
			if (fabs(done[i])<0.99*fabs(dist[i])) {
				Velocity[i]=0.0;
			}
		}

		UpdateBusyState();
	}

	return Busy;
}


void emKineticViewAnimator::UpdateBusyState()
{
	if (IsActive() && GetAbsVelocity()>0.01) {
		if (!Busy) {
			Busy=true;
			WakeUp();
		}
	}
	else {
		Velocity[0]=0.0;
		Velocity[1]=0.0;
		Velocity[2]=0.0;
		Busy=false;
	}
}


void emKineticViewAnimator::UpdateZoomFixPoint()
{
	double sx,sy,sw,sh,x1,y1,x2,y2;

	if (ZoomFixPointCentered) {
		x1=GetView().GetCurrentX();
		y1=GetView().GetCurrentY();
		x2=x1+GetView().GetCurrentWidth();
		y2=y1+GetView().GetCurrentHeight();
		if (GetView().IsPoppedUp()) {
			GetView().GetMaxPopupViewRect(&sx,&sy,&sw,&sh);
			if (x1<sx) x1=sx;
			if (y1<sy) y1=sy;
			if (x2>sx+sw) x2=sx+sw;
			if (y2>sy+sh) y2=sy+sh;
		}
		ZoomFixX=(x1+x2)*0.5;
		ZoomFixY=(y1+y2)*0.5;
	}
}


//==============================================================================
//=========================== emSpeedingViewAnimator ===========================
//==============================================================================

emSpeedingViewAnimator::emSpeedingViewAnimator(emView & view)
	: emKineticViewAnimator(view)
{
	TargetVelocity[0]=0.0;
	TargetVelocity[1]=0.0;
	TargetVelocity[2]=0.0;
	Acceleration=1.0;
	ReverseAcceleration=1.0;
	Busy=false;
}


emSpeedingViewAnimator::~emSpeedingViewAnimator()
{
}


void emSpeedingViewAnimator::Activate()
{
	if (!IsActive()) {
		emKineticViewAnimator::Activate();
		UpdateBusyState();
	}
}


void emSpeedingViewAnimator::Deactivate()
{
	emKineticViewAnimator::Deactivate();
}


double emSpeedingViewAnimator::GetAbsTargetVelocity() const
{
	return sqrt(
		TargetVelocity[0]*TargetVelocity[0] +
		TargetVelocity[1]*TargetVelocity[1] +
		TargetVelocity[2]*TargetVelocity[2]
	);
}


void emSpeedingViewAnimator::SetTargetVelocity(int dimension, double targetVelocity)
{
	TargetVelocity[dimension]=targetVelocity;
	UpdateBusyState();
}


void emSpeedingViewAnimator::SetAcceleration(double acceleration)
{
	Acceleration=acceleration;
}


void emSpeedingViewAnimator::SetReverseAcceleration(double reverseAcceleration)
{
	ReverseAcceleration=reverseAcceleration;
}


bool emSpeedingViewAnimator::CycleAnimation(double dt)
{
	double v1,v2,vt,adt;
	bool frictionEnabled,baseBusy;
	int i;

	if (Busy) {
		frictionEnabled=IsFrictionEnabled();
		for (i=0; i<3; i++) {
			v1=GetVelocity(i);
			vt=TargetVelocity[i];
			if (v1*vt<-0.1) adt=ReverseAcceleration*dt;
			else if (fabs(v1)<fabs(vt)) adt=Acceleration*emMin(dt,0.1);
			else if (frictionEnabled) adt=GetFriction()*dt;
			else adt=0.0;
			if (v1-adt>vt) {
				v2=v1-adt;
			}
			else if (v1+adt<vt) {
				v2=v1+adt;
			}
			else {
				v2=vt;
			}
			SetVelocity(i,v2);
		}
		SetFrictionEnabled(false);
		baseBusy=emKineticViewAnimator::CycleAnimation(dt);
		SetFrictionEnabled(frictionEnabled);
		UpdateBusyState();
	}
	else {
		baseBusy=emKineticViewAnimator::CycleAnimation(dt);
	}
	return Busy || baseBusy;
}


void emSpeedingViewAnimator::UpdateBusyState()
{
	if (IsActive() && GetAbsTargetVelocity()>0.01) {
		if (!Busy) {
			Busy=true;
			WakeUp();
		}
	}
	else {
		Busy=false;
	}
}


//==============================================================================
//=========================== emSwipingViewAnimator ============================
//==============================================================================

emSwipingViewAnimator::emSwipingViewAnimator(emView & view)
	: emKineticViewAnimator(view)
{
	Gripped=false;
	SpringExtension[0]=0.0;
	SpringExtension[1]=0.0;
	SpringExtension[2]=0.0;
	InstantaneousVelocity[0]=GetVelocity(0);
	InstantaneousVelocity[1]=GetVelocity(1);
	InstantaneousVelocity[2]=GetVelocity(2);
	SpringConstant=1.0;
	Busy=false;
}


emSwipingViewAnimator::~emSwipingViewAnimator()
{
}


void emSwipingViewAnimator::Activate()
{
	if (!IsActive()) {
		emKineticViewAnimator::Activate();
		SpringExtension[0]=0.0;
		SpringExtension[1]=0.0;
		SpringExtension[2]=0.0;
		InstantaneousVelocity[0]=GetVelocity(0);
		InstantaneousVelocity[1]=GetVelocity(1);
		InstantaneousVelocity[2]=GetVelocity(2);
		UpdateBusyState();
	}
}


void emSwipingViewAnimator::Deactivate()
{
	if (IsActive()) {
		SpringExtension[0]=0.0;
		SpringExtension[1]=0.0;
		SpringExtension[2]=0.0;
		emKineticViewAnimator::Deactivate();
	}
}


void emSwipingViewAnimator::SetGripped(bool gripped)
{
	if (Gripped!=gripped) {
		Gripped=gripped;
		if (!Gripped) {
			SpringExtension[0]=0.0;
			SpringExtension[1]=0.0;
			SpringExtension[2]=0.0;
			InstantaneousVelocity[0]=GetVelocity(0);
			InstantaneousVelocity[1]=GetVelocity(1);
			InstantaneousVelocity[2]=GetVelocity(2);
		}
	}
}


void emSwipingViewAnimator::MoveGrip(int dimension, double distance)
{
	if (Gripped) {
		SpringExtension[dimension]+=distance;
		UpdateBusyState();
	}
}


void emSwipingViewAnimator::SetSpringConstant(double springConstant)
{
	SpringConstant=springConstant;
}


double emSwipingViewAnimator::GetAbsSpringExtension() const
{
	return sqrt(
		SpringExtension[0]*SpringExtension[0] +
		SpringExtension[1]*SpringExtension[1] +
		SpringExtension[2]*SpringExtension[2]
	);
}


bool emSwipingViewAnimator::CycleAnimation(double dt)
{
	double v1,v2,e1,e2,w;
	bool frictionEnabled,baseBusy;
	int i;

	if (Busy && Gripped) {
		for (i=0; i<3; i++) {
			e1=SpringExtension[i];
			v1=InstantaneousVelocity[i];
			if (SpringConstant<1E5 && fabs(SpringExtension[i]/dt)>20.0) {
				// Critically damped spring.
				w=sqrt(SpringConstant);
				e2=(e1+(e1*w-v1)*dt)*exp(-w*dt);
				v2=(v1+(e1*w-v1)*dt*w)*exp(-w*dt);
			}
			else {
				v2=0.0;
				e2=0.0;
			}
			SpringExtension[i]=e2;
			InstantaneousVelocity[i]=v2;
			SetVelocity(i,(e1-e2)/dt);
		}
		frictionEnabled=IsFrictionEnabled();
		SetFrictionEnabled(false);
		baseBusy=emKineticViewAnimator::CycleAnimation(dt);
		SetFrictionEnabled(frictionEnabled);
	}
	else {
		baseBusy=emKineticViewAnimator::CycleAnimation(dt);
	}
	UpdateBusyState();
	return Busy || baseBusy;
}


void emSwipingViewAnimator::UpdateBusyState()
{
	if (
		IsActive() && Gripped &&
		(GetAbsSpringExtension()>0.01 || GetAbsVelocity()>0.01)
	) {
		if (!Busy) {
			Busy=true;
			WakeUp();
		}
	}
	else {
		SpringExtension[0]=0.0;
		SpringExtension[1]=0.0;
		SpringExtension[2]=0.0;
		Busy=false;
	}
}


//==============================================================================
//=========================== emMagneticViewAnimator ===========================
//==============================================================================

emMagneticViewAnimator::emMagneticViewAnimator(emView & view)
	: emKineticViewAnimator(view)
{
	CoreConfig=emCoreConfig::Acquire(view.GetRootContext());
	MagnetismActive=false;
	SetDeactivateWhenIdle();
}


emMagneticViewAnimator::~emMagneticViewAnimator()
{
}


void emMagneticViewAnimator::Activate()
{
	emKineticViewAnimator * oldKVA;
	emViewAnimator * va;

	if (!IsActive()) {
		MagnetismActive=false;
		oldKVA=NULL;
		for (va=GetView().GetActiveAnimator(); va; va=va->GetActiveSlave()) {
			oldKVA=dynamic_cast<emKineticViewAnimator*>(va);
			if (oldKVA) break;
		}
		if (oldKVA) {
			SetFriction(oldKVA->GetFriction());
			SetFrictionEnabled(oldKVA->IsFrictionEnabled());
		}
		else {
			SetFriction(1E10);
			SetFrictionEnabled(true);
		}
		emKineticViewAnimator::Activate();
	}
}


void emMagneticViewAnimator::Deactivate()
{
	emKineticViewAnimator::Deactivate();
}


bool emMagneticViewAnimator::CycleAnimation(double dt)
{
	double radiusFactor,minRadiusFactor,speedFactor,maxSpeedFactor;
	double x,y,w,h,v,d,t,fdt,k,a,absDist,maxDist;
	double dist[3];
	bool busy,frictionEnabled;

	radiusFactor=CoreConfig->MagnetismRadius;
	minRadiusFactor=CoreConfig->MagnetismRadius.GetMinValue();
	speedFactor=CoreConfig->MagnetismSpeed;
	maxSpeedFactor=CoreConfig->MagnetismSpeed.GetMaxValue();

	GetViewRect(&x,&y,&w,&h);
	maxDist=(w+h)*0.09*radiusFactor;
	if (radiusFactor<=minRadiusFactor*1.0001) {
		maxDist=0.0;
	}

	absDist=CalculateDistance(&dist[0],&dist[1],&dist[2]);

	busy=false;

	if (absDist<=maxDist && absDist>1E-3) {
		if (!MagnetismActive && GetAbsVelocity()<10.0) {
			CenterZoomFixPoint();
			MagnetismActive=true;
		}
		busy=true;
	}
	else {
		if (MagnetismActive) {
			SetVelocity(0,0.0);
			SetVelocity(1,0.0);
			SetVelocity(2,0.0);
			MagnetismActive=false;
		}
		if (GetAbsVelocity()>=0.01) {
			busy=true;
		}
	}

	if (MagnetismActive) {
		if (speedFactor>=maxSpeedFactor*0.9999 || absDist<1.0) {
			v=absDist/dt;
		}
		else {
			v=(
				GetVelocity(0)*dist[0] +
				GetVelocity(1)*dist[1] +
				GetVelocity(2)*dist[2]
			)/absDist;
			if (v<0.0) v=0.0;

			d=0.0;
			t=0.0;
			for (;;) {
				fdt = emMin(dt-t,0.01);
				if (fdt<1E-10) break;

				// Slope of hill.
				k=(absDist-d)/maxDist*4.0;
				if (fabs(k)>1.0) k=1.0/k;

				// Acceleration through rolling downhill.
				a=k*maxDist*25.0*speedFactor*speedFactor;

				// Damping
				a-=fabs(v)*15.0*speedFactor;

				v+=a*fdt;
				d+=v*fdt;
				if (d>=absDist) {
					d=absDist;
					break;
				}
				t+=fdt;
			}
			v=d/dt;
		}
		SetVelocity(0,v*dist[0]/absDist);
		SetVelocity(1,v*dist[1]/absDist);
		SetVelocity(2,v*dist[2]/absDist);
	}

	frictionEnabled=IsFrictionEnabled();
	SetFrictionEnabled(frictionEnabled && !MagnetismActive);
	if (emKineticViewAnimator::CycleAnimation(dt)) busy=true;
	SetFrictionEnabled(frictionEnabled);

	return busy;
}


double emMagneticViewAnimator::CalculateDistance(
	double * pDX, double * pDY, double * pDZ
) const
{
	double dd,vx,vy,vw,vh,zflpp,x,y,w,h,tx,ty,tz,td;
	emPanel * svp, * p;

	*pDX=1E+10;
	*pDY=1E+10;
	*pDZ=1E+10;
	dd=3E+100;

	if ((GetView().GetViewFlags()&emView::VF_POPUP_ZOOM)!=0) {
		// ??? emMagneticViewAnimator is still not functioning
		// ??? properly with pop-up-zoom.
		return sqrt(dd);
	}

	svp=GetView().GetSupremeViewedPanel();
	if (svp) {
		GetViewRect(&vx,&vy,&vw,&vh);
		zflpp=GetView().GetZoomFactorLogarithmPerPixel();
		for (p=svp;;) {
			if (p->IsViewed() && p->IsFocusable()) {
				p->GetEssenceRect(&x,&y,&w,&h);
				x=p->PanelToViewX(x);
				y=p->PanelToViewY(y);
				w=p->PanelToViewDeltaX(w);
				h=p->PanelToViewDeltaY(h);
				if (w>1E-3 && h>1E-3) {
					// Maximize panel in view (centered).
					tx=(x+w*0.5)-(vx+vw*0.5);
					ty=(y+h*0.5)-(vy+vh*0.5);
					if (w*vh>=h*vw) {
						tz=log(vw/w)/zflpp;
					}
					else {
						tz=log(vh/h)/zflpp;
					}
					td=tx*tx+ty*ty+tz*tz;
					if (td<dd) {
						*pDX=tx;
						*pDY=ty;
						*pDZ=tz;
						dd=td;
					}
					// Maximize view in panel.
					/*??? Helpful only for certain panel types, disturbing for others...
					static const double minViewInPanelFac = 1.5;
					if (w*vh>=h*vw*minViewInPanelFac) {
						tx=x-vx;
						if (tx<0.0) {
							tx=(x+w)-(vx+vw);
							if (tx>0.0) tx=0.0;
						}
						ty=(y+h*0.5)-(vy+vh*0.5);
						tz=log(vh/h)/zflpp;
						td=tx*tx+ty*ty+tz*tz;
						if (td<dd) {
							*pDX=tx;
							*pDY=ty;
							*pDZ=tz;
							dd=td;
						}
					}
					else if (h*vw>=w*vh*minViewInPanelFac) {
						tx=(x+w*0.5)-(vx+vw*0.5);
						ty=y-vy;
						if (ty<0.0) {
							ty=(y+h)-(vy+vh);
							if (ty>0.0) ty=0.0;
						}
						tz=log(vw/w)/zflpp;
						td=tx*tx+ty*ty+tz*tz;
						if (td<dd) {
							*pDX=tx;
							*pDY=ty;
							*pDZ=tz;
							dd=td;
						}
					}
					*/
				}
			}
			if (p->GetFirstChild()) p=p->GetFirstChild();
			else if (p==svp) break;
			else if (p->GetNext()) p=p->GetNext();
			else {
				do {
					p=p->GetParent();
				} while (p!=svp && !p->GetNext());
				if (p==svp) break;
				p=p->GetNext();
			}
		}
	}

	return sqrt(dd);
}


void emMagneticViewAnimator::GetViewRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	if ((GetView().GetViewFlags()&emView::VF_POPUP_ZOOM)!=0) {
		GetView().GetMaxPopupViewRect(pX,pY,pW,pH);
	}
	else {
		*pX=GetView().GetHomeX();
		*pY=GetView().GetHomeY();
		*pW=GetView().GetHomeWidth();
		*pH=GetView().GetHomeHeight();
	}
}


//==============================================================================
//=========================== emVisitingViewAnimator ===========================
//==============================================================================

emVisitingViewAnimator::emVisitingViewAnimator(emView & view)
	: emViewAnimator(view)
{
	Animated=false;
	Acceleration=5.0;
	MaxCuspSpeed=2.0;
	MaxAbsoluteSpeed=5.0;
	State=ST_NO_GOAL;
	VisitType=VT_VISIT;
	RelX=RelY=RelA=0;
	Adherent=false;
	UtilizeView=false;
	MaxDepthSeen=-1;
	Speed=0.0;
	TimeSlicesWithoutHope=0;
	GiveUpClock=0;
	SetDeactivateWhenIdle();
}


emVisitingViewAnimator::~emVisitingViewAnimator()
{
}


void emVisitingViewAnimator::SetAnimated(bool animated)
{
	Animated=animated;
}


void emVisitingViewAnimator::SetAcceleration(double acceleration)
{
	Acceleration=acceleration;
}


void emVisitingViewAnimator::SetMaxCuspSpeed(double maxCuspSpeed)
{
	MaxCuspSpeed=maxCuspSpeed;
}


void emVisitingViewAnimator::SetMaxAbsoluteSpeed(double maxAbsoluteSpeed)
{
	MaxAbsoluteSpeed=maxAbsoluteSpeed;
}


void emVisitingViewAnimator::SetAnimParamsByCoreConfig(const emCoreConfig & coreConfig)
{
	double f,fMax;

	f=coreConfig.VisitSpeed;
	fMax=coreConfig.VisitSpeed.GetMaxValue();

	Animated=(f<fMax*0.99999);
	Acceleration=35.0*f;
	MaxAbsoluteSpeed=35.0*f;
	MaxCuspSpeed=MaxAbsoluteSpeed*0.5;
}


void emVisitingViewAnimator::SetGoal(
	const char * identity, bool adherent, const char * subject
)
{
	SetGoal(VT_VISIT,identity,0.0,0.0,0.0,adherent,false,subject);
}


void emVisitingViewAnimator::SetGoal(
	const char * identity, double relX, double relY, double relA,
	bool adherent, const char * subject
)
{
	SetGoal(VT_VISIT_REL,identity,relX,relY,relA,adherent,false,subject);
}


void emVisitingViewAnimator::SetGoalFullsized(
	const char * identity, bool adherent, bool utilizeView, const char * subject
)
{
	SetGoal(VT_VISIT_FULLSIZED,identity,0.0,0.0,0.0,adherent,utilizeView,subject);
}


void emVisitingViewAnimator::ClearGoal()
{
	if (State!=ST_NO_GOAL) {
		State=ST_NO_GOAL;
		VisitType=VT_VISIT;
		Identity.Clear();
		RelX=RelY=RelA=0;
		Adherent=false;
		UtilizeView=false;
		Subject.Clear();
		Names.Clear();
		if (IsActive()) {
			GetView().SetSeekPos(NULL,NULL);
			MaxDepthSeen=-1;
			TimeSlicesWithoutHope=0;
			GiveUpClock=0;
			InvalidatePainting();
		}
	}
}


void emVisitingViewAnimator::Activate()
{
	if (!IsActive()) {
		emViewAnimator::Activate();
		if (State!=ST_NO_GOAL) {
			State=ST_CURVE;
			MaxDepthSeen=-1;
			TimeSlicesWithoutHope=0;
			GiveUpClock=0;
			Speed=0.0;
			WakeUp();
		}
	}
}


void emVisitingViewAnimator::Deactivate()
{
	if (IsActive()) {
		emViewAnimator::Deactivate();
		GetView().SetSeekPos(NULL,NULL);
		InvalidatePainting();
	}
}


void emVisitingViewAnimator::Input(emInputEvent & event, const emInputState & state)
{
	if (!IsActive() || (State!=ST_SEEK && State!=ST_GIVING_UP)) return;

	if (!event.IsEmpty()) {
		event.Eat();
		Deactivate();
	}
}


void emVisitingViewAnimator::Paint(const emPainter & painter) const
{
	double vx,vy,vw,vh,f,x,y,w,h,ws,tw,ch;
	emString str;
	int l1,l2;

	if (!IsActive() || (State!=ST_SEEK && State!=ST_GIVING_UP)) return;

	vx=GetView().GetCurrentX();
	vy=GetView().GetCurrentY();
	vw=GetView().GetCurrentWidth();
	vh=GetView().GetCurrentHeight();

	w=emMin(emMax(vw,vh)*0.6,vw);
	h=w*0.25;
	f=vh*0.8/h;
	if (f<1.0) { w*=f; h*=f; }
	x=vx+(vw-w)*0.5;
	y=emMax(vy+vh*0.5-h*1.25,vy);

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

	if (State == ST_GIVING_UP) {
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
	if (GetView().SeekPosPanel) str=GetView().SeekPosPanel->GetIdentity();
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


bool emVisitingViewAnimator::CycleAnimation(double dt)
{
	double relX,relY,relA,distFinal,dirX,dirY,distXY,distZ;
	double curveDist,curvePos,deltaX,deltaY,deltaZ,deltaXY,delta;
	double zflpp,vx,vy,vw,vh,doneX,doneY,doneZ,done;
	int depth,panelsAfter;
	bool adherent;
	emPanel * nep, * panel;

	switch (State) {
	case ST_NO_GOAL:
	case ST_GIVEN_UP:
	case ST_GOAL_REACHED:
		return false;
	case ST_GIVING_UP:
		if (emGetClockMS()<GiveUpClock+1500) {
			return true;
		}
		State=ST_GIVEN_UP;
		return false;
	case ST_CURVE:
	case ST_DIRECT:
	case ST_SEEK:
		break;
	}

	nep=GetNearestExistingPanel(
		&relX,&relY,&relA,&adherent,&depth,&panelsAfter,&distFinal
	);
	if (!nep) {
		State=ST_GIVING_UP;
		GiveUpClock=emGetClockMS();
		InvalidatePainting();
		return true;
	}

	// Activate nearest existing panel. But remember, that while the
	// animation is running, the goal panel may forward the activation
	// to a child while making itself unfocusable.
	if (nep->IsFocusable()) {
		nep->Activate(adherent);
	}
	else {
		panel=nep;
		while (panel->GetParent() && !panel->IsFocusable()) {
			panel=panel->GetParent();
		}
		if (!panel->IsInActivePath()) {
			panel->Activate(adherent);
		}
	}

	if (Animated) {
		if (MaxDepthSeen<depth) {
			if (State==ST_SEEK) {
				GetView().SetSeekPos(NULL,NULL);
				State=ST_CURVE;
			}
			MaxDepthSeen=depth;
		}
	}
	else {
		State=ST_SEEK;
		if (MaxDepthSeen<depth) {
			MaxDepthSeen=depth;
		}
	}

	if (State==ST_CURVE || State==ST_DIRECT) {

		GetDistanceTo(nep,relX,relY,relA,&dirX,&dirY,&distXY,&distZ);

		if (State==ST_DIRECT) {
			curvePos=0.0;
			curveDist=GetDirectDist(distXY,distZ);
		}
		else {
			GetCurvePosDist(distXY,distZ,&curvePos,&curveDist);
		}

		UpdateSpeed(curvePos,curveDist,panelsAfter,distFinal,dt);

		if (State==ST_DIRECT) {
			GetDirectPoint(distXY,distZ,Speed*dt,&deltaXY,&deltaZ);
		}
		else {
			CurvePoint cp1=GetCurvePoint(curvePos);
			CurvePoint cp2=GetCurvePoint(curvePos+Speed*dt);
			deltaXY=(cp2.X-cp1.X)*exp(cp1.Z);
			deltaZ=(cp2.Z-cp1.Z);
		}

		zflpp=GetView().GetZoomFactorLogarithmPerPixel();
		deltaXY/=zflpp;
		deltaZ/=zflpp;
		deltaX=dirX*deltaXY;
		deltaY=dirY*deltaXY;

		GetViewRect(&vx,&vy,&vw,&vh);
		GetView().RawScrollAndZoom(
			vx+vw*0.5, vy+vh*0.5,
			deltaX, deltaY, deltaZ,
			GetNearestViewedPanel(nep),
			&doneX, &doneY, &doneZ
		);

		delta=sqrt(deltaX*deltaX+deltaY*deltaY+deltaZ*deltaZ);
		done=sqrt(doneX*doneX+doneY*doneY+doneZ*doneZ);

		if (curveDist<=1E-6) {
			if (panelsAfter>0) {
				State=ST_SEEK;
			}
			else {
				State=ST_GOAL_REACHED;
				return false;
			}
		}
		else if (done < delta*0.2) {
			if (State==ST_CURVE) {
				State=ST_DIRECT;
			}
			else {
				State=ST_SEEK;
			}
		}
	}

	if (State==ST_SEEK) {
		if (depth+1>=Names.GetCount()) {
			GetView().RawVisit(nep,relX,relY,relA);
			State=ST_GOAL_REACHED;
			return false;
		}
		else if (GetView().SeekPosPanel!=nep) {
			GetView().SetSeekPos(nep,Names[depth+1]);
			GetView().RawVisitFullsized(nep);
			InvalidatePainting();
			TimeSlicesWithoutHope=4;
		}
		else if (GetView().IsHopeForSeeking()) {
			TimeSlicesWithoutHope=0;
		}
		else {
			TimeSlicesWithoutHope++;
			if (TimeSlicesWithoutHope>10) {
				State=ST_GIVING_UP;
				GiveUpClock=emGetClockMS();
				InvalidatePainting();
			}
		}
	}

	return true;
}


void emVisitingViewAnimator::SetGoal(
	VisitTypeEnum visitType, const char * identity, double relX, double relY,
	double relA, bool adherent, bool utilizeView, const char * subject
)
{
	VisitType=visitType;
	RelX=relX;
	RelY=relY;
	RelA=relA;
	Adherent=adherent;
	UtilizeView=utilizeView;
	Subject=subject;
	if (State==ST_NO_GOAL || Identity != identity) {
		State=ST_CURVE;
		Identity=identity;
		Names=emPanel::DecodeIdentity(Identity);
		if (IsActive()) {
			GetView().SetSeekPos(NULL,NULL);
			MaxDepthSeen=-1;
			TimeSlicesWithoutHope=0;
			GiveUpClock=0;
			InvalidatePainting();
		}
	}
}


void emVisitingViewAnimator::UpdateSpeed(
	double pos, double dist, int panelsAfter, double distFinal, double dt
)
{
	double s,v;

	Speed+=Acceleration*dt;

	s=dist+panelsAfter*log(2.0)+distFinal;
	if (s<0.0) s=0.0;
	v=sqrt(Acceleration*s*2.0);
	if (Speed>v) Speed=v;

	if (pos<0.0) {
		v=sqrt(Acceleration*(-pos)*2.0+MaxCuspSpeed*MaxCuspSpeed);
		if (Speed>v) Speed=v;
	}

	if (Speed>MaxAbsoluteSpeed) Speed=MaxAbsoluteSpeed;

	if (Speed>dist/dt) Speed=dist/dt;
}


emPanel * emVisitingViewAnimator::GetNearestExistingPanel(
	double * pRelX, double * pRelY, double * pRelA, bool * pAdherent,
	int * pDepth, int * pPanelsAfter, double * pDistFinal
) const
{
	emPanel * p, * c;
	int i;

	p=GetView().GetRootPanel();
	if (!p || Names.GetCount()<1 || Names[0]!=p->GetName()) {
		*pRelX=0.0;
		*pRelY=0.0;
		*pRelA=0.0;
		*pAdherent=false;
		*pDepth=0;
		*pPanelsAfter=Names.GetCount();
		*pDistFinal=0.0;
		return NULL;
	}

	for (i=1; i<Names.GetCount(); i++) {
		c=p->GetChild(Names[i]);
		if (!c) break;
		p=c;
	}

	if (i<Names.GetCount()) {
		GetView().CalcVisitFullsizedCoords(p,pRelX,pRelY,pRelA);
		*pAdherent=false;
		*pDepth=i-1;
		*pPanelsAfter=Names.GetCount()-i;
		switch (VisitType) {
		case VT_VISIT:
			*pDistFinal=0.0;
			break;
		case VT_VISIT_REL:
			if (RelA<=0.0 || RelA>=1.0) {
				*pDistFinal=0.0;
			}
			else {
				*pDistFinal=log(1.0/sqrt(RelA));
			}
			break;
		default:
			*pDistFinal=0.0;
			break;
		}
		return p;
	}

	switch (VisitType) {
	case VT_VISIT:
		GetView().CalcVisitCoords(p,pRelX,pRelY,pRelA);
		break;
	case VT_VISIT_REL:
		if (RelA<=0.0) {
			GetView().CalcVisitFullsizedCoords(p,pRelX,pRelY,pRelA,RelA<-0.9);
		}
		else {
			*pRelX=RelX;
			*pRelY=RelY;
			*pRelA=RelA;
		}
		break;
	default:
		GetView().CalcVisitFullsizedCoords(p,pRelX,pRelY,pRelA,UtilizeView);
		break;
	}
	*pAdherent=Adherent;
	*pDepth=Names.GetCount()-1;
	*pPanelsAfter=0;
	*pDistFinal=0.0;
	return p;
}


emPanel * emVisitingViewAnimator::GetNearestViewedPanel(
	emPanel * nearestExistingPanel
) const
{
	emPanel * p;

	p=nearestExistingPanel;
	while (p && !p->IsInViewedPath()) {
		p=p->GetParent();
	}
	while (
		p &&
		p->GetParent() &&
		p->GetParent()->IsViewed() && (
			!p->IsViewed() ||
			p->GetViewedWidth()<GetView().GetCurrentWidth()*1E-5 ||
			p->GetViewedHeight()<GetView().GetCurrentHeight()*1E-5
		)
	) {
		p=p->GetParent();
	}
	if (p && !p->IsViewed()) {
		p=GetView().GetSupremeViewedPanel();
	}
	return p;
}


void emVisitingViewAnimator::GetDistanceTo(
	emPanel * panel, double relX, double relY, double relA,
	double * pDirX, double * pDirY, double * pDistXY, double * pDistZ
) const
{
	double hx,hy,hw,hh,hp,sx,sy,sw,sh;
	double vx,vy,vw,vh,ax,ay,aw,ah,bx,by,bw,bh;
	double extremeDist,dx,dy,dz,dxy,t,f;
	emPanel * b, * a;

	// Home coordinates of the view.
	hx=GetView().GetHomeX();
	hy=GetView().GetHomeY();
	hw=GetView().GetHomeWidth();
	hh=GetView().GetHomeHeight();
	hp=GetView().GetHomePixelTallness();

	// Maximum coordinates of the view.
	GetViewRect(&sx,&sy,&sw,&sh);

	// Calculate rectangle "b": Where shall the view be at the end,
	// in the target panel, in panel coordinates.
	vw=sqrt(hw*hh*hp/(relA*panel->GetHeight()));
	vh=vw*panel->GetHeight()/hp;
	vx=hx+hw*0.5-(relX+0.5)*vw;
	vy=hy+hh*0.5-(relY+0.5)*vh;
	bx=(sx-vx)/vw;
	by=(sy-vy)/vw;
	bw=sw/vw;
	bh=sh/vw;

	// Go up with "b" in the tree until InViewedPath, but
	// at least until SVP.
	for (b=panel;;) {
		if (!b->GetParent()) break;
		if (b->IsInViewedPath() && !b->GetParent()->IsViewed()) break;
		bx=b->GetLayoutX()+bx*b->GetLayoutWidth();
		by=b->GetLayoutY()+by*b->GetLayoutWidth();
		bw=bw*b->GetLayoutWidth();
		bh=bh*b->GetLayoutWidth();
		b=b->GetParent();
	}

	// Get SVP and rectangle "a" therein:  Where is the view now.
	a=GetView().GetSupremeViewedPanel();
	ax=(sx-a->GetViewedX())/a->GetViewedWidth();
	ay=(sy-a->GetViewedY())/a->GetViewedWidth();
	aw=sw/a->GetViewedWidth();
	ah=sh/a->GetViewedWidth();

	// Go up with "a" until reaching "b", so that both rectangles
	// are in the same panel.
	while (a!=b) {
		ax=a->GetLayoutX()+ax*a->GetLayoutWidth();
		ay=a->GetLayoutY()+ay*a->GetLayoutWidth();
		aw=aw*a->GetLayoutWidth();
		ah=ah*a->GetLayoutWidth();
		a=a->GetParent();
	}

	// Calculate 3D distance.
	extremeDist=50.0;
	dx=bx-ax+(bw-aw)*0.5;
	dy=by-ay+(bh-ah)*0.5;
	t=aw+ah;
	if (t<1E-100) {
		dx=0.0;
		dy=0.0;
		dz=-extremeDist;
	}
	else {
		f=(sw+sh)*GetView().GetZoomFactorLogarithmPerPixel();
		dx=dx/t*f;
		dy=dy/t*f;
		t=(bw+bh)/t;
		if (t<exp(-extremeDist)) {
			dz = extremeDist;
		}
		else if (t>exp(extremeDist)) {
			dz = -extremeDist;
		}
		else {
			dz = -log(t);
		}
	}

	// Calculate 2D distance.
	dxy=sqrt(dx*dx+dy*dy);
	if (dxy<1E-100) {
		dxy=0.0;
		*pDirX = 1.0;
		*pDirY = 0.0;
	}
	else {
		*pDirX = dx/dxy;
		*pDirY = dy/dxy;
	}
	if (dxy>exp(extremeDist)) {
		*pDistXY=0.0;
		*pDistZ=-extremeDist;
	}
	else {
		*pDistXY=dxy;
		*pDistZ=dz;
	}
}


void emVisitingViewAnimator::GetViewRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	if ((GetView().GetViewFlags()&emView::VF_POPUP_ZOOM)!=0) {
		GetView().GetMaxPopupViewRect(pX,pY,pW,pH);
	}
	else {
		*pX=GetView().GetHomeX();
		*pY=GetView().GetHomeY();
		*pW=GetView().GetHomeWidth();
		*pH=GetView().GetHomeHeight();
	}
}


double emVisitingViewAnimator::GetDirectDist(double x, double z)
{
	double fixX;

	if (fabs(z)<0.1) {
		return sqrt(x*x+z*z);
	}
	else {
		fixX = x/(1-exp(-z));
		return fabs(z) * sqrt(fixX*fixX+1);
	}
}


void emVisitingViewAnimator::GetDirectPoint(
	double x, double z, double d,
	double * pX, double * pZ
)
{
	double fixX,dist,t;

	if (fabs(z)<0.1) {
		dist=sqrt(x*x+z*z);
		if (dist<1E-100) {
			t=0.0;
		}
		else {
			t=d/dist;
		}
		*pX = x*t;
		*pZ = z*t;
	}
	else {
		fixX = x/(1-exp(-z));
		dist = fabs(z) * sqrt(fixX*fixX+1);
		t = d/dist;
		*pX = fixX * (1-exp(-z*t));
		*pZ = z*t;
	}
}


void emVisitingViewAnimator::GetCurvePosDist(
	double x, double z, double * pCurvePos, double * pCurveDist
)
{
	double a,b,aMin,aMax,bMin,bMax;
	CurvePoint ap,bp,tp;
	bool neg,swap;
	int i,j;

	neg=false;
	swap=false;
	if (z<0) {
		z=-z;
		x/=exp(z);
		neg=true;
		swap=true;
	}
	if (x<0) {
		x=-x;
		neg=!neg;
	}

	aMin=-x;
	aMax=CurveMaxIndex*CurveDeltaDist;
	for (i=0;; i++) {
		a=(aMin+aMax)*0.5;
		ap=GetCurvePoint(a);
		tp.X=ap.X+x/exp(ap.Z);
		tp.Z=ap.Z+z;
		if (aMax-aMin<1E-12 || i>=48) break;
		if (tp.X<=0.0) {
			aMin=a;
			continue;
		}
		if (tp.X>=CurvePoints[CurveMaxIndex].X) {
			aMax=a;
			continue;
		}
		bMin=tp.Z;
		bMax=tp.Z+tp.X;
		for (j=0;; j++) {
			b=(bMin+bMax)*0.5;
			bp=GetCurvePoint(b);
			if (bMax-bMin<1E-12 || j>=48) break;
			if (tp.Z>bp.Z) {
				if (tp.X<=bp.X) break;
				bMin=b;
			}
			else {
				if (tp.X>=bp.X) break;
				bMax=b;
			}
		}
		if (tp.Z>bp.Z) aMin=a; else aMax=a;
	}
	bMin=tp.Z;
	bMax=tp.Z+tp.X;
	if (bMin<a+z) bMin=a+z;
	if (bMax<bMin) bMax=bMin;
	for (j=0;; j++) {
		b=(bMin+bMax)*0.5;
		if (bMax-bMin<1E-12 || j>=48) break;
		bp=GetCurvePoint(b);
		if (tp.Z>bp.Z) bMin=b; else bMax=b;
	}

	if (neg) {
		a=-a;
		b=-b;
	}
	if (swap) {
		*pCurvePos=b;
		*pCurveDist=a-b;
	}
	else {
		*pCurvePos=a;
		*pCurveDist=b-a;
	}
}


emVisitingViewAnimator::CurvePoint emVisitingViewAnimator::GetCurvePoint(double d)
{
	double t,x1,z1,x2,z2,x3,z3,c1,c2,c3,dx1,dz1,dx2,dz2;
	CurvePoint cp;
	int i;

	if (fabs(d)>=CurveMaxIndex*CurveDeltaDist) {
		cp=CurvePoints[CurveMaxIndex];
		if (d<0) cp.X = -cp.X;
		cp.Z+=fabs(d)-CurveMaxIndex*CurveDeltaDist;
		return cp;
	}

	t=fabs(d)/CurveDeltaDist;
	i=(int)t;
	if (i<0) i=0; // Can happen when d is a nan.
	if (i>=CurveMaxIndex) i=CurveMaxIndex-1;
	t-=i;
	if (t<0.0) t=0.0;
	if (t>1.0) t=1.0;

	x1=CurvePoints[i].X;
	z1=CurvePoints[i].Z;
	x2=CurvePoints[i+1].X;
	z2=CurvePoints[i+1].Z;

	if (i<=0) {
		dx1=CurveDeltaDist*0.5;
		dz1=0.0;
	}
	else {
		dx1=(x2-CurvePoints[i-1].X)*0.25;
		dz1=(z2-CurvePoints[i-1].Z)*0.25;
	}

	if (i+2>CurveMaxIndex) {
		dx2=0.0;
		dz2=CurveDeltaDist*0.5;
	}
	else {
		dx2=(CurvePoints[i+2].X-x1)*0.25;
		dz2=(CurvePoints[i+2].Z-z1)*0.25;
	}

	x3=(x1+dx1+x2-dx2)*0.5;
	z3=(z1+dz1+z2-dz2)*0.5;

	c1 = (1.0-t)*(1.0-t);
	c2 = t*t;
	c3 = 2*t*(1.0-t);
	cp.X = x1*c1 + x2*c2 + x3*c3;
	cp.Z = z1*c1 + z2*c2 + z3*c3;
	if (d<0) cp.X = -cp.X;
	return cp;
}


const double emVisitingViewAnimator::CurveDeltaDist = 0.0703125;


const emVisitingViewAnimator::CurvePoint emVisitingViewAnimator::CurvePoints[]={
	// This table was created with the following program. It's about the
	// shortest way respectively the way of minimum cost between two points
	// in an (x,z) coordinate system, where x is scrolling, and where z is
	// zooming as the natural logarithm of zoom factor, and where moving an
	// infinitesimal step in x has the same cost as moving the same step in
	// z, at z=0. For other z, cost of x has to be multiplied with exp(z).
	// I tried hard but did not find a direct function for the problem. So
	// the program solves it with iterations. I could write book how I came
	// to this (five different iterative algorithms...). If one ever works
	// on this: Be aware of the extreme behavior of the curve when it comes
	// to x near 1 - big risks of calculation errors - always check the
	// results very carefully (even graphical display of derivative...).
	//
	// #include <boost/multiprecision/mpfr.hpp>
	// #include <stdio.h>
	//
	// typedef boost::multiprecision::number<
	//   boost::multiprecision::mpfr_float_backend<50>
	// > FLT;
	//
	// double FLT2DBL(const FLT & x) {
	//   return x.convert_to<double>();
	// }
	//
	// static FLT Cost(FLT x1, FLT z1, FLT x2, FLT z2)
	// {
	//   FLT s = (z2-z1)/(x2-x1);
	//   if (fabs(s)<1E-10) {
	//     return sqrt(pow(z2-z1,2) + pow((x2-x1)*exp((z2+z1)*0.5),2));
	//   }
	//   // Calculate integral from x1 to x2 of:
	//   //   sqrt((s*dx)^2 + (dx*exp(z1+s*(x-x1)))^2)
	//   // Same as:
	//   //   sqrt(s^2 + exp(z1+s*(x-x1))^2) * dx
	//   // Solution:
	//   FLT w1 = sqrt(exp(2*z1) + s*s);
	//   FLT w2 = sqrt(exp(2*(s*(x2-x1)+z1)) + s*s);
	//   FLT c1 = log(w1-s)/2 - log(w1+s)/2 + w1/s;
	//   FLT c2 = log(w2-s)/2 - log(w2+s)/2 + w2/s;
	//   return c2-c1;
	// }
	//
	// int main(int argc, char * argv[])
	// {
	//   const int curveSize=128;
	//   FLT curveX[curveSize];
	//   FLT curveZ[curveSize];
	//
	//   FLT dt=1.0/curveSize*9.0;
	//   FLT x1=0.0;
	//   FLT z1=0.0;
	//   FLT x3=0.0;
	//   FLT z3=0.0;
	//   curveX[0]=0.0;
	//   curveZ[0]=0.0;
	//   for (int i=1; i<curveSize; i++) {
	//     // Calculate (x3,z3) so that:
	//     // - The distance from (x2,z2) to (x3,z3) equals dt.
	//     // - The cheapest way from (x1,z1) to (x3,z3) goes via (x2,z2).
	//     // Where (x2,z2) is (0,0).
	//     FLT minA=0.0;
	//     FLT maxA=acos((FLT)0.0);
	//     for (int j=0; j<128; j++) {
	//       FLT a=(minA+maxA)*0.5;
	//       z3=sin(a)*dt;
	//       if (fabs(z3)>1E-3) {
	//         x3=cos(a)*dt*(1.0-exp(-z3))/z3;
	//       }
	//       else {
	//         x3=cos(a)*dt/exp(z3*0.5);
	//       }
	//       if (i==1) {
	//         x1=-x3;
	//         z1=z3;
	//       }
	//       FLT dx=x3-x1;
	//       FLT dz=z3-z1;
	//       FLT l=sqrt(dx*dx+dz*dz);
	//       FLT q=1E-12;
	//       FLT x2b=-dz/l*q;
	//       FLT z2b=dx/l*q;
	//       FLT cost1 = Cost(x1,z1,0.0,0.0) + Cost(0.0,0.0,x3,z3);
	//       FLT cost2 = Cost(x1,z1,x2b,z2b) + Cost(x2b,z2b,x3,z3);
	//       if (cost2<cost1) maxA=a; else minA=a;
	//     }
	//     curveX[i]=curveX[i-1]+x3/exp(curveZ[i-1]);
	//     curveZ[i]=curveZ[i-1]+z3;
	//     x1=-x3*exp(z3);
	//     z1=-z3;
	//   }
	//
	//   for (int i=0; i<curveSize; i++) {
	//     // A little "correction".
	//     curveX[i]/=curveX[curveSize-1];
	//   }
	//
	//   printf("double CurveDeltaDist = %.8f;\n\n",FLT2DBL(dt));
	//   printf("CurvePoint CurvePoints[]={");
	//   for (int i=0; i<curveSize; i++) {
	//     printf("\n\t{ %.12f, %.8f }",FLT2DBL(curveX[i]),FLT2DBL(curveZ[i]));
	//     if (i<curveSize-1) printf(",");
	//   }
	//   printf("\n};\n");
	//   return 0;
	// }
	{ 0.000000000000, 0.00000000 },
	{ 0.070196996568, 0.00246786 },
	{ 0.139706409829, 0.00984731 },
	{ 0.207867277855, 0.02206685 },
	{ 0.274069721820, 0.03901038 },
	{ 0.337775385698, 0.06052148 },
	{ 0.398532523720, 0.08640897 },
	{ 0.455985066529, 0.11645328 },
	{ 0.509875667166, 0.15041328 },
	{ 0.560043303236, 0.18803304 },
	{ 0.606416423020, 0.22904841 },
	{ 0.649002844597, 0.27319286 },
	{ 0.687877659276, 0.32020270 },
	{ 0.723170290571, 0.36982132 },
	{ 0.755051666347, 0.42180262 },
	{ 0.783722223449, 0.47591350 },
	{ 0.809401221691, 0.53193559 },
	{ 0.832317626300, 0.58966630 },
	{ 0.852702640725, 0.64891924 },
	{ 0.870783840857, 0.70952419 },
	{ 0.886780775044, 0.77132669 },
	{ 0.900901845307, 0.83418737 },
	{ 0.913342265529, 0.89798109 },
	{ 0.924282893555, 0.96259598 },
	{ 0.933889748709, 1.02793239 },
	{ 0.942314048229, 1.09390185 },
	{ 0.949692621183, 1.16042604 },
	{ 0.956148583577, 1.22743579 },
	{ 0.961792181829, 1.29487016 },
	{ 0.966721732461, 1.36267553 },
	{ 0.971024603574, 1.43080482 },
	{ 0.974778198188, 1.49921674 },
	{ 0.978050911290, 1.56787514 },
	{ 0.980903041613, 1.63674836 },
	{ 0.983387646274, 1.70580876 },
	{ 0.985551331675, 1.77503218 },
	{ 0.987434978026, 1.84439754 },
	{ 0.989074397556, 1.91388643 },
	{ 0.990500928466, 1.98348284 },
	{ 0.991741967860, 2.05317279 },
	{ 0.992821447684, 2.12294410 },
	{ 0.993760258098, 2.19278617 },
	{ 0.994576622816, 2.26268978 },
	{ 0.995286430928, 2.33264690 },
	{ 0.995903529539, 2.40265055 },
	{ 0.996439981325, 2.47269463 },
	{ 0.996906290795, 2.54277387 },
	{ 0.997311602787, 2.61288367 },
	{ 0.997663876350, 2.68302002 },
	{ 0.997970036920, 2.75317946 },
	{ 0.998236109346, 2.82335896 },
	{ 0.998467334097, 2.89355590 },
	{ 0.998668268700, 2.96376798 },
	{ 0.998842876218, 3.03399323 },
	{ 0.998994602405, 3.10422992 },
	{ 0.999126442933, 3.17447655 },
	{ 0.999241001961, 3.24473182 },
	{ 0.999340543132, 3.31499459 },
	{ 0.999427033975, 3.38526388 },
	{ 0.999502184543, 3.45553885 },
	{ 0.999567481032, 3.52581873 },
	{ 0.999624215036, 3.59610289 },
	{ 0.999673508983, 3.66639077 },
	{ 0.999716338261, 3.73668188 },
	{ 0.999753550459, 3.80697579 },
	{ 0.999785882091, 3.87727215 },
	{ 0.999813973141, 3.94757062 },
	{ 0.999838379703, 4.01787093 },
	{ 0.999859584970, 4.08817284 },
	{ 0.999878008788, 4.15847614 },
	{ 0.999894015951, 4.22878064 },
	{ 0.999907923421, 4.29908620 },
	{ 0.999920006597, 4.36939266 },
	{ 0.999930504759, 4.43969992 },
	{ 0.999939625809, 4.51000786 },
	{ 0.999947550381, 4.58031641 },
	{ 0.999954435420, 4.65062547 },
	{ 0.999960417283, 4.72093498 },
	{ 0.999965614444, 4.79124489 },
	{ 0.999970129838, 4.86155513 },
	{ 0.999974052897, 4.93186567 },
	{ 0.999977461322, 5.00217647 },
	{ 0.999980422622, 5.07248749 },
	{ 0.999982995451, 5.14279871 },
	{ 0.999985230769, 5.21311009 },
	{ 0.999987172851, 5.28342162 },
	{ 0.999988860164, 5.35373328 },
	{ 0.999990326129, 5.42404505 },
	{ 0.999991599784, 5.49435691 },
	{ 0.999992706355, 5.56466886 },
	{ 0.999993667762, 5.63498088 },
	{ 0.999994503048, 5.70529296 },
	{ 0.999995228757, 5.77560510 },
	{ 0.999995859265, 5.84591728 },
	{ 0.999996407059, 5.91622951 },
	{ 0.999996882992, 5.98654177 },
	{ 0.999997296489, 6.05685406 },
	{ 0.999997655742, 6.12716639 },
	{ 0.999997967867, 6.19747873 },
	{ 0.999998239046, 6.26779109 },
	{ 0.999998474650, 6.33810348 },
	{ 0.999998679346, 6.40841587 },
	{ 0.999998857189, 6.47872829 },
	{ 0.999999011703, 6.54904071 },
	{ 0.999999145946, 6.61935314 },
	{ 0.999999262578, 6.68966558 },
	{ 0.999999363911, 6.75997803 },
	{ 0.999999451949, 6.83029049 },
	{ 0.999999528439, 6.90060295 },
	{ 0.999999594894, 6.97091542 },
	{ 0.999999652632, 7.04122789 },
	{ 0.999999702795, 7.11154036 },
	{ 0.999999746377, 7.18185284 },
	{ 0.999999784242, 7.25216532 },
	{ 0.999999817140, 7.32247781 },
	{ 0.999999845722, 7.39279029 },
	{ 0.999999870554, 7.46310278 },
	{ 0.999999892129, 7.53341527 },
	{ 0.999999910874, 7.60372776 },
	{ 0.999999927159, 7.67404025 },
	{ 0.999999941309, 7.74435274 },
	{ 0.999999953602, 7.81466524 },
	{ 0.999999964282, 7.88497773 },
	{ 0.999999973561, 7.95529023 },
	{ 0.999999981623, 8.02560272 },
	{ 0.999999988627, 8.09591522 },
	{ 0.999999994713, 8.16622772 },
	{ 1.000000000000, 8.23654021 }
};


const int emVisitingViewAnimator::CurveMaxIndex=
	sizeof(emVisitingViewAnimator::CurvePoints) /
	sizeof(emVisitingViewAnimator::CurvePoint) - 1
;
