//------------------------------------------------------------------------------
// emFractalFilePanel.cpp
//
// Copyright (C) 2004-2008,2014,2016-2017 Oliver Hamann.
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

#include <emFractal/emFractalFilePanel.h>


emFractalFilePanel::emFractalFilePanel(
	ParentArg parent, const emString & name, emFractalFileModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	RenderThreadPool=emRenderThreadPool::Acquire(GetRootContext());
	Mdl=fileModel;
	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
	Colors.SetTuningLevel(4);
	Prepare();
}


emFractalFilePanel::~emFractalFilePanel()
{
}


emString emFractalFilePanel::GetTitle() const
{
	return "Fractal";
}


emString emFractalFilePanel::GetIconFileName() const
{
	return "fractal.tga";
}


bool emFractalFilePanel::Cycle()
{
	bool busy;

	if (
		IsSignaled(Mdl->GetChangeSignal()) ||
		IsSignaled(GetVirFileStateSignal())
	) Prepare();

	if (!Image.IsEmpty() && PixStep>=0) {
		CommonRenderVars crv;
		crv.Panel=this;
		crv.InvX1=Image.GetWidth();
		crv.InvY1=Image.GetHeight();
		crv.InvX2=0;
		crv.InvY2=0;

		while (PixStep>=0 && !IsTimeSliceAtEnd()) {
			RenderThreadPool->CallParallel(ThreadRenderFunc, &crv);
			if (PixY>=Image.GetHeight()) {
				PixY=0;
				if (PixStep>0) {
					PixStep>>=1;
					PixX=PixStep&~PixY;
				}
				else {
					PixStep--;
					PixX=0;
				}
			}
		}

		if (crv.InvX1<crv.InvX2 && crv.InvY1<crv.InvY2) {
			InvalidatePainting(
				ViewToPanelX(ImgX1+crv.InvX1),
				ViewToPanelY(ImgY1+crv.InvY1),
				ViewToPanelDeltaX(crv.InvX2-crv.InvX1),
				ViewToPanelDeltaY(crv.InvY2-crv.InvY1)
			);
		}
	}

	busy = !Image.IsEmpty() && PixStep>=0;
	if (emFilePanel::Cycle()) busy=true;
	return busy;
}


void emFractalFilePanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if ((flags&NF_VIEWING_CHANGED)!=0) {
		Prepare();
		WakeUp();
	}
}


bool emFractalFilePanel::IsOpaque() const
{
	if (Image.IsEmpty()) {
		return emFilePanel::IsOpaque();
	}
	else {
		return true;
	}
}


void emFractalFilePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	if (Image.IsEmpty()) {
		emFilePanel::Paint(painter,canvasColor);
	}
	else {
		painter.PaintImage(
			ViewToPanelX(ImgX1),
			ViewToPanelY(ImgY1),
			ViewToPanelDeltaX(ImgX2-ImgX1),
			ViewToPanelDeltaY(ImgY2-ImgY1),
			Image,
			255,
			canvasColor
		);
	}
}


void emFractalFilePanel::Prepare()
{
	emColor c1, c2;
	int i,j,k,n,m,w,h;

	ImgX1=floor(GetClipX1());
	ImgY1=floor(GetClipY1());
	ImgX2=ceil(GetClipX2());
	ImgY2=ceil(GetClipY2());

	if (!IsViewed() || !IsVFSGood() || ImgX1>=ImgX2-2 || ImgY1>=ImgY2-2) {
		Image.Clear();
		Colors.Clear();
		return;
	}

	w=(int)(ImgX2-ImgX1);
	h=(int)(ImgY2-ImgY1);
	if (Image.GetWidth()!=w || Image.GetHeight()!=h || Image.GetChannelCount()!=3) {
		Image.Setup(w,h,3);
		Image.Fill(0);
	}
	if (GetViewedWidth()<GetViewedHeight()*GetViewedPixelTallness()) {
		FrcSX=4.0/GetViewedWidth();
		FrcSY=FrcSX*GetViewedPixelTallness();
	}
	else {
		FrcSY=4.0/GetViewedHeight();
		FrcSX=FrcSY/GetViewedPixelTallness();
	}
	FrcX=(ImgX1-GetViewedX()-GetViewedWidth()/2)*FrcSX;
	FrcY=(ImgY1-GetViewedY()-GetViewedHeight()/2)*FrcSY;
	if (Mdl->Type!=emFractalFileModel::JULIA_TYPE) FrcX+=0.5;
	PixStep=1;
	while (PixStep<Image.GetWidth()) PixStep<<=1;
	while (PixStep<Image.GetHeight()) PixStep<<=1;
	PixY=0;
	PixX=0;

	n=Mdl->Colors.GetCount();
	for (i=0, m=n; i<n; i++) m+=Mdl->Colors[i].Fade;
	Colors.SetCount(m,true);
	for (i=0, j=0; i<n; i++) {
		c1=Mdl->Colors[i].Color;
		Colors.Set(j++,c1);
		m=Mdl->Colors[i].Fade;
		if (m>0) {
			c2=Mdl->Colors[(i+1)%n].Color;
			for (k=0; k<m; k++, j++) Colors.Set(j,c1.GetBlended(c2,(k+1)*100.0f/m));
		}
	}
}


void emFractalFilePanel::ThreadRenderFunc(void * data, int index)
{
	CommonRenderVars * crv=(CommonRenderVars*)data;
	crv->Panel->ThreadRenderRun(*crv);
}


