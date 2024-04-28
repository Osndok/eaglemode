//------------------------------------------------------------------------------
// emStocksConfig.h
//
// Copyright (C) 2021-2022,2024 Oliver Hamann.
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

#ifndef emStocksConfig_h
#define emStocksConfig_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif

#ifndef emStocksRec_h
#include <emStocks/emStocksRec.h>
#endif


class emStocksConfig :
	public emConfigModel,
	public emStructRec
{
public:

	static emRef<emStocksConfig> Acquire(
		emContext & context, const emString & name="", bool common=true
	);

	virtual const char * GetFormatName() const;

	enum PeriodType {
		PT_1_WEEK,
		PT_2_WEEKS,
		PT_1_MONTH,
		PT_3_MONTHS,
		PT_6_MONTHS,
		PT_1_YEAR,
		PT_3_YEARS,
		PT_5_YEARS,
		PT_10_YEARS,
		PT_20_YEARS
	};

	enum SortingType {
		SORT_BY_NAME,
		SORT_BY_TRADE_DATE,
		SORT_BY_INQUIRY_DATE,
		SORT_BY_ACHIEVEMENT,
		SORT_BY_ONE_WEEK_RISE,
		SORT_BY_THREE_WEEK_RISE,
		SORT_BY_NINE_WEEK_RISE,
		SORT_BY_DIVIDEND,
		SORT_BY_PURCHASE_VALUE,
		SORT_BY_VALUE,
		SORT_BY_DIFFERENCE
	};

	emStringRec ApiScript;
	emStringRec ApiScriptInterpreter;
	emStringRec ApiKey;
	emStringRec WebBrowser;
	emBoolRec AutoUpdateDates;
	emBoolRec TriggeringOpensWebPage;
	emEnumRec ChartPeriod;
	emStocksRec::InterestRec MinVisibleInterest;
	emTArrayRec<emStringRec> VisibleCountries;
	emTArrayRec<emStringRec> VisibleSectors;
	emTArrayRec<emStringRec> VisibleCollections;
	emEnumRec Sorting;
	emBoolRec OwnedSharesFirst;
	emStringRec SearchText;

	int CalculateChartPeriodDays(const char * endDate) const;

	static bool IsInVisibleCategories(
		const emTArrayRec<emStringRec> & categoriesRec,
		const char * category
	);

protected:

	emStocksConfig(emContext & context, const emString & name);
	virtual ~emStocksConfig();
};


#endif
