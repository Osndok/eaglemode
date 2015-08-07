//------------------------------------------------------------------------------
// emTiling.cpp
//
// Copyright (C) 2005-2011,2014-2015 Oliver Hamann.
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

#include <emCore/emTiling.h>


emTiling::emTiling(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	SpaceL=0.0;
	SpaceT=0.0;
	SpaceH=0.0;
	SpaceV=0.0;
	SpaceR=0.0;
	SpaceB=0.0;
	PCT=0.2;
	PCTPos.SetTuningLevel(4);
	PCTNeg.SetTuningLevel(4);
	FixedColumnCount=0;
	FixedRowCount=0;
	MinCellCount=0;
	FCTColumn=-1;
	FCTRow=-1;
	Alignment=EM_ALIGN_CENTER;
	RowByRow=0;
	SetFocusable(false);
}


emTiling::emTiling(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon,
	int notWarningDeprecatedForInternalUse
)
	: emBorder(parent,name,caption,description,icon)
{
	SpaceL=0.0;
	SpaceT=0.0;
	SpaceH=0.0;
	SpaceV=0.0;
	SpaceR=0.0;
	SpaceB=0.0;
	PCT=0.2;
	PCTPos.SetTuningLevel(4);
	PCTNeg.SetTuningLevel(4);
	FixedColumnCount=0;
	FixedRowCount=0;
	MinCellCount=0;
	FCTColumn=-1;
	FCTRow=-1;
	Alignment=EM_ALIGN_CENTER;
	RowByRow=0;
	SetFocusable(false);
}


emTiling::~emTiling()
{
}


