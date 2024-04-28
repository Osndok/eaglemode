//------------------------------------------------------------------------------
// emHmiDemoButton.cpp
//
// Copyright (C) 2012,2014,2016,2020,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoButton.h>
#include <emCore/emRes.h>


emHmiDemoButton::emHmiDemoButton(
	ParentArg parent, const emString & name,
	const emString & caption,
	const emString & description,
	const emImage & icon
)
	: emBorder(parent,name,caption,description,icon),
	Timer(GetScheduler())
{
	Checked=false;
	ButtonOffImage=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		emGetChildPath("Button","off.tga")
	);
	ButtonOnImage=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		emGetChildPath("Button","on.tga")
	);
	ButtonLightImage=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		emGetChildPath("Button","light.tga")
	);
	BlinkFrequency=0;
	AddWakeUpSignal(Timer.GetSignal());
	SetLabelInBorder(false);
	SetLabelAlignment(EM_ALIGN_CENTER);
	SetBorderType(OBT_NONE,IBT_NONE);
}


emHmiDemoButton::~emHmiDemoButton()
{
}


void emHmiDemoButton::SetChecked(bool checked)
{
	if (Checked!=checked) {
		Checked=checked;
		InvalidatePainting();
	}
}


void emHmiDemoButton::SetAnimation(double blinkFrequency)
{
	BlinkFrequency=blinkFrequency;
	if (blinkFrequency>0) Timer.Start(30,true);
	else Timer.Stop(true);
}


bool emHmiDemoButton::Cycle()
{
	emUInt64 t,p;
	bool busy;

	busy=emBorder::Cycle();

	if (IsSignaled(Timer.GetSignal())) {
		t=(emUInt64)(1000.0/BlinkFrequency+0.5);
		if (t<=0) t=1;
		p=emGetClockMS()%t;
		SetChecked(p<t/2);
	}

	return busy;
}


void emHmiDemoButton::PaintContent(
	const emPainter & painter, double x, double y, double w,
	double h, emColor canvasColor
) const
{
	emColor col1,col2;
	double vc,r,x2,y2,w2,h2,x3,y3,w3,h3;

	vc=GetViewCondition(VCT_MIN_EXT);

	if (vc<3) return;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	if (w>h) { x+=(w-h)*0.5; w=h; }
	else     { y+=(h-w)*0.5; h=w; }

	w2=w*0.744;
	h2=w2*h/w;
	x2=x+(w-w2)*0.5;
	y2=y+(h-h2)*0.5;

	col1=GetLook().GetBgColor();
	col2=GetLook().GetOutputBgColor();
	if (!col1.IsTotallyTransparent()) {
		painter.PaintEllipse(x,y,w,h,col1,canvasColor);
		canvasColor=col1;
	}
	painter.PaintEllipse(x2,y2,w2,h2,col2,canvasColor);

	painter.PaintImage(
		x,y,w,h,
		Checked ? ButtonOnImage : ButtonOffImage
	);

	if (vc<8) return;

	w3=w2*0.8;
	h3=h2*0.5;
	x3=x2+(w2-w3)*0.5;
	y3=y2+(h2-h3)*0.5;
	PaintLabel(
		painter,
		x3,y3,w3,h3,
		GetLook().GetOutputFgColor(),
		0
	);

	if (Checked) {
		painter.PaintImageColored(
			x,y,w,h,
			ButtonLightImage,
			0,col2.GetBlended(0xFFFFFFFF,50.0F),
			emTexture::EXTEND_ZERO
		);
	}
}
