//------------------------------------------------------------------------------
// emColorField.cpp
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

#include <emCore/emColorField.h>


emColorField::emColorField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, emColor color,
	bool editable, bool alphaEnabled
)
	: emBorder(parent,name,caption,description,icon)
{
	Exp=NULL;
	Color=color;
	Editable=editable;
	AlphaEnabled=alphaEnabled;
	Pressed=false;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
	SetAutoExpansionThreshold(9,VCT_MIN_EXT);
}


emColorField::~emColorField()
{
	if (Exp) delete Exp;
}


void emColorField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		InvalidatePainting();
		UpdateExpAppearance();
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
	}
}


void emColorField::SetAlphaEnabled(bool alphaEnabled)
{
	if (AlphaEnabled!=alphaEnabled) {
		AlphaEnabled=alphaEnabled;
		InvalidatePainting();
		UpdateExpAppearance();
		if (!alphaEnabled && Color.GetAlpha()!=255) {
			Color.SetAlpha(255);
			UpdateRGBAOutput();
			Signal(ColorSignal);
			ColorChanged();
		}
	}
}


void emColorField::SetColor(emColor color)
{
	if (Color!=color) {
		Color=color;
		UpdateRGBAOutput();
		UpdateHSVOutput();
		UpdateNameOutput();
		InvalidatePainting();
		Signal(ColorSignal);
		ColorChanged();
	}
}


void emColorField::ColorChanged()
{
}


