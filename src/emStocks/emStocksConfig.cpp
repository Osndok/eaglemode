//------------------------------------------------------------------------------
// emStocksConfig.cpp
//
// Copyright (C) 2021-2022 Oliver Hamann.
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

#include <emStocks/emStocksConfig.h>
#include <emStocks/emStocksFileModel.h>
#include <emCore/emInstallInfo.h>


emRef<emStocksConfig> emStocksConfig::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emStocksConfig,context,name,common)
}


const char * emStocksConfig::GetFormatName() const
{
	return "emStocksConfig";
}


int emStocksConfig::CalculateChartPeriodDays(const char * endDate) const
{
	int y1,m1,d1,y2,m2,d2;

	switch (ChartPeriod.Get()) {
	case PT_1_WEEK:
		return 7;
	case PT_2_WEEKS:
		return 14;
	default:
		break;
	}

	emStocksFileModel::ParseDate(endDate,&y2,&m2,&d2);
	y1=y2;
	m1=m2;
	switch (ChartPeriod.Get()) {
	case PT_1_MONTH:
		m1--;
		break;
	case PT_3_MONTHS:
		m1-=3;
		break;
	case PT_6_MONTHS:
		m1-=6;
		break;
	case PT_1_YEAR:
		y1--;
		break;
	case PT_3_YEARS:
		y1-=3;
		break;
	case PT_5_YEARS:
		y1-=5;
		break;
	case PT_10_YEARS:
		y1-=10;
		break;
	case PT_20_YEARS:
		y1-=20;
		break;
	default:
		emFatalError("emStocksConfig::CalculateChartPeriodDays: illegal chart period");
	}
	while (m1<=0) { m1+=12; y1--; }
	d1=emMin(d2,emStocksFileModel::GetDaysOfMonth(y1,m1));
	return emStocksFileModel::GetDateDifference(y1,m1,d1,y2,m2,d2);
}


bool emStocksConfig::IsInVisibleCategories(
	const emTArrayRec<emStringRec> & categoriesRec,
	const char * category
)
{
	int i1,i2,i,d;

	i2=categoriesRec.GetCount();
	if (i2<=0) return true;
	i1=0;
	do {
		i=(i1+i2)>>1;
		d=strcoll(categoriesRec[i].Get().Get(),category);
		if (d>0) i2=i;
		else if (d<0) i1=i+1;
		else return true;
	} while (i1<i2);
	return false;
}


emStocksConfig::emStocksConfig(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	ApiScript(this,"ApiScript"),
	ApiScriptInterpreter(this,"ApiScriptInterpreter","perl"),
	ApiKey(this,"ApiKey"),
	WebBrowser(this,"WebBrowser",
#		if defined(_WIN32)
			"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"
#		else
			"firefox"
#		endif
	),
	AutoUpdateDates(this,"AutoUpdateDates"),
	TriggeringOpensWebPage(this,"TriggeringOpensWebPage"),
	ChartPeriod(this,"ChartPeriod",
		PT_1_YEAR,
		"PT_1_WEEK",
		"PT_2_WEEKS",
		"PT_1_MONTH",
		"PT_3_MONTHS",
		"PT_6_MONTHS",
		"PT_1_YEAR",
		"PT_3_YEARS",
		"PT_5_YEARS",
		"PT_10_YEARS",
		"PT_20_YEARS",
		NULL
	),
	MinVisibleInterest(this,"MinVisibleInterest",
		emStocksFileModel::LOW_INTEREST,
		"HIGH_INTEREST",
		"MEDIUM_INTEREST",
		"LOW_INTEREST",
		NULL
	),
	VisibleCountries(this,"VisibleCountries"),
	VisibleSectors(this,"VisibleSectors"),
	VisibleCollections(this,"VisibleCollections"),
	Sorting(this,"Sorting",
		SORT_BY_NAME,
		"SORT_BY_NAME",
		"SORT_BY_TRADE_DATE",
		"SORT_BY_INQUIRY_DATE",
		"SORT_BY_ACHIEVEMENT",
		"SORT_BY_ONE_WEEK_RISE",
		"SORT_BY_THREE_WEEK_RISE",
		"SORT_BY_NINE_WEEK_RISE",
		"SORT_BY_DIVIDEND",
		"SORT_BY_PURCHASE_VALUE",
		"SORT_BY_VALUE",
		"SORT_BY_DIFFERENCE",
		NULL
	),
	OwnedSharesFirst(this,"OwnedSharesFirst"),
	SearchText(this,"SearchText")
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emStocks","config.rec")
	);
	SetMinCommonLifetime(60);
	SetAutoSaveDelaySeconds(5);
	LoadOrInstall();
}


emStocksConfig::~emStocksConfig()
{
	Save();
}
