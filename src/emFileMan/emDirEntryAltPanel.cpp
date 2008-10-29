//------------------------------------------------------------------------------
// emDirEntryAltPanel.cpp
//
// Copyright (C) 2007-2008 Oliver Hamann.
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
	SharedVar=emVarModel<SharedStuff>::Acquire(GetRootContext(),"");
	if (!SharedVar->Var.FileMan) {
		SharedVar->Var.FileMan=emFileManModel::Acquire(GetRootContext());
		SharedVar->Var.InnerBorderImage=emGetInsResImage(
			GetRootContext(),"emFileMan","images/InnerBorder.tga"
		);
	}

	AddWakeUpSignal(SharedVar->Var.FileMan->GetSelectionSignal());

	SetFocusable(false);
}


emDirEntryAltPanel::~emDirEntryAltPanel()
{
}


void emDirEntryAltPanel::UpdateDirEntry(const emDirEntry & dirEntry)
{
	emPanel * p;

	if (DirEntry == dirEntry) return;

	if (dirEntry.GetPath() != DirEntry.GetPath()) {
		emFatalError("emDirEntryAltPanel::UpdateDirEntry: different path");
	}

	p=GetChild(AltName);
	if (p) {
		((emDirEntryAltPanel*)p)->UpdateDirEntry(dirEntry);
	}

	DirEntry=dirEntry;
	InvalidatePainting();
}


bool emDirEntryAltPanel::Cycle()
{
	if (IsSignaled(SharedVar->Var.FileMan->GetSelectionSignal())) {
		InvalidatePainting();
	}
	return false;
}


void emDirEntryAltPanel::Notice(NoticeFlags flags)
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
					DirEntry.GetStat()->st_mode,
					Alternative
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
					Alternative+1
				);
				p->Layout(
					LayoutAltX,LayoutAltY,
					LayoutAltW,LayoutAltH,
					GetCanvasColor()
				);
			}
		}
		else if (p && !p->IsInVisitedPath()) {
			delete p;
		}
	}

	if (flags&NF_LAYOUT_CHANGED) {
		p=GetChild(AltName);
		if (p) {
			p->Layout(
				LayoutAltX,LayoutAltY,
				LayoutAltW,LayoutAltH,
				GetCanvasColor()
			);
		}
	}
}


bool emDirEntryAltPanel::IsOpaque()
{
	return false;
}


void emDirEntryAltPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	char tmp[256];

	sprintf(tmp, "Alternative Content Panel #%d", Alternative);
	painter.PaintTextBoxed(
		LayoutLabelX,
		LayoutLabelY,
		LayoutLabelW,
		LayoutLabelH,
		tmp,
		LayoutLabelH,
		ColorInfoLabel,
		canvasColor,
		EM_ALIGN_LEFT,
		EM_ALIGN_LEFT,
		0.5,
		false
	);
	if (GetViewedWidth()*LayoutContentW >= MinContentVW) {
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
	else {
		painter.PaintRect(
			LayoutContentX,LayoutContentY,
			LayoutContentW,LayoutContentH,
			emColor(0,0,0,20),canvasColor
		);
	}
}


const emString emDirEntryAltPanel::ContentName="";
const emString emDirEntryAltPanel::AltName="a";
const double emDirEntryAltPanel::LayoutLabelX=0.0/0.604;
const double emDirEntryAltPanel::LayoutLabelY=0.0/0.604;
const double emDirEntryAltPanel::LayoutLabelW=0.575/0.604;
const double emDirEntryAltPanel::LayoutLabelH=0.004/0.604;
const double emDirEntryAltPanel::LayoutPathX=0.0/0.604;
const double emDirEntryAltPanel::LayoutPathY=0.004/0.604;
const double emDirEntryAltPanel::LayoutPathW=0.575/0.604;
const double emDirEntryAltPanel::LayoutPathH=0.01/0.604;
const double emDirEntryAltPanel::MinAltVW=25.0;
const double emDirEntryAltPanel::LayoutAltX=0.57634/0.604;
const double emDirEntryAltPanel::LayoutAltY=0.004/0.604;
const double emDirEntryAltPanel::LayoutAltW=0.02766/0.604;
const double emDirEntryAltPanel::LayoutAltH=0.01/0.604;
const double emDirEntryAltPanel::MinContentVW=45.0;
const double emDirEntryAltPanel::LayoutContentFrame=0.002/0.604;
const double emDirEntryAltPanel::LayoutContentX=0.002/0.604;
const double emDirEntryAltPanel::LayoutContentY=0.0163333333/0.604;
const double emDirEntryAltPanel::LayoutContentW=0.6/0.604;
const double emDirEntryAltPanel::LayoutContentH=0.2/0.604;
const emColor emDirEntryAltPanel::ColorBGNormal=emColor(187,187,187);
const emColor emDirEntryAltPanel::ColorInfoLabel=emColor(136,136,136);
const emColor emDirEntryAltPanel::ColorPath=emColor(68,68,68);
