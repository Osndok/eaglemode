//------------------------------------------------------------------------------
// emTimeZonesModel.cpp
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

#include <emClock/emTimeZonesModel.h>
#include <emCore/emInstallInfo.h>


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


void emTimeZonesModel::TryGetZoneTime(
	time_t time, ZoneId zoneId, int * pYear, int * pMonth, int * pDay,
	int * pDayOfWeek, int * pHour, int * pMinute, int * pSecond
) throw(emString)
{
	struct tm tmbuf;
	struct tm * ptm;
	City * city;

	if (zoneId>=0 && zoneId<Cities.GetCount()) {
		if (ChildProcState==CP_ERRORED) throw ChildProcError;
		city=Cities[zoneId];
		if (!city->TzFileChecked) {
			city->TzFileExists=emIsRegularFile(
				emGetChildPath(ZoneInfoDir,city->Name)
			);
			city->TzFileChecked=true;
		}
		if (!city->TzFileExists) {
			throw emString("TZ file not found.");
		}
		city->TimeNeeded=3;
		if (!city->TimeValid) {
			RequestCityTime(city);
			throw emString("wait");
		}
		if (pYear     ) *pYear     =city->Year     ;
		if (pMonth    ) *pMonth    =city->Month    ;
		if (pDay      ) *pDay      =city->Day      ;
		if (pDayOfWeek) *pDayOfWeek=city->DayOfWeek;
		if (pHour     ) *pHour     =city->Hour     ;
		if (pMinute   ) *pMinute   =city->Minute   ;
		if (pSecond   ) *pSecond   =city->Second   ;
	}
	else {
		if (zoneId==LOCAL_ZONE_ID) {
			ptm=localtime_r(&time,&tmbuf);
		}
		else if (zoneId==UTC_ZONE_ID) {
			ptm=gmtime_r(&time,&tmbuf);
		}
		else {
			throw emString("Illegal time zone ID.");
		}
		if (!ptm) {
			throw emString("Time not available.");
		}
		if (pYear     ) *pYear     =(int)ptm->tm_year+1900;
		if (pMonth    ) *pMonth    =(int)ptm->tm_mon+1;
		if (pDay      ) *pDay      =(int)ptm->tm_mday;
		if (pDayOfWeek) *pDayOfWeek=(int)ptm->tm_wday;
		if (pHour     ) *pHour     =(int)ptm->tm_hour;
		if (pMinute   ) *pMinute   =(int)ptm->tm_min;
		if (pSecond   ) *pSecond   =(int)ptm->tm_sec;
	}
}


double emTimeZonesModel::GetJulianDate(time_t time)
{
	int year, month, day, hour, minute, second;

	//??? This may not be correct before the beginning
	//??? of the Gregorian Calender (1582-10-15).

	try {
		TryGetZoneTime(
			time,UTC_ZONE_ID,&year,&month,&day,NULL,&hour,&minute,&second
		);
	}
	catch (emString) {
		return 0.0;
	}
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
	ChildProcState=CP_STOPPED;
	ChildProcIdleClock=0;
	ReadBufFill=0;
	WriteBufFill=0;
	ReadBufSize=16384;
	WriteBufSize=16384;
	ReadBuf=(char*)malloc(ReadBufSize);
	WriteBuf=(char*)malloc(WriteBufSize);
	InitCities();
	WakeUp();
}


emTimeZonesModel::~emTimeZonesModel()
{
	int i;

	ChildProc.Terminate();
	Requests.Empty();
	for (i=0; i<Cities.GetCount(); i++) delete Cities[i];
	Cities.Empty();
	free(ReadBuf);
	free(WriteBuf);
}


bool emTimeZonesModel::Cycle()
{
	time_t t;
	City * city;
	bool newSecond;
	int i;

	t=time(NULL);
	if (Time!=t) {
		Time=t;
		newSecond=true;
	}
	else {
		newSecond=false;
	}

	if (newSecond) {
		for (i=0; i<Cities.GetCount(); i++) {
			city=Cities[i];
			if (city->TimeRequested) continue;
			city->TimeValid=false;
			if (city->TimeNeeded<=0) continue;
			city->TimeNeeded--;
			RequestCityTime(city);
		}
	}

	ManageChildProc();

	if (newSecond && Requests.IsEmpty()) Signal(TimeSignal);

	if (ReplyCityTimes()) Signal(TimeSignal);

	return true;
}


