//------------------------------------------------------------------------------
// emPdfSelection.cpp
//
// Copyright (C) 2023-2024 Oliver Hamann.
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

#include <emPdf/emPdfSelection.h>


emPdfSelection::emPdfSelection(emView & view, emPdfFileModel * fileModel)
	: emEngine(view.GetScheduler()),
	FileModel(NULL),
	Clipboard(emClipboard::LookupInherited(view)),
	SelectionId(-1),
	SelectedTextPending(false),
	CopyToClipboardPending(false),
	MousePressed(false),
	MouseSelectionPending(false),
	MouseSelectionStyle(emPdfServerModel::SEL_GLYPHS),
	MouseStartPage(0),
	MouseEndPage(0),
	MouseStartX(0.0),
	MouseStartY(0.0),
	MouseEndX(0.0),
	MouseEndY(0.0)
{
	SetFileModel(fileModel);
}


emPdfSelection::~emPdfSelection()
{
	EmptySelection(false);
}


void emPdfSelection::SetFileModel(emPdfFileModel * fileModel)
{
	EmptySelection();
	if (FileModel) {
		RemoveWakeUpSignal(FileModel->GetFileStateSignal());
		RemoveWakeUpSignal(FileModel->GetChangeSignal());
	}
	FileModel=fileModel;
	if (FileModel) {
		AddWakeUpSignal(FileModel->GetFileStateSignal());
		AddWakeUpSignal(FileModel->GetChangeSignal());
	}
	WakeUp();
}


emPdfSelection::PageSelection::PageSelection(
	bool nonEmpty, emPdfServerModel::SelectionStyle style,
	double x1, double y1, double x2, double y2
) :
	NonEmpty(nonEmpty),
	Style(style),
	X1(x1),
	Y1(y1),
	X2(x2),
	Y2(y2)
{
}


bool emPdfSelection::PageSelection::operator == (const PageSelection & s) const
{
	if (NonEmpty!=s.NonEmpty) return false;
	if (!NonEmpty) return true;
	return Style==s.Style && X1==s.X1 && Y1==s.Y1 && X2==s.X2 && Y2==s.Y2;
}


const emPdfSelection::PageSelection & emPdfSelection::GetPageSelection(
	int page
) const
{
	static const PageSelection empty;
	if (page<0 || page>=Pages.GetCount()) return empty;
	return Pages[page].Selection;
}


bool emPdfSelection::IsSelectionEmpty() const
{
	return !SelectedTextPending && SelectedText.IsEmpty();
}


const emSignal & emPdfSelection::GetSelectionSignal() const
{
	return SelectionSignal;
}


void emPdfSelection::Select(
	emPdfServerModel::SelectionStyle style, int startPage, double startX,
	double startY, int endPage, double endX, double endY, bool publish
)
{
	PageData * page;
	double d;
	int i,pageCount;

	EmptySelection();

	pageCount=Pages.GetCount();
	if (pageCount<=0 || pageCount!=FileModel->GetPageCount()) return;

	if (startPage>endPage) {
		i=startPage; startPage=endPage; endPage=i;
		d=startX; startX=endX; endX=d;
		d=startY; startY=endY; endY=d;
	}
	if (startPage<0) {
		startPage=0;
		startX=0.0;
		startY=0.0;
	}
	if (endPage>=pageCount) {
		endPage=pageCount-1;
		endX=FileModel->GetPageWidth(pageCount-1);
		endY=FileModel->GetPageHeight(pageCount-1);
	}

	if (startPage==endPage && startX==endX && startY==endY) return;

	for (i=startPage; i<=endPage; i++) {
		page=&Pages.GetWritable(i);
		page->Selection.NonEmpty=true;
		page->Selection.Style=style;
		if (i==startPage) {
			page->Selection.X1=startX;
			page->Selection.Y1=startY;
		}
		else {
			page->Selection.X1=0.0;
			page->Selection.Y1=0.0;
		}
		if (i==endPage) {
			page->Selection.X2=endX;
			page->Selection.Y2=endY;
		}
		else {
			page->Selection.X2=FileModel->GetPageWidth(i);
			page->Selection.Y2=FileModel->GetPageHeight(i);
		}
	}

	SelectedTextPending=true;

	Signal(SelectionSignal);

	if (publish) PublishSelection();
}


void emPdfSelection::SelectAll(bool publish)
{
	int pageCount;

	pageCount=Pages.GetCount();
	if (pageCount>0 && FileModel->GetPageCount()==pageCount) {
		Select(
			emPdfServerModel::SEL_GLYPHS,
			0,
			0.0,
			0.0,
			pageCount-1,
			FileModel->GetPageWidth(pageCount-1),
			FileModel->GetPageHeight(pageCount-1),
			publish
		);
	}
}


