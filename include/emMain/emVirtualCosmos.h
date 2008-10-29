//------------------------------------------------------------------------------
// emVirtualCosmos.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef emVirtualCosmos_h
#define emVirtualCosmos_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//=========================== emVirtualCosmosItemRec ===========================
//==============================================================================

class emVirtualCosmosItemRec : public emStructRec {

public:

	emVirtualCosmosItemRec(const emString & name);
	virtual ~emVirtualCosmosItemRec();

	const emString & GetName();

	virtual const char * GetFormatName() const;

	emStringRec Title;

	emDoubleRec PosX;
	emDoubleRec PosY;
	emDoubleRec Width;
	emDoubleRec ContentTallness;

	emDoubleRec BorderScaling;

	emColorRec BackgroundColor;
	emColorRec BorderColor;
	emColorRec TitleColor;

	emBoolRec Focusable;

	emStringRec FileName;
	emBoolRec CopyToUser;
	emIntRec Alternative;

	void TryPrepareItemFile(
		const emString & origDir, const emString & userDir
	) throw(emString);
		// Sets the item file path, and if CopyToUser is true, that copy
		// is made if necessary. This is called by emVirtualCosmosModel
		// when loading.

	const emString & GetItemFilePath();
		// Get the path of the file to be shown for this item. This is
		// not valid before TryPrepareItemFile has been called.

private:
	emString Name;
	emString ItemFilePath;
};

inline const emString & emVirtualCosmosItemRec::GetName()
{
	return Name;
}

inline const emString & emVirtualCosmosItemRec::GetItemFilePath()
{
	return ItemFilePath;
}


//==============================================================================
//============================ emVirtualCosmosModel ============================
//==============================================================================

class emVirtualCosmosModel : public emModel {

public:

	static emRef<emVirtualCosmosModel> Acquire(emRootContext & rootContext);

	const emArray<emVirtualCosmosItemRec*> & GeItemRecs() const;

	const emSignal & GetChangeSignal() const;

protected:

	emVirtualCosmosModel(emContext & context, const emString & name);
	virtual ~emVirtualCosmosModel();

	virtual bool Cycle();

private:

	struct Item {
		emString FileName;
		time_t MTime;
		emVirtualCosmosItemRec * ItemRec;
	};

	void Reload();

	static int CompareItemRecs(
		emVirtualCosmosItemRec * const * pItemRec1,
		emVirtualCosmosItemRec * const * pItemRec2,
		void * model
	);

	emRef<emSigModel> FileUpdateSignalModel;
	emSignal ChangeSignal;
	emString ItemsDir;
	emString ItemFilesDir;
	emArray<Item> Items; // Sorted by file name.
	emArray<emVirtualCosmosItemRec*> ItemRecs; // Sorted by position.
};

inline const emArray<emVirtualCosmosItemRec*> & emVirtualCosmosModel::GeItemRecs() const
{
	return ItemRecs;
}

inline const emSignal & emVirtualCosmosModel::GetChangeSignal() const
{
	return ChangeSignal;
}


//==============================================================================
//========================== emVirtualCosmosItemPanel ==========================
//==============================================================================

class emVirtualCosmosItemPanel : public emPanel, private emRecListener {

public:

	emVirtualCosmosItemPanel(ParentArg parent, const emString & name);
	virtual ~emVirtualCosmosItemPanel();

	emVirtualCosmosItemRec * GetItemRec();
	void SetItemRec(emVirtualCosmosItemRec * itemRec);

	virtual emString GetTitle();

protected:

	virtual bool Cycle();
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual void AutoExpand();
	virtual void AutoShrink();
	virtual void LayoutChildren();

	virtual void OnRecChanged();

private:

	void CalcBorders(double * pL, double * pT, double * pR, double * pB);
	void UpdateFromRec();
	void LayoutContentPanel();

	emPanel * ContentPanel;
	emString Path;
	int Alt;
	bool ItemFocusable;
	bool UpdateFromRecNeeded;
	emImage OuterBorderImage, InnerBorderImage;
};

inline emVirtualCosmosItemRec * emVirtualCosmosItemPanel::GetItemRec()
{
	return (emVirtualCosmosItemRec*)GetListenedRec();
}


//==============================================================================
//============================ emVirtualCosmosPanel ============================
//==============================================================================

class emVirtualCosmosPanel : public emPanel {

public:

	emVirtualCosmosPanel(ParentArg parent, const emString & name);
	virtual ~emVirtualCosmosPanel();

	virtual emString GetTitle();

protected:

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual bool IsOpaque();

private:

	void UpdateChildren();

	emRef<emVirtualCosmosModel> Model;
	emPanel * BackgroundPanel;
};


#endif
