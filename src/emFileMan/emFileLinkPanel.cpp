//------------------------------------------------------------------------------
// emFileLinkPanel.cpp
//
// Copyright (C) 2007-2008,2010,2014,2016-2017 Oliver Hamann.
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
	HaveDirEntryPanel=false;
	DirEntryUpToDate=false;
	ChildPanel=NULL;

	SetAutoplayHandling(0);

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
	UpdateDataAndChildPanel();
}


bool emFileLinkPanel::Cycle()
{
	bool busy,doUpdate;

	busy=emFilePanel::Cycle();

	doUpdate=false;

	if (IsSignaled(GetVirFileStateSignal())) {
		InvalidatePainting();
		doUpdate=true;
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
		doUpdate=true;
	}

	if (doUpdate) UpdateDataAndChildPanel();

	return busy;
}


void emFileLinkPanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);
	if (flags&NF_VIEWING_CHANGED) UpdateDataAndChildPanel();
}


bool emFileLinkPanel::IsOpaque() const
{
	if (!IsVFSGood() && !ChildPanel) {
		return emFilePanel::IsOpaque();
	}
	else if (HaveBorder) {
		return BorderBgColor.IsOpaque();
	}
	else if (HaveDirEntryPanel) {
		return Config->GetTheme().DirContentColor.Get().IsOpaque();
	}
	else {
		return false;
	}
}


void emFileLinkPanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	double x,y,w,h,d,t;

	if (!IsVFSGood() && !ChildPanel) {
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
		t=emMin(x,y)*0.2;
		painter.PaintTextBoxed(
			t,0.0,1.0-t*2.0,y-t,
			emString::Format(
				"emFileLink to %s",
				FullPath.Get()
			),
			(y-t)*0.9,
			BorderFgColor,
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
		if (HaveDirEntryPanel) {
			painter.PaintRect(x,y,w,h,Config->GetTheme().DirContentColor,canvasColor);
		}
	}
	else if (HaveDirEntryPanel) {
		painter.Clear(Config->GetTheme().DirContentColor,canvasColor);
	}
}


void emFileLinkPanel::LayoutChildren()
{
	LayoutChildPanel();
}


void emFileLinkPanel::CalcContentCoords(
	double * pX, double * pY, double * pW, double * pH
) const
{
	const emFileManTheme * theme;
	double t,x,y,w,h,pl,pt,pr,pb;

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
	if (HaveDirEntryPanel) {
		theme = &Config->GetTheme();
		t=theme->Height;
		if (!HaveBorder) {
			pl=theme->LnkPaddingL;
			pt=theme->LnkPaddingT;
			pr=theme->LnkPaddingR;
			pb=theme->LnkPaddingB;
			w/=(pl+1+pr);
			h/=(pt+t+pb)/t;
			x+=pl*w;
			y+=pt*h/t;
		}
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


void emFileLinkPanel::UpdateDataAndChildPanel()
{
	emDirEntryPanel * dep;
	emString fullPath;
	bool haveDirEntryPanel,viewConditionGood;

	viewConditionGood = (GetViewCondition()>=60.0);
	if (!viewConditionGood) {
		DeleteChildPanel();
	}

	if (IsVFSGood()) {
		fullPath=Model->GetFullPath();
		haveDirEntryPanel=Model->HaveDirEntry.Get();
		if (HaveDirEntryPanel!=haveDirEntryPanel || FullPath!=fullPath) {
			DeleteChildPanel();
			FullPath=fullPath;
			HaveDirEntryPanel=haveDirEntryPanel;
			DirEntryUpToDate=false;
			InvalidatePainting();
		}
	}
	else {
		if (
			ChildPanel &&
			!ChildPanel->IsInActivePath() && (
				!ChildPanel->IsInViewedPath() || IsViewed()
			)
		) {
			DeleteChildPanel();
		}
		if (!ChildPanel) {
			fullPath="";
			haveDirEntryPanel=false;
			if (HaveDirEntryPanel!=haveDirEntryPanel || FullPath!=fullPath) {
				FullPath=fullPath;
				HaveDirEntryPanel=haveDirEntryPanel;
				DirEntryUpToDate=false;
				InvalidatePainting();
			}
		}
	}

	if (ChildPanel && !DirEntryUpToDate) {
		emDirEntry oldDirEntry(DirEntry);
		DirEntry=emDirEntry(FullPath);
		DirEntryUpToDate=true;
		if (DirEntry!=oldDirEntry) {
			if (HaveDirEntryPanel) {
				dep=dynamic_cast<emDirEntryPanel*>(ChildPanel);
				if (dep) {
					dep->UpdateDirEntry(DirEntry);
				}
			}
			else if (
				DirEntry.GetPath()!=oldDirEntry.GetPath() ||
				DirEntry.GetStatErrNo()!=oldDirEntry.GetStatErrNo() ||
				(DirEntry.GetStat()->st_mode&S_IFMT)!=(oldDirEntry.GetStat()->st_mode&S_IFMT)
			) {
				DeleteChildPanel();
			}
		}
	}

	if (!ChildPanel && IsVFSGood() && viewConditionGood) {
		if (!DirEntryUpToDate) {
			DirEntry=emDirEntry(FullPath);
			DirEntryUpToDate=true;
		}
		CreateChildPanel();
	}
}


void emFileLinkPanel::CreateChildPanel()
{
	emRef<emFpPluginList> fppl;

	if (!ChildPanel) {
		if (HaveDirEntryPanel) {
			ChildPanel=new emDirEntryPanel(this,"",DirEntry);
			if (!HaveBorder) {
				ChildPanel->SetAutoplayHandling(
					ChildPanel->GetAutoplayHandling() | APH_ITEM
				);
			}
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
			if (!HaveBorder) {
				if (ChildPanel->GetAutoplayHandling() & APH_DIRECTORY) {
					ChildPanel->SetAutoplayHandling(
						ChildPanel->GetAutoplayHandling() | APH_ITEM
					);
				}
			}
		}
		if (!HaveBorder) {
			if (IsActive()) {
				ChildPanel->Activate(IsActivatedAdherent());
			}
			SetFocusable(false);
		}
		InvalidatePainting();
	}
}


void emFileLinkPanel::DeleteChildPanel()
{
	if (ChildPanel) {
		if (!HaveBorder) SetFocusable(true);
		delete ChildPanel;
		ChildPanel=NULL;
		InvalidatePainting();
	}
}


void emFileLinkPanel::LayoutChildPanel()
{
	double x,y,w,h;

	if (ChildPanel) {
		CalcContentCoords(&x,&y,&w,&h);
		ChildPanel->Layout(
			x,y,w,h,
			HaveDirEntryPanel ? Config->GetTheme().DirContentColor.Get() :
			HaveBorder ? BorderBgColor :
			GetCanvasColor()
		);
	}
}

const emColor emFileLinkPanel::BorderBgColor=0xBBBBBBFF;
const emColor emFileLinkPanel::BorderFgColor=0x444444FF;
