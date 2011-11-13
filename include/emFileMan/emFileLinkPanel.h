//------------------------------------------------------------------------------
// emFileLinkPanel.h
//
// Copyright (C) 2007-2008,2010 Oliver Hamann.
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

#ifndef emFileLinkPanel_h
#define emFileLinkPanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emDirEntry_h
#include <emFileMan/emDirEntry.h>
#endif

#ifndef emFileLinkModel_h
#include <emFileMan/emFileLinkModel.h>
#endif

#ifndef emFileManViewConfig_h
#include <emFileMan/emFileManViewConfig.h>
#endif


class emFileLinkPanel : public emFilePanel {

public:

	emFileLinkPanel(ParentArg parent, const emString & name,
	                emFileLinkModel * fileModel);

	virtual ~emFileLinkPanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void CalcContentCoords(double * pX, double * pY, double * pW, double * pH);
	void UpdateChildPanel(bool forceRecreation=false);
	void LayoutChildPanel();

	bool HaveBorder;
	emFileLinkModel * Model;
	emRef<emSigModel> UpdateSignalModel;
	emRef<emFileManViewConfig> Config;
	emPanel * ChildPanel;
	emString CachedFullPath;
	emDirEntry DirEntry;
	bool DirEntryUpToDate;
	static const emColor BorderBgColor;
	static const emColor BorderFgColor;
};


#endif
