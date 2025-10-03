//------------------------------------------------------------------------------
// emOsmTilePanel.cpp
//
// Copyright (C) 2011-2012,2014-2017,2019,2022,2024 Oliver Hamann.
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

#include <emOsm/emOsmTilePanel.h>


emOsmTilePanel::emOsmTilePanel(
	ParentArg parent, const emString & name, emOsmTileCache * tileCache,
	const emString & tilesUrl, int maxZ, int tileZ, int tileX, int tileY
)
	: emPanel(parent,name),
	TileCache(tileCache),
	TilesUrl(tilesUrl),
	MaxZ(maxZ),
	TileZ(tileZ),
	TileX(tileX),
	TileY(tileY),
	BusyCleanerToAllow(false),
	State(S_NOT_VIEWED)
{
	SetAutoExpansionThreshold(300.0,VCT_WIDTH);

	if (dynamic_cast<emOsmTilePanel*>(GetParent())==NULL) {
		BusyCleanerToAllow=true;
	}

	WakeUp();
}


emOsmTilePanel::~emOsmTilePanel()
{
	ClearAll();
}


emString emOsmTilePanel::GetTitle() const
{
	return "emOsm";
}


emString emOsmTilePanel::GetIconFileName() const
{
	return "earth.tga";
}


bool emOsmTilePanel::Cycle()
{
	bool busy;

	busy=emPanel::Cycle();

	if (
		BusyCleanerToAllow &&
		IsInViewedPath() && (
			!IsViewed() ||
			(GetClipX2()-GetClipX1())*(GetClipY2()-GetClipY1()) >
				0.5*GetView().GetCurrentWidth()*GetView().GetCurrentHeight()
		)
	) {
		TileCache->AllowBusyCleaner();
		BusyCleanerToAllow=false;
	}

	UpdateState();

	return busy;
}


void emOsmTilePanel::Notice(NoticeFlags flags)
{
	if (flags&NF_VIEWING_CHANGED) {
		WakeUp();
	}
	if ((flags&NF_UPDATE_PRIORITY_CHANGED)!=0) {
		if (LoadJob) {
			LoadJob->SetPriority(GetUpdatePriority());
		}
	}
}


bool emOsmTilePanel::IsOpaque() const
{
	return State==S_LOADED;
}


void emOsmTilePanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	emOsmTilePanel * p;
	double h;
	int i;

	for (i=3; i>=0; i--) {
		p=Children[i];
		if (!p) break;
		if (p->IsInViewedPath() && p->State!=S_LOADED) break;
	}
	if (i<0) return;

	h=GetHeight();
	if (State==S_LOADED) {
		painter.PaintImage(0.0,0.0,1.0,h,Image,255,canvasColor);
	}
	else if (State==S_ERROR) {
		painter.PaintTextBoxed(
			0.0,0.0,1.0,h,
			ErrorText,
			h/5,
			emColor(255,0,0,255),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER,
			1.0
		);
	}
	else {
		painter.PaintTextBoxed(
			0.0,0.0,1.0,h,
			"Wait...",
			h/5,
			emColor(136,136,0,32),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER,
			1.0
		);
	}
}


void emOsmTilePanel::AutoExpand()
{
	emOsmTilePanel * p;
	int i;

	if (TileZ<MaxZ && State==S_LOADED) {
		for (i=0; i<4; i++) {
			p=new emOsmTilePanel(
				this,
				emString::Format("%d",i),
				TileCache,
				TilesUrl,
				MaxZ,
				TileZ+1,
				TileX*2+((i&1)?1:0),
				TileY*2+((i&2)?1:0)
			);
			p->SetFocusable(false);
			Children[i]=p;
		}
	}
}


void emOsmTilePanel::LayoutChildren()
{
	emOsmTilePanel * p;
	double h;
	int i;

	h=GetHeight();
	for (i=0; i<4; i++) {
		p=Children[i];
		if (p) {
			p->Layout(
				(i&1)?0.5:0.0,
				(i&2)?h*0.5:0.0,
				0.5,
				h*0.5,
				0
			);
		}
	}
}


void emOsmTilePanel::UpdateState()
{
	if (!IsInViewedPath()) {
		if (State!= S_NOT_VIEWED) {
			ClearAll();
			State=S_NOT_VIEWED;
			InvalidatePainting();
			InvalidateAutoExpansion();
		}
		return;
	}

	switch (State) {
	case S_NOT_VIEWED:
		LoadJob=new emOsmTileCache::LoadJob(
			TilesUrl,TileZ,TileX,TileY,GetUpdatePriority()
		);
		TileCache->EnqueueJob(*LoadJob);
		AddWakeUpSignal(LoadJob->GetStateSignal());
		State=S_LOADING;
		// no break
	case S_LOADING:
		switch (LoadJob->GetState()) {
		case emJob::ST_WAITING:
		case emJob::ST_RUNNING:
			break;
		case emJob::ST_SUCCESS:
			Image=LoadJob->GetImage();
			LoadJob=NULL;
			State=S_LOADED;
			InvalidatePainting();
			InvalidateAutoExpansion();
			break;
		case emJob::ST_ERROR:
			SetError(LoadJob->GetErrorText());
			break;
		default:
			SetError("Aborted");
			break;
		}
		break;
	case S_LOADED:
	case S_ERROR:
		break;
	}
}


void emOsmTilePanel::ClearAll()
{
	ErrorText.Clear();
	Image.Clear();
	if (LoadJob) {
		TileCache->AbortJob(*LoadJob);
		LoadJob=NULL;
	}
}


void emOsmTilePanel::SetError(const emString & errorText)
{
	emString et;

	et=errorText;
	if (et.IsEmpty()) et="unknown error";
	ClearAll();
	ErrorText=et;
	State=S_ERROR;
	InvalidatePainting();
	InvalidateAutoExpansion();
}
