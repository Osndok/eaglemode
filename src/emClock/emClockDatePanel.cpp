//------------------------------------------------------------------------------
// emClockDatePanel.cpp
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

#include <emClock/emClockDatePanel.h>


emClockDatePanel::emClockDatePanel(
	ParentArg parent, const emString & name, emColor fgColor
)
	: emPanel(parent,name)
{
	FgColor=fgColor;
	Year=0;
	Month=0;
	Day=0;
	DayOfWeek=0;
	Hour=0;
	Minute=0;
	Second=0;
}


emClockDatePanel::~emClockDatePanel()
{
}


void emClockDatePanel::SetFgColor(emColor fgColor)
{
	if (FgColor!=fgColor) {
		FgColor=fgColor;
		InvalidatePainting();
	}
}


void emClockDatePanel::SetDate(
	int year, int month, int day, int dayOfWeek, int hour, int minute,
	int second
)
{
	if (Year!=year || Month!=month || Day!=day || DayOfWeek!=dayOfWeek ||
	    Hour!=hour || Minute!=minute || Second!=second) {
		Year=year;
		Month=month;
		Day=day;
		DayOfWeek=dayOfWeek;
		Hour=hour;
		Minute=minute;
		Second=second;
		InvalidatePainting();
	}
}


bool emClockDatePanel::IsOpaque()
{
	return false;
}


void emClockDatePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	static const char * weekDayNames[7]={
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	};
	char tmp[256];
	double h;

	h=GetHeight();

	painter.PaintRoundRectOutline(
		0.005,
		0.005,
		1.0-0.01,
		h-0.01,
		0.1,
		0.1,
		0.01,
		FgColor,
		canvasColor
	);

	painter.PaintTextBoxed(
		0.05,
		h*0.05,
		0.9,
		h*0.08,
		weekDayNames[DayOfWeek%7],
		h*0.08,
		FgColor,
		canvasColor
	);

	sprintf(tmp,"%d",Day);
	painter.PaintTextBoxed(
		0.02,
		h*0.11,
		0.94,
		h*0.83,
		tmp,
		h*0.83,
		FgColor,
		canvasColor
	);

	sprintf(
		tmp,
		"%04d-%02d-%02d %02d:%02d:%02d",
		Year,Month,Day,Hour,Minute,Second
	);
	painter.PaintTextBoxed(
		0.05,
		h*0.87,
		0.9,
		h*0.08,
		tmp,
		h*0.08,
		FgColor,
		canvasColor
	);
}
