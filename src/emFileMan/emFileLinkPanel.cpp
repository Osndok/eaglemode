//------------------------------------------------------------------------------
// emFileLinkPanel.cpp
//
// Copyright (C) 2007-2008,2010 Oliver Hamann.
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
#include <emFileMan/emFileLinkPanel.h>
#include <emFileMan/emDirEntryPanel.h>
#include <emFileMan/emDirEntryAltPanel.h>


emFileLinkPanel::emFileLinkPanel(
	ParentArg parent, const emString & name, emFileLinkModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true)
{
	emPanel * p;

	p=GetParent();
	HaveBorder = (
		p && (
			dynamic_cast<emDirEntryPanel *>(p)!=NULL ||
			dynamic_cast<emDirEntryAltPanel *>(p)!=NULL ||
			dynamic_cast<emFileLinkPanel *>(p)!=NULL
		)
	);

	Model=fileModel;
	UpdateSignalModel=emFileModel::AcquireUpdateSignalModel(GetRootContext());
	Config=emFileManViewConfig::Acquire(GetView());
	ChildPanel=NULL;
	DirEntryUpToDate=false;

	AddWakeUpSignal(UpdateSignalModel->Sig);
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(Config->GetChangeSignal());
	if (Model) AddWakeUpSignal(Model->GetChangeSignal());
}


emFileLinkPanel::~emFileLinkPanel()
{
}


void emFileLinkPanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (Model) RemoveWakeUpSignal(Model->GetChangeSignal());
	Model=dynamic_cast<emFileLinkModel*>(fileModel);
	emFilePanel::SetFileModel(Model,updateFileModel);
	if (Model) AddWakeUpSignal(Model->GetChangeSignal());
	CachedFullPath.Empty();
	DirEntryUpToDate=false;
	InvalidatePainting();
	UpdateChildPanel(true);
}


bool emFileLinkPanel::Cycle()
{
	bool busy,doUpdate,fromScratch;

	busy=emFilePanel::Cycle();

	doUpdate=false;
	fromScratch=false;

	if (IsSignaled(GetVirFileStateSignal())) {
		doUpdate=true;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}

	if (IsSignaled(UpdateSignalModel->Sig)) {
		DirEntryUpToDate=false;
		doUpdate=true;
	}

	if (IsSignaled(Config->GetChangeSignal())) {
		InvalidatePainting();
		InvalidateChildrenLayout();
	}

	if (Model && IsSignaled(Model->GetChangeSignal())) {
		CachedFullPath.Empty();
		DirEntryUpToDate=false;
		doUpdate=true;
		fromScratch=true;
		InvalidatePainting();
	}

	if (doUpdate) UpdateChildPanel(fromScratch);

	return busy;
}


void emFileLinkPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if (flags&NF_VIEWING_CHANGED) UpdateChildPanel();
}


bool emFileLinkPanel::IsOpaque()
{
	if (!IsVFSGood()) {
		return emFilePanel::IsOpaque();
	}
	else if (HaveBorder) {
		return BorderBgColor.IsOpaque();
	}
	else if (Model->HaveDirEntry) {
		return Config->GetTheme().DirContentColor.Get().IsOpaque();
	}
	else {
		return false;
	}
}


void emFileLinkPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double x,y,w,h,d,t;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	if (HaveBorder) {
		painter.Clear(BorderBgColor);
		canvasColor=BorderBgColor;
		CalcContentCoords(&x,&y,&w,&h);
		d=emMin(x,y)*0.15;
		t=emMin(x,y)*0.03;
		painter.PaintRectOutline(
			x-d*0.5,
			y-d*0.5,
			w+d,
			h+d,
			t,
			BorderFgColor
		);
		if (CachedFullPath.IsEmpty()) CachedFullPath=Model->GetFullPath();
		t=emMin(x,y)*0.2;
		painter.PaintTextBoxed(
			t,0.0,1.0-t*2.0,y-t,
			emString::Format(
				"emFileLink to %s",
				CachedFullPath.Get()
			),
			(y-t)*0.9,
			BorderFgColor,
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
		if (Model->HaveDirEntry) {
			painter.PaintRect(x,y,w,h,Config->GetTheme().DirContentColor,canvasColor);
		}
	}
	else if (Model->HaveDirEntry) {
		painter.Clear(Config->GetTheme().DirContentColor,canvasColor);
	}
}


