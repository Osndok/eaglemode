//------------------------------------------------------------------------------
// emSubViewPanel.cpp
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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

#include <emCore/emSubViewPanel.h>


emSubViewPanel::emSubViewPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
	SubView=new SubViewClass(*this);
	SubViewPort=new SubViewPortClass(*this);
}


emSubViewPanel::~emSubViewPanel()
{
	delete SubViewPort;
	delete SubView;
}


emString emSubViewPanel::GetTitle()
{
	return SubView->GetTitle();
}


double emSubViewPanel::GetTouchEventPriority(double touchX, double touchY)
{
	return SubView->GetTouchEventPriority(touchX,touchY);
}


void emSubViewPanel::Notice(NoticeFlags flags)
{
	if ((flags&NF_FOCUS_CHANGED)!=0) {
		SubViewPort->SetViewFocused(IsFocused());
	}
	if ((flags&NF_VIEWING_CHANGED)!=0) {
		if (IsViewed()) {
			SubViewPort->SetViewGeometry(
				GetViewedX(),GetViewedY(),
				GetViewedWidth(),GetViewedHeight(),
				GetViewedPixelTallness()
			);
		}
		else {
			SubViewPort->SetViewGeometry(0.0,0.0,1.0,GetHeight(),1.0);
		}
	}
}


void emSubViewPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (IsFocusable() && (event.IsMouseEvent() || event.IsTouchEvent())) {
		Focus();
		SubViewPort->SetViewFocused(IsFocused());
	}
	SubViewPort->InputToView(event,state);
	emPanel::Input(event,state,mx,my);
}


emCursor emSubViewPanel::GetCursor()
{
	return SubViewPort->GetViewCursor();
}


bool emSubViewPanel::IsOpaque()
{
	return true;
}


void emSubViewPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	SubViewPort->PaintView(
		emPainter(
			painter,
			painter.GetClipX1(),
			painter.GetClipY1(),
			painter.GetClipX2(),
			painter.GetClipY2(),
			painter.GetOriginX()-GetViewedX(),
			painter.GetOriginY()-GetViewedY(),
			1.0,
			1.0
		),
		canvasColor
	);
}


emSubViewPanel::SubViewClass::SubViewClass(emSubViewPanel & superPanel)
	: emView((emContext&)superPanel.GetView()),
	SuperPanel(superPanel)
{
}


emSubViewPanel::SubViewClass::~SubViewClass()
{
	if (GetRootPanel()) delete GetRootPanel();
}


void emSubViewPanel::SubViewClass::InvalidateTitle()
{
	emView::InvalidateTitle();
	SuperPanel.InvalidateTitle();
}


emSubViewPanel::SubViewPortClass::SubViewPortClass(emSubViewPanel & superPanel)
	: emViewPort(*superPanel.SubView),
	SuperPanel(superPanel)
{
}


void emSubViewPanel::SubViewPortClass::RequestFocus()
{
	SuperPanel.Focus();
	SetViewFocused(SuperPanel.IsFocused());
}


bool emSubViewPanel::SubViewPortClass::IsSoftKeyboardShown()
{
	return SuperPanel.GetView().IsSoftKeyboardShown();
}


void emSubViewPanel::SubViewPortClass::ShowSoftKeyboard(bool show)
{
	SuperPanel.GetView().ShowSoftKeyboard(show);
}

emUInt64 emSubViewPanel::SubViewPortClass::GetInputClockMS()
{
	return SuperPanel.GetInputClockMS();
}


void emSubViewPanel::SubViewPortClass::InvalidateCursor()
{
	SuperPanel.InvalidateCursor();
}


void emSubViewPanel::SubViewPortClass::InvalidatePainting(
	double x, double y, double w, double h
)
{
	if (SuperPanel.IsViewed()) {
		SuperPanel.InvalidatePaintingOnView(x,y,w,h);
	}

/*
	Here is the original version. It is slower, but it does not need the
	method emSubViewPanel::InvalidatePaintingOnView and the friend
	declaration for emSubViewPanel in emView. The above fast version
	possibly would not be correct if emSubViewPanel::IsOpaque would ever be
	changed so that it performs some forwarding from the root panel of this
	view.

	double fx,fy;

	if (SuperPanel.IsViewed()) {
		fx=1.0/SuperPanel.GetViewedWidth();
		fy=fx*SuperPanel.GetViewedPixelTallness();
		SuperPanel.InvalidatePainting(
			(x-SuperPanel.GetViewedX())*fx,
			(y-SuperPanel.GetViewedY())*fy,
			w*fx,
			h*fy
		);
	}
*/
}
