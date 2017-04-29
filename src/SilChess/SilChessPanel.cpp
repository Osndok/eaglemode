//------------------------------------------------------------------------------
// SilChessPanel.cpp
//
// Copyright (C) 2007-2008,2014,2016-2017 Oliver Hamann.
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

#include <SilChess/SilChessControlPanel.h>
#include <SilChess/SilChessPanel.h>

//#define DO_IT_WITH_ALPHA //???

SilChessPanel::SilChessPanel(
	ParentArg parent, const emString & name, SilChessModel * model
)
	: emFilePanel(parent,name,model,true)
{
	RenderThreadPool=emRenderThreadPool::Acquire(GetRootContext());
	Mdl=model;
	HaveControlPanel=IsVFSGood();
	SelX=-1;
	SelY=-1;
	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
	PrepareRendering(true);
}


SilChessPanel::~SilChessPanel()
{
}


emString SilChessPanel::GetTitle() const
{
	return "SilChess";
}


emString SilChessPanel::GetIconFileName() const
{
	return "silchess.tga";
}


void SilChessPanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	*pX=EssenceX;
	*pY=EssenceY;
	*pW=EssenceW;
	*pH=EssenceH;
}


bool SilChessPanel::Cycle()
{
	bool busy,vfsGood;

	if (IsSignaled(GetVirFileStateSignal())) {
		vfsGood=IsVFSGood();
		if (HaveControlPanel!=vfsGood) {
			HaveControlPanel=vfsGood;
			InvalidateControlPanel();
		}
	}

	if (
		IsSignaled(Mdl->GetChangeSignal()) ||
		IsSignaled(GetVirFileStateSignal())
	) {
		SelX=-1;
		SelY=-1;
		InvalidatePainting();
		PrepareRendering(false);
	}

	if (!Image.IsEmpty() && PixStep>0) {
		CommonRenderVars crv;
		crv.Panel=this;
		crv.InvX1=Image.GetWidth();
		crv.InvY1=Image.GetHeight();
		crv.InvX2=0;
		crv.InvY2=0;

		while (
			PixStep>0 &&
			(!ImageGood || AnimCnt<AnimEnd) &&
			!IsTimeSliceAtEnd()
		) {
			RenderThreadPool->CallParallel(ThreadRenderFunc, &crv);
			if (PixY>=Image.GetHeight()) {
				PixY=0;
				PixStep>>=1;
				PixX=PixStep&~PixY;
				if (!PixStep) ImageGood=true;
			}
		}

		AnimEnd+=emMax(1,Image.GetHeight()/60);

		if (crv.InvX1<crv.InvX2 && crv.InvY1<crv.InvY2) {
			InvalidatePainting(
				ViewToPanelX(ImgX1+crv.InvX1),
				ViewToPanelY(ImgY1+crv.InvY1),
				ViewToPanelDeltaX(crv.InvX2-crv.InvX1),
				ViewToPanelDeltaY(crv.InvY2-crv.InvY1)
			);
		}
	}

	busy = !Image.IsEmpty() && PixStep>0;
	if (emFilePanel::Cycle()) busy=true;
	return busy;
}


void SilChessPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if ((flags&NF_VIEWING_CHANGED)!=0) {
		PrepareRendering(true);
		WakeUp();
	}
}


void SilChessPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	SilChessMachine * machine;
	SilChessMachine::Move m;
	int x,y,i;

	if (!IsVFSGood() || Image.IsEmpty()) {
		emFilePanel::Input(event,state,mx,my);
		return;
	}

	machine=Mdl->GetMachine();

	switch (event.GetKey()) {
	case EM_KEY_N:
		if (state.IsCtrlMod()) {
			machine->StartNewGame();
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_F:
		if (state.IsCtrlMod()) {
			machine->SetHumanWhite(!machine->IsHumanWhite());
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_Z:
		if (state.IsCtrlMod()) {
			machine->UndoMove();
			if (!machine->IsHumanOn()) machine->UndoMove();
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_H:
		if (state.IsCtrlMod()) {
			Mdl->RequestHint();
			event.Eat();
		}
		break;
	case EM_KEY_0:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(0);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_1:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(1);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_2:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(2);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_3:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(3);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_4:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(4);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_5:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(5);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_6:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(6);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_7:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(7);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_8:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(8);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_9:
		if (state.IsCtrlMod()) {
			machine->SetSearchDepth(9);
			Mdl->SaveAndSignalChanges();
			event.Eat();
		}
		break;
	case EM_KEY_LEFT_BUTTON:
		if (state.IsNoMod()) {
			PanelToBoard(mx,my,&x,&y);
			if (x<0 || y<0 || x>7 || y>7 || (x==SelX && y==SelY)) {
				if (SelX!=-1 || SelY!=-1) {
					SelX=SelY=-1;
					InvalidatePainting();
				}
			}
			else if (machine->IsHumanOn()) {
				i=machine->GetField(x,y);
				if (i!=0 && ((i<7) == machine->IsWhiteOn())) {
					if (SelX!=x || SelY!=y) {
						SelX=x;
						SelY=y;
						InvalidatePainting();
					}
				}
				else if (SelX!=-1 && SelY!=-1) {
					m.X1=(signed char)SelX;
					m.Y1=(signed char)SelY;
					m.X2=(signed char)x;
					m.Y2=(signed char)y;
					if (machine->IsLegalMove(m)) {
						machine->DoMove(m);
						Mdl->SaveAndSignalChanges();
					}
					SelX=SelY=-1;
					InvalidatePainting();
				}
			}
			Focus();
			event.Eat();
		}
	default:
		break;
	}

	emFilePanel::Input(event,state,mx,my);
}


bool SilChessPanel::IsOpaque() const
{
	if (!IsVFSGood() || Image.IsEmpty()) {
		return emFilePanel::IsOpaque();
	}
	else {
#ifdef DO_IT_WITH_ALPHA
		return false;
#else
		return true;
#endif
	}
}


void SilChessPanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	if (!IsVFSGood() || Image.IsEmpty()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	painter.PaintImage(
		ViewToPanelX(ImgX1),
		ViewToPanelY(ImgY1),
		ViewToPanelDeltaX(ImgX2-ImgX1),
		ViewToPanelDeltaY(ImgY2-ImgY1),
		Image,
		255,
		canvasColor
	);

	PaintSelection(painter);

	PaintArrow(painter);
}


emPanel * SilChessPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (HaveControlPanel) {
		return new SilChessControlPanel(parent,name,Mdl);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void SilChessPanel::PrepareRendering(bool viewingChanged)
{
	double h,vx,vy,vw,vt,s,k,tx,ty,tz,cx,cy,cz,ppw,pph,ppwe,pphe;
	double ppy,ppz,ppny,ppnz,ppd;
	int imgW,imgH;

	ImgX1=floor(GetClipX1());
	ImgY1=floor(GetClipY1());
	ImgX2=ceil(GetClipX2());
	ImgY2=ceil(GetClipY2());

	if (!IsViewed() || !IsVFSGood() || ImgX1>=ImgX2-2 || ImgY1>=ImgY2-2) {
		Image.Clear();
		ImageGood=false;
		EssenceX=0.0;
		EssenceY=0.0;
		EssenceW=1.0;
		EssenceH=GetHeight();
		return;
	}

	imgW=(int)(ImgX2-ImgX1);
	imgH=(int)(ImgY2-ImgY1);
#ifdef DO_IT_WITH_ALPHA
	if (Image.GetWidth()!=imgW || Image.GetHeight()!=imgH || Image.GetChannelCount()!=4) {
		Image.Setup(imgW,imgH,4);
#else
	if (Image.GetWidth()!=imgW || Image.GetHeight()!=imgH || Image.GetChannelCount()!=3) {
		Image.Setup(imgW,imgH,3);
#endif
		Image.Fill(0);
		ImageGood=false;
	}

	PixStep=1;
	while (PixStep<imgW) PixStep<<=1;
	while (PixStep<imgH) PixStep<<=1;
	PixY=0;
	PixX=0;

	AnimCnt=0;
	AnimEnd=0;

	HumanIsWhite=Mdl->GetMachine()->IsHumanWhite();
	RayTracer.SetWorld(Mdl->GetMachine());

	if (ImageGood && !viewingChanged) {
		PixStep=1;
		return;
	}
	ImageGood=false;

	//---------- Camera Setup --------------

	// Position of the center of the projection plane within the yz-plane in
	// world coordinates.
	ppy=-2.3;
	ppz=1.8;

	// Normal of projection plane within the yz-plane.
	ppny=1.0;
	ppnz=-1.253;

	// Minimum size of projection plane in world coordinates.
	ppw=11.0;
	pph=10.0;

	// Same as above, but for essence rect.
	ppwe=9.0;
	pphe=6.6;

	// Distance between camera and projection plane when shown full-sized.
	ppd=15.5;

	//--------------------------------------

	k=1.0/sqrt(ppny*ppny+ppnz*ppnz);
	ppny*=k;
	ppnz*=k;

	h=GetHeight();
	s=emMin(h/pph,1.0/ppw);

	EssenceW=ppwe*s;
	EssenceH=pphe*s;
	EssenceX=(1.0-EssenceW)*0.5;
	EssenceY=(h-EssenceH)*0.5;

	tx=ViewToPanelX(GetView().GetCurrentX()+GetView().GetCurrentWidth()*0.5);
	ty=ViewToPanelY(GetView().GetCurrentY()+GetView().GetCurrentHeight()*0.5);
	cx=(tx-0.5)/s;
	cy=(ty-h*0.5)/s;
	k=emMax(
		PanelToViewDeltaX(EssenceW)/GetView().GetCurrentWidth(),
		PanelToViewDeltaY(EssenceH)/GetView().GetCurrentHeight()
	);
	cz=ppd/k;
	tz=cz*s;
	if (k>1.0) {
		tz*=k/(2.0-1.0/k);
		cz*=1.0-(1.0-1.0/k)*log(k)*0.5;
	}
	CameraX=cx;
	CameraY=ppy-ppny*cz+ppnz*cy;
	CameraZ=ppz-ppnz*cz-ppny*cy;

	vx=GetViewedX();
	vy=GetViewedY();
	vw=GetViewedWidth();
	vt=GetViewedPixelTallness();

	RayXA=1.0/vw;
	RayXB=(ImgX1-vx)/vw-tx;
	RayYA=ppnz*vt/vw;
	RayYB=ppny*tz-ppnz*ty+ppnz*(ImgY1-vy)*vt/vw;
	RayZA=-ppny*vt/vw;
	RayZB=ppnz*tz+ppny*ty-ppny*(ImgY1-vy)*vt/vw;
}


void SilChessPanel::ThreadRenderFunc(void * data, int index)
{
	CommonRenderVars * crv=(CommonRenderVars*)data;
	crv->Panel->ThreadRenderRun(*crv);
}


void SilChessPanel::ThreadRenderRun(CommonRenderVars & crv)
{
	static const int MAX_PIXELS_AT_ONCE = 1000;
	ThreadRenderVars trv;
	int x,y,s,dx,n,endX,msk;

	crv.Mutex.Lock();

	trv.ImgWidth=Image.GetWidth();
	trv.ImgHeight=Image.GetHeight();
	trv.ImgMap=Image.GetWritableMap();
	trv.InvX1=crv.InvX1;
	trv.InvY1=crv.InvY1;
	trv.InvX2=crv.InvX2;
	trv.InvY2=crv.InvY2;

	if (!ImageGood) {
		while (PixY<trv.ImgHeight && !IsTimeSliceAtEnd()) {
			x=PixX;
			y=PixY;
			s=PixStep;

			dx=s+(s&~y);
			n=(trv.ImgWidth-x+dx-1)/dx;
			if (n>MAX_PIXELS_AT_ONCE) n=MAX_PIXELS_AT_ONCE;
			endX=x+n*dx;

			PixX=endX;
			if (PixX>=trv.ImgWidth) {
				PixY+=PixStep;
				PixX=PixStep&~PixY;
			}

			crv.Mutex.Unlock();
			for (; x<endX; x+=dx) {
				RenderPixel(trv,x,y,s);
			}
			crv.Mutex.Lock();
		}
	}
	else {
		while (PixY<trv.ImgHeight && AnimCnt<AnimEnd && !IsTimeSliceAtEnd()) {
			x=PixX;
			y=PixY;

			n=trv.ImgWidth-x;
			if (n>MAX_PIXELS_AT_ONCE) n=MAX_PIXELS_AT_ONCE;
			endX=x+n;

			PixX=endX;
			if (PixX>=trv.ImgWidth) {
				for (msk=0x3ff; msk<trv.ImgHeight-1; msk=(msk<<1)|1);
				do { PixY=(PixY+269779)&msk; } while (PixY>=trv.ImgHeight);
				if (PixY==0) PixY=trv.ImgHeight;
				PixX=0;
				AnimCnt++;
			}

			crv.Mutex.Unlock();
			for (; x<endX; x++) {
				RenderPixel(trv,x,y,1);
			}
			crv.Mutex.Lock();
		}
	}

	if (crv.InvX1>trv.InvX1) crv.InvX1=trv.InvX1;
	if (crv.InvY1>trv.InvY1) crv.InvY1=trv.InvY1;
	if (crv.InvX2<trv.InvX2) crv.InvX2=trv.InvX2;
	if (crv.InvY2<trv.InvY2) crv.InvY2=trv.InvY2;

	crv.Mutex.Unlock();
}


void SilChessPanel::RenderPixel(
	ThreadRenderVars & trv, int pixX, int pixY, int pixSize
)
{
	SilChessRayTracer::Color col;
	double k,rx,ry,rz;
	emByte * p, * pxe;
	int r,g,b,w,h,s,t;

	rx=RayXA*(pixX+0.5)+RayXB;
	ry=RayYA*(pixY+0.5)+RayYB;
	rz=RayZA*(pixY+0.5)+RayZB;
	k=1.0/sqrt(ry*ry+rz*rz+rx*rx);
#ifdef DO_IT_WITH_ALPHA
	int a;
	a=RayTracer.TraceRay(1,CameraX,CameraY,CameraZ,rx*k,ry*k,rz*k,&col)?255:0;
#else
	RayTracer.TraceRay(1,CameraX,CameraY,CameraZ,rx*k,ry*k,rz*k,&col);
#endif

	r=col.Red  ; if (r>255) r=255;
	g=col.Green; if (g>255) g=255;
	b=col.Blue ; if (b>255) b=255;

	w=trv.ImgWidth;
	h=trv.ImgHeight;
	s=pixSize;
	if (s>32) s=32;
	t=s;
	if (s>w-pixX) s=w-pixX;
	if (t>h-pixY) t=h-pixY;
	if (trv.InvX1>pixX) trv.InvX1=pixX;
	if (trv.InvY1>pixY) trv.InvY1=pixY;
	if (trv.InvX2<pixX+s) trv.InvX2=pixX+s;
	if (trv.InvY2<pixY+t) trv.InvY2=pixY+t;
#ifdef DO_IT_WITH_ALPHA
	w*=4;
	s*=4;
	p=trv.ImgMap+pixY*w+pixX*4;
	w-=s;
	do {
		pxe=p+s;
		do {
			p[0]=(emByte)r;
			p[1]=(emByte)g;
			p[2]=(emByte)b;
			p[3]=(emByte)a;
			p+=4;
		} while (p<pxe);
		p+=w;
		t--;
	} while (t>0);
#else
	w*=3;
	s*=3;
	p=trv.ImgMap+pixY*w+pixX*3;
	w-=s;
	do {
		pxe=p+s;
		do {
			p[0]=(emByte)r;
			p[1]=(emByte)g;
			p[2]=(emByte)b;
			p+=3;
		} while (p<pxe);
		p+=w;
		t--;
	} while (t>0);
#endif
}


void SilChessPanel::PanelToBoard(double x, double y, int * bx, int * by) const
{
	double rx,ry,rz,k;
	int ix,iy;

	x=PanelToViewX(x)-ImgX1;
	y=PanelToViewY(y)-ImgY1;
	rx=RayXA*x+RayXB;
	ry=RayYA*y+RayYB;
	rz=RayZA*y+RayZB;
	k=1.0/sqrt(ry*ry+rz*rz+rx*rx);
	rx*=k; ry*=k; rz*=k;
	*bx=-1; *by=-1;
	if (rz<=-0.0001) {
		ix=(int)(CameraX-rx*CameraZ/rz+6)-2;
		iy=(int)(CameraY-ry*CameraZ/rz+6)-2;
		if (ix>=0 && ix<8 && iy>=0 && iy<8) {
			if (HumanIsWhite) iy=7-iy; else ix=7-ix;
			*bx=ix; *by=iy;
		}
	}
}


void SilChessPanel::BoardToPanel(double x, double y, double * px, double * py) const
{
	if (HumanIsWhite) y=8.0-y; else x=8.0-x;
	x=(CameraX+4.0-x)/CameraZ;
	y=(CameraY+4.0-y)/CameraZ;
	y=(RayYB-y*RayZB)/(y*RayZA-RayYA);
	x=(x*(RayZA*y+RayZB)-RayXB)/RayXA;
	*px=ViewToPanelX(ImgX1+x);
	*py=ViewToPanelY(ImgY1+y);
}


void SilChessPanel::PaintSelection(const emPainter & painter) const
{
	static const emColor col(0,0,0);
	double xy[6*2];
	double d,l;

	if (SelX<0 || SelY<0) return;

	d=0.006;
	l=1.0/6.0;

	BoardToPanel(SelX-d,SelY-d,xy+ 0,xy+ 1);
	BoardToPanel(SelX+l,SelY-d,xy+ 2,xy+ 3);
	BoardToPanel(SelX+l,SelY+d,xy+ 4,xy+ 5);
	BoardToPanel(SelX+d,SelY+d,xy+ 6,xy+ 7);
	BoardToPanel(SelX+d,SelY+l,xy+ 8,xy+ 9);
	BoardToPanel(SelX-d,SelY+l,xy+10,xy+11);
	painter.PaintPolygon(xy,6,col);

	BoardToPanel(SelX+1+d,SelY-d,xy+ 0,xy+ 1);
	BoardToPanel(SelX+1-l,SelY-d,xy+ 2,xy+ 3);
	BoardToPanel(SelX+1-l,SelY+d,xy+ 4,xy+ 5);
	BoardToPanel(SelX+1-d,SelY+d,xy+ 6,xy+ 7);
	BoardToPanel(SelX+1-d,SelY+l,xy+ 8,xy+ 9);
	BoardToPanel(SelX+1+d,SelY+l,xy+10,xy+11);
	painter.PaintPolygon(xy,6,col);

	BoardToPanel(SelX-d,SelY+1+d,xy+ 0,xy+ 1);
	BoardToPanel(SelX+l,SelY+1+d,xy+ 2,xy+ 3);
	BoardToPanel(SelX+l,SelY+1-d,xy+ 4,xy+ 5);
	BoardToPanel(SelX+d,SelY+1-d,xy+ 6,xy+ 7);
	BoardToPanel(SelX+d,SelY+1-l,xy+ 8,xy+ 9);
	BoardToPanel(SelX-d,SelY+1-l,xy+10,xy+11);
	painter.PaintPolygon(xy,6,col);

	BoardToPanel(SelX+1+d,SelY+1+d,xy+ 0,xy+ 1);
	BoardToPanel(SelX+1-l,SelY+1+d,xy+ 2,xy+ 3);
	BoardToPanel(SelX+1-l,SelY+1-d,xy+ 4,xy+ 5);
	BoardToPanel(SelX+1-d,SelY+1-d,xy+ 6,xy+ 7);
	BoardToPanel(SelX+1-d,SelY+1-l,xy+ 8,xy+ 9);
	BoardToPanel(SelX+1+d,SelY+1-l,xy+10,xy+11);
	painter.PaintPolygon(xy,6,col);
}


void SilChessPanel::PaintArrow(const emPainter & painter) const
{
	static const emColor mateColor =emColor(187,0,68,80);
	static const emColor checkColor=emColor(187,0,68,80);
	static const emColor drawColor =emColor(0,68,187,80);
	static const emColor onColor   =emColor(0,68,187,80);
	SilChessMachine * machine;
	double xy[4*2];
	emColor color;
	int i,n;

	machine=Mdl->GetMachine();
	if (machine->IsMate()) {
		color=mateColor;
		n=4;
	}
	else if (machine->IsDraw() || machine->IsEndless()) {
		color=drawColor;
		n=4;
	}
	else if (machine->IsCheck()) {
		color=checkColor;
		n=3;
	}
	else {
		color=onColor;
		n=3;
	}

	if (n==3) {
		xy[0]=4.00; xy[1]=-0.70;
		xy[2]=3.70; xy[3]=-0.96;
		xy[4]=4.30; xy[5]=-0.96;
	}
	else {
		xy[0]=3.83; xy[1]=-0.96;
		xy[2]=4.17; xy[3]=-0.96;
		xy[4]=4.17; xy[5]=-0.70;
		xy[6]=3.83; xy[7]=-0.70;
	}

	if (machine->IsWhiteOn()) {
		for (i=0; i<n; i++) {
			xy[i*2]=8.0-xy[i*2];
			xy[i*2+1]=8.0-xy[i*2+1];
		}
	}

	for (i=0; i<n; i++) BoardToPanel(xy[i*2],xy[i*2+1],&xy[i*2],&xy[i*2+1]);
	painter.PaintPolygon(xy,n,color);
}
