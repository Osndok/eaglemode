//------------------------------------------------------------------------------
// emPdfControlPanel.cpp
//
// Copyright (C) 2023 Oliver Hamann.
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

#include <emPdf/emPdfControlPanel.h>


emPdfControlPanel::emPdfControlPanel(
	ParentArg parent, const emString & name, emPdfFileModel * fileModel,
	emPdfSelection & selection
) :
	emLinearGroup(parent,name,"PDF File"),
	FileModel(fileModel),
	Selection(&selection),
	Title(NULL),
	Author(NULL),
	Subject(NULL),
	Keywords(NULL),
	Creator(NULL),
	Producer(NULL),
	CreationDate(NULL),
	ModificationDate(NULL),
	Version(NULL),
	PageCount(NULL),
	PageSize(NULL),
	Copy(NULL),
	SelectAll(NULL),
	ClearSelection(NULL)
{
	if (FileModel) {
		AddWakeUpSignal(FileModel->GetFileStateSignal());
		AddWakeUpSignal(FileModel->GetChangeSignal());
	}
	if (Selection) AddWakeUpSignal(Selection->GetSelectionSignal());
}


emPdfControlPanel::~emPdfControlPanel()
{
}


bool emPdfControlPanel::Cycle()
{
	bool busy;

	busy=emLinearGroup::Cycle();

	if (
		FileModel && (
			IsSignaled(FileModel->GetFileStateSignal()) ||
			IsSignaled(FileModel->GetChangeSignal())
		)
	) {
		UpdateControls();
	}

	if (Selection) {
		if (IsSignaled(Selection->GetSelectionSignal())) {
			UpdateControls();
		}
		if (Copy && IsSignaled(Copy->GetClickSignal())) {
			Selection->CopySelectedTextToClipboard();
		}
		if (SelectAll && IsSignaled(SelectAll->GetClickSignal())) {
			Selection->SelectAll(true);
		}
		if (ClearSelection && IsSignaled(ClearSelection->GetClickSignal())) {
			Selection->EmptySelection();
		}
	}

	return busy;
}


void emPdfControlPanel::AutoExpand()
{
	emRasterLayout * subjectAndKeywords, * creatorEtc, * versionAndPages;
	emRasterGroup * infos;
	emLinearGroup * selection;

	emLinearGroup::AutoExpand();

	SetChildWeight(1,0.2);

	infos=new emRasterGroup(this,"infos","Infos");
	infos->SetPrefChildTallness(0.08);

	Title=new emTextField(
		infos,
		"title",
		"Title"
	);
	Title->SetMultiLineMode();

	Author=new emTextField(
		infos,
		"author",
		"Author"
	);
	Author->SetMultiLineMode();

	subjectAndKeywords=new emRasterLayout(infos,"s");
	subjectAndKeywords->SetPrefChildTallness(0.08);

	Subject=new emTextField(
		subjectAndKeywords,
		"subject",
		"Subject"
	);
	Subject->SetMultiLineMode();

	Keywords=new emTextField(
		subjectAndKeywords,
		"keywords",
		"Keywords"
	);
	Keywords->SetMultiLineMode();

	creatorEtc=new emRasterLayout(infos,"c");
	creatorEtc->SetPrefChildTallness(0.08);

	Creator=new emTextField(
		creatorEtc,
		"creator",
		"Creator"
	);
	Creator->SetMultiLineMode();

	Producer=new emTextField(
		creatorEtc,
		"producer",
		"Producer"
	);
	Producer->SetMultiLineMode();

	CreationDate=new emTextField(
		creatorEtc,
		"creation_date",
		"Creation Date"
	);

	ModificationDate=new emTextField(
		creatorEtc,
		"modification_date",
		"Modification Date"
	);

	versionAndPages=new emRasterLayout(infos,"v");
	creatorEtc->SetPrefChildTallness(0.2);

	Version=new emTextField(
		versionAndPages,
		"version",
		"Version"
	);
	Version->SetMultiLineMode();

	PageCount=new emTextField(
		versionAndPages,
		"page_count",
		"Number of Pages"
	);

	PageSize=new emTextField(
		infos,
		"page_size",
		"Page Size"
	);
	PageSize->SetMultiLineMode();

	selection=new emLinearGroup(this,"selection","Selection");

	Copy=new emButton(
		selection,
		"copy",
		"Copy",
		"Copy the selected text to the clipboard.\n"
		"\n"
		"Hotkey: Ctrl+C"
	);
	AddWakeUpSignal(Copy->GetClickSignal());

	SelectAll=new emButton(
		selection,
		"selectAll",
		"Select All",
		"Select all text.\n"
		"\n"
		"Hotkey: Ctrl+A"
	);
	AddWakeUpSignal(SelectAll->GetClickSignal());

	ClearSelection=new emButton(
		selection,
		"clearSelection",
		"Clear Selection",
		"Deselect all text.\n"
		"\n"
		"Hotkey: Shift+Ctrl+A"
	);
	AddWakeUpSignal(ClearSelection->GetClickSignal());

	UpdateControls();
}


