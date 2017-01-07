//------------------------------------------------------------------------------
// emScalarField.cpp
//
// Copyright (C) 2005-2011,2014-2016 Oliver Hamann.
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

#include <emCore/emScalarField.h>


emScalarField::emScalarField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, emInt64 minValue,
	emInt64 maxValue, emInt64 value, bool editable
)
	: emBorder(parent,name,caption,description,icon)
{
	Editable=editable;
	MinValue=minValue;
	if (maxValue<minValue) maxValue=minValue;
	MaxValue=maxValue;
	if (value<minValue) value=minValue;
	if (value>maxValue) value=maxValue;
	Value=value;
	ScaleMarkIntervals.SetTuningLevel(4);
	ScaleMarkIntervals.Add(1);
	MarksNeverHidden=false;
	TextOfValueFunc=DefaultTextOfValue;
	TextOfValueFuncContext=NULL;
	TextBoxTallness=0.5;
	KBInterval=0;
	Pressed=false;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
}


emScalarField::~emScalarField()
{
}


void emScalarField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		if (editable) {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
		InvalidatePainting();
	}
}


void emScalarField::SetMinValue(emInt64 minValue)
{
	if (MinValue!=minValue) {
		MinValue=minValue;
		if (MaxValue<MinValue) MaxValue=MinValue;
		InvalidatePainting();
		if (Value<MinValue) SetValue(MinValue);
	}
}


void emScalarField::SetMaxValue(emInt64 maxValue)
{
	if (MaxValue!=maxValue) {
		MaxValue=maxValue;
		if (MinValue>MaxValue) MinValue=MaxValue;
		InvalidatePainting();
		if (Value>MaxValue) SetValue(MaxValue);
	}
}


void emScalarField::SetMinMaxValues(emInt64 minValue, emInt64 maxValue)
{
	SetMinValue(minValue);
	SetMaxValue(maxValue);
}


void emScalarField::SetValue(emInt64 value)
{
	if (value<MinValue) value=MinValue;
	if (value>MaxValue) value=MaxValue;
	if (Value!=value) {
		Value=value;
		InvalidatePainting();
		Signal(ValueSignal);
		ValueChanged();
	}
}


void emScalarField::SetScaleMarkIntervals(const emArray<emUInt64> & intervals)
{
	int i;

	for (i=0; i<intervals.GetCount(); i++) {
		if (intervals[i]==0 || (i>0 && intervals[i]>=intervals[i-1])) {
			emFatalError("emScalarField::SetScaleMarkIntervals: Illegal argument.");
		}
	}
	if (ScaleMarkIntervals.GetCount()==intervals.GetCount()) {
		for (i=intervals.GetCount()-1; i>=0; i--) {
			if (ScaleMarkIntervals[i]!=intervals[i]) break;
		}
		if (i<0) return;
	}
	ScaleMarkIntervals=intervals;
	InvalidatePainting();
}


void emScalarField::SetScaleMarkIntervals(
	unsigned interval1, unsigned interval2, ...
)
{
	emArray<emUInt64> intervals;
	va_list ap;
	unsigned u;

	intervals.SetTuningLevel(4);
	if (interval1) {
		intervals.Add((emUInt64)interval1);
		if (interval2) {
			intervals.Add((emUInt64)interval2);
			va_start(ap,interval2);
			for (;;) {
				u=va_arg(ap,unsigned);
				if (!u) break;
				intervals.Add((emUInt64)u);
			}
			va_end(ap);
		}
	}
	SetScaleMarkIntervals(intervals);
}


void emScalarField::SetNeverHideMarks(bool neverHide)
{
	if (MarksNeverHidden!=neverHide) {
		MarksNeverHidden=neverHide;
		InvalidatePainting();
	}
}


void emScalarField::TextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval
) const
{
	TextOfValueFunc(buf,bufSize,value,markInterval,TextOfValueFuncContext);
}


void emScalarField::SetTextOfValueFunc(
	void(*textOfValueFunc)(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	),
	void * context
)
{
	if (
		TextOfValueFunc!=textOfValueFunc ||
		TextOfValueFuncContext!=context
	) {
		TextOfValueFunc=textOfValueFunc;
		TextOfValueFuncContext=context;
		InvalidatePainting();
	}
}


void emScalarField::DefaultTextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	int l;

	l=emInt64ToStr(buf,bufSize,value);
	if (l<0 || l>=bufSize) l=0;
	buf[l]=0;
}


void emScalarField::SetTextBoxTallness(double textBoxTallness)
{
	if (TextBoxTallness!=textBoxTallness) {
		TextBoxTallness=textBoxTallness;
		InvalidatePainting();
	}
}


