//------------------------------------------------------------------------------
// emPackLayout.cpp
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

#include <emCore/emPackLayout.h>


emPackLayout::emPackLayout(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	DefaultWeight=1.0;
	DefaultPCT=0.2;
	WeightArray.SetTuningLevel(4);
	PCTArray.SetTuningLevel(4);
	MinCellCount=0;
	TI=NULL;
	Ratings=0;
	SetFocusable(false);
}


emPackLayout::~emPackLayout()
{
}


void emPackLayout::SetMinCellCount(int minCellCount)
{
	if (minCellCount<0) minCellCount=0;
	if (MinCellCount!=minCellCount) {
		MinCellCount=minCellCount;
		InvalidateChildrenLayout();
	}
}


double emPackLayout::GetChildWeight(int index) const
{
	if (index>=0 && index<WeightArray.GetCount()) {
		return WeightArray[index];
	}
	else {
		return DefaultWeight;
	}
}


void emPackLayout::SetChildWeight(int index, double weight)
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


void emPackLayout::SetChildWeight(double weight)
{
	if (DefaultWeight!=weight || !WeightArray.IsEmpty()) {
		DefaultWeight=weight;
		WeightArray.Clear();
		InvalidateChildrenLayout();
	}
}


double emPackLayout::GetPrefChildTallness(int index) const
{
	if (index>=0 && index<PCTArray.GetCount()) {
		return PCTArray[index];
	}
	else {
		return DefaultPCT;
	}
}


void emPackLayout::SetPrefChildTallness(int index, double pct)
{
	int n;

	if (index>=0) {
		n=PCTArray.GetCount();
		if (index<n) {
			if (PCTArray[index]!=pct) {
				PCTArray.Set(index,pct);
				InvalidateChildrenLayout();
			}
		}
		else {
			if (DefaultPCT!=pct) {
				if (index>n) PCTArray.Add(DefaultPCT,index-n);
				PCTArray.Add(pct);
				InvalidateChildrenLayout();
			}
		}
	}
}


void emPackLayout::SetPrefChildTallness(double pct)
{
	if (DefaultPCT!=pct || !PCTArray.IsEmpty()) {
		DefaultPCT=pct;
		PCTArray.Clear();
		InvalidateChildrenLayout();
	}
}


void emPackLayout::LayoutChildren()
{
	TmpPanelInfo autoMem[64];
	TmpPanelInfo * dynMem;
	TmpInfo ti;
	double x,y,w,h;
	int cells,tpiCnt;
	emColor cc;

	emBorder::LayoutChildren();

	cells=CountCells();
	if (cells>0) {
		TI=&ti;

		tpiCnt=cells+1;
		if (tpiCnt*sizeof(TmpPanelInfo)<=sizeof(autoMem)) {
			dynMem=NULL;
			TI->TPIs=autoMem;
		}
		else {
			dynMem=(TmpPanelInfo*)malloc(tpiCnt*sizeof(TmpPanelInfo));
			TI->TPIs=dynMem;
		}
		FillTPIs(tpiCnt);

		GetContentRectUnobscured(&x,&y,&w,&h,&TI->CanvasColor);
		if (w<1E-100) w=1E-100;
		if (h<1E-100) h=1E-100;

		Ratings=0;

		PackN(0, cells, x, y, w, h, 1E100, true);

		emDLog(
			"emPackLayout::LayoutChildren: cells = %d, ratings = %d, ratings/cell = %g",
			cells, Ratings, cells>0 ? ((double)Ratings)/cells : 0.0
		);

		TI=NULL;
		if (dynMem) free(dynMem);
	}
}


int emPackLayout::CountCells()
{
	emPanel * p, * aux;
	int cells;

	aux=GetAuxPanel();
	for (cells=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p!=aux) cells++;
	}
	if (cells<MinCellCount) cells=MinCellCount;
	return cells;
}


void emPackLayout::FillTPIs(int count)
{
	double pct,cumulativeWeight,cumulativeLogPCT;
	emPanel * p, * aux;
	int i;

	pct=1.0;
	cumulativeWeight=0.0;
	cumulativeLogPCT=0.0;
	for (i=0; i<count; i++) {
		if (i>0) {
			cumulativeWeight+=GetChildWeight(i-1);
			cumulativeLogPCT+=log(pct);
		}
		pct=emMax(1E-4,GetPrefChildTallness(i));
		TI->TPIs[i].PCT=pct;
		TI->TPIs[i].CumulativeWeight=cumulativeWeight;
		TI->TPIs[i].CumulativeLogPCT=cumulativeLogPCT;
		TI->TPIs[i].Panel=NULL;
	}

	aux=GetAuxPanel();
	for (i=0, p=GetFirstChild(); p && i<count; p=p->GetNext()) {
		if (p!=aux) TI->TPIs[i++].Panel=p;
	}
}


double emPackLayout::GetTPIWeightSum(int index, int count) const
{
	return
		TI->TPIs[index+count].CumulativeWeight -
		TI->TPIs[index].CumulativeWeight
	;
}


