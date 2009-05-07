//------------------------------------------------------------------------------
// emClockPanel.cpp
//
// Copyright (C) 2006-2009 Oliver Hamann.
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

#include <emClock/emClockPanel.h>


emClockPanel::emClockPanel(
	ParentArg parent, const emString & name,
	emClockFileModel * fileModel, emTimeZonesModel::ZoneId zone
)
	: emFilePanel(parent,name,fileModel,true)
{
	FileModel=fileModel;
	TimeZonesModel=emTimeZonesModel::Acquire(GetRootContext());

	Zone=zone;

	DatePanel=NULL;
	StopwatchPanel=NULL;
	AlarmClockPanel=NULL;
	UTCPanel=NULL;
	WorldClockPanel=NULL;
	HandsPanel=NULL;

	BorderColor=emColor::BLACK;
	BgColor=emColor::WHITE;
	FgColor=emColor::BLACK;
	HandsColor=emColor::BLACK;

	CenterX=0.5;
	CenterY=0.5;
	Radius=0.5;

	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(TimeZonesModel->GetTimeSignal());
}


emClockPanel::~emClockPanel()
{
}


emString emClockPanel::GetTitle()
{
	if (Zone==emTimeZonesModel::UTC_ZONE_ID) {
		return "UTC";
	}
	else if (Zone!=emTimeZonesModel::LOCAL_ZONE_ID) {
		return GetName();
	}
	else {
		return "Clock";
	}
}


void emClockPanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	if (!IsVFSGood()) {
		emFilePanel::GetEssenceRect(pX,pY,pW,pH);
		return;
	}
	*pX=CenterX-Radius;
	*pY=CenterY-Radius;
	*pW=Radius*2;
	*pH=Radius*2;
}


bool emClockPanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();

	if (IsSignaled(GetVirFileStateSignal())) {
		CreateOrDestroyChildren();
		UpdateColors();
	}

	if (IsSignaled(FileModel->GetChangeSignal())) {
		UpdateColors();
	}

	if (IsSignaled(TimeZonesModel->GetTimeSignal())) {
		UpdateTime();
	}

	return busy;
}


void emClockPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);

	if (flags&NF_LAYOUT_CHANGED) {
		CenterX=0.5;
		CenterY=GetHeight()*0.5;
		Radius=emMin(CenterX,CenterY);
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
	if (flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED)) {
		CreateOrDestroyChildren();
	}
}


void emClockPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (
		IsVFSGood() &&
		Zone!=emTimeZonesModel::LOCAL_ZONE_ID &&
		Zone!=emTimeZonesModel::UTC_ZONE_ID
	) {
		if (event.IsMouseEvent()) {
			if ((mx-CenterX)*(mx-CenterX)+(my-CenterY)*(my-CenterY)<=Radius*Radius) {
				if (event.GetKey()==EM_KEY_RIGHT_BUTTON) BeFirst();
				else BeLast();
				Focus();
				event.Eat();
			}
		}
		else {
			emFilePanel::Input(event,state,mx,my);
		}
	}
	else {
		emFilePanel::Input(event,state,mx,my);
	}
}


bool emClockPanel::IsOpaque()
{
	if (!IsVFSGood()) return emFilePanel::IsOpaque();
	return false;
}


void emClockPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	char tmp[256];
	double xy[4*2];
	double vc,r1,r2,r3,r4,dx,dy,t,x,y,w,h;
	int i;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	painter.PaintEllipse(
		CenterX-Radius,
		CenterY-Radius,
		Radius*2,
		Radius*2,
		BgColor,
		canvasColor
	);
	canvasColor=BgColor;

	vc=GetViewCondition(VCT_MIN_EXT);

	if (vc>10.0) {
		r1=Radius*0.92;
		r2=Radius*0.935;
		r3=Radius*0.985;
		r4=Radius;
		painter.PaintEllipseOutline(
			CenterX-(r3+r4)*0.5,
			CenterY-(r3+r4)*0.5,
			r3+r4,
			r3+r4,
			r4-r3,
			BorderColor.GetLighted(-50.0f),
			canvasColor
		);
		painter.PaintEllipseOutline(
			CenterX-(r2+r3)*0.5,
			CenterY-(r2+r3)*0.5,
			r2+r3,
			r2+r3,
			r3-r2,
			BorderColor,
			canvasColor
		);
		painter.PaintEllipseOutline(
			CenterX-(r1+r2)*0.5,
			CenterY-(r1+r2)*0.5,
			r1+r2,
			r1+r2,
			r2-r1,
			BorderColor.GetLighted(-50.0f),
			canvasColor
		);
	}

	if (vc>12.0) {
		for (i=1; i<=12; i++) {
			sprintf(tmp,"%d",i);
			w=Radius*0.3;
			h=Radius*0.2;
			x=CenterX+sin(M_PI/6.0*i)*Radius*0.72-w*0.5;
			y=CenterY-cos(M_PI/6.0*i)*Radius*0.72-h*0.5+Radius*0.008;
			painter.PaintTextBoxed(
				x,
				y,
				w,
				h,
				tmp,
				h,
				FgColor,
				canvasColor
			);
		}
	}

	if (vc>12.0) {
		if (Zone==emTimeZonesModel::UTC_ZONE_ID) {
			painter.PaintTextBoxed(
				CenterX-Radius*0.5,
				CenterY-Radius*0.52,
				Radius*1.0,
				Radius*0.38,
				"UTC",
				Radius*0.38,
				FgColor,
				canvasColor
			);
		}
		else if (Zone!=emTimeZonesModel::LOCAL_ZONE_ID) {
			painter.PaintTextBoxed(
				CenterX-Radius*0.4,
				CenterY-Radius*0.53,
				Radius*0.8,
				Radius*0.3,
				GetName(),
				Radius*0.3,
				FgColor,
				canvasColor,
				EM_ALIGN_CENTER,
				EM_ALIGN_LEFT,
				0.8
			);
		}
	}

	if (vc>20.0) {
		t=Radius*0.05;
		painter.PaintEllipse(
			CenterX-t,
			CenterY-t,
			t*2,
			t*2,
			FgColor,
			canvasColor
		);
	}

	if (vc>20.0) {
		for (i=0; i<60; i+=5) {
			dx=sin(M_PI/30.0*i);
			dy=-cos(M_PI/30.0*i);
			r1=Radius*0.84;
			r2=Radius*0.9;
			t=Radius*0.03;
			xy[0]=CenterX+dx*r1+dy*t;
			xy[1]=CenterY+dy*r1-dx*t;
			xy[2]=CenterX+dx*r1-dy*t;
			xy[3]=CenterY+dy*r1+dx*t;
			xy[4]=CenterX+dx*r2-dy*t;
			xy[5]=CenterY+dy*r2+dx*t;
			xy[6]=CenterX+dx*r2+dy*t;
			xy[7]=CenterY+dy*r2-dx*t;
			painter.PaintPolygon(xy,4,FgColor,canvasColor);
		}
	}

	if (vc>30.0) {
		for (i=0; i<60; i++) {
			if (i%5==0) continue;
			dx=sin(M_PI/30.0*i);
			dy=-cos(M_PI/30.0*i);
			r1=Radius*0.85;
			r2=Radius*0.9;
			t=Radius*0.005;
			xy[0]=CenterX+dx*r1+dy*t;
			xy[1]=CenterY+dy*r1-dx*t;
			xy[2]=CenterX+dx*r1-dy*t;
			xy[3]=CenterY+dy*r1+dx*t;
			xy[4]=CenterX+dx*r2-dy*t;
			xy[5]=CenterY+dy*r2+dx*t;
			xy[6]=CenterX+dx*r2+dy*t;
			xy[7]=CenterY+dy*r2-dx*t;
			painter.PaintPolygon(xy,4,FgColor,canvasColor);
		}
	}

	if (!TimeError.IsEmpty() && vc>20.0 ) {
		w=Radius*1.6;
		h=Radius*0.2;
		x=CenterX-w*0.5;
		y=CenterY-h*0.5;
		painter.PaintRect(
			x,y,w,h,
			0x880000FF
		);
		painter.PaintTextBoxed(
			x,y,w,h,
			TimeError,
			h,
			0xFFFF00FF
		);
	}
}


