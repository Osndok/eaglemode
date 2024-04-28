//------------------------------------------------------------------------------
// emHmiDemoAnalogDisplay.cpp
//
// Copyright (C) 2012,2014,2016,2022,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoAnalogDisplay.h>


emHmiDemoAnalogDisplay::emHmiDemoAnalogDisplay(
	ParentArg parent, const emString & name,
	const emString & unit,
	emInt64 minValue, emInt64 maxValue, emInt64 value
)
	: emBorder(parent,name),
	Timer(GetScheduler())
{
	Unit=unit;
	MinValue=minValue;
	if (maxValue<minValue) maxValue=minValue;
	MaxValue=maxValue;
	if (value<minValue) value=minValue;
	if (value>maxValue) value=maxValue;
	Value=value;
	ScaleMarkInterval1=5;
	ScaleMarkInterval2=1;
	Radix=1;
	AnalogDigitsAfterRadix=0;
	DigitalDigitsAfterRadix=0;
	TextBoxTallness=0.5;
	ColoredRanges.SetTuningLevel(4);
	AnimValue=Value;
	AnimAmplitude=0;
	AnimFrequency=0;
	AddWakeUpSignal(Timer.GetSignal());
	SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
}


emHmiDemoAnalogDisplay::~emHmiDemoAnalogDisplay()
{
}


void emHmiDemoAnalogDisplay::SetMinValue(emInt64 minValue)
{
	if (MinValue!=minValue) {
		MinValue=minValue;
		if (MaxValue<MinValue) MaxValue=MinValue;
		InvalidatePainting();
		if (Value<MinValue) SetValue(MinValue);
	}
}


void emHmiDemoAnalogDisplay::SetMaxValue(emInt64 maxValue)
{
	if (MaxValue!=maxValue) {
		MaxValue=maxValue;
		if (MinValue>MaxValue) MinValue=MaxValue;
		InvalidatePainting();
		if (Value>MaxValue) SetValue(MaxValue);
	}
}


void emHmiDemoAnalogDisplay::SetMinMaxValues(emInt64 minValue, emInt64 maxValue)
{
	SetMinValue(minValue);
	SetMaxValue(maxValue);
}


