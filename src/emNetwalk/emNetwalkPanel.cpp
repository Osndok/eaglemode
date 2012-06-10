//------------------------------------------------------------------------------
// emNetwalkPanel.cpp
//
// Copyright (C) 2010-2012 Oliver Hamann.
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

#include <emCore/emRes.h>
#include <emNetwalk/emNetwalkPanel.h>
#include <emNetwalk/emNetwalkControlPanel.h>


emNetwalkPanel::emNetwalkPanel(
	ParentArg parent, const emString & name, emNetwalkModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	Mdl=fileModel;
	HaveControlPanel=IsVFSGood();
	Scrolling=false;
	PrepareTransformation();
	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
}


emNetwalkPanel::~emNetwalkPanel()
{
}


emString emNetwalkPanel::GetTitle()
{
	return "Netwalk";
}


void emNetwalkPanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	*pX=EssenceX;
	*pY=EssenceY;
	*pW=EssenceW;
	*pH=EssenceH;
}


bool emNetwalkPanel::Cycle()
{
	bool vfsGood;

	if (IsSignaled(GetVirFileStateSignal())) {
		vfsGood=IsVFSGood();
		if (HaveControlPanel!=vfsGood) {
			HaveControlPanel=vfsGood;
			InvalidateControlPanel();
		}
		if (!vfsGood && Scrolling) {
			Scrolling=false;
			InvalidateCursor();
		}
	}

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		IsSignaled(Mdl->GetChangeSignal())
	) {
		PrepareTransformation();
		InvalidatePainting();
	}

	return emFilePanel::Cycle();
}


void emNetwalkPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if (flags&NF_LAYOUT_CHANGED) {
		PrepareTransformation();
		InvalidatePainting();
	}
}


void emNetwalkPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	int x,y;

	if (!IsViewed() || !IsVFSGood()) {
		emFilePanel::Input(event,state,mx,my);
		return;
	}

	if (Scrolling) {
		if (!state.Get(EM_KEY_LEFT_BUTTON) || !Mdl->IsBorderless()) {
			Scrolling=false;
			InvalidateCursor();
		}
		else {
			x=(int)floor((mx-ScrollX0)/DX+0.5);
			y=(int)floor((my-ScrollY0)/DY+0.5);
			if (x || y) {
				Mdl->Scroll(x,y);
				ScrollX0+=x*DX;
				ScrollY0+=y*DY;
			}
		}
	}
	else if (event.IsLeftButton() && state.IsCtrlMod() && Mdl->IsBorderless()) {
		Scrolling=true;
		ScrollX0=mx;
		ScrollY0=my;
		InvalidateCursor();
	}

	if (!event.IsEmpty()) {
		x=(int)floor((mx-X0)/DX);
		y=(int)floor((my-Y0)/DY);
		if (Mdl->IsBorderless()) {
			if (x==-1) x=Mdl->GetWidth()-1;
			if (y==-1) y=Mdl->GetHeight()-1;
			if (x==Mdl->GetWidth()) x=0;
			if (y==Mdl->GetHeight()) y=0;
		}
		if (x>=0 && y>=0 && x<Mdl->GetWidth() && y<Mdl->GetHeight()) {
			if (event.IsLeftButton() && !Scrolling) {
				if (state.IsNoMod()) Mdl->RotateLeft(x,y);
				else if (state.IsShiftMod()) Mdl->RotateRight(x,y);
			}
			if (event.IsRightButton() && state.IsNoMod()) {
				Mdl->MarkOrUnmark(x,y);
			}
		}
	}

	if (event.IsKey(EM_KEY_N) && state.IsCtrlMod()) {
		try {
			Mdl->TrySetup(
				Mdl->GetWidth(),Mdl->GetHeight(),Mdl->IsBorderless(),
				Mdl->IsNoFourWayJunctions(),Mdl->GetComplexity(),
				Mdl->IsDigMode(),Mdl->IsAutoMark()
			);
		}
		catch (emString errorMessage) {
			emTkDialog::ShowMessage(GetViewContext(),"Error",errorMessage);
		}
		event.Eat();
	}

	if (event.IsKey(EM_KEY_U) && state.IsCtrlMod()) {
		Mdl->UnmarkAll();
		event.Eat();
	}

	emFilePanel::Input(event,state,mx,my);
}


