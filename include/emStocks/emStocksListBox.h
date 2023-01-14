//------------------------------------------------------------------------------
// emStocksListBox.h
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

#ifndef emStocksListBox_h
#define emStocksListBox_h

#ifndef emStocksItemPanel_h
#include <emStocks/emStocksItemPanel.h>
#endif


class emStocksListBox : public emListBox {

public:

	emStocksListBox(ParentArg parent, const emString & name,
	                emStocksFileModel & fileModel,
	                emStocksConfig & config);

	virtual ~emStocksListBox();

	emStocksRec::StockRec * GetStockByItemIndex(int itemIndex) const;
	int GetItemIndexByStock(const emStocksRec::StockRec * stockRec) const;

	const emSignal & GetSelectedDateSignal() const;
	const emString & GetSelectedDate() const;
	void SetSelectedDate(const emString & selectedDate);
	void GoBackInHistory();
	void GoForwardInHistory();

	void NewStock();
	void CutStocks(bool ask=true);
	bool CopyStocks();
	void PasteStocks(bool ask=true);
	void DeleteStocks(bool ask=true);
	void StartToFetchSharePrices();
	void StartToFetchSharePrices(const emArray<emString> & stockIds);
	void DeleteSharePrices();
	void SetInterest(emStocksFileModel::InterestType interest,
	                 bool ask=true);
	void ShowFirstWebPages() const;
	void ShowAllWebPages() const;
	void ShowWebPages(const emArray<emString> & webPages) const;
	void FindSelected();
	void FindNext();
	void FindPrevious();
	bool IsVisibleStock(const emStocksRec::StockRec & stockRec) const;

protected:

	virtual bool Cycle();

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual void CreateItemPanel(const emString & name, int itemIndex);

private:

	void UpdateItems();

	static int CompareItems(
		const emString & item1name, const emString & item1text,
		const emAnything & item1data,
		const emString & item2name, const emString & item2text,
		const emAnything & item2data,
		void * context
	);

	emStocksFileModel & FileModel;
	emStocksConfig & Config;

	emSignal SelectedDateSignal;
	emString SelectedDate;

	emCrossPtr<emDialog> CutStocksDialog;

	emCrossPtr<emDialog> PasteStocksDialog;

	emCrossPtr<emDialog> DeleteStocksDialog;

	emCrossPtr<emDialog> InterestDialog;
	emStocksFileModel::InterestType InterestToSet;
};


inline const emSignal & emStocksListBox::GetSelectedDateSignal() const
{
	return SelectedDateSignal;
}

inline const emString & emStocksListBox::GetSelectedDate() const
{
	return SelectedDate;
}


#endif
