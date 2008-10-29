//------------------------------------------------------------------------------
// emDirEntryPanel.cpp
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

#include <sys/stat.h>
#if defined(_WIN32)
#	include <io.h>
#	ifndef R_OK
#		define R_OK 4
#	endif
#	ifndef S_IRUSR
#		define S_IRUSR _S_IREAD
#	endif
#	ifndef S_IWUSR
#		define S_IWUSR _S_IWRITE
#	endif
#	ifndef S_IXUSR
#		define S_IXUSR _S_IEXEC
#	endif
#	ifndef S_IRGRP
#		define S_IRGRP S_IRUSR
#	endif
#	ifndef S_IWGRP
#		define S_IWGRP S_IWUSR
#	endif
#	ifndef S_IXGRP
#		define S_IXGRP S_IXUSR
#	endif
#	ifndef S_IROTH
#		define S_IROTH S_IRGRP
#	endif
#	ifndef S_IWOTH
#		define S_IWOTH S_IWGRP
#	endif
#	ifndef S_IXOTH
#		define S_IXOTH S_IXGRP
#	endif
#else
#	include <unistd.h>
#endif
#include <emCore/emFpPlugin.h>
#include <emFileMan/emDirPanel.h>
#include <emFileMan/emDirEntryPanel.h>
#include <emFileMan/emDirEntryAltPanel.h>
#include <emFileMan/emFileManControlPanel.h>
#include <emCore/emRes.h>


emDirEntryPanel::emDirEntryPanel(
	ParentArg parent, const emString & name, const emDirEntry & dirEntry
)
	: emPanel(parent,name), DirEntry(dirEntry)
{
	SharedVar=emVarModel<SharedStuff>::Acquire(GetRootContext(),"");
	if (!SharedVar->Var.FileMan) {
		SharedVar->Var.FileMan=emFileManModel::Acquire(GetRootContext());
		SharedVar->Var.InnerBorderImage=emGetInsResImage(
			GetRootContext(),"emFileMan","images/InnerBorder.tga"
		);
		SharedVar->Var.OuterBorderImage=emGetInsResImage(
			GetRootContext(),"emFileMan","images/OuterBorder.tga"
		);
	}

	AddWakeUpSignal(SharedVar->Var.FileMan->GetSelectionSignal());

	UpdateBgColor();
}


emDirEntryPanel::~emDirEntryPanel()
{
}


void emDirEntryPanel::UpdateDirEntry(const emDirEntry & dirEntry)
{
	emPanel * p;

	if (DirEntry == dirEntry) return;

	if (dirEntry.GetPath() != DirEntry.GetPath()) {
		emFatalError("emDirEntryPanel::UpdateDirEntry: different path");
	}

	p=GetChild(AltName);
	if (p) {
		((emDirEntryAltPanel*)p)->UpdateDirEntry(dirEntry);
	}

	DirEntry=dirEntry;
	InvalidatePainting();
}


emString emDirEntryPanel::GetTitle()
{
	return DirEntry.GetPath();
}


bool emDirEntryPanel::Cycle()
{
	if (IsSignaled(SharedVar->Var.FileMan->GetSelectionSignal())) {
		UpdateBgColor();
	}
	return false;
}