emCursor emNetwalkPanel::GetCursor()
{
	if (Scrolling) return emCursor::LEFT_RIGHT_UP_DOWN_ARROW;
	else return emFilePanel::GetCursor();
}


bool emNetwalkPanel::IsOpaque()
{
	if (IsVFSGood()) return true;
	else return emFilePanel::IsOpaque();
}


void emNetwalkPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	int x,y,x1,y1,x2,y2,w,h,d;
	double fx1,fy1,fx2,fy2,gx1,gy1,gx2,gy2,bx,by;
	emString str;
	double t;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	if (ImgSymbols.IsEmpty()) {
		try {
			ImgBackground=emTryGetInsResImage(GetRootContext(),"emNetwalk","Background.tga");
			ImgBorder    =emTryGetInsResImage(GetRootContext(),"emNetwalk","Border.tga");
			ImgLights    =emTryGetInsResImage(GetRootContext(),"emNetwalk","Lights.tga");
			ImgMarks     =emTryGetInsResImage(GetRootContext(),"emNetwalk","Marks.tga");
			ImgNoBorder  =emTryGetInsResImage(GetRootContext(),"emNetwalk","NoBorder.tga");
			ImgPipes     =emTryGetInsResImage(GetRootContext(),"emNetwalk","Pipes.tga");
			ImgSymbols   =emTryGetInsResImage(GetRootContext(),"emNetwalk","Symbols.tga");
		}
		catch (emString errorMessage) {
			SetCustomError(errorMessage);
			return;
		}
	}

	painter.Clear(BgColor,canvasColor);
	canvasColor=BgColor;

	w=Mdl->GetWidth();
	h=Mdl->GetHeight();

	x1=0; y1=0; x2=w; y2=h;
	if (Mdl->IsBorderless()) { x1--; y1--; x2++; y2++; }
	for (y=y1; y<y2; y++) {
		for (x=x1; x<x2; x++) {
			PaintPieceBackground(painter,X0+x*DX,Y0+y*DY,DX,DY,x,y,canvasColor);
		}
	}
	for (y=y1; y<y2; y++) {
		for (x=x1; x<x2; x++) {
			PaintPiecePipe(painter,X0+x*DX,Y0+y*DY,DX,DY,x,y);
		}
	}

	if (Mdl->IsBorderless()) {
		fx1=X0-DX;
		fy1=Y0-DY;
		fx2=X0+DX*(w+1);
		fy2=Y0+DY*(h+1);
		d=(ImgNoBorder.GetWidth()-1)/2;
		painter.PaintBorderImage(
			fx1,fy1,fx2-fx1,fy2-fy1,
			DX,DY,DX,DY,
			ImgNoBorder,
			d,d,d,d,
			255,0,0757
		);
		gx1=painter.RoundUpX(fx1);
		gy1=painter.RoundUpY(fy1);
		gx2=painter.RoundDownX(fx2);
		gy2=painter.RoundDownY(fy2);
		fx1=painter.RoundDownX(fx1);
		fy1=painter.RoundDownY(fy1);
		fx2=painter.RoundUpX(fx2);
		fy2=painter.RoundUpY(fy2);
		painter.PaintRect(fx1,fy1,fx2-fx1,gy1-fy1,BgColor);
		painter.PaintRect(fx1,gy1,gx1-fx1,gy2-gy1,BgColor);
		painter.PaintRect(gx2,gy1,fx2-gx2,gy2-gy1,BgColor);
		painter.PaintRect(fx1,gy2,fx2-fx1,fy2-gy2,BgColor);
	}
	else {
		d=(ImgBorder.GetWidth()-1)/2;
		bx=DX*BorderSize;
		by=DY*BorderSize;
		painter.PaintBorderImage(
			X0-bx,Y0-by,DX*w+2*bx,DY*h+2*by,
			bx,by,bx,by,
			ImgBorder,
			d,d,d,d,
			255,0,0757
		);
	}

	if (Mdl->IsFinished()) {
		str=emString::Format(
			"Net complete!\n"
			"Penalty: %d",
			Mdl->GetPenaltyPoints()
		);
		t=EssenceW*0.003;
		painter.PaintTextBoxed(
			EssenceX+t,EssenceY+t,EssenceW,EssenceH,
			str,
			EssenceW/12,
			0x444400FF,0,EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
		painter.PaintTextBoxed(
			EssenceX,EssenceY,EssenceW,EssenceH,
			str,
			EssenceW/12,
			0xFFFF88FF,0,EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
	}
}


emPanel * emNetwalkPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (HaveControlPanel) {
		return new emNetwalkControlPanel(parent,name,GetView(),Mdl);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emNetwalkPanel::PaintPieceBackground(
	const emPainter & painter, double x, double y, double w, double h,
	int px, int py, emColor canvasColor
)
{
	int piece,west,north,northwest,tileSize,tx,ty;
	double w2,h2;

	piece=Mdl->GetPiece(px,py);
	west=Mdl->GetPiece(px-1,py);
	north=Mdl->GetPiece(px,py-1);
	northwest=Mdl->GetPiece(px-1,py-1);

	tileSize=ImgBackground.GetWidth()/8;

	if (piece&PF_BLOCKED) {
		tileSize*=2;
		if (west&PF_EAST) {
			if (north&PF_SOUTH) { tx=3; ty=0; }
			else { tx=2; ty=0; }
		}
		else {
			if (north&PF_SOUTH) { tx=1; ty=0; }
			else { tx=0; ty=0; }
		}
		painter.PaintImage(
			x,y,w,h,
			ImgBackground,
			tx*tileSize,ty*tileSize,tileSize,tileSize,
			255,canvasColor
		);
		return;
	}

	w2=w*0.5;
	h2=h*0.5;

	if (north&PF_BLOCKED) {
		if (west&PF_BLOCKED) { tx=6; ty=2; }
		else if (northwest&PF_BLOCKED) { tx=4; ty=2; }
		else { tx=2; ty=2; }
	}
	else if (northwest&PF_BLOCKED) {
		if (west&PF_BLOCKED) { tx=5; ty=2; }
		else { tx=1; ty=2; }
	}
	else {
		if (west&PF_BLOCKED) { tx=3; ty=2; }
		else { tx=0; ty=2; }
	}
	painter.PaintImage(
		x,y,w2,h2,
		ImgBackground,
		tx*tileSize,ty*tileSize,tileSize,tileSize,
		255,canvasColor
	);

	if (piece&PF_NORTH) {
		if ((piece&PF_CONMASK)==(PF_NORTH|PF_EAST)) {
			if (north&PF_BLOCKED) { tx=3; ty=4; }
			else if (north&PF_SOUTH) { tx=7; ty=3; }
			else { tx=3; ty=3; }
		}
		else if ((piece&PF_CONMASK)==(PF_NORTH|PF_WEST)) {
			if (north&PF_BLOCKED) { tx=2; ty=4; }
			else if (north&PF_SOUTH) { tx=6; ty=3; }
			else { tx=2; ty=3; }
		}
		else {
			if (north&PF_BLOCKED) { tx=1; ty=4; }
			else if (north&PF_SOUTH) { tx=5; ty=3; }
			else { tx=1; ty=3; }
		}
	}
	else {
		if (north&PF_BLOCKED) { tx=0; ty=4; }
		else if (north&PF_SOUTH) { tx=4; ty=3; }
		else { tx=0; ty=3; }
	}
	painter.PaintImage(
		x+w2,y,w2,h2,
		ImgBackground,
		tx*tileSize,ty*tileSize,tileSize,tileSize,
		255,canvasColor
	);

	if (piece&PF_WEST) {
		if ((piece&PF_CONMASK)==(PF_WEST|PF_SOUTH)) {
			if (west&PF_BLOCKED) { tx=3; ty=6; }
			else if (west&PF_EAST) { tx=7; ty=5; }
			else { tx=3; ty=5; }
		}
		else if ((piece&PF_CONMASK)==(PF_WEST|PF_NORTH)) {
			if (west&PF_BLOCKED) { tx=2; ty=6; }
			else if (west&PF_EAST) { tx=6; ty=5; }
			else { tx=2; ty=5; }
		}
		else {
			if (west&PF_BLOCKED) { tx=1; ty=6; }
			else if (west&PF_EAST) { tx=5; ty=5; }
			else { tx=1; ty=5; }
		}
	}
	else {
		if (west&PF_BLOCKED) { tx=0; ty=6; }
		else if (west&PF_EAST) { tx=4; ty=5; }
		else { tx=0; ty=5; }
	}
	painter.PaintImage(
		x,y+h2,w2,h2,
		ImgBackground,
		tx*tileSize,ty*tileSize,tileSize,tileSize,
		255,canvasColor
	);

	if (piece&PF_NORTH) {
		if (piece&PF_WEST) {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=7; ty=8; } else { tx=6; ty=8; }
			}
			else {
				if (piece&PF_EAST) { tx=5; ty=8; } else { tx=4; ty=8; }
			}
		}
		else {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=3; ty=8; } else { tx=2; ty=8; }
			}
			else {
				if (piece&PF_EAST) { tx=1; ty=8; } else { tx=0; ty=8; }
			}
		}
	}
	else {
		if (piece&PF_WEST) {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=7; ty=7; } else { tx=6; ty=7; }
			}
			else {
				if (piece&PF_EAST) { tx=5; ty=7; } else { tx=4; ty=7; }
			}
		}
		else {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=3; ty=7; } else { tx=2; ty=7; }
			}
			else {
				if (piece&PF_EAST) { tx=1; ty=7; } else { tx=0; ty=7; }
			}
		}
	}
	painter.PaintImage(
		x+w2,y+h2,w2,h2,
		ImgBackground,
		tx*tileSize,ty*tileSize,tileSize,tileSize,
		255,canvasColor
	);

	if (!(piece&PF_BLOCKED) && (piece&(PF_SOURCE|PF_TARGET))) {
		if (piece&PF_SOURCE) tx=0;
		else if (!(piece&PF_FILLED)) tx=1;
		else tx=2;
		tileSize=ImgSymbols.GetWidth()/3;
		painter.PaintImage(
			x,y,w,h,
			ImgSymbols,
			tx*tileSize,tileSize,tileSize,tileSize
		);
	}
}