emTimeZonesModel::City::City()
{
	CountryCode[0]=0;
	Latitude=0.0;
	Longitude=0.0;
	TzFileChecked=0;
	TzFileExists=0;
	TimeValid=0;
	TimeRequested=0;
	TimeNeeded=0;
	Year=0;
	Month=0;
	Day=0;
	DayOfWeek=0;
	Hour=0;
	Minute=0;
	Second=0;
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
	emWarning(
		"Failed to read \"%s\": %s",
		zoneTabPath.Get(),
		emGetErrorText(errno).Get()
	);
	if (city) delete city;
	if (f) fclose(f);
	delete [] buf;
	Cities.Empty(true);
#endif
}


void emTimeZonesModel::RequestCityTime(City * city)
{
	int l;

	if (city->TimeRequested) return;

	l=city->Name.GetLen()+1;
	if (WriteBufSize-WriteBufFill<l) {
		WriteBufSize=WriteBufSize*2+l;
		WriteBuf=(char*)realloc(WriteBuf,WriteBufSize);
	}
	strcpy(WriteBuf+WriteBufFill,city->Name.Get());
	WriteBufFill+=l;
	WriteBuf[WriteBufFill-1]='\n';

	Requests.Add(city);

	city->TimeRequested=true;
}


bool emTimeZonesModel::ReplyCityTimes()
{
	City * const * r;
	City * city;
	char * p, * p2, * pEnd;
	bool gotSome;

	gotSome=false;
	for (p=ReadBuf, pEnd=ReadBuf+ReadBufFill; p<pEnd; ) {
		r=Requests.GetFirst();
		if (!r) break;
		city=*r;
		while (p<pEnd && (*p==0x0a || *p==0x0d)) p++;
		p2=p;
		while (p2<pEnd && *p2!=0x0a && *p2!=0x0d) p2++;
		if (p2>=pEnd) break;
		*p2=0;
		sscanf(
			p,
			"%d-%d-%d %d %d:%d:%d",
			&city->Year,
			&city->Month,
			&city->Day,
			&city->DayOfWeek,
			&city->Hour,
			&city->Minute,
			&city->Second
		);
		city->TimeValid=true;
		city->TimeRequested=false;
		Requests.RemoveFirst();
		p=p2+1;
		gotSome=true;
	}
	if (p>ReadBuf) {
		ReadBufFill-=(int)(p-ReadBuf);
		if (ReadBufFill>0) memmove(ReadBuf,p,ReadBufFill);
	}
	return gotSome;
}


void emTimeZonesModel::ManageChildProc()
{
	City * const * r;
	City * city;
	int l;
	emUInt64 clk;

	if (ChildProcState==CP_STOPPING) {
		if (!ChildProc.IsRunning()) {
			ChildProcState=CP_STOPPED;
		}
	}

	if (ChildProcState==CP_STOPPED && WriteBufFill>0) {
		try {
			ChildProc.TryStart(
				emArray<emString>(
					emGetChildPath(
						emGetInstallPath(EM_IDT_LIB,"emClock","emClock"),
						"emTimeZonesProc"
					)
				),
				emArray<emString>(),
				NULL,
				emProcess::SF_PIPE_STDIN|
				emProcess::SF_PIPE_STDOUT|
				emProcess::SF_SHARE_STDERR
			);
			ChildProcState=CP_RUNNING;
		}
		catch (emString errorMessage) {
			ChildProc.Terminate();
			ChildProcState=CP_ERRORED;
			ChildProcError=errorMessage;
		}
	}

	if (ChildProcState==CP_RUNNING) {
		clk=emGetClockMS();
		try {
			l=ChildProc.TryWrite(WriteBuf,WriteBufFill);
			if (l>0) {
				ChildProcIdleClock=clk;
				WriteBufFill-=l;
				if (WriteBufFill>0) memmove(WriteBuf,WriteBuf+l,WriteBufFill);
			}
			if (ReadBufFill<ReadBufSize) {
				l=ChildProc.TryRead(ReadBuf,ReadBufSize-ReadBufFill);
				if (l>0) {
					ChildProcIdleClock=clk;
					ReadBufFill+=l;
				}
			}
			if (ReadBufFill>=ReadBufSize) {
				ReadBufSize*=2;
				ReadBuf=(char*)realloc(ReadBuf,ReadBufSize);
			}
		}
		catch (emString errorMessage) {
			ChildProc.Terminate();
			ChildProcState=CP_ERRORED;
			ChildProcError=errorMessage;
		}

		if (ChildProcState==CP_RUNNING && clk-ChildProcIdleClock>10000) {
			ChildProc.CloseWriting();
			ChildProc.CloseReading();
			ChildProc.SendTerminationSignal();
			ChildProcState=CP_STOPPING;
		}
	}

	if (ChildProcState!=CP_RUNNING) {
		ReadBufFill=0;
		WriteBufFill=0;
		for (;;) {
			r=Requests.GetFirst();
			if (!r) break;
			city=*r;
			city->TimeRequested=false;
			Requests.RemoveFirst();
		}
	}
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
