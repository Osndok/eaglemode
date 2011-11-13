//------------------------------------------------------------------------------
// emTreeDumpControlPanel.h
//
// Copyright (C) 2011 Oliver Hamann.
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

#ifndef emTreeDumpControlPanel_h
#define emTreeDumpControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emTreeDumpRec_h
#include <emTreeDump/emTreeDumpRec.h>
#endif


class emTreeDumpControlPanel : public emTkGroup {

public:

	emTreeDumpControlPanel(
		ParentArg parent, const emString & name, emView & contentView,
		emTreeDumpRec * rec,  const emString & dir
	);

	virtual ~emTreeDumpControlPanel();

	// Hint: The rec is referred by an emCrossPtr. So it's okay to delete
	// the rec before the panel.

protected:

	virtual bool Cycle();

	virtual void AutoExpand();

private:

	void RunCommand(int index);

	emView & ContentView;
	emCrossPtr<emTreeDumpRec> Rec;
	emString Dir;
};


#endif
