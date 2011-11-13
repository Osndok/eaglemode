//------------------------------------------------------------------------------
// emAvFilePanel.cpp
//
// Copyright (C) 2005-2011 Oliver Hamann.
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
#include <emAv/emAvFilePanel.h>
#include <emAv/emAvFileControlPanel.h>


emAvFilePanel::emAvFilePanel(
	ParentArg parent, const emString & name, emAvFileModel * fileModel,
	bool updateFileModel
)
	: emFilePanel(parent,name),
	CursorTimer(GetScheduler())
{
	CursorHidden=false;
	ScreensaverDisabled=false;
	HaveControlPanel=false;
	WarningStartTime=0;
	WarningAlpha=0;
	OldMouseX=0.0;
	OldMouseY=0.0;
	UpdateEssenceRect();
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(CursorTimer.GetSignal());
	SetFileModel(fileModel,updateFileModel);
}


emAvFilePanel::~emAvFilePanel()
{
	if (ScreensaverDisabled) {
		ScreensaverDisabled=false;
		if (GetScreen()) GetScreen()->EnableScreensaver();
	}
}


void emAvFilePanel::SetFileModel(emFileModel * fileModel, bool updateFileModel)
{
	emAvFileModel * fm;

	if (fileModel && (dynamic_cast<emAvFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}

	fm=(emAvFileModel*)GetFileModel();
	if (fm) {
		RemoveWakeUpSignal(fm->GetInfoSignal());
		RemoveWakeUpSignal(fm->GetPlayStateSignal());
		RemoveWakeUpSignal(fm->GetImageSignal());
	}

	emFilePanel::SetFileModel(fileModel,updateFileModel);

	fm=(emAvFileModel*)GetFileModel();
	if (fm) {
		AddWakeUpSignal(fm->GetInfoSignal());
		AddWakeUpSignal(fm->GetPlayStateSignal());
		AddWakeUpSignal(fm->GetImageSignal());
	}
}


void emAvFilePanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	if (GetVirFileState()==VFS_LOADED) {
		*pX=EX;
		*pY=EY;
		*pW=EW;
		*pH=EH;
	}
	else {
		emFilePanel::GetEssenceRect(pX,pY,pW,pH);
	}
}


bool emAvFilePanel::Cycle()
{
	emAvFileModel * fm;
	bool busy,loaded;
	int t;

	busy=emFilePanel::Cycle();

	loaded=(GetVirFileState()==VFS_LOADED);
	fm=(emAvFileModel*)GetFileModel();

	if (IsSignaled(GetVirFileStateSignal())) {
		if (HaveControlPanel!=loaded) {
			HaveControlPanel=loaded;
			InvalidateControlPanel();
		}
		UpdateEssenceRect();
		InvalidatePainting();
	}

	if (loaded) {
		if (
			IsSignaled(fm->GetInfoSignal()) ||
			IsSignaled(fm->GetPlayStateSignal()) ||
			IsSignaled(fm->GetImageSignal())
		) {
			InvalidatePainting(EX,EY,EW,EH);
			UpdateEssenceRect();
			InvalidatePainting(EX,EY,EW,EH);
		}
	}

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		(loaded && IsSignaled(fm->GetInfoSignal()))
	) {
		if (!loaded) BgImage.Empty();
		else {
			BgImage=emGetInsResImage(
				GetRootContext(),"emAv",
				fm->IsVideo() ? "Masks.tga" : "Notes.tga"
			);
		}
		if (WarningText!=fm->GetWarningText()) {
			WarningText=fm->GetWarningText();
			if (!WarningText.IsEmpty()) {
				WarningStartTime=emGetClockMS();
				WarningAlpha=255;
			}
			else {
				WarningAlpha=0;
			}
		}
	}

	if (WarningAlpha) {
		t=emGetClockMS()-WarningStartTime;
		t=emMin(128+127*t/500,255-(t-1500)*192/2500);
		if (t<24) t=0; else if (t>216) t=216;
		if (WarningAlpha!=t) {
			WarningAlpha=t;
			InvalidatePainting(EX,EY,EW,EH);
		}
		if (WarningAlpha>0) busy=true;
	}

	if (IsSignaled(CursorTimer.GetSignal())) {
		CursorHidden=true;
		InvalidateCursor();
	}

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		(loaded && IsSignaled(fm->GetPlayStateSignal()))
	) {
		UpdateCursorHiding(false);
		UpdateScreensaverDisabling();
	}

	return busy;
}


void emAvFilePanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if (flags&NF_LAYOUT_CHANGED) UpdateEssenceRect();
	if (flags&NF_FOCUS_CHANGED) UpdateCursorHiding(false);
	if (flags&NF_VIEWING_CHANGED) {
		UpdateCursorHiding(true);
		UpdateScreensaverDisabling();
	}
}


void emAvFilePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	static const int posStep=20000;
	static const int volStep=5;
	emAvFileModel * fm;
	bool adjustingEnabled;

	if (GetVirFileState()!=VFS_LOADED) {
		emFilePanel::Input(event,state,mx,my);
		return;
	}

	fm=(emAvFileModel*)GetFileModel();
	adjustingEnabled=(fm->GetPlayState()!=emAvFileModel::PS_STOPPED);

	switch (event.GetKey()) {
	case EM_KEY_LEFT_BUTTON:
		if (state.IsNoMod() && mx>=EX && my>=EY && mx<EX+EW && my<EY+EH) {
			switch (fm->GetPlayState()) {
			case emAvFileModel::PS_STOPPED:
			case emAvFileModel::PS_PAUSED:
				fm->Play();
				break;
			default:
				fm->Pause();
				break;
			}
			Focus();
			event.Eat();
		}
		break;
	case EM_KEY_RIGHT_BUTTON:
		if (state.IsNoMod() && mx>=EX && my>=EY && mx<EX+EW && my<EY+EH) {
			switch (fm->GetPlayState()) {
			case emAvFileModel::PS_STOPPED:
				fm->PlaySolely();
				break;
			default:
				fm->Stop();
				break;
			}
			Focus();
			event.Eat();
		}
		break;
	case EM_KEY_SPACE:
		if (state.IsNoMod()) {
			switch (fm->GetPlayState()) {
			case emAvFileModel::PS_STOPPED:
			case emAvFileModel::PS_PAUSED:
				fm->Play();
				break;
			default:
				fm->Pause();
				break;
			}
			event.Eat();
		}
		break;
	case EM_KEY_0:
		if (state.IsNoMod()) {
			fm->Stop();
			event.Eat();
		}
		break;
	case EM_KEY_1:
		if (state.IsNoMod()) {
			fm->SetPlayPos(0);
			event.Eat();
		}
		break;
	case EM_KEY_2:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*1);
			event.Eat();
		}
		break;
	case EM_KEY_3:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*2);
			event.Eat();
		}
		break;
	case EM_KEY_4:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*3);
			event.Eat();
		}
		break;
	case EM_KEY_5:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*4);
			event.Eat();
		}
		break;
	case EM_KEY_6:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*5);
			event.Eat();
		}
		break;
	case EM_KEY_7:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*6);
			event.Eat();
		}
		break;
	case EM_KEY_8:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*7);
			event.Eat();
		}
		break;
	case EM_KEY_9:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayLength()/9*8);
			event.Eat();
		}
		break;
	case EM_KEY_A:
		if ((state.IsCtrlMod() || state.IsShiftCtrlMod()) && adjustingEnabled) {
			fm->SetAudioChannel(fm->GetAudioChannel()+(state.GetShift()?-1:1));
			event.Eat();
		}
		break;
	case EM_KEY_D:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayPos()-posStep);
			event.Eat();
		}
		break;
	case EM_KEY_F:
		if (state.IsNoMod()) {
			fm->PlayFast();
			event.Eat();
		}
		break;
	case EM_KEY_I:
		if (state.IsNoMod()) {
			fm->SetPlayPos(fm->GetPlayPos()+posStep);
			event.Eat();
		}
		break;
	case EM_KEY_N:
		if (state.IsNoMod()) {
			fm->Play();
			event.Eat();
		}
		break;
	case EM_KEY_P:
		if (state.IsNoMod()) {
			fm->Pause();
			event.Eat();
		}
		break;
	case EM_KEY_S:
		if (state.IsNoMod()) {
			fm->PlaySlow();
			event.Eat();
		}
		else if ((state.IsCtrlMod() || state.IsShiftCtrlMod()) && adjustingEnabled) {
			fm->SetSpuChannel(fm->GetSpuChannel()+(state.GetShift()?-1:1));
			event.Eat();
		}
		break;
	case EM_KEY_U:
		if (state.IsNoMod() && adjustingEnabled) {
			fm->SetAudioMute(!fm->GetAudioMute());
			event.Eat();
		}
		break;
	case EM_KEY_Z:
		if ((state.IsCtrlMod() || state.IsShiftCtrlMod()) && adjustingEnabled) {
			fm->SetAudioVisu(fm->GetAudioVisu()+(state.GetShift()?-1:1));
			event.Eat();
		}
		break;
	default:
		break;
	}

	if (event.GetChars()=="+" && adjustingEnabled) {
		fm->SetAudioVolume(fm->GetAudioVolume()+volStep);
		event.Eat();
	}
	else if (event.GetChars()=="-" && adjustingEnabled) {
		fm->SetAudioVolume(fm->GetAudioVolume()-volStep);
		event.Eat();
	}

	if (
		fabs(OldMouseX-state.GetMouseX())>2.5 ||
		fabs(OldMouseY-state.GetMouseY())>2.5 ||
		state.GetLeftButton() ||
		state.GetMiddleButton() ||
		state.GetRightButton()
	) {
		OldMouseX=state.GetMouseX();
		OldMouseY=state.GetMouseY();
		UpdateCursorHiding(true);
	}

	emFilePanel::Input(event,state,mx,my);
}


