//------------------------------------------------------------------------------
// emDirEntryAltPanel.cpp
//
// Copyright (C) 2007-2010 Oliver Hamann.
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
#include <emCore/emFpPlugin.h>
#include <emFileMan/emDirEntryAltPanel.h>
#include <emFileMan/emFileManControlPanel.h>


emDirEntryAltPanel::emDirEntryAltPanel(
	ParentArg parent, const emString & name,
	const emDirEntry & dirEntry, int alternative
)
	: emPanel(parent,name), DirEntry(dirEntry), Alternative(alternative)
{
	FileMan=emFileManModel::Acquire(GetRootContext());
	Config=emFileManViewConfig::Acquire(GetView());

	AddWakeUpSignal(FileMan->GetSelectionSignal());
	AddWakeUpSignal(Config->GetChangeSignal());

	SetFocusable(false);
}


emDirEntryAltPanel::~emDirEntryAltPanel()
{
}


void emDirEntryAltPanel::UpdateDirEntry(const emDirEntry & dirEntry)
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

	p=GetChild(AltName);
	if (p) {
		((emDirEntryAltPanel*)p)->UpdateDirEntry(dirEntry);
	}
}


bool emDirEntryAltPanel::Cycle()
{
	if (IsSignaled(FileMan->GetSelectionSignal())) {
		InvalidatePainting();
	}
	if (IsSignaled(Config->GetChangeSignal())) {
		InvalidatePainting();
		UpdateContentPanel(false,true);
		UpdateAltPanel(false,true);
	}
	return false;
}


void emDirEntryAltPanel::Notice(NoticeFlags flags)
{
	if ((flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED|NF_VISIT_CHANGED))!=0) {
		UpdateContentPanel();
		UpdateAltPanel();
	}
	if (flags&NF_LAYOUT_CHANGED) {
		UpdateAltPanel(false,true);
	}
}


bool emDirEntryAltPanel::IsOpaque()
{
	return false;
}


void emDirEntryAltPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	char tmp[256];
	const emFileManTheme * theme;

	theme = &Config->GetTheme();

	sprintf(tmp, "Alternative Content Panel #%d", Alternative);
	painter.PaintTextBoxed(
		theme->AltLabelX,
		theme->AltLabelY,
		theme->AltLabelW,
		theme->AltLabelH,
		tmp,
		theme->AltLabelH,
		theme->LabelColor,
		canvasColor,
		theme->AltLabelAlignment,
		EM_ALIGN_LEFT,
		0.5,
		false
	);
	if (GetViewedWidth()*theme->AltContentW >= theme->MinContentVW) {
		painter.PaintTextBoxed(
			theme->AltPathX,
			theme->AltPathY,
			theme->AltPathW,
			theme->AltPathH,
			DirEntry.GetPath(),
			theme->AltPathH,
			theme->PathColor,
			canvasColor,
			theme->AltPathAlignment,
			EM_ALIGN_LEFT,
			0.5,
			false
		);
		painter.PaintBorderImage(
			theme->AltInnerBorderX,
			theme->AltInnerBorderY,
			theme->AltInnerBorderW,
			theme->AltInnerBorderH,
			theme->AltInnerBorderL,
			theme->AltInnerBorderT,
			theme->AltInnerBorderR,
			theme->AltInnerBorderB,
			theme->AltInnerBorderImg.GetImage(),
			theme->AltInnerBorderImgL,
			theme->AltInnerBorderImgT,
			theme->AltInnerBorderImgR,
			theme->AltInnerBorderImgB,
			255,canvasColor,0757
		);
		painter.PaintRect(
			theme->AltContentX,theme->AltContentY,
			theme->AltContentW,theme->AltContentH,
			theme->BackgroundColor,
			canvasColor
		);
	}
	else {
		painter.PaintRect(
			theme->AltContentX,theme->AltContentY,
			theme->AltContentW,theme->AltContentH,
			theme->LabelColor.Get().GetTransparented(68.0f),
			canvasColor
		);
	}
}


void emDirEntryAltPanel::UpdateContentPanel(bool forceRecreation, bool forceRelayout)
{
	const char * soughtName;
	emRef<emFpPluginList> fppl;
	emPanel * p;
	const emFileManTheme * theme;

	theme = &Config->GetTheme();
	p=GetChild(ContentName);
	if (forceRecreation && p) { delete p; p=NULL; }
	soughtName=GetSoughtName();
	if (
		(
			soughtName &&
			strcmp(soughtName,ContentName)==0
		) ||
		(
			IsViewed() &&
			GetViewedWidth()*theme->AltContentW>=theme->MinContentVW &&
			PanelToViewX(theme->AltContentX)<GetClipX2() &&
			PanelToViewX(theme->AltContentX+theme->AltContentW)>GetClipX1() &&
			PanelToViewY(theme->AltContentY)<GetClipY2() &&
			PanelToViewY(theme->AltContentY+theme->AltContentH)>GetClipY1()
		)
	) {
		if (!p) {
			fppl=emFpPluginList::Acquire(GetRootContext());
			p=fppl->CreateFilePanel(
				this,
				ContentName,
				DirEntry.GetPath(),
				DirEntry.GetStatErrNo(),
				DirEntry.GetStat()->st_mode,
				Alternative
			);
			p->BeFirst();
			forceRelayout=true;
		}
	}
	else if (p && !p->IsInVisitedPath()) {
		delete p;
		p=NULL;
	}

	if (p && forceRelayout) {
		p->Layout(
			theme->AltContentX,theme->AltContentY,
			theme->AltContentW,theme->AltContentH,
			theme->BackgroundColor
		);
	}
}


void emDirEntryAltPanel::UpdateAltPanel(bool forceRecreation, bool forceRelayout)
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
			GetViewedWidth()*theme->AltAltW>=theme->MinAltVW &&
			PanelToViewX(theme->AltAltX)<GetClipX2() &&
			PanelToViewX(theme->AltAltX+theme->AltAltW)>GetClipX1() &&
			PanelToViewY(theme->AltAltY)<GetClipY2() &&
			PanelToViewY(theme->AltAltY+theme->AltAltH)>GetClipY1()
		)
	) {
		if (!p) {
			p=new emDirEntryAltPanel(
				this,
				AltName,
				DirEntry,
				Alternative+1
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
			theme->AltAltX,theme->AltAltY,
			theme->AltAltW,theme->AltAltH,
			GetCanvasColor()
		);
	}
}


const char * const emDirEntryAltPanel::ContentName="";
const char * const emDirEntryAltPanel::AltName="a";