void emScalarField::SetKeyboardInterval(emUInt64 kbInterval)
{
	KBInterval=kbInterval;
}


void emScalarField::ValueChanged()
{
}


void emScalarField::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	bool inArea;
	emInt64 mv;

	inArea=CheckMouse(mx,my,&mv);

	if (Pressed) {
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			Pressed=false;
			InvalidatePainting();
		}
		if (Value!=mv && IsEditable() && IsEnabled()) {
			SetValue(mv);
		}
	}
	else if (
		inArea && event.IsKey(EM_KEY_LEFT_BUTTON) && IsEditable() &&
		IsEnabled() && GetViewCondition(VCT_MIN_EXT)>=8.0
	) {
		Pressed=true;
		InvalidatePainting();
		if (Value!=mv) {
			SetValue(mv);
		}
	}
	else if (strcmp(event.GetChars(),"+")==0 && IsEditable() && IsEnabled()) {
		StepByKeyboard(1);
		event.Eat();
	}
	else if (strcmp(event.GetChars(),"-")==0 && IsEditable() && IsEnabled()) {
		StepByKeyboard(-1);
		event.Eat();
	}

	emBorder::Input(event,state,mx,my);
}


bool emScalarField::HasHowTo() const
{
	return true;
}


emString emScalarField::GetHowTo() const
{
	emString h;

	h=emBorder::GetHowTo();
	h+=HowToScalarField;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emScalarField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	DoScalarField(
		SCALAR_FIELD_FUNC_PAINT,&painter,canvasColor,0.0,0.0,NULL,NULL
	);
}


bool emScalarField::CheckMouse(
	double mx, double my, emInt64 * pValue
) const
{
	bool b;

	DoScalarField(SCALAR_FIELD_FUNC_CHECK_MOUSE,NULL,0,mx,my,pValue,&b);
	return b;
}


