//------------------------------------------------------------------------------
// emGifFilePanel.cpp
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#include <emCore/emToolkit.h>
#include <emGif/emGifFilePanel.h>


emGifFilePanel::emGifFilePanel(
	ParentArg parent, const emString & name,
	emGifFileModel * fileModel, bool updateFileModel
)
	: emFilePanel(parent,name), Timer(GetScheduler())
{
	RIndex=-1;
	Playing=false;
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(Timer.GetSignal());
	SetFileModel(fileModel,updateFileModel);
}


emGifFilePanel::~emGifFilePanel()
{
}


void emGifFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (fileModel && (dynamic_cast<emGifFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}
	emFilePanel::SetFileModel(fileModel,updateFileModel);
}


void emGifFilePanel::StopPlaying()
{
	if (Playing) {
		Timer.Stop(true);
		Playing=false;
		Signal(PlaySignal);
	}
}


void emGifFilePanel::ContinuePlaying()
{
	emGifFileModel * gfm;

	gfm=(emGifFileModel *)GetFileModel();
	if (!Image.IsEmpty() && gfm && gfm->IsAnimated()) {
		if (!Playing) {
			Playing=true;
			Timer.Start(0);
			Signal(PlaySignal);
		}
		else if (gfm->GetRenderInput(RIndex)) {
			Timer.Start(0);
		}
	}
}


void emGifFilePanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	if (Image.IsEmpty() || !GetFileModel()) {
		emFilePanel::GetEssenceRect(pX,pY,pW,pH);
	}
	else {
		CalcImageLayout(pX,pY,pW,pH);
	}
}


bool emGifFilePanel::Cycle()
{
	emGifFileModel * gfm;
	bool oldPlaying,baseBusy,init;
	int x,y,w,h;

	baseBusy=emFilePanel::Cycle();

	if (IsSignaled(GetVirFileStateSignal())) {
		InvalidateControlPanel(); //??? very cheap solution, but okay for now.
	}

	oldPlaying=Playing;
	if (IsSignaled(GetVirFileStateSignal()) || GetVirFileState()!=VFS_LOADED) {
		if (!Image.IsEmpty()) {
			Image.Empty();
			InvalidatePainting();
		}
		UndoImage.Empty();
		RIndex=-1;
		Playing=false;
		Timer.Stop(true);
	}
	init=false;
	gfm=(emGifFileModel *)GetFileModel();
	if (GetVirFileState()==VFS_LOADED && Image.IsEmpty() &&
	    gfm->GetWidth()>0 && gfm->GetHeight()>0) {
		Image.Setup(gfm->GetWidth(),gfm->GetHeight(),gfm->GetChannelCount());
		RIndex=-1;
		InvalidatePainting();
		Playing=true;
		init=true;
	}
	if (Playing && (init || IsSignaled(Timer.GetSignal()))) {
		for (;;) {
			if (gfm->GetRenderCount()<=0) {
				Image.Fill(gfm->GetBGColor());
				InvalidatePerImage(0,0,Image.GetWidth(),Image.GetHeight());
				Playing=false;
				RIndex=-1;
				break;
			}
			if (RIndex<0 || RIndex>=gfm->GetRenderCount()-1) {
				if (
					gfm->IsRenderTransparent(0) ||
					gfm->GetRenderX(0)!=0 ||
					gfm->GetRenderY(0)!=0 ||
					gfm->GetRenderWidth(0)!=Image.GetWidth() ||
					gfm->GetRenderHeight(0)!=Image.GetHeight()
				) {
					Image.Fill(gfm->GetBGColor());
					InvalidatePerImage(0,0,Image.GetWidth(),Image.GetHeight());
				}
				RIndex=0;
			}
			else {
				x=gfm->GetRenderX(RIndex);
				y=gfm->GetRenderY(RIndex);
				w=gfm->GetRenderWidth(RIndex);
				h=gfm->GetRenderHeight(RIndex);
				if (gfm->GetRenderDisposal(RIndex)==2) {
					Image.Fill(x,y,w,h,gfm->GetBGColor());
					InvalidatePerImage(x,y,w,h);
				}
				else if (gfm->GetRenderDisposal(RIndex)==3) {
					if (!UndoImage.IsEmpty()) {
						Image.Copy(x,y,UndoImage,0,0,w,h);
					}
					else {
						Image.Fill(x,y,w,h,gfm->GetBGColor());
					}
					InvalidatePerImage(x,y,w,h);
				}
				RIndex++;
			}
			x=gfm->GetRenderX(RIndex);
			y=gfm->GetRenderY(RIndex);
			w=gfm->GetRenderWidth(RIndex);
			h=gfm->GetRenderHeight(RIndex);
			if (gfm->GetRenderDisposal(RIndex)==3) {
				UndoImage.Setup(w,h,Image.GetChannelCount());
				UndoImage.Copy(-x,-y,Image);
			}
			else {
				UndoImage.Empty();
			}
			gfm->RenderImage(RIndex,&Image);
			InvalidatePerImage(x,y,w,h);
			if (gfm->GetRenderDelay(RIndex)>0) {
				Timer.Start(gfm->GetRenderDelay(RIndex)*10);
				break;
			}
			if (gfm->GetRenderInput(RIndex)) {
				Playing=false;
				break;
			}
			if (RIndex==gfm->GetRenderCount()-1) {
				Playing=false;
				break;
			}
		}
	}
	if (Playing!=oldPlaying) {
		Signal(PlaySignal);
	}

	return baseBusy;
}


void emGifFilePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (event.IsLeftButton() && state.IsNoMod()) {
		if (IsPlaying()) StopPlaying();
		else ContinuePlaying();
		Focus();
		event.Eat();
	}
	emPanel::Input(event,state,mx,my);
}


bool emGifFilePanel::IsOpaque()
{
	if (Image.IsEmpty()) return emFilePanel::IsOpaque();
	else return false;
}


void emGifFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double x,y,w,h;

	if (Image.IsEmpty()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}
	CalcImageLayout(&x,&y,&w,&h);
	painter.PaintImage(
		x,y,w,h,
		Image,
		255,
		canvasColor
	);
}


emPanel * emGifFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	emGifFileModel * gfm;
	emTkGroup * grp;
	emTkTextField * tf;
	emString str;

	gfm=(emGifFileModel *)GetFileModel();
	if (gfm && IsVFSGood()) {
		grp=new emTkGroup(
			parent,
			name,
			"GIF File Info"
		);
		grp->SetFixedColumnCount(1);
		if (gfm->IsAnimated()) {
			str=emString::Format(
				"Animated GIF (%d frames)",
				gfm->GetRenderCount()
			);
		}
		else {
			str="GIF";
		}
		new emTkTextField(
			grp,
			"format",
			"File Format",
			emString(),
			emImage(),
			str
		);
		new emTkTextField(
			grp,
			"size",
			"Size",
			emString(),
			emImage(),
			emString::Format(
				"%dx%d pixels",
				gfm->GetWidth(),
				gfm->GetHeight()
			)
		);
		tf=new emTkTextField(
			grp,
			"comment",
			"Comment",
			emString(),
			emImage(),
			gfm->GetComment()
		);
		tf->SetMultiLineMode();
		return grp;
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emGifFilePanel::CalcImageLayout(
	double * pX, double * pY, double * pW, double * pH
) const
{
	emGifFileModel * gfm;
	double x,y,w,h,d,t;

	x=0;
	y=0;
	w=1;
	h=GetHeight();

	gfm=(emGifFileModel *)GetFileModel();
	if (gfm) {
		t=gfm->GetTallness();
		if (h/w>=t) {
			d=w*t;
			y+=(h-d)/2;
			h=d;
		}
		else {
			d=h/t;
			x+=(w-d)/2;
			w=d;
		}
	}

	*pX=x;
	*pY=y;
	*pW=w;
	*pH=h;
}


void emGifFilePanel::InvalidatePerImage(int x, int y, int w, int h)
{
	double lx,ly,lw,lh,fx,fy;

	if (!Image.IsEmpty()) {
		CalcImageLayout(&lx,&ly,&lw,&lh);
		fx=lw/Image.GetWidth();
		fy=lh/Image.GetHeight();
		InvalidatePainting(lx+x*fx,ly+y*fy,w*fx,h*fy);
	}
}
