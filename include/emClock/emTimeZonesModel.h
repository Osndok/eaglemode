//------------------------------------------------------------------------------
// emTimeZonesModel.h
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

#ifndef emTimeZonesModel_h
#define emTimeZonesModel_h

#ifndef emModel_h
#include <emCore/emModel.h>
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

	bool GetZoneTime(
		ZoneId zoneId,
		int * pYear,
		int * pMonth,
		int * pDay,
		int * pDayOfWeek,
		int * pHour,
		int * pMinute,
		int * pSecond
	);

	bool GetZoneTime(
		time_t time,
		ZoneId zoneId,
		int * pYear,
		int * pMonth,
		int * pDay,
		int * pDayOfWeek,
		int * pHour,
		int * pMinute,
		int * pSecond
	);

	double GetJulianDate();
	double GetJulianDate(time_t time);

protected:

	emTimeZonesModel(emContext & context, const emString & name);

	virtual ~emTimeZonesModel();

	virtual bool Cycle();

private:

	struct City {
		char CountryCode[4];
		double Latitude;
		double Longitude;
		emString Name;
		emString Comment;
		bool TzFileChecked;
		bool TzFileExists;
	};

	void InitCities();

	static int CmpCityAndName(
		City * const * obj, void * key, void * context
	);

	emString ZoneInfoDir;
	emSignal TimeSignal;
	time_t Time;
	emArray<City*> Cities; // Sorted by name
};

inline const emSignal & emTimeZonesModel::GetTimeSignal() const
{
	return TimeSignal;
}

inline time_t emTimeZonesModel::GetTime() const
{
	return Time;
}

inline bool emTimeZonesModel::GetZoneTime(
	ZoneId zoneId, int * pYear, int * pMonth, int * pDay, int * pDayOfWeek,
	int * pHour, int * pMinute, int * pSecond
)
{
	return GetZoneTime(
		Time,zoneId,pYear,pMonth,pDay,pDayOfWeek,pHour,pMinute,pSecond
	);
}

inline double emTimeZonesModel::GetJulianDate()
{
	return GetJulianDate(Time);
}


#endif
