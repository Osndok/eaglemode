//------------------------------------------------------------------------------
// emFileLinkPanel.cpp
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

	if (HaveBorder) {
		BorderImage=emGetInsResImage(
			GetRootContext(),"emFileMan","images/InnerBorder.tga"
		);
	}

	Model=fileModel;
	UpdateSignalModel=emFileModel::AcquireUpdateSignalModel(GetRootContext());
	ChildPanel=NULL;
	DirEntryUpToDate=false;

	AddWakeUpSignal(UpdateSignalModel->Sig);
	AddWakeUpSignal(GetVirFileStateSignal());
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
	}

	if (IsSignaled(UpdateSignalModel->Sig)) {
		DirEntryUpToDate=false;
		doUpdate=true;
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
	return false;
}


void emFileLinkPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double x,y,w,h,bw,bh,d;

	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	if (!HaveBorder) return;

	bw=BorderSize;
	bh=BorderSize*GetHeight();
	d=emMin(bw,bh)*0.03;
	x=bw-d;
	y=bh-d;
	w=1.0-2*bw+2*d;
	h=GetHeight()-2*bh+2*d;
	painter.PaintBorderImage(
		x,y,w,h,
		d,d,d,d,
		BorderImage,
		64.0,64.0,64.0,64.0,
		255,canvasColor,0757
	);

	if (CachedFullPath.IsEmpty()) CachedFullPath=Model->GetFullPath();
	d=y*0.2;
	painter.PaintTextBoxed(
		d,
		d,
		1.0-2*d,
		y-2*d,
		emString::Format(
			"emFileLink to: %s",
			CachedFullPath.Get()
		),
		y,
		emColor(221,255,255),
		canvasColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);
}


void emFileLinkPanel::LayoutChildren()
{
	LayoutChildPanel();
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
		if (HaveBorder) {
			x=BorderSize;
			y=BorderSize*GetHeight();
			w=1.0-2*x;
			h=GetHeight()-2*y;
		}
		else {
			x=0.0;
			y=0.0;
			w=1.0;
			h=GetHeight();
		}
		ChildPanel->Layout(x,y,w,h,GetCanvasColor());
	}
}


const double emFileLinkPanel::BorderSize=0.15;
