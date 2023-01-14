//------------------------------------------------------------------------------
// emStocksControlPanel.h
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

#ifndef emStocksControlPanel_h
#define emStocksControlPanel_h

#ifndef emStocksListBox_h
#include <emStocks/emStocksListBox.h>
#endif


class emStocksControlPanel : public emLinearGroup {

public:

	emStocksControlPanel(ParentArg parent, const emString & name,
	                     emStocksFileModel & fileModel,
	                     emStocksConfig & config,
	                     emStocksListBox & listBox);
	virtual ~emStocksControlPanel();

protected:

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void UpdateControls();

	static void ChartPeriodTextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);

	static bool ValidateDate(
		const emTextField & textField, int & pos, int & removeLen,
		emString & insertText, void * context
	);

	static emString BreakPath(emString path, const emString & brkStr, int len);

	enum FileFieldType {
		FT_SCRIPT,
		FT_INTERPRETER,
		FT_BROWSER
	};

	class FileFieldPanel : public emLinearGroup {
	public:
		FileFieldPanel(
			ParentArg parent, const emString & name,
			emStocksControlPanel & itemPanel,
			FileFieldType type,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		);
		virtual ~FileFieldPanel();
	protected:
		virtual bool Cycle();
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateControls();
		emStringRec * GetFileRec();
		emStocksControlPanel & ControlPanel;
		FileFieldType Type;
		emTextField * TextField;
		emFileSelectionBox * FileSelectionBox;
		bool UpdateControlsNeeded;
	};

	enum CategoryType {
		CT_COUNTRY,
		CT_SECTOR,
		CT_COLLECTION
	};

	class CategoryPanel : public emListBox, private emRecListener {
	public:
		CategoryPanel(
			ParentArg parent, const emString & name,
			emStocksControlPanel & controlPanel,
			CategoryType type,
			emTArrayRec<emStringRec> & categoriesConfigRec,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		);
		virtual ~CategoryPanel();
	protected:
		virtual bool Cycle();
		virtual void Notice(NoticeFlags flags);
		virtual void OnRecChanged();
	private:
		void UpdateItems();
		void UpdateSelection();
		void UpdateFromSelection();
		static int CompareItems(
			const emString & item1name, const emString & item1text,
			const emAnything & item1data,
			const emString & item2name, const emString & item2text,
			const emAnything & item2data,
			void * context
		);
		emStringRec * GetCategoryRec(emStocksRec::StockRec * stockRec);
		emStocksControlPanel & ControlPanel;
		CategoryType Type;
		emTArrayRec<emStringRec> & CategoriesConfigRec;
		emString NameOfAll;
		bool UpdateItemsNeeded;
		bool UpdateSelectionNeeded;
		bool HaveListBoxContent;
	};

	emRef<emStocksFileModel> FileModel;
	emRef<emStocksConfig> Config;
	emCrossPtr<emStocksListBox> ListBox;

	bool UpdateControlsNeeded;

	FileFieldPanel * ApiScript;
	FileFieldPanel * ApiScriptInterpreter;
	emTextField * ApiKey;
	FileFieldPanel * WebBrowser;
	emCheckBox * AutoUpdateDates;
	emCheckBox * TriggeringOpensWebPage;
	emScalarField * ChartPeriod;

	emRadioButton::LinearGroup * MinVisibleInterest;
	CategoryPanel * VisibleCountries;   // Sorted by strcoll
	CategoryPanel * VisibleSectors;     // Sorted by strcoll
	CategoryPanel * VisibleCollections; // Sorted by strcoll

	emRadioButton::RasterGroup * Sorting;

	emCheckButton * OwnedSharesFirst;

	emButton * FetchSharePrices;
	emButton * DeleteSharePrices;
	emButton * GoBackInHistory;
	emButton * GoForwardInHistory;
	emTextField * SelectedDate;
	emTextField * TotalPurchaseValue;
	emTextField * TotalCurrentValue;
	emTextField * TotalDifferenceValue;

	emButton * NewStock;
	emButton * CutStocks;
	emButton * CopyStocks;
	emButton * PasteStocks;
	emButton * DeleteStocks;
	emButton * SelectAll;
	emButton * ClearSelection;
	emButton * SetHighInterest;
	emButton * SetMediumInterest;
	emButton * SetLowInterest;
	emButton * ShowFirstWebPages;
	emButton * ShowAllWebPages;
	emButton * FindSelected;
	emTextField * SearchText;
	emButton * FindNext;
	emButton * FindPrevious;
};


#endif