void emTiling::SetRowByRow(bool rowByRow)
{
	if ((bool)RowByRow!=rowByRow) {
		RowByRow=rowByRow?1:0;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetFixedColumnCount(int fixedColumnCount)
{
	if (fixedColumnCount<0) fixedColumnCount=0;
	if (FixedColumnCount!=fixedColumnCount) {
		FixedColumnCount=fixedColumnCount;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetFixedRowCount(int fixedRowCount)
{
	if (fixedRowCount<0) fixedRowCount=0;
	if (FixedRowCount!=fixedRowCount) {
		FixedRowCount=fixedRowCount;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetMinCellCount(int minCellCount)
{
	if (minCellCount<0) minCellCount=0;
	if (MinCellCount!=minCellCount) {
		MinCellCount=minCellCount;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetChildTallness(double ct)
{
	SetPrefChildTallness(ct);
	SetChildTallnessForced();
}


void emTiling::SetPrefChildTallness(double pct)
{
	SetPrefChildTallness(pct,0,true);
}


double emTiling::GetPrefChildTallness(int idx) const
{
	if (idx!=0) {
		if (idx>0) {
			if (idx>PCTPos.GetCount()) idx=PCTPos.GetCount();
			if (idx>0) return PCTPos[idx-1];
		}
		else if (idx<0) {
			idx=-idx;
			if (idx>PCTNeg.GetCount()) idx=PCTNeg.GetCount();
			if (idx>0) return PCTNeg[idx-1];
		}
	}
	return PCT;
}


void emTiling::SetPrefChildTallness(double pct, int idx, bool allFurther)
{
	emArray<double> * arr;
	bool modified;
	double last;
	int n, m;

	if (pct<1E-100) pct=1E-100;

	modified=false;
	if (idx==0) {
		if (allFurther) {
			if (!PCTPos.IsEmpty()) { PCTPos.Clear(); modified=true; }
			if (!PCTNeg.IsEmpty()) { PCTNeg.Clear(); modified=true; }
		}
		if (PCT!=pct) {
			if (!allFurther) {
				if (PCTPos.IsEmpty()) PCTPos.Add(PCT);
				if (PCTNeg.IsEmpty()) PCTNeg.Add(PCT);
			}
			PCT=pct;
			modified=true;
		}
	}
	else {
		if (idx>0) arr=&PCTPos;
		else { idx=-idx; arr=&PCTNeg; }
		n=arr->GetCount();
		if (idx<n) {
			if (arr->Get(idx-1)!=pct) {
				arr->Set(idx-1,pct);
				modified=true;
			}
			if (allFurther) {
				arr->SetCount(idx);
				modified=true;
			}
		}
		else {
			last=n>0?arr->Get(n-1):PCT;
			if (pct!=last) {
				m=idx-n;
				if (!allFurther) m++;
				if (m>0) arr->Add(last,m);
				arr->Set(idx-1,pct);
				modified=true;
			}
		}
	}

	if (modified) InvalidateChildrenLayout();
}


void emTiling::SetChildTallnessForced(bool forced)
{
	SetForcedChildTallnessColumn(forced ? 0 : -1);
	SetForcedChildTallnessRow(forced ? 0 : -1);
}


void emTiling::SetForcedChildTallnessColumn(int column)
{
	if (FCTColumn!=column) {
		FCTColumn=column;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetForcedChildTallnessRow(int row)
{
	if (FCTRow!=row) {
		FCTRow=row;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetAlignment(emAlignment alignment)
{
	if (Alignment!=alignment) {
		Alignment=alignment;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceL(double l)
{
	if (l<0.0) l=0.0;
	if (SpaceL!=l) {
		SpaceL=l;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceT(double t)
{
	if (t<0.0) t=0.0;
	if (SpaceT!=t) {
		SpaceT=t;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceH(double h)
{
	if (h<0.0) h=0.0;
	if (SpaceH!=h) {
		SpaceH=h;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceV(double v)
{
	if (v<0.0) v=0.0;
	if (SpaceV!=v) {
		SpaceV=v;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceR(double r)
{
	if (r<0.0) r=0.0;
	if (SpaceR!=r) {
		SpaceR=r;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpaceB(double b)
{
	if (b<0.0) b=0.0;
	if (SpaceB!=b) {
		SpaceB=b;
		InvalidateChildrenLayout();
	}
}


void emTiling::SetSpace(
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


void emTiling::SetSpace(double lr, double tb, double h, double v)
{
	SetSpace(lr,tb,h,v,lr,tb);
}


void emTiling::SetInnerSpace(double h, double v)
{
	SetSpaceH(h);
	SetSpaceV(v);
}


void emTiling::SetOuterSpace(double l, double t, double r, double b)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emTiling::SetOuterSpace(double lr, double tb)
{
	SetOuterSpace(lr,tb,lr,tb);
}


void emTiling::LayoutChildren()
{
	emPanel * p, * aux;
	double x,y,w,h,t,err,errBest,cx,cy,cw,ch,tw,th,fx,fy,sx,sy,ux,uy;
	int n,m,cells,cols,rows,rowsBest,col,row;
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
		t=h/w;
		for (rows=1;;) {
			cols=(cells+rows-1)/rows;
			cw=1.0;
			m=PCTPos.GetCount();
			if (m) {
				n=cols-1;
				if (n>m) { n=m-1; cw+=PCT/PCTPos[n]*(cols-m); }
				while (n>0) cw+=PCT/PCTPos[--n];
			}
			else cw*=cols;
			ch=PCT;
			m=PCTNeg.GetCount();
			if (m) {
				n=rows-1;
				if (n>m) { n=m-1; ch+=PCTNeg[n]*(rows-m); }
				while (n>0) ch+=PCTNeg[--n];
			}
			else ch*=rows;
			cw=((SpaceL+SpaceR+SpaceH*(cols-1))/cols+1.0)*cw;
			ch=((SpaceT+SpaceB+SpaceV*(rows-1))/rows+1.0)*ch;
			err=fabs(log(t*cw/ch));
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

	cw=1.0;
	m=PCTPos.GetCount();
	if (m) {
		n=cols-1;
		if (n>m) { n=m-1; cw+=PCT/PCTPos[n]*(cols-m); }
		while (n>0) cw+=PCT/PCTPos[--n];
	}
	else cw*=cols;

	ch=PCT;
	m=PCTNeg.GetCount();
	if (m) {
		n=rows-1;
		if (n>m) { n=m-1; ch+=PCTNeg[n]*(rows-m); }
		while (n>0) ch+=PCTNeg[--n];
	}
	else ch*=rows;

	sx=SpaceL+SpaceR+SpaceH*(cols-1);
	sy=SpaceT+SpaceB+SpaceV*(rows-1);
	ux=sx/cols+1.0;
	uy=sy/rows+1.0;
	cw*=ux;
	ch*=uy;
	fx=w/cw;
	fy=h/ch;
	tw=w;
	th=h;
	if (FCTColumn>=0 && FCTColumn<cols) {
		if (cols==1 || (FCTRow>=0 && FCTRow<rows)) {
			if (fx>fy) { fx=fy; tw=cw*fx; }
			else if (fx<fy) { fy=fx; th=ch*fy; }
		}
		else {
			t=ux*PCT/GetPrefChildTallness(FCTColumn);
			fx=(w-t*fy)/(cw-t);
			if (fx<0.0) { fx=0.0; fy=w/t; th=ch*fy; }
		}
	}
	else if (FCTRow>=0 && FCTRow<rows) {
		if (rows==1) {
			if (fx>fy) { fx=fy; tw=cw*fx; }
			else if (fx<fy) { fy=fx; th=ch*fy; }
		}
		else {
			t=uy*GetPrefChildTallness(-FCTRow);
			fy=(h-t*fx)/(ch-t);
			if (fy<0.0) { fy=0.0; fx=h/t; tw=cw*fx; }
		}
	}

	if (Alignment&EM_ALIGN_RIGHT) x+=w-tw;
	else if (!(Alignment&EM_ALIGN_LEFT)) x+=(w-tw)*0.5;

	if (Alignment&EM_ALIGN_BOTTOM) y+=h-th;
	else if (!(Alignment&EM_ALIGN_TOP)) y+=(h-th)*0.5;

	if (sx>=1E-100) {
		sx=(tw-tw/ux)/sx;
		x+=sx*SpaceL;
		sx*=SpaceH;
	}
	else sx=0.0;

	if (sy>=1E-100) {
		sy=(th-th/uy)/sy;
		y+=sy*SpaceT;
		sy*=SpaceV;
	}
	else sy=0.0;

	for (cx=x, cy=y, row=0, col=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p==aux) continue;
		n=emMin(col,PCTPos.GetCount());
		if (n==0) cw=1.0; else cw=PCT/PCTPos[n-1];
		n=emMin(row,PCTNeg.GetCount());
		if (n==0) ch=PCT; else ch=PCTNeg[n-1];
		if (col==FCTColumn) cw*=fy; else cw*=fx;
		if (row==FCTRow) ch*=fx; else ch*=fy;
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
