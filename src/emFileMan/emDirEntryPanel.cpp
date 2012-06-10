//------------------------------------------------------------------------------
// emDirEntryPanel.cpp
//
// Copyright (C) 2004-2011 Oliver Hamann.
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

#if defined(_WIN32)
#	include <windows.h>
#else
#	include <unistd.h>
#endif
#include <emCore/emFpPlugin.h>
#include <emFileMan/emDirPanel.h>
#include <emFileMan/emDirEntryPanel.h>
#include <emFileMan/emDirEntryAltPanel.h>
#include <emFileMan/emFileManControlPanel.h>
#include <emCore/emRes.h>
#if defined(_WIN32)
#	ifndef S_IXUSR
#		define S_IXUSR 0100
#	endif
#	ifndef S_IXGRP
#		define S_IXGRP 0010
#	endif
#	ifndef S_IXOTH
#		define S_IXOTH 0001
#	endif
#endif


emDirEntryPanel::emDirEntryPanel(
	ParentArg parent, const emString & name, const emDirEntry & dirEntry
)
	: emPanel(parent,name), DirEntry(dirEntry)
{
	FileMan=emFileManModel::Acquire(GetRootContext());
	Config=emFileManViewConfig::Acquire(GetView());
	BgColor=0;

	AddWakeUpSignal(FileMan->GetSelectionSignal());
	AddWakeUpSignal(Config->GetChangeSignal());

	UpdateBgColor();
}


emDirEntryPanel::~emDirEntryPanel()
{
}


void emDirEntryPanel::UpdateDirEntry(const emDirEntry & dirEntry)
{
	bool pathChanged, errOrFmtChanged;
	emPanel * p;

	if (DirEntry == dirEntry) return;

	pathChanged = (dirEntry.GetPath() != DirEntry.GetPath());

	errOrFmtChanged = (
		dirEntry.GetStatErrNo() != DirEntry.GetStatErrNo() ||
		(dirEntry.GetStat()->st_mode&S_IFMT) != (DirEntry.GetStat()->st_mode&S_IFMT)
	);

	DirEntry=dirEntry;

	InvalidatePainting();

	if (pathChanged || errOrFmtChanged) UpdateContentPanel(true);

	if (pathChanged) UpdateBgColor();

	p=GetChild(AltName);
	if (p) {
		((emDirEntryAltPanel*)p)->UpdateDirEntry(dirEntry);
	}
}


emString emDirEntryPanel::GetTitle()
{
	return DirEntry.GetPath();
}


bool emDirEntryPanel::Cycle()
{
	if (IsSignaled(FileMan->GetSelectionSignal())) {
		UpdateBgColor();
	}
	if (IsSignaled(Config->GetChangeSignal())) {
		InvalidatePainting();
		UpdateContentPanel(false,true);
		UpdateAltPanel(false,true);
		UpdateBgColor();
	}
	return false;
}


void emDirEntryPanel::Notice(NoticeFlags flags)
{
	if ((flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED|NF_VISIT_CHANGED))!=0) {
		UpdateContentPanel();
		UpdateAltPanel();
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
		FileMan->HotkeyInput(GetView(),event,state);
	}
}


bool emDirEntryPanel::IsOpaque()
{
	const emFileManTheme * theme;

	theme = &Config->GetTheme();

	return
		BgColor.IsOpaque() &&
		theme->BackgroundX <= 0.0 &&
		theme->BackgroundY <= 0.0 &&
		theme->BackgroundW >= 1.0 &&
		theme->BackgroundH >= GetHeight() &&
		theme->BackgroundRX <= 0.0 &&
		theme->BackgroundRY <= 0.0
	;
}


void emDirEntryPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	const emFileManTheme * theme;
	emColor color;
	double t;

	theme = &Config->GetTheme();

	painter.PaintRoundRect(
		theme->BackgroundX,
		theme->BackgroundY,
		theme->BackgroundW,
		theme->BackgroundH,
		theme->BackgroundRX,
		theme->BackgroundRY,
		BgColor,
		canvasColor
	);

	if (
		BgColor==canvasColor || (
			theme->OuterBorderX >= theme->BackgroundX + theme->BackgroundRX * 0.3 &&
			theme->OuterBorderY >= theme->BackgroundY + theme->BackgroundRY * 0.3 &&
			theme->OuterBorderW <= theme->BackgroundX + theme->BackgroundW -
				theme->BackgroundRX * 0.3 - theme->OuterBorderX &&
			theme->OuterBorderH <= theme->BackgroundY + theme->BackgroundH -
				theme->BackgroundRY * 0.3 - theme->OuterBorderY
		)
	) {
		canvasColor=BgColor;
	}
	else {
		canvasColor=0;
	}
	painter.PaintBorderImage(
		theme->OuterBorderX,
		theme->OuterBorderY,
		theme->OuterBorderW,
		theme->OuterBorderH,
		theme->OuterBorderL,
		theme->OuterBorderT,
		theme->OuterBorderR,
		theme->OuterBorderB,
		theme->OuterBorderImg.GetImage(),
		theme->OuterBorderImgL,
		theme->OuterBorderImgT,
		theme->OuterBorderImgR,
		theme->OuterBorderImgB,
		255,
		canvasColor,
		0757
	);

	canvasColor=BgColor;

	if (DirEntry.IsRegularFile()) {
		if (DirEntry.GetStat()->st_mode&(S_IXUSR|S_IXGRP|S_IXOTH))
			color=theme->ExeNameColor;
		else
			color=theme->NormalNameColor;
	}
	else if (DirEntry.IsDirectory()) color=theme->DirNameColor;
#if !defined(_WIN32)
	else if (S_ISFIFO(DirEntry.GetStat()->st_mode)) color=theme->FifoNameColor;
	else if (S_ISBLK (DirEntry.GetStat()->st_mode)) color=theme->BlkNameColor;
	else if (S_ISCHR (DirEntry.GetStat()->st_mode)) color=theme->ChrNameColor;
	else if (S_ISSOCK(DirEntry.GetStat()->st_mode)) color=theme->SockNameColor;
