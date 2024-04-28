//------------------------------------------------------------------------------
// emStocksRec.h
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

#ifndef emStocksRec_h
#define emStocksRec_h

#ifndef emCrossPtr_h
#include <emCore/emCrossPtr.h>
#endif

#ifndef emRec_h
#include <emCore/emRec.h>
#endif


class emStocksRec : public emStructRec
{
public:

	emStocksRec();
	virtual ~emStocksRec();

	virtual const char * GetFormatName() const;

	enum InterestType {
		HIGH_INTEREST   = 0,
		MEDIUM_INTEREST = 1,
		LOW_INTEREST    = 2
	};

	class InterestRec : public emEnumRec {
	public:
		// Class exists only for supporting deprecated (partly buggy)
		// identifiers.
		InterestRec(emStructRec * parent, const char * varIdentifier,
		            int defaultValue, bool bugInDeprecatedIdentifiers);
		InterestRec & operator = (int value);
		virtual void TryStartReading(emRecReader & reader);
	private:
		bool BugInDeprecatedIdentifiers;
	};

	class StockRec : public emStructRec {
	public:
		StockRec();
		virtual ~StockRec();

		void LinkCrossPtr(emCrossPtrPrivate & crossPtr);

		emStringRec Id;
		emStringRec Name;
		emStringRec Symbol;
		emStringRec WKN;
		emStringRec ISIN;
		emStringRec Country;
		emStringRec Sector;
		emStringRec Collection;
		emStringRec Comment;

		emBoolRec OwningShares;
		emStringRec OwnShares;

		emStringRec TradePrice;
		emStringRec TradeDate;
			// If OwningShares then purchase price and date else sale price and date.

		enum { MAX_NUM_PRICES = 366*20 };
		emStringRec Prices;
			// Prices separated by '|', one price per day, unknown
			// prices empty, latest price last.
		emStringRec LastPriceDate;

		emStringRec DesiredPrice;
			// If OwningShares then desired sale price else desired purchase price.

		emStringRec ExpectedDividend;

		emStringRec InquiryDate;
			// When did one update desired price and expected dividend

		InterestRec Interest;

		emTArrayRec<emStringRec> WebPages;

		const char * GetPricePtrOfDate(const char * date) const;
		emString GetPriceOfDate(const char * date) const;
		emString GetPricesDateBefore(const char * date) const;
		emString GetPricesDateAfter(const char * date) const;
		void AddPrice(const char * date, const char * price);
		bool IsMatchingSearchText(const char * searchText) const;

		bool GetTradeValue(double * pResult) const;
		bool GetValueOfDate(double * pResult, const char * date) const;
		bool GetDifferenceValueOfDate(double * pResult, const char * date) const;
		bool GetAchievementOfDate(double * pResult, const char * date,
		                          bool relative=false) const;
		bool GetRiseUntilDate(double * pResult, const char * date, int days) const;

	private:
		emCrossPtrList CrossPtrList;
	};

	emTArrayRec<StockRec> Stocks;

	emString InventStockId() const;

	int GetStockIndexById(const char * id) const;
	int GetStockIndexByStock(const emStocksRec::StockRec * stockRec) const;

	static bool ParseDate(const char * date, int * pYear=NULL, int * pMonth=NULL,
	                      int * pDay=NULL);
	static int CompareDates(const char * date1, const char * date2);
	static int GetDaysOfMonth(int year, int month);
	static void AddDaysToDate(int days, int * pYear, int * pMonth, int * pDay);
	static emString AddDaysToDate(int days, const char * date);
	static int GetDateDifference(int y1, int m1, int d1, int y2, int m2, int d2);
	static int GetDateDifference(const char * fromDate, const char * toDate,
	                             bool * pDatesValid=NULL);
	static emString GetCurrentDate();
	emString GetLatestPricesDate() const;
	emString GetPricesDateBefore(const char * date) const;
	emString GetPricesDateAfter(const char * date) const;

	static void SharePriceToString(double price, char * buf, int bufSize);
	static emString SharePriceToString(double price);
	static void PaymentPriceToString(double price, char * buf, int bufSize);
	static emString PaymentPriceToString(double price);
};


inline void emStocksRec::StockRec::LinkCrossPtr(
	emCrossPtrPrivate & crossPtr
)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}


#endif