void emClockPanel::LayoutChildren()
{
	emFilePanel::LayoutChildren();

	if (DatePanel) {
		DatePanel->Layout(
			CenterX+Radius*0.28,
			CenterY-Radius*0.18,
			Radius*0.36,
			Radius*0.36,
			BgColor
		);
	}
	if (StopwatchPanel) {
		StopwatchPanel->Layout(
			CenterX+Radius*0.05,
			CenterY+Radius*0.3,
			Radius*0.4,
			Radius*0.18,
			BgColor
		);
	}
	if (AlarmClockPanel) {
		AlarmClockPanel->Layout(
			CenterX-Radius*0.45,
			CenterY+Radius*0.29,
			Radius*0.4,
			Radius*0.2,
			BgColor
		);
	}
	if (UTCPanel) {
		UTCPanel->Layout(
			CenterX-Radius*0.62,
			CenterY-Radius*0.2,
			Radius*0.4,
			Radius*0.4,
			BgColor
		);
	}
	if (WorldClockPanel) {
		WorldClockPanel->Layout(
			CenterX-Radius*0.25,
			CenterY-Radius*0.55,
			Radius*0.5,
			Radius*0.35,
			BgColor
		);
	}
	if (HandsPanel) {
		HandsPanel->Layout(
			CenterX-Radius*0.91,
			CenterY-Radius*0.91,
			Radius*2*0.91,
			Radius*2*0.91
		);
	}
}


void emClockPanel::CreateOrDestroyChildren()
{
	bool haveDate, haveStopwatch, haveAlarmClock, haveUTC, haveWorldClock;
	bool haveHands, b, anyCreated;
	double vc;

	b = (IsVFSGood() && TimeError.IsEmpty());
	haveDate=b;
	haveStopwatch=b;
	haveAlarmClock=b;
	haveUTC=b;
	haveWorldClock=b;
	haveHands=b;

	if (!GetSoughtName()) {
		vc=GetViewCondition(VCT_MIN_EXT);
		if (vc<20.0) haveDate=false;
		if (vc<25.0) haveStopwatch=false;
		if (vc<25.0) haveAlarmClock=false;
		if (vc<22.0) haveUTC=false;
		if (vc<22.0) haveWorldClock=false;
		if (vc< 8.0) haveHands=false;
	}

	if (Zone!=emTimeZonesModel::LOCAL_ZONE_ID) {
		haveStopwatch=false;
		haveAlarmClock=false;
		haveUTC=false;
		haveWorldClock=false;
	}

	anyCreated=false;

	if (haveDate) {
		if (!DatePanel) {
			DatePanel=new emClockDatePanel(this,"date",FgColor);
			anyCreated=true;
		}
	}
	else {
		if (DatePanel) {
			delete DatePanel;
			DatePanel=NULL;
		}
	}

	if (haveStopwatch) {
		if (!StopwatchPanel) {
			StopwatchPanel=new emStopwatchPanel(this,"stopwatch",FileModel,FgColor);
			anyCreated=true;
		}
	}
	else {
		if (StopwatchPanel) {
			delete StopwatchPanel;
			StopwatchPanel=NULL;
		}
	}

	if (haveAlarmClock) {
		if (!AlarmClockPanel) {
			AlarmClockPanel=new emAlarmClockPanel(this,"alarm",FileModel,FgColor);
			anyCreated=true;
		}
	}
	else {
		if (AlarmClockPanel) {
			delete AlarmClockPanel;
			AlarmClockPanel=NULL;
		}
	}

	if (haveUTC) {
		if (!UTCPanel) {
			UTCPanel=new emClockPanel(
				this,"utc",FileModel,emTimeZonesModel::UTC_ZONE_ID
			);
			anyCreated=true;
		}
	}
	else {
		if (UTCPanel) {
			delete UTCPanel;
			UTCPanel=NULL;
		}
	}

	if (haveWorldClock) {
		if (!WorldClockPanel) {
			WorldClockPanel=new emWorldClockPanel(this,"world",FileModel);
			anyCreated=true;
		}
	}
	else {
		if (WorldClockPanel) {
			delete WorldClockPanel;
			WorldClockPanel=NULL;
		}
	}

	if (haveHands) {
		if (!HandsPanel) {
			HandsPanel=new emClockHandsPanel(this,"hands",HandsColor);
			anyCreated=true;
		}
	}
	else {
		if (HandsPanel) {
			delete HandsPanel;
			HandsPanel=NULL;
		}
	}

	if (anyCreated) {
		if (HandsPanel) HandsPanel->BeLast();
		UpdateTime();
	}
}


