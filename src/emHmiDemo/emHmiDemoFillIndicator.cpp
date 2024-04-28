//------------------------------------------------------------------------------
// emHmiDemoFillIndicator.cpp
//
// Copyright (C) 2012,2014,2016,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoFillIndicator.h>
#include <emCore/emRes.h>


emHmiDemoFillIndicator::emHmiDemoFillIndicator(
	ParentArg parent, const emString & name, double fill,
	emColor color
)
	: emBorder(parent,name)
{
	Fill=fill;
	Color=color;
	TickMark=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		"TickMark.tga"
	);
	SetBorderType(OBT_NONE,IBT_CUSTOM_RECT);
	SetBorderScaling(8.0);
	SetFocusable(false);
}


emHmiDemoFillIndicator::~emHmiDemoFillIndicator()
{
}


void emHmiDemoFillIndicator::SetFill(double fill)
{
	if (Fill!=fill) {
		Fill=fill;
		InvalidatePainting();
	}
}


bool emHmiDemoFillIndicator::IsOpaque() const
{
	return false;
}


void emHmiDemoFillIndicator::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double x,y,w,h,sw,sh,sy,sx;
	emColor cc;
	int i;

	emBorder::Paint(painter,canvasColor);
	GetContentRect(&x,&y,&w,&h,&cc);
	painter.PaintRect(x,y,w,h*(1-Fill),0x303030E0,cc);
	painter.PaintRect(x,y+h*(1-Fill),w,h*Fill,Color,cc);
	if (GetViewCondition(VCT_HEIGHT)>=40) {
		y+=h*0.01;
		h-=h*0.01*2;
		for (i=0; i<5; i++) {
			sw=w*((i&1)?0.4:0.6);
			sh=h*0.02;
			sx=x;
			sy=y+i*h/4-sh*0.5;
			painter.PaintBorderImage(
				sx,sy,sw,sh,
				sh*0.5,0,sh*0.5,0,
				TickMark,
				16,0,16,0,
				160,0,
				0222
			);
		}
	}
}