void emFileLinkPanel::LayoutChildren()
{
	LayoutChildPanel();
}


void emFileLinkPanel::CalcContentCoords(
	double * pX, double * pY, double * pW, double * pH
)
{
	double t,x,y,w,h;

	if (HaveBorder) {
		x=0.15;
		y=x*GetHeight();
		w=1.0-2*x;
		h=GetHeight()-2*y;
	}
	else {
		x=0.0;
		y=0.0;
		w=1.0;
		h=GetHeight();
	}
	if (IsVFSGood() && Model->HaveDirEntry) {
		t=Config->GetTheme().Height;
		if (h>w*t) {
			y+=(h-w*t)*0.5;
			h=w*t;
		}
		else {
			x+=(w-h/t)*0.5;
			w=h/t;
		}
	}
	*pX=x;
	*pY=y;
	*pW=w;
	*pH=h;
}


void emFileLinkPanel::UpdateChildPanel(bool forceRecreation)
{
	emRef<emFpPluginList> fppl;
	emDirEntryPanel * dep;
	emString str;
	bool haveIt;

	haveIt = (IsVFSGood() && GetViewCondition()>=60.0);

	emDirEntry oldDirEntry(DirEntry);

	if (haveIt) {
		if (CachedFullPath.IsEmpty()) CachedFullPath=Model->GetFullPath();
		if (!DirEntryUpToDate) {
			DirEntry=emDirEntry(CachedFullPath);
			DirEntryUpToDate=true;
			if (DirEntry!=oldDirEntry && ChildPanel && !forceRecreation) {
				dep=dynamic_cast<emDirEntryPanel*>(ChildPanel);
				if (dep) {
					dep->UpdateDirEntry(DirEntry);
				}
				else if (
					DirEntry.GetPath()!=oldDirEntry.GetPath() ||
					DirEntry.GetStatErrNo()!=oldDirEntry.GetStatErrNo() ||
					DirEntry.GetStat()->st_mode!=oldDirEntry.GetStat()->st_mode
				) {
					forceRecreation=true;
				}
			}
		}
	}

	if (!haveIt || forceRecreation) {
		if (ChildPanel) {
			if (!HaveBorder) SetFocusable(true);
			delete ChildPanel;
			ChildPanel=NULL;
		}
	}

	if (haveIt && !ChildPanel) {
		if (Model->HaveDirEntry) {
			ChildPanel=new emDirEntryPanel(this,"",DirEntry);
		}
		else {
			fppl=emFpPluginList::Acquire(GetRootContext());
			ChildPanel=fppl->CreateFilePanel(
				this,
				"",
				DirEntry.GetPath(),
				DirEntry.GetStatErrNo(),
				DirEntry.GetStat()->st_mode
			);
		}
		if (!HaveBorder) {
			if (IsActive()) {
				LayoutChildPanel();
				GetView().VisitLazy(ChildPanel,GetView().IsVisitAdherent());
			}
			SetFocusable(false);
		}
	}
}


void emFileLinkPanel::LayoutChildPanel()
{
	double x,y,w,h;

	if (ChildPanel) {
		CalcContentCoords(&x,&y,&w,&h);
		ChildPanel->Layout(
			x,y,w,h,
			Model->HaveDirEntry ? Config->GetTheme().DirContentColor.Get() :
			HaveBorder ? BorderBgColor :
			GetCanvasColor()
		);
	}
}

const emColor emFileLinkPanel::BorderBgColor=0xBBBBBBFF;
const emColor emFileLinkPanel::BorderFgColor=0x444444FF;
