//------------------------------------------------------------------------------
// emClockHandsPanel.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emClock/emClockHandsPanel.h>


emClockHandsPanel::emClockHandsPanel(
	ParentArg parent, const emString & name, emColor fgColor
)
	: emPanel(parent,name)
{
	FgColor=fgColor;
	Hour=0;
	Minute=0;
	Second=0;
	SetFocusable(false);
}


emClockHandsPanel::~emClockHandsPanel()
{
}


void emClockHandsPanel::SetFgColor(emColor fgColor)
{
	FgColor=fgColor;
	InvalidatePainting();
}


void emClockHandsPanel::SetTime(int hour, int minute, int second)
{
	if (Hour!=hour || Minute!=minute || Second!=second) {
		Hour=hour;
		Minute=minute;
		Second=second;
		InvalidatePainting();
	}
}


bool emClockHandsPanel::IsOpaque()
{
	return false;
}


void emClockHandsPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double hxy[5*2];
	double mxy[5*2];
	double sxy[4*2];
	double u,v,a,cx,cy,r,dx,dy,shx,shy,smx,smy,ssx,ssy;
	emColor color,shadow;
	int i;

	u=emMin(GetViewedWidth(),GetViewedHeight());
	v=emMin(GetView().GetCurrentWidth(),GetView().GetCurrentHeight());
	a=(v/u*0.75-0.08)*255.0;
	if (a<=0.0) return;
	if (a>255.0) a=255.0;
	shadow=emColor(0,0,0,(emByte)(a*0.2));
	color=FgColor;
	color.SetAlpha((emByte)a);

	cx=0.5;
	cy=GetHeight()*0.5;
	r=emMin(cx,cy);

	shx=r*0.010;
	shy=r*0.015;
	smx=r*0.016;
	smy=r*0.024;
	ssx=r*0.020;
	ssy=r*0.030;

	dx=r*sin((Hour+Minute/60.0+Second/3600.0)*M_PI/6.0);
	dy=-r*cos((Hour+Minute/60.0+Second/3600.0)*M_PI/6.0);
	hxy[0]=cx+shx-0.100*dx+0.039*dy;
	hxy[1]=cy+shy-0.100*dy-0.039*dx;
	hxy[2]=cx+shx-0.100*dx-0.039*dy;
	hxy[3]=cy+shy-0.100*dy+0.039*dx;
	hxy[4]=cx+shx+0.530*dx-0.039*dy;
	hxy[5]=cy+shy+0.530*dy+0.039*dx;
	hxy[6]=cx+shx+0.610*dx;
	hxy[7]=cy+shy+0.610*dy;
	hxy[8]=cx+shx+0.530*dx+0.039*dy;
	hxy[9]=cy+shy+0.530*dy-0.039*dx;

	dx=r*sin((Minute+Second/60.0)*M_PI/30.0);
	dy=-r*cos((Minute+Second/60.0)*M_PI/30.0);
	mxy[0]=cx+smx-0.100*dx+0.035*dy;
	mxy[1]=cy+smy-0.100*dy-0.035*dx;
	mxy[2]=cx+smx-0.100*dx-0.035*dy;
	mxy[3]=cy+smy-0.100*dy+0.035*dx;
	mxy[4]=cx+smx+0.910*dx-0.035*dy;
	mxy[5]=cy+smy+0.910*dy+0.035*dx;
	mxy[6]=cx+smx+0.960*dx;
	mxy[7]=cy+smy+0.960*dy;
	mxy[8]=cx+smx+0.910*dx+0.035*dy;
	mxy[9]=cy+smy+0.910*dy-0.035*dx;

	dx=r*sin(Second*M_PI/30.0);
	dy=-r*cos(Second*M_PI/30.0);
	sxy[0]=cx+ssx-0.150*dx+0.008*dy;
	sxy[1]=cy+ssy-0.150*dy-0.008*dx;
	sxy[2]=cx+ssx-0.150*dx-0.008*dy;
	sxy[3]=cy+ssy-0.150*dy+0.008*dx;
	sxy[4]=cx+ssx+1.000*dx-0.008*dy;
	sxy[5]=cy+ssy+1.000*dy+0.008*dx;
	sxy[6]=cx+ssx+1.000*dx+0.008*dy;
	sxy[7]=cy+ssy+1.000*dy-0.008*dx;

	painter.PaintPolygon(hxy,5,shadow);
	painter.PaintPolygon(mxy,5,shadow);
	painter.PaintPolygon(sxy,4,shadow);

	for (i=0; i<5; i++) { hxy[i*2]-=shx; hxy[i*2+1]-=shy; }
	for (i=0; i<5; i++) { mxy[i*2]-=smx; mxy[i*2+1]-=smy; }
	for (i=0; i<4; i++) { sxy[i*2]-=ssx; sxy[i*2+1]-=ssy; }

	painter.PaintPolygon(hxy,5,color);
	painter.PaintPolygon(mxy,5,color);
	painter.PaintPolygon(sxy,4,color);
}
