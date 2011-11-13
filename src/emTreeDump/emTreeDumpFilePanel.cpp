//------------------------------------------------------------------------------
// emTreeDumpFilePanel.cpp
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#include <emTreeDump/emTreeDumpFilePanel.h>


emTreeDumpFilePanel::emTreeDumpFilePanel(
	ParentArg parent, const emString & name, emTreeDumpFileModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	FileModel=fileModel;
	RecPanel=NULL;
	if (FileModel) AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(GetVirFileStateSignal());
}


emTreeDumpFilePanel::~emTreeDumpFilePanel()
{
}


void emTreeDumpFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (FileModel) RemoveWakeUpSignal(FileModel->GetChangeSignal());
	FileModel=dynamic_cast<emTreeDumpFileModel*>(fileModel);
	emFilePanel::SetFileModel(FileModel,updateFileModel);
	if (FileModel) AddWakeUpSignal(FileModel->GetChangeSignal());
}


bool emTreeDumpFilePanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();

	if (FileModel && IsSignaled(FileModel->GetChangeSignal())) {
		if (RecPanel) {
			delete RecPanel;
			RecPanel=NULL;
			UpdateRecPanel();
		}
	}

	if (IsSignaled(GetVirFileStateSignal())) {
		UpdateRecPanel();
	}

	return busy;
}


bool emTreeDumpFilePanel::IsOpaque()
{
	if (!IsVFSGood()) {
		return emFilePanel::IsOpaque();
	}
	return false;
}


void emTreeDumpFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
	}
}


void emTreeDumpFilePanel::LayoutChildren()
{
	double x,y,w,h,bh;

	if (RecPanel) {
		x=0.0;
		y=0.0;
		w=1.0;
		h=GetHeight();
		bh=emTreeDumpRecPanel::GetBestHeight();
		if (bh>h) {
			x=(w-h/bh)*0.5;
			w=h/bh;
		}
		else {
			y=(h-bh)*0.5;
			h=bh;
		}
		RecPanel->Layout(x,y,w,h,GetCanvasColor());
	}
}


void emTreeDumpFilePanel::UpdateRecPanel()
{
	if (IsVFSGood()) {
		if (!RecPanel) {
			RecPanel=new emTreeDumpRecPanel(
				this,"rootrec",FileModel,
				emGetParentPath(FileModel->GetFilePath())
			);
		}
	}
	else {
		if (RecPanel) {
			delete RecPanel;
			RecPanel=NULL;
		}
	}
}
