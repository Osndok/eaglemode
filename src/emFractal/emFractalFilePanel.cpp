//------------------------------------------------------------------------------
// emFractalFilePanel.cpp
//
// Copyright (C) 2004-2008 Oliver Hamann.
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
	Mdl=fileModel;
	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
	Colors.SetTuningLevel(4);
	Prepare();
}


emFractalFilePanel::~emFractalFilePanel()
{
}


emString emFractalFilePanel::GetTitle()
{
	return "Fractal";
}


bool emFractalFilePanel::Cycle()
{
	bool busy;

	if (
		IsSignaled(Mdl->GetChangeSignal()) ||
		IsSignaled(GetVirFileStateSignal())
	) Prepare();

	if (!Image.IsEmpty() && PixStep>0) {
		InvX1=Image.GetWidth();
		InvY1=Image.GetHeight();
		InvX2=0;
		InvY2=0;
		do {
			PutPixel(CalcPixel());
			PixX+=PixStep+(PixStep&~PixY);
			while (PixX>=Image.GetWidth()) {
				PixY+=PixStep;
				if (PixY>=Image.GetHeight()) {
					PixStep>>=1;
					if (PixStep==0) break;
					PixY=0;
				}
				PixX=PixStep&~PixY;
			}
		} while (!IsTimeSliceAtEnd() && PixStep>0);
		InvalidatePainting(
			ViewToPanelX(ImgX1+InvX1),
			ViewToPanelY(ImgY1+InvY1),
			ViewToPanelDeltaX(InvX2-InvX1),
			ViewToPanelDeltaY(InvY2-InvY1)
		);
	}

	busy = !Image.IsEmpty() && PixStep>0;
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


bool emFractalFilePanel::IsOpaque()
{
	if (Image.IsEmpty()) {
		return emFilePanel::IsOpaque();
	}
	else {
		return true;
	}
}


void emFractalFilePanel::Paint(const emPainter & painter, emColor canvasColor)
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

	ImgX1=GetClipX1();
	ImgY1=GetClipY1();
	ImgX2=GetClipX2();
	ImgY2=GetClipY2();

	if (!IsViewed() || !IsVFSGood() || ImgX1>=ImgX2-2 || ImgY1>=ImgY2-2) {
		Image.Empty();
		Colors.Empty();
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


emColor emFractalFilePanel::CalcPixel() const
{
	double x,y,r,s,rr,ss;
	int d;

	switch (Mdl->Type) {
	case emFractalFileModel::MANDELBROT_TYPE:
		x=FrcX+FrcSX*PixX;
		y=FrcY+FrcSY*PixY;
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
		r=FrcX+FrcSX*PixX;
		s=FrcY+FrcSY*PixY;
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
		x=floor((FrcX+FrcSX*PixX)*100)/100;
		y=floor((FrcY+FrcSY*PixY)*100)/100;
		r=(x-(FrcX+FrcSX*PixX))*100*3.4+1.7;
		s=(y-(FrcY+FrcSY*PixY))*100*3.4+1.7;
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


void emFractalFilePanel::PutPixel(emColor color)
{
	emByte * p, * pxe;
	int w,h,s,t;

	w=Image.GetWidth();
	h=Image.GetHeight();
	s=PixStep;
	if (s>32) s=32;
	t=s;
	if (s>w-PixX) s=w-PixX;
	if (t>h-PixY) t=h-PixY;
	if (InvX1>PixX) InvX1=PixX;
	if (InvY1>PixY) InvY1=PixY;
	if (InvX2<PixX+s) InvX2=PixX+s;
	if (InvY2<PixY+t) InvY2=PixY+t;
	w*=3;
	s*=3;
	p=Image.GetWritableMap()+PixY*w+PixX*3;
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
