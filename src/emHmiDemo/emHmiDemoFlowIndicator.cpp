//------------------------------------------------------------------------------
// emHmiDemoFlowIndicator.cpp
//
// Copyright (C) 2012,2016,2022,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoFlowIndicator.h>
#include <emCore/emRes.h>


emHmiDemoFlowIndicator::emHmiDemoFlowIndicator(
	ParentArg parent, const emString & name, double rpm, int shape
)
	: emPanel(parent,name),
	Timer(GetScheduler())
{
	RPM=0.0;
	Shape=shape;
	Angle=0.0;
	Time=emGetClockMS();
	Color1=emColor(204,204,204);
	Color2=emColor(51,51,51);
	Border=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		"RoundBorder.tga"
	);
	SetRPM(rpm);
	AddWakeUpSignal(Timer.GetSignal());
	SetFocusable(false);
}


emHmiDemoFlowIndicator::~emHmiDemoFlowIndicator()
{
}


void emHmiDemoFlowIndicator::SetRPM(double rpm)
{
	if (RPM!=rpm) {
		RPM=rpm;
		if (RPM==0.0) {
			Angle=0.0;
			Timer.Stop(true);
			InvalidatePainting();
		}
		else if (!Timer.IsRunning()) {
			Time=emGetClockMS();
			Timer.Start(30,true);
		}
	}
}


void emHmiDemoFlowIndicator::SetShape(int shape)
{
	if (Shape!=shape) {
		Shape=shape;
		InvalidatePainting();
	}
}


bool emHmiDemoFlowIndicator::Cycle()
{
	emUInt64 dt;

	if (IsSignaled(Timer.GetSignal())) {
		dt=emGetClockMS()-Time;
		Time+=dt;
		Angle=fmod(Angle+360.0*RPM/60000.0*dt,360.0);
		InvalidatePainting();
	}
	return false;
}


bool emHmiDemoFlowIndicator::IsOpaque() const
{
	return false;
}


void emHmiDemoFlowIndicator::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double d,x,y,w,h,rx,ry;
	int i;

	x=0.0;
	y=0.0;
	w=emMin(1.0,GetHeight());
	h=w;
	painter.PaintImage(x,y,w,h,Border,255,canvasColor);
	d=0.046;
	x+=d;
	y+=d;
	w-=d*2;
	h-=d*2;
	painter.PaintEllipse(x,y,w,h,Color1,canvasColor);
	switch (Shape) {
	case 0:
		painter.PaintEllipseSector(x,y,w,h,Angle,90.0,Color2,Color1);
		painter.PaintEllipseSector(x,y,w,h,Angle+180.0,90.0,Color2,Color1);
		break;
	case 1:
		rx=w*0.15;
		ry=h*0.15;
		for (i=0; i<6; i++) {
			painter.PaintEllipse(
				x+w*0.5+(w*0.5-rx)*cos((Angle+60.0*i)*M_PI/180.0)-rx,
				y+h*0.5+(h*0.5-ry)*sin((Angle+60.0*i)*M_PI/180.0)-ry,
				2*rx,
				2*ry,
				Color2,
				Color1
			);
		}
		break;
	case 2:
		painter.PaintLine(
			x+w*0.5+w*0.45*cos(Angle*M_PI/180.0),
			y+h*0.5+h*0.45*sin(Angle*M_PI/180.0),
			x+w*0.5-w*0.45*cos(Angle*M_PI/180.0),
			y+h*0.5-h*0.45*sin(Angle*M_PI/180.0),
			w*0.2,
			Color2,
			emStrokeEnd(),
			emStrokeEnd(),
			Color1
		);
		break;
	}
}
