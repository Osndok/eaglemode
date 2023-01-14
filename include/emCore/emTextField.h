//------------------------------------------------------------------------------
// emTextField.h
//
// Copyright (C) 2005-2010,2014,2016,2018,2021 Oliver Hamann.
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
		// Get number of bytes in the text.

	int GetCursorIndex() const;
	void SetCursorIndex(int index);
		// Position of cursor as a byte index to the text.

	const emSignal & GetSelectionSignal() const;
		// Signaled whenever the selection has changed.

	int GetSelectionStartIndex() const;
		// Get start of selection as a byte index to the text.

	int GetSelectionEndIndex() const;
		// Get end of selection as an exclusive byte index to the text.

	bool IsSelectionEmpty() const;
		//  Whether the selection is empty.

	void Select(int startIndex, int endIndex, bool publish);
		// Set the selection.
		// Arguments:
		//   startIndex  - Start as a byte index to the text.
		//   endIndex    - End as an exclusive byte index to the text.
		//   publish     - Whether to publish the selection.

	void SelectAll(bool publish);
		// Select the whole text.
		// Arguments:
		//   publish - Whether to publish the selection.

	void EmptySelection();
		// Empty the selection.

	void PublishSelection();
		// Publish the current selection.

	void CutSelectedTextToClipboard();
		// Remove the selected text and put it to the clipboard.

	void CopySelectedTextToClipboard();
		// Copy the selected text to the clipboard.

	void PasteSelectedTextFromClipboard();
		// Paste selected text from the clipboard. If the selection is
		// empty, the text from the clipboard is inserted at the cursor
		// position. Otherwise the selected text is replaced.

	void PasteSelectedText(const emString & text);
		// Paste selected text. If the selection is empty, the given
		// text is inserted at the cursor position. Otherwise the
		// selected text is replaced.

	void DeleteSelectedText();
		// Remove the selected text.

	bool IsCursorBlinkOn() const;
		// Whether the cursor is shown at this moment.

	const emSignal & GetCanUndoRedoSignal() const;
		// Signaled whenever the result of CanUndo() or CanRedo() has changed.

	bool CanUndo() const;
		// Whether there is at least one entry in the undo buffer.

	bool CanRedo() const;
		// Whether there is at least one entry in the redo buffer.

	void Undo();
		// Undo last change if possible.

	void Redo();
		// Redo last undone change if possible.

	virtual bool Validate(int & pos, int & removeLen, emString & insertText) const;
		// This method is called whenever the text is about to change
		// (except by SetText(..)). The implementation may either accept
		// the change (return true), adapt it (modify pos, removeLen
		// and/or insertText, and return true), or reject the change
		// (return false). The default implementation uses the callback
		// function set with SetValidateFunc(..).
		// Arguments:
		//   pos        - Byte index where the text is changed.
		//   removeLen  - How many bytes to remove at position pos.
		//   insertText - The text to be inserted at position pos.
		// Returns:
		//   true on proceed with the change, false to abort.

	void SetValidateFunc(
		bool(*validateFunc)(
			const emTextField & textField, int & pos, int & removeLen,
			emString & insertText, void * context
		),
		void * context=NULL
	);
		// Set a validate function. The arguments to the function are
		// like Validate(..). The context can be any pointer, it is
		// forwarded to the function.

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

	struct UndoEntry {
		UndoEntry * Prev;
		UndoEntry * Next;
		int Pos;
		int RemoveLen;
		emString InsertText;
	};

	struct RedoEntry {
		RedoEntry * Next;
		int Pos;
		int RemoveLen;
		emString InsertText;
	};

	enum UndoMergeType {
		UM_NO_MERGE,
		UM_BACKSPACE,
		UM_DELETE,
		UM_ALPHA_NUM,
		UM_NON_ALPHA_NUM,
		UM_NEW_LINE,
		UM_MOVE
	};

	enum ModifyFlags {
		MF_VALIDATE      = 1<<0,
		MF_CREATE_UNDO   = 1<<1,
		MF_CREATE_REDO   = 1<<2,
		MF_NO_CLEAR_REDO = 1<<3,
		MF_SELECT        = 1<<4
	};

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

	void ClearUndo();
	void ClearRedo();
	void CreateUndo(int pos, int removeLen, const emString & insertText,
	                UndoMergeType undoMerge);
	void CreateRedo(int pos, int removeLen, const emString & insertText);
	void ModifySelectedText(const emString & insertText, int modifyFlags,
	                        UndoMergeType undoMerge=UM_NO_MERGE);
	void ModifyText(int pos, int removeLen, emString insertText,
	                int modifyFlags, UndoMergeType undoMerge=UM_NO_MERGE);

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
	void ModifySelection(int oldIndex, int newIndex, bool publish);
	emMBState GetMBStateAtIndex(int index) const;
	int GetNextIndex(int index, emMBState * mbState=NULL) const;
	int GetPrevIndex(int index) const;
	int GetNextWordBoundaryIndex(int index, bool * pIsDelimiter=NULL,
	                             emMBState * mbState=NULL) const;
	int GetPrevWordBoundaryIndex(int index, bool * pIsDelimiter=NULL) const;
	int GetNextWordIndex(int index, emMBState * mbState=NULL) const;
	int GetPrevWordIndex(int index) const;
	int GetRowStartIndex(int index) const;
	int GetRowEndIndex(int index) const;
	int GetNextRowIndex(int index, emMBState * mbState=NULL) const;
	int GetPrevRowIndex(int index) const;
	int GetNextParagraphIndex(int index, emMBState * mbState=NULL) const;
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

	UndoEntry * FirstUndo;
	UndoEntry * LastUndo;
	size_t UndoSize;
	int UndoCount;
	UndoMergeType UndoMerge;
	RedoEntry * FirstRedo;
	emSignal CanUndoRedoSignal;

	bool(*ValidateFunc)(
		const emTextField & textField, int & pos, int & removeLen,
		emString & insertText, void * context
	);
	void * ValidateFuncContext;

	static const char * const HowToTextField;
	static const char * const HowToMultiLineOff;
	static const char * const HowToMultiLineOn;
	static const char * const HowToReadOnly;
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

inline const emSignal & emTextField::GetCanUndoRedoSignal() const
{
	return CanUndoRedoSignal;
}

inline bool emTextField::CanUndo() const
{
	return FirstUndo!=NULL;
}

inline bool emTextField::CanRedo() const
{
	return FirstRedo!=NULL;
}


#endif
