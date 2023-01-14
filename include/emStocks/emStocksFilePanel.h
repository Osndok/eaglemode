//------------------------------------------------------------------------------
// emStocksFilePanel.h
//
// Copyright (C) 2021-2022 Oliver Hamann.
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

#ifndef emStocksFilePanel_h
#define emStocksFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emStocksListBox_h
#include <emStocks/emStocksListBox.h>
#endif


class emStocksFilePanel : public emFilePanel {

public:

	emStocksFilePanel(ParentArg parent, const emString & name,
	                  emStocksFileModel * fileModel);

	virtual ~emStocksFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

	virtual emString GetIconFileName() const;

protected:

	virtual bool Cycle();

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void UpdateControls();

	emStocksFileModel * FileModel;
	emRef<emStocksConfig> Config;
	emStocksListBox * ListBox;
	emColor BgColor;
};


#endif
