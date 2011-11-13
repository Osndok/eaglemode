//------------------------------------------------------------------------------
// emClockFileModel.h
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

#ifndef emClockFileModel_h
#define emClockFileModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emClockFileModel : public emRecFileModel, public emStructRec
{
public:

	static emRef<emClockFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	virtual const char * GetFormatName() const;

	emColorRec ClockBorderColor;
	emColorRec ClockBackgroundColor;
	emColorRec ClockForegroundColor;
	emColorRec ClockHandsColor;

	emColorRec UTCClockBorderColor;
	emColorRec UTCClockBackgroundColor;
	emColorRec UTCClockForegroundColor;
	emColorRec UTCClockHandsColor;

	emColorRec WorldClockBorderColor;
	emColorRec WorldClockBackgroundColor;
	emColorRec WorldClockForegroundColor;
	emColorRec WorldClockHandsColor;

	emDoubleRec WorldClockMinRadius;
	emDoubleRec WorldClockMaxRadius;

	emColorRec WorldWaterColor;
	emColorRec WorldLandColor;
	emColorRec WorldShadowColor;

	emIntRec AlarmHour;
	emIntRec AlarmMinute;
	emIntRec AlarmSecond;

	emBoolRec StopwatchRunning;
	emStringRec StopwatchState;

	emTkLook TkLook;

	bool IsStopwatchRunning() const;
	emInt64 GetStopwatchTimeMS() const;
	void StartStopwatch();
	void StopStopwatch();
	void ClearStopwatch();
	emInt64 GetStopwatchState() const;
	void SetStopwatchState(emInt64 state);

protected:

	emClockFileModel(emContext & context, const emString & name);
	virtual ~emClockFileModel();

private:

	static emInt64 GetTimeMS();
};


#endif
