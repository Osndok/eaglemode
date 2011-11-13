//------------------------------------------------------------------------------
// emPdfPagePanel.cpp
//
// Copyright (C) 2011 Oliver Hamann.
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

#include <emCore/emRes.h>
#include <emPdf/emPdfPagePanel.h>


emPdfPagePanel::emPdfPagePanel(
	ParentArg parent, const emString & name,
	emPdfFileModel * fileModel, int pageIndex
) :
	emPanel(parent,name),
	JobDelayTimer(GetScheduler()),
	IconTimer(GetScheduler())
{
	Server=fileModel->GetServerModel();
	FileModel=fileModel;
	PageIndex=pageIndex;
	Job=NULL;
	JobUpToDate=false;
	JobDelayStartTime=emGetClockMS();
	WaitIcon=emGetInsResImage(GetRootContext(),"emPs","waiting.tga");
	RenderIcon=emGetInsResImage(GetRootContext(),"emPs","rendering.tga");
	ShowIcon=false;
	AddWakeUpSignal(JobDelayTimer.GetSignal());
	AddWakeUpSignal(IconTimer.GetSignal());
	UpdatePageDisplay(false);
}


emPdfPagePanel::~emPdfPagePanel()
{
	if (Job) Server->CloseJob(Job);
}


void emPdfPagePanel::Notice(NoticeFlags flags)
{
	emPanel::Notice(flags);

	if ((flags&(NF_VIEWING_CHANGED|NF_MEMORY_LIMIT_CHANGED))!=0) {
		UpdatePageDisplay(true);
	}
	if ((flags&NF_UPDATE_PRIORITY_CHANGED)!=0) {
		if (Job) Server->SetJobPriority(Job,GetUpdatePriority());
	}
}


bool emPdfPagePanel::Cycle()
{
	UpdatePageDisplay(false);
	return false;
}


bool emPdfPagePanel::IsOpaque()
{
	return true;
}