void emClockPanel::UpdateColors()
{
	emColor bo,bg,fg,hd;

	if (Zone==emTimeZonesModel::LOCAL_ZONE_ID) {
		bo=FileModel->ClockBorderColor;
		bg=FileModel->ClockBackgroundColor;
		fg=FileModel->ClockForegroundColor;
		hd=FileModel->ClockHandsColor;
	}
	else if (Zone==emTimeZonesModel::UTC_ZONE_ID) {
		bo=FileModel->UTCClockBorderColor;
		bg=FileModel->UTCClockBackgroundColor;
		fg=FileModel->UTCClockForegroundColor;
		hd=FileModel->UTCClockHandsColor;
	}
	else {
		bo=FileModel->WorldClockBorderColor;
		bg=FileModel->WorldClockBackgroundColor;
		fg=FileModel->WorldClockForegroundColor;
		hd=FileModel->WorldClockHandsColor;
	}

	if (BorderColor!=bo) {
		BorderColor=bo;
		InvalidatePainting();
	}

	if (BgColor!=bg) {
		BgColor=bg;
		InvalidateChildrenLayout();
		InvalidatePainting();
	}

	if (FgColor!=fg) {
		FgColor=fg;
		InvalidatePainting();
		if (DatePanel) DatePanel->SetFgColor(FgColor);
		if (StopwatchPanel) StopwatchPanel->SetFgColor(FgColor);
		if (AlarmClockPanel) AlarmClockPanel->SetFgColor(FgColor);
	}

	if (HandsColor!=hd) {
		HandsColor=hd;
		if (HandsPanel) HandsPanel->SetFgColor(HandsColor);
	}
}


void emClockPanel::UpdateTime()
{
	int year,month,day,dayOfWeek,hour,minute,second;

	if (IsVFSGood() && (DatePanel || HandsPanel || !TimeError.IsEmpty())) {
		try {
			TimeZonesModel->TryGetZoneTime(
				Zone,
				&year,
				&month,
				&day,
				&dayOfWeek,
				&hour,
				&minute,
				&second
			);
			if (!TimeError.IsEmpty()) {
				TimeError.Empty();
				CreateOrDestroyChildren();
				InvalidatePainting();
			}
		}
		catch (emString errorMessage) {
			if (TimeError!=errorMessage) {
				TimeError=errorMessage;
				CreateOrDestroyChildren();
				InvalidatePainting();
			}
			year=month=day=dayOfWeek=hour=minute=second=0;
		}
		if (DatePanel) {
			DatePanel->SetDate(year,month,day,dayOfWeek,hour,minute,second);
		}
		if (HandsPanel) {
			HandsPanel->SetTime(hour,minute,second);
		}
	}
}
