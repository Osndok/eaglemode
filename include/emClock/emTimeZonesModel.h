//------------------------------------------------------------------------------
// emTimeZonesModel.h
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

#ifndef emTimeZonesModel_h
#define emTimeZonesModel_h

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif


class emTimeZonesModel : public emModel {

public:

	static emRef<emTimeZonesModel> Acquire(emRootContext & rootContext);

	typedef int ZoneId;

	enum {
		LOCAL_ZONE_ID = -1,
		UTC_ZONE_ID   = -2
	};

	int GetCityCount() const;
	emString GetCityName(int cityIndex) const;
	ZoneId GetCityZoneId(int cityIndex) const;
	double GetCityLatitude(int cityIndex) const;
	double GetCityLongitude(int cityIndex) const;

	const emSignal & GetTimeSignal() const;

	time_t GetTime() const;

	void TryGetZoneTime(
		ZoneId zoneId,
		int * pYear,
		int * pMonth,
		int * pDay,
		int * pDayOfWeek,
		int * pHour,
		int * pMinute,
		int * pSecond
	) throw(emString);

	void TryGetZoneTime(
		time_t time,
		ZoneId zoneId,
		int * pYear,
		int * pMonth,
		int * pDay,
		int * pDayOfWeek,
		int * pHour,
		int * pMinute,
		int * pSecond
	) throw(emString);

	double GetJulianDate();
	double GetJulianDate(time_t time);

protected:

	emTimeZonesModel(emContext & context, const emString & name);

	virtual ~emTimeZonesModel();

	virtual bool Cycle();

private:

	struct City {
		City();
		char CountryCode[4];
		double Latitude;
		double Longitude;
		emString Name;
		emString Comment;
		bool TzFileChecked;
		bool TzFileExists;
		bool TimeValid;
		bool TimeRequested;
		int TimeNeeded;
		int Year,Month,Day,DayOfWeek,Hour,Minute,Second;
	};

	void InitCities();

	void RequestCityTime(City * city);

	bool ReplyCityTimes();

	void ManageChildProc();

	static int CmpCityAndName(
		City * const * obj, void * key, void * context
	);

	emString ZoneInfoDir;
	emSignal TimeSignal;
	time_t Time;
	emArray<City*> Cities; // Sorted by name
	emList<City*> Requests;

	emProcess ChildProc;
	enum {
		CP_STOPPED,
		CP_RUNNING,
		CP_STOPPING,
		CP_ERRORED
	} ChildProcState;
	emString ChildProcError;
	emUInt64 ChildProcIdleClock;

	int ReadBufSize, WriteBufSize;
	int ReadBufFill, WriteBufFill;
	char * ReadBuf, * WriteBuf;
};

inline const emSignal & emTimeZonesModel::GetTimeSignal() const
{
	return TimeSignal;
}

inline time_t emTimeZonesModel::GetTime() const
{
	return Time;
}

inline void emTimeZonesModel::TryGetZoneTime(
	ZoneId zoneId, int * pYear, int * pMonth, int * pDay, int * pDayOfWeek,
	int * pHour, int * pMinute, int * pSecond
) throw(emString)
{
	TryGetZoneTime(
		Time,zoneId,pYear,pMonth,pDay,pDayOfWeek,pHour,pMinute,pSecond
	);
}

inline double emTimeZonesModel::GetJulianDate()
{
	return GetJulianDate(Time);
}


#endif
