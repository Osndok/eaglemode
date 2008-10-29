//------------------------------------------------------------------------------
// emTmpConvPanel.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emCore/emFpPlugin.h>
#include <emTmpConv/emTmpConvPanel.h>


emTmpConvPanel::emTmpConvPanel(
	ParentArg parent, const emString & name, emTmpConvModel * model,
	double minViewPercentForTriggering, double minViewPercentForHolding
)
	: emPanel(parent,name)
{
	ModelClient.SetModel(model);
	ChildPanel=NULL;
	if (minViewPercentForHolding>minViewPercentForTriggering) {
		minViewPercentForHolding=minViewPercentForTriggering;
	}
	MinViewPercentForTriggering=minViewPercentForTriggering;
	MinViewPercentForHolding=minViewPercentForHolding;
	AddWakeUpSignal(model->GetChangeSignal());
}


emTmpConvPanel::~emTmpConvPanel()
{
}


emString emTmpConvPanel::GetTitle()
{
	if (GetVirtualConversionState()==emTmpConvModel::CS_UP) {
		return GetModel()->GetOutputFilePath();
	}
	else {
		return emPanel::GetTitle();
	}
}


bool emTmpConvPanel::Cycle()
{
	emTmpConvModel * mdl;

	mdl=GetModel();
	if (IsSignaled(mdl->GetChangeSignal())) {
		if (ChildPanel) {
			SetFocusable(true);
			delete ChildPanel;
			ChildPanel=NULL;
		}
		UpdateModelClientAndChildPanel();
		InvalidatePainting();
	}
	return false;
}


void emTmpConvPanel::Notice(NoticeFlags flags)
{
	if ((flags&NF_UPDATE_PRIORITY_CHANGED)!=0) {
		ModelClient.SetPriority(GetUpdatePriority());
	}
	if ((flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED))!=0) {
		UpdateModelClientAndChildPanel();
	}
}


bool emTmpConvPanel::IsOpaque()
{
	switch (GetVirtualConversionState()) {
	case emTmpConvModel::CS_ERROR:
		return true;
	default:
		return false;
	}
}


void emTmpConvPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emColor c;

	switch (GetVirtualConversionState()) {
	case emTmpConvModel::CS_DOWN:
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
	case emTmpConvModel::CS_WAITING:
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
	case emTmpConvModel::CS_CONVERTING:
		painter.PaintTextBoxed(
			0,
			0,
			1,
			GetHeight(),
			"Converting...",
			GetHeight()/6,
			emColor(0,136,0),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	case emTmpConvModel::CS_UP:
		break;
	case emTmpConvModel::CS_ERROR:
		c.Set(128,0,0);
		painter.PaintRect(0,0,1,GetHeight(),c,canvasColor);
		painter.PaintTextBoxed(
			0.05,
			GetHeight()*0.15,
			0.9,
			GetHeight()*0.1,
			"Conversion Failed",
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
			GetModel()->GetErrorText(),
			GetHeight()*0.4,
			emColor(255,255,0),
			c,
			EM_ALIGN_CENTER,
			EM_ALIGN_LEFT,
			1.0
		);
		break;
	}
}


void emTmpConvPanel::LayoutChildren()
{
	if (ChildPanel) {
		ChildPanel->Layout(0.0,0.0,1.0,GetHeight(),GetCanvasColor());
	}
}


bool emTmpConvPanel::IsHopeForSeeking()
{
	switch (GetVirtualConversionState()) {
	case emTmpConvModel::CS_WAITING:
	case emTmpConvModel::CS_CONVERTING:
		return true;
	default:
		return false;
	}
}


emTmpConvModel::ConversionState emTmpConvPanel::GetVirtualConversionState()
{
	if (!ModelClient.IsConversionWanted()) {
		return emTmpConvModel::CS_DOWN;
	}
	else {
		return GetModel()->GetConversionState();
	}
}


void emTmpConvPanel::UpdateModelClientAndChildPanel()
{
	emRef<emFpPluginList> fppl;
	emTmpConvModel * mdl;
	double p;

	mdl=GetModel();

	if (GetSoughtName()) p=100.0;
	else if (!IsInViewedPath()) p=0.0;
	else if (!IsViewed()) p=100.0;
	else {
		p=
			(GetViewedWidth()*GetViewedHeight()) /
			(GetView().GetCurrentWidth()*GetView().GetCurrentHeight()) * 100.0
		;
	}

	if (p>=MinViewPercentForTriggering) {
		ModelClient.SetConversionWanted(true);
	}
	else if (
		p>=MinViewPercentForHolding && (
			mdl->GetConversionState()==emTmpConvModel::CS_CONVERTING ||
			mdl->GetConversionState()==emTmpConvModel::CS_UP
		)
	) {
		ModelClient.SetConversionWanted(true);
	}
	else {
		ModelClient.SetConversionWanted(false);
	}

	if (GetVirtualConversionState()==emTmpConvModel::CS_UP) {
		if (!ChildPanel) {
			fppl=emFpPluginList::Acquire(GetRootContext());
			ChildPanel=fppl->CreateFilePanel(
				this,
				"conv",
				mdl->GetOutputFilePath()
			);
			if (IsActive()) {
				ChildPanel->Layout(0.0,0.0,1.0,GetHeight(),GetCanvasColor());
				GetView().VisitLazy(ChildPanel,GetView().IsVisitAdherent());
			}
			SetFocusable(false);
		}
	}
	else {
		if (ChildPanel) {
			SetFocusable(true);
			delete ChildPanel;
			ChildPanel=NULL;
		}
	}
}
