//------------------------------------------------------------------------------
// emRasterLayout.cpp
//
// Copyright (C) 2015 Oliver Hamann.
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

#include <emCore/emRasterLayout.h>


emRasterLayout::emRasterLayout(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	PrefCT=0.2;
	MinCT=0.0;
	MaxCT=1E100;
	SpaceL=0.0;
	SpaceT=0.0;
	SpaceH=0.0;
	SpaceV=0.0;
	SpaceR=0.0;
	SpaceB=0.0;
	FixedColumnCount=0;
	FixedRowCount=0;
	MinCellCount=0;
	Alignment=EM_ALIGN_CENTER;
	StrictRaster=false;
	RowByRow=false;
	SetFocusable(false);
}


emRasterLayout::~emRasterLayout()
{
}


void emRasterLayout::SetRowByRow(bool rowByRow)
{
	if ((bool)RowByRow!=rowByRow) {
		RowByRow=rowByRow?1:0;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetFixedColumnCount(int fixedColumnCount)
{
	if (fixedColumnCount<0) fixedColumnCount=0;
	if (FixedColumnCount!=fixedColumnCount) {
		FixedColumnCount=fixedColumnCount;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetFixedRowCount(int fixedRowCount)
{
	if (fixedRowCount<0) fixedRowCount=0;
	if (FixedRowCount!=fixedRowCount) {
		FixedRowCount=fixedRowCount;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetMinCellCount(int minCellCount)
{
	if (minCellCount<0) minCellCount=0;
	if (MinCellCount!=minCellCount) {
		MinCellCount=minCellCount;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetPrefChildTallness(double prefCT)
{
	if (PrefCT!=prefCT) {
		PrefCT=prefCT;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetMinChildTallness(double minCT)
{
	if (MinCT!=minCT) {
		MinCT=minCT;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetMaxChildTallness(double maxCT)
{
	if (MaxCT!=maxCT) {
		MaxCT=maxCT;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetChildTallness(double tallness)
{
	if (PrefCT!=tallness || MinCT!=tallness || MaxCT!=tallness) {
		PrefCT=tallness;
		MinCT=tallness;
		MaxCT=tallness;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetStrictRaster(bool strictRaster)
{
	if (StrictRaster!=strictRaster) {
		StrictRaster=strictRaster;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetAlignment(emAlignment alignment)
{
	if (Alignment!=alignment) {
		Alignment=alignment;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceL(double l)
{
	if (l<0.0) l=0.0;
	if (SpaceL!=l) {
		SpaceL=l;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceT(double t)
{
	if (t<0.0) t=0.0;
	if (SpaceT!=t) {
		SpaceT=t;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceH(double h)
{
	if (h<0.0) h=0.0;
	if (SpaceH!=h) {
		SpaceH=h;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceV(double v)
{
	if (v<0.0) v=0.0;
	if (SpaceV!=v) {
		SpaceV=v;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceR(double r)
{
	if (r<0.0) r=0.0;
	if (SpaceR!=r) {
		SpaceR=r;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpaceB(double b)
{
	if (b<0.0) b=0.0;
	if (SpaceB!=b) {
		SpaceB=b;
		InvalidateChildrenLayout();
	}
}


void emRasterLayout::SetSpace(
	double l, double t, double h, double v, double r, double b
)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceH(h);
	SetSpaceV(v);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emRasterLayout::SetSpace(double lr, double tb, double h, double v)
{
	SetSpace(lr,tb,h,v,lr,tb);
}


void emRasterLayout::SetInnerSpace(double h, double v)
{
	SetSpaceH(h);
	SetSpaceV(v);
}


void emRasterLayout::SetOuterSpace(double l, double t, double r, double b)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emRasterLayout::SetOuterSpace(double lr, double tb)
{
	SetOuterSpace(lr,tb,lr,tb);
}


void emRasterLayout::LayoutChildren()
{
	emPanel * p, * aux;
	double minCT,maxCT,prefCT,x,y,w,h,t,err,errBest;
	double cx,cy,cw,ch,ct,sx,sy,ux,uy;
	int cells,cols,rows,rowsBest,col,row;
	emColor cc;

	emBorder::LayoutChildren();

	aux=GetAuxPanel();

	for (cells=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p!=aux) cells++;
	}
	if (!cells) return;
	if (cells<MinCellCount) cells=MinCellCount;

	GetContentRectUnobscured(&x,&y,&w,&h,&cc);
	if (w<1E-100) w=1E-100;
	if (h<1E-100) h=1E-100;

	minCT=MinCT;
	if (minCT<0.0) minCT=0.0;
	maxCT=MaxCT;
	if (maxCT<minCT) maxCT=minCT;
	prefCT=PrefCT;
	if (prefCT<minCT) prefCT=minCT;
	if (prefCT>maxCT) prefCT=maxCT;

	if (FixedColumnCount>0) {
		cols=FixedColumnCount;
		rows=(cells+cols-1)/cols;
		if (rows<FixedRowCount) rows=FixedRowCount;
	}
	else if (FixedRowCount>0) {
		rows=FixedRowCount;
		cols=(cells+rows-1)/rows;
	}
	else {
		rowsBest=1;
		errBest=0.0;
		for (rows=1;;) {
			cols=(cells+rows-1)/rows;
			sx=SpaceL+SpaceR+SpaceH*(cols-1);
			sy=SpaceT+SpaceB+SpaceV*(rows-1);
			ux=sx/cols+1.0;
			uy=sy/rows+1.0;
			ct=h*ux*cols/(w*uy*rows);
			err=fabs(log(prefCT/ct));
			if (rows==1 || err<errBest) {
				rowsBest=rows;
				errBest=err;
			}
			if (cols==1) break;
			rows=(cells+cols-2)/(cols-1); // skip rows until cols decreases
		}
		rows=rowsBest;
		cols=(cells+rows-1)/rows;
	}

	sx=SpaceL+SpaceR+SpaceH*(cols-1);
	sy=SpaceT+SpaceB+SpaceV*(rows-1);
	ux=sx/cols+1.0;
	uy=sy/rows+1.0;
	ct=h*ux*cols/(w*uy*rows);

	if (StrictRaster) {
		if (RowByRow) {
			if (FixedColumnCount<=0) {
				while (cols<cells && ct<minCT) {
					cols++;
					rows=(cells+cols-1)/cols;
					if (rows<FixedRowCount) rows=FixedRowCount;
					sx=SpaceL+SpaceR+SpaceH*(cols-1);
					sy=SpaceT+SpaceB+SpaceV*(rows-1);
					ux=sx/cols+1.0;
					uy=sy/rows+1.0;
					ct=h*ux*cols/(w*uy*rows);
				}
			}
		}
		else {
			if (FixedRowCount<=0) {
				while (rows<cells && ct>maxCT) {
					rows++;
					cols=(cells+rows-1)/rows;
					if (cols<FixedColumnCount) cols=FixedColumnCount;
					sx=SpaceL+SpaceR+SpaceH*(cols-1);
					sy=SpaceT+SpaceB+SpaceV*(rows-1);
					ux=sx/cols+1.0;
					uy=sy/rows+1.0;
					ct=h*ux*cols/(w*uy*rows);
				}
			}
		}
	}

	if (ct<minCT) ct=minCT;
	else if (ct>maxCT) ct=maxCT;
	cw=cols*ux;
	ch=ct*rows*uy;

	if (w*ch >= h*cw) {
		t=h*cw/ch;
		if (Alignment&EM_ALIGN_RIGHT) x+=w-t;
		else if (!(Alignment&EM_ALIGN_LEFT)) x+=(w-t)*0.5;
		w=t;
	}
	else {
		t=w*ch/cw;
		if (Alignment&EM_ALIGN_BOTTOM) y+=h-t;
		else if (!(Alignment&EM_ALIGN_TOP)) y+=(h-t)*0.5;
		h=t;
	}

	if (sx>=1E-100) {
		sx=(w-w/ux)/sx;
		x+=sx*SpaceL;
		sx*=SpaceH;
	}
	else sx=0.0;

	if (sy>=1E-100) {
		sy=(h-h/uy)/sy;
		y+=sy*SpaceT;
		sy*=SpaceV;
	}
	else sy=0.0;

	cw=w/cols/ux;
	ch=h/rows/uy;

	for (cx=x, cy=y, row=0, col=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p==aux) continue;
		p->Layout(cx,cy,cw,ch,cc);
		if (RowByRow) {
			cx+=cw+sx;
			col++;
			if (col>=cols) {
				row++;
				cy+=ch+sy;
				col=0;
				cx=x;
			}
		}
		else {
			cy+=ch+sy;
			row++;
			if (row>=rows) {
				col++;
				cx+=cw+sx;
				row=0;
				cy=y;
			}
		}
	}
}
