//------------------------------------------------------------------------------
// emAlarmClockModel.h
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

#ifndef emAlarmClockModel_h
#define emAlarmClockModel_h

#ifndef emView_h
#include <emCore/emView.h>
#endif

#ifndef emTimeZonesModel_h
#include <emClock/emTimeZonesModel.h>
#endif


class emAlarmClockModel : public emModel {

public:

	static emRef<emAlarmClockModel> Acquire(
		emView & view, const emString & name, bool common=true
	);

	void EnableAlarm(
		const emString & panelIdentity,
		int alarmSecOfDay,
		int preventAlarmSecs = 3,
		int alarmDurationSecs = 1800,
		int beepIntervalMillisecs = 500
	);

	void DisableAlarm();

	void ConfirmAlarm();

	void Beep();

	int GetAlarmSecOfDay() const;

	bool IsAlarmEnabled() const;

	bool IsAlarming() const;

	const emSignal & GetChangeSignal() const;

protected:

	emAlarmClockModel(emView & view, const emString & name);

	virtual ~emAlarmClockModel();

	virtual bool Cycle();

private:

	emTimer BeepTimer;
	emTimer VisitTimer;
	emView * View;
	emRef<emTimeZonesModel> TimeZonesModel;
	emSignal ChangeSignal;
	emString PanelIdentity;
	int AlarmSecOfDay;
	int PreventAlarmSecs;
	int AlarmDurationSecs;
	int BeepIntervalMillisecs;
	bool AlarmEnabled;
	bool AlarmTrigger;
	bool Alarming;
};

inline int emAlarmClockModel::GetAlarmSecOfDay() const
{
	return AlarmSecOfDay;
}

inline bool emAlarmClockModel::IsAlarmEnabled() const
{
	return AlarmEnabled;
}

inline bool emAlarmClockModel::IsAlarming() const
{
	return Alarming;
}

inline const emSignal & emAlarmClockModel::GetChangeSignal() const
{
	return ChangeSignal;
}


#endif
