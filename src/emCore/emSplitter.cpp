//------------------------------------------------------------------------------
// emSplitter.cpp
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

#include <emCore/emSplitter.h>


emSplitter::emSplitter(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, bool vertical,
	double minPos, double maxPos, double pos
)
	: emBorder(parent,name,caption,description,icon)
{
	Vertical=vertical;
	if (minPos<0.0) minPos=0.0;
	if (minPos>1.0) minPos=1.0;
	if (maxPos<0.0) maxPos=0.0;
	if (maxPos>1.0) maxPos=1.0;
	if (minPos>maxPos) {
		minPos=maxPos=(minPos+maxPos)*0.5;
	}
	MinPos=minPos;
	MaxPos=maxPos;
	if (pos<MinPos) pos=MinPos;
	if (pos>MaxPos) pos=MaxPos;
	Pos=pos;
	Pressed=false;
	MousePosInGrip=0.0;
	MouseInGrip=false;
}


emSplitter::~emSplitter()
{
}


void emSplitter::SetVertical(bool vertical)
{
	if (Vertical!=vertical) {
		Vertical=vertical;
		InvalidateCursor();
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emSplitter::SetMinMaxPos(double minPos, double maxPos)
{
	if (minPos<0.0) minPos=0.0;
	if (minPos>1.0) minPos=1.0;
	if (maxPos<0.0) maxPos=0.0;
	if (maxPos>1.0) maxPos=1.0;
	if (minPos>maxPos) {
		minPos=maxPos=(minPos+maxPos)*0.5;
	}
	MinPos=minPos;
	MaxPos=maxPos;
	if (Pos<MinPos) SetPos(MinPos);
	if (Pos>MaxPos) SetPos(MaxPos);
}


void emSplitter::SetPos(double pos)
{
	if (pos<MinPos) pos=MinPos;
	if (pos>MaxPos) pos=MaxPos;
	if (Pos!=pos) {
		Pos=pos;
		Signal(PosSignal);
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emSplitter::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double cx,cy,cw,ch,gx,gy,gw,gh,mig,d;

	GetContentRectUnobscured(&cx,&cy,&cw,&ch);
	CalcGripRect(cx,cy,cw,ch,&gx,&gy,&gw,&gh);

	if (mx>=gx && my>=gy && mx<gx+gw && my<gy+gh) {
		if (!MouseInGrip) {
			MouseInGrip=true;
			InvalidateCursor();
		}
	}
	else {
		if (MouseInGrip) {
			MouseInGrip=false;
			InvalidateCursor();
		}
	}

	if (Pressed) {
		if (Vertical) {
			mig=my-gy;
			if (MousePosInGrip!=mig) {
				d=ch-gh;
				if (d>0.0001) {
					if (IsInActivePath() && !IsActive()) Activate();
					SetPos((gy-cy-MousePosInGrip+mig)/d);
				}
			}
		}
		else {
			mig=mx-gx;
			if (MousePosInGrip!=mig) {
				d=cw-gw;
				if (d>0.0001) {
					if (IsInActivePath() && !IsActive()) Activate();
					SetPos((gx-cx-MousePosInGrip+mig)/d);
				}
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			Pressed=false;
			InvalidateCursor();
			InvalidatePainting();
		}
	}
	else if (MouseInGrip && event.IsKey(EM_KEY_LEFT_BUTTON) && IsEnabled()) {
		Pressed=true;
		if (Vertical) MousePosInGrip=my-gy; else MousePosInGrip=mx-gx;
		InvalidateCursor();
		InvalidatePainting();
		Focus();
		event.Eat();
	}
	emBorder::Input(event,state,mx,my);
}


emCursor emSplitter::GetCursor() const
{
	if ((!MouseInGrip && !Pressed) || !IsEnabled()) return emBorder::GetCursor();
	else if (Vertical) return emCursor::UP_DOWN_ARROW;
	else return emCursor::LEFT_RIGHT_ARROW;
}


void emSplitter::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	double gx,gy,gw,gh,d;
	emColor btBgCol;

	CalcGripRect(x,y,w,h,&gx,&gy,&gw,&gh);
	btBgCol=GetLook().GetButtonBgColor();
	painter.PaintRect(
		gx,gy,gw,gh,
		btBgCol,
		canvasColor
	);
	d=emMin(gw,gh)*0.5;
	painter.PaintBorderImage(
		gx,gy,gw,gh,
		d,d,d,d,
		Pressed ?
			GetTkResources().ImgSplitterPressed :
			GetTkResources().ImgSplitter,
		150.0,150.0,149.0,149.0,
		IsEnabled() ? 255 : 64,
		btBgCol,
		0757
	);
}


void emSplitter::LayoutChildren()
{
	double cx,cy,cw,ch,gx,gy,gw,gh,x,y,w,h;
	emColor canvasColor;
	emPanel * p, * aux;

	emBorder::LayoutChildren();

	p=GetFirstChild();
	if (!p) return;
	aux=GetAuxPanel();
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}
	GetContentRectUnobscured(&cx,&cy,&cw,&ch,&canvasColor);
	CalcGripRect(cx,cy,cw,ch,&gx,&gy,&gw,&gh);
	if (Vertical) {
		x=cx;
		y=cy;
		w=cw;
		h=gy-cy;
	}
	else {
		x=cx;
		y=cy;
		w=gx-cx;
		h=ch;
	}
	p->Layout(x,y,w,h,canvasColor);

	p=p->GetNext();
	if (!p) return;
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}
	if (Vertical) {
		x=cx;
		y=gy+gh;
		w=cw;
		h=cy+ch-y;
	}
	else {
		x=gx+gw;
		y=cy;
		w=cx+cw-x;
		h=ch;
	}
	p->Layout(x,y,w,h,canvasColor);
}


void emSplitter::CalcGripRect(
	double contentX, double contentY, double contentW, double contentH,
	double * pX, double * pY, double * pW, double * pH
) const
{
	double gs;

	gs=0.015*GetBorderScaling();
	if (Vertical) {
		gs*=contentH;
		if (gs>contentH*0.5) gs=contentH*0.5;
		*pX=contentX;
		*pY=contentY+Pos*(contentH-gs);
		*pW=contentW;
		*pH=gs;
	}
	else {
		gs*=contentW;
		if (gs>contentW*0.5) gs=contentW*0.5;
		*pX=contentX+Pos*(contentW-gs);
		*pY=contentY;
		*pW=gs;
		*pH=contentH;
	}
}
