//------------------------------------------------------------------------------
// emSvgFilePanel.cpp
//
// Copyright (C) 2010-2011,2014-2016,2023-2024 Oliver Hamann.
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

#include <emSvg/emSvgFilePanel.h>
#include <emCore/emRes.h>
#include <emCore/emToolkit.h>


emSvgFilePanel::emSvgFilePanel(
	ParentArg parent, const emString & name, emSvgFileModel * fileModel,
	bool updateFileModel
)
	: emFilePanel(parent,name),
	JobDelayTimer(GetScheduler()),
	IconTimer(GetScheduler())
{
	ServerModel=emSvgServerModel::Acquire(GetRootContext());
	JobUpToDate=false;
	JobDelayStartTime=emGetClockMS();
	RenderIcon=emGetInsResImage(GetRootContext(),"emPs","rendering.tga");
	ShowIcon=false;
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(JobDelayTimer.GetSignal());
	AddWakeUpSignal(IconTimer.GetSignal());
	SetFileModel(fileModel,updateFileModel);
}


emSvgFilePanel::~emSvgFilePanel()
{
	ClearSvgDisplay();
}


void emSvgFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (fileModel && (dynamic_cast<emSvgFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}
	emFilePanel::SetFileModel(fileModel,updateFileModel);
}


emString emSvgFilePanel::GetIconFileName() const
{
	return "drawing.tga";
}


void emSvgFilePanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	if (IsVFSGood() && RenderError.IsEmpty()) {
		GetOutputRect(pX,pY,pW,pH);
	}
	else {
		emFilePanel::GetEssenceRect(pX,pY,pW,pH);
	}
}


bool emSvgFilePanel::Cycle()
{
	if (IsSignaled(GetVirFileStateSignal())) {
		InvalidateControlPanel(); //??? very cheap solution, but okay for now.
		ClearSvgDisplay();
	}

	UpdateSvgDisplay(false);

	return emFilePanel::Cycle();
}


void emSvgFilePanel::Notice(NoticeFlags flags)
{
	if (flags&NF_VIEWING_CHANGED) {
		UpdateSvgDisplay(true);
	}
	if (flags&NF_UPDATE_PRIORITY_CHANGED) {
		if (RenderJob) {
			RenderJob->SetPriority(GetUpdatePriority());
		}
	}
	emFilePanel::Notice(flags);
}


bool emSvgFilePanel::IsOpaque() const
{
	if (!IsVFSGood()) {
		return emFilePanel::IsOpaque();
	}
	else if (!RenderError.IsEmpty()) {
		return true;
	}
	else {
		return false;
	}
}


void emSvgFilePanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	static const emColor RENDER_COLOR=emColor(0xEEEEFFFF);
	emSvgFileModel * fm;
	double fw,fh,ox,oy,ow,oh,sx,sy,sw,sh,ix,iy,iw,ih,t;
	emColor c;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	if (!RenderError.IsEmpty()) {
		c.Set(128,0,0);
		painter.Clear(c,canvasColor);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.15,
			0.9,
			GetHeight()*0.1,
			"Rendering Failed",
			GetHeight()*0.1,
			emColor(204,136,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.3,
			0.9,
			GetHeight()*0.4,
			RenderError,
			GetHeight()*0.4,
			emColor(255,255,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		return;
	}

	GetOutputRect(&ox,&oy,&ow,&oh);

	if (Img.IsEmpty()) {
		painter.PaintRect(
			ox,oy,ow,oh,
			RENDER_COLOR,
			canvasColor
		);
		canvasColor=RENDER_COLOR;
	}
	else {
		fm=(emSvgFileModel*)GetFileModel();
		fw=fm->GetWidth();
		fh=fm->GetHeight();
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
		if (sy>oy) painter.PaintRect(
			ox,oy,ow,sy-oy,
			RENDER_COLOR,
			canvasColor
		);
		if (sx>ox) painter.PaintRect(
			ox,emMax(sy,oy),sx-ox,emMin(sy+sh,oy+oh)-emMax(sy,oy),
			RENDER_COLOR,
			canvasColor
		);
		if (sx+sw<ox+ow) painter.PaintRect(
			sx+sw,emMax(sy,oy),ox+ow-sx-sw,emMin(sy+sh,oy+oh)-emMax(sy,oy),
			RENDER_COLOR,
			canvasColor
		);
		if (sy+sh<oy+oh) painter.PaintRect(
			ox,sy+sh,ow,oy+oh-sy-sh,
			RENDER_COLOR,
			canvasColor
		);
		canvasColor=0;
	}

	if (ShowIcon) {
		iw=ViewToPanelDeltaX(RenderIcon.GetWidth());
		if (iw>ow) iw=ow;
		ih=RenderIcon.GetHeight()*iw/RenderIcon.GetWidth();
		if (ih>oh) { iw=iw/ih*oh; ih=oh; }
		t=sqrt(oh*iw/ih)/5;
		if (iw>t) { ih=ih/iw*t; iw=t; }
		ix=ViewToPanelX(GetClipX1());
		iy=ViewToPanelY(GetClipY1());
		if (ix<ox) ix=ox;
		if (iy<oy) iy=oy;
		if (ix>ox+ow-iw) ix=ox+ow-iw;
		if (iy>oy+oh-ih) iy=oy+oh-ih;
		painter.PaintImage(ix,iy,iw,ih,RenderIcon,255,canvasColor);
	}
}


emPanel * emSvgFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	emSvgFileModel * fm;
	emLinearLayout * mainLayout;
	emLinearGroup * grp;
	emTextField * tf;

	if (IsVFSGood()) {
		fm=(emSvgFileModel*)GetFileModel();
		mainLayout=new emLinearLayout(parent,name);
		mainLayout->SetMinChildTallness(0.03);
		mainLayout->SetMaxChildTallness(0.6);
		mainLayout->SetAlignment(EM_ALIGN_TOP_LEFT);
		grp=new emLinearGroup(
			mainLayout,
			"",
			"SVG File Info"
		);
		grp->SetOrientationThresholdTallness(0.07);
		tf=new emTextField(
			grp,
			"title",
			"Title",
			emString(),
			emImage(),
			fm->GetTitle()
		);
		tf->SetMultiLineMode();
		tf=new emTextField(
			grp,
			"desc",
			"Description",
			emString(),
			emImage(),
			fm->GetDescription()
		);
		tf->SetMultiLineMode();
		tf=new emTextField(
			grp,
			"size",
			"Default Size (Pixels)",
			emString(),
			emImage(),
			emString::Format(
				"%g x %g",
				fm->GetWidth(),
				fm->GetHeight()
			)
		);
		return mainLayout;
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emSvgFilePanel::GetOutputRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	const emSvgFileModel * fm;
	double x,y,w,h,d,fw,fh;

	if (IsVFSGood()) {
		fm=(const emSvgFileModel*)GetFileModel();
		fw=fm->GetWidth();
		fh=fm->GetHeight();
	}
	else {
		fw=4.0;
		fh=3.0;
	}
	x=0;
	y=0;
	w=1;
	h=GetHeight();
	if (fw*h>=fh*w) {
		d=w*fh/fw;
		y+=(h-d)/2;
		h=d;
	}
	else {
		d=h*fw/fh;
		x+=(w-d)/2;
		w=d;
	}
	*pX=x;
	*pY=y;
	*pW=w;
	*pH=h;
}


void emSvgFilePanel::ClearSvgDisplay()
{
	if (RenderJob) {
		ServerModel->AbortJob(*RenderJob);
		RenderJob=NULL;
	}
	if (!Img.IsEmpty()) {
		Img.Clear();
		InvalidatePainting();
	}
	if (!RenderError.IsEmpty()) {
		RenderError.Clear();
		InvalidatePainting();
	}
	JobUpToDate=false;
	IconTimer.Stop(true);
	ShowIcon=false;
}


void emSvgFilePanel::UpdateSvgDisplay(bool viewingChanged)
{
	emSvgFileModel * fm;
	double fw,fh,ox,oy,ow,oh,ix,iy,iw,ih,sx,sy,sw,sh,qx1,qx2,qy1,qy2,q;
	emUInt64 tm,dt;

	if (!IsVFSGood()) return;
	if (!RenderError.IsEmpty()) return;
	if (!IsViewed()) return;

	if (JobUpToDate) JobDelayStartTime=emGetClockMS();
	if (viewingChanged) JobUpToDate=false;

	if (RenderJob) {
		switch (RenderJob->GetState()) {
		case emJob::ST_WAITING:
		case emJob::ST_RUNNING:
			if (!ShowIcon && !IconTimer.IsRunning()) {
				ShowIcon=true;
				InvalidatePainting();
			}
			return;
		case emJob::ST_SUCCESS:
			SrcX=RenderJob->GetSrcX();
			SrcY=RenderJob->GetSrcY();
			SrcW=RenderJob->GetSrcWidth();
			SrcH=RenderJob->GetSrcHeight();
			Img=RenderJob->GetImage();
			RenderJob=NULL;
			if (JobUpToDate) {
				IconTimer.Stop(true);
				ShowIcon=false;
			}
			JobDelayStartTime=emGetClockMS();
			InvalidatePainting();
			break;
		default:
			if (RenderJob->GetState()==emJob::ST_ERROR) {
				RenderError=RenderJob->GetErrorText();
				if (RenderError.IsEmpty()) RenderError="unknown error";
			}
			else {
				RenderError="Aborted";
			}
			RenderJob=NULL;
			Img.Clear();
			JobUpToDate=false;
			IconTimer.Stop(true);
			ShowIcon=false;
			InvalidatePainting();
			return;
		}
	}

	if (JobUpToDate) return;

	fm=(emSvgFileModel*)GetFileModel();
	fw=fm->GetWidth();
	fh=fm->GetHeight();

	GetOutputRect(&ox,&oy,&ow,&oh);
	ox=PanelToViewX(ox);
	oy=PanelToViewY(oy);
	ow=PanelToViewDeltaX(ow);
	oh=PanelToViewDeltaY(oh);

	ix=floor(emMax(GetClipX1(),ox));
	iy=floor(emMax(GetClipY1(),oy));
	iw=ceil(emMin(GetClipX2(),ox+ow))-ix;
	ih=ceil(emMin(GetClipY2(),oy+oh))-iy;

	sx=(ix-ox)*fw/ow;
	sy=(iy-oy)*fh/oh;
	sw=iw*fw/ow;
	sh=ih*fh/oh;

	if (iw<1.0 || ih<1.0) {
		Img.Clear();
		SrcX=sx;
		SrcY=sy;
		SrcW=sw;
		SrcH=sh;
		InvalidatePainting();
		JobUpToDate=true;
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

	RenderJob=new emSvgServerModel::RenderJob(
		fm->GetSvgInstance(),
		sx,sy,sw,sh,
		emColor(0xffffffff),
		(int)(iw+0.5),(int)(ih+0.5),
		GetUpdatePriority()
	);
	ServerModel->EnqueueJob(*RenderJob);
	AddWakeUpSignal(RenderJob->GetStateSignal());
	if (!ShowIcon) IconTimer.Start(500);
	JobUpToDate=true;
}
