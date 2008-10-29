//------------------------------------------------------------------------------
// emTimeZonesModel.cpp
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

#include <emClock/emTimeZonesModel.h>


emRef<emTimeZonesModel> emTimeZonesModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emTimeZonesModel,rootContext,"")
}


int emTimeZonesModel::GetCityCount() const
{
	return Cities.GetCount();
}


emString emTimeZonesModel::GetCityName(int cityIndex) const
{
	return Cities[cityIndex]->Name;
}


emTimeZonesModel::ZoneId emTimeZonesModel::GetCityZoneId(int cityIndex) const
{
	return cityIndex;
}


double emTimeZonesModel::GetCityLatitude(int cityIndex) const
{
	return Cities[cityIndex]->Latitude;
}


double emTimeZonesModel::GetCityLongitude(int cityIndex) const
{
	return Cities[cityIndex]->Longitude;
}


bool emTimeZonesModel::GetZoneTime(
	time_t time, ZoneId zoneId, int * pYear, int * pMonth, int * pDay,
	int * pDayOfWeek, int * pHour, int * pMinute, int * pSecond
)
{
	struct tm * ptm;
#if defined(_WIN32)
#else
	emString tzFile;
	char * oldTZ;
#endif

	//??? This is not thread-reentrant. We would have to use localtime_r
	//??? and gmtime_r, but because of playing with the environment
	//??? variable TZ, this stuff cannot be thread-reentrant and will have
	//??? to go into a separate child process.
	//??? Another problem is the heavy use of setenv and unsetenv: Do they
	//??? never produce memory leaks? On all systems?

	ptm=NULL;
	if (zoneId==LOCAL_ZONE_ID) {
		ptm=localtime(&time);
	}
	else if (zoneId==UTC_ZONE_ID) {
		ptm=gmtime(&time);
	}
	else if (zoneId>=0 && zoneId<Cities.GetCount()) {
#if defined(_WIN32)
		//...???...
#else
		if (!Cities[zoneId]->TzFileChecked) {
			tzFile=emGetChildPath(ZoneInfoDir,Cities[zoneId]->Name);
			Cities[zoneId]->TzFileExists=emIsRegularFile(tzFile);
			Cities[zoneId]->TzFileChecked=true;
		}
		if (Cities[zoneId]->TzFileExists) {
			oldTZ=getenv("TZ");
			if (setenv("TZ",Cities[zoneId]->Name.Get(),1)==0) {
				ptm=localtime(&time);
			}
			if (oldTZ) setenv("TZ",oldTZ,1);
			else unsetenv("TZ");
		}
#endif
	}

	if (ptm) {
		if (pYear     ) *pYear     =(int)ptm->tm_year+1900;
		if (pMonth    ) *pMonth    =(int)ptm->tm_mon+1;
		if (pDay      ) *pDay      =(int)ptm->tm_mday;
		if (pDayOfWeek) *pDayOfWeek=(int)ptm->tm_wday;
		if (pHour     ) *pHour     =(int)ptm->tm_hour;
		if (pMinute   ) *pMinute   =(int)ptm->tm_min;
		if (pSecond   ) *pSecond   =(int)ptm->tm_sec;
		return true;
	}
	else {
		if (pYear     ) *pYear     =0;
		if (pMonth    ) *pMonth    =0;
		if (pDay      ) *pDay      =0;
		if (pDayOfWeek) *pDayOfWeek=0;
		if (pHour     ) *pHour     =0;
		if (pMinute   ) *pMinute   =0;
		if (pSecond   ) *pSecond   =0;
		return false;
	}
}


double emTimeZonesModel::GetJulianDate(time_t time)
{
	int year, month, day, hour, minute, second;

	//??? This may not be correct before the beginning
	//??? of the Gregorian Calender (1582-10-15).

	GetZoneTime(time,UTC_ZONE_ID,&year,&month,&day,NULL,&hour,&minute,&second);
	if(month<=2) {
		month+=12;
		year--;
	}
	return
		second/(24.0*60.0*60.0)
		+minute/(24.0*60.0)
		+hour/24.0
		+day
		+((int)((month+1)*153/5))
		+((int)(year/400))
		-((int)(year/100))
		+((int)(year/4))
		+year*365
		+1720996.5
	;
}


emTimeZonesModel::emTimeZonesModel(emContext & context, const emString & name)
	: emModel(context,name)
{
	Time=time(NULL);
	Cities.SetTuningLevel(4);
	InitCities();
	WakeUp();
}


emTimeZonesModel::~emTimeZonesModel()
{
	int i;

	for (i=0; i<Cities.GetCount(); i++) delete Cities[i];
	Cities.Empty();
}


bool emTimeZonesModel::Cycle()
{
	time_t t;

	t=time(NULL);
	if (Time!=t) {
		Time=t;
		Signal(TimeSignal);
	}
	return true;
}