#endif
	else color=theme->OtherNameColor;
	if (DirEntry.IsHidden()) color=color.GetTransparented(27.0F);
	painter.PaintTextBoxed(
		theme->NameX,
		theme->NameY,
		theme->NameW,
		theme->NameH,
		DirEntry.GetName(),
		theme->NameH,
		color,
		canvasColor,
		theme->NameAlignment,
		EM_ALIGN_LEFT,
		0.5,
		false
	);

	PaintInfo(
		painter,
		theme->InfoX,
		theme->InfoY,
		theme->InfoW,
		theme->InfoH,
		theme->InfoAlignment,
		canvasColor
	);

	t = DirEntry.IsDirectory() ? theme->DirContentW : theme->FileContentW;
	if (GetChild(ContentName) || GetViewedWidth()*t >= theme->MinContentVW) {
		painter.PaintTextBoxed(
			theme->PathX,
			theme->PathY,
			theme->PathW,
			theme->PathH,
			DirEntry.GetPath(),
			theme->PathH,
			theme->PathColor,
			canvasColor,
			theme->PathAlignment,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
		if (DirEntry.IsDirectory()) {
			painter.PaintBorderImage(
				theme->DirInnerBorderX,
				theme->DirInnerBorderY,
				theme->DirInnerBorderW,
				theme->DirInnerBorderH,
				theme->DirInnerBorderL,
				theme->DirInnerBorderT,
				theme->DirInnerBorderR,
				theme->DirInnerBorderB,
				theme->DirInnerBorderImg.GetImage(),
				theme->DirInnerBorderImgL,
				theme->DirInnerBorderImgT,
				theme->DirInnerBorderImgR,
				theme->DirInnerBorderImgB,
				255,canvasColor,0757
			);
			painter.PaintRect(
				theme->DirContentX,
				theme->DirContentY,
				theme->DirContentW,
				theme->DirContentH,
				theme->DirContentColor,
				canvasColor
			);
		}
		else {
			painter.PaintBorderImage(
				theme->FileInnerBorderX,
				theme->FileInnerBorderY,
				theme->FileInnerBorderW,
				theme->FileInnerBorderH,
				theme->FileInnerBorderL,
				theme->FileInnerBorderT,
				theme->FileInnerBorderR,
				theme->FileInnerBorderB,
				theme->FileInnerBorderImg.GetImage(),
				theme->FileInnerBorderImgL,
				theme->FileInnerBorderImgT,
				theme->FileInnerBorderImgR,
				theme->FileInnerBorderImgB,
				255,canvasColor,0757
			);
			painter.PaintRect(
				theme->FileContentX,
				theme->FileContentY,
				theme->FileContentW,
				theme->FileContentH,
				theme->FileContentColor,
				canvasColor
			);
		}
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


void emDirEntryPanel::PaintInfo(
	const emPainter & painter, double infoX, double infoY,
	double infoW, double infoH, emAlignment alignment, emColor canvasColor
)
{
	const char * label[6] = {
		"Type",
#		if defined(_WIN32)
			"File Attributes",
#		else
			"Permissions of Owner, Group and Others",
#		endif
		"Owner",
		"Group",
		"Size in Bytes",
		"Time of Last Modification"
	};
	double bx[6],by[6],bw[6],bh[6];
	double spx,spy,tw,th,ws,cw,lh,x,t;
	const emFileManTheme * theme;
	emString str;
	char tmp[1024];
	const char * p;
	int i,j,k,len;

	theme = &Config->GetTheme();

	t=infoH/infoW;
	if (t>0.9) {
		th=infoW*1.4;
		if (infoH>th) {
			if (alignment&EM_ALIGN_BOTTOM) infoY+=infoH-th;
			else if (!(alignment&EM_ALIGN_TOP)) infoY+=(infoH-th)*0.5;
			infoH=th;
		}
		th=infoH/(7+(7-2)*0.087);
		if (th*GetViewedWidth()<=1.15) return;
		spy=(infoH-7*th)/(7-2);
		for (i=0; i<6; i++) {
			bx[i]=infoX;
			by[i]=infoY+i*(th+spy);
			bw[i]=infoW;
			bh[i]=th;
		}
		bh[5]*=2;
		lh=th/7.6666;
	}
	else if (t>0.04) {
		infoH*=1.03; // Because time stamp has no descenders
		th=infoH/(4+(4-1)*0.087);
		if (th*GetViewedWidth()<=1.15) return;
		spy=(infoH-4*th)/(4-1);
		spx=th*0.483;
		tw=(infoW-spx)/2;
		bx[0]=infoX;        by[0]=infoY;            bw[0]=infoW; bh[0]=th;
		bx[1]=infoX;        by[1]=infoY+th+spy;     bw[1]=tw;    bh[1]=th;
		bx[2]=infoX;        by[2]=infoY+2*(th+spy); bw[2]=tw;    bh[2]=th;
		bx[3]=infoX+tw+spx; by[3]=infoY+2*(th+spy); bw[3]=tw;    bh[3]=th;
		bx[4]=infoX+tw+spx; by[4]=infoY+th+spy;     bw[4]=tw;    bh[4]=th;
		bx[5]=infoX;        by[5]=infoY+3*(th+spy); bw[5]=infoW; bh[5]=th;
		lh=th/7.6666;
	}
	else {
		if (infoH*GetViewedWidth()<=1.15) return;
		tw=infoH/0.025;
		if (infoW>tw) {
			if (alignment&EM_ALIGN_RIGHT) infoX+=infoW-tw;
			else if (!(alignment&EM_ALIGN_LEFT)) infoX+=(infoW-tw)*0.5;
			infoW=tw;
		}
		tw=infoW/(6+(6-1)*0.087);
		spx=(infoW-6*tw)/(6-1);
		for (i=0; i<6; i++) {
			bx[i]=infoX+i*(tw+spx);
			by[i]=infoY;
			bw[i]=tw;
			bh[i]=infoH;
		}
		lh=infoH/7.6666;
	}

	if (lh*GetViewedWidth()>1.0) {
		for (i=0; i<6; i++) {
			painter.PaintTextBoxed(
				bx[i],by[i],bw[i],bh[i],
				label[i],lh,
				theme->LabelColor,canvasColor,
				EM_ALIGN_TOP_LEFT,EM_ALIGN_LEFT
			);
		}
	}

	for (i=0; i<6; i++) { by[i]+=lh; bh[i]-=lh; }

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
			bx[0],by[0],bw[0],bh[0]/2,
			tmp,bh[0]/2,
			theme->SymLinkColor,canvasColor,
			EM_ALIGN_LEFT,EM_ALIGN_LEFT,
			0.5,false
		);
		if (DirEntry.GetTargetPathErrNo()) {
			str=emGetErrorText(DirEntry.GetTargetPathErrNo());
		}
		else {
			str=DirEntry.GetTargetPath();
		}
		painter.PaintTextBoxed(
			bx[0],by[0]+bh[0]/2,bw[0],bh[0]/2,
			str,bh[0]/2,
			theme->SymLinkColor,canvasColor,
			EM_ALIGN_LEFT,EM_ALIGN_LEFT,
			0.5,false
		);
	}
	else {
		painter.PaintTextBoxed(
			bx[0],by[0],bw[0],bh[0],
			p,bh[0],
			theme->InfoColor,canvasColor,
			EM_ALIGN_LEFT,EM_ALIGN_LEFT,
			0.5,false
		);
	}

#if defined(_WIN32)
	tmp[0]=(DirEntry.GetWndsFileAttributes()&FILE_ATTRIBUTE_READONLY)?'R':'-';
	tmp[1]=(DirEntry.GetWndsFileAttributes()&FILE_ATTRIBUTE_HIDDEN  )?'H':'-';
	tmp[2]=(DirEntry.GetWndsFileAttributes()&FILE_ATTRIBUTE_SYSTEM  )?'S':'-';
	tmp[3]=(DirEntry.GetWndsFileAttributes()&FILE_ATTRIBUTE_ARCHIVE )?'A':'-';
	tmp[4]=0;
	painter.PaintTextBoxed(
		bx[1],by[1],bw[1],bh[1],
		tmp,bh[1],
		theme->InfoColor,canvasColor,
		EM_ALIGN_LEFT,EM_ALIGN_LEFT,
		0.5,false
	);
#else
	cw=painter.GetTextSize("X",bh[1],false);
	ws=bw[1]/(cw*10);
	if (ws>1.0) ws=1.0;
	tmp[0]=((DirEntry.GetStat()->st_mode&S_IRUSR)?'r':'-');
	tmp[1]=((DirEntry.GetStat()->st_mode&S_IWUSR)?'w':'-');
	tmp[2]=((DirEntry.GetStat()->st_mode&S_IXUSR)?'x':'-');
	tmp[3]=0;
	painter.PaintText(bx[1],by[1],tmp,bh[1],ws,theme->InfoColor,canvasColor);
	tmp[0]=((DirEntry.GetStat()->st_mode&S_IRGRP)?'r':'-');
	tmp[1]=((DirEntry.GetStat()->st_mode&S_IWGRP)?'w':'-');
	tmp[2]=((DirEntry.GetStat()->st_mode&S_IXGRP)?'x':'-');
	tmp[3]=0;
	painter.PaintText(bx[1]+cw*3.5*ws,by[1],tmp,bh[1],ws,theme->InfoColor,canvasColor);
	tmp[0]=((DirEntry.GetStat()->st_mode&S_IROTH)?'r':'-');
	tmp[1]=((DirEntry.GetStat()->st_mode&S_IWOTH)?'w':'-');
	tmp[2]=((DirEntry.GetStat()->st_mode&S_IXOTH)?'x':'-');
	tmp[3]=0;
	painter.PaintText(bx[1]+cw*7*ws,by[1],tmp,bh[1],ws,theme->InfoColor,canvasColor);
#endif

	painter.PaintTextBoxed(
		bx[2],by[2],bw[2],bh[2],
		DirEntry.GetOwner(),bh[2],
		theme->InfoColor,canvasColor,
		EM_ALIGN_LEFT,EM_ALIGN_LEFT,
		0.5,false
	);

	painter.PaintTextBoxed(
		bx[3],by[3],bw[3],bh[3],
		DirEntry.GetGroup(),bh[3],
		theme->InfoColor,canvasColor,
		EM_ALIGN_LEFT,EM_ALIGN_LEFT,
		0.5,false
	);

	len=emUInt64ToStr(tmp,sizeof(tmp),DirEntry.GetStat()->st_size);
	cw=painter.GetTextSize("X",bh[4],false);
	ws=bw[4]/(cw*len*16/15);
	if (ws>1.0) ws=1.0;
	x=bx[4];
	for (i=0; i<len; i+=j) {
		j=(len-i)-(len-i-1)/3*3;
		painter.PaintText(x,by[4],tmp+i,bh[4],ws,theme->InfoColor,canvasColor,j);
		x+=cw*j*ws;
		k=(len-i-j)/3-1;
		if (k>=0) {
			painter.PaintText(
				x,by[4]+bh[4]*0.75,"kMGTPEZY"+k,bh[4]/5,ws,
				theme->InfoColor,canvasColor,1
			);
		}
		x+=cw/5*ws;
	}

	FormatTime(DirEntry.GetStat()->st_mtime,tmp,bw[5]/bh[5]<6.0);
	painter.PaintTextBoxed(
		bx[5],by[5],bw[5],bh[5],
		tmp,bh[5],
		theme->InfoColor,canvasColor,
		EM_ALIGN_LEFT,EM_ALIGN_LEFT,
		0.5,true
	);
}