void emPdfControlPanel::AutoShrink()
{
	Title=NULL;
	Author=NULL;
	Subject=NULL;
	Keywords=NULL;
	Creator=NULL;
	Producer=NULL;
	CreationDate=NULL;
	ModificationDate=NULL;
	Version=NULL;
	PageCount=NULL;
	PageSize=NULL;
	Copy=NULL;
	SelectAll=NULL;
	ClearSelection=NULL;

	emLinearGroup::AutoShrink();
}


void emPdfControlPanel::UpdateControls()
{
	const emPdfServerModel::DocumentInfo * doc;
	struct tm tmbuf;
	struct tm * p;
	emString str;

	if (!IsAutoExpanded()) return;

	if (
		!FileModel || !Selection || (
			FileModel->GetFileState() != emFileModel::FS_LOADED &&
			FileModel->GetFileState() != emFileModel::FS_UNSAVED
		)
	) {
		Title->SetEnableSwitch(false);
		Title->SetText(emString());

		Author->SetEnableSwitch(false);
		Author->SetText(emString());

		Subject->SetEnableSwitch(false);
		Subject->SetText(emString());

		Keywords->SetEnableSwitch(false);
		Keywords->SetText(emString());

		Creator->SetEnableSwitch(false);
		Creator->SetText(emString());

		Producer->SetEnableSwitch(false);
		Producer->SetText(emString());

		CreationDate->SetEnableSwitch(false);
		CreationDate->SetText(emString());

		ModificationDate->SetEnableSwitch(false);
		ModificationDate->SetText(emString());

		Version->SetEnableSwitch(false);
		Version->SetText(emString());

		PageCount->SetEnableSwitch(false);
		PageCount->SetText(emString());

		PageSize->SetEnableSwitch(false);
		PageSize->SetText(emString());

		Copy->SetEnableSwitch(false);

		SelectAll->SetEnableSwitch(false);

		ClearSelection->SetEnableSwitch(false);

		return;
	}

	doc=&FileModel->GetDocumentInfo();

	Title->SetEnableSwitch(true);
	Title->SetText(doc->Title);

	Author->SetEnableSwitch(true);
	Author->SetText(doc->Author);

	Subject->SetEnableSwitch(true);
	Subject->SetText(doc->Subject);

	Keywords->SetEnableSwitch(true);
	Keywords->SetText(doc->Keywords);

	Creator->SetEnableSwitch(true);
	Creator->SetText(doc->Creator);

	Producer->SetEnableSwitch(true);
	Producer->SetText(doc->Producer);

	CreationDate->SetEnableSwitch(true);
	str.Clear();
	if (doc->CreationDate) {
		p=localtime_r(&doc->CreationDate,&tmbuf);
		if (p) {
			str=emString::Format(
				"%04d-%02d-%02d %02d:%02d:%02d",
				(int)p->tm_year+1900,
				(int)p->tm_mon+1,
				(int)p->tm_mday,
				(int)p->tm_hour,
				(int)p->tm_min,
				(int)p->tm_sec
			);
		}
	}
	CreationDate->SetText(str);

	ModificationDate->SetEnableSwitch(true);
	str.Clear();
	if (doc->ModificationDate) {
		p=localtime_r(&doc->ModificationDate,&tmbuf);
		if (p) {
			str=emString::Format(
				"%04d-%02d-%02d %02d:%02d:%02d",
				(int)p->tm_year+1900,
				(int)p->tm_mon+1,
				(int)p->tm_mday,
				(int)p->tm_hour,
				(int)p->tm_min,
				(int)p->tm_sec
			);
		}
	}
	ModificationDate->SetText(str);

	Version->SetEnableSwitch(true);
	Version->SetText(doc->Version);

	PageCount->SetEnableSwitch(true);
	PageCount->SetText(emString::Format("%d",FileModel->GetPageCount()));

	PageSize->SetEnableSwitch(true);
	PageSize->SetText(CalculatePageSizes());

	Copy->SetEnableSwitch(!Selection->IsSelectionEmpty());
	SelectAll->SetEnableSwitch(true);
	ClearSelection->SetEnableSwitch(!Selection->IsSelectionEmpty());
}


