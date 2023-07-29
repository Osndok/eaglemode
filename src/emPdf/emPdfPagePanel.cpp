//------------------------------------------------------------------------------
// emPdfPagePanel.cpp
//
// Copyright (C) 2011,2014,2016,2020-2023 Oliver Hamann.
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

#include <emPdf/emPdfPagePanel.h>
#include <emCore/emDialog.h>
#include <emCore/emRes.h>


emPdfPagePanel::emPdfPagePanel(
	ParentArg parent, const emString & name,
	emPdfFileModel * fileModel, int pageIndex,
	emPdfSelection & selection
) :
	emPanel(parent,name),
	Selection(selection),
	IconState(IS_NONE),
	CurrentMX(0.0),
	CurrentMY(0.0),
	CurrentRectType(RT_NONE),
	CurrentRectIndex(-1),
	PressedRectType(RT_NONE),
	PressedRectIndex(-1),
	ForceTextCursor(false)
{
	Server=fileModel->GetServerModel();
	FileModel=fileModel;
	PageIndex=pageIndex;

	Layers[LT_PREVIEW].Type=LT_PREVIEW;
	Layers[LT_CONTENT].Type=LT_CONTENT;
	Layers[LT_SELECTION].Type=LT_SELECTION;

	WaitIcon=emGetInsResImage(GetRootContext(),"emPs","waiting.tga");
	RenderIcon=emGetInsResImage(GetRootContext(),"emPs","rendering.tga");
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(Selection.GetSelectionSignal());
	AddWakeUpSignal(FileModel->GetPageAreasMap().GetPageAreasSignal());

	WakeUp();
}


emPdfPagePanel::~emPdfPagePanel()
{
	int i;

	for (i=0; i<3; i++) {
		ResetLayer(Layers[i], true);
	}
}


bool emPdfPagePanel::Cycle()
{
	int i;
	bool busy;

	busy=emPanel::Cycle();

	if (IsSignaled(FileModel->GetChangeSignal())) {
		for (i=0; i<3; i++) {
			ResetLayer(Layers[i], true);
		}
		if (CurrentRectType!=RT_NONE) {
			CurrentRectType=RT_NONE;
			InvalidateCursor();
		}
		PressedRectType=RT_NONE;
	}

	if (
		IsSignaled(Selection.GetSelectionSignal()) &&
		PageSelection!=Selection.GetPageSelection(PageIndex)
	) {
		PageSelection=Selection.GetPageSelection(PageIndex);
		Layers[LT_SELECTION].ContentUpToDate=false;
	}

	if (IsSignaled(FileModel->GetPageAreasMap().GetPageAreasSignal())) {
		UpdateCurrentRect();
	}

	for (i=0; i<3; i++) {
		if (UpdateLayer(Layers[i])) busy=true;
	}

	UpdateIconState();

	return busy;
}


void emPdfPagePanel::Notice(NoticeFlags flags)
{
	int i;

	emPanel::Notice(flags);

	if ((flags&NF_VIEWING_CHANGED)!=0) {
		Layers[LT_CONTENT].CoordinatesUpToDate=false;
		if (PageSelection.NonEmpty) {
			Layers[LT_SELECTION].CoordinatesUpToDate=false;
		}
		WakeUp();
	}
	if ((flags&NF_UPDATE_PRIORITY_CHANGED)!=0) {
		for (i=0; i<3; i++) {
			if (Layers[i].Job) {
				Server->SetJobPriority(Layers[i].Job,GetUpdatePriority());
			}
		}
	}
}


void emPdfPagePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	bool forceTextCursor;

	CurrentMX=mx;
	CurrentMY=my;
	UpdateCurrentRect();

	if (
		PageIndex<0 || PageIndex>=FileModel->GetPageCount() ||
		!IsInViewedPath()
	) {
		PressedRectType=RT_NONE;
		emPanel::Input(event,state,mx,my);
		return;
	}

	if (
		event.IsKey(EM_KEY_LEFT_BUTTON) &&
		!state.GetCtrl() && !state.GetAlt() && !state.GetMeta() &&
		(CurrentRectType==RT_URI || CurrentRectType==RT_REF)
	) {
		PressedRectType=CurrentRectType;
		PressedRectIndex=CurrentRectIndex;
		InvalidateCursor();
		Focus();
		event.Eat();
	}

	if (!state.Get(EM_KEY_LEFT_BUTTON) && PressedRectType!=RT_NONE) {
		if (
			PressedRectType==CurrentRectType &&
			PressedRectIndex==CurrentRectIndex
		) {
			TriggerCurrectRect();
		}
		PressedRectType=RT_NONE;
		InvalidateCursor();
	}

	Selection.PageInput(
		PageIndex,event,state,
		mx*FileModel->GetPageWidth(PageIndex),
		my/GetHeight()*FileModel->GetPageHeight(PageIndex)
	);

	forceTextCursor=
		Selection.IsSelectingByMouse() || (
			PressedRectType==RT_NONE &&
			(CurrentRectType==RT_URI || CurrentRectType==RT_REF) &&
			(state.GetAlt() || state.GetMeta())
		)
	;
	if (ForceTextCursor!=forceTextCursor) {
		ForceTextCursor=forceTextCursor;
		InvalidateCursor();
	}

	emPanel::Input(event,state,mx,my);
}