void emDirEntryPanel::Notice(NoticeFlags flags)
{
	const char * soughtName;
	emRef<emFpPluginList> fppl;
	emPanel * p;

	if ((flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED|NF_VISIT_CHANGED))!=0) {
		soughtName=GetSoughtName();
		p=GetChild(ContentName);
		if (
			(
				soughtName &&
				strcmp(soughtName,ContentName.Get())==0
			) ||
			(
				IsViewed() &&
				GetViewedWidth()*LayoutContentW>=MinContentVW &&
				PanelToViewX(LayoutContentX)<GetClipX2() &&
				PanelToViewX(LayoutContentX+LayoutContentW)>GetClipX1() &&
				PanelToViewY(LayoutContentY)<GetClipY2() &&
				PanelToViewY(LayoutContentY+LayoutContentH)>GetClipY1()
			)
		) {
			if (!p) {
				fppl=emFpPluginList::Acquire(GetRootContext());
				p=fppl->CreateFilePanel(
					this,
					ContentName,
					DirEntry.GetPath(),
					DirEntry.GetStatErrNo(),
					DirEntry.GetStat()->st_mode
				);
				p->BeFirst();
				p->Layout(
					LayoutContentX,LayoutContentY,
					LayoutContentW,LayoutContentH,
					ColorBGNormal
				);
			}
		}
		else if (p && !p->IsInVisitedPath()) {
			delete p;
		}
		p=GetChild(AltName);
		if (
			(
				soughtName &&
				strcmp(soughtName,AltName.Get())==0
			) ||
			(
				IsViewed() &&
				GetViewedWidth()*LayoutAltW>=MinAltVW &&
				PanelToViewX(LayoutAltX)<GetClipX2() &&
				PanelToViewX(LayoutAltX+LayoutAltW)>GetClipX1() &&
				PanelToViewY(LayoutAltY)<GetClipY2() &&
				PanelToViewY(LayoutAltY+LayoutAltH)>GetClipY1()
			)
		) {
			if (!p) {
				p=new emDirEntryAltPanel(
					this,
					AltName,
					DirEntry,
					1
				);
				p->Layout(
					LayoutAltX,LayoutAltY,
					LayoutAltW,LayoutAltH,
					BgColor
				);
			}
		}
		else if (p && !p->IsInVisitedPath()) {
			delete p;
		}
	}
}


void emDirEntryPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (event.IsKeyboardEvent() && !IsActive()) {
		event.Eat();
	}

	switch (event.GetKey()) {
	case EM_KEY_LEFT_BUTTON:
		if (event.GetRepeat() && state.IsNoMod()) {
			SelectSolely();
			RunDefaultCommand();
			Focus();
			event.Eat();
		}
		else if (!state.GetAlt() && !state.GetMeta()) {
			Select(state.GetShift(),state.GetCtrl());
			Focus();
			event.Eat();
		}
		break;
/* ??? The good old context menu no longer exists...
	case EM_KEY_RIGHT_BUTTON:
		if (!state.GetAlt() && !state.GetMeta()) {
			if (!fm->IsSelectedAsTarget(DirEntry.GetPath())) {
				Select(state.GetShift(),state.GetCtrl());
			}
			ShowContextMenu();
			Focus();
			event.Eat();
		}
		break;
*/
	case EM_KEY_ENTER:
		if (state.IsNoMod()) {
			SelectSolely();
			RunDefaultCommand();
			event.Eat();
		}
		break;
	case EM_KEY_SPACE:
		if (!state.GetAlt() && !state.GetMeta()) {
			Select(state.GetShift(),state.GetCtrl());
			event.Eat();
		}
		break;
	default:
		break;
	}

	emPanel::Input(event,state,mx,my);

	if (event.IsKeyboardEvent()) {
		SharedVar->Var.FileMan->HotkeyInput(GetView(),event,state);
	}
}


bool emDirEntryPanel::IsOpaque()
{
	return false;
}


void emDirEntryPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emColor color;
	char tmp[1024];
	const char * p;
	double w,ws,ch,cw,ch2,x,y,x1,y1;
	int i,j,k,len;

	painter.PaintRoundRect(
		0.0,
		0.0,
		1-LayoutFrame*(32.0/96.0),
		HeightFactor-LayoutFrame*(32.0/96.0),
		LayoutFrame,
		LayoutFrame,
		BgColor,
		canvasColor
	);
	if (!canvasColor.IsOpaque() || BgColor!=canvasColor) canvasColor=0;
	painter.PaintBorderImage(
		0,0,1,HeightFactor,
		LayoutFrame*(124.0/96.0),
		LayoutFrame*(121.0/96.0),
		LayoutFrame*(129.0/96.0),
		LayoutFrame*(128.0/96.0),
		SharedVar->Var.OuterBorderImage,
		124.0,
		121.0,
		129.0,
		128.0,
		255,
		canvasColor,
		0757
	);
	canvasColor=BgColor;

	if (DirEntry.IsRegularFile()) {
		if (DirEntry.GetStat()->st_mode&(S_IXUSR|S_IXGRP|S_IXOTH))
			color=ColorNameExe;
		else
			color=ColorNameNormal;
	}
	else if (DirEntry.IsDirectory()) color=ColorNameDir;
