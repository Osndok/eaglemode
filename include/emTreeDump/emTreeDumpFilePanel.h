//------------------------------------------------------------------------------
// emTreeDumpFilePanel.h
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

#ifndef emTreeDumpFilePanel_h
#define emTreeDumpFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emTreeDumpFileModel_h
#include <emTreeDump/emTreeDumpFileModel.h>
#endif

#ifndef emTreeDumpRecPanel_h
#include <emTreeDump/emTreeDumpRecPanel.h>
#endif


class emTreeDumpFilePanel : public emFilePanel {

public:

	emTreeDumpFilePanel(ParentArg parent, const emString & name,
	                    emTreeDumpFileModel * fileModel);
	virtual ~emTreeDumpFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

protected:

	virtual bool Cycle();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void UpdateRecPanel();

	emTreeDumpFileModel * FileModel;
	emTreeDumpRecPanel * RecPanel;
};


#endif
