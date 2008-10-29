//------------------------------------------------------------------------------
// emFilePanel.cpp
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

#include <emCore/emFilePanel.h>


emFilePanel::emFilePanel(
	ParentArg parent, const emString & name, emFileModel * fileModel,
	bool updateFileModel
)
	: emPanel(parent,name)
{
	CustomError=NULL;
	SetFileModel(fileModel,updateFileModel);
}


emFilePanel::~emFilePanel()
{
	if (CustomError) delete CustomError;
}


void emFilePanel::SetFileModel(emFileModel * fileModel, bool updateFileModel)
{
	emFileModel * fm;

	fm=FileModelClient.GetModel();
	if (fm != fileModel) {
		if (fm) RemoveWakeUpSignal(fm->GetFileStateSignal());
		FileModelClient.SetModel(fileModel);
		if (fileModel) AddWakeUpSignal(fileModel->GetFileStateSignal());
		Signal(VirFileStateSignal);
		InvalidatePainting();
	}
	if (fileModel && updateFileModel) fileModel->Update();
}


void emFilePanel::SetCustomError(const emString & message)
{
	if (CustomError) delete CustomError;
	CustomError=new emString(message);
	Signal(VirFileStateSignal);
	InvalidatePainting();
}


void emFilePanel::ClearCustomError()
{
	if (CustomError) {
		delete CustomError;
		CustomError=NULL;
		Signal(VirFileStateSignal);
		InvalidatePainting();
	}
}


emString emFilePanel::GetCustomError()
{
	if (CustomError) return *CustomError;
	else return emString();
}


emFilePanel::VirtualFileState emFilePanel::GetVirFileState() const
{
	const emFileModel * fm;

	if (CustomError) return VFS_CUSTOM_ERROR;
	fm=FileModelClient.GetModel();
	if (!fm) return VFS_NO_FILE_MODEL;
	if (fm->GetMemoryNeed()>FileModelClient.GetMemoryLimit()) return VFS_TOO_COSTLY;
	return (VirtualFileState)fm->GetFileState();
}


bool emFilePanel::IsVFSGood() const
{
	VirtualFileState s;

	s=GetVirFileState();
	return s==VFS_LOADED || s==VFS_UNSAVED;
}


bool emFilePanel::Cycle()
{
	emFileModel * fm;

	fm=FileModelClient.GetModel();
	if (fm && IsSignaled(fm->GetFileStateSignal())) {
		InvalidatePainting();
		Signal(VirFileStateSignal);
	}
	return false;
}


void emFilePanel::Notice(NoticeFlags flags)
{
	emUInt64 m;
	VirtualFileState vfs;

	if (flags&NF_UPDATE_PRIORITY_CHANGED) {
		FileModelClient.SetPriority(GetUpdatePriority());
	}
	if (flags&NF_MEMORY_LIMIT_CHANGED) {
		m=GetMemoryLimit();
		if (m!=FileModelClient.GetMemoryLimit()) {
			vfs=GetVirFileState();
			FileModelClient.SetMemoryLimit(m);
			if (vfs!=GetVirFileState()) {
				Signal(VirFileStateSignal);
				InvalidatePainting();
			}
		}
	}
}


bool emFilePanel::IsOpaque()
{
	switch (GetVirFileState()) {
	case VFS_LOAD_ERROR:
	case VFS_SAVE_ERROR:
	case VFS_CUSTOM_ERROR:
		return true;
	default:
		return false;
	}
}


void emFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	char tmp[256];
	emColor c;

	switch (GetVirFileState()) {
	case VFS_WAITING:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"Wait...",
			GetHeight()/6,
			emColor(136,136,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_LOADING:
		sprintf(tmp,"Loading: %.1f%%",GetFileModel()->GetFileProgress());
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			tmp,
			GetHeight()/6,
			emColor(0,136,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_LOADED:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"Loaded",
			GetHeight()/6,
			emColor(0,136,136),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_UNSAVED:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"Unsaved",
			GetHeight()/6,
			emColor(136,0,136),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_SAVING:
		sprintf(tmp,"Saving: %.1f%%",GetFileModel()->GetFileProgress()),
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			tmp,
			GetHeight()/6,
			emColor(0,136,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_TOO_COSTLY:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"Costly",
			GetHeight()/6,
			emColor(136,102,102),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_LOAD_ERROR:
		c.Set(128,0,0);
		painter.Clear(c,canvasColor);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.15,
			0.9,
			GetHeight()*0.1,
			"Loading Failed",
			GetHeight()*0.1,
			emColor(204,136,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.3,
			0.9,
			GetHeight()*0.4,
			GetFileModel()->GetErrorText(),
			GetHeight()*0.4,
			emColor(255,255,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_SAVE_ERROR:
		c.Set(128,0,0);
		painter.Clear(c,canvasColor);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.15,
			0.9,
			GetHeight()*0.3,
			"Saving Failed",
			GetHeight()*0.3,
			emColor(255,0,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.5,
			0.9,
			GetHeight()*0.3,
			GetFileModel()->GetErrorText(),
			GetHeight()*0.3,
			emColor(255,255,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_CUSTOM_ERROR:
		c.Set(128,0,0);
		painter.Clear(c,canvasColor);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.15,
			0.9,
			GetHeight()*0.2,
			"Error",
			GetHeight()*0.2,
			emColor(221,0,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.45,
			0.9,
			GetHeight()*0.3,
			*CustomError,
			GetHeight()*0.4,
			emColor(255,255,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_NO_FILE_MODEL:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"No file model",
			GetHeight()/6,
			emColor(136,0,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	}
}


bool emFilePanel::IsHopeForSeeking()
{
	VirtualFileState s;

	s=GetVirFileState();
	return s==VFS_WAITING || s==VFS_LOADING || s==VFS_SAVING;
}