void emNetwalkPanel::PaintPiecePipe(
	const emPainter & painter, double x, double y, double w, double h,
	int px, int py
)
{
	int piece,east,west,south,north,tileSize,tx,ty;

	piece=Mdl->GetPiece(px,py);
	east=Mdl->GetPiece(px+1,py);
	west=Mdl->GetPiece(px-1,py);
	south=Mdl->GetPiece(px,py+1);
	north=Mdl->GetPiece(px,py-1);

	tileSize=ImgLights.GetWidth()/4;
	if ((piece&PF_EAST)==0 && (east&PF_WEST)!=0 && (east&PF_FILLED)!=0) {
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgLights,
			0*tileSize,4*tileSize,tileSize,tileSize,0,LightColor
		);
	}
	if ((piece&PF_SOUTH)==0 && (south&PF_NORTH)!=0 && (south&PF_FILLED)!=0) {
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgLights,
			3*tileSize,3*tileSize,tileSize,tileSize,0,LightColor
		);
	}
	if ((piece&PF_WEST)==0 && (west&PF_EAST)!=0 && (west&PF_FILLED)!=0) {
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgLights,
			1*tileSize,4*tileSize,tileSize,tileSize,0,LightColor
		);
	}
	if ((piece&PF_NORTH)==0 && (north&PF_SOUTH)!=0 && (north&PF_FILLED)!=0) {
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgLights,
			3*tileSize,4*tileSize,tileSize,tileSize,0,LightColor
		);
	}

	if (piece&PF_NORTH) {
		if (piece&PF_WEST) {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=1; ty=1; } else { tx=2; ty=1; }
			}
			else {
				if (piece&PF_EAST) { tx=1; ty=2; } else { tx=2; ty=2; }
			}
		}
		else {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=0; ty=1; } else { tx=3; ty=1; }
			}
			else {
				if (piece&PF_EAST) { tx=0; ty=2; } else { tx=3; ty=2; }
			}
		}
	}
	else {
		if (piece&PF_WEST) {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=1; ty=0; } else { tx=2; ty=0; }
			}
			else {
				if (piece&PF_EAST) { tx=1; ty=3; } else { tx=2; ty=3; }
			}
		}
		else {
			if (piece&PF_SOUTH) {
				if (piece&PF_EAST) { tx=0; ty=0; } else { tx=3; ty=0; }
			}
			else {
				if (piece&PF_EAST) { tx=0; ty=3; } else { tx=3; ty=3; }
			}
		}
	}

	if ((piece&PF_FILLED)==0 || !LightColor.IsOpaque()) {
		tileSize=ImgPipes.GetWidth()/4;
		PaintImageWithRoundedEdges(
			painter,x,y,w,h,ImgPipes,
			tx*tileSize,ty*tileSize,tileSize,tileSize
		);
	}
	if (piece&PF_FILLED) {
		tileSize=ImgLights.GetWidth()/4;
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgLights,
			tx*tileSize,ty*tileSize,tileSize,tileSize,0,LightColor
		);
	}

	if (piece&PF_MARKED) {
		tileSize=ImgMarks.GetWidth()/4;
		PaintShapeWithRoundedEdges(
			painter,x,y,w,h,ImgMarks,
			tx*tileSize,ty*tileSize,tileSize,tileSize,0,MarkColor
		);
	}

	if (piece&(PF_SOURCE|PF_TARGET)) {
		if (piece&PF_SOURCE) tx=0;
		else if (!(piece&PF_FILLED)) tx=1;
		else tx=2;
		tileSize=ImgSymbols.GetWidth()/3;
		painter.PaintImage(
			x,y,w,h,
			ImgSymbols,
			tx*tileSize,0,tileSize,tileSize
		);
	}
}


