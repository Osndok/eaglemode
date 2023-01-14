//------------------------------------------------------------------------------
// emButton.cpp
//
// Copyright (C) 2005-2011,2014-2016,2019-2022 Oliver Hamann.
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

#include <emCore/emButton.h>


emButton::emButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	Pressed=false;
	BoxPressed=false;
	NoEOI=false;
	ShownChecked=false;
	ShownBoxed=false;
	ShownRadioed=false;
	SetOuterBorderType(OBT_INSTRUMENT_MORE_ROUND);
	SetLabelInBorder(false);
	SetLabelAlignment(EM_ALIGN_CENTER);
}


emButton::~emButton()
{
}


void emButton::SetNoEOI(bool noEOI)
{
	NoEOI=noEOI;
}


void emButton::Click(bool shift)
{
	if (IsEnabled()) {
		if (!shift && !NoEOI) GetView().SignalEOIDelayed();
		Signal(ClickSignal);
		Clicked();
	}
}


void emButton::Clicked()
{
}


void emButton::PressStateChanged()
{
}


void emButton::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	static const double minExt=7.0;
	double vmx,vmy;
	bool inBox;

	if (
		event.IsLeftButton() && (state.IsNoMod() || state.IsShiftMod()) &&
		IsEnabled() && CheckMouse(mx,my,&inBox) &&
		GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		Focus();
		event.Eat();
		Pressed=true;
		BoxPressed=inBox;
		InvalidatePainting();
		Signal(PressStateSignal);
		PressStateChanged();
	}

	if (Pressed && !state.GetLeftButton()) {
		Pressed=false;
		BoxPressed=false;
		InvalidatePainting();
		Signal(PressStateSignal);
		PressStateChanged();
		if (CheckMouse(mx,my) && IsEnabled() && IsViewed()) {
			vmx=PanelToViewX(mx);
			vmy=PanelToViewY(my);
			if (
				vmx>=GetClipX1() && vmx<GetClipX2() &&
				vmy>=GetClipY1() && vmy<GetClipY2()
			) {
				Click(state.IsShiftMod());
			}
		}
	}

	if (
		event.IsKey(EM_KEY_ENTER) &&
		(state.IsNoMod() || state.IsShiftMod()) &&
		IsActive() && IsEnabled() && GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		event.Eat();
		Click(state.IsShiftMod());
	}

	emPanel::Input(event,state,mx,my);
}


bool emButton::HasHowTo() const
{
	return true;
}


emString emButton::GetHowTo() const
{
	emString h;

	h=emBorder::GetHowTo();
	h+=HowToButton;
	if (!NoEOI) h+=HowToEOIButton;
	return h;
}


void emButton::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	DoButton(BUTTON_FUNC_PAINT,&painter,canvasColor,0,0,NULL,NULL);
}


void emButton::PaintBoxSymbol(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	double xy[3*2];
	double d;

	if (ShownChecked) {
		if (ShownRadioed) {
			d=w*0.25;
			painter.PaintEllipse(
				x+d,y+d,w-2*d,h-2*d,
				GetLook().GetInputFgColor(),
				canvasColor
			);
		}
		else {
			xy[0]=x+w*0.2;
			xy[1]=y+h*0.6;
			xy[2]=x+w*0.4;
			xy[3]=y+h*0.8;
			xy[4]=x+w*0.8;
			xy[5]=y+h*0.2;
			painter.PaintPolyline(
				xy,3,
				w*0.16,
				emRoundedStroke(GetLook().GetInputFgColor()),
				emStrokeEnd::CAP,emStrokeEnd::CAP,
				canvasColor
			);
		}
	}
}


bool emButton::CheckMouse(double mx, double my, bool * pInBox) const
{
	bool b;

	DoButton(BUTTON_FUNC_CHECK_MOUSE,NULL,0,mx,my,&b,pInBox);
	return b;
}


void emButton::SetShownChecked(bool shownChecked)
{
	if (((bool)ShownChecked)!=shownChecked) {
		ShownChecked=shownChecked;
		InvalidatePainting();
	}
}


void emButton::SetShownBoxed(bool shownBoxed)
{
	if (((bool)ShownBoxed)!=shownBoxed) {
		ShownBoxed=shownBoxed;
		InvalidatePainting();
	}
}


void emButton::SetShownRadioed(bool shownRadioed)
{
	if (((bool)ShownRadioed)!=shownRadioed) {
		ShownRadioed=shownRadioed;
		InvalidatePainting();
	}
}


