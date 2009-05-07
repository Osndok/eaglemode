//------------------------------------------------------------------------------
// emClockPanel.h
//
// Copyright (C) 2006-2009 Oliver Hamann.
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

#ifndef emClockPanel_h
#define emClockPanel_h

#ifndef emTimeZonesModel_h
#include <emClock/emTimeZonesModel.h>
#endif

#ifndef emClockDatePanel_h
#include <emClock/emClockDatePanel.h>
#endif

#ifndef emStopwatchPanel_h
#include <emClock/emStopwatchPanel.h>
#endif

#ifndef emAlarmClockPanel_h
#include <emClock/emAlarmClockPanel.h>
#endif

#ifndef emWorldClockPanel_h
#include <emClock/emWorldClockPanel.h>
#endif

#ifndef emClockHandsPanel_h
#include <emClock/emClockHandsPanel.h>
#endif


class emClockPanel : public emFilePanel {

public:

	emClockPanel(
		ParentArg parent, const emString & name,
		emClockFileModel * fileModel,
		emTimeZonesModel::ZoneId zone=emTimeZonesModel::LOCAL_ZONE_ID
	);

	virtual ~emClockPanel();

	virtual emString GetTitle();

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void CreateOrDestroyChildren();
	void UpdateColors();
	void UpdateTime();

	emRef<emClockFileModel> FileModel;
	emRef<emTimeZonesModel> TimeZonesModel;

	emTimeZonesModel::ZoneId Zone;

	emClockDatePanel * DatePanel;
	emStopwatchPanel * StopwatchPanel;
	emAlarmClockPanel * AlarmClockPanel;
	emClockPanel * UTCPanel;
	emWorldClockPanel * WorldClockPanel;
	emClockHandsPanel * HandsPanel;

	emColor BorderColor;
	emColor BgColor;
	emColor FgColor;
	emColor HandsColor;

	double CenterX;
	double CenterY;
	double Radius;

	emString TimeError;
};


#endif
