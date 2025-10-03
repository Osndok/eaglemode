//------------------------------------------------------------------------------
// emOsmFilePanel.cpp
//
// Copyright (C) 2012,2016,2024 Oliver Hamann.
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

#include <emOsm/emOsmFilePanel.h>
#include <emOsm/emOsmControlPanel.h>


emOsmFilePanel::emOsmFilePanel(
	ParentArg parent, const emString & name, emOsmFileModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	FileModel=fileModel;
	TilePanel=NULL;
	if (FileModel) AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
}


emOsmFilePanel::~emOsmFilePanel()
{
}


void emOsmFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (FileModel) RemoveWakeUpSignal(FileModel->GetChangeSignal());
	FileModel=dynamic_cast<emOsmFileModel*>(fileModel);
	emFilePanel::SetFileModel(FileModel,updateFileModel);
	if (FileModel) AddWakeUpSignal(FileModel->GetChangeSignal());
	InvalidateControlPanel();
}


bool emOsmFilePanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();

	if (FileModel && IsSignaled(FileModel->GetChangeSignal())) {
		if (TilePanel) {
			delete TilePanel;
			TilePanel=NULL;
		}
		UpdateTilePanel();
	}

	if (IsSignaled(GetVirFileStateSignal())) {
		UpdateTilePanel();
	}

	return busy;
}


bool emOsmFilePanel::IsOpaque() const
{
	if (!IsVFSGood()) {
		return emFilePanel::IsOpaque();
	}
	return false;
}


void emOsmFilePanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
	}
}


void emOsmFilePanel::LayoutChildren()
{
	double x,y,w,h;

	if (TilePanel) {
		x=0.0;
		y=0.0;
		w=1.0;
		h=GetHeight();
		if (w>h) {
			x=(w-h)*0.5;
			w=h;
		}
		else {
			y=(h-w)*0.5;
			h=w;
		}
		TilePanel->Layout(x,y,w,h,GetCanvasColor());
	}
}


emPanel * emOsmFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (!FileModel) return NULL;

	return new emOsmControlPanel(parent,name,*FileModel);
}


void emOsmFilePanel::UpdateTilePanel()
{
	if (IsVFSGood()) {
		if (!TilePanel) {
			TilePanel=new emOsmTilePanel(
				this,
				"osm",
				emOsmTileCache::Acquire(GetRootContext()),
				FileModel->TilesUrl,
				FileModel->MaxZ
			);
			TilePanel->SetFocusable(false);
		}
	}
	else {
		if (TilePanel) {
			delete TilePanel;
			TilePanel=NULL;
		}
	}
}
