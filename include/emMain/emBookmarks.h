//------------------------------------------------------------------------------
// emBookmarks.h
//
// Copyright (C) 2007-2008,2011,2014-2016 Oliver Hamann.
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

#ifndef emBookmarks_h
#define emBookmarks_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

class emBookmarkRec;


//==============================================================================
//=============================== emBookmarksRec ===============================
//==============================================================================

class emBookmarksRec : public emArrayRec {

public:

	// This is an emArrayRec where each element is an emUnionRec. And each
	// emUnionRec has either an emBookmarkRec or an emBookmarkGroupRec (both
	// are based on emBookmarkEntryRec). The indices to the variants are:
	enum Variants {
		BOOKMARK=0, // emBookmarkRec
		GROUP=1     // emBookmarkGroupRec
	};

	emBookmarksRec();
	emBookmarksRec(emStructRec * parent, const char * varIdentifier);
	virtual ~emBookmarksRec();

	void InsertNewBookmark(int index, emView * contentView);
	void InsertNewGroup(int index);
	void CopyToClipboard(int index, emClipboard & clipboard);
	void TryInsertFromClipboard(int index, emClipboard & clipboard) throw(emException);

	emBookmarkRec * SearchBookmarkByHotkey(const emInputHotkey & hotkey);

	emBookmarkRec * SearchStartLocation();
	void SetStartLocation(emBookmarkRec * rec);
	void ClearStartLocation();

	virtual const char * GetFormatName() const;

private:

	static emString BookmarkNameFromPanelTitle(const emString & title);
	static emRec * AllocateUnion();

};


//==============================================================================
//============================= emBookmarkEntryRec =============================
//==============================================================================

class emBookmarkEntryRec : public emStructRec {

public:

	emBookmarkEntryRec(
		const emColor & defaultBgColor,
		const emColor & defaultFgColor
	);
	virtual ~emBookmarkEntryRec();

	emStringRec Name;
	emStringRec Description;
	emStringRec Icon;
	emColorRec BgColor;
	emColorRec FgColor;

	emBookmarksRec * GetBookmarksRec();
	int GetIndexInBookmarksRec();

	void TryPasteColorsFromClipboard(emClipboard & clipboard) throw(emException);
};


//==============================================================================
//============================= emBookmarkGroupRec =============================
//==============================================================================

class emBookmarkGroupRec : public emBookmarkEntryRec {

public:

	emBookmarkGroupRec();
	virtual ~emBookmarkGroupRec();

	emBookmarksRec Bookmarks;
};


//==============================================================================
//=============================== emBookmarkRec ================================
//==============================================================================

class emBookmarkRec : public emBookmarkEntryRec {

public:

	emBookmarkRec();
	virtual ~emBookmarkRec();

	emStringRec Hotkey;
	emStringRec LocationIdentity;
	emDoubleRec LocationRelX;
	emDoubleRec LocationRelY;
	emDoubleRec LocationRelA;
	emBoolRec VisitAtProgramStart;
};


//==============================================================================
//============================== emBookmarksModel ==============================
//==============================================================================

class emBookmarksModel : public emConfigModel, public emBookmarksRec {

public:

	static emRef<emBookmarksModel> Acquire(emRootContext & rootContext);

	static emString GetDefaultIconDir();

	static emString GetNormalizedIconFileName(const emString & iconFile);

protected:

	emBookmarksModel(emContext & context, const emString & name);
	virtual ~emBookmarksModel();

private:

	void Update_0_90_0();
	void Update_0_91_0();
};


//==============================================================================
//========================== emBookmarkEntryAuxPanel ===========================
//==============================================================================

class emBookmarkEntryAuxPanel : public emLinearGroup, private emRecListener {

public:

	emBookmarkEntryAuxPanel(
		ParentArg parent, const emString & name, emView * contentView,
		emBookmarksModel * model, emBookmarkEntryRec * rec
	);
	virtual ~emBookmarkEntryAuxPanel();

	virtual void SetLook(const emLook & look, bool recursively=false);

protected:

	virtual void OnRecChanged();

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void Update();

	emCrossPtr<emView> ContentView;
	emRef<emBookmarksModel> Model;
	bool UpToDate;
	emButton * BtNewBookmarkBefore;
	emButton * BtNewGroupBefore;
	emButton * BtPasteBefore;
	emButton * BtCut;
	emButton * BtCopy;
	emTextField * TfName;
	emTextField * TfDescription;
	emFileSelectionBox * FlbIcon;
	emColorField * CfBgColor;
	emColorField * CfFgColor;
	emButton * BtPasteColors;
	emTextField * TfLocationIdentity;
	emTextField * TfLocationRelX;
	emTextField * TfLocationRelY;
	emTextField * TfLocationRelA;
	emButton * BtSetLocation;
	emTextField * TfHotkey;
	emRadioButton * RbVisitAtProgramStart;
	emButton * BtNewBookmarkAfter;
	emButton * BtNewGroupAfter;
	emButton * BtPasteAfter;
};


//==============================================================================
//============================ emBookmarksAuxPanel =============================
//==============================================================================

class emBookmarksAuxPanel : public emLinearGroup, private emRecListener {

public:

	emBookmarksAuxPanel(
		ParentArg parent, const emString & name, emView * contentView,
		emBookmarksModel * model, emBookmarksRec * rec
	);
	virtual ~emBookmarksAuxPanel();

	virtual void SetLook(const emLook & look, bool recursively=false);

protected:

	virtual void OnRecChanged();

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	emCrossPtr<emView> ContentView;
	emRef<emBookmarksModel> Model;

	emButton * BtNewBookmark;
	emButton * BtNewGroup;
	emButton * BtPaste;
};


//==============================================================================
//============================== emBookmarkButton ==============================
//==============================================================================

class emBookmarkButton : public emButton, private emRecListener {

public:

	emBookmarkButton(
		ParentArg parent, const emString & name, emView * contentView,
		emBookmarksModel * model, emBookmarkRec * rec
	);
	virtual ~emBookmarkButton();

	emBookmarkRec * GetRec();

	virtual void SetLook(const emLook & look, bool recursively=false);

protected:

	virtual void OnRecChanged();

	virtual bool Cycle();

	virtual void AutoExpand();

private:

	void Update();

	emRef<emBookmarksModel> Model;
	emCrossPtr<emView> ContentView;
	bool UpToDate;
};

inline emBookmarkRec * emBookmarkButton::GetRec()
{
	return (emBookmarkRec*)GetListenedRec();
}


//==============================================================================
//============================== emBookmarksPanel ==============================
//==============================================================================

class emBookmarksPanel : public emRasterGroup, private emRecListener {

public:

	emBookmarksPanel(
		ParentArg parent, const emString & name, emView * contentView,
		emBookmarksModel * model, emRec * rec=NULL
	);
		// The rec should be an emBookmarksRec or an emBookmarkGroupRec,
		// contained in the model. If rec is NULL, the root record of
		// the model is used.

	virtual ~emBookmarksPanel();

	emRec * GetRec();

	virtual void SetLook(const emLook & look, bool recursively=false);

protected:

	virtual void OnRecChanged();

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void Update();

	emRef<emBookmarksModel> Model;
	emCrossPtr<emView> ContentView;
	bool UpToDate;
	emRasterLayout * RasterLayout;
};

inline emRec * emBookmarksPanel::GetRec()
{
	return GetListenedRec();
}


#endif