void emFractalFilePanel::ThreadRenderRun(CommonRenderVars & crv)
{
	static const int MAX_PIXELS_AT_ONCE = 100;
	ThreadRenderVars trv;
	emColor c1,c2,c3,c4;
	int x,y,s,dx,n,endX;

	crv.Mutex.Lock();

	trv.ImgWidth=Image.GetWidth();
	trv.ImgHeight=Image.GetHeight();
	trv.ImgMap=Image.GetWritableMap();
	trv.InvX1=crv.InvX1;
	trv.InvY1=crv.InvY1;
	trv.InvX2=crv.InvX2;
	trv.InvY2=crv.InvY2;

	if (PixStep>0) {
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
				PutPixel(trv,x,y,s,CalcPixel(x,y));
			}
			crv.Mutex.Lock();
		}
	}
	else {
		// Anti-aliasing...
		while (PixY<trv.ImgHeight && !IsTimeSliceAtEnd()) {
			x=PixX;
			y=PixY;

			n=trv.ImgWidth-x;
			if (n>MAX_PIXELS_AT_ONCE/3) n=MAX_PIXELS_AT_ONCE/3;
			endX=x+n;

			PixX=endX;
			if (PixX>=trv.ImgWidth) {
				PixY++;
				PixX=0;
			}

			crv.Mutex.Unlock();
			for (; x<endX; x++) {
				c1=PeekPixel(trv,x,y);
				c2=CalcPixel(x+0.5,y);
				c3=CalcPixel(x,y+0.5);
				c4=CalcPixel(x+0.5,y+0.5);
				PutPixel(
					trv,x,y,1,
					emColor(
						(emByte)((
							(unsigned)c1.GetRed()+
							(unsigned)c2.GetRed()+
							(unsigned)c3.GetRed()+
							(unsigned)c4.GetRed()+
							2
						)>>2),
						(emByte)((
							(unsigned)c1.GetGreen()+
							(unsigned)c2.GetGreen()+
							(unsigned)c3.GetGreen()+
							(unsigned)c4.GetGreen()+
							2
						)>>2),
						(emByte)((
							(unsigned)c1.GetBlue()+
							(unsigned)c2.GetBlue()+
							(unsigned)c3.GetBlue()+
							(unsigned)c4.GetBlue()+
							2
						)>>2)
					)
				);
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


emColor emFractalFilePanel::CalcPixel(double pixX, double pixY) const
{
	double x,y,r,s,rr,ss;
	int d;

	switch (Mdl->Type) {
	case emFractalFileModel::MANDELBROT_TYPE:
		x=FrcX+FrcSX*pixX;
		y=FrcY+FrcSY*pixY;
		r=0;
		s=0;
		rr=0;
		ss=0;
		d=Mdl->Depth;
		do {
			s=s*r*2-y;
			r=rr-ss-x;
			ss=s*s;
			rr=r*r;
			d--;
		} while (d>0 && ss+rr<8.0);
		d=Mdl->Depth-d;
		return Colors[d%Colors.GetCount()];
	case emFractalFileModel::JULIA_TYPE:
		x=Mdl->JuliaX;
		y=Mdl->JuliaY;
		r=FrcX+FrcSX*pixX;
		s=FrcY+FrcSY*pixY;
		rr=r*r;
		ss=s*s;
		d=Mdl->Depth;
		do {
			s=s*r*2-y;
			r=rr-ss-x;
			ss=s*s;
			rr=r*r;
			d--;
		} while (d>0 && ss+rr<8.0);
		d=Mdl->Depth-d;
		return Colors[d%Colors.GetCount()];
	case emFractalFileModel::MULTI_JULIA_TYPE:
		x=floor((FrcX+FrcSX*pixX)*100)/100;
		y=floor((FrcY+FrcSY*pixY)*100)/100;
		r=(x-(FrcX+FrcSX*pixX))*100*3.4+1.7;
		s=(y-(FrcY+FrcSY*pixY))*100*3.4+1.7;
		rr=r*r;
		ss=s*s;
		d=Mdl->Depth;
		do {
			s=s*r*2-y;
			r=rr-ss-x;
			ss=s*s;
			rr=r*r;
			d--;
		} while (d>0 && ss+rr<8.0);
		d=Mdl->Depth-d;
		return Colors[d%Colors.GetCount()];
	default:
		return 0;
	}
}


void emFractalFilePanel::PutPixel(
	ThreadRenderVars & trv, int x, int y, int s, emColor color
)
{
	emByte * p, * pxe;
	int w,h,t;

	w=trv.ImgWidth;
	h=trv.ImgHeight;
	if (s>32) s=32;
	t=s;
	if (s>w-x) s=w-x;
	if (t>h-y) t=h-y;
	if (trv.InvX1>x) trv.InvX1=x;
	if (trv.InvY1>y) trv.InvY1=y;
	if (trv.InvX2<x+s) trv.InvX2=x+s;
	if (trv.InvY2<y+t) trv.InvY2=y+t;
	w*=3;
	s*=3;
	p=trv.ImgMap+y*w+x*3;
	w-=s;
	do {
		pxe=p+s;
		do {
			p[0]=color.GetRed();
			p[1]=color.GetGreen();
			p[2]=color.GetBlue();
			p+=3;
		} while (p<pxe);
		p+=w;
		t--;
	} while (t>0);
}


emColor emFractalFilePanel::PeekPixel(
	const ThreadRenderVars & trv, int x, int y
)
{
	const emByte * p;
	int w,h;

	w=trv.ImgWidth;
	h=trv.ImgHeight;
	if (x>=0 && x<w && y>=0 && y<h) {
		p=trv.ImgMap+(y*w+x)*3;
		return emColor(p[0],p[1],p[2]);
	}
	else {
		return 0;
	}
}
