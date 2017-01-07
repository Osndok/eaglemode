//------------------------------------------------------------------------------
// emFileSelectionBox.h
//
// Copyright (C) 2015-2016 Oliver Hamann.
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

#ifndef emFileSelectionBox_h
#define emFileSelectionBox_h

#ifndef emCheckBox_h
#include <emCore/emCheckBox.h>
#endif

#ifndef emListBox_h
#include <emCore/emListBox.h>
#endif

#ifndef emTextField_h
#include <emCore/emTextField.h>
#endif


//==============================================================================
//============================= emFileSelectionBox =============================
//==============================================================================

class emFileSelectionBox : public emBorder {

public:

	// Class for a file selection box. This allows the user to walk through
	// the file system, view the contents of a directory, and select one or
	// optionally more files in it. He may also enter a new file name. The
	// file selection box is an emBorder whith the following child panels:
	//  - An editable text field for the path of the directory.
	//  - A check box for switching the showing of hidden files on and off.
	//  - A list box which shows the contents of the directory. It has one
	//    item for each file or sub-directory, and the item ".." for the
	//    parent directory. Each item shows the name and an icon (if it's a
	//    directory) or preview (if it's a file).
	//  - An editable text field for the file name.
	//  - A list box for file type filters.

	emFileSelectionBox(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.

	virtual ~emFileSelectionBox();
		// Destructor.

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

	const emSignal & GetFileTriggerSignal() const;
		// This signal is signaled when the user double-clicks an item
		// or presses enter in the name field - but only if it is not a
		// directory. (If it is a directory, the parent directory is
		// changed instead of sending the signal). The associated name
		// can be get with GetTriggeredFileName().

	const emString & GetTriggeredFileName() const;
		// Get the name of the file which was triggered by a
		// double-click or enter key press.

	void TriggerFile(const emString & name);
		// Trigger a file name programmatically.

	void EnterSubDir(const emString & name);
		// Enter a sub-directory programmatically. This is like
		// SetParentDirectory, but the given string is the name of a
		// sub-directory instead of a path, and some checks are
		// performed whether it is really a directory and whether it is
		// readable.

	bool IsParentDirFieldHidden() const;
		// Ask whether the text field for the parent directory is
		// hidden.

	void SetParentDirFieldHidden(bool parentDirFieldHidden=true);
		// Set whether to hide the text field for the parent directory.
		// The default is false.

	bool IsHiddenCheckBoxHidden() const;
		// Ask whether the "show hidden files" check box is hidden.

	void SetHiddenCheckBoxHidden(bool hiddenCheckBoxHidden=true);
		// Set whether to hide the "show hidden files" check box. The
		// default is false.

	bool IsNameFieldHidden() const;
		// Ask whether the text field for the name is hidden.

	void SetNameFieldHidden(bool nameFieldHidden=true);
		// Set whether to hide the text field for the name. The default
		// is false.

	bool IsFilterHidden() const;
		// Ask whether the list box for the filters is hidden.

	void SetFilterHidden(bool filterHidden=true);
		// Set whether to hide the list box for the filters. The default
		// is false.

protected:

	virtual bool Cycle();
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual void AutoExpand();
	virtual void AutoShrink();
	virtual void LayoutChildren();

private:

	void InvalidateListing();
	void ReloadListing();
	void SelectionToListBox();
	void SelectionFromListBox();

	static int CompareNames(
		const emString * name1, const emString * name2, void * context
	);

	static bool MatchFileNameFilter(const char * fileName, const char * filter);
	static bool MatchFileNamePattern(const char * fileName, const char * pattern,
	                                 const char * patternEnd);

	class FilesListBox;
	class FileItemPanel;
	class FileOverlayPanel;
	friend class FilesListBox;
	friend class FileItemPanel;

	class FilesListBox : public emListBox {
	public:
		FilesListBox(emFileSelectionBox & parent, const emString & name);
		virtual ~FilesListBox();
		emFileSelectionBox & GetFileSelectionBox() const;
		const emImage & GetDirImage() const;
		const emImage & GetDirUpImage() const;
	protected:
		virtual void CreateItemPanel(const emString & name, int itemIndex);
	};

	class FileItemPanel : public emPanel, public emListBox::ItemPanelInterface {
	public:
		FileItemPanel(FilesListBox & listBox, const emString & name, int itemIndex);
		virtual ~FileItemPanel();
	protected:
		virtual void Notice(NoticeFlags flags);
		virtual void Input(emInputEvent & event, const emInputState & state,
		                   double mx, double my);
		virtual bool IsOpaque() const;
		virtual void Paint(const emPainter & painter, emColor canvasColor) const;
		virtual void AutoExpand();
		virtual void AutoShrink();
		virtual void LayoutChildren();
		virtual void ItemTextChanged();
		virtual void ItemDataChanged();
		virtual void ItemSelectionChanged();
	private:
		emColor GetBgColor() const;
		emColor GetFgColor() const;
		friend class FileOverlayPanel;
		FilesListBox & ListBox;
		emPanel * FilePanel;
		emPanel * OverlayPanel;
	};

	class FileOverlayPanel : public emPanel {
	public:
		FileOverlayPanel(FileItemPanel & parent, const emString & name);
		virtual ~FileOverlayPanel();
	protected:
		virtual void Input(emInputEvent & event, const emInputState & state,
		                   double mx, double my);
	};

	struct FileItemData {
		bool IsDirectory;
		bool IsReadable;
		bool IsHidden;
	};

	bool MultiSelectionEnabled;
	emString ParentDir;
	emArray<emString> SelectedNames;
	emSignal SelectionSignal;
	emArray<emString> Filters;
	int SelectedFilterIndex;
	bool HiddenFilesShown;

	emSignal FileTriggerSignal;
	emString TriggeredFileName;

	bool ParentDirFieldHidden;
	bool HiddenCheckBoxHidden;
	bool NameFieldHidden;
	bool FilterHidden;

	emTextField * ParentDirField;
	emCheckBox * HiddenCheckBox;
	FilesListBox * FilesLB;
	emTextField * NameField;
	emListBox * FiltersLB;

	bool ListingInvalid;
};

inline bool emFileSelectionBox::IsMultiSelectionEnabled() const
{
	return MultiSelectionEnabled;
}

inline const emString & emFileSelectionBox::GetParentDirectory() const
{
	return ParentDir;
}

inline const emArray<emString> & emFileSelectionBox::GetSelectedNames() const
{
	return SelectedNames;
}

inline const emSignal & emFileSelectionBox::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline const emArray<emString> & emFileSelectionBox::GetFilters() const
{
	return Filters;
}

inline int emFileSelectionBox::GetSelectedFilterIndex() const
{
	return SelectedFilterIndex;
}

inline bool emFileSelectionBox::AreHiddenFilesShown() const
{
	return HiddenFilesShown;
}

inline const emSignal & emFileSelectionBox::GetFileTriggerSignal() const
{
	return FileTriggerSignal;
}

inline const emString & emFileSelectionBox::GetTriggeredFileName() const
{
	return TriggeredFileName;
}

inline bool emFileSelectionBox::IsParentDirFieldHidden() const
{
	return ParentDirFieldHidden;
}

inline bool emFileSelectionBox::IsHiddenCheckBoxHidden() const
{
	return HiddenCheckBoxHidden;
}

inline bool emFileSelectionBox::IsNameFieldHidden() const
{
	return NameFieldHidden;
}

inline bool emFileSelectionBox::IsFilterHidden() const
{
	return FilterHidden;
}

inline emFileSelectionBox & emFileSelectionBox::FilesListBox::GetFileSelectionBox() const
{
	return *((emFileSelectionBox*)GetParent());
}

inline const emImage & emFileSelectionBox::FilesListBox::GetDirImage() const
{
	return GetTkResources().ImgDir;
}

inline const emImage & emFileSelectionBox::FilesListBox::GetDirUpImage() const
{
	return GetTkResources().ImgDirUp;
}


#endif
