//------------------------------------------------------------------------------
// emFileDialog.h
//
// Copyright (C) 2015 Oliver Hamann.
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

#ifndef emFileDialog_h
#define emFileDialog_h

#ifndef emDialog_h
#include <emCore/emDialog.h>
#endif

#ifndef emFileSelectionBox_h
#include <emCore/emFileSelectionBox.h>
#endif


//==============================================================================
//================================ emFileDialog ================================
//==============================================================================

class emFileDialog : public emDialog {

public:

	// Class for a dialog which allows the user to choose a file (or
	// optionally multiple files). This is an emDialog with an
	// emFileSelectionBox as the content panel, and with OK and Cancel
	// buttons. Most of the public methods of emFileSelectionBox are copied
	// here with forwarding implementations. Additional features are for the
	// dialog aspects. Especially see ModeType:

	enum ModeType {
		// Data type for the general mode of the file dialog.

		M_SELECT,
			// The dialog is for selecting a file without popping up
			// any child dialogs like "file not existing" or "are
			// you sure to overwrite". The title is set to "Files",
			// and the button are "OK" and "Cancel"

		M_OPEN,
			// The dialog is for opening (reading) a file. If the
			// user tries to open a file that does not exist, an
			// error message is shown in a child dialog. The title
			// is set to "Open", and the buttons are "Open" and
			// "Cancel"

		M_SAVE
			// The dialog is for saving (writing) a file. If the
			// user tries to save a file which already exists, a
			// child dialog is shown which asks the user if he
			// really wants to overwrite that file. The title is set
			// to "Save As", and the buttons are "Save" and "Cancel"
	};

	emFileDialog(
		emContext & parentContext,
		ModeType mode=M_SELECT,
		ViewFlags viewFlags=VF_POPUP_ZOOM|VF_ROOT_SAME_TALLNESS,
		WindowFlags windowFlags=WF_MODAL,
		const emString & wmResName="emDialog"
	);
		// Constructor.
		// Arguments:
		//   parentContext - Parent context(/window/dialog) for this
		//                   dialog.
		//   mode          - Mode of the file dialog.
		//   viewFlags     - View features.
		//   windowFlags   - Window features.
		//   wmResName     - A resource name for the dialog. This is
		//                   reported to the window manager.

	virtual ~emFileDialog();
		// Destructor.

	ModeType GetMode() const;
		// Get the file dialog mode.

	void SetMode(ModeType mode);
		// Set the file dialog mode. This also sets the dialog title and
		// the button texts accordingly.

	bool IsDirectoryResultAllowed() const;
		// Ask whether it is allowed to open/save a directory.

	void SetDirectoryResultAllowed(bool dirAllowed=true);
		// Set whether it is allowed to open/save a directory. If false
		// (the default), pressing "OK" while a directory is selected
		// has the effect of entering that directory instead of
		// finishing the dialog.

	bool IsMultiSelectionEnabled() const;
		// Ask whether the user is allowed to select multiple entries.

	void SetMultiSelectionEnabled(bool enabled=true);
		// Set whether the user is allowed to select multiple entries.
		// The default is false.

	const emString & GetParentDirectory() const;
		// Get the absolute path of the parent directory.

	void SetParentDirectory(const emString & parentDirectory);
		// Set the parent directory.

	emString GetSelectedName() const;
		// Get the name of the selected entry. If nothing is selected,
		// an empty string is returned. If multiple entries are
		// selected, the first one is returned.

	const emArray<emString> & GetSelectedNames() const;
		// Get an array with the names of all the selected entries.

	void SetSelectedName(const emString & selectedName);
		// Select a single entry. An empty string means to clear the
		// selection.

	void SetSelectedNames(const emArray<emString> & selectedNames);
		// Select any number of entries.

	void ClearSelection();
		// Clear the selection.

	emString GetSelectedPath() const;
		// Get the absolute path of the selected entry. If nothing is
		// selected, the absolute path of the parent directory is
		// returned. If multiple entries are selected, the absolute path
		// of the first selected one is returned.

	void SetSelectedPath(const emString & selectedPath);
		// Change the parent directory and the selection in one call. If
		// the given path is a directory, the parent directory is set to
		// that directory, and the selection is cleared. Otherwise the
		// parent directory is set to the parent path of the given path,
		// and the selection is set to the final name in the given path.

