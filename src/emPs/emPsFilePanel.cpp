//------------------------------------------------------------------------------
// emPsFilePanel.cpp
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

#include <emPs/emPsFilePanel.h>


emPsFilePanel::emPsFilePanel(
	ParentArg parent, const emString & name,
	emPsFileModel * fileModel, bool updateFileModel
)
	: emFilePanel(parent,name,fileModel,updateFileModel)
{
	DocPanel=NULL;
	AddWakeUpSignal(GetVirFileStateSignal());
	UpdateDocPanel();
}


emPsFilePanel::~emPsFilePanel()
{
}


void emPsFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (fileModel && (dynamic_cast<emPsFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}
	emFilePanel::SetFileModel(fileModel,updateFileModel);
}


bool emPsFilePanel::Cycle()
{
	if (IsSignaled(GetVirFileStateSignal())) UpdateDocPanel();
	return emFilePanel::Cycle();
}


bool emPsFilePanel::IsOpaque()
{
	if (!DocPanel) return emFilePanel::IsOpaque();
	return false;
}


void emPsFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (!DocPanel) emFilePanel::Paint(painter,canvasColor);
}


void emPsFilePanel::LayoutChildren()
{
	if (DocPanel) {
		DocPanel->Layout(0,0,1,GetHeight(),GetCanvasColor());
	}
}


void emPsFilePanel::UpdateDocPanel()
{
	emPsFileModel * fm;

	if (IsVFSGood()) {
		HaveDocPanel(true);
		fm=(emPsFileModel*)GetFileModel();
		DocPanel->SetDocument(fm->GetDocument());
	}
	else {
		HaveDocPanel(false);
	}
}


void emPsFilePanel::HaveDocPanel(bool haveIt)
{
	emPsFileModel * fm;

	if (haveIt) {
		if (!DocPanel) {
			fm=(emPsFileModel*)GetFileModel();
			DocPanel=new emPsDocumentPanel(this,"doc",fm->GetDocument());
			if (IsActive()) {
				DocPanel->Layout(0,0,1,GetHeight(),GetCanvasColor());
				GetView().VisitLazy(DocPanel,GetView().IsVisitAdherent());
			}
			SetFocusable(false);
		}
	}
	else {
		if (DocPanel) {
			SetFocusable(true);
			delete DocPanel;
			DocPanel=NULL;
		}
	}
}