emCursor emAvFilePanel::GetCursor()
{
	if (CursorHidden) return emCursor::INVISIBLE;
	else return emFilePanel::GetCursor();
}


bool emAvFilePanel::IsOpaque()
{
	if (GetVirFileState()==VFS_LOADED) return true;
	else return emFilePanel::IsOpaque();
}


void emAvFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emAvFileModel * fm;
	const emImage * image;
	double xy[2*10];
	double x1,y1,x2,y2,x,y,w,h,d,e,t;
	emColor c1,c2,c3,c4;

	if (GetVirFileState()!=VFS_LOADED) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	fm=(emAvFileModel*)GetFileModel();
	image=&fm->GetImage();

	c1=0x000000FF;
	h=GetHeight();
	x1=painter.RoundUpX(EX);
	y1=painter.RoundUpY(EY);
	x2=painter.RoundDownX(EX+EW);
	y2=painter.RoundDownY(EY+EH);
	if (x1>=x2 || y1>=y2) {
		painter.PaintRect(0,0,1,h,c1,canvasColor);
		canvasColor=c1;
	}
	else {
		xy[ 0]=0.0; xy[ 1]=0.0;
		xy[ 2]=1.0; xy[ 3]=0.0;
		xy[ 4]=1.0; xy[ 5]=h;
		xy[ 6]=0.0; xy[ 7]=h;
		xy[ 8]=0.0; xy[ 9]=0.0;
		xy[10]=x1;  xy[11]=y1;
		xy[12]=x1;  xy[13]=y2;
		xy[14]=x2;  xy[15]=y2;
		xy[16]=x2;  xy[17]=y1;
		xy[18]=x1;  xy[19]=y1;
		painter.PaintPolygon(xy,10,c1,canvasColor);
		canvasColor=0;
	}

	if (fm->GetPlayState()==emAvFileModel::PS_STOPPED) {
		if (fm->IsVideo()) {
			c1=0x666677FF;
			c2=0x556666FF;
			c3=0xFFFFFFFF;
			c4=0xBBFFDDD0;
		}
		else {
			c1=0x556666FF;
			c2=0x666677FF;
			c3=0xFFFFFFFF;
			c4=0xBBEEDDD0;
		}
		painter.PaintRect(EX,EY,EW,EH,c1,canvasColor);
		canvasColor=c1;
		if (!BgImage.IsEmpty()) {
			w=EW*0.9;
			h=EH*0.9;
			t=BgImage.GetHeight()/(double)BgImage.GetWidth();
			if (h>=t*w) h=w*t; else w=h/t;
			x=EX+(EW-w)*0.5;
			y=EY+(EH-h)*0.5;
			painter.PaintShape(
				x,y,w,h,
				BgImage,0,
				c2,
				canvasColor
			);
			canvasColor=0;
		}
		d=emMin(EW,EH)*0.05;
		if (fm->GetPlayPos()>0) e=d*2.5; else e=0;
		painter.PaintTextBoxed(
			EX+d,
			EY+d,
			EW-2*d,
			EH-2*d-e,
			fm->GetInfoText(),
			EH,
			c3,
			canvasColor,
			EM_ALIGN_TOP_LEFT,
			EM_ALIGN_LEFT
		);
		if (fm->GetPlayPos()>0) {
			x1=EX+d;
			y1=EY+EH-e-d*0.4;
			x2=EX+EW-d;
			y2=EY+EH-d;
			d=e*0.09;
			painter.PaintRectOutline(
				x1+d*0.5,
				y1+d*0.5,
				x2-x1-d,
				y2-y1-d,
				d,
				c3,
				canvasColor
			);
			d*=1.7;
			x1+=d;
			y1+=d;
			x2-=d;
			y2-=d;
			painter.PaintRect(
				x1,
				y1,
				(x2-x1)*fm->GetPlayPos()/fm->GetPlayLength(),
				y2-y1,
				c3,
				canvasColor
			);
		}
		d=emMin(EW,EH)*0.02;
		painter.PaintTextBoxed(
			EX,
			EY+EH-d,
			EW,
			d,
			"Left mouse button: Start or pause playing.  "
			"Right mouse button: Start playing solely or stop playing.",
			d,
			c4,
			canvasColor,
			EM_ALIGN_BOTTOM,
			EM_ALIGN_LEFT
		);
	}
	else if (image->IsEmpty()) {
		c1=fm->IsVideo() ? 0x222233FF : 0x112222FF;
		painter.PaintRect(EX,EY,EW,EH,c1,canvasColor);
		canvasColor=c1;
	}
	else {
		painter.PaintImage(EX,EY,EW,EH,*image,255,canvasColor);
		canvasColor=0;
	}

	if (WarningAlpha) {
		w=EW*0.95;
		h=EH*0.15;
		x=EX+(EW-w)*0.5;
		y=EY+(EH-h)*0.5;
		c1=emColor(136,0,0,WarningAlpha);
		c2=emColor(255,255,0,WarningAlpha);
		painter.PaintRect(x,y,w,h,c1,canvasColor);
		d=h*0.02;
		painter.PaintRectOutline(x-d,y-d,w+2*d,h+2*d,d*2,c2);
		d=h*0.1;
		painter.PaintTextBoxed(x+d,y+d,w-2*d,h-2*d,WarningText,h,c2);
	}
}