	const emSignal & GetSelectionSignal() const;
		// Signaled when the selection or the parent directory has
		// changed.

	const emArray<emString> & GetFilters() const;
		// Get the file type filters. See SetFilters for more.

	void SetFilters(const emArray<emString> & filters);
		// Set the file type filters, from which the user can choose
		// one. The default is to have no filters, which means to show
		// all entries. If no filter is selected at the time of calling
		// SetFilters with a non-empty array, the first filter is
		// selected automatically. Each filter is a string of the form:
		//   <description>'('<pattern>[<separator><pattern>...]')'
		// With:
		//   <description> = Any text.
		//   <pattern>     = A pattern for the file name. The only supported
		//                   meta character is an asterisk ('*'). It
		//                   means to match any sequence (including the
		//                   empty sequence). Everything else is taken
		//                   literal, but case-insensitive.
		//   <separator>   = A space char, a semicolon, a comma, or a
		//                   vertical bar.
		// Examples:
		//   "All files (*)"
		//   "Targa files (*.tga)"
		//   "HTML files (*.htm *.html)"

	int GetSelectedFilterIndex() const;
		// Get the index of the selected filter, or -1 if none is
		// selected.

	void SetSelectedFilterIndex(int selectedFilterIndex);
		// Set the index of the selected filter. -1 means to select none.

	bool AreHiddenFilesShown() const;
		// Ask whether hidden files are shown.

	void SetHiddenFilesShown(bool hiddenFilesShown=true);
		// Set whether hidden files are shown. The default is false.

protected:

	virtual bool Cycle();
	virtual bool CheckFinish(int result);

private:

	emFileSelectionBox * Fsb;
	ModeType Mode;
	bool DirAllowed;

	emCrossPtr<emDialog> OverwriteDialog;
	emString OverwriteAsked;
	emString OverwriteConfirmed;
};

inline emFileDialog::ModeType emFileDialog::GetMode() const
{
	return Mode;
}

inline bool emFileDialog::IsDirectoryResultAllowed() const
{
	return DirAllowed;
}

inline bool emFileDialog::IsMultiSelectionEnabled() const
{
	return Fsb->IsMultiSelectionEnabled();
}

inline void emFileDialog::SetMultiSelectionEnabled(bool enabled)
{
	Fsb->SetMultiSelectionEnabled(enabled);
}

inline const emString & emFileDialog::GetParentDirectory() const
{
	return Fsb->GetParentDirectory();
}

inline void emFileDialog::SetParentDirectory(const emString & parentDirectory)
{
	Fsb->SetParentDirectory(parentDirectory);
}

inline emString emFileDialog::GetSelectedName() const
{
	return Fsb->GetSelectedName();
}

inline const emArray<emString> & emFileDialog::GetSelectedNames() const
{
	return Fsb->GetSelectedNames();
}

inline void emFileDialog::SetSelectedName(const emString & selectedName)
{
	Fsb->SetSelectedName(selectedName);
}

inline void emFileDialog::SetSelectedNames(const emArray<emString> & selectedNames)
{
	Fsb->SetSelectedNames(selectedNames);
}

inline void emFileDialog::ClearSelection()
{
	Fsb->ClearSelection();
}

inline emString emFileDialog::GetSelectedPath() const
{
	return Fsb->GetSelectedPath();
}

inline void emFileDialog::SetSelectedPath(const emString & selectedPath)
{
	Fsb->SetSelectedPath(selectedPath);
}

inline const emSignal & emFileDialog::GetSelectionSignal() const
{
	return Fsb->GetSelectionSignal();
}

inline const emArray<emString> & emFileDialog::GetFilters() const
{
	return Fsb->GetFilters();
}

inline void emFileDialog::SetFilters(const emArray<emString> & filters)
{
	Fsb->SetFilters(filters);
}

inline int emFileDialog::GetSelectedFilterIndex() const
{
	return Fsb->GetSelectedFilterIndex();
}

inline void emFileDialog::SetSelectedFilterIndex(int selectedFilterIndex)
{
	Fsb->SetSelectedFilterIndex(selectedFilterIndex);
}

inline bool emFileDialog::AreHiddenFilesShown() const
{
	return Fsb->AreHiddenFilesShown();
}

inline void emFileDialog::SetHiddenFilesShown(bool hiddenFilesShown)
{
	Fsb->SetHiddenFilesShown(hiddenFilesShown);
}


#endif