#if !defined(_WIN32)
	else if (S_ISFIFO(DirEntry.GetStat()->st_mode)) color=ColorNameFifo;
	else if (S_ISBLK (DirEntry.GetStat()->st_mode)) color=ColorNameBlk;
	else if (S_ISCHR (DirEntry.GetStat()->st_mode)) color=ColorNameChr;
	else if (S_ISSOCK(DirEntry.GetStat()->st_mode)) color=ColorNameSock;
#endif
	else color=ColorNameOther;
	if (DirEntry.IsHidden()) color=color.GetTransparented(27.0F);
	painter.PaintTextBoxed(
		LayoutTitleX,
		LayoutTitleY,
		LayoutTitleW,
		LayoutTitleH,
		DirEntry.GetName(),
		LayoutTitleH,
		color,
		canvasColor,
		EM_ALIGN_LEFT,
		EM_ALIGN_LEFT,
		0.5,
		false
	);

	if (BgColor==ColorBGTgt || BgColor==ColorBGSrc) {
		if (BgColor==ColorBGTgt) {
			p="Target-Selected";
			color=0x440000C0;
		}
		else {
			p="Source-Selected";
			color=0x004400C0;
		}
		painter.PaintTextBoxed(
			LayoutTitleX,
			LayoutFrame,
			LayoutTitleW,
			LayoutTitleH,
			p,
			LayoutTitleH*0.12,
			color,
			0,
			EM_ALIGN_TOP_RIGHT
		);
	}

	ch=LayoutInfoH*0.21;
	if (ch*GetViewedWidth()>1.0) {
		cw=painter.GetTextSize("X",ch,false);
		ch2=ch*0.15;
		x1=LayoutInfoX;
		y1=LayoutInfoY;
		w=LayoutInfoW;

		x=x1;
		y=y1;

		if (ch2*GetViewedWidth()>1.0) {
			painter.PaintText(x,y,"Type",ch2,1.0,ColorInfoLabel,canvasColor);
		}
		y+=ch2;

		if      (DirEntry.IsRegularFile())              p="File";
		else if (DirEntry.IsDirectory())                p="Directory";
#if !defined(_WIN32)
		else if (S_ISFIFO(DirEntry.GetStat()->st_mode)) p="FIFO";
		else if (S_ISBLK (DirEntry.GetStat()->st_mode)) p="Block Device";
		else if (S_ISCHR (DirEntry.GetStat()->st_mode)) p="Char Device";
		else if (S_ISSOCK(DirEntry.GetStat()->st_mode)) p="Socket";
#endif
		else                                            p="Unknown Type";
		if (DirEntry.IsSymbolicLink()) {
			sprintf(tmp,"Symbolic Link to %s:",p);
			painter.PaintTextBoxed(
				x,
				y,
				w,
				ch/2,
				tmp,
				ch/2,
				ColorSymLink,
				canvasColor,
				EM_ALIGN_LEFT,
				EM_ALIGN_LEFT,
				0.5,
				false
			);
			if (DirEntry.GetTargetPathErrNo()) {
				p=strerror(DirEntry.GetTargetPathErrNo());
			}
			else {
				p=DirEntry.GetTargetPath();
			}
			painter.PaintTextBoxed(
				x,
				y+ch/2,
				w,
				ch/2,
				p,
				ch/2,
				ColorSymLink,
				canvasColor,
				EM_ALIGN_LEFT,
				EM_ALIGN_LEFT,
				0.5,
				false
			);
		}
		else {
			painter.PaintTextBoxed(
				x,
				y,
				w,
				ch,
				p,
				ch,
				ColorInfo,
				canvasColor,
				EM_ALIGN_LEFT,
				EM_ALIGN_LEFT,
				0.5,
				false
			);
		}
		y+=ch*1.1;

		if (ch2*GetViewedWidth()>1.0) {
			painter.PaintText(
				x,y,"Permissions of Owner, Group and Others",ch2,1.0,
				ColorInfoLabel,canvasColor
			);
			painter.PaintText(
				x+w/2+cw/2,y,"Size in Bytes",ch2,1.0,
				ColorInfoLabel,canvasColor
			);
		}
		y+=ch2;
		ws=(w-cw)/(cw*20);
		if (ws>1.0) ws=1.0;
		tmp[0]=((DirEntry.GetStat()->st_mode&S_IRUSR)?'r':'-');
		tmp[1]=((DirEntry.GetStat()->st_mode&S_IWUSR)?'w':'-');
		tmp[2]=((DirEntry.GetStat()->st_mode&S_IXUSR)?'x':'-');
		tmp[3]=0;
		painter.PaintText(x,y,tmp,ch,ws,ColorInfo,canvasColor);
		tmp[0]=((DirEntry.GetStat()->st_mode&S_IRGRP)?'r':'-');
		tmp[1]=((DirEntry.GetStat()->st_mode&S_IWGRP)?'w':'-');
		tmp[2]=((DirEntry.GetStat()->st_mode&S_IXGRP)?'x':'-');
		tmp[3]=0;
		painter.PaintText(x+cw*3.5*ws,y,tmp,ch,ws,ColorInfo,canvasColor);
		tmp[0]=((DirEntry.GetStat()->st_mode&S_IROTH)?'r':'-');
		tmp[1]=((DirEntry.GetStat()->st_mode&S_IWOTH)?'w':'-');
		tmp[2]=((DirEntry.GetStat()->st_mode&S_IXOTH)?'x':'-');
		tmp[3]=0;
		painter.PaintText(x+cw*7*ws,y,tmp,ch,ws,ColorInfo,canvasColor);
		len=emUInt64ToStr(tmp,sizeof(tmp),DirEntry.GetStat()->st_size);
		ws=(w-cw)/(cw*len*32/15);
		if (ws>1.0) ws=1.0;
		x=x1+(w+cw)/2;
		for (i=0; i<len; i+=j) {
			j=(len-i)-(len-i-1)/3*3;
			painter.PaintText(x,y,tmp+i,ch,ws,ColorInfo,canvasColor,j);
			x+=cw*j*ws;
			k=(len-i-j)/3-1;
			if (k>=0) {
				painter.PaintText(
					x,
					y+ch*0.75,
					"kMGTPEZY"+k,
					ch/5,
					ws,
					ColorInfo,
					canvasColor,
					1
				);
			}
			x+=cw/5*ws;
		}
		x=x1;
		y+=ch*1.1;

		if (ch2*GetViewedWidth()>1.0) {
			painter.PaintText(x,y,"Owner",ch2,1.0,ColorInfoLabel,canvasColor);
			painter.PaintText(
				x+w/2+cw/2,y,"Group",ch2,1.0,
				ColorInfoLabel,canvasColor
			);
		}
		y+=ch2;

		painter.PaintTextBoxed(
			x,
			y,
			(w-cw)/2,
			ch,
			DirEntry.GetOwner(),
			ch,
			ColorInfo,
			canvasColor,
			EM_ALIGN_LEFT,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
		painter.PaintTextBoxed(
			x+(w+cw)/2,
			y,
			(w-cw)/2,
			ch,
			DirEntry.GetGroup(),
			ch,
			ColorInfo,
			canvasColor,
			EM_ALIGN_LEFT,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
		y+=ch*1.1;

		if (ch2*GetViewedWidth()>1.0) {
			painter.PaintText(
				x,y,"Time of Last Modification",ch2,1.0,
				ColorInfoLabel,canvasColor
			);
		}
		y+=ch2;
		FormatTime(DirEntry.GetStat()->st_mtime,tmp);
		painter.PaintTextBoxed(
			x,
			y,
			w,
			ch,
			tmp,
			ch,
			ColorInfo,
			canvasColor,
			EM_ALIGN_LEFT,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
	}

	if (GetChild(ContentName) || GetViewedWidth()*LayoutContentW >= MinContentVW) {
		painter.PaintTextBoxed(
			LayoutPathX,
			LayoutPathY,
			LayoutPathW,
			LayoutPathH,
			DirEntry.GetPath(),
			LayoutPathH,
			ColorPath,
			canvasColor,
			EM_ALIGN_BOTTOM_LEFT,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
		painter.PaintBorderImage(
			LayoutContentX-LayoutContentFrame,
			LayoutContentY-LayoutContentFrame,
			LayoutContentW+LayoutContentFrame*2,
			LayoutContentH+LayoutContentFrame*2,
			LayoutContentFrame,
			LayoutContentFrame,
			LayoutContentFrame,
			LayoutContentFrame,
			SharedVar->Var.InnerBorderImage,
			64.0,64.0,64.0,64.0,
			255,canvasColor,0757
		);
		painter.PaintRect(
			LayoutContentX,LayoutContentY,
			LayoutContentW,LayoutContentH,
			ColorBGNormal,canvasColor
		);
	}
}


emPanel * emDirEntryPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (IsActive()) {
		return new emFileManControlPanel(parent,name,GetView());
	}
	else {
		return NULL;
	}
}


void emDirEntryPanel::UpdateBgColor()
{
	emFileManModel * fm;
	emPanel * p;
	bool selSrc,selTgt;
	emColor newBgColor;

	fm=SharedVar->Var.FileMan;
	selSrc=fm->IsSelectedAsSource(DirEntry.GetPath());
	selTgt=fm->IsSelectedAsTarget(DirEntry.GetPath());
	if (selTgt) {
		if (selSrc) newBgColor=ColorBGTgt.GetBlended(ColorBGSrc,50.0F);
		else newBgColor=ColorBGTgt;
	}
	else {
		if (selSrc) newBgColor=ColorBGSrc;
		else newBgColor=ColorBGNormal;
	}
	if (BgColor!=newBgColor) {
		BgColor=newBgColor;
		InvalidatePainting();
		p=GetChild(AltName);
		if (p) {
			p->Layout(
				LayoutAltX,LayoutAltY,
				LayoutAltW,LayoutAltH,
				BgColor
			);
		}
	}
}


void emDirEntryPanel::Select(bool shift, bool ctrl)
{
	emFileManModel * fm;
	emScreen * screen;
	emDirEntryPanel * ep;
	emDirPanel * dp;
	emPanel * p, * c;
	int i,i1,i2;

	fm=SharedVar->Var.FileMan;

	if (!shift && !ctrl) {
		fm->ClearSourceSelection();
		fm->SwapSelection();
	}

	if (shift) {
		if (
			(p=GetParent())==NULL ||
			(dp=dynamic_cast<emDirPanel*>(p))==NULL ||
			!dp->IsContentComplete()
		) {
			screen=GetScreen();
			if (screen) screen->Beep();
			return;
		}

		i1=-1;
		i2=-1;
		for (i=0, c=p->GetFirstChild(); c; i++, c=c->GetNext()) {
			if ((ep=dynamic_cast<emDirEntryPanel*>(c))!=NULL) {
				if (ep==this) i1=i;
				if (ep->DirEntry.GetPath()==fm->GetShiftTgtSelPath()) i2=i;
			}
		}
		if (i1>=0 && i2>=0) {
			if (i1>i2) {
				i=i1;
				i1=i2;
				i2=i;
			}
			for (i=0, c=p->GetFirstChild(); c; i++, c=c->GetNext()) {
				if (i>i1 && i<i2) {
					if ((ep=dynamic_cast<emDirEntryPanel*>(c))!=NULL) {
						if (ctrl && fm->IsSelectedAsTarget(ep->DirEntry.GetPath())) {
							fm->DeselectAsTarget(ep->DirEntry.GetPath());
						}
						else {
							fm->DeselectAsSource(ep->DirEntry.GetPath());
							fm->SelectAsTarget(ep->DirEntry.GetPath());
						}
					}
				}
			}
		}
	}

	if (ctrl && fm->IsSelectedAsTarget(DirEntry.GetPath())) {
		fm->DeselectAsTarget(DirEntry.GetPath());
	}
	else {
		fm->DeselectAsSource(DirEntry.GetPath());
		fm->SelectAsTarget(DirEntry.GetPath());
	}

	fm->SetShiftTgtSelPath(DirEntry.GetPath());
}


void emDirEntryPanel::SelectSolely()
{
	emFileManModel * fm;

	fm=SharedVar->Var.FileMan;

	fm->ClearSourceSelection();
	fm->ClearTargetSelection();
	fm->SelectAsTarget(DirEntry.GetPath());

	fm->SetShiftTgtSelPath(DirEntry.GetPath());
}


void emDirEntryPanel::RunDefaultCommand()
{
	const emFileManModel::CommandNode * cmd;
	emFileManModel * fm;

	fm=SharedVar->Var.FileMan;
	cmd=fm->SearchDefaultCommandFor(DirEntry.GetPath());
	if (cmd) fm->RunCommand(cmd,GetView());
}


void emDirEntryPanel::FormatTime(time_t t, char * buf)
{
	struct tm * p;

	p=localtime(&t); //??? not thread-reentrant (use localtime_r)
	if (!p) {
		strcpy(buf,"0000-00-00 00:00:00");
	}
	else {
		sprintf(
			buf,
			"%04d-%02d-%02d %02d:%02d:%02d",
			(int)p->tm_year+1900,
			(int)p->tm_mon+1,
			(int)p->tm_mday,
			(int)p->tm_hour,
			(int)p->tm_min,
			(int)p->tm_sec
		);
	}
}


const emString emDirEntryPanel::ContentName="";
const emString emDirEntryPanel::AltName="a";
const double emDirEntryPanel::HeightFactor=1/3.0;
const double emDirEntryPanel::LayoutFrame=0.01;
const double emDirEntryPanel::LayoutTitleX=0.012;
const double emDirEntryPanel::LayoutTitleY=0.012;
const double emDirEntryPanel::LayoutTitleW=0.9745;
const double emDirEntryPanel::LayoutTitleH=0.0915;
const double emDirEntryPanel::LayoutInfoX=0.012;
const double emDirEntryPanel::LayoutInfoY=0.1055;
const double emDirEntryPanel::LayoutInfoW=0.3685;
const double emDirEntryPanel::LayoutInfoH=0.214;
const double emDirEntryPanel::LayoutPathX=0.3825;
const double emDirEntryPanel::LayoutPathY=0.1055;
const double emDirEntryPanel::LayoutPathW=0.575;
const double emDirEntryPanel::LayoutPathH=0.01;
const double emDirEntryPanel::MinAltVW=25.0;
const double emDirEntryPanel::LayoutAltX=0.95884;
const double emDirEntryPanel::LayoutAltY=0.1055;
const double emDirEntryPanel::LayoutAltW=0.02766;
const double emDirEntryPanel::LayoutAltH=0.01;
const double emDirEntryPanel::MinContentVW=45.0;
const double emDirEntryPanel::LayoutContentFrame=0.002;
const double emDirEntryPanel::LayoutContentX=0.3845;
const double emDirEntryPanel::LayoutContentY=0.1178333333;
const double emDirEntryPanel::LayoutContentW=0.6;
const double emDirEntryPanel::LayoutContentH=0.2;
const emColor emDirEntryPanel::ColorBGNormal=emColor(187,187,187);
const emColor emDirEntryPanel::ColorBGSrc=emColor(187,221,187);
const emColor emDirEntryPanel::ColorBGTgt=emColor(255,153,153);
const emColor emDirEntryPanel::ColorNameNormal=emColor(0,0,0);
const emColor emDirEntryPanel::ColorNameExe=emColor(0,85,0);
const emColor emDirEntryPanel::ColorNameDir=emColor(0,0,204);
const emColor emDirEntryPanel::ColorNameFifo=emColor(204,0,102);
const emColor emDirEntryPanel::ColorNameBlk=emColor(102,68,0);
const emColor emDirEntryPanel::ColorNameChr=emColor(85,102,0);
const emColor emDirEntryPanel::ColorNameSock=emColor(187,0,187);
const emColor emDirEntryPanel::ColorNameOther=emColor(153,34,17);
const emColor emDirEntryPanel::ColorSymLink=emColor(255,255,255);
const emColor emDirEntryPanel::ColorInfo=emColor(102,102,102);
const emColor emDirEntryPanel::ColorInfoLabel=emColor(136,136,136);
const emColor emDirEntryPanel::ColorPath=emColor(68,68,68);
