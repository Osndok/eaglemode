//------------------------------------------------------------------------------
// emTextFilePanel.h
//
// Copyright (C) 2004-2008,2010,2016,2018,2023 Oliver Hamann.
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

#ifndef emTextFilePanel_h
#define emTextFilePanel_h

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emTextFileModel_h
#include <emText/emTextFileModel.h>
#endif


class emTextFilePanel : public emFilePanel {

public:

	emTextFilePanel(ParentArg parent, const emString & name,
	                emTextFileModel * fileModel=NULL,
	                bool updateFileModel=true, bool alternativeView=false);
	~emTextFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);
	emTextFileModel * GetFileModel() const;

	virtual emString GetIconFileName() const;

	bool IsHexView() const;

	const emSignal & GetSelectionSignal() const;
	int GetSelectionStartIndex() const;
	int GetSelectionEndIndex() const;
	bool IsSelectionEmpty() const;
	void Select(int startIndex, int endIndex, bool publish);
	void SelectAll(bool publish);
	void EmptySelection();
	void PublishSelection();
	void CopySelectedTextToClipboard();

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual emCursor GetCursor() const;

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	enum DragModeType {
		DM_NONE,
		DM_OVER_TEXT,
		DM_SELECT,
		DM_SELECT_BY_WORDS,
		DM_SELECT_BY_ROWS
	};

	void SetDragMode(DragModeType dragMode);

	void UpdateTextLayout();

	bool CheckMouse(double mx, double my,
	                double * pCol, double * pRow) const;

	void ModifySelection(int oldIndex, int newIndex, bool publish);

	emString ConvertSelectedTextToCurrentLocale() const;

	void PaintAsText(const emPainter & painter, emColor canvasColor) const;

	void PaintTextRowsSilhouette(
		const emPainter & painter, double x, double y, int row, int endRow
	) const;

	void PaintTextRows(
		const emPainter & painter, double x, double y, int row, int endRow
	) const;

	int PaintTextRowPart(
		const emPainter & painter, double rowX, double rowY, int column,
		const char * src, const char * srcEnd, emMBState * mbState,
		emColor fgColor, emColor bgColor, emColor canvasColor
	) const;

	void PaintAsHex(const emPainter & painter, emColor canvasColor) const;

	bool AlternativeView;
	emTextFileModel * Model;
	emRef<emClipboard> Clipboard;
	int PageCount,PageRows,PageCols;
	double PageWidth,PageGap,CharWidth,CharHeight;
	emSignal SelectionSignal;
	int SelectionStartIndex,SelectionEndIndex;
	emInt64 SelectionId;
	DragModeType DragMode;
	int DragIndex;

	static const emColor TextBgColor;
	static const emColor TextFgColor;
	static const emColor TextFg96Color;
	static const emColor TextSelFgColor;
	static const emColor TextSelFg96Color;
	static const emColor TextSelBgColor;
	static const emColor HexBgColor;
	static const emColor HexAddrColor;
	static const emColor HexDataColor;
	static const emColor HexAscColor;
	static const emColor HexAddr64Color;
	static const emColor HexData48Color;
	static const emColor HexAsc64Color;
	static const emColor HexAddr96Color;
	static const emColor HexData96Color;
};


inline emTextFileModel * emTextFilePanel::GetFileModel() const
{
	return Model;
}

inline const emSignal & emTextFilePanel::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline int emTextFilePanel::GetSelectionStartIndex() const
{
	return SelectionStartIndex;
}

inline int emTextFilePanel::GetSelectionEndIndex() const
{
	return SelectionEndIndex;
}

inline bool emTextFilePanel::IsSelectionEmpty() const
{
	return SelectionStartIndex>=SelectionEndIndex;
}


#endif