double emPackLayout::GetTPILogPCTSum(int index, int count) const
{
	return (
		TI->TPIs[index+count].CumulativeLogPCT -
		TI->TPIs[index].CumulativeLogPCT
	);
}


double emPackLayout::RateCell(int index, double w, double h)
{
	double error;

	Ratings++;

	error=w/h*TI->TPIs[index].PCT;
	if (error<1.0) error=1.0/error;
	error=pow(error,3.0)-1.0;
	return error;
}


double emPackLayout::Pack1(
	int index,
	double x, double y, double w, double h,
	bool execute
)
{
	emPanel * p;

	if (execute) {
		p=TI->TPIs[index].Panel;
		if (p) p->Layout(x,y,w,h,TI->CanvasColor);
	}
	return RateCell(index,w,h);
}


double emPackLayout::Pack2(
	int index,
	double x, double y, double w, double h,
	double bestError, bool execute
)
{
	double s1,w1,h1,error;
	emPanel * p;
	int solution;

	s1=GetTPIWeightSum(index,1)/GetTPIWeightSum(index,2);
	w1=w*s1;
	h1=h*s1;

	solution=-1;

	error=RateCell(index,w1,h);
	if (error<bestError) {
		error+=RateCell(index+1,w-w1,h);
		if (error<bestError) {
			bestError=error;
			solution=0;
		}
	}
	error=RateCell(index,w,h1);
	if (error<bestError) {
		error+=RateCell(index+1,w,h-h1);
		if (error<bestError) {
			bestError=error;
			solution=1;
		}
	}

	if (execute) {
		switch (solution) {
		case 0:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w1,h,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x+w1,y,w-w1,h,TI->CanvasColor);
			break;
		default:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w,h1,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x,y+h1,w,h-h1,TI->CanvasColor);
			break;
		}
	}

	return solution>=0 ? bestError : 1E100;
}


double emPackLayout::Pack3(
	int index,
	double x, double y, double w, double h,
	double bestError, bool execute
)
{
	double f,a1,a2,a3,w1,w2,w3,w12,w23,h1,h2,h3,h12,h23,error,e1,e3;
	emPanel * p;
	int solution;

	f=w*h/GetTPIWeightSum(index,3);
	a1=GetTPIWeightSum(index,1)*f;
	a2=GetTPIWeightSum(index+1,1)*f;
	a3=GetTPIWeightSum(index+2,1)*f;

	solution=-1;

	w1=a1/h;
	w2=a2/h;
	w3=a3/h;
	w12=w1+w2;
	w23=w2+w3;
	e1=RateCell(index,w1,h);
	e3=RateCell(index+2,w3,h);
	if (e1<bestError) {
		error=e1+e3;
		if (error<bestError) {
			error+=RateCell(index+1,w2,h);
			if (error<bestError) {
				bestError=error;
				solution=0;
			}
		}
		error=e1+RateCell(index+1,w23,a2/w23);
		if (error<bestError) {
			error+=RateCell(index+2,w23,a3/w23);
			if (error<bestError) {
				bestError=error;
				solution=1;
			}
		}
	}
	if (e3<bestError) {
		error=e3+RateCell(index,w12,a1/w12);
		if (error<bestError) {
			error+=RateCell(index+1,w12,a2/w12);
			if (error<bestError) {
				bestError=error;
				solution=2;
			}
		}
	}

	h1=a1/w;
	h2=a2/w;
	h3=a3/w;
	h12=h1+h2;
	h23=h2+h3;
	e1=RateCell(index,w,h1);
	e3=RateCell(index+2,w,h3);
	if (e1<bestError) {
		error=e1+e3;
		if (error<bestError) {
			error+=RateCell(index+1,w,h2);
			if (error<bestError) {
				bestError=error;
				solution=3;
			}
		}
		error=e1+RateCell(index+1,a2/h23,h23);
		if (error<bestError) {
			error+=RateCell(index+2,a3/h23,h23);
			if (error<bestError) {
				bestError=error;
				solution=4;
			}
		}
	}
	if (e3<bestError) {
		error=e3+RateCell(index,a1/h12,h12);
		if (error<bestError) {
			error+=RateCell(index+1,a2/h12,h12);
			if (error<bestError) {
				bestError=error;
				solution=5;
			}
		}
	}

	if (execute) {
		switch (solution) {
		case 0:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w1,h,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x+w1,y,w2,h,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x+w12,y,w3,h,TI->CanvasColor);
			break;
		case 1:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w1,h,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x+w1,y,w23,a2/w23,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x+w1,y+a2/w23,w23,a3/w23,TI->CanvasColor);
			break;
		case 2:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w12,a1/w12,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x,y+a1/w12,w12,a2/w12,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x+w12,y,w3,h,TI->CanvasColor);
			break;
		case 3:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w,h1,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x,y+h1,w,h2,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x,y+h12,w,h3,TI->CanvasColor);
			break;
		case 4:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,w,h1,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x,y+h1,a2/h23,h23,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x+a2/h23,y+h1,a3/h23,h23,TI->CanvasColor);
			break;
		default:
			p=TI->TPIs[index].Panel;
			if (p) p->Layout(x,y,a1/h12,h12,TI->CanvasColor);
			p=TI->TPIs[index+1].Panel;
			if (p) p->Layout(x+a1/h12,y,a2/h12,h12,TI->CanvasColor);
			p=TI->TPIs[index+2].Panel;
			if (p) p->Layout(x,y+h12,w,h3,TI->CanvasColor);
			break;
		}
	}

	return solution>=0 ? bestError : 1E100;
}