void emDirEntryPanel::UpdateContentPanel(bool forceRecreation, bool forceRelayout)
{
	const char * soughtName;
	emRef<emFpPluginList> fppl;
	emPanel * p;
	const emFileManTheme * theme;
	double cx,cy,cw,ch;
	emColor cc;

	theme = &Config->GetTheme();
	p=GetChild(ContentName);
	if (forceRecreation && p) { delete p; p=NULL; }

	if (DirEntry.IsDirectory()) {
		cx=theme->DirContentX;
		cy=theme->DirContentY;
		cw=theme->DirContentW;
		ch=theme->DirContentH;
		cc=theme->DirContentColor;
	}
	else {
		cx=theme->FileContentX;
		cy=theme->FileContentY;
		cw=theme->FileContentW;
		ch=theme->FileContentH;
		cc=theme->FileContentColor;
	}

	soughtName=GetSoughtName();

	if (
		(
			soughtName &&
			strcmp(soughtName,ContentName)==0
		) ||
		(
			IsViewed() &&
			GetViewedWidth()*cw>=theme->MinContentVW &&
			PanelToViewX(cx)<GetClipX2() &&
			PanelToViewX(cx+cw)>GetClipX1() &&
			PanelToViewY(cy)<GetClipY2() &&
			PanelToViewY(cy+ch)>GetClipY1()
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
			forceRelayout=true;
		}
	}
	else if (p && !p->IsInVisitedPath()) {
		delete p;
		p=NULL;
	}

	if (p && forceRelayout) p->Layout(cx,cy,cw,ch,cc);
}