void emHmiDemoAnalogDisplay::SetValue(emInt64 value)
{
	if (value<MinValue) value=MinValue;
	if (value>MaxValue) value=MaxValue;
	if (Value!=value) {
		Value=value;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::SetScaleMarkIntervals(
	emUInt64 scaleMarkInterval1, emUInt64 scaleMarkInterval2
)
{
	if (scaleMarkInterval2>scaleMarkInterval1) {
		scaleMarkInterval2=scaleMarkInterval1;
	}
	if (
		ScaleMarkInterval1!=scaleMarkInterval1 ||
		ScaleMarkInterval2!=scaleMarkInterval2
	) {
		ScaleMarkInterval1=scaleMarkInterval1;
		ScaleMarkInterval2=scaleMarkInterval2;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::SetRadix(emUInt64 radix)
{
	if (Radix!=radix) {
		Radix=radix;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::SetAnalogDigitsAfterRadix(unsigned analogDigitsAfterRadix)
{
	if (AnalogDigitsAfterRadix!=analogDigitsAfterRadix) {
		AnalogDigitsAfterRadix=analogDigitsAfterRadix;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::SetDigitalDigitsAfterRadix(unsigned digitalDigitsAfterRadix)
{
	if (DigitalDigitsAfterRadix!=digitalDigitsAfterRadix) {
		DigitalDigitsAfterRadix=digitalDigitsAfterRadix;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::SetTextBoxTallness(double textBoxTallness)
{
	if (TextBoxTallness!=textBoxTallness) {
		TextBoxTallness=textBoxTallness;
		InvalidatePainting();
	}
}


void emHmiDemoAnalogDisplay::AddColoredRange(emInt64 startValue, emInt64 endValue, emColor color)
{
	ColoredRange cr;

	cr.StartValue=startValue;
	cr.EndValue=endValue;
	cr.Color=color;
	ColoredRanges.Add(cr);
}


void emHmiDemoAnalogDisplay::SetAnimation(
	emInt64 value, emInt64 amplitude, double frequency
)
{
	AnimValue=value;
	AnimAmplitude=amplitude;
	AnimFrequency=frequency;
	if (amplitude!=0 && frequency>0) Timer.Start(30,true);
	else Timer.Stop(true);
}


bool emHmiDemoAnalogDisplay::Cycle()
{
	emUInt64 t,p;
	bool busy;

	busy=emBorder::Cycle();

	if (IsSignaled(Timer.GetSignal())) {
		t=(emUInt64)(1000.0/AnimFrequency+0.5);
		if (t<=0) t=1;
		p=emGetClockMS()%t;
		SetValue(AnimValue+(emInt64)floor(AnimAmplitude*sin(p*2*M_PI/t)+0.5));
	}

	return busy;
}


void emHmiDemoAnalogDisplay::PaintContent(
	const emPainter & painter, double x, double y, double w,
	double h, emColor canvasColor
) const
{
	emColor fgCol;
	char buf[256];
	double poly[8];
	double vc,r,as,ar,cx,cy,rx,ry,f,t,a1,a2,r1,r2,r3,r4,r5,r6,tx,ty,tw,th;
	emUInt64 vRange;
	emInt64 v;
	int i;

	vc=GetViewCondition(VCT_MIN_EXT);

	if (vc<9) return;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	vRange=MaxValue-MinValue;

	as=135.0;
	ar=270.0;
	cx=x+w*0.5;
	cy=y+h*0.5;
	rx=emMin(w,h)*0.5;
	ry=rx;

	r1=0.90;
	r2=0.86;
	r3=0.85;
	r4=0.78;
	r5=0.75;
	r6=0.64;

	if (GetInnerBorderType()==IBT_INPUT_FIELD) {
		fgCol=GetLook().GetInputFgColor();
	}
	else if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
		fgCol=GetLook().GetOutputFgColor();
	}
	else {
		fgCol=GetLook().GetFgColor();
	}

	if (vc>18) {
		for (i=0; i<ColoredRanges.GetCount(); i++) {
			f=(r1+r2)*0.5;
			t=r1-r2;
			a1=(ColoredRanges[i].StartValue-MinValue)*ar/vRange;
			a2=(ColoredRanges[i].EndValue-MinValue)*ar/vRange;
			painter.PaintEllipseArc(
				cx-rx*f,
				cy-ry*f,
				rx*2*f,
				ry*2*f,
				as+a1,a2-a1,
				(rx+ry)*0.5*t,
				ColoredRanges[i].Color
			);
		}
	}

	if (vc>=20) {
		v=MinValue;
		if (v>0) v=(v-1)/ScaleMarkInterval2+1;
		else if (v<0) v=-((-MinValue)/ScaleMarkInterval2);
		v*=ScaleMarkInterval2;
		for (;;) {
			t=emMin(
				2*M_PI*emMin(rx,ry)*r5*ScaleMarkInterval2/vRange*ar/360.0*0.5,
				(rx+ry)*0.5*(r3-r4)*0.4
			);
			if (v%ScaleMarkInterval1==0) {
				f=r5;
			}
			else {
				f=r4;
				t*=0.5;
			}
			a1=as+(v-MinValue)*ar/vRange;
			if (vc>=30) {
				painter.PaintLine(
					cx+rx*cos(a1*M_PI/180.0)*r3,
					cy+ry*sin(a1*M_PI/180.0)*r3,
					cx+rx*cos(a1*M_PI/180.0)*f,
					cy+ry*sin(a1*M_PI/180.0)*f,
					t,
					fgCol
				);
			}
			if (v%ScaleMarkInterval1==0) {
				sprintf(buf,"%.*f",AnalogDigitsAfterRadix,((double)v)/Radix);
				tx=cx+rx*cos(a1*M_PI/180.0)*r6;
				ty=cy+ry*sin(a1*M_PI/180.0)*r6;
				tw=(rx+ry)*0.5*(r5-r6)*1.8;
				th=tw;
				if (TextBoxTallness>1.0) tw/=TextBoxTallness;
				else th*=TextBoxTallness;
				painter.PaintTextBoxed(
					tx-tw*0.5,
					ty-th*0.5,
					tw,
					th,
					buf,th,
					fgCol,0,
					EM_ALIGN_CENTER,EM_ALIGN_CENTER
				);
			}
			if (v>=MaxValue) break;
			v+=ScaleMarkInterval2;
		}
	}

	if (vc>=28) {
		f=rx*0.1;
		tw=f*6;
		th=f*2;
		tx=cx-tw*0.5;
		ty=cy+ry*r6*0.5-th*0.5;
		painter.PaintTextBoxed(
			tx,ty,tw,th,
			Unit,th,
			fgCol,0,
			EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
	}

	if (vc>=25) {
		sprintf(buf,"%.*f",DigitalDigitsAfterRadix,((double)Value)/Radix);
		f=rx*0.1;
		tw=f*7.6;
		th=f*2.3;
		tx=cx-tw*0.5;
		ty=cy+ry*(r1-0.07)-th;
		painter.PaintTextBoxed(
			tx,ty,tw,th,
			buf,th,
			fgCol,0,
			EM_ALIGN_RIGHT,EM_ALIGN_RIGHT
		);
		if (vc>=50) {
			f*=0.2;
			painter.PaintRoundRectOutline(
				tx-f,ty-f,tw+2*f,th+2*f,
				f*1.5,f*1.5,
				f*0.5,
				fgCol
			);
		}
	}

	a1=as+(Value-MinValue)*ar/vRange;
	f=r3*0.8+r4*0.2;
	poly[0]=cx+rx*cos(a1*M_PI/180.0)*f;
	poly[1]=cy+ry*sin(a1*M_PI/180.0)*f;
	poly[2]=cx+rx*cos((a1+90)*M_PI/180.0)*f*0.08;
	poly[3]=cy+ry*sin((a1+90)*M_PI/180.0)*f*0.08;
	poly[4]=cx+rx*cos((a1+180)*M_PI/180.0)*f*0.3;
	poly[5]=cy+ry*sin((a1+180)*M_PI/180.0)*f*0.3;
	poly[6]=cx+rx*cos((a1+270)*M_PI/180.0)*f*0.08;
	poly[7]=cy+ry*sin((a1+270)*M_PI/180.0)*f*0.08;
	painter.PaintPolygon(poly,4,fgCol);

	if (!IsEnabled()) {
		painter.PaintRoundRect(
			x,y,w,h,r,r,
			GetLook().GetBgColor().GetTransparented(20.0F)
		);
	}
}
