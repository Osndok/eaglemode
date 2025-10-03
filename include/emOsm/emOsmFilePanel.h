//------------------------------------------------------------------------------
// emOsmFilePanel.h
//
// Copyright (C) 2012-2016,2024 Oliver Hamann.
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

#ifndef emOsmFilePanel_h
#define emOsmFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emOsmFileModel_h
#include <emOsm/emOsmFileModel.h>
#endif

#ifndef emOsmTilePanel_h
#include <emOsm/emOsmTilePanel.h>
#endif


class emOsmFilePanel : public emFilePanel {

public:

	emOsmFilePanel(ParentArg parent, const emString & name,
	               emOsmFileModel * fileModel);

	virtual ~emOsmFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

protected:

	virtual bool Cycle();

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void UpdateTilePanel();

	emOsmFileModel * FileModel;
	emOsmTilePanel * TilePanel;
};


#endif