void emDirEntryPanel::UpdateAltPanel(bool forceRecreation, bool forceRelayout)
{
	const char * soughtName;
	emPanel * p;
	const emFileManTheme * theme;

	theme = &Config->GetTheme();
	p=GetChild(AltName);
	if (forceRecreation && p) { delete p; p=NULL; }
	soughtName=GetSoughtName();
	if (
		(
			soughtName &&
			strcmp(soughtName,AltName)==0
		) ||
		(
			IsViewed() &&
			GetViewedWidth()*theme->AltW>=theme->MinAltVW &&
			PanelToViewX(theme->AltX)<GetClipX2() &&
			PanelToViewX(theme->AltX+theme->AltW)>GetClipX1() &&
			PanelToViewY(theme->AltY)<GetClipY2() &&
			PanelToViewY(theme->AltY+theme->AltH)>GetClipY1()
		)
	) {
		if (!p) {
			p=new emDirEntryAltPanel(
				this,
				AltName,
				DirEntry,
				1
			);
			forceRelayout=true;
		}
	}
	else if (p && !p->IsInVisitedPath()) {
		delete p;
		p=NULL;
	}

	if (p && forceRelayout) {
		p->Layout(
			theme->AltX,theme->AltY,
			theme->AltW,theme->AltH,
			BgColor
		);
	}
}


void emDirEntryPanel::UpdateBgColor()
{
	const emFileManTheme * theme;
	emFileManModel * fm;
	bool selSrc,selTgt;
	emColor newBgColor;

	theme = &Config->GetTheme();
	fm=FileMan;
	selSrc=fm->IsSelectedAsSource(DirEntry.GetPath());
	selTgt=fm->IsSelectedAsTarget(DirEntry.GetPath());
	if (selTgt) {
		newBgColor=theme->TargetSelectionColor;
		if (selSrc) {
			newBgColor=newBgColor.GetBlended(
				theme->SourceSelectionColor,
				50.0F
			);
		}
	}
	else {
		if (selSrc) newBgColor=theme->SourceSelectionColor;
		else newBgColor=theme->BackgroundColor;
	}
	if (BgColor!=newBgColor) {
		BgColor=newBgColor;
		InvalidatePainting();
		UpdateAltPanel(false,true);
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

	fm=FileMan;

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

	fm=FileMan;

	fm->ClearSourceSelection();
	fm->ClearTargetSelection();
	fm->SelectAsTarget(DirEntry.GetPath());

	fm->SetShiftTgtSelPath(DirEntry.GetPath());
}


void emDirEntryPanel::RunDefaultCommand()
{
	const emFileManModel::CommandNode * cmd;
	emFileManModel * fm;

	fm=FileMan;
	cmd=fm->SearchDefaultCommandFor(DirEntry.GetPath());
	if (cmd) fm->RunCommand(cmd,GetView());
}


void emDirEntryPanel::FormatTime(time_t t, char * buf, bool nl)
{
	struct tm tmbuf;
	struct tm * p;

	p=localtime_r(&t,&tmbuf);
	if (!p) {
		sprintf(buf,"0000-00-00%c00:00:00",nl?'\n':' ');
	}
	else {
		sprintf(
			buf,
			"%04d-%02d-%02d%c%02d:%02d:%02d",
			(int)p->tm_year+1900,
			(int)p->tm_mon+1,
			(int)p->tm_mday,
			nl ? '\n' : ' ',
			(int)p->tm_hour,
			(int)p->tm_min,
			(int)p->tm_sec
		);
	}
}


const char * const emDirEntryPanel::ContentName="";
const char * const emDirEntryPanel::AltName="a";
