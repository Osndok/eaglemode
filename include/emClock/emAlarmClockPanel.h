//------------------------------------------------------------------------------
// emAlarmClockPanel.h
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

#ifndef emAlarmClockPanel_h
#define emAlarmClockPanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emAlarmClockModel_h
#include <emClock/emAlarmClockModel.h>
#endif

#ifndef emClockFileModel_h
#include <emClock/emClockFileModel.h>
#endif


class emAlarmClockPanel : public emFilePanel {

public:

	emAlarmClockPanel(ParentArg parent, const emString & name,
	                  emClockFileModel * fileModel, emColor fgColor);

	virtual ~emAlarmClockPanel();

	void SetFgColor(emColor fgColor);

	virtual emString GetTitle();

protected:

	virtual bool Cycle();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void UpdateFieldsAndButtons();

	static void TimeFieldTextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);

	emRef<emClockFileModel> FileModel;
	emRef<emAlarmClockModel> AlarmModel;

	emColor FgColor;
	emTkScalarField * TimeField;
	emTkRadioButton * OnButton;
	emTkRadioButton * OffButton;
	emTkButton * TestButton;
	emTkButton * ConfirmButton;
};


#endif
