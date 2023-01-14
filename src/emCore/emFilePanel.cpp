//------------------------------------------------------------------------------
// emFilePanel.cpp
//
// Copyright (C) 2004-2008,2016-2018,2022 Oliver Hamann.
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
	MemoryLimit=GetMemoryLimit();
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
	emFilePanel * fp;
	emPanel * p;

	fm=FileModelClient.GetModel();
	if (fm != fileModel) {
		if (fm) RemoveWakeUpSignal(fm->GetFileStateSignal());
		FileModelClient.SetModel(fileModel);
		if (fileModel) AddWakeUpSignal(fileModel->GetFileStateSignal());
		Signal(VirFileStateSignal);
		InvalidatePainting();
	}
	if (fileModel && updateFileModel) {
		// Workaround: If there are multiple file panels in a panel tree
		// path sharing the same file model, an endless recursion of
		// recreations and reloads could happen, or even a crash
		// (depending on derivative implementation). To avoid that, we
		// do not update from this file panel if it has an ancestor file
		// panel that shares the same file model.
		if (!FileModelClient.IsTheOnlyClient()) {
			for (p=GetParent(); p; p=p->GetParent()) {
				fp=dynamic_cast<emFilePanel*>(p);
				if (fp && fp->GetFileModel()==fileModel) {
					updateFileModel=false;
					break;
				}
			}
		}
		if (updateFileModel) fileModel->Update();
	}
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


emString emFilePanel::GetCustomError() const
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
	if (fm->GetMemoryNeed()>MemoryLimit) return VFS_TOO_COSTLY;
	return (VirtualFileState)fm->GetFileState();
}


bool emFilePanel::IsVFSGood() const
{
	VirtualFileState s;

	s=GetVirFileState();
	return s==VFS_LOADED || s==VFS_UNSAVED;
}


bool emFilePanel::IsReloadAnnoying() const
{
	return IsInActivePath();
}


emString emFilePanel::GetIconFileName() const
{
	return "file.tga";
}


bool emFilePanel::IsContentReady(bool * pReadying) const
{
	switch (GetVirFileState()) {
	case VFS_WAITING:
	case VFS_LOADING:
		if (pReadying) *pReadying=true;
		return false;
	case VFS_LOADED:
	case VFS_UNSAVED:
	case VFS_SAVING:
		return emPanel::IsContentReady(pReadying);
	default:
		if (pReadying) *pReadying=false;
		return false;
	}
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
		FileModelClient.InvalidatePriority();
	}
	if (flags&NF_MEMORY_LIMIT_CHANGED) {
		m=GetMemoryLimit();
		if (MemoryLimit!=m) {
			vfs=GetVirFileState();
			MemoryLimit=m;
			FileModelClient.InvalidateMemoryLimit();
			if (vfs!=GetVirFileState()) {
				Signal(VirFileStateSignal);
				InvalidatePainting();
			}
		}
	}
}


bool emFilePanel::IsOpaque() const
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


void emFilePanel::Paint(const emPainter & painter, emColor canvasColor) const
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
			emColor(92,92,0,192),
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
			emColor(0,112,0,192),
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
			emColor(0,116,112,192),
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
			emColor(144,0,144,192),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case VFS_SAVING:
		sprintf(tmp,"Saving: %.1f%%",GetFileModel()->GetFileProgress());
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			tmp,
			GetHeight()/6,
			emColor(0,112,0,192),
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
			emColor(112,64,64,192),
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
			emColor(128,0,0,192),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	}
}


bool emFilePanel::IsHopeForSeeking() const
{
	VirtualFileState s;

	s=GetVirFileState();
	return s==VFS_WAITING || s==VFS_LOADING || s==VFS_SAVING;
}


emUInt64  emFilePanel::FileModelClientClass::GetMemoryLimit() const
{
	return GetFilePanel().MemoryLimit;
}


double  emFilePanel::FileModelClientClass::GetPriority() const
{
	return GetFilePanel().GetUpdatePriority();
}


bool  emFilePanel::FileModelClientClass::IsReloadAnnoying() const
{
	return GetFilePanel().IsReloadAnnoying();
}