emPanel * emAvFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	emAvFileModel * fm;

	fm=(emAvFileModel*)GetFileModel();
	if (HaveControlPanel && fm) {
		return new emAvFileControlPanel(parent,name,fm);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emAvFilePanel::UpdateEssenceRect()
{
	emAvFileModel * fm;
	double h,ew,eh,t;

	h=GetHeight();
	if (GetVirFileState()==VFS_LOADED) {
		fm=(emAvFileModel*)GetFileModel();
		ew=emMin(1.0,h*(1280.0/1024.0));
		eh=emMin(h,1.0/2.11*0.9);
		t=fm->GetTallness();
		if (eh>=ew*t) eh=ew*t; else ew=eh/t;
	}
	else {
		ew=1.0;
		eh=h;
	}
	EX=(1.0-ew)*0.5;
	EY=(h-eh)*0.5;
	EW=ew;
	EH=eh;
}


void emAvFilePanel::UpdateCursorHiding(bool restart)
{
	bool toHide;

	toHide=
		IsFocused() &&
		IsViewed() &&
		(GetClipX2()-GetClipX1())*(GetClipY2()-GetClipY1()) >
			0.6*GetView().GetCurrentWidth()*GetView().GetCurrentHeight() &&
		GetVirFileState()==VFS_LOADED &&
		((emAvFileModel*)GetFileModel())->GetPlayState()!=emAvFileModel::PS_STOPPED
	;
	if (!toHide || restart) {
		if (CursorHidden) {
			CursorHidden=false;
			InvalidateCursor();
		}
		CursorTimer.Stop(true);
	}
	if (toHide && !CursorHidden && !CursorTimer.IsRunning()) {
		CursorTimer.Start(3000);
	}
}


void emAvFilePanel::UpdateScreensaverDisabling()
{
	emScreen * screen;
	emAvFileModel * fm;
	emAvFileModel::PlayStateType playState;
	double a,sx,sy,sw,sh;

	screen=GetScreen();
	if (!screen) return;
	for (;;) {
		if (!IsViewed()) break;
		if (GetVirFileState()!=VFS_LOADED) break;
		fm=(emAvFileModel*)GetFileModel();
		playState=fm->GetPlayState();
		if (playState==emAvFileModel::PS_STOPPED) break;
		if (playState==emAvFileModel::PS_PAUSED) break;
		if (!fm->IsVideo()) break;
		a=(GetClipX2()-GetClipX1())*(GetClipY2()-GetClipY1());
		screen->GetVisibleRect(&sx,&sy,&sw,&sh);
		if (a<0.6*sw*sh) break;
		if (!ScreensaverDisabled) {
			ScreensaverDisabled=true;
			screen->DisableScreensaver();
		}
		return;
	}
	if (ScreensaverDisabled) {
		ScreensaverDisabled=false;
		screen->EnableScreensaver();
	}
}