void emPdfPagePanel::Paint(
	const emPainter & painter, emColor canvasColor
)
{
	static const emColor bgCol=emColor(221,255,255);
	double h,fw,fh,ox,oy,ow,oh,sx,sy,sw,sh,ix,iy,iw,ih,t;
	double sx1,sy1,sx2,sy2,pw,ph,px1,py1,px2,py2;
	emImage ico;

	ox=0.0;
	oy=0.0;
	ow=1.0;
	oh=GetHeight();

	if (!Img.IsEmpty()) {
		fw=FileModel->GetPageWidth(PageIndex);
		fh=FileModel->GetPageHeight(PageIndex);
		sx=ox+SrcX*ow/fw;
		sy=oy+SrcY*oh/fh;
		sw=SrcW*ow/fw;
		sh=SrcH*oh/fh;
		emPainter(
			painter,
			painter.GetOriginX()+ox*painter.GetScaleX(),
			painter.GetOriginY()+oy*painter.GetScaleY(),
			painter.GetOriginX()+(ox+ow)*painter.GetScaleX(),
			painter.GetOriginY()+(oy+oh)*painter.GetScaleY()
		).PaintImage(
			sx,sy,sw,sh,
			Img,
			255,
			canvasColor
		);
		sx1=emMax(ox,sx);
		sy1=emMax(oy,sy);
		sx2=emMin(ox+ow,sx+sw);
		sy2=emMin(oy+oh,sy+sh);
		pw=PreImg.GetWidth();
		ph=PreImg.GetHeight();
		px1=(sx1-ox)*pw/ow;
		py1=(sy1-oy)*ph/oh;
		px2=(sx2-oy)*pw/ow;
		py2=(sy2-oy)*ph/oh;
		if (sy1>oy) painter.PaintImage(
			ox,oy,ow,sy1-oy,
			PreImg,
			0.0,0.0,pw,py1,
			255,canvasColor
		);
		if (sx1>ox) painter.PaintImage(
			ox,sy1,sx1-ox,sy2-sy1,
			PreImg,
			0.0,py1,px1,py2-py1,
			255,canvasColor
		);
		if (sx2<ox+ow) painter.PaintImage(
			sx2,sy1,ox+ow-sx2,sy2-sy1,
			PreImg,
			px2,py1,pw-px2,py2-py1,
			255,canvasColor
		);
		if (sy2<oy+oh) painter.PaintImage(
			ox,sy2,ow,oy+oh-sy2,
			PreImg,
			0.0,py2,pw,ph-py2,
			255,canvasColor
		);
		canvasColor=0;
	}
	else if (!PreImg.IsEmpty()) {
		painter.PaintImage(
			ox,oy,ow,oh,
			PreImg,255,
			canvasColor
		);
		canvasColor=0;
	}
	else {
		painter.PaintRect(
			ox,oy,ow,oh,
			bgCol,
			canvasColor
		);
		canvasColor=bgCol;
	}

	if (!JobErrorText.IsEmpty()) {
		painter.PaintTextBoxed(
			0.0,
			0.0,
			1.0,
			GetHeight(),
			"ERROR:\n" + JobErrorText,
			GetHeight()/10,
			emColor(255,0,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
	}
	else if (ShowIcon) {
		if (Job && Server->GetJobState(Job)==emPdfServerModel::JS_WAITING) ico=WaitIcon;
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


void emPdfPagePanel::UpdatePageDisplay(bool viewingChanged)
{
	double fw,fh,ox,oy,ow,oh,ix,iy,iw,ih,sx,sy,sw,sh,qx1,qx2,qy1,qy2,q;
	emUInt64 tm,dt;

	if (!IsViewed() || PageIndex<0 || PageIndex>=FileModel->GetPageCount()) {
		if (Job) {
			Server->CloseJob(Job);
			Job=NULL;;
		}
		if (!JobImg.IsEmpty()) {
			JobImg.Empty();
		}
		if (!Img.IsEmpty()) {
			Img.Empty();
			InvalidatePainting();
		}
		if (!PreImg.IsEmpty()) {
			if (PageIndex<0 || PageIndex>=FileModel->GetPageCount()) {
				PreImg.Empty();
				InvalidatePainting();
			}
		}
		if (JobErrorText.IsEmpty()) {
			JobErrorText.Empty();
			InvalidatePainting();
		}
		JobUpToDate=false;
		IconTimer.Stop(true);
		ShowIcon=false;
		return;
	}

	if (JobUpToDate) JobDelayStartTime=emGetClockMS();
	if (viewingChanged) JobUpToDate=false;

	if (Job) {
		switch (Server->GetJobState(Job)) {
		case emPdfServerModel::JS_WAITING:
		case emPdfServerModel::JS_RUNNING:
			if (!ShowIcon && !IconTimer.IsRunning()) {
				ShowIcon=true;
				InvalidatePainting();
			}
			return;
		case emPdfServerModel::JS_ERROR:
			JobErrorText=Server->GetJobErrorText(Job);
			if (JobErrorText.IsEmpty()) JobErrorText="unknown error";
			Server->CloseJob(Job);
			Job=NULL;;
			JobImg.Empty();
			Img.Empty();
			PreImg.Empty();
			JobUpToDate=false;
			IconTimer.Stop(true);
			ShowIcon=false;
			InvalidatePainting();
			return;
		case emPdfServerModel::JS_SUCCESS:
			Server->CloseJob(Job);
			Job=NULL;
			if (PreImg.IsEmpty()) {
				PreImg=JobImg;
				JobUpToDate=false;
			}
			Img=JobImg;
			SrcX=JobSrcX;
			SrcY=JobSrcY;
			SrcW=JobSrcW;
			SrcH=JobSrcH;
			JobImg.Empty();
			if (JobUpToDate) {
				IconTimer.Stop(true);
				ShowIcon=false;
			}
			JobDelayStartTime=emGetClockMS();
			InvalidatePainting();
			break;
		}
	}

	if (JobUpToDate) return;

	fw=FileModel->GetPageWidth(PageIndex);
	fh=FileModel->GetPageHeight(PageIndex);

	ox=PanelToViewX(0.0);
	oy=PanelToViewY(0.0);
	ow=PanelToViewDeltaX(1.0);
	oh=PanelToViewDeltaY(GetHeight());

	if (PreImg.IsEmpty()) {
		q=sqrt(1000.0/(fw*fh));
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

	if (iw<1.0 || ih<1.0 || iw/sw<=PreImg.GetWidth()/fw) {
		Img.Empty();
		SrcX=0.0;
		SrcY=0.0;
		SrcW=fw;
		SrcH=fh;
		InvalidatePainting();
		JobUpToDate=true;
		IconTimer.Stop(true);
		ShowIcon=false;
		return;
	}

	if (!Img.IsEmpty()) {
		qx1=emMax(SrcX,sx);
		qx2=emMin(SrcX+SrcW,sx+sw);
		qy1=emMax(SrcY,sy);
		qy2=emMin(SrcY+SrcH,sy+sh);
		if (qx2<qx1) qx2=qx1;
		if (qy2<qy1) qy2=qy1;
		q=(qx2-qx1)*(qy2-qy1)/(sw*sh);
		q=(q-0.9)*10.0;
		if (q>0.0 && Img.GetWidth()/SrcW>0.9*iw/sw) {
			dt=(emUInt64)(q*q*500.0+0.5);
			tm=emGetClockMS();
			if (JobDelayStartTime+dt>tm) {
				JobDelayTimer.Start(JobDelayStartTime+dt-tm);
				return;
			}
		}
	}

	JobSrcX=sx;
	JobSrcY=sy;
	JobSrcW=sw;
	JobSrcH=sh;

	Job=Server->StartRenderJob(
		FileModel->GetPdfHandle(),
		PageIndex,
		JobSrcX,
		JobSrcY,
		JobSrcW,
		JobSrcH,
		(int)(iw+0.5),
		(int)(ih+0.5),
		emColor(0xffffffff),
		&JobImg,
		GetUpdatePriority(),
		this
	);
	if (!ShowIcon) IconTimer.Start(2000);
	JobUpToDate=true;
}
