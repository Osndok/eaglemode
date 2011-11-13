//------------------------------------------------------------------------------
// emMainPanel.cpp
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emCore/emToolkit.h>
#include <emMain/emMainPanel.h>


emMainPanel::emMainPanel(
	ParentArg parent, const emString & name, double controlTallness
)
	: emPanel(parent, name),
	SliderTimer(GetScheduler())
{
	MainConfig=emMainConfig::Acquire(GetRootContext());

	ControlEdgesColor=emTkLook().GetBgColor();

	ControlEdgesImage=emGetInsResImage(GetRootContext(),"emMain","ControlEdges.tga");

	ControlViewPanel=new emSubViewPanel(this,"control view");
	ContentViewPanel=new emSubViewPanel(this,"content view");
	Slider=new SliderPanel(*this,"slider");

	GetControlView().SetViewFlags(
		emView::VF_POPUP_ZOOM          |
		emView::VF_ROOT_SAME_TALLNESS  |
		emView::VF_NO_ACTIVE_HIGHLIGHT
	);

	GetControlView().SetBackgroundColor(emTkLook().GetBgColor());

	GetContentView().SetViewFlags(
		emView::VF_ROOT_SAME_TALLNESS
	);

	ControlTallness=controlTallness;

	UnifiedSliderPos=MainConfig->ControlViewSize;

	FullscreenOn=false;

	OldMouseX=0.0;
	OldMouseY=0.0;

	ContentViewPanel->ActivateLater();

	AddWakeUpSignal(GetControlView().GetEOISignal());
	AddWakeUpSignal(SliderTimer.GetSignal());
	if (GetWindow()) AddWakeUpSignal(GetWindow()->GetWindowFlagsSignal());

	UpdateCoordinates();
	UpdateFullscreen();
	UpdateSliderHiding(false);
}


emMainPanel::~emMainPanel()
{
}


void emMainPanel::SetControlEdgesColor(emColor controlEdgesColor)
{
	controlEdgesColor.SetAlpha(255);
	if (ControlEdgesColor!=controlEdgesColor) {
		ControlEdgesColor=controlEdgesColor;
		InvalidatePainting();
	}
}


bool emMainPanel::Cycle()
{
	bool busy;

	busy=emPanel::Cycle();

	if (IsSignaled(GetControlView().GetEOISignal())) {
		GetControlView().ZoomOut();
		ContentViewPanel->Activate();
	}

	if (IsSignaled(SliderTimer.GetSignal())) {
		Slider->SetHidden(true);
	}

	if (GetWindow() && IsSignaled(GetWindow()->GetWindowFlagsSignal())) {
		UpdateFullscreen();
		UpdateSliderHiding(false);
	}

	return busy;
}


void emMainPanel::Notice(NoticeFlags flags)
{
	if (flags&NF_LAYOUT_CHANGED) {
		UpdateCoordinates();
	}
	emPanel::Notice(flags);
}


void emMainPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (
		fabs(OldMouseX-state.GetMouseX())>2.5 ||
		fabs(OldMouseY-state.GetMouseY())>2.5 ||
		state.GetLeftButton() ||
		state.GetMiddleButton() ||
		state.GetRightButton()
	) {
		OldMouseX=state.GetMouseX();
		OldMouseY=state.GetMouseY();
		UpdateSliderHiding(true);
	}
}


bool emMainPanel::IsOpaque()
{
	return true;
}


void emMainPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double x1,y1,w1,h1,x2,y2,w2,h2,x,y,w,h,d;

	if (ContentY<=1E-10) return;

	d=ControlH*0.007;
	x1=0.0;
	y1=0.0;
	w1=ControlX-d;
	h1=ControlH;
	x2=ControlX+ControlW+d;
	y2=0.0;
	w2=1.0-x2;
	h2=ControlH;

	x=0.0;
	y=painter.RoundDownY(ControlH);
	w=1.0;
	h=painter.RoundUpY(ContentY)-y;
	painter.PaintRect(x,y,w,h,0x000000ff);

	d=ControlH*0.015;
	if (ControlX>1E-10) {
		x=painter.RoundDownX(x1+w1);
		y=0.0;
		w=painter.RoundUpX(ControlX)-x;
		h=painter.RoundUpY(ContentY);
		painter.PaintRect(x,y,w,h,0x000000ff);
		painter.PaintRect(x1,y1,w1,h1,ControlEdgesColor);
		painter.PaintBorderImage(
			x1,y1,w1,h1,
			0.0,d,d,d,
			ControlEdgesImage,
			191.0,0.0,190.0,11.0,
			0.0,5.0,5.0,5.0,
			255,ControlEdgesColor,057
		);
	}
	if (1.0-ControlX-ControlW>1E-10) {
		x=painter.RoundDownX(ControlX+ControlW);
		y=0.0;
		w=painter.RoundUpX(x2)-x;
		h=painter.RoundUpY(ContentY);
		painter.PaintRect(x,y,w,h,0x000000ff);
		painter.PaintRect(x2,y2,w2,h2,ControlEdgesColor);
		painter.PaintBorderImage(
			x2,y2,w2,h2,
			d,d,0.0,d,
			ControlEdgesImage,
			0.0,0.0,190.0,11.0,
			5.0,5.0,0.0,5.0,
			255,ControlEdgesColor,0750
		);
	}
}


