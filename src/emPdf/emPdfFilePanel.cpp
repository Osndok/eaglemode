//------------------------------------------------------------------------------
// emPdfFilePanel.cpp
//
// Copyright (C) 2011-2014,2016 Oliver Hamann.
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

#include <emPdf/emPdfFilePanel.h>
#include <emCore/emRes.h>


emPdfFilePanel::emPdfFilePanel(
	ParentArg parent, const emString & name,
	emPdfFileModel * fileModel, bool updateFileModel
)
	: emFilePanel(parent,name,fileModel,updateFileModel)
{
	BGColor=emColor(0,0,0,0);
	FGColor=emColor(0,0,0);
	LayoutValid=false;
	ShadowImage=emGetInsResImage(GetRootContext(),"emPs","page_shadow.tga");
	AddWakeUpSignal(GetVirFileStateSignal());
	CalcLayout();
	UpdatePagePanels();
}


emPdfFilePanel::~emPdfFilePanel()
{
	DestroyPagePanels();
}


void emPdfFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (fileModel && (dynamic_cast<emPdfFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}
	if (GetFileModel() != fileModel) {
		DestroyPagePanels();
		emFilePanel::SetFileModel(fileModel,updateFileModel);
		CalcLayout();
		UpdatePagePanels();
	}
}


