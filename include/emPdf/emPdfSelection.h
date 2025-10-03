//------------------------------------------------------------------------------
// emPdfSelection.h
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

#ifndef emPdfSelection_h
#define emPdfSelection_h

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif

#ifndef emView_h
#include <emCore/emView.h>
#endif

#ifndef emPdfFileModel_h
#include <emPdf/emPdfFileModel.h>
#endif


class emPdfSelection : public emEngine {

public:

	emPdfSelection(emView & view, emPdfFileModel * fileModel);

	virtual ~emPdfSelection();

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);

	void SetFileModel(emPdfFileModel * fileModel);

	struct PageSelection {
		PageSelection(
			bool nonEmpty=false,
			emPdfServerModel::SelectionStyle style=emPdfServerModel::SEL_GLYPHS,
			double x1=0.0, double y1=0.0, double x2=0.0, double y2=0.0
		);
		bool operator == (const PageSelection & s) const;
		bool operator != (const PageSelection & s) const;

		bool NonEmpty;
		emPdfServerModel::SelectionStyle Style;
		double X1,Y1,X2,Y2;
	};

	const PageSelection & GetPageSelection(int page) const;

	bool IsSelectionEmpty() const;

	const emSignal & GetSelectionSignal() const;

	void Select(
		emPdfServerModel::SelectionStyle style,
		int startPage, double startX, double startY,
		int endPage, double endX, double endY,
		bool publish
	);

	void SelectAll(bool publish=true);

	void EmptySelection(bool unpublish=true);

	void PublishSelection();

	void CopySelectedTextToClipboard();

	void PageInput(
		int page, emInputEvent & event, const emInputState & state,
		double mx, double my
	);

	bool IsSelectingByMouse() const;

protected:

	virtual bool Cycle();

private:

	enum DragModeType {
		DM_NONE,
		DM_SELECT,
		DM_SELECT_BY_WORDS,
		DM_SELECT_BY_ROWS
	};

	struct PageData {
		PageData();
		PageData(const PageData & pageData);
		~PageData();
		PageData & operator = (const PageData & pageData);
		PageSelection Selection;
		emRef<emPdfServerModel::GetSelectedTextJob> GetSelectedTextJob;
		emString TempText;
		emString ErrorText;
	};

	void FinishJobs();

	emRef<emPdfFileModel> FileModel;
	emCrossPtrList CrossPtrList;
	emRef<emClipboard> Clipboard;
	emArray<PageData> Pages;
	emSignal SelectionSignal;
	emString SelectedText;
	emInt64 SelectionId;
	bool SelectedTextPending;
	bool CopyToClipboardPending;
	bool MousePressed;
	bool MouseSelectionPending;
	emPdfServerModel::SelectionStyle MouseSelectionStyle;
	int MouseStartPage,MouseEndPage;
	double MouseStartX,MouseStartY,MouseEndX,MouseEndY;
};


inline void emPdfSelection::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline bool emPdfSelection::PageSelection::operator != (
	const PageSelection & s
) const
{
	return !(*this==s);
}

inline bool emPdfSelection::IsSelectingByMouse() const
{
	return MousePressed;
}


#endif
