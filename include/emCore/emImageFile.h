//------------------------------------------------------------------------------
// emImageFile.h
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#ifndef emImageFile_h
#define emImageFile_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif


//==============================================================================
//============================== emImageFileModel ==============================
//==============================================================================

class emImageFileModel : public emFileModel {

public:

	// Abstract base class for an emFileModel on an image file. Derived
	// classes still have to overload and implement the file model methods
	// for loading and saving. If one of the set methods below is called,
	// the state of the file model is set to FS_UNSAVED. Remember, if the
	// state was FS_SAVING, the file will be corrupted, because the saving
	// is aborted.

	const emImage & GetImage() const;
	void SetImage(const emImage & image);
		// The image.

	const emString & GetComment() const;
	void SetComment(const emString & comment);
		// A comment on the image, stored in the file. May not be
		// supported by every derived class.

	const emString & GetFileFormatInfo() const;
	void SetFileFormatInfo(const emString & fileFormatInfo);
		// User-readable Information about the file format. Setting this
		// may not be useful.

	const emSignal & GetChangeSignal() const;
		// Signaled on every change of the image, the comment or the
		// file format info.

protected:

	emImageFileModel(emContext & context, const emString & name);
	virtual ~emImageFileModel();

	virtual void ResetData();
		// Overloaded from emFileModel. This clears the image, the
		// comment and the file format info, and it fires the signal.

	emImage Image;
	emString Comment;
	emString FileFormatInfo;
		// To be modified directly by the implementation of
		// TryStartLoading and/or TryContinueLoading (do not call the
		// set methods therein, because they set the file state to
		// FS_UNSAVED).

	emSignal ChangeSignal;
		// To be signaled by the implementation of TryStartLoading
		// and/or TryContinueLoading.
};

inline const emImage & emImageFileModel::GetImage() const
{
	return Image;
}

inline const emString & emImageFileModel::GetComment() const
{
	return Comment;
}

inline const emString & emImageFileModel::GetFileFormatInfo() const
{
	return FileFormatInfo;
}

inline const emSignal & emImageFileModel::GetChangeSignal() const
{
	return ChangeSignal;
}


//==============================================================================
//============================== emImageFilePanel ==============================
//==============================================================================

class emImageFilePanel : public emFilePanel {

public:

	// Panel for showing an emImageFileModel.

	emImageFilePanel(ParentArg parent, const emString & name,
	                 emImageFileModel * fileModel=NULL,
	                 bool updateFileModel=true);

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);
};


#endif
