//------------------------------------------------------------------------------
// emStocksItemChart.h
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

#ifndef emStocksItemChart_h
#define emStocksItemChart_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emStocksConfig_h
#include <emStocks/emStocksConfig.h>
#endif

#ifndef emStocksFileModel_h
#include <emStocks/emStocksFileModel.h>
#endif

class emStocksListBox;


class emStocksItemChart : public emBorder, private emRecListener {

public:

	emStocksItemChart(ParentArg parent, const emString & name,
	                  emStocksListBox & listBox,
	                  emStocksConfig & config);
	virtual ~emStocksItemChart();

	emStocksRec::StockRec * GetStockRec() const;
	void SetStockRec(emStocksRec::StockRec * stockRec);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w, double h,
		emColor canvasColor
	) const;

private:

	virtual void OnRecChanged();

	void InvalidateData();
	void UpdateData();
	void UpdateTimeRange();
	int CalculateDaysPerPrice();
	void UpdatePrices1();
	void UpdatePrices2();
	void UpdateTransformation();

	void PaintXScaleLines(const emPainter & painter) const;
	void PaintXScaleLabels(const emPainter & painter) const;
	void PaintYScaleLines(const emPainter & painter) const;
	void PaintYScaleLabels(const emPainter & painter) const;
	void CalculateYScaleLevelRange(
		int * pMinLevel, double * pMinDist, int * pMaxLevel
	) const;
	double GetMaxLabelHeight() const;

	void PaintPriceBar(const emPainter & painter) const;
	void PaintDesiredPrice(const emPainter & painter) const;
	void PaintGraph(const emPainter & painter) const;

	struct Price {
		void Set(const char * str);
		operator bool () const;
		bool Valid;
		double Value;
	};

	emStocksListBox & ListBox;
	emStocksConfig & Config;

	bool DataUpToDate;
	emUInt64 UpdateTimeout;

	emString StartDate;
	int StartYear;
	int StartMonth;
	int StartDay;
	emString EndDate;
	int TotalDays;
	int DaysPerPrice;

	bool OwningShares;
	Price TradePrice;
	emString TradePriceText;
	int TradeOffsetDays;
	Price PriceOnSelectedDate;
	emString PriceOnSelectedDateText;
	Price DesiredPrice;
	emString DesiredPriceText;
	emArray<Price> Prices;
	Price MinPrice;
	Price MaxPrice;

	double XOffset;
	double XFactor;
	double YOffset;
	double YFactor;
	double LowerPrice;
	double UpperPrice;
};


inline emStocksRec::StockRec * emStocksItemChart::GetStockRec() const
{
	return (emStocksRec::StockRec*)GetListenedRec();
}

inline emStocksItemChart::Price::operator bool () const
{
	return Valid;
}

#endif