void emPdfFilePanel::SetBGColor(emColor bgColor)
{
	if (BGColor!=bgColor) {
		BGColor=bgColor;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emPdfFilePanel::SetFGColor(emColor fgColor)
{
	if (FGColor!=fgColor) {
		FGColor=fgColor;
		InvalidatePainting();
	}
}


emString emPdfFilePanel::GetIconFileName() const
{
	return "document.tga";
}


bool emPdfFilePanel::Cycle()
{
	if (IsSignaled(GetVirFileStateSignal())) {
		CalcLayout();
		UpdatePagePanels();
	}
	return emFilePanel::Cycle();
}


void emPdfFilePanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if (flags&(NF_LAYOUT_CHANGED|NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED)) {
		if (LayoutValid && (flags&NF_LAYOUT_CHANGED)!=0) {
			CalcLayout();
		}
		UpdatePagePanels();
	}
}


bool emPdfFilePanel::IsOpaque() const
{
	if (!IsVFSGood() || !LayoutValid) {
		return emFilePanel::IsOpaque();
	}
	else {
		return false;
	}
}


void emPdfFilePanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	emPdfFileModel * fm;
	double f,cx,cy,bx1,by1,bx2,by2,tx,ty,tw,th,pw,ph;
	int i,n;

	if (!IsVFSGood() || !LayoutValid) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	fm=(emPdfFileModel*)GetFileModel();
	if (!BGColor.IsTotallyTransparent()) {
		painter.Clear(BGColor,canvasColor);
		canvasColor=BGColor;
	}
	n=fm->GetPageCount();
	for (i=0; i<n; i++) {
		cx=CellX0+CellW*(i/Rows);
		cy=CellY0+CellH*(i%Rows);
		pw=fm->GetPageWidth(i)*PerPoint;
		ph=fm->GetPageHeight(i)*PerPoint;
		if (i<PagePanels.GetCount() && PagePanels[i]) {
			f=ShadowSize/151.0;
			bx1=cx+PgX-64.0*f;
			by1=cy+PgY-63.0*f;
			bx2=cx+PgX+pw+131.0*f;
			by2=cy+PgY+ph+151.0*f;
			painter.PaintBorderShape(
				bx1,by1,bx2-bx1,by2-by1,
				337.0*f,337.0*f,391.0*f,410.0*f,
				ShadowImage,
				337.0,337.0,391.0,410.0,
				0,emColor(0,0,0,180),canvasColor,0757
			);
			if (n>1) {
				tx=cx;
				ty=cy+PgY;
				tw=emMin(PgX*0.94,bx1-tx);
				th=emMin(ph,tw*0.6);
				painter.PaintTextBoxed(
					tx,
					ty,
					tw,
					th,
					fm->GetPageLabel(i),
					th,
					FGColor,
					canvasColor,
					EM_ALIGN_TOP_RIGHT
				);
			}
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


void emPdfFilePanel::LayoutChildren()
{
	emPdfFileModel * fm;
	emColor cc;
	int i,n;

	if (!IsVFSGood() || !LayoutValid) return;
	if (!BGColor.IsTotallyTransparent()) cc=BGColor;
	else cc=GetCanvasColor();
	fm=(emPdfFileModel*)GetFileModel();
	n=fm->GetPageCount();
	for (i=0; i<n; i++) {
		if (i<PagePanels.GetCount() && PagePanels[i]) {
			PagePanels[i]->Layout(
				CellX0+CellW*(i/Rows)+PgX,
				CellY0+CellH*(i%Rows)+PgY,
				fm->GetPageWidth(i)*PerPoint,
				fm->GetPageHeight(i)*PerPoint,
				cc
			);
		}
	}
}


emPanel * emPdfFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	return NULL;
}


void emPdfFilePanel::CalcLayout()
{
	emPdfFileModel * fm;
	double w,h,bestf,f,pgW,pgH;
	int i,n,nr,nc,bestnr;

	if (!IsVFSGood()) {
		if (LayoutValid) {
			LayoutValid=false;
			InvalidatePainting();
		}
		return;
	}

	fm=(emPdfFileModel*)GetFileModel();
	n=fm->GetPageCount();
	if (n<1) {
		n=1;
		pgW=1.0;
		pgH=1.0;
	}
	else {
		pgW=0.0;
		pgH=0.0;
		for (i=0; i<n; i++) {
			w=fm->GetPageWidth(i);
			h=fm->GetPageHeight(i);
			if (pgW<w) pgW=w;
			if (pgH<h) pgH=h;
		}
	}

	f=(pgW+pgH)*0.06;
	CellW=pgW+f;
	CellH=pgH+f;
	PgX=f*0.5;
	PgY=f*0.5;
	ShadowSize=emMin(pgW,pgH)*0.04;
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
	LayoutValid=true;
	InvalidatePainting();
	InvalidateChildrenLayout();
}


void emPdfFilePanel::CreatePagePanels()
{
	emPdfFileModel * fm;
	char name[256];
	int i,n;

	if (!IsVFSGood() || !LayoutValid) return;

	if (PagePanels.IsEmpty()) {
		fm=(emPdfFileModel*)GetFileModel();
		n=fm->GetPageCount();
		for (i=0; i<n; i++) {
			sprintf(name,"%d",i);
			PagePanels.Add(new emPdfPagePanel(this,name,fm,i));
		}
	}
}


void emPdfFilePanel::DestroyPagePanels()
{
	int i;

	if (!PagePanels.IsEmpty()) {
		for (i=0; i<PagePanels.GetCount(); i++) {
			if (PagePanels[i]) delete PagePanels[i];
		}
		PagePanels.Clear();
	}
}


bool emPdfFilePanel::ArePagePanelsToBeShown()
{
	emPdfFileModel * fm;
	double w,h;

	if (!IsVFSGood()) return false;
	if (!LayoutValid) return false;
	fm=(emPdfFileModel*)GetFileModel();
	if (fm->GetPageCount()<1) return false;
	if (GetSoughtName()) return true;
	if (!IsViewed()) return IsInViewedPath();
	w=PanelToViewDeltaX(CellW);
	h=PanelToViewDeltaY(CellH);
	if (w<5.0 || h<5.0 || w*h<36.0) return false;
	return true;
}


void emPdfFilePanel::UpdatePagePanels()
{
	if (ArePagePanelsToBeShown()) {
		CreatePagePanels();
	}
	else {
		DestroyPagePanels();
	}
}