void emScalarField::DoScalarField(
	DoScalarFieldFunc func, const emPainter * painter, emColor canvasColor,
	double mx, double my, emInt64 * pValue, bool * pHit
) const
{
	emColor bgCol,fgCol,col;
	double x,y,w,h,r,tx,ty,tw,th,s,d,e,f,ax,ay,aw,ah,dx,dy;
	double rx,ry,rw,rh,x3,w3,h4,h5,mw,mtw,mth,mah;
	char buf[256];
	double xy[5*2];
	const emUInt64 * ivals;
	emUInt64 ivalSum,vRange;
	emInt64 v,k1,k2;
	int ivalCnt,l;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	vRange=GetMaxValue()-GetMinValue();
	ivals=GetScaleMarkIntervals().Get();
	ivalCnt=GetScaleMarkIntervals().GetCount();
	if (!IsNeverHidingMarks()) {
		while (ivalCnt>1 && ivals[0]>vRange) { ivals++; ivalCnt--; }
	}
	ivalSum=0;
	for (l=0; l<ivalCnt; l++) ivalSum+=ivals[l];

	mtw=1.0;
	mth=GetTextBoxTallness();
	mah=emMin(mtw,mth)*0.5;
	d=1.0/(mth+mah);
	mtw*=d;
	mth*=d;
	mah*=d;
	mw=mtw*1.5;

	rx=x+r*0.5;
	ry=y+r*0.5;
	rw=w-r;
	rh=h-r;

	s=emMin(rh,rw);
	d=s*0.04;
	ax=rx+d;
	ay=ry+d;
	aw=rw-2*d;
	ah=rh-2*d;

	e=s*0.3*0.5;
	d=e-d;
	if (d<0.0) d=0.0;
	if (ivalCnt>0 && vRange) {
		th=ah;
		f=th*ivals[0]/ivalSum;
		tw=f*mw*vRange/ivals[0];
		f*=mtw;
		if (tw+f>aw) f*=aw/(tw+f);
		f*=0.5;
		if (d<f) d=f;
		f=aw*0.2;
		if (d>f) d=f;
		if (tw>aw-2*d) th*=(aw-2*d)/tw;
		ay+=ah-th;
		ah=th;
	}
	ax+=d;
	aw-=2*d;

	if (func==SCALAR_FIELD_FUNC_CHECK_MOUSE) {
		dx=emMax(emMax(x-mx,mx-x-w)+r,0.0);
		dy=emMax(emMax(y-my,my-y-h)+r,0.0);
		*pHit = dx*dx+dy*dy <= r*r;
		d=(mx-ax)/aw;
		d=d*vRange+GetMinValue();
		if (d<(double)GetMinValue()) d=(double)GetMinValue();
		if (d>(double)GetMaxValue()) d=(double)GetMaxValue();
		v=(emInt64)floor(d+0.5);
		if (v<GetMinValue()) v=GetMinValue();
		if (v>GetMaxValue()) v=GetMaxValue();
		*pValue=v;
		return;
	}

	if (GetInnerBorderType()==IBT_INPUT_FIELD) {
		bgCol=GetLook().GetInputBgColor();
		fgCol=GetLook().GetInputFgColor();
	}
	else if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
		bgCol=GetLook().GetOutputBgColor();
		fgCol=GetLook().GetOutputFgColor();
	}
	else {
		bgCol=GetLook().GetBgColor();
		fgCol=GetLook().GetFgColor();
	}

	if (!IsEnabled()) {
		bgCol=bgCol.GetBlended(GetLook().GetBgColor(),80.0F);
		fgCol=fgCol.GetBlended(GetLook().GetBgColor(),80.0F);
	}

	col=bgCol.GetBlended(fgCol,25);
	painter->PaintRect(rx,ry,ax-rx,rh,col,canvasColor);
	painter->PaintRect(ax+aw,ry,rx+rw-ax-aw,rh,col,canvasColor);
	canvasColor=0;

	if (!vRange) {
		tx=ax+aw*0.5;
	}
	else {
		tx=ax+aw*(((double)GetValue())-GetMinValue())/vRange;
	}
	if (e>ay+ah-ry) e=ay+ah-ry;
	xy[0]=tx-e; xy[1]=ry;
	xy[2]=tx+e; xy[3]=ry;
	xy[4]=tx+e; xy[5]=ay+ah-e;
	xy[6]=tx;   xy[7]=ay+ah;
	xy[8]=tx-e; xy[9]=ay+ah-e;
	painter->PaintPolygon(xy,5,fgCol,canvasColor);
	canvasColor=0;

	if (ivalCnt>0 && vRange) {
		f=aw/vRange;
		col=bgCol.GetBlended(fgCol,66);
		for (l=0, ty=ay; l<ivalCnt; l++) {
			th=ah/ivalSum*ivals[l];
			tw=mtw*th;
			if (tw*painter->GetScaleX()>1.0) {
				h4=mth*th;
				h5=mah*th;
				x3=painter->GetUserClipX1()-tw*0.5;
				w3=painter->GetUserClipX2()+tw*0.5-x3;
				if (x3<ax) x3=ax;
				if (w3>ax+aw-x3) w3=ax+aw-x3;
				k1=(emInt64)ceil(((x3-ax)/f+GetMinValue()-0.01)/ivals[l]);
				k2=(emInt64)floor(((x3+w3-ax)/f+GetMinValue()+0.01)/ivals[l]);
				while (k1<=k2) {
					v=k1*ivals[l];
					tx=(v-GetMinValue())*f+ax;
					TextOfValue(buf,sizeof(buf),v,ivals[l]);
					buf[sizeof(buf)-1]=0;
					painter->PaintTextBoxed(
						tx-tw*0.5,ty,tw,h4,
						buf,h4,
						col,canvasColor
					);
					xy[0]=tx-h5*0.5; xy[1]=ty+h4;
					xy[2]=tx+h5*0.5; xy[3]=ty+h4;
					xy[4]=tx;        xy[5]=ty+h4+h5;
					painter->PaintPolygon(xy,3,col,canvasColor);
					k1++;
				}
			}
			ty+=th;
		}
	}
}


void emScalarField::StepByKeyboard(int dir)
{
	emUInt64 r,dv,mindv;
	emInt64 v;
	int i;

	if (KBInterval>0) dv=KBInterval;
	else {
		r=MaxValue-MinValue;
		mindv=r/129;
		if (mindv<1) mindv=1;
		dv=mindv;
		for (i=0; i<ScaleMarkIntervals.GetCount(); i++) {
			if (ScaleMarkIntervals[i]>=mindv || i==0) {
				dv=ScaleMarkIntervals[i];
			}
		}
	}
	if (dir<0) {
		v=Value-dv;
		if (v<0) v=-(-v/dv)*dv; else v=(v+dv-1)/dv*dv;
	}
	else {
		v=Value+dv;
		if (v<0) v=-((-v+dv-1)/dv)*dv; else v=(v/dv)*dv;
	}
	SetValue(v);
}


const char * emScalarField::HowToScalarField=
	"\n"
	"\n"
	"SCALAR FIELD\n"
	"\n"
	"This is a scalar field. In such a field, a scalar value can be viewed and\n"
	"edited. Usually it is a number, but it can even be a choice of a series of\n"
	"possibilities.\n"
	"\n"
	"To move the needle to a desired value, click or drag with the left mouse button.\n"
	"Alternatively, you can move the needle by pressing the + and - keys.\n"
;


const char * emScalarField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This scalar field is read-only. You cannot move the needle.\n"
;