void emPdfSelection::EmptySelection(bool unpublish)
{
	PageData * page;
	int i;
	bool changed;

	changed=false;
	for (i=0; i<Pages.GetCount(); i++) {
		page=&Pages.GetWritable(i);
		if (page->Selection.NonEmpty) {
			page->Selection.NonEmpty=false;
			changed=true;
		}
		if (page->GetSelectedTextJob) {
			FileModel->GetServerModel()->AbortJob(*page->GetSelectedTextJob);
			page->GetSelectedTextJob=NULL;
		}
		page->TempText.Clear();
		page->ErrorText.Clear();
	}

	if (SelectedTextPending) {
		SelectedTextPending=false;
		changed=true;
	}
	CopyToClipboardPending=false;
	if (!SelectedText.IsEmpty()) {
		SelectedText.Clear();
		changed=true;
	}

	if (unpublish && SelectionId!=-1) {
		Clipboard->Clear(true,SelectionId);
		SelectionId=-1;
	}

	if (changed) Signal(SelectionSignal);
}


void emPdfSelection::PublishSelection()
{
	emPdfServerModel * serverModel;
	PageData * page;
	int i,pageCount;

	if (SelectionId!=-1) return;
	if (!SelectedTextPending) return;

	pageCount=Pages.GetCount();
	if (pageCount<=0 || pageCount!=FileModel->GetPageCount()) return;

	serverModel=FileModel->GetServerModel();

	for (i=0; i<pageCount; i++) {
		page=&Pages.GetWritable(i);
		if (
			page->Selection.NonEmpty &&
			!page->GetSelectedTextJob &&
			page->TempText.IsEmpty()
		) {
			page->GetSelectedTextJob=new emPdfServerModel::GetSelectedTextJob(
				FileModel->GetPdfInstance(),
				i,
				page->Selection.Style,
				page->Selection.X1,
				page->Selection.Y1,
				page->Selection.X2,
				page->Selection.Y2,
				0.0
			);
			serverModel->EnqueueJob(*page->GetSelectedTextJob);
			AddWakeUpSignal(page->GetSelectedTextJob->GetStateSignal());
		}
	}
}


void emPdfSelection::CopySelectedTextToClipboard()
{
	if (SelectedTextPending) {
		CopyToClipboardPending=true;
		return;
	}
	if (!SelectedText.IsEmpty()) Clipboard->PutText(SelectedText);
	CopyToClipboardPending=false;
}


void emPdfSelection::PageInput(
	int page, emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double x1,y1,x2,y2,h,dx1,dy1,dx2,dy2;
	int i,pg1,pg2;

	if (page<0 || page>=FileModel->GetPageCount()) return;

	// Allow Alt and Meta for selecting links, see emPdfPagePanel::Input.
	if (event.IsKey(EM_KEY_LEFT_BUTTON) && !state.GetCtrl()) {
		if (event.GetRepeat()>2) {
			MousePressed=false;
			MouseSelectionPending=false;
			SelectAll();
			return;
		}

		MousePressed=true;
		MouseSelectionStyle=
			event.GetRepeat()==0 ? emPdfServerModel::SEL_GLYPHS:
			event.GetRepeat()==1 ? emPdfServerModel::SEL_WORDS:
			emPdfServerModel::SEL_LINES
		;
		MouseStartPage=MouseEndPage=page;
		MouseStartX=MouseEndX=mx;
		MouseStartY=MouseEndY=my;
		if (event.GetRepeat()>0) MouseStartX-=1.0;
		if (state.GetShift()) {
			pg1=pg2=-1;
			for (i=0; i<Pages.GetCount(); i++) {
				if (Pages[i].Selection.NonEmpty) {
					if (pg1<0) pg1=i;
					pg2=i;
				}
			}
			if (pg1>=0) {
				x1=Pages[pg1].Selection.X1;
				y1=Pages[pg1].Selection.Y1;
				x2=Pages[pg2].Selection.X2;
				y2=Pages[pg2].Selection.Y2;
				h=FileModel->GetPageHeight(page);
				dx1=mx-x1;
				dy1=page*h+my-pg1*h-y1;
				dx2=mx-x2;
				dy2=page*h+my-pg2*h-y2;
				if (dx1*dx1+dy1*dy1<dx2*dx2+dy2*dy2) {
					MouseStartPage=pg2;
					MouseStartX=x2;
					MouseStartY=y2;
				}
				else {
					MouseStartPage=pg1;
					MouseStartX=x1;
					MouseStartY=y1;
				}
				MouseSelectionStyle=Pages[pg1].Selection.Style;
			}
		}
		EmptySelection();
		MouseSelectionPending=true;
		WakeUp();
		return;
	}

	if (MousePressed && !MouseSelectionPending) {
		// First page of input round.
		MouseEndPage=page;
		MouseEndX=mx;
		MouseEndY=my;
		MouseSelectionPending=true;
		WakeUp();
	}
	else if (MouseSelectionPending) {
		// Other page of same input round - is it closer to the mouse?
		// MousePressed possibly already false.
		dx1=mx-FileModel->GetPageWidth(page)*0.5;
		dy1=my-FileModel->GetPageHeight(page)*0.5;
		dx2=MouseEndX-FileModel->GetPageWidth(MouseEndPage)*0.5;
		dy2=MouseEndY-FileModel->GetPageHeight(MouseEndPage)*0.5;
		if (dx1*dx1+dy1*dy1<dx2*dx2+dy2*dy2) {
			MouseEndPage=page;
			MouseEndX=mx;
			MouseEndY=my;
		}
	}

	if (!state.Get(EM_KEY_LEFT_BUTTON)) {
		MousePressed=false;
	}
}


