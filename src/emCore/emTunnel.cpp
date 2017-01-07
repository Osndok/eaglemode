//------------------------------------------------------------------------------
// emTunnel.cpp
//
// Copyright (C) 2005-2011,2014,2016 Oliver Hamann.
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

#include <emCore/emTunnel.h>


emTunnel::emTunnel(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	ChildTallness=0.0;
	Depth=10.0;
	SetBorderType(OBT_INSTRUMENT,IBT_GROUP);
}


void emTunnel::SetChildTallness(double childTallness)
{
	if (ChildTallness!=childTallness) {
		ChildTallness=childTallness;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTunnel::SetDepth(double depth)
{
	if (depth<1E-10) depth=1E-10;
	if (Depth!=depth) {
		Depth=depth;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTunnel::GetChildRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
) const
{
	DoTunnel(TUNNEL_FUNC_CHILD_RECT,NULL,0,pX,pY,pW,pH,pCanvasColor);
}


void emTunnel::GetChildRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	((const emTunnel*)this)->GetChildRect(pX,pY,pW,pH,pCanvasColor);
}


void emTunnel::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	DoTunnel(TUNNEL_FUNC_PAINT,&painter,canvasColor,NULL,NULL,NULL,NULL,NULL);
}


void emTunnel::LayoutChildren()
{
	emPanel * p, * aux;
	double x,y,w,h;
	emColor cc;

	emBorder::LayoutChildren();

	p=GetFirstChild();
	if (!p) return;

	aux=GetAuxPanel();
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}

	GetChildRect(&x,&y,&w,&h,&cc);
	p->Layout(x,y,w,h,cc);
}


void emTunnel::DoTunnel(
	DoTunnelFunc func, const emPainter * painter, emColor canvasColor,
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
) const
{
	double d,f,ax,ay,aw,ah,ar,bx,by,bw,bh,br,circleQuality,imgRY,imgRX,dx,dy;
	double xy[4*2];
	int i,n,m,ja,jb;
	const emImage * img;
	emColor cc;

	GetContentRoundRect(&ax,&ay,&aw,&ah,&ar,&cc);

	d=1.0/(Depth+1.0);
	bw=aw*d;
	bh=ah*d;
	br=ar*d;
	if (ChildTallness>1E-100) {
		bw=sqrt((bw-br)*(bh-br)/ChildTallness);
		bh=bw*ChildTallness;
		br=ar/(emMin(aw,ah)-ar)*emMin(bw,bh);
		bw+=br;
		bh+=br;
		f=aw*0.999999/bw;
		if (f<1.0) { bw*=f; bh*=f; br*=f; }
		f=ah*0.999999/bh;
		if (f<1.0) { bw*=f; bh*=f; br*=f; }
	}
	bx=ax+(aw-bw)*0.5;
	by=ay+(ah-bh)*0.5;

	if (func==TUNNEL_FUNC_CHILD_RECT) {
		if (pX) *pX=bx+0.5*br;
		if (pY) *pY=by+0.5*br;
		if (pW) *pW=bw-br;
		if (pH) *pH=bh-br;
		if (pCanvasColor) *pCanvasColor=cc;
		return;
	}

	img=&GetTkResources().ImgTunnel;
	imgRX=img->GetWidth()*0.5;
	imgRY=img->GetHeight()*0.5;

	circleQuality=4.5;
	f=circleQuality*sqrt(ar*(painter->GetScaleX()+painter->GetScaleY()));
	if (f>256.0) f=256.0;
	f*=0.25;
	if (f<=1.0) n=1;
	else if (f>=64.0) n=64;
	else n=(int)(f+0.5);
	for (i=0, m=n*4, ja=0, jb=1; i<=m; i++, ja^=3, jb^=3) {
		f=(i+0.5)*(M_PI*2.0)/m;
		dx=cos(f);
		dy=sin(f);
		if ((i/n+1)&2) {
			xy[ja*2]=ax+(dx+1.0)*ar;
			xy[jb*2]=bx+(dx+1.0)*br;
		}
		else {
			xy[ja*2]=ax+aw+(dx-1.0)*ar;
			xy[jb*2]=bx+bw+(dx-1.0)*br;
		}
		if ((i/n)&2) {
			xy[ja*2+1]=ay+(dy+1.0)*ar;
			xy[jb*2+1]=by+(dy+1.0)*br;
		}
		else {
			xy[ja*2+1]=ay+ah+(dy-1.0)*ar;
			xy[jb*2+1]=by+bh+(dy-1.0)*br;
		}
		if (i>0) {
			f=i*(M_PI*2.0)/m;
			dx=cos(f);
			dy=sin(f);
			painter->PaintPolygon(
				xy,4,
				img->GetPixel(
					(int)(imgRX+(imgRX-0.6)*dx+0.5),
					(int)(imgRY+(imgRY-0.6)*dy+0.5)
				),
				canvasColor
			);
		}
	}
}
