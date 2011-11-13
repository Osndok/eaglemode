//------------------------------------------------------------------------------
// emPsDocumentPanel.cpp
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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

#include <emPs/emPsDocumentPanel.h>


emPsDocumentPanel::emPsDocumentPanel(
	ParentArg parent, const emString & name, const emPsDocument & document
) :
	emPanel(parent,name)
{
	BGColor=emColor(0,0,0,0);
	FGColor=emColor(0,0,0);
	PagePanels=NULL;
	CalcLayout();
	SetDocument(document);
}


emPsDocumentPanel::~emPsDocumentPanel()
{
	DestroyPagePanels();
}


void emPsDocumentPanel::SetDocument(const emPsDocument & document)
{
	if (Document!=document) {
		DestroyPagePanels();
		Document=document;
		CalcLayout();
		if (ArePagePanelsToBeShown()) CreatePagePanels();
	}
}


void emPsDocumentPanel::SetBGColor(emColor bgColor)
{
	if (BGColor!=bgColor) {
		BGColor=bgColor;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emPsDocumentPanel::SetFGColor(emColor fgColor)
{
	if (FGColor!=fgColor) {
		FGColor=fgColor;
		InvalidatePainting();
	}
}


void emPsDocumentPanel::Notice(NoticeFlags flags)
{
	emPanel::Notice(flags);
	if (flags&(NF_LAYOUT_CHANGED|NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED)) {
		if (flags&NF_LAYOUT_CHANGED) {
			CalcLayout();
			InvalidateChildrenLayout();
		}
		if (ArePagePanelsToBeShown()) {
			if (!PagePanels) CreatePagePanels();
		}
		else {
			if (PagePanels) DestroyPagePanels();
		}
	}
}


bool emPsDocumentPanel::IsOpaque()
{
	return BGColor.IsOpaque();
}


void emPsDocumentPanel::Paint(
	const emPainter & painter, emColor canvasColor
)
{
	double xy[6*2];
	double cx,cy,tx,ty,tw,th,pw,ph;
	int i,n;

	if (!BGColor.IsTotallyTransparent()) {
		painter.Clear(BGColor,canvasColor);
		canvasColor=BGColor;
	}
	n=Document.GetPageCount();
	for (i=0; i<n; i++) {
		cx=CellX0+CellW*(i/Rows);
		cy=CellY0+CellH*(i%Rows);
		pw=Document.GetPageWidth(i)*PerPoint;
		ph=Document.GetPageHeight(i)*PerPoint;
		if (PagePanels) {
			if (n>1) {
				tx=cx;
				ty=cy+PgY;
				tw=PgX*0.94;
				th=emMin(ph,tw*0.6);
				painter.PaintTextBoxed(
					tx,
					ty,
					tw,
					th,
					Document.GetPageLabel(i),
					th,
					FGColor,
					canvasColor,
					EM_ALIGN_TOP_RIGHT
				);
			}
			xy[ 0]=cx+PgX+pw;
			xy[ 1]=cy+PgY+ShadowSize;
			xy[ 2]=cx+PgX+pw+ShadowSize;
			xy[ 3]=cy+PgY+ShadowSize;
			xy[ 4]=cx+PgX+pw+ShadowSize;
			xy[ 5]=cy+PgY+ph+ShadowSize;
			xy[ 6]=cx+PgX+ShadowSize;
			xy[ 7]=cy+PgY+ph+ShadowSize;
			xy[ 8]=cx+PgX+ShadowSize;
			xy[ 9]=cy+PgY+ph;
			xy[10]=cx+PgX+pw;
			xy[11]=cy+PgY+ph;
			painter.PaintPolygon(xy,6,emColor(0,0,0,160),canvasColor);
		}
		else {
			painter.PaintRect(
				cx+PgX,
				cy+PgY,
				pw,
				ph,
				emColor(221,221,221),
				canvasColor
			);
		}
	}
}


void emPsDocumentPanel::LayoutChildren()
{
	emColor cc;
	int i,n;

	if (!PagePanels) return;
	if (!BGColor.IsTotallyTransparent()) cc=BGColor;
	else cc=GetCanvasColor();
	n=Document.GetPageCount();
	for (i=0; i<n; i++) {
		if (PagePanels[i]) {
			PagePanels[i]->Layout(
				CellX0+CellW*(i/Rows)+PgX,
				CellY0+CellH*(i%Rows)+PgY,
				Document.GetPageWidth(i)*PerPoint,
				Document.GetPageHeight(i)*PerPoint,
				cc
			);
		}
	}
}


void emPsDocumentPanel::CalcLayout()
{
	double w,h,bestf,f,pgW,pgH;
	int n,nr,nc,bestnr;

	n=Document.GetPageCount();
	if (n<1) {
		n=1;
		pgW=1.0;
		pgH=1.0;
	}
	else {
		pgW=Document.GetMaxPageWidth();
		pgH=Document.GetMaxPageHeight();
	}

	f=(pgW+pgH)*0.06;
	CellW=pgW+f;
	CellH=pgH+f;
	PgX=f*0.5;
	PgY=f*0.5;
	ShadowSize=emMin(pgW,pgH)*0.02;
	if (n>1) {
		f*=2;
		CellW+=f;
		PgX+=f;
	}

	w=1.0;
	h=GetHeight();
	f=emMin(w,h)*0.02;
	w-=f;
	h-=f;

	bestnr=1;
	bestf=0.0;
	for (nr=1;;) {
		nc=(n+nr-1)/nr;
		f=emMin(h/(nr*CellH),w/(nc*CellW));
		if (nr==1 || f>bestf) {
			bestnr=nr;
			bestf=f;
		}
		if (nc==1) break;
		nr=(n+nc-2)/(nc-1);
	}
	Rows=bestnr;
	Columns=(n+Rows-1)/Rows;
	PerPoint=bestf;
	CellW*=PerPoint;
	CellH*=PerPoint;
	PgX*=PerPoint;
	PgY*=PerPoint;
	ShadowSize*=PerPoint;
	CellX0=(1.0-CellW*Columns)*0.5;
	CellY0=(GetHeight()-CellH*Rows)*0.5;
}


void emPsDocumentPanel::CreatePagePanels()
{
	char name[256];
	int i,n;

	if (PagePanels) return;
	n=Document.GetPageCount();
	if (n>0) {
		PagePanels=new emPsPagePanel*[n];
		for (i=0; i<n; i++) {
			sprintf(name,"%d",i);
			PagePanels[i]=new emPsPagePanel(this,name,Document,i);
		}
	}
}


void emPsDocumentPanel::DestroyPagePanels()
{
	int i,n;

	if (!PagePanels) return;
	n=Document.GetPageCount();
	for (i=0; i<n; i++) {
		if (PagePanels[i]) delete PagePanels[i];
	}
	delete [] PagePanels;
	PagePanels=NULL;
}


bool emPsDocumentPanel::ArePagePanelsToBeShown()
{
	double w,h;

	if (Document.GetPageCount()<1) return false;
	if (GetSoughtName()) return true;
	if (!IsViewed()) return IsInViewedPath();
	w=PanelToViewDeltaX(CellW);
	h=PanelToViewDeltaY(CellH);
	if (w<5.0 || h<5.0 || w*h<36.0) return false;
	return true;
}
