//------------------------------------------------------------------------------
// emTextField.h
//
// Copyright (C) 2005-2010,2014,2016 Oliver Hamann.
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

#ifndef emTextField_h
#define emTextField_h

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================ emTextField =================================
//==============================================================================

class emTextField : public emBorder {

public:

	// Class for a data field panel showing a single line of text which can
	// optionally be edited by the user. An optional multi-line mode is also
	// provided. Selection and clipboard functions are supported.

	emTextField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		const emString & text=emString(),
		bool editable=false
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   editable    - Whether the text can be edited by the user.

	virtual ~emTextField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the text can be edited by the user.

	bool GetMultiLineMode() const;
	void SetMultiLineMode(bool multiLineMode=true);
		// Whether the text may have multiple lines.

	bool GetPasswordMode() const;
	void SetPasswordMode(bool passwordMode=true);
		// Whether the text is a password that should not really be
		// shown.

	bool GetOverwriteMode() const;
	void SetOverwriteMode(bool overwriteMode=true);
		// Current mode of overwriting or inserting (can be changed with
		// the insert key).

	const emSignal & GetTextSignal() const;
		// Signaled whenever the text has changed.

	const emString & GetText() const;
	void SetText(const emString & text);
		// The text.

	int GetTextLen() const;
	int GetCursorIndex() const;
	void SetCursorIndex(int index);
	const emSignal & GetSelectionSignal() const;
	int GetSelectionStartIndex() const;
	int GetSelectionEndIndex() const;
	void Select(int startIndex, int endIndex, bool publish);
	bool IsSelectionEmpty() const;
	void EmptySelection();
	void SelectAll(bool publish);
	void PublishSelection();
	void CutSelectedTextToClipboard();
	void CopySelectedTextToClipboard();
	void PasteSelectedTextFromClipboard();
	void PasteSelectedText(const emString & text);
	void DeleteSelectedText();
	bool IsCursorBlinkOn() const;
		// Advanced stuff - still undocumented.

protected:

	virtual void TextChanged();
		// Called when the text has changed.

	virtual void SelectionChanged();
		// Called when the selection has changed.

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo() const;
	virtual emString GetHowTo() const;

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

	virtual bool CheckMouse(double mx, double my,
	                        double * pCol, double * pRow) const;

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
	virtual bool CheckMouse(double mx, double my,
	                        double * pCol, double * pRow) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	enum DoTextFieldFunc {
		TEXT_FIELD_FUNC_PAINT,
		TEXT_FIELD_FUNC_XY2CR,
		TEXT_FIELD_FUNC_CR2XY
	};
	void DoTextField(
		DoTextFieldFunc func, const emPainter * painter,
		emColor canvasColor,
		double xIn, double yIn, double * pXOut, double * pYOut, bool * pHit
	) const;

	enum DragModeType {
		DM_NONE,
		DM_SELECT,
		DM_SELECT_BY_WORDS,
		DM_SELECT_BY_ROWS,
		DM_INSERT,
		DM_MOVE
	};

	void SetDragMode(DragModeType dragMode);
	void RestartCursorBlinking();
	void ScrollToCursor();
	int ColRow2Index(double column, double row, bool forCursor) const;
	void Index2ColRow(int index, int * pColumn, int * pRow) const;
	void CalcTotalColsRows(int * pCols, int * pRows) const;
	int GetNormalizedIndex(int index) const;
	void ModifySelection(int oldColumn, int newColumn, bool publish);
	int GetNextIndex(int index) const;
	int GetPrevIndex(int index) const;
	int GetNextWordBoundaryIndex(int index, bool * pIsDelimiter=NULL) const;
	int GetPrevWordBoundaryIndex(int index, bool * pIsDelimiter=NULL) const;
	int GetNextWordIndex(int index) const;
	int GetPrevWordIndex(int index) const;
	int GetRowStartIndex(int index) const;
	int GetRowEndIndex(int index) const;
	int GetNextRowIndex(int index) const;
	int GetPrevRowIndex(int index) const;
	int GetNextParagraphIndex(int index) const;
	int GetPrevParagraphIndex(int index) const;

	emRef<emClipboard> Clipboard;
	emSignal TextSignal;
	emSignal SelectionSignal;
	bool Editable;
	bool MultiLineMode;
	bool PasswordMode;
	bool OverwriteMode;
	emString Text;
	int TextLen,CursorIndex,SelectionStartIndex,SelectionEndIndex;
	int MagicCursorColumn;
	emInt64 SelectionId;
	emUInt64 CursorBlinkTime;
	bool CursorBlinkOn;
	DragModeType DragMode;
	double DragPosC,DragPosR;

	static const char * HowToTextField;
	static const char * HowToMultiLineOff;
	static const char * HowToMultiLineOn;
	static const char * HowToReadOnly;
};

inline bool emTextField::IsEditable() const
{
	return Editable;
}

inline bool emTextField::GetMultiLineMode() const
{
	return MultiLineMode;
}

inline bool emTextField::GetPasswordMode() const
{
	return PasswordMode;
}

inline bool emTextField::GetOverwriteMode() const
{
	return OverwriteMode;
}

inline const emSignal & emTextField::GetTextSignal() const
{
	return TextSignal;
}

inline const emString & emTextField::GetText() const
{
	return Text;
}

inline int emTextField::GetTextLen() const
{
	return TextLen;
}

inline int emTextField::GetCursorIndex() const
{
	return CursorIndex;
}

inline const emSignal & emTextField::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline int emTextField::GetSelectionStartIndex() const
{
	return SelectionStartIndex;
}

inline int emTextField::GetSelectionEndIndex() const
{
	return SelectionEndIndex;
}

inline bool emTextField::IsSelectionEmpty() const
{
	return SelectionStartIndex>=SelectionEndIndex;
}

inline bool emTextField::IsCursorBlinkOn() const
{
	return CursorBlinkOn;
}


#endif