void emMainPanel::LayoutChildren()
{
	ControlViewPanel->Layout(ControlX,ControlY,ControlW,ControlH);
	ContentViewPanel->Layout(ContentX,ContentY,ContentW,ContentH);
	Slider->Layout(SliderX,SliderY,SliderW,SliderH);
}


void emMainPanel::UpdateCoordinates()
{
	double h,t,spaceFac;

	h=GetHeight();

	SliderMinY=0.0;
	SliderMaxY=emMin(ControlTallness,h*0.5);
	SliderY=(SliderMaxY-SliderMinY)*UnifiedSliderPos+SliderMinY;
	SliderW=emMin(emMin(1.0,h)*0.1,emMax(1.0,h)*0.02);
	SliderH=SliderW*1.2;
	SliderX=1.0-SliderW;

	spaceFac=1.015;
	t=SliderH*0.5;
	if (SliderY<t) {
		ControlH=SliderY+SliderH*SliderY/t;
	}
	else {
		ControlH=(SliderY+SliderH)/spaceFac;
	}
	if (ControlH<1E-5) {
		ControlH=1E-5;
		ControlW=ControlH/ControlTallness;
		ControlX=0.5*(1.0-ControlW);
		ControlY=0.0; // Do not hide, because otherwise popping up the
		              // control view by keyboard would not work
		              // properly.
		ContentX=0.0;
		ContentY=0.0;
		ContentW=1.0;
		ContentH=h;
	}
	else {
		ControlW=ControlH/ControlTallness;
		ControlX=emMin((1.0-ControlW)*0.5,SliderX-ControlW);
		ControlY=0.0;
		if (ControlX<1E-5) {
			ControlW=1.0-SliderW;
			ControlX=0.0;
			ControlH=ControlW*ControlTallness;
			if (ControlH<SliderY) {
				ControlH=SliderY;
				ControlW=ControlH/ControlTallness;
			}
			else {
				if (!Slider->Pressed) {
					SliderY=ControlH*spaceFac-SliderH;
				}
			}
		}
		ContentY=ControlY+ControlH*spaceFac;
		ContentX=0.0;
		ContentW=1.0;
		ContentH=h-ContentY;
	}

	InvalidatePainting();
	InvalidateChildrenLayout();
}


void emMainPanel::UpdateFullscreen()
{
	if (!GetWindow()) return;
	if (GetWindow()->GetWindowFlags()&emWindow::WF_FULLSCREEN) {
		if (!FullscreenOn) {
			FullscreenOn=true;
			if (MainConfig->AutoHideControlView) {
				UnifiedSliderPos=0.0;
				UpdateCoordinates();
				UpdateSliderHiding(false);
			}
		}
	}
	else {
		if (FullscreenOn) {
			FullscreenOn=false;
			if (MainConfig->AutoHideControlView) {
				UnifiedSliderPos=MainConfig->ControlViewSize;
				UpdateCoordinates();
				UpdateSliderHiding(false);
			}
		}
	}
}