void emTimeZonesModel::InitCities()
{
#if defined(_WIN32)

	//...???...

#else

	emString zoneTabPath;
	FILE * f;
	char * buf;
	const char * p;
	int i,bufSize,l,sign;
	City * city;

	for (;;) {
		ZoneInfoDir="/usr/share/zoneinfo";
		if (emIsDirectory(ZoneInfoDir)) break;
		ZoneInfoDir="/usr/share/lib/zoneinfo";
		if (emIsDirectory(ZoneInfoDir)) break;
		ZoneInfoDir="/usr/lib/zoneinfo";
		if (emIsDirectory(ZoneInfoDir)) break;
		ZoneInfoDir="/usr/zoneinfo";
		if (emIsDirectory(ZoneInfoDir)) break;
		return;
	}
	for (;;) {
		zoneTabPath=ZoneInfoDir+"/zone.tab";
		if (emIsRegularFile(zoneTabPath)) break;
		zoneTabPath=ZoneInfoDir+"/tab/zone.tab";
		if (emIsRegularFile(zoneTabPath)) break;
		zoneTabPath=ZoneInfoDir+"/tab/zone_sun.tab";
		if (emIsRegularFile(zoneTabPath)) break;
		zoneTabPath=ZoneInfoDir+"/zone_sun.tab";
		if (emIsRegularFile(zoneTabPath)) break;
		return;
	}

	bufSize=65536;
	buf=new char[bufSize];

	f=fopen(zoneTabPath.Get(),"r");
	if (!f) goto L_FileError;

	city=NULL;
	for (;;) {
		if (!fgets(buf,bufSize,f)) {
			if (ferror(f)) goto L_FileError;
			break;
		}
		p=buf;

		if (!city) city=new City;

		while (*p && (emByte)*p<=32) { p++; }
		if (*p=='#' || (emByte)*p<=32) continue;
		city->CountryCode[0]=*p++;
		if ((emByte)*p<=32) continue;
		city->CountryCode[1]=*p++;
		city->CountryCode[2]=0;
		if ((emByte)*p>32) continue;

		while (*p && (emByte)*p<=32) { p++; }
		if (*p=='+') { sign=1; p++; }
		else if (*p=='-') { sign=-1; p++; }
		else sign=1;
		if (*p<'0' || *p>'9') continue;
		city->Latitude=((*p++)-'0')*10.0;
		if (*p<'0' || *p>'9') continue;
		city->Latitude+=((*p++)-'0');
		if (*p<'0' || *p>'9') continue;
		city->Latitude+=((*p++)-'0')/6.0;
		if (*p<'0' || *p>'9') continue;
		city->Latitude+=((*p++)-'0')/60.0;
		if (*p>='0' && *p<='9') {
			city->Latitude+=((*p++)-'0')/360.0;
			if (*p<'0' || *p>'9') continue;
			city->Latitude+=((*p++)-'0')/3600.0;
			if (*p>='0' && *p<='9') continue;
		}
		city->Latitude*=sign;

		while (*p && (emByte)*p<=32) { p++; }
		if (*p=='+') { sign=1; p++; }
		else if (*p=='-') { sign=-1; p++; }
		else sign=1;
		if (*p<'0' || *p>'9') continue;
		city->Longitude=((*p++)-'0')*100.0;
		if (*p<'0' || *p>'9') continue;
		city->Longitude+=((*p++)-'0')*10.0;
		if (*p<'0' || *p>'9') continue;
		city->Longitude+=((*p++)-'0');
		if (*p<'0' || *p>'9') continue;
		city->Longitude+=((*p++)-'0')/6.0;
		if (*p<'0' || *p>'9') continue;
		city->Longitude+=((*p++)-'0')/60.0;
		if (*p>='0' && *p<='9') {
			city->Longitude+=((*p++)-'0')/360.0;
			if (*p<'0' || *p>'9') continue;
			city->Longitude+=((*p++)-'0')/3600.0;
			if (*p>='0' && *p<='9') continue;
		}
		city->Longitude*=sign;

		if (fabs(city->Latitude)<0.0001 && fabs(city->Longitude)<0.0001) {
			continue;
		}

		while (*p && (emByte)*p<=32) { p++; }
		for (l=0; (emByte)p[l]>32; l++);
		if (!l) continue;
		city->Name=emString(p,l);
		p+=l;

		while (*p && (emByte)*p<=32) { p++; }
		l=strlen(p);
		while (l>0 && (emByte)p[l-1]<=32) l--;
		city->Comment=emString(p,l);

		city->TzFileChecked=false;
		city->TzFileExists=false;

		i=Cities.BinarySearchByKey((void*)city->Name.Get(),CmpCityAndName);
		if (i<0) {
			Cities.Insert(~i,city);
			city=NULL;
		}
	}

	if (city) delete city;
	fclose(f);
	delete [] buf;
	Cities.Compact();
	return;

L_FileError:
	emWarning("Failed to read \"%s\": %s",zoneTabPath.Get(),strerror(errno));
	if (city) delete city;
	if (f) fclose(f);
	delete [] buf;
	Cities.Empty(true);
#endif
}


int emTimeZonesModel::CmpCityAndName(
	City * const * obj, void * key, void * context
)
{
	const City * city;
	const char * name;

	city=*obj;
	name=(const char*)key;
	return strcmp(city->Name.Get(),name);
}
