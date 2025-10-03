//------------------------------------------------------------------------------
// emPsPagePanel.cpp
//
// Copyright (C) 2006-2008,2014,2016,2024 Oliver Hamann.
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

#include <emPs/emPsPagePanel.h>
#include <emCore/emRes.h>


emPsPagePanel::emPsPagePanel(
	ParentArg parent, const emString & name,
	const emPsDocument & document, int pageIndex
) :
	emPanel(parent,name)
{
	Document=document;
	PageIndex=pageIndex;
	Renderer=emPsRenderer::Acquire(GetRootContext());
	JobState=emJob::ST_SUCCESS;
	WaitIcon=emGetInsResImage(GetRootContext(),"emPs","waiting.tga");
	RenderIcon=emGetInsResImage(GetRootContext(),"emPs","rendering.tga");
	UpdateJobAndImage();
}


emPsPagePanel::~emPsPagePanel()
{
	if (Job) Renderer->AbortJob(*Job);
}


void emPsPagePanel::SetPage(const emPsDocument & document, int pageIndex)
{
	if (Document==document && PageIndex==pageIndex) return;
	if (Job) {
		Renderer->AbortJob(*Job);
		Job=NULL;
	}
	Document=document;
	PageIndex=pageIndex;
	Image.Clear();
	JobState=emJob::ST_SUCCESS;
	JobErrorText.Clear();
	InvalidatePainting();
	UpdateJobAndImage();
}


void emPsPagePanel::Notice(NoticeFlags flags)
{
	emPanel::Notice(flags);

	if ((flags&(NF_VIEWING_CHANGED|NF_MEMORY_LIMIT_CHANGED))!=0) {
		UpdateJobAndImage();
	}
	if ((flags&NF_UPDATE_PRIORITY_CHANGED)!=0) {
		if (Job) Job->SetPriority(GetUpdatePriority());
	}
}


bool emPsPagePanel::Cycle()
{
	UpdateJobAndImage();
	return false;
}


bool emPsPagePanel::IsOpaque() const
{
	return true;
}


void emPsPagePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	static const emColor bgCol=emColor(230,255,255);
	double h,ix,iy,iw,ih,t;
	emImage ico;

	if (Image.IsEmpty()) {
		painter.Clear(bgCol,canvasColor);
		canvasColor=bgCol;
	}
	else {
		painter.PaintImage(
			0.0,0.0,1.0,GetHeight(),
			Image,255,
			canvasColor
		);
		canvasColor=0;
	}

	if (JobState==emJob::ST_ERROR) {
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
	else if (JobState!=emJob::ST_SUCCESS) {
		if (JobState==emJob::ST_WAITING) ico=WaitIcon;
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


void emPsPagePanel::UpdateJobAndImage()
{
	emJob::StateEnum s;
	emString e;
	double hrel,wmin,hmin,wmax,hmax,t;
	int currentWidth,currentHeight,iw,ih;

	static const double maxImageArea=5000*5000;
	static const double maxImageSize=10000;

	if (JobState==emJob::ST_ERROR) return;

	if (Job) {
		currentWidth=Job->GetWidth();
		currentHeight=Job->GetHeight();
	}
	else {
		currentWidth=Image.GetWidth();
		currentHeight=Image.GetHeight();
	}

	if (!IsViewed() || PageIndex<0 || PageIndex>=Document.GetPageCount()) {
		iw=0;
		ih=0;
	}
	else {
		hrel=GetViewedHeight()/GetViewedWidth();
		wmin=GetViewedWidth()*1.7;
		wmax=GetViewedWidth()*2.3;
		t=(double)GetMemoryLimit();
		t*=0.5; // For the memory effort of emPsFileModel.
		t=sqrt(t/3.0/hrel);
		if (wmax>t) wmax=t;
		t=sqrt(maxImageArea/hrel);
		if (wmax>t) wmax=t;
		if (wmax>maxImageSize) wmax=maxImageSize;
		if (wmax>maxImageSize/hrel) wmax=maxImageSize/hrel;
		if (wmin>wmax) wmin=wmax;
		hmin=wmin*hrel;
		hmax=wmax*hrel;
		if (wmax<5.0 || hmax<5.0) {
			iw=0;
			ih=0;
		}
		else if (currentWidth<=wmin-1.0 || currentHeight<=hmin-1.0) {
			iw=(int)(wmax+0.5);
			ih=(int)(hmax+0.5);
		}
		else if (currentWidth>=wmax+1.0 || currentHeight>=hmax+1.0) {
			iw=(int)(wmin+0.5);
			ih=(int)(hmin+0.5);
		}
		else {
			iw=currentWidth;
			ih=currentHeight;
		}
	}

	if (currentWidth!=iw || currentHeight!=ih) {
		if (Job) {
			Renderer->AbortJob(*Job);
			Job=NULL;
			JobState=emJob::ST_SUCCESS;
		}
		if (iw<=0 || ih<=0) {
			Image.Clear();
		}
		else {
			Job=new emPsRenderer::RenderJob(
				Document,
				PageIndex,
				iw,ih,
				GetUpdatePriority()
			);
			Renderer->EnqueueJob(*Job);
			JobState=Job->GetState();
			AddWakeUpSignal(Job->GetStateSignal());
		}
		InvalidatePainting();
	}

	if (Job) {
		s=Job->GetState();
		if (JobState!=s) {
			JobState=s;
			InvalidatePainting();
		}
		if (s==emJob::ST_ERROR) {
			Image.Clear();
			JobErrorText=Job->GetErrorText();
			Job=NULL;
		}
		else if (s==emJob::ST_SUCCESS) {
			Image=Job->GetImage();
			Job=NULL;
		}
		else if (s==emJob::ST_ABORTED) {
			Image.Clear();
			JobErrorText="Aborted";
			Job=NULL;
		}
	}
}
