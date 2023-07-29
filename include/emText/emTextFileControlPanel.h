//------------------------------------------------------------------------------
// emTextFileControlPanel.h
//
// Copyright (C) 2023 Oliver Hamann.
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

#ifndef emTextFileControlPanel_h
#define emTextFileControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emTextFilePanel_h
#include <emText/emTextFilePanel.h>
#endif


class emTextFileControlPanel : public emLinearGroup {

public:

	emTextFileControlPanel(ParentArg parent, const emString & name,
	                       emTextFilePanel & filePanel);
	~emTextFileControlPanel();

protected:

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void UpdateControls();

	emRef<emTextFileModel> FileModel;
	emCrossPtr<emTextFilePanel> FilePanel;

	emTextField * CharEncoding;
	emTextField * LineBreakEncoding;
	emTextField * NumberOfLines;
	emTextField * NumberOfColumns;
	emButton * Copy;
	emButton * SelectAll;
	emButton * ClearSelection;
};


#endif
