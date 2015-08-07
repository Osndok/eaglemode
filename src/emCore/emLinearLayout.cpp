//------------------------------------------------------------------------------
// emLinearLayout.cpp
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

#include <emCore/emLinearLayout.h>


emLinearLayout::emLinearLayout(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	OrientationThreshold=0.2;
	DefaultWeight=1.0;
	DefaultMinTallness=0.0;
	DefaultMaxTallness=1E100;
	SpaceL=0.0;
	SpaceT=0.0;
	SpaceH=0.0;
	SpaceV=0.0;
	SpaceR=0.0;
	SpaceB=0.0;
	WeightArray.SetTuningLevel(4);
	MinCTArray.SetTuningLevel(4);
	MaxCTArray.SetTuningLevel(4);
	MinCellCount=0;
	Alignment=EM_ALIGN_CENTER;
	SetFocusable(false);
}


emLinearLayout::~emLinearLayout()
{
}


void emLinearLayout::SetHorizontal()
{
	SetOrientationThresholdTallness(1E100);
}


void emLinearLayout::SetVertical()
{
	SetOrientationThresholdTallness(0.0);
}


void emLinearLayout::SetOrientationThresholdTallness(double tallness)
{
	if (OrientationThreshold!=tallness) {
		OrientationThreshold=tallness;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetMinCellCount(int minCellCount)
{
	if (minCellCount<0) minCellCount=0;
	if (MinCellCount!=minCellCount) {
		MinCellCount=minCellCount;
		InvalidateChildrenLayout();
	}
}


double emLinearLayout::GetChildWeight(int index) const
{
	if (index>=0 && index<WeightArray.GetCount()) {
		return WeightArray[index];
	}
	else {
		return DefaultWeight;
	}
}


void emLinearLayout::SetChildWeight(int index, double weight)
{
	int n;

	if (index>=0) {
		n=WeightArray.GetCount();
		if (index<n) {
			if (WeightArray[index]!=weight) {
				WeightArray.Set(index,weight);
				InvalidateChildrenLayout();
			}
		}
		else {
			if (DefaultWeight!=weight) {
				if (index>n) WeightArray.Add(DefaultWeight,index-n);
				WeightArray.Add(weight);
				InvalidateChildrenLayout();
			}
		}
	}
}


void emLinearLayout::SetChildWeight(double weight)
{
	if (DefaultWeight!=weight || !WeightArray.IsEmpty()) {
		DefaultWeight=weight;
		WeightArray.Clear();
		InvalidateChildrenLayout();
	}
}


double emLinearLayout::GetMinChildTallness(int index) const
{
	if (index>=0 && index<MinCTArray.GetCount()) {
		return MinCTArray[index];
	}
	else {
		return DefaultMinTallness;
	}
}


void emLinearLayout::SetMinChildTallness(int index, double minCT)
{
	int n;

	if (index>=0) {
		n=MinCTArray.GetCount();
		if (index<n) {
			if (MinCTArray[index]!=minCT) {
				MinCTArray.Set(index,minCT);
				InvalidateChildrenLayout();
			}
		}
		else {
			if (DefaultMinTallness!=minCT) {
				if (index>n) MinCTArray.Add(DefaultMinTallness,index-n);
				MinCTArray.Add(minCT);
				InvalidateChildrenLayout();
			}
		}
	}
}


void emLinearLayout::SetMinChildTallness(double minCT)
{
	if (DefaultMinTallness!=minCT || !MinCTArray.IsEmpty()) {
		DefaultMinTallness=minCT;
		MinCTArray.Clear();
		InvalidateChildrenLayout();
	}
}


double emLinearLayout::GetMaxChildTallness(int index) const
{
	if (index>=0 && index<MaxCTArray.GetCount()) {
		return MaxCTArray[index];
	}
	else {
		return DefaultMaxTallness;
	}
}


void emLinearLayout::SetMaxChildTallness(int index, double maxCT)
{
	int n;

	if (index>=0) {
		n=MaxCTArray.GetCount();
		if (index<n) {
			if (MaxCTArray[index]!=maxCT) {
				MaxCTArray.Set(index,maxCT);
				InvalidateChildrenLayout();
			}
		}
		else {
			if (DefaultMaxTallness!=maxCT) {
				if (index>n) MaxCTArray.Add(DefaultMaxTallness,index-n);
				MaxCTArray.Add(maxCT);
				InvalidateChildrenLayout();
			}
		}
	}
}


void emLinearLayout::SetMaxChildTallness(double maxCT)
{
	if (DefaultMaxTallness!=maxCT || !MaxCTArray.IsEmpty()) {
		DefaultMaxTallness=maxCT;
		MaxCTArray.Clear();
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetChildTallness(int index, double tallness)
{
	SetMinChildTallness(index,tallness);
	SetMaxChildTallness(index,tallness);
}


void emLinearLayout::SetChildTallness(double tallness)
{
	SetMinChildTallness(tallness);
	SetMaxChildTallness(tallness);
}


void emLinearLayout::SetAlignment(emAlignment alignment)
{
	if (Alignment!=alignment) {
		Alignment=alignment;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceL(double l)
{
	if (l<0.0) l=0.0;
	if (SpaceL!=l) {
		SpaceL=l;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceT(double t)
{
	if (t<0.0) t=0.0;
	if (SpaceT!=t) {
		SpaceT=t;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceH(double h)
{
	if (h<0.0) h=0.0;
	if (SpaceH!=h) {
		SpaceH=h;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceV(double v)
{
	if (v<0.0) v=0.0;
	if (SpaceV!=v) {
		SpaceV=v;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceR(double r)
{
	if (r<0.0) r=0.0;
	if (SpaceR!=r) {
		SpaceR=r;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpaceB(double b)
{
	if (b<0.0) b=0.0;
	if (SpaceB!=b) {
		SpaceB=b;
		InvalidateChildrenLayout();
	}
}


void emLinearLayout::SetSpace(
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


void emLinearLayout::SetSpace(double lr, double tb, double h, double v)
{
	SetSpace(lr,tb,h,v,lr,tb);
}


void emLinearLayout::SetInnerSpace(double h, double v)
{
	SetSpaceH(h);
	SetSpaceV(v);
}


void emLinearLayout::SetOuterSpace(double l, double t, double r, double b)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emLinearLayout::SetOuterSpace(double lr, double tb)
{
	SetOuterSpace(lr,tb,lr,tb);
}


void emLinearLayout::LayoutChildren()
{
	emPanel * p, * aux;
	double minCT,maxCT,x,y,w,h,t,force,weight,length;
	double cx,cy,cw,ch,sx,sy,ux,uy;
	int cells,cols,rows,i;
	bool horizontal;
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

	horizontal = (h/w <= OrientationThreshold);

	cols=(horizontal ? cells : 1);
	rows=(horizontal ? 1 : cells);
	sx=SpaceL+SpaceR+SpaceH*(cols-1);
	sy=SpaceT+SpaceB+SpaceV*(rows-1);
	ux=sx/cols+1.0;
	uy=sy/rows+1.0;

	force=CalculateForce(cells,w/ux,h/uy,horizontal);

	length=0.0;
	for (i=0; i<cells; i++) {
		weight=GetChildWeight(i);
		minCT=GetMinChildTallness(i);
		maxCT=GetMaxChildTallness(i);
		if (maxCT<minCT) maxCT=minCT;
		if (horizontal) {
			cw=weight*force;
			ch=1.0;
			if (ch<cw*minCT) {
				cw=ch/minCT;
			}
			else if (ch>cw*maxCT) {
				cw=ch/maxCT;
			}
			length+=cw;
		}
		else {
			cw=1.0;
			ch=weight*force;
			if (ch<cw*minCT) {
				ch=cw*minCT;
			}
			else if (ch>cw*maxCT) {
				ch=cw*maxCT;
			}
			length+=ch;
		}
	}

	if (horizontal) {
		cw=h/uy*ux*length;
		ch=h;
	}
	else {
		cw=w;
		ch=w/ux*uy*length;
	}

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

	for (cx=x, cy=y, i=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p==aux) continue;

		weight=GetChildWeight(i);
		minCT=GetMinChildTallness(i);
		maxCT=GetMaxChildTallness(i);
		if (maxCT<minCT) maxCT=minCT;
		if (horizontal) {
			ch=h/uy;
			cw=weight*force*ch;
			if (ch<cw*minCT) {
				cw=ch/minCT;
			}
			else if (ch>cw*maxCT) {
				cw=ch/maxCT;
			}
		}
		else {
			cw=w/ux;
			ch=weight*force*cw;
			if (ch<cw*minCT) {
				ch=cw*minCT;
			}
			else if (ch>cw*maxCT) {
				ch=cw*maxCT;
			}
		}

		p->Layout(cx,cy,cw,ch,cc);

		if (horizontal) {
			cx+=cw+sx;
		}
		else {
			cy+=ch+sy;
		}

		i++;
	}
}


double emLinearLayout::CalculateForce(int cells, double w, double h, bool horizontal)
{
	double totalLength,weight,force,minCT,maxCT,ct,cw;
	double compressedLength,expandedLength,freeLength;
	int * dynMem, * succ, * compressedListEnd, * expandedListEnd, * freeListEnd;
	int i,inputList,compressedList,expandedList,freeList;
	int autoMem[256];

	if (cells*sizeof(int)<=sizeof(autoMem)) {
		dynMem=NULL;
		succ=autoMem;
	}
	else {
		dynMem=(int*)malloc(cells*sizeof(int));
		succ=dynMem;
	}

	inputList=-1;
	for (i=cells-1; i>=0; i--) {
		succ[i]=inputList;
		inputList=i;
	}

	totalLength=(horizontal ? w/h : h/w);
	force=0.0;

	for (;;) {
		weight=0.0;
		for (i=inputList; i>=0; i=succ[i]) weight+=GetChildWeight(i);
		if (weight<1E-100) break;
		force=totalLength/weight;

		compressedLength=0.0;
		compressedList=-1;
		compressedListEnd=&compressedList;
		expandedLength=0.0;
		expandedList=-1;
		expandedListEnd=&expandedList;
		freeLength=0.0;
		freeList=-1;
		freeListEnd=&freeList;

		while (inputList>=0) {
			i=inputList;
			inputList=succ[i];
			succ[i]=-1;
			weight=GetChildWeight(i);
			minCT=GetMinChildTallness(i);
			maxCT=GetMaxChildTallness(i);
			if (maxCT<minCT) maxCT=minCT;
			if (horizontal) {
				cw=weight*force;
				ct=1.0/cw;
				if (ct<=minCT) {
					expandedLength+=1.0/minCT;
					*expandedListEnd=i;
					expandedListEnd=&succ[i];
				}
				else if (ct>=maxCT) {
					compressedLength+=1.0/maxCT;
					*compressedListEnd=i;
					compressedListEnd=&succ[i];
				}
				else {
					freeLength+=cw;
					*freeListEnd=i;
					freeListEnd=&succ[i];
				}
			}
			else {
				ct=weight*force;
				if (ct<=minCT) {
					compressedLength+=minCT;
					*compressedListEnd=i;
					compressedListEnd=&succ[i];
				}
				else if (ct>=maxCT) {
					expandedLength+=maxCT;
					*expandedListEnd=i;
					expandedListEnd=&succ[i];
				}
				else {
					freeLength+=ct;
					*freeListEnd=i;
					freeListEnd=&succ[i];
				}
			}
		}

		if (compressedList<0) {
			if (expandedList<0) {
					break;
			}
			else {
				totalLength-=expandedLength;
				inputList=freeList;
			}
		}
		else if (expandedList<0) {
			totalLength-=compressedLength;
			inputList=freeList;
		}
		else if (compressedLength+expandedLength+freeLength<totalLength) {
			totalLength-=expandedLength;
			*freeListEnd=compressedList;
			inputList=freeList;
		}
		else {
			totalLength-=compressedLength;
			*freeListEnd=expandedList;
			inputList=freeList;
		}
	}

	if (dynMem) free(dynMem);
	return force;
}