void emMainPanel::UpdateSliderHiding(bool restart)
{
	bool toHide;

	toHide=
		UnifiedSliderPos<1E-15 &&
		GetWindow()!=NULL &&
		(GetWindow()->GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0 &&
		MainConfig->AutoHideSlider
	;
	if (!toHide || restart) {
		if (Slider->Hidden) Slider->SetHidden(false);
		SliderTimer.Stop(true);
	}
	if (toHide && !Slider->Hidden && !SliderTimer.IsRunning()) {
		SliderTimer.Start(5000);
	}
}


void emMainPanel::DragSlider(double deltaY)
{
	double y,n;

	y=SliderY+deltaY;
	if (y<=SliderMinY) y=SliderMinY;
	else if (y>SliderMaxY) y=SliderMaxY;
	n=(y-SliderMinY)/(SliderMaxY-SliderMinY);
	if (UnifiedSliderPos!=n) {
		UnifiedSliderPos=n;
		UpdateCoordinates();
		UpdateSliderHiding(false);
		MainConfig->ControlViewSize=UnifiedSliderPos;
		MainConfig->Save();
	}
}


void emMainPanel::DoubleClickSlider()
{
	if (UnifiedSliderPos<0.01) {
		if (MainConfig->ControlViewSize<0.01) {
			MainConfig->ControlViewSize=0.7;
			MainConfig->Save();
		}
		UnifiedSliderPos=MainConfig->ControlViewSize;
	}
	else {
		UnifiedSliderPos=0.0;
	}
	UpdateCoordinates();
	UpdateSliderHiding(false);
}


emMainPanel::SliderPanel::SliderPanel(emMainPanel & parent, const emString & name)
	: emPanel(parent,name),
	MainPanel(parent)
{
	MouseOver=false;
	Pressed=false;
	Hidden=false;
	PressMY=0.0;
	PressSliderY=0.0;
	SetFocusable(false);
	SliderImage=emGetInsResImage(GetRootContext(),"emMain","Slider.tga");
}


emMainPanel::SliderPanel::~SliderPanel()
{
}


void emMainPanel::SliderPanel::SetHidden(bool hidden)
{
	if (Hidden!=hidden) {
		Hidden=hidden;
		InvalidatePainting();
	}
}


void emMainPanel::SliderPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double dy;
	bool mo;

	mo = (mx>0.05 && my>0.0 && mx<1.0 && my<GetHeight()-0.05);
	if (MouseOver!=mo) {
		MouseOver=mo;
		InvalidateCursor();
		InvalidatePainting();
	}

	if (MouseOver && event.IsMouseEvent()) {
		if (event.IsKey(EM_KEY_LEFT_BUTTON)) {
			if (event.GetRepeat()==0 && !Pressed) {
				Pressed=true;
				PressMY=my;
				PressSliderY=MainPanel.SliderY;
				InvalidatePainting();
			}
			else if (event.GetRepeat()==1) {
				if (Pressed) {
					Pressed=false;
					MainPanel.UpdateCoordinates();
					InvalidatePainting();
				}
				MainPanel.DoubleClickSlider();
			}
		}
		event.Eat();
	}

	if (Pressed) {
		dy=(my-PressMY)*GetLayoutWidth();
		if (state.Get(EM_KEY_SHIFT)) {
			dy=(dy+MainPanel.SliderY-PressSliderY)*0.25+PressSliderY-MainPanel.SliderY;
		}
		MainPanel.DragSlider(dy);
	}

	if (Pressed && !state.Get(EM_KEY_LEFT_BUTTON)) {
		Pressed=false;
		MainPanel.UpdateCoordinates();
		InvalidatePainting();
	}
}


emCursor emMainPanel::SliderPanel::GetCursor()
{
	//if (MouseOver) return emCursor::UP_DOWN_ARROW;
	return emPanel::GetCursor();
}


void emMainPanel::SliderPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double xy[10*2];
	double h,x1,y1,x2,y2;
	int i;

	if (!MouseOver && Hidden) return;
	h=GetHeight();
	painter.PaintRoundRect(
		0.0,0.0,2.0,h,
		6.0/64.0,
		6/75.0*h,
		Pressed ? 0x002244C0 : MouseOver ? 0x006688A0 : 0x33445580
	);
	if (MouseOver || Pressed) {
		x1=0.2;
		x2=0.4;
		y1=0.1*h;
		y2=0.3*h;
		i=0;
		if (MainPanel.SliderY>MainPanel.SliderMinY+1E-5) {
			xy[i++]=x1;     xy[i++]=y2;
			xy[i++]=0.5;    xy[i++]=y1;
			xy[i++]=1.0-x1; xy[i++]=y2;
		}
		xy[i++]=1.0-x2; xy[i++]=y2;
		xy[i++]=1.0-x2; xy[i++]=h-y2;
		if (MainPanel.SliderY<MainPanel.SliderMaxY-1E-5) {
			xy[i++]=1.0-x1; xy[i++]=h-y2;
			xy[i++]=0.5;    xy[i++]=h-y1;
			xy[i++]=x1;     xy[i++]=h-y2;
		}
		xy[i++]=x2; xy[i++]=h-y2;
		xy[i++]=x2; xy[i++]=y2;
		painter.PaintPolygon(xy,i/2,Pressed ? 0xEEDD99D0 : 0xEEDD9960);
	}

	painter.PaintImage(0.0,0.0,1.0,h,SliderImage);
}