void emButton::DoButton(
	DoButtonFunc func, const emPainter * painter, emColor canvasColor,
	double mx, double my, bool * pHit, bool * pInBox
) const
{
	double x,y,w,h,r,d,f,bx,by,bw,bh,fx,fy,fw,fh,fr,lx,ly,lw,lh,dx,dy;
	bool hasLabel;
	emColor color;

	if (ShownBoxed) {

		GetContentRect(&x,&y,&w,&h);

		hasLabel=HasLabel();
		if (hasLabel) {
			lw=1.0;
			lh=emMax(0.2,GetBestLabelTallness());
			bw=lh;
			d=bw*0.1;
			f=emMin(w/(bw+d+lw),h/lh);
			bw*=f;
			d*=f;
			lw=w-bw-d;
			lh=bw;
			lx=x+w-lw;
			ly=y+(h-lh)*0.5;
		}
		else {
			bw=emMin(w,h);
			lx=x;
			ly=y;
			lw=1E-100;
			lh=1E-100;
		}
		bh=bw;
		bx=x;
		by=y+(h-bh)*0.5;

		d=bw*0.13;
		bx+=d;
		by+=d;
		bw-=2*d;
		bh-=2*d;

		d=bw*30.0/380;
		fx=bx+d;
		fy=by+d;
		fw=bw-2*d;
		fh=bh-2*d;
		if (ShownRadioed) fr=fw*0.5;
		else fr=bw*50.0/380;

		r=h*0.2;

		if (func==BUTTON_FUNC_CHECK_MOUSE) {
			dx=emMax(emMax(x-mx,mx-x-w)+r,0.0);
			dy=emMax(emMax(y-my,my-y-h)+r,0.0);
			*pHit = dx*dx+dy*dy <= r*r;
			if (pInBox) {
				dx=emMax(emMax(fx-mx,mx-fx-fw)+fr,0.0);
				dy=emMax(emMax(fy-my,my-fy-fh)+fr,0.0);
				*pInBox = dx*dx+dy*dy <= fr*fr;
			}
			return;
		}

		color=GetLook().GetFgColor();
		if (!IsEnabled()) color=color.GetTransparented(75.0F);

		if (hasLabel) {
			if (Pressed && !BoxPressed) {
				bx+=lw*0.003;
				fx+=lw*0.003;
				lx+=lw*0.003;
				ly+=lh*0.007;
				lw*=0.986;
				lh*=0.986;
			}
			PaintLabel(
				*painter,
				lx,ly,lw,lh,
				color,
				canvasColor
			);
		}

		color=GetLook().GetInputBgColor();
		painter->PaintRoundRect(fx,fy,fw,fh,fr,fr,color,canvasColor);
		canvasColor=color;

		PaintBoxSymbol(*painter,fx,fy,fw,fh,canvasColor);

		if (!IsEnabled()) painter->PaintRoundRect(fx,fy,fw,fh,fr,fr,0x888888E0);

		painter->PaintImage(
			bx,by,bw,bh,
			ShownRadioed ? (
				BoxPressed ?
					GetTkResources().ImgRadioBoxPressed :
					GetTkResources().ImgRadioBox
			)
			: (
				BoxPressed ?
					GetTkResources().ImgCheckBoxPressed :
					GetTkResources().ImgCheckBox
			),
			255
		);

		if (Pressed && !BoxPressed) {
			painter->PaintBorderImage(
				x,y,w,h,r,r,r,r,
				GetTkResources().ImgGroupInnerBorder,
				225,225,225,225,
				255,0,0757
			);
		}

	}
	else {

		GetContentRoundRect(&x,&y,&w,&h,&r);
		r=emMax(r,emMin(w,h)*GetBorderScaling()*0.223);

		d=(1-(264.0-14.0)/264.0)*r;
		fx=x+d;
		fy=y+d;
		fw=w-2*d;
		fh=h-2*d;
		fr=r-d;
		if (func==BUTTON_FUNC_CHECK_MOUSE) {
			dx=emMax(emMax(fx-mx,mx-fx-fw)+fr,0.0);
			dy=emMax(emMax(fy-my,my-fy-fh)+fr,0.0);
			*pHit = dx*dx+dy*dy <= fr*fr;
			if (pInBox) *pInBox=false;
			return;
		}
		color=GetLook().GetButtonBgColor();
		painter->PaintRoundRect(
			fx,fy,fw,fh,
			fr,fr,
			color,
			canvasColor
		);
		canvasColor=color;

		d=emMin(fw,fh)*0.1;
		dx=emMax(r*0.7,d);
		dy=emMax(r*0.4,d);
		lx=fx+dx;
		ly=fy+dy;
		lw=fw-2*dx;
		lh=fh-2*dy;
		if (Pressed || ShownChecked) {
			d=Pressed ? 0.98 : 0.983;
			lx+=(1-d)*0.5*lw;
			lw*=d;
			ly+=(1-d)*0.5*lh;
			lh*=d;
		}
		color=GetLook().GetButtonFgColor();
		if (!IsEnabled()) color=color.GetTransparented(75.0F);
		PaintLabel(
			*painter,
			lx,ly,lw,lh,
			color,
			canvasColor
		);

		if (Pressed) {
			painter->PaintBorderImage(
				x,y,w,h,
				360.0/264*r,374.0/264*r,264.0/264*r,264.0/264*r,
				GetTkResources().ImgButtonPressed,
				360,374,264,264,
				255,0,0757
			);
		}
		else if (ShownChecked) {
			painter->PaintBorderImage(
				x,y,w,h,
				340.0/264*r,374.0/264*r,264.0/264*r,264.0/264*r,
				GetTkResources().ImgButtonChecked,
				340,374,264,264,
				255,0,0757
			);
		}
		else {
			painter->PaintBorderImage(
				x,y,
				w+(658.0-648.0)/264*r,
				h+(658.0-648.0)/264*r,
				278.0/264*r,278.0/264*r,278.0/264*r,278.0/264*r,
				GetTkResources().ImgButton,
				278,278,278,278,
				255,0,0757
			);
		}
	}
}


const char * const emButton::HowToButton=
	"\n"
	"\n"
	"BUTTON\n"
	"\n"
	"This is a button. Buttons can be triggered to perform an application defined\n"
	"function.\n"
	"\n"
	"In order to trigger a button, move the mouse pointer over the button and click\n"
	"the left mouse button. The function is triggered when releasing the mouse\n"
	"button, but only if the mouse pointer is still over the button.\n"
	"\n"
	"Alternatively, a button can be triggered by giving it the focus and pressing the\n"
	"Enter key.\n"
;


const char * const emButton::HowToEOIButton=
	"\n"
	"\n"
	"EOI BUTTON\n"
	"\n"
	"This is an End Of Interaction button. The exact behavior is application defined,\n"
	"but it usually means that if the button is in a view that has popped up, the\n"
	"view is popped down automatically when the button is triggered. If you want to\n"
	"bypass that, hold the Shift key while triggering the button.\n"
;