bool emColorField::Cycle()
{
	bool busy,hsvChanged,rgbaChanged,textChanged;
	emColor oldColor;
	emString str;
	emInt64 v;

	busy=emBorder::Cycle();

	if (!Exp) return busy;

	hsvChanged=false;
	rgbaChanged=false;
	textChanged=false;
	oldColor=Color;

	if (IsSignaled(Exp->SfRed->GetValueSignal())) {
		v=Exp->SfRed->GetValue();
		if (Exp->RedOut!=v) {
			Exp->RedOut=v;
			Color.SetRed((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfGreen->GetValueSignal())) {
		v=Exp->SfGreen->GetValue();
		if (Exp->GreenOut!=v) {
			Exp->GreenOut=v;
			Color.SetGreen((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfBlue->GetValueSignal())) {
		v=Exp->SfBlue->GetValue();
		if (Exp->BlueOut!=v) {
			Exp->BlueOut=v;
			Color.SetBlue((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfAlpha->GetValueSignal())) {
		v=Exp->SfAlpha->GetValue();
		if (Exp->AlphaOut!=v) {
			Exp->AlphaOut=v;
			Color.SetAlpha((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfHue->GetValueSignal())) {
		v=Exp->SfHue->GetValue();
		if (Exp->HueOut!=v) {
			Exp->HueOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->SfSat->GetValueSignal())) {
		v=Exp->SfSat->GetValue();
		if (Exp->SatOut!=v) {
			Exp->SatOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->SfVal->GetValueSignal())) {
		v=Exp->SfVal->GetValue();
		if (Exp->ValOut!=v) {
			Exp->ValOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->TfName->GetTextSignal())) {
		str=Exp->TfName->GetText();
		if (Exp->NameOut!=str) {
			Exp->NameOut=str;
			try {
				Color.TryParse(str);
			}
			catch (emException &) {
				Color=oldColor;
			}
			Color.SetAlpha(oldColor.GetAlpha());
			textChanged=true;
		}
	}

	if (Color!=oldColor) {
		if (hsvChanged || textChanged) UpdateRGBAOutput();
		if (rgbaChanged || textChanged) UpdateHSVOutput();
		if (rgbaChanged || hsvChanged) UpdateNameOutput();
		InvalidatePainting();
		Signal(ColorSignal);
		ColorChanged();
	}

	return busy;
}


void emColorField::AutoExpand()
{
	emArray<emUInt64> percentIntervals;
	emRasterLayout * rl;
	emScalarField * sf;
	emTextField * tf;

	emBorder::AutoExpand();

	Exp=new Expansion;

	rl=new emRasterLayout(this,"emColorField::InnerStuff");
	rl->BeFirst();
	rl->SetSpace(0.08,0.2,0.04,0.1);
	rl->SetFixedColumnCount(2);
	rl->SetChildTallness(0.2);
	rl->SetAlignment(EM_ALIGN_RIGHT);
	Exp->Layout=rl;

	percentIntervals.Add(2500);
	percentIntervals.Add(500);
	percentIntervals.Add(100);

	sf=new emScalarField(
		rl,"r","Red",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfRed=sf;

	sf=new emScalarField(
		rl,"g","Green",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfGreen=sf;

	sf=new emScalarField(
		rl,"b","Blue",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfBlue=sf;

	sf=new emScalarField(
		rl,"a","Alpha",
		"The lower the more transparent."
		,emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfAlpha=sf;

	sf=new emScalarField(
		rl,"h","Hue",emString(),emImage(),0,36000,0,true
	);
	sf->SetScaleMarkIntervals(6000,1500,500,100,0);
	sf->SetTextOfValueFunc(&TextOfHueValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	sf->SetTextBoxTallness(0.35);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfHue=sf;

	sf=new emScalarField(
		rl,"s","Saturation",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfSat=sf;

	sf=new emScalarField(
		rl,"v","Value (brightness)",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfVal=sf;

	tf=new emTextField(
		rl,"n",
		"Name",
		"Here you can enter a color name like 'powder blue',\n"
		"or a hexadecimal RGB value like '#c88' or '#73c81D'.",
		emImage(),emString(),true
	);
	tf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	tf->SetBorderScaling(2.0);
	AddWakeUpSignal(tf->GetTextSignal());
	Exp->TfName=tf;

	UpdateExpAppearance();
	UpdateRGBAOutput();
	UpdateHSVOutput(true);
	UpdateNameOutput();
}


void emColorField::AutoShrink()
{
	emBorder::AutoShrink();
	delete Exp;
	Exp=NULL;
}


void emColorField::LayoutChildren()
{
	double d,x,y,w,h;

	emBorder::LayoutChildren();
	if (Exp) {
		GetContentRectUnobscured(&x,&y,&w,&h);
		d=emMin(w,h)*0.05;
		x+=d;
		y+=d;
		w-=2*d;
		h-=2*d;
		Exp->Layout->Layout(x+w*0.5,y,w*0.5,h);
	}
}


bool emColorField::HasHowTo() const
{
	return true;
}


emString emColorField::GetHowTo() const
{
	emString h;

	h=emBorder::GetHowTo();
	h+=HowToColorField;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emColorField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	double d,r;

	GetContentRoundRect(&x,&y,&w,&h,&r);
	d=emMin(w,h)*0.1;
	if (!Color.IsOpaque()) {
		painter.PaintTextBoxed(
			x+d,y+d,w-2*d,h-2*d,
			"transparent",
			h,
			Editable ?
				GetLook().GetInputFgColor()
			:
				GetLook().GetOutputFgColor(),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
		canvasColor=0;
	}
	painter.PaintRect(
		x+d,y+d,w-2*d,h-2*d,
		Color,
		canvasColor
	);
	painter.PaintRectOutline(
		x+d,y+d,w-2*d,h-2*d,d*0.08,
		GetLook().GetInputFgColor()
	);
}


void emColorField::UpdateRGBAOutput()
{
	if (!Exp) return;
	Exp->RedOut=(Color.GetRed()*10000+127)/255;
	Exp->SfRed->SetValue(Exp->RedOut);
	Exp->GreenOut=(Color.GetGreen()*10000+127)/255;
	Exp->SfGreen->SetValue(Exp->GreenOut);
	Exp->BlueOut=(Color.GetBlue()*10000+127)/255;
	Exp->SfBlue->SetValue(Exp->BlueOut);
	Exp->AlphaOut=(Color.GetAlpha()*10000+127)/255;
	Exp->SfAlpha->SetValue(Exp->AlphaOut);
}


void emColorField::UpdateHSVOutput(bool initial)
{
	float h,s,v;

	if (!Exp) return;
	h=Color.GetHue();
	s=Color.GetSat();
	v=Color.GetVal();
	if (v>0.0F || initial) {
		if (s>0.0F || initial) {
			Exp->HueOut=(emInt64)(h*100.0F+0.5F);
			Exp->SfHue->SetValue(Exp->HueOut);
		}
		Exp->SatOut=(emInt64)(s*100.0F+0.5F);
		Exp->SfSat->SetValue(Exp->SatOut);
	}
	Exp->ValOut=(emInt64)(v*100.0F+0.5F);
	Exp->SfVal->SetValue(Exp->ValOut);
}


void emColorField::UpdateNameOutput()
{
	if (!Exp) return;
	Exp->NameOut=emString::Format(
		"#%02X%02X%02X",
		(int)Color.GetRed(),
		(int)Color.GetGreen(),
		(int)Color.GetBlue()
	);
	Exp->TfName->SetText(Exp->NameOut);
}


void emColorField::UpdateExpAppearance()
{
	emLook look;
	emColor bg,fg;

	if (!Exp) return;

	look=GetLook();
	if (IsEnabled()) {
		if (Editable) {
			bg=look.GetInputBgColor();
			fg=look.GetInputFgColor();
		}
		else {
			bg=look.GetOutputBgColor();
			fg=look.GetOutputFgColor();
		}
		look.SetBgColor(bg);
		look.SetFgColor(fg);
	}
	Exp->Layout->SetLook(look,true);

	Exp->SfRed  ->SetEditable(Editable);
	Exp->SfGreen->SetEditable(Editable);
	Exp->SfBlue ->SetEditable(Editable);
	Exp->SfAlpha->SetEditable(Editable);
	Exp->SfHue  ->SetEditable(Editable);
	Exp->SfSat  ->SetEditable(Editable);
	Exp->SfVal  ->SetEditable(Editable);
	Exp->TfName ->SetEditable(Editable);

	Exp->SfAlpha->SetEnableSwitch(AlphaEnabled);
}


void emColorField::TextOfPercentValue (
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	snprintf(buf,bufSize,"%G%%",value/100.0);
}


void emColorField::TextOfHueValue (
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	const char * name;

	if (markInterval>=6000) {
		switch ((int)value) {
			case  6000: name="Yellow" ; break;
			case 12000: name="Green"  ; break;
			case 18000: name="Cyan"   ; break;
			case 24000: name="Blue"   ; break;
			case 30000: name="Magenta"; break;
			default   : name="Red"    ; break;
		}
		snprintf(buf,bufSize,"%s",name);
	}
	else {
		snprintf(
			buf,bufSize,
			emIsUtf8System() ? "%G\302\260" : "%G\260",
			value/100.0
		);
	}
}


const char * emColorField::HowToColorField=
	"\n"
	"\n"
	"COLOR FIELD\n"
	"\n"
	"This panel is for viewing and editing a color. For editing, refer to the inner\n"
	"fields.\n"
;


const char * emColorField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This color field is read-only. You cannot edit the color.\n"
;
