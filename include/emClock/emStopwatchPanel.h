//------------------------------------------------------------------------------
// emStopwatchPanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emStopwatchPanel_h
#define emStopwatchPanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emClockFileModel_h
#include <emClock/emClockFileModel.h>
#endif


class emStopwatchPanel : public emFilePanel {

public:

	emStopwatchPanel(ParentArg parent, const emString & name,
	                 emClockFileModel * fileModel, emColor fgColor);

	virtual ~emStopwatchPanel();

	void SetFgColor(emColor fgColor);

	virtual emString GetTitle();

protected:

	virtual bool Cycle();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void UpdateTimeFieldAndButtons();

	emRef<emClockFileModel> FileModel;
	emColor FgColor;
	emTkTextField * TimeField;
	emTkButton * StartStopButton;
	emTkButton * ClearButton;
};


#endif
