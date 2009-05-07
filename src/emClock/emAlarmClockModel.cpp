//------------------------------------------------------------------------------
// emAlarmClockModel.cpp
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

#include <emCore/emWindow.h>
#include <emClock/emAlarmClockModel.h>


emRef<emAlarmClockModel> emAlarmClockModel::Acquire(
	emView & view, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emAlarmClockModel,view,name,common)
}


void emAlarmClockModel::EnableAlarm(
	const emString & panelIdentity, int alarmSecOfDay, int preventAlarmSecs,
	int alarmDurationSecs, int beepIntervalMillisecs
)
{
	PanelIdentity=panelIdentity;
	AlarmSecOfDay=alarmSecOfDay;
	PreventAlarmSecs=preventAlarmSecs;
	AlarmDurationSecs=alarmDurationSecs;
	BeepIntervalMillisecs=beepIntervalMillisecs;
	AlarmEnabled=true;
	AlarmTrigger=false;
	Alarming=false;
	Signal(ChangeSignal);
	BeepTimer.Stop(true);
	VisitTimer.Stop(true);
	SetMinCommonLifetime(UINT_MAX);
}


void emAlarmClockModel::DisableAlarm()
{
	if (AlarmEnabled) {
		AlarmEnabled=false;
		AlarmTrigger=false;
		Alarming=false;
		Signal(ChangeSignal);
		BeepTimer.Stop(true);
		VisitTimer.Stop(true);
		SetMinCommonLifetime(0);
	}
}


void emAlarmClockModel::ConfirmAlarm()
{
	if (Alarming) {
		AlarmTrigger=false;
		Alarming=false;
		Signal(ChangeSignal);
		BeepTimer.Stop(true);
		VisitTimer.Stop(true);
	}
}


void emAlarmClockModel::Beep()
{
	emScreen * screen;

	screen=View->GetScreen();
	if (screen) screen->Beep();
}


emAlarmClockModel::emAlarmClockModel(emView & view, const emString & name)
	: emModel(view,name),
	BeepTimer(GetScheduler()),
	VisitTimer(GetScheduler())
{
	View=&view;
	TimeZonesModel=emTimeZonesModel::Acquire(GetRootContext());
	AlarmSecOfDay=0;
	PreventAlarmSecs=0;
	AlarmDurationSecs=0;
	BeepIntervalMillisecs=0;
	AlarmEnabled=false;
	AlarmTrigger=false;
	Alarming=false;
	AddWakeUpSignal(BeepTimer.GetSignal());
	AddWakeUpSignal(VisitTimer.GetSignal());
	AddWakeUpSignal(TimeZonesModel->GetTimeSignal());
}


emAlarmClockModel::~emAlarmClockModel()
{
}


bool emAlarmClockModel::Cycle()
{
	emWindow * window;
	int hour,minute,second,d;

	if (IsSignaled(BeepTimer.GetSignal())) {
		Beep();
	}

	if (IsSignaled(VisitTimer.GetSignal())) {
		window=View->GetWindow();
		if (window) window->Raise();
		View->Focus();
		View->SeekFullsized(PanelIdentity,true,"Alarm Clock");
	}

	if (IsSignaled(TimeZonesModel->GetTimeSignal())) {
		if (AlarmEnabled) {
			try {
				TimeZonesModel->TryGetZoneTime(
					emTimeZonesModel::LOCAL_ZONE_ID,
					NULL,NULL,NULL,NULL,
					&hour,&minute,&second
				);
			}
			catch (emString) {
				hour=minute=second=0;
			}
			d=hour*3600+minute*60+second-AlarmSecOfDay;
			while (d>43200) d-=86400;
			while (d<-43200) d+=86400;
			if (Alarming) {
				if (d<0 || d>=AlarmDurationSecs) ConfirmAlarm();
			}
			else if (d<-PreventAlarmSecs) {
				AlarmTrigger=true;
				PreventAlarmSecs=0;
			}
			else if (d>=0 && d<AlarmDurationSecs && AlarmTrigger) {
				Alarming=true;
				Beep();
				BeepTimer.Start(BeepIntervalMillisecs,true);
				VisitTimer.Start(800,false);
				Signal(ChangeSignal);
			}
		}
	}

	return false;
}