emString emPdfControlPanel::CalculatePageSizes() const
{
	struct PageSize {
		int w;
		int h;
	};
	emArray<PageSize> pageSizes;
	emArray<char> buf;
	PageSize ps;
	emString str;
	int i,j,n,w,h;

	pageSizes.SetTuningLevel(4);
	n=FileModel->GetPageCount();
	for (i=0; i<n; i++) {
		w=(int)(FileModel->GetPageWidth(i)/72.0*25.4+0.5);
		h=(int)(FileModel->GetPageHeight(i)/72.0*25.4+0.5);
		for (j=pageSizes.GetCount()-1; j>=0; j--) {
			if (w==pageSizes[j].w && h==pageSizes[j].h) break;
		}
		if (j<0) {
			ps.w=w;
			ps.h=h;
			pageSizes+=ps;
		}
	}

	buf.SetTuningLevel(4);
	for (i=0; i<pageSizes.GetCount(); i++) {
		str=PageSizeToString(pageSizes[i].w,pageSizes[i].h);
		if (i>0) buf+='\n';
		buf.Add(str.Get(),str.GetCount());
	}
	return emString(buf.Get(),buf.GetCount());
}


emString emPdfControlPanel::PageSizeToString(int w, int h)
{
	struct Size {
		int w;
		int h;
		const char * name;
	};
	static const Size sizes[] = {
		{ 594, 841, "A1" },
		{ 420, 594, "A2" },
		{ 297, 420, "A3" },
		{ 210, 297, "A4" },
		{ 148, 210, "A5" },
		{ 105, 148, "A6" },
		{ 500, 707, "B2" },
		{ 353, 500, "B3" },
		{ 250, 353, "B4" },
		{ 176, 250, "B5" },
		{ 216, 356, "Legal" },
		{ 216, 279, "Letter" }
	};
	static const int numSizes=sizeof(sizes)/sizeof(Size);
	const char * p1, * p2;
	int i;

	p1="";
	p2="";
	for (i=0; i<numSizes; i++) {
		if (w==sizes[i].w && h==sizes[i].h) {
			p1=sizes[i].name;
			p2="";
			break;
		}
		if (w==sizes[i].h && h==sizes[i].w) {
			p1=sizes[i].name;
			p2=" Landscape";
			break;
		}
	}

	if (*p1) {
		return emString::Format(
			"%s%s / %d x %d mm / %.2f x %.2f inch",
			p1,p2,
			w,h,
			w/25.4,h/25.4
		);
	}
	else {
		return emString::Format(
			"%d x %d mm / %.2f x %.2f inch",
			w,h,
			w/25.4,h/25.4
		);
	}
}