double emPackLayout::PackN(
	int index, int count,
	double x, double y, double w, double h,
	double bestError, bool execute
)
{
	double error,totalWeight,s1,w1,h1;
	int i,n,div,bestDiv;
	bool bestHorizontal,testHorizontalFirst;

	if (count==1) {
		return Pack1(index,x,y,w,h,execute);
	}
	if (count==2) {
		return Pack2(index,x,y,w,h,bestError,execute);
	}
	if (count==3) {
		return Pack3(index,x,y,w,h,bestError,execute);
	}

	totalWeight=GetTPIWeightSum(index,count);
	testHorizontalFirst=(log(h/w) < GetTPILogPCTSum(index,count)/count);

	if      (count <=  7) n=(count-1)*2;
	else if (count ==  8) n=11;
	else if (count ==  9) n=8;
	else if (count == 10) n=6;
	else if (count == 11) n=4;
	else if (count <= 15) n=3;
	else if (count <= 20) n=2;
	else                  n=1;

	if (n<=1) {
		bestDiv=count>>1;
		bestHorizontal=testHorizontalFirst;
		bestError=1E100;
	}
	else {
		bestDiv=-1;
		bestHorizontal=testHorizontalFirst;
		for (i=0; i<n; i+=2) {
			if (i&2) div=(count+(i>>1)+1)>>1;
			else div=(count-(i>>1))>>1;
			s1=GetTPIWeightSum(index,div)/totalWeight;
			h1=h*s1;
			w1=w*s1;
			if (testHorizontalFirst) {
				error=RateHorizontally(index,count,div,x,y,w1,w-w1,h,bestError);
				if (error<bestError) {
					bestError=error;
					bestDiv=div;
					bestHorizontal=true;
				}
				if (i+1==n) break;
				error=RateVertically(index,count,div,x,y,w,h1,h-h1,bestError);
				if (error<bestError) {
					bestError=error;
					bestDiv=div;
					bestHorizontal=false;
				}
			}
			else {
				error=RateVertically(index,count,div,x,y,w,h1,h-h1,bestError);
				if (error<bestError) {
					bestError=error;
					bestDiv=div;
					bestHorizontal=false;
				}
				if (i+1==n) break;
				error=RateHorizontally(index,count,div,x,y,w1,w-w1,h,bestError);
				if (error<bestError) {
					bestError=error;
					bestDiv=div;
					bestHorizontal=true;
				}
			}
		}
	}

	if (execute) {
		div=bestDiv;
		if (div<0) div=count>>1;
		s1=GetTPIWeightSum(index,div)/totalWeight;
		bestError=bestError*1.00000001+1E-100;
		if (!bestHorizontal) {
			h1=h*s1;
			PackN(
				index, div,
				x, y, w, h1,
				bestError, true
			);
			PackN(
				index+div, count-div,
				x, y+h1, w, h-h1,
				bestError, true
			);
		}
		else {
			w1=w*s1;
			PackN(
				index, div,
				x, y, w1, h,
				bestError, true
			);
			PackN(
				index+div, count-div,
				x+w1, y, w-w1, h,
				bestError, true
			);
		}
	}

	return bestDiv>=0 ? bestError : 1E100;
}


double emPackLayout::RateHorizontally(
	int index, int count, int div,
	double x, double y, double w1, double w2, double h,
	double bestError
)
{
	double error;

	if (div<=count/2) {
		error=PackN(
			index,div,x,y,w1,h,bestError,false
		);
		if (error<bestError) {
			error+=PackN(
				index+div,count-div,x+w1,y,w2,h,bestError,false
			);
		}
	}
	else {
		error=PackN(
			index+div,count-div,x+w1,y,w2,h,bestError,false
		);
		if (error<bestError) {
			error+=PackN(
				index,div,x,y,w1,h,bestError,false
			);
		}
	}

	return error;
}


double emPackLayout::RateVertically(
	int index, int count, int div,
	double x, double y, double w, double h1, double h2,
	double bestError
)
{
	double error;

	if (div<=count/2) {
		error=PackN(
			index,div,x,y,w,h1,bestError,false
		);
		if (error<bestError) {
			error+=PackN(
				index+div,count-div,x,y+h1,w,h2,bestError,false
			);
		}
	}
	else{
		error=PackN(
			index+div,count-div,x,y+h1,w,h2,bestError,false
		);
		if (error<bestError) {
			error+=PackN(
				index,div,x,y,w,h1,bestError,false
			);
		}
	}

	return error;
}
