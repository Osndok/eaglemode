//------------------------------------------------------------------------------
// emClockDatePanel.h
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

#ifndef emClockDatePanel_h
#define emClockDatePanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


class emClockDatePanel : public emPanel {

public:

	emClockDatePanel(ParentArg parent, const emString & name,
	                 emColor fgColor);

	virtual ~emClockDatePanel();

	void SetFgColor(emColor fgColor);

	void SetDate(int year, int month, int day, int dayOfWeek,
	             int hour, int minute, int second);

protected:

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	emColor FgColor;
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};


#endif
