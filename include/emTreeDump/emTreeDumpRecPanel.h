//------------------------------------------------------------------------------
// emTreeDumpRecPanel.h
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#ifndef emTreeDumpRecPanel_h
#define emTreeDumpRecPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emTreeDumpRec_h
#include <emTreeDump/emTreeDumpRec.h>
#endif


class emTreeDumpRecPanel : public emPanel {

public:

	emTreeDumpRecPanel(ParentArg parent, const emString & name,
	                   emTreeDumpRec * rec,  const emString & dir);
	virtual ~emTreeDumpRecPanel();

	// Hint: The rec is referred by an emCrossPtr. So it's okay to delete
	// the rec before the panel.

	static double GetBestHeight();

protected:

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void AutoExpand();

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	emCrossPtr<emTreeDumpRec> Rec;
	emString Dir;
	emColor BgColor;
};


#endif