void emNetwalkPanel::PaintShapeWithRoundedEdges(
	const emPainter & painter, double x, double y, double w, double h,
	const emImage & img, double srcX, double srcY, double srcW, double srcH,
	int channel, emColor color, emColor canvasColor
)
{
	double rx,ry,rw,rh;

	rx=painter.RoundX(x);
	ry=painter.RoundY(y);
	rw=painter.RoundX(x+w)-rx;
	rh=painter.RoundY(y+h)-ry;
	painter.PaintShape(
		rx,ry,rw,rh,
		img,
		srcX+srcW/w*(rx-x),
		srcY+srcH/h*(ry-y),
		srcW/w*rw,
		srcH/h*rh,
		channel,color,canvasColor
	);
}


void emNetwalkPanel::PaintImageWithRoundedEdges(
	const emPainter & painter, double x, double y, double w, double h,
	const emImage & img, double srcX, double srcY, double srcW, double srcH,
	int alpha, emColor canvasColor
)
{
	double rx,ry,rw,rh;

	rx=painter.RoundX(x);
	ry=painter.RoundY(y);
	rw=painter.RoundX(x+w)-rx;
	rh=painter.RoundY(y+h)-ry;
	painter.PaintImage(
		rx,ry,rw,rh,
		img,
		srcX+srcW/w*(rx-x),
		srcY+srcH/h*(ry-y),
		srcW/w*rw,
		srcH/h*rh,
		alpha,canvasColor
	);
}


void emNetwalkPanel::PrepareTransformation()
{
	double h,bs,tw,th;

	if (!IsViewed() || !IsVFSGood()) {
		EssenceX=0.0;
		EssenceY=0.0;
		EssenceW=1.0;
		EssenceH=GetHeight();
		X0=0.0;
		Y0=0.0;
		DX=1.0;
		DY=1.0;
	}
	else {
		h=GetHeight();
		bs=Mdl->IsBorderless() ? 1.0 : BorderSize;
		tw=Mdl->GetWidth()+2*bs;
		th=Mdl->GetHeight()+2*bs;
		DX=emMin(1.0/tw,h/th)*0.85;
		DY=DX;
		EssenceW=tw*DX;
		EssenceH=th*DY;
		EssenceX=(1.0-EssenceW)*0.5;
		EssenceY=(h-EssenceH)*0.5;
		X0=EssenceX+bs*DX;
		Y0=EssenceY+bs*DY;
	}
}


const double emNetwalkPanel::BorderSize=0.333333;
const emColor emNetwalkPanel::BgColor=0x000000FF;
const emColor emNetwalkPanel::LightColor=0x00FFCCBB;
const emColor emNetwalkPanel::MarkColor=0xDDDDDD55;
