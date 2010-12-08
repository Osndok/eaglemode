//------------------------------------------------------------------------------
// emFilePanel.h
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

#ifndef emFilePanel_h
#define emFilePanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif


//==============================================================================
//================================ emFilePanel =================================
//==============================================================================

class emFilePanel : public emPanel {

public:

	// Base class for a panel with which the user can view or edit a file
	// that is interfaced with an emFileModel. Internally, an object of this
	// class manages an emFileModelClient. The memory limit and priority of
	// that client is set and updated from the panel properties. In
	// addition, a virtual file state is provided (a more correct name would
	// be: "virtual file model state"). This is similar to the file state of
	// the model, but with some extensions (see GetVirFileState()). The
	// panel shows information about that virtual file state. A derived
	// class should overload the Paint method for showing the file contents
	// when the virtual file state is good (loaded or unsaved).

	emFilePanel(
		ParentArg parent, const emString & name,
		emFileModel * fileModel=NULL, bool updateFileModel=true
	);
		// Constructor.
		// Arguments:
		//   parent    - Parent for this panel (emPanel or emView).
		//   name      - The name for this panel.
		//   fileModel - See SetFileModel.
		//   updateFileModel - See SetFileModel.

	virtual ~emFilePanel();
		// Destructor.

	const emFileModel * GetFileModel() const;
	emFileModel * GetFileModel();
		// Get the file model, NULL if none.

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);
		// Set the file model, NULL for none. If updateFileModel==true,
		// fileModel->Update() is called.

	void SetCustomError(const emString & message);
	void ClearCustomError();
	emString GetCustomError();
		// Set, clear or get a custom error message. If set, the message
		// is shown by the default implementation of Paint.

	const emSignal & GetVirFileStateSignal() const;
		// Signaled when the virtual file state has changed.

	enum VirtualFileState {
		VFS_WAITING       = emFileModel::FS_WAITING,
		VFS_LOADING       = emFileModel::FS_LOADING,
		VFS_LOADED        = emFileModel::FS_LOADED,
		VFS_UNSAVED       = emFileModel::FS_UNSAVED,
		VFS_SAVING        = emFileModel::FS_SAVING,
		VFS_TOO_COSTLY    = emFileModel::FS_TOO_COSTLY,
		VFS_LOAD_ERROR    = emFileModel::FS_LOAD_ERROR,
		VFS_SAVE_ERROR    = emFileModel::FS_SAVE_ERROR,
		VFS_NO_FILE_MODEL = emFileModel::FS_MAX_VAL+1,
		VFS_CUSTOM_ERROR  = emFileModel::FS_MAX_VAL+2
	};
	VirtualFileState GetVirFileState() const;
		// Get the virtual file state. This is like the file state of
		// the model (GetFileMode()->GetFileState()), but:
		// - There is the additional state VFS_CUSTOM_ERROR. It is set
		//   when a custom error has been reported through
		//   SetCustomError(...).
		// - There is the additional state VFS_NO_FILE_MODEL. It is set
		//   when GetFileModel()==NULL.
		// - If the memory limit of this panel is smaller than the
		//   memory need of the model, the virtual file state is forced
		//   to VFS_TOO_COSTLY even when the model is in loaded state.
		//   (Otherwise the show state of the panel could depend on the
		//   show state of another panel - too ugly)

	bool IsVFSGood() const;
		// This is a short-cut for:
		//  (GetVirFileState()==VFS_LOADED ||
		//   GetVirFileState()==VFS_UNSAVED)
		// It means that the file model data can safely be shown and
		// modified.

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);
		// Paints some info about the virtual file state including any
		// error messages. Should be overloaded for showing the file
		// model data when IsVFSGood()==true. InvalidatePainting is
		// called automatically on each change of the virtual file
		// state.

	virtual bool IsHopeForSeeking();
		// Returns true if VFS_WAITING, VFS_LOADING or VFS_SAVING.

private:

	emFileModelClient FileModelClient;
	emString * CustomError;
	emSignal VirFileStateSignal;
};

inline const emFileModel * emFilePanel::GetFileModel() const
{
	return FileModelClient.GetModel();
}

inline emFileModel * emFilePanel::GetFileModel()
{
	return FileModelClient.GetModel();
}

inline const emSignal & emFilePanel::GetVirFileStateSignal() const
{
	return VirFileStateSignal;
}


#endif
