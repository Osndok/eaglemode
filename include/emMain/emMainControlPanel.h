//------------------------------------------------------------------------------
// emMainControlPanel.h
//
// Copyright (C) 2014-2016 Oliver Hamann.
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

#ifndef emMainControlPanel_h
#define emMainControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emBookmarks_h
#include <emMain/emBookmarks.h>
#endif

#ifndef emMainConfig_h
#include <emMain/emMainConfig.h>
#endif

class emMainWindow;


class emMainControlPanel : public emLinearGroup {

public:

	emMainControlPanel(ParentArg parent, const emString & name,
	                   emMainWindow & mainWin, emView & contentView);

	virtual ~emMainControlPanel();

	virtual emString GetTitle() const;

protected:

	virtual bool Cycle();

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

private:

	void RecreateContentControlPanel();

	emMainWindow & MainWin;
	emView & ContentView;

	emPanel * ContentControlPanel;

	emRef<emMainConfig> MainConfig;
	emRef<emBookmarksModel> BookmarksModel;

	emButton * BtNewWindow;
	emCheckButton * BtFullscreen;
	emCheckButton * BtAutoHideControlView;
	emCheckButton * BtAutoHideSlider;
	emButton * BtReload;
	emButton * BtClose;
	emButton * BtQuit;
};


#endif