emCursor emPdfPagePanel::GetCursor() const
{
	if (ForceTextCursor) return emCursor::TEXT;
	switch (CurrentRectType) {
		case RT_TEXT:
			return emCursor::TEXT;
		case RT_URI:
		case RT_REF:
			if (
				PressedRectType!=RT_NONE && (
					PressedRectType!=CurrentRectType ||
					PressedRectIndex!=CurrentRectIndex
				)
			) {
				return emCursor::NORMAL;
			}
			return emCursor::HAND;
		default:
			return emCursor::NORMAL;
	}
}


bool emPdfPagePanel::IsOpaque() const
{
	return true;
}


void emPdfPagePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	const emString * pErrText;
	emString errText;
	emImage ico;
	double h,ix,iy,iw,ih,t;
	int i;

	pErrText=FileModel->GetPageAreasMap().GetError(PageIndex);
	if (pErrText) errText=*pErrText;
	for (i=0; i<3; i++) {
		PaintLayer(painter,Layers[i],&canvasColor);
		if (!Layers[i].JobErrorText.IsEmpty()) errText=Layers[i].JobErrorText;
	}

	if (!errText.IsEmpty()) {
		painter.PaintTextBoxed(
			0.0,
			0.0,
			1.0,
			GetHeight(),
			"ERROR:\n" + errText,
			GetHeight()/10,
			emColor(255,0,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
	}
	else if (IconState!=IS_NONE) {
		if (IconState==IS_WAITING) ico=WaitIcon;
		else ico=RenderIcon;
		h=GetHeight();
		iw=ViewToPanelDeltaX(ico.GetWidth());
		if (iw>1.0) iw=1.0;
		ih=ico.GetHeight()*iw/ico.GetWidth();
		if (ih>h) { iw=iw/ih*h; ih=h; }
		t=sqrt(h*iw/ih)/5;
		if (iw>t) { ih=ih/iw*t; iw=t; }
		ix=ViewToPanelX(GetClipX1());
		iy=ViewToPanelY(GetClipY1());
		if (ix>1.0-iw) ix=1.0-iw;
		if (iy>h-ih) iy=h-ih;
		painter.PaintImage(ix,iy,iw,ih,ico,255,canvasColor);
	}
}


emPdfPagePanel::Layer::Layer()
	: SrcX(0.0),
	SrcY(0.0),
	SrcW(0.0),
	SrcH(0.0),
	Job(NULL),
	JobSrcX(0.0),
	JobSrcY(0.0),
	JobSrcW(0.0),
	JobSrcH(0.0),
	JobDelayStartTime(0),
	CoordinatesUpToDate(false),
	ContentUpToDate(false),
	JobDelayStartTimeSet(false),
	Type(LT_PREVIEW)
{
}


emPdfPagePanel::Layer::~Layer()
{
}


void emPdfPagePanel::ResetLayer(Layer & layer, bool clearImage)
{
	if (layer.Job) {
		Server->CloseJob(layer.Job);
		layer.Job=NULL;
		layer.CoordinatesUpToDate=false;
		layer.ContentUpToDate=false;
	}
	if (!layer.JobImg.IsEmpty()) {
		layer.JobImg.Clear();
	}
	if (clearImage) {
		if (!layer.Img.IsEmpty()) {
			layer.Img.Clear();
			InvalidatePainting();
		}
		if (!layer.JobErrorText.IsEmpty()) {
			layer.JobErrorText.Clear();
			InvalidatePainting();
		}
		layer.CoordinatesUpToDate=false;
		layer.ContentUpToDate=false;
	}
	layer.JobDelayStartTimeSet=false;
}


bool emPdfPagePanel::UpdateLayer(Layer & layer)
{
	double fw,fh,ox,oy,ow,oh,ix,iy,iw,ih,sx,sy,sw,sh,qx1,qx2,qy1,qy2,q,tx,ty;
	emUInt64 tm,dt;

	if (PageIndex<0 || PageIndex>=FileModel->GetPageCount()) {
		ResetLayer(layer, true);
		return false;
	}

	if (!IsViewed()) {
		ResetLayer(layer, layer.Type!=LT_PREVIEW);
		return false;
	}

	if (layer.Job) {
		switch (Server->GetJobState(layer.Job)) {
		case emPdfServerModel::JS_WAITING:
		case emPdfServerModel::JS_RUNNING:
			return true;
		case emPdfServerModel::JS_ERROR:
			layer.JobErrorText=Server->GetJobErrorText(layer.Job);
			if (layer.JobErrorText.IsEmpty()) {
				layer.JobErrorText="unknown error";
			}
			Server->CloseJob(layer.Job);
			layer.Job=NULL;
			layer.JobImg.Clear();
			layer.Img.Clear();
			InvalidatePainting();
			return false;
		case emPdfServerModel::JS_SUCCESS:
			Server->CloseJob(layer.Job);
			layer.Job=NULL;
			layer.Img=layer.JobImg;
			layer.SrcX=layer.JobSrcX;
			layer.SrcY=layer.JobSrcY;
			layer.SrcW=layer.JobSrcW;
			layer.SrcH=layer.JobSrcH;
			layer.JobImg.Clear();
			InvalidatePainting();
			break;
		}
	}

	if (layer.CoordinatesUpToDate && layer.ContentUpToDate) return false;
	if (!layer.JobErrorText.IsEmpty()) return false;

	if (layer.Type==LT_SELECTION && !PageSelection.NonEmpty) {
		ResetLayer(layer, true);
		layer.CoordinatesUpToDate=true;
		layer.ContentUpToDate=true;
		return false;
	}

	if (layer.Type==LT_CONTENT && Layers[LT_PREVIEW].Img.IsEmpty()) {
		return false;
	}

	fw=FileModel->GetPageWidth(PageIndex);
	fh=FileModel->GetPageHeight(PageIndex);

	ox=PanelToViewX(0.0);
	oy=PanelToViewY(0.0);
	ow=PanelToViewDeltaX(1.0);
	oh=PanelToViewDeltaY(GetHeight());

	if (layer.Type==LT_PREVIEW) {
		q=sqrt(3000.0/(fw*fh));
		iw=fw*q;
		ih=fh*q;
		sx=0.0;
		sy=0.0;
		sw=fw;
		sh=fh;
	}
	else {
		ix=floor(emMax(GetClipX1(),ox));
		iy=floor(emMax(GetClipY1(),oy));
		iw=ceil(emMin(GetClipX2(),ox+ow))-ix;
		ih=ceil(emMin(GetClipY2(),oy+oh))-iy;
		sx=(ix-ox)*fw/ow;
		sy=(iy-oy)*fh/oh;
		sw=iw*fw/ow;
		sh=ih*fh/oh;
	}

	if (
		iw<1.0 || ih<1.0 ||
		(layer.Type==LT_CONTENT && iw/sw<=Layers[LT_PREVIEW].Img.GetWidth()/fw)
	) {
		ResetLayer(layer, true);
		layer.CoordinatesUpToDate=true;
		layer.ContentUpToDate=true;
		return false;
	}

	if (
		layer.Type!=LT_PREVIEW &&
		layer.ContentUpToDate &&
		!layer.Img.IsEmpty()
	) {
		if (layer.Img.GetWidth()==iw && layer.Img.GetHeight()==ih) {
			tx=sw/layer.Img.GetWidth()*0.05;
			ty=sh/layer.Img.GetHeight()*0.05;
			if (
				fabs(layer.SrcX-sx)<=tx &&
				fabs(layer.SrcX+layer.SrcW-sx-sw)<=tx &&
				fabs(layer.SrcY-sy)<=ty &&
				fabs(layer.SrcY+layer.SrcH-sy-sh)<=ty
			) {
					layer.CoordinatesUpToDate=true;
					return false;
			}
		}
		qx1=emMax(layer.SrcX,sx);
		qx2=emMin(layer.SrcX+layer.SrcW,sx+sw);
		qy1=emMax(layer.SrcY,sy);
		qy2=emMin(layer.SrcY+layer.SrcH,sy+sh);
		if (qx2<qx1) qx2=qx1;
		if (qy2<qy1) qy2=qy1;
		q=(qx2-qx1)*(qy2-qy1)/(sw*sh);
		q=(q-0.9)*10.0;
		if (q>0.0 && layer.Img.GetWidth()/layer.SrcW>0.9*iw/sw) {
			dt=(emUInt64)(q*q*500.0+0.5);
			tm=emGetClockMS();
			if (!layer.JobDelayStartTimeSet) {
				layer.JobDelayStartTime=tm;
				layer.JobDelayStartTimeSet=true;
			}
			if (tm-layer.JobDelayStartTime<dt) {
				return true;
			}
		}
	}

	if (layer.Type==LT_SELECTION) {
		layer.Job=Server->StartRenderSelectionJob(
			FileModel->GetPdfHandle(),
			PageIndex,sx,sy,sw,sh,
			(int)(iw+0.5),(int)(ih+0.5),
			PageSelection.Style,
			PageSelection.X1,PageSelection.Y1,
			PageSelection.X2,PageSelection.Y2,
			&layer.JobImg,GetUpdatePriority(),this
		);
	}
	else {
		layer.Job=Server->StartRenderJob(
			FileModel->GetPdfHandle(),
			PageIndex,sx,sy,sw,sh,
			(int)(iw+0.5),(int)(ih+0.5),
			&layer.JobImg,GetUpdatePriority(),this
		);
	}
	layer.JobSrcX=sx;
	layer.JobSrcY=sy;
	layer.JobSrcW=sw;
	layer.JobSrcH=sh;
	layer.CoordinatesUpToDate=true;
	layer.ContentUpToDate=true;
	layer.JobDelayStartTimeSet=false;
	layer.JobStartTime=emGetClockMS();
	return true;
}


void emPdfPagePanel::PaintLayer(
	const emPainter & painter, const Layer & layer, emColor * canvasColor
) const
{
	static const emColor bgCol=emColor(221,255,255);
	double fw,fh,ox,oy,ow,oh,sx,sy,sw,sh;
	double sx1,sy1,sx2,sy2;
	int pw,ph,px1,py1,px2,py2;

	ox=0.0;
	oy=0.0;
	ow=1.0;
	oh=GetHeight();

	if (layer.Img.IsEmpty()) {
		if (layer.Type == LT_PREVIEW) {
			painter.PaintRect(
				ox,oy,ow,oh,
				bgCol,
				*canvasColor
			);
			*canvasColor=bgCol;
		}
		return;
	}

	if (layer.Type == LT_PREVIEW) {
		if (Layers[LT_CONTENT].Img.IsEmpty()) {
			painter.PaintImage(
				ox,oy,ow,oh,
				layer.Img,255,
				*canvasColor
			);
			*canvasColor=0;
		}
		return;
	}

	fw=FileModel->GetPageWidth(PageIndex);
	fh=FileModel->GetPageHeight(PageIndex);
	sx=ox+layer.SrcX*ow/fw;
	sy=oy+layer.SrcY*oh/fh;
	sw=layer.SrcW*ow/fw;
	sh=layer.SrcH*oh/fh;

	if (layer.Type == LT_SELECTION) {
		painter.PaintImageColored(
			sx,sy,sw,sh,
			layer.Img,
			emColor(16,56,192),
			emColor(255,255,255),
			*canvasColor
		);
		*canvasColor=0;
		return;
	}

	painter.PaintImage(
		sx,sy,sw,sh,
		layer.Img,
		255,
		*canvasColor
	);

	if (!Layers[LT_PREVIEW].Img.IsEmpty()) {
		sx1=emMax(ox,sx);
		sy1=emMax(oy,sy);
		sx2=emMin(ox+ow,sx+sw);
		sy2=emMin(oy+oh,sy+sh);
		pw=Layers[LT_PREVIEW].Img.GetWidth();
		ph=Layers[LT_PREVIEW].Img.GetHeight();
		px1=(int)((sx1-ox)*pw/ow+0.5);
		py1=(int)((sy1-oy)*ph/oh+0.5);
		px2=(int)((sx2-oy)*pw/ow+0.5);
		py2=(int)((sy2-oy)*ph/oh+0.5);
		if (sy1>oy) painter.PaintImage(
			ox,oy,ow,sy1-oy,
			Layers[LT_PREVIEW].Img,
			0,0,pw,py1,
			255,*canvasColor
		);
		if (sx1>ox) painter.PaintImage(
			ox,sy1,sx1-ox,sy2-sy1,
			Layers[LT_PREVIEW].Img,
			0,py1,px1,py2-py1,
			255,*canvasColor
		);
		if (sx2<ox+ow) painter.PaintImage(
			sx2,sy1,ox+ow-sx2,sy2-sy1,
			Layers[LT_PREVIEW].Img,
			px2,py1,pw-px2,py2-py1,
			255,*canvasColor
		);
		if (sy2<oy+oh) painter.PaintImage(
			ox,sy2,ow,oy+oh-sy2,
			Layers[LT_PREVIEW].Img,
			0,py2,pw,ph-py2,
			255,*canvasColor
		);
	}

	*canvasColor=0;
}


void emPdfPagePanel::UpdateIconState()
{
	IconStateType iconState;
	int i;

	iconState=IS_NONE;
	for (i=0; i<3; i++) {
		if (!Layers[i].Job) continue;
		if (emGetClockMS()-Layers[i].JobStartTime<2000) continue;
		if (
			iconState!=IS_RENDERING &&
			Server->GetJobState(Layers[i].Job)==emPdfServerModel::JS_WAITING
		) {
			iconState=IS_WAITING;
		}
		else {
			iconState=IS_RENDERING;
		}
	}
	if (IconState!=iconState) {
		IconState=iconState;
		InvalidatePainting();
	}
}


void emPdfPagePanel::UpdateCurrentRect()
{
	const emPdfServerModel::PageAreas * areas;
	RectType newType;
	int x,y,newIndex,i;

	newType=RT_NONE;
	newIndex=0;

	if (
		PageIndex>=0 && PageIndex<FileModel->GetPageCount() &&
		IsInViewedPath() &&
		CurrentMX>=0.0 && CurrentMX<1.0 &&
		CurrentMY>=0.0 && CurrentMY<GetHeight()
	) {
		areas=FileModel->GetPageAreasMap().GetPageAreas(PageIndex);
		if (areas) {
			x=(int)(CurrentMX*FileModel->GetPageWidth(PageIndex)+0.5);
			y=(int)(CurrentMY/GetHeight()*FileModel->GetPageHeight(PageIndex)+0.5);
			for (i=areas->TextRects.GetCount()-1; i>=0; i--) {
				if (areas->TextRects[i].Contains(x,y)) {
					newType=RT_TEXT;
					newIndex=i;
					break;
				}
			}
			for (i=areas->UriRects.GetCount()-1; i>=0; i--) {
				if (areas->UriRects[i].Contains(x,y)) {
					newType=RT_URI;
					newIndex=i;
					break;
				}
			}
			for (i=areas->RefRects.GetCount()-1; i>=0; i--) {
				if (areas->RefRects[i].Contains(x,y)) {
					newType=RT_REF;
					newIndex=i;
					break;
				}
			}
		}
		else {
			FileModel->GetPageAreasMap().RequestPageAreas(
				PageIndex,GetUpdatePriority()
			);
		}
	}

	if (CurrentRectType!=newType || CurrentRectIndex!=newIndex) {
		CurrentRectType=newType;
		CurrentRectIndex=newIndex;
		InvalidateCursor();
	}
}


void emPdfPagePanel::TriggerCurrectRect()
{
	const emPdfServerModel::PageAreas * areas;
	const emPdfServerModel::RefRect * rr;
	emPanel * panel;
	emPdfPagePanel * pagePanel;
	double pw,ph,pt,vt,relX,relY,relA;

	areas=FileModel->GetPageAreasMap().GetPageAreas(PageIndex);
	if (!areas) return;

	if (
		CurrentRectType==RT_URI &&
		CurrentRectIndex>=0 &&
		CurrentRectIndex<areas->UriRects.GetCount()
	) {
		emDialog::ShowMessage(GetView(),"Error","Triggering URI not implemented");
	}

	if (
		CurrentRectType==RT_REF &&
		CurrentRectIndex>=0 &&
		CurrentRectIndex<areas->RefRects.GetCount() &&
		GetParent()
	) {
		rr=&areas->RefRects[CurrentRectIndex];
		pagePanel=NULL;
		for (panel=GetParent()->GetFirstChild(); panel; panel=panel->GetNext()) {
			pagePanel=dynamic_cast<emPdfPagePanel*>(panel);
			if (pagePanel && pagePanel->GetPageIndex()==rr->TargetPage) break;
			pagePanel=NULL;
		}
		if (!pagePanel) return;

		pw=FileModel->GetPageWidth(rr->TargetPage);
		ph=FileModel->GetPageHeight(rr->TargetPage);
		pt=ph/pw;
		vt=GetView().GetCurrentTallness();
		if (vt>=pt) {
			GetView().VisitFullsized(pagePanel,true);
			return;
		}
		relX=0.0;
		relY=-(1.0-vt/pt)*0.5;
		relY+=emMin(1.0-vt/pt,emMax(0.0,rr->TargetY/ph));
		relA=vt/pt;
		GetView().Visit(pagePanel,relX,relY,relA,true);
	}
}
