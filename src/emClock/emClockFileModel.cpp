//------------------------------------------------------------------------------
// emClockFileModel.cpp
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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

#if defined(_WIN32)
#	include <sys/timeb.h>
#else
#	include <sys/time.h>
#endif
#include <emClock/emClockFileModel.h>


emRef<emClockFileModel> emClockFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emClockFileModel,context,name,common)
}


const char * emClockFileModel::GetFormatName() const
{
	return "emClock";
}


bool emClockFileModel::IsStopwatchRunning() const
{
	return StopwatchRunning;
}


emInt64 emClockFileModel::GetStopwatchTimeMS() const
{
	emInt64 t;

	t=GetStopwatchState();
	if (StopwatchRunning) t=GetTimeMS()-t;
	return t;
}


void emClockFileModel::StartStopwatch()
{
	if (!StopwatchRunning) {
		StopwatchRunning=true;
		SetStopwatchState(GetTimeMS()-GetStopwatchState());
	}
}


void emClockFileModel::StopStopwatch()
{
	if (StopwatchRunning) {
		StopwatchRunning=false;
		SetStopwatchState(GetTimeMS()-GetStopwatchState());
	}
}


void emClockFileModel::ClearStopwatch()
{
	StopwatchRunning=false;
	SetStopwatchState(0);
}


emInt64 emClockFileModel::GetStopwatchState() const
{
	const char * str;
	emInt64 t;

	str=StopwatchState.Get().Get();
	if (emStrToInt64(str,strlen(str),&t)<=0) t=0;
	return t;
}


void emClockFileModel::SetStopwatchState(emInt64 state)
{
	char str[64];
	int l;

	l=emInt64ToStr(str,sizeof(str),state);
	str[l]=0;
	StopwatchState=str;
}


emClockFileModel::emClockFileModel(emContext & context, const emString & name)
	: emRecFileModel(context,name),
	emStructRec(),
	ClockBorderColor(this,"ClockBorderColor",emColor(187,170,102),true),
	ClockBackgroundColor(this,"ClockBackgroundColor",emColor(221,221,221),true),
	ClockForegroundColor(this,"ClockForegroundColor",emColor(17,17,17),true),
	ClockHandsColor(this,"ClockHandsColor",emColor(0,0,0),true),
	UTCClockBorderColor(this,"UTCClockBorderColor",emColor(102,102,102),true),
	UTCClockBackgroundColor(this,"UTCClockBackgroundColor",emColor(204,204,204),true),
	UTCClockForegroundColor(this,"UTCClockForegroundColor",emColor(68,34,17),true),
	UTCClockHandsColor(this,"UTCClockHandsColor",emColor(51,34,34),true),
	WorldClockBorderColor(this,"WorldClockBorderColor",emColor(221,221,153),true),
	WorldClockBackgroundColor(this,"WorldClockBackgroundColor",emColor(221,221,221,160),true),
	WorldClockForegroundColor(this,"WorldClockForegroundColor",emColor(17,17,17),true),
	WorldClockHandsColor(this,"WorldClockHandsColor",emColor(0,0,0),true),
	WorldClockMinRadius(this,"WorldClockMinRadius",1.0,0.01,100.0),
	WorldClockMaxRadius(this,"WorldClockMaxRadius",0.1,0.01,100.0),
	WorldWaterColor(this,"WorldWaterColor",emColor(102,102,204),true),
	WorldLandColor(this,"WorldLandColor",emColor(136,187,0),true),
	WorldShadowColor(this,"WorldShadowColor",emColor(0,0,51,128),true),
	AlarmHour(this,"AlarmHour",0),
	AlarmMinute(this,"AlarmMinute",0),
	AlarmSecond(this,"AlarmSecond",0),
	StopwatchRunning(this,"StopwatchRunning",false),
	StopwatchState(this,"StopwatchState","0")
{
	TkLook.SetBgColor      (0xAAAAAAFF);
	TkLook.SetFgColor      (0x000000FF);
	TkLook.SetButtonBgColor(0xCCCCCCFF);
	TkLook.SetButtonFgColor(0x000000FF);
	TkLook.SetInputBgColor (0xFFFFFFFF);
	TkLook.SetInputFgColor (0x000000FF);
	TkLook.SetInputHlColor (0x0033BBFF);
	TkLook.SetOutputBgColor(0xBBBBBBFF);
	TkLook.SetOutputFgColor(0x000000FF);
	TkLook.SetOutputHlColor(0x0033BBFF);
	PostConstruct(*this);
}


emClockFileModel::~emClockFileModel()
{
}


emInt64 emClockFileModel::GetTimeMS()
{
#if defined(_WIN32)
	struct timeb tb;

	memset(&tb,0,sizeof(tb));
	ftime(&tb);
	return ((emInt64)tb.time)*1000+(emInt64)tb.millitm;
#else
	struct timeval tv;
	struct timezone tz;

	memset(&tv,0,sizeof(tv));
	memset(&tz,0,sizeof(tz));
	gettimeofday(&tv,&tz);
	return ((emInt64)tv.tv_sec)*1000+((emInt64)tv.tv_usec+500)/1000;
#endif
}
