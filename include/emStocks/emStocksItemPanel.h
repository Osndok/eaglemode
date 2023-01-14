//------------------------------------------------------------------------------
// emStocksItemPanel.h
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

#ifndef emStocksItemPanel_h
#define emStocksItemPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emStocksConfig_h
#include <emStocks/emStocksConfig.h>
#endif

#ifndef emStocksFileModel_h
#include <emStocks/emStocksFileModel.h>
#endif

class emStocksItemChart;
class emStocksListBox;


class emStocksItemPanel :
	public emLinearGroup,
	public emListBox::ItemPanelInterface,
	private emRecListener {

public:

	emStocksItemPanel(emStocksListBox & parent, const emString & name,
	                  int itemIndex, emStocksFileModel & fileModel,
	                  emStocksConfig & config);
	virtual ~emStocksItemPanel();

	emStocksRec::StockRec * GetStockRec() const;
	void SetStockRec(emStocksRec::StockRec * stockRec);

	virtual emString GetTitle() const;

protected:

	virtual bool Cycle();

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual void ItemTextChanged();
	virtual void ItemDataChanged();
	virtual void ItemSelectionChanged();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	virtual void OnRecChanged();

	void UpdateControls();

	static bool ValidateNumber(
		const emTextField & textField, int & pos, int & removeLen,
		emString & insertText, void * context
	);
	static bool ValidateDate(
		const emTextField & textField, int & pos, int & removeLen,
		emString & insertText, void * context
	);

	enum CategoryType {
		CT_COUNTRY,
		CT_SECTOR,
		CT_COLLECTION
	};

	class CategoryPanel : public emLinearGroup {
	public:
		CategoryPanel(
			ParentArg parent, const emString & name,
			emStocksItemPanel & itemPanel,
			CategoryType type,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		);
		virtual ~CategoryPanel();
	protected:
		virtual bool Cycle();
		virtual void Notice(NoticeFlags flags);
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateControls();
		static int CompareItems(
			const emString & item1name, const emString & item1text,
			const emAnything & item1data,
			const emString & item2name, const emString & item2text,
			const emAnything & item2data,
			void * context
		);
		emStringRec * GetCategoryRec(emStocksRec::StockRec * stockRec);
		emTArrayRec<emStringRec> & GetCategoriesConfigRec();
		emStocksItemPanel & ItemPanel;
		CategoryType Type;
		emLinearLayout * WTLayout;
		emLabel * WarningLabel;
		emTextField * TextField;
		emListBox * ListBox;
		emString PreservedCategory;
		bool UpdateControlsNeeded;
		bool HaveListBoxContent;
	};

	emStocksListBox & ListBox;
	emStocksFileModel & FileModel;
	emStocksConfig & Config;

	bool UpdateControlsNeeded;

	emLabel * NameLabel;
	emTextField * Name;
	emTextField * Symbol;
	emTextField * WKN;
	emTextField * ISIN;

	CategoryPanel * Country;
	CategoryPanel * Sector;
	CategoryPanel * Collection;

	emCheckButton * OwningShares;
	emTextField * OwnShares;

	emTextField * TradePrice;
	emTextField * TradeDate;
	emButton * UpdateTradeDate;

	emTextField * Price;
	emTextField * PriceDate;
	emButton * FetchSharePrice;

	emTextField * DesiredPrice;
	emTextField * ExpectedDividend;
	emTextField * InquiryDate;
	emButton * UpdateInquiryDate;

	emRadioButton::LinearGroup * Interest;

	enum { NUM_WEB_PAGES=4 };
	emTextField * WebPage[NUM_WEB_PAGES];
	emButton * ShowWebPage[NUM_WEB_PAGES];
	emButton * ShowAllWebPages;

	emTextField * Comment;
	emTextField * TradeValue;
	emTextField * CurrentValue;
	emTextField * DifferenceValue;
	emStocksItemChart * Chart;

	emString PrevOwnShares;
	emString PrevPurchasePrice;
	emString PrevPurchaseDate;
	emString PrevSalePrice;
	emString PrevSaleDate;
};


inline emStocksRec::StockRec * emStocksItemPanel::GetStockRec() const
{
	return (emStocksRec::StockRec*)GetListenedRec();
}


#endif
