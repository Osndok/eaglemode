//------------------------------------------------------------------------------
// emPsPagePanel.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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
#include <emPs/emPsPagePanel.h>


emPsPagePanel::emPsPagePanel(
	ParentArg parent, const emString & name,
	const emPsDocument & document, int pageIndex
) :
	emPanel(parent,name)
{
	Document=document;
	PageIndex=pageIndex;
	Renderer=emPsRenderer::Acquire(GetRootContext());
	Job=NULL;
	JobState=emPsRenderer::JS_SUCCESS;
	WaitIcon=emGetInsResImage(GetRootContext(),"emPs","waiting.tga");
	RenderIcon=emGetInsResImage(GetRootContext(),"emPs","rendering.tga");
	UpdateJobAndImage();
}


emPsPagePanel::~emPsPagePanel()
{
	if (Job) Renderer->CloseJob(Job);
}


void emPsPagePanel::SetPage(const emPsDocument & document, int pageIndex)
{
	if (Document==document && PageIndex==pageIndex) return;
	if (Job) {
		Renderer->CloseJob(Job);
		Job=NULL;
	}
	Document=document;
	PageIndex=pageIndex;
	Image.Empty();
	JobState=emPsRenderer::JS_SUCCESS;
	JobErrorText.Empty();
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
		if (Job) Renderer->SetJobPriority(Job,GetUpdatePriority());
	}
}


bool emPsPagePanel::Cycle()
{
	UpdateJobAndImage();
	return false;
}


bool emPsPagePanel::IsOpaque()
{
	return true;
}


void emPsPagePanel::Paint(
	const emPainter & painter, emColor canvasColor
)
{
	static const emColor bgCol=emColor(221,255,255);
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

	if (JobState==emPsRenderer::JS_ERROR) {
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
	else if (JobState!=emPsRenderer::JS_SUCCESS) {
		if (JobState==emPsRenderer::JS_WAITING) ico=WaitIcon;
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
	emPsRenderer::JobState s;
	emString e;
	double hrel,wmin,hmin,wmax,hmax,t;
	int iw,ih;
	emImage * oldImg;

	static const double maxImageArea=5000*5000; //??? To be reconsidered
	static const double maxImageSize=10000;     //???   every 3 years.

	if (JobState==emPsRenderer::JS_ERROR) return;

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
		else if (Image.GetWidth()<=wmin-1.0 || Image.GetHeight()<=hmin-1.0) {
			iw=(int)(wmax+0.5);
			ih=(int)(hmax+0.5);
		}
		else if (Image.GetWidth()>=wmax+1.0 || Image.GetHeight()>=hmax+1.0) {
			iw=(int)(wmin+0.5);
			ih=(int)(hmin+0.5);
		}
		else {
			iw=Image.GetWidth();
			ih=Image.GetHeight();
		}
	}

	if (Image.GetWidth()!=iw || Image.GetHeight()!=ih) {
		if (Job) {
			Renderer->CloseJob(Job);
			Job=NULL;
			JobState=emPsRenderer::JS_SUCCESS;
		}
		if (iw<=0 || ih<=0) {
			Image.Empty();
		}
		else {
			if (Image.IsEmpty()) oldImg=NULL;
			else oldImg=new emImage(Image);
			Image.Setup(iw,ih,3);
			if (oldImg) {
				Image.CopyTransformed(
					0,0,iw,ih,
					emScaleATM(
						((double)iw)/oldImg->GetWidth(),
						((double)ih)/oldImg->GetHeight()
					),
					*oldImg,
					false,
					emColor::WHITE
				);
				delete oldImg;
			}
			else {
				Image.Fill(emColor(238,255,255));
			}
			Job=Renderer->StartJob(
				Document,
				PageIndex,
				Image,
				GetUpdatePriority(),
				this
			);
		}
		InvalidatePainting();
	}

	if (Job) {
		s=Renderer->GetJobState(Job);
		if (JobState!=s) {
			JobState=s;
			InvalidatePainting();
		}
		if (s==emPsRenderer::JS_ERROR) {
			JobErrorText=Renderer->GetJobErrorText(Job);
			Renderer->CloseJob(Job);
			Job=NULL;
			Image.Empty();
		}
		else if (s==emPsRenderer::JS_SUCCESS) {
			Renderer->CloseJob(Job);
			Job=NULL;
		}
	}
}