bool emPdfSelection::Cycle()
{
	if (
		IsSignaled(FileModel->GetChangeSignal()) ||
		Pages.GetCount()!=FileModel->GetPageCount()
	) {
		EmptySelection();
		MousePressed=false;
		MouseSelectionPending=false;
		Pages.SetCount(FileModel->GetPageCount());
	}

	FinishJobs();

	if (MouseSelectionPending) {
		Select(
			MouseSelectionStyle,
			MouseStartPage,
			MouseStartX,
			MouseStartY,
			MouseEndPage,
			MouseEndX,
			MouseEndY,
			!MousePressed
		);
		MouseSelectionPending=false;
	}

	if (CopyToClipboardPending && !SelectedTextPending) {
		CopySelectedTextToClipboard();
	}

	return false;
}


emPdfSelection::PageData::PageData()
{
}


emPdfSelection::PageData::PageData(const PageData & pageData)
	: Selection(pageData.Selection),
	GetSelectedTextJob(pageData.GetSelectedTextJob),
	TempText(pageData.TempText),
	ErrorText(pageData.ErrorText)
{
}


emPdfSelection::PageData::~PageData()
{
}


emPdfSelection::PageData & emPdfSelection::PageData::operator = (
	const PageData & pageData
)
{
	Selection=pageData.Selection;
	GetSelectedTextJob=pageData.GetSelectedTextJob;
	TempText=pageData.TempText;
	ErrorText=pageData.ErrorText;
	return *this;
}


void emPdfSelection::FinishJobs()
{
	emPdfServerModel * serverModel;
	PageData * page;
	char * p;
	int i,len,l;
	bool allDone;

	if (!SelectedTextPending) return;

	allDone=true;
	serverModel=FileModel->GetServerModel();
	for (i=0; i<Pages.GetCount(); i++) {
		page=&Pages.GetWritable(i);
		if (!page->GetSelectedTextJob) continue;
		switch (page->GetSelectedTextJob->GetState()) {
		case emJob::ST_ERROR:
			page->ErrorText=page->GetSelectedTextJob->GetErrorText();
			page->GetSelectedTextJob=NULL;
			break;
		case emJob::ST_ABORTED:
			page->ErrorText="Aborted";
			page->GetSelectedTextJob=NULL;
			break;
		case emJob::ST_SUCCESS:
			page->TempText=page->GetSelectedTextJob->GetSelectedText();
			page->GetSelectedTextJob=NULL;
			break;
		default:
			allDone=false;
			break;
		}
	}
	if (!allDone) return;

	for (len=0, i=0; i<Pages.GetCount(); i++) {
		page=&Pages.GetWritable(i);
		len+=page->TempText.GetLen();
	}
	p=SelectedText.SetLenGetWritable(len);
	for (i=0; i<Pages.GetCount(); i++) {
		page=&Pages.GetWritable(i);
		l=page->TempText.GetLen();
		if (!l) continue;
		memcpy(p,page->TempText.Get(),l);
		p+=l;
		page->TempText.Clear();
	}

	if (!SelectedText.IsEmpty()) {
		SelectionId=Clipboard->PutText(SelectedText,true);
	}
	SelectedTextPending=false;
	Signal(SelectionSignal);
}
