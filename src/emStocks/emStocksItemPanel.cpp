//------------------------------------------------------------------------------
// emStocksItemPanel.cpp
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

#include <emStocks/emStocksItemPanel.h>
#include <emStocks/emStocksItemChart.h>
#include <emStocks/emStocksListBox.h>


emStocksItemPanel::emStocksItemPanel(
	emStocksListBox & parent, const emString & name, int itemIndex,
	emStocksFileModel & fileModel, emStocksConfig & config
)
	: emLinearGroup(parent,name),
	ItemPanelInterface(parent,itemIndex),
	ListBox(parent),
	FileModel(fileModel),
	Config(config),
	UpdateControlsNeeded(true),
	NameLabel(NULL),
	Name(NULL),
	Symbol(NULL),
	WKN(NULL),
	ISIN(NULL),
	Country(NULL),
	Sector(NULL),
	Collection(NULL),
	OwningShares(NULL),
	OwnShares(NULL),
	TradePrice(NULL),
	TradeDate(NULL),
	UpdateTradeDate(NULL),
	Price(NULL),
	PriceDate(NULL),
	FetchSharePrice(NULL),
	DesiredPrice(NULL),
	ExpectedDividend(NULL),
	InquiryDate(NULL),
	UpdateInquiryDate(NULL),
	Interest(NULL),
	ShowAllWebPages(NULL),
	Comment(NULL),
	TradeValue(NULL),
	CurrentValue(NULL),
	DifferenceValue(NULL),
	Chart(NULL)
{
	int i;

	for (i=0; i<NUM_WEB_PAGES; i++) {
		WebPage[i]=NULL;
		ShowWebPage[i]=NULL;
	}

	SetBorderType(OBT_INSTRUMENT,IBT_NONE);
	SetAutoplayHandling(APH_ITEM|APH_CUTOFF);

	AddWakeUpSignal(Config.GetChangeSignal());
	AddWakeUpSignal(ListBox.GetSelectedDateSignal());
	WakeUp();
}


emStocksItemPanel::~emStocksItemPanel()
{
}


void emStocksItemPanel::SetStockRec(emStocksRec::StockRec * stockRec)
{
	if (GetStockRec()!=stockRec) {
		SetListenedRec(stockRec);
		if (Chart) Chart->SetStockRec(stockRec);
		UpdateControlsNeeded=true;
		WakeUp();
	}
}


emString emStocksItemPanel::GetTitle() const
{
	emStocksRec::StockRec * stockRec;

	stockRec=GetStockRec();
	if (!stockRec) {
		return emLinearGroup::GetTitle();
	}
	if (stockRec->Name.Get().IsEmpty()) {
		return "<unnamed>";
	}
	return stockRec->Name.Get();
}


bool emStocksItemPanel::Cycle()
{
	emStocksRec::StockRec * stockRec;
	bool busy;
	int i;

	busy=emLinearGroup::Cycle();

	stockRec=GetStockRec();
	if (!stockRec || !IsAutoExpanded()) return busy;

	if (
		IsSignaled(Config.GetChangeSignal()) ||
		IsSignaled(ListBox.GetSelectedDateSignal())
	) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(Name->GetTextSignal())) {
		stockRec->Name=Name->GetText();
	}

	if (IsSignaled(Symbol->GetTextSignal())) {
		if (stockRec->Symbol.Get()!=Symbol->GetText()) {
			stockRec->Symbol=Symbol->GetText();
			stockRec->Prices=emString();
			stockRec->LastPriceDate=emString();
		}
	}

	if (IsSignaled(WKN->GetTextSignal())) {
		stockRec->WKN=WKN->GetText();
	}

	if (IsSignaled(ISIN->GetTextSignal())) {
		stockRec->ISIN=ISIN->GetText();
	}

	if (IsSignaled(OwningShares->GetCheckSignal())) {
		if (stockRec->OwningShares!=OwningShares->IsChecked()) {
			stockRec->OwningShares=OwningShares->IsChecked();
			if (stockRec->OwningShares.Get()) {
				if (stockRec->OwnShares.Get().IsEmpty()) {
					stockRec->OwnShares=PrevOwnShares;
					PrevSalePrice=stockRec->TradePrice;
					PrevSaleDate=stockRec->TradeDate;
					stockRec->TradePrice=PrevPurchasePrice;
					stockRec->TradeDate=PrevPurchaseDate;
				}
			}
			else {
				if (!stockRec->OwnShares.Get().IsEmpty()) {
					PrevOwnShares=stockRec->OwnShares;
					stockRec->OwnShares="";
					PrevPurchasePrice=stockRec->TradePrice;
					PrevPurchaseDate=stockRec->TradeDate;
					stockRec->TradePrice=PrevSalePrice;
					stockRec->TradeDate=PrevSaleDate;
				}
			}
		}
	}

	if (IsSignaled(OwnShares->GetTextSignal())) {
		stockRec->OwnShares=OwnShares->GetText();
	}

	if (IsSignaled(TradePrice->GetTextSignal())) {
		if (stockRec->TradePrice.Get()!=TradePrice->GetText()) {
			stockRec->TradePrice=TradePrice->GetText();
			if (Config.AutoUpdateDates) {
				stockRec->TradeDate=emStocksFileModel::GetCurrentDate();
			}
		}
	}

	if (IsSignaled(TradeDate->GetTextSignal())) {
		stockRec->TradeDate=TradeDate->GetText();
	}

	if (IsSignaled(UpdateTradeDate->GetClickSignal())) {
		stockRec->TradeDate=emStocksFileModel::GetCurrentDate();
	}

	if (IsSignaled(FetchSharePrice->GetClickSignal())) {
		ListBox.StartToFetchSharePrices(emArray<emString>(stockRec->Id.Get()));
	}

	if (IsSignaled(DesiredPrice->GetTextSignal())) {
		if (stockRec->DesiredPrice.Get()!=DesiredPrice->GetText()) {
			stockRec->DesiredPrice=DesiredPrice->GetText();
			if (Config.AutoUpdateDates) {
				stockRec->InquiryDate=emStocksFileModel::GetCurrentDate();
			}
		}
	}

	if (IsSignaled(ExpectedDividend->GetTextSignal())) {
		if (stockRec->ExpectedDividend.Get()!=ExpectedDividend->GetText()) {
			stockRec->ExpectedDividend=ExpectedDividend->GetText();
			if (Config.AutoUpdateDates) {
				stockRec->InquiryDate=emStocksFileModel::GetCurrentDate();
			}
		}
	}

	if (IsSignaled(InquiryDate->GetTextSignal())) {
		stockRec->InquiryDate=InquiryDate->GetText();
	}

	if (IsSignaled(UpdateInquiryDate->GetClickSignal())) {
		stockRec->InquiryDate=emStocksFileModel::GetCurrentDate();
	}

	if (IsSignaled(Interest->GetCheckSignal())) {
		stockRec->Interest=Interest->GetCheckIndex();
	}

	for (i=0; i<NUM_WEB_PAGES; i++) {
		if (IsSignaled(WebPage[i]->GetTextSignal())) {
			if (!WebPage[i]->GetText().IsEmpty() && stockRec->WebPages.GetCount()<=i) {
				stockRec->WebPages.SetCount(i+1);
			}
			if (stockRec->WebPages.GetCount()>i) {
				stockRec->WebPages[i]=WebPage[i]->GetText();
			}
			while (
				stockRec->WebPages.GetCount()>0 &&
				stockRec->WebPages[stockRec->WebPages.GetCount()-1].Get().IsEmpty()
			) {
				stockRec->WebPages.Remove(stockRec->WebPages.GetCount()-1);
			}
		}
		if (IsSignaled(ShowWebPage[i]->GetClickSignal())) {
			if (!WebPage[i]->GetText().IsEmpty()) {
				ListBox.ShowWebPages(emArray<emString>(WebPage[i]->GetText()));
			}
		}
	}

	if (IsSignaled(ShowAllWebPages->GetClickSignal())) {
		emArray<emString> webPages;
		for (i=0; i<NUM_WEB_PAGES; i++) {
			if (!WebPage[i]->GetText().IsEmpty()) {
				webPages.Add(WebPage[i]->GetText());
			}
		}
		if (!webPages.IsEmpty()) ListBox.ShowWebPages(webPages);
	}

	if (IsSignaled(Comment->GetTextSignal())) {
		stockRec->Comment=Comment->GetText();
	}

	if (UpdateControlsNeeded) UpdateControls();

	return busy;
}


void emStocksItemPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (event.GetKey()==EM_KEY_ENTER && !IsActive()) {
		// Do not trigger item by input in a child.
		event.Eat();
	}
	ProcessItemInput(this,event,state);
	emLinearGroup::Input(event,state,mx,my);
}


void emStocksItemPanel::ItemTextChanged()
{
}


void emStocksItemPanel::ItemDataChanged()
{
}


void emStocksItemPanel::ItemSelectionChanged()
{
	emLook look;

	if (IsItemSelected()) {
		look=GetLook();
		look.SetBgColor(emColor(0,56,212));
		SetLook(look,false);
	}
	else {
		SetLook(GetListBox().GetLook(),false);
	}
}


void emStocksItemPanel::AutoExpand()
{
	emLinearLayout * l1, * alg, * swi, * cats, * trd, * prc, * div, * web, * val;
	emRasterLayout * r1;
	emString s;
	int i;

	emLinearGroup::AutoExpand();

	SetOrientationThresholdTallness(0.02);
	SetChildWeight(0,0.5);

	NameLabel=new emLabel(this,"NameLabel");

	l1=new emLinearLayout(this,"l1");
	l1->SetLook(GetListBox().GetLook());
	l1->SetOrientationThresholdTallness(1.0);
	l1->SetMinChildTallness(0,0.1);
	l1->SetMaxChildTallness(0,10.0);
	l1->SetChildTallness(1,0.56);
	l1->SetInnerSpace(0.01,0.01);

	r1=new emRasterLayout(l1,"r1");
	r1->SetBorderType(OBT_NONE,IBT_GROUP);
	r1->SetBorderScaling(2.0);
	r1->SetPrefChildTallness(1.0);

		alg=new emLinearLayout(r1,"alg");
		alg->SetChildWeight(3,1.6);
		alg->SetChildWeight(4,7.0);

			Name=new emTextField(alg,"Name","Name","The name of the stock");
			Name->SetEditable();
			AddWakeUpSignal(Name->GetTextSignal());

			swi=new emLinearLayout(alg,"swi");
			swi->SetChildWeight(0,0.6);
			swi->SetChildWeight(1,0.6);
			swi->SetChildWeight(2,1.1);
			swi->SetOrientationThresholdTallness(0.5);

				Symbol=new emTextField(
					swi,"Symbol","Symbol",
					"The ticker symbol of the stock. This is given to the API script\n"
					"when it is run to get the share price for this stock. The API\n"
					"script is configured in the emStocks Preferences.\n"
				);
				Symbol->SetEditable();
				AddWakeUpSignal(Symbol->GetTextSignal());

				WKN=new emTextField(
					swi,"WKN","WKN",
					"The Wertpapierkennnummer (WKN) of the stock."
				);
				WKN->SetEditable();
				AddWakeUpSignal(WKN->GetTextSignal());

				ISIN=new emTextField(
					swi,"ISIN","ISIN",
					"The International Securities Identification Number (ISIN) of the stock."
				);
				ISIN->SetEditable();
				AddWakeUpSignal(ISIN->GetTextSignal());

			cats=new emLinearLayout(alg,"cats");

				Country=new CategoryPanel(
					cats,"Country",*this,CT_COUNTRY,"Country",
					"The country of the stock (full name or abbreviation - what you like)."
				);
				Sector=new CategoryPanel(
					cats,"Sector",*this,CT_SECTOR,"Sector",
					"The sector of the stock."
				);
				Collection=new CategoryPanel(
					cats,"Collection",*this,CT_COLLECTION,"Collection",
					"This is meant as a custom categorization."
				);

			Comment=new emTextField(
				alg,"Comment","Comment",
				"Any comments on the stock."
			);
			Comment->SetBorderScaling(1.0/1.6);
			Comment->SetMultiLineMode();
			Comment->SetEditable();
			AddWakeUpSignal(Comment->GetTextSignal());

			web=new emLinearLayout(alg,"web");

				for (i=0; i<NUM_WEB_PAGES; i++) {
					web->SetChildWeight(i*2,0.4);
					s=emString::Format("Web Page %d",i+1);
					WebPage[i]=new emTextField(
						web,s,s,
						"URL of a web page related to the stock."
					);
					WebPage[i]->SetBorderScaling(1.5);
					WebPage[i]->SetEditable();
					AddWakeUpSignal(WebPage[i]->GetTextSignal());
					s=emString::Format("Show Web Page %d",i+1);
					ShowWebPage[i]=new emButton(
						web,s,s,
						"Run the web browser with the URL of this web page. The web browser\n"
						"executable is configured in the emStocks Preferences."
					);
					AddWakeUpSignal(ShowWebPage[i]->GetClickSignal());
				}

				ShowAllWebPages=new emButton(
					web,"ShowAllWebPages","Show All Web Pages",
					"Run the web browser with all non-empty web page URLs of this stock.");
				AddWakeUpSignal(ShowAllWebPages->GetClickSignal());


		trd=new emLinearLayout(r1,"trd");
		trd->SetChildWeight(2,2.0);

			OwningShares=new emCheckBox(
				trd,"OwningShares","Owning Shares",
				"Set this checked if you own shares of this stock and want to sell them,\n"
				"or set this unchecked if you want to purchase shares."
			);
			AddWakeUpSignal(OwningShares->GetCheckSignal());

			OwnShares=new emTextField(
				trd,"OwnShares","Own Shares",
				"If you own shares, then you should enter the number of\n"
				"shares that you own here."
			);
			OwnShares->SetEditable();
			OwnShares->SetValidateFunc(&ValidateNumber,this);
			AddWakeUpSignal(OwnShares->GetTextSignal());

			TradePrice=new emTextField(trd,"TradePrice","Trade Price");
			TradePrice->SetEditable();
			TradePrice->SetValidateFunc(&ValidateNumber,this);
			AddWakeUpSignal(TradePrice->GetTextSignal());

			TradeDate=new emTextField(trd,"TradeDate","Trade Date");
			TradeDate->SetEditable();
			TradeDate->SetValidateFunc(&ValidateDate,this);
			AddWakeUpSignal(TradeDate->GetTextSignal());

			UpdateTradeDate=new emButton(trd,"UpdateTradeDate","Update Trade Date");
			AddWakeUpSignal(UpdateTradeDate->GetClickSignal());

		prc=new emLinearLayout(r1,"prc");
		prc->SetChildWeight(0,2.0);
		prc->SetChildWeight(1,2.0);

			FetchSharePrice=new emButton(
				prc,"FetchSharePrice","Fetch",
				"Fetch the latest share prices of this stock and update the chart best\n"
				"possible. This runs the API script with the symbol of this stock (the\n"
				"API script is configured in the emStocks Preferences). The symbol\n"
				"must not be empty. A pop-up dialog shows the progress of fetching."
			);
			AddWakeUpSignal(FetchSharePrice->GetClickSignal());

			Price=new emTextField(
				prc,"Price","Price",
				"Share price of this stock at the selected date. This field is empty\n"
				"if there is no known price for that date."
			);

			PriceDate=new emTextField(
				prc,"PriceDate","Price Date",
				"This shows the date of the shown share price (if there is one). It is\n"
				"identical to the selected date in the control panel."
			);

			Interest=new emRadioButton::LinearGroup(
				prc,"Interest","Interest",
				"Here you can set how high your current interest in this stock is."
			);
			Interest->SetBorderScaling(3.0);
			Interest->SetOrientationThresholdTallness(0.25);
			new emRadioButton(Interest,"high","High");
			new emRadioButton(Interest,"medium","Medium");
			new emRadioButton(Interest,"low","Low");
			AddWakeUpSignal(Interest->GetCheckSignal());

		div=new emLinearLayout(r1,"div");
		div->SetChildWeight(0,2.0);
		div->SetChildWeight(1,2.0);

			ExpectedDividend=new emTextField(
				div,"ExpectedDividend","Expected Dividend",
				"Here you may enter the dividend percentage you expect for this stock.\n"
				"The only function around this is the possibility to sort the stocks by\n"
				"the dividends."
			);
			ExpectedDividend->SetEditable();
			ExpectedDividend->SetValidateFunc(&ValidateNumber,this);
			AddWakeUpSignal(ExpectedDividend->GetTextSignal());

			DesiredPrice=new emTextField(div,"DesiredPrice","Desired Price");
			DesiredPrice->SetEditable();
			DesiredPrice->SetValidateFunc(&ValidateNumber,this);
			AddWakeUpSignal(DesiredPrice->GetTextSignal());

			InquiryDate=new emTextField(
				div,"InquiryDate","Inquiry Date",
				"Here you may enter the date on which you updated the expected dividend\n"
				"and the desired price. The date must have the form YYYY-MM-DD."
			);
			InquiryDate->SetEditable();
			InquiryDate->SetValidateFunc(&ValidateDate,this);
			AddWakeUpSignal(InquiryDate->GetTextSignal());

			UpdateInquiryDate=new emButton(
				div,"UpdateInquiryDate","Update Inquiry Date",
				"Set the inquiry date to the current date. Note: In the emStocks\n"
				"Preferences is a check box for automatically updating dates, so that\n"
				"the inquiry date is updated whenever the expected dividend or the\n"
				"desired price is modified."
			);
			AddWakeUpSignal(UpdateInquiryDate->GetClickSignal());

		val=new emLinearLayout(r1,"val");

			TradeValue=new emTextField(
				val,"TradeValue","Purchase Value",
				"If you own shares, then this shows the product of the purchase price\n"
				"and the number of owned shares."
			);

			CurrentValue=new emTextField(
				val,"CurrentValue","Value On Selected Date",
				"If you own shares, then this shows the product of the share price on the\n"
				"currently selected date and the number of owned shares."
			);

			DifferenceValue=new emTextField(
				val,"DifferenceValue","Difference Value",
				"If you own shares, then this shows the difference between the purchase\n"
				"value and the value on the selected date."
			);

	Chart=new emStocksItemChart(l1,"Chart",ListBox,Config);
	Chart->SetBorderScaling(0.3);
	Chart->SetStockRec(GetStockRec());

	UpdateControlsNeeded=true;
	WakeUp();
}


void emStocksItemPanel::AutoShrink()
{
	int i;

	NameLabel=NULL;
	Name=NULL;
	Symbol=NULL;
	WKN=NULL;
	ISIN=NULL;
	Country=NULL;
	Sector=NULL;
	Collection=NULL;
	OwningShares=NULL;
	OwnShares=NULL;
	TradePrice=NULL;
	TradeDate=NULL;
	UpdateTradeDate=NULL;
	Price=NULL;
	PriceDate=NULL;
	FetchSharePrice=NULL;
	DesiredPrice=NULL;
	ExpectedDividend=NULL;
	InquiryDate=NULL;
	UpdateInquiryDate=NULL;
	Interest=NULL;
	for (i=0; i<NUM_WEB_PAGES; i++) {
		WebPage[i]=NULL;
		ShowWebPage[i]=NULL;
	}
	ShowAllWebPages=NULL;
	Comment=NULL;
	TradeValue=NULL;
	CurrentValue=NULL;
	DifferenceValue=NULL;
	Chart=NULL;

	emLinearGroup::AutoShrink();
}


void emStocksItemPanel::OnRecChanged()
{
	InvalidateTitle();
	UpdateControlsNeeded=true;
	WakeUp();
}


void emStocksItemPanel::UpdateControls()
{
	emStocksRec::StockRec * stockRec;
	emLook look;
	double d;
	int i;
	emByte alpha;

	UpdateControlsNeeded=false;
	stockRec=GetStockRec();
	if (!stockRec || !IsAutoExpanded()) return;

	if (stockRec->Name.Get().IsEmpty()) {
		NameLabel->SetCaption("<unnamed>");
		alpha=64;
	}
	else {
		NameLabel->SetCaption(stockRec->Name.Get());
		alpha=255;
	}
	look=NameLabel->GetLook();
	if (stockRec->OwningShares) {
		look.SetFgColor(emColor(240,255,160,alpha));
	}
	else {
		look.SetFgColor(emColor(240,240,240,alpha));
	}
	NameLabel->SetLook(look,true);

	Name->SetText(stockRec->Name);
	ListBox.SetItemText(GetItemIndex(),stockRec->Name); // Needed for key-walk

	Symbol->SetText(stockRec->Symbol);

	WKN->SetText(stockRec->WKN);

	ISIN->SetText(stockRec->ISIN);

	OwningShares->SetChecked(stockRec->OwningShares);

	OwnShares->SetEnableSwitch(stockRec->OwningShares);
	OwnShares->SetText(stockRec->OwnShares);

	TradePrice->SetCaption(
		stockRec->OwningShares ? "Purchase Price" : "Sale Price"
	);
	TradePrice->SetDescription(
		stockRec->OwningShares ?
		"Here you should enter the share price at which you bought shares of this stock." :
		"Here you may enter the share price at which you sold shares of this stock."
	);
	TradePrice->SetText(stockRec->TradePrice);

	TradeDate->SetCaption(
		stockRec->OwningShares ? "Purchase Date" : "Sale Date"
	);
	TradeDate->SetDescription(
		stockRec->OwningShares ?
		"Here you may enter the date on which you bought the shares.\n"
		"The date must have the form YYYY-MM-DD." :
		"Here you may enter the date on which you sold shares of this stock.\n"
		"The date must have the form YYYY-MM-DD."
	);
	TradeDate->SetText(stockRec->TradeDate);

	UpdateTradeDate->SetCaption(
		stockRec->OwningShares ? "Update Purchase Date" : "Update Sale Date"
	);
	UpdateTradeDate->SetDescription(
		stockRec->OwningShares ?
		"Set the purchase date to the current date. Note: In the emStocks\n"
		"Preferences is a check box for automatically updating dates, so that\n"
		"the purchase date is updated whenever the purchase price is modified." :
		"Set the sale date to the current date. Note: In the emStocks\n"
		"Preferences is a check box for automatically updating dates, so that\n"
		"the sale date is updated whenever the sale price is modified."
	);

	FetchSharePrice->SetEnableSwitch(!stockRec->Symbol.Get().IsEmpty());

	Price->SetText(stockRec->GetPriceOfDate(ListBox.GetSelectedDate()));

	if (Price->GetText().IsEmpty()) PriceDate->SetText(emString());
	else PriceDate->SetText(ListBox.GetSelectedDate());

	ExpectedDividend->SetText(stockRec->ExpectedDividend);

	DesiredPrice->SetCaption(
		stockRec->OwningShares ? "Desired Sale Price" : "Desired Purchase Price"
	);
	DesiredPrice->SetDescription(
		stockRec->OwningShares ?
		"Here you should enter the share price at which you want to sell your\n"
		"shares of this stock." :
		"Here you should enter the share price at which you want to purchase\n"
		"shares of this stock."
	);
	DesiredPrice->SetText(stockRec->DesiredPrice);

	InquiryDate->SetText(stockRec->InquiryDate);

	Interest->SetCheckIndex(stockRec->Interest.Get());

	for (i=0; i<NUM_WEB_PAGES; i++) {
		WebPage[i]->SetText(
			i<stockRec->WebPages.GetCount() ?
			stockRec->WebPages[i].Get() :
			emString()
		);
		ShowWebPage[i]->SetEnableSwitch(!WebPage[i]->GetText().IsEmpty());
	}

	ShowAllWebPages->SetEnableSwitch(stockRec->WebPages.GetCount() != 0);

	Comment->SetText(stockRec->Comment);

	if (stockRec->GetTradeValue(&d)) {
		TradeValue->SetText(emStocksFileModel::PaymentPriceToString(d));
	}
	else {
		TradeValue->SetText("");
	}

	if (stockRec->GetValueOfDate(&d,ListBox.GetSelectedDate())) {
		CurrentValue->SetText(emStocksFileModel::PaymentPriceToString(d));
	}
	else {
		CurrentValue->SetText("");
	}

	if (stockRec->GetDifferenceValueOfDate(&d,ListBox.GetSelectedDate())) {
		DifferenceValue->SetText(emStocksFileModel::PaymentPriceToString(d));
	}
	else {
		DifferenceValue->SetText("");
	}
}


bool emStocksItemPanel::ValidateNumber(
	const emTextField & textField, int & pos, int & removeLen,
	emString & insertText, void * context
)
{
	static const int DECIMAL_POINT='.';
	static const int DECIMAL_POINT_REPLACE=',';
	const char * p;
	int i,c;
	bool dpFound;

	dpFound=false;
	p=strchr(textField.GetText().Get(),DECIMAL_POINT);
	if (p) {
		i=p-textField.GetText().Get();
		if (i<pos || i>=pos+removeLen) dpFound=true;
	}

	for (i=insertText.GetLen()-1; i>=0; i--) {
		c=insertText[i];
		if (c>='0' && c<='9') continue;
		if (c==DECIMAL_POINT && !dpFound) {
			dpFound=true;
			continue;
		}
		if (c==DECIMAL_POINT_REPLACE && !dpFound) {
			insertText.Replace(i,1,DECIMAL_POINT);
			dpFound=true;
			continue;
		}
		insertText.Remove(i,1);
	}

	i=emMax(32-textField.GetTextLen()+removeLen,0);
	if (insertText.GetLen()>i) insertText.Remove(i,insertText.GetLen()-i);

	return true;
}


bool emStocksItemPanel::ValidateDate(
	const emTextField & textField, int & pos, int & removeLen,
	emString & insertText, void * context
)
{
	const char * p;
	int i,c,mCount;

	mCount=0;
	for (p=textField.GetText().Get(); ; p++) {
		p=strchr(p,'-');
		if (!p) break;
		i=p-textField.GetText().Get();
		if (i<pos || i>=pos+removeLen) mCount++;
	}

	for (i=insertText.GetLen()-1; i>=0; i--) {
		c=insertText[i];
		if (c>='0' && c<='9') continue;
		if (c=='-' && mCount<2) { mCount++; continue; }
		insertText.Remove(i,1);
	}

	i=emMax(32-textField.GetTextLen()+removeLen,0);
	if (insertText.GetLen()>i) insertText.Remove(i,insertText.GetLen()-i);

	return true;
}


emStocksItemPanel::CategoryPanel::CategoryPanel(
	ParentArg parent, const emString & name, emStocksItemPanel & itemPanel,
	CategoryType type, const emString & caption, const emString & description,
	const emImage & icon
) :
	emLinearGroup(parent,name,caption,description,icon),
	ItemPanel(itemPanel),
	Type(type),
	TextField(NULL),
	ListBox(NULL),
	UpdateControlsNeeded(false),
	HaveListBoxContent(false)
{
	SetBorderType(OBT_INSTRUMENT,IBT_INPUT_FIELD);
	SetOrientationThresholdTallness(0.5);
	SetChildWeight(1,0.2);
	AddWakeUpSignal(ItemPanel.FileModel.GetChangeSignal());
	AddWakeUpSignal(ItemPanel.Config.GetChangeSignal());
}


emStocksItemPanel::CategoryPanel::~CategoryPanel()
{
}


bool emStocksItemPanel::CategoryPanel::Cycle()
{
	emStocksRec::StockRec * stockRec;
	emStringRec * categoryRec;
	bool busy;

	busy=emLinearGroup::Cycle();

	stockRec=ItemPanel.GetStockRec();
	if (!stockRec || !IsAutoExpanded()) return busy;
	categoryRec=GetCategoryRec(stockRec);

	if (
		IsSignaled(ItemPanel.FileModel.GetChangeSignal()) ||
		IsSignaled(ItemPanel.Config.GetChangeSignal())
	) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(TextField->GetTextSignal())) {
		if (categoryRec->Get() != TextField->GetText()) {
			PreservedCategory=TextField->GetText();
			categoryRec->Set(TextField->GetText());
		}
	}

	if (IsSignaled(ListBox->GetSelectionSignal())) {
		if (ListBox->GetSelectedIndex()>=0) {
			if (PreservedCategory.IsEmpty()) PreservedCategory=categoryRec->Get();
			categoryRec->Set(ListBox->GetItemName(ListBox->GetSelectedIndex()));
		}
	}

	if (UpdateControlsNeeded) UpdateControls();

	return busy;
}


void emStocksItemPanel::CategoryPanel::Notice(NoticeFlags flags)
{
	bool b;

	emLinearGroup::Notice(flags);

	if (flags&NF_VIEWING_CHANGED) {
		b=(GetViewCondition() > 1000.0);
		if (HaveListBoxContent!=b) {
			HaveListBoxContent=b;
			UpdateControlsNeeded=true;
			WakeUp();
		}
	}
}


void emStocksItemPanel::CategoryPanel::AutoExpand()
{
	emLook look;

	emLinearGroup::AutoExpand();

	WTLayout=new emLinearLayout(this,"wt");
	WTLayout->SetVertical();

	WarningLabel=new emLabel(WTLayout,"");
	look=WarningLabel->GetLook();
	look.SetFgColor(0xCC5533FF);
	WarningLabel->SetLook(look);

	TextField=new emTextField(WTLayout,"t");
	TextField->SetBorderType(OBT_NONE,IBT_NONE);
	TextField->SetEditable();
	AddWakeUpSignal(TextField->GetTextSignal());

	ListBox=new emListBox(this,"l","Used Names");
	ListBox->SetBorderType(OBT_NONE,IBT_CUSTOM_RECT);
	look=ListBox->GetLook();
	look.SetBgColor(look.GetInputBgColor());
	look.SetFgColor(look.GetInputFgColor());
	ListBox->SetLook(look);
	AddWakeUpSignal(ListBox->GetSelectionSignal());

	UpdateControlsNeeded=true;
	WakeUp();
}


void emStocksItemPanel::CategoryPanel::AutoShrink()
{
	TextField=NULL;
	ListBox=NULL;
	emLinearGroup::AutoShrink();
}


void emStocksItemPanel::CategoryPanel::UpdateControls()
{
	emStocksRec::StockRec * stockRec;
	emStringRec * categoryRec;
	const emString * cat;
	bool listChanged;
	int i,j;

	UpdateControlsNeeded=false;
	stockRec=ItemPanel.GetStockRec();
	if (!stockRec || !IsAutoExpanded()) return;
	categoryRec=GetCategoryRec(stockRec);

	if (GetCategoriesConfigRec().GetCount()>0) {
		WTLayout->SetChildWeight(0,0.2);
		WarningLabel->SetCaption(
			"This category type is filtered - a change can make this stock invisible! "
		);
	}
	else {
		WTLayout->SetChildWeight(0,0.0001);
		WarningLabel->SetCaption(emString());
	}

	TextField->SetText(categoryRec->Get());

	if (!HaveListBoxContent) {
		ListBox->ClearItems();
		return;
	}

	emAnything obsolete = emCastAnything<bool>(true);
	for (i=ListBox->GetItemCount()-1; i>=0; i--) {
		ListBox->SetItemData(i,obsolete);
	}

	listChanged=false;

	for (i=ItemPanel.FileModel.Stocks.GetCount()-1; i>=-1; i--) {
		if (i<0) {
			if (PreservedCategory.IsEmpty()) continue;
			cat=&PreservedCategory;
		}
		else {
			cat=&GetCategoryRec(&ItemPanel.FileModel.Stocks[i])->Get();
		}

		j=ListBox->GetItemIndex(*cat);
		if (j>=0) {
			ListBox->SetItemData(j,emAnything());
		}
		else {
			ListBox->AddItem(*cat,cat->IsEmpty()?emString("<blank>"):*cat);
			listChanged=true;
		}
	}

	for (i=ListBox->GetItemCount()-1; i>=0; i--) {
		if (emCastAnything<bool>(ListBox->GetItemData(i))) {
			ListBox->RemoveItem(i);
			listChanged=true;
		}
	}

	if (listChanged) {
		ListBox->SortItems(CompareItems,this);
	}

	ListBox->SetSelectedIndex(ListBox->GetItemIndex(categoryRec->Get()));
}


int emStocksItemPanel::CategoryPanel::CompareItems(
	const emString & item1name, const emString &, const emAnything &,
	const emString & item2name, const emString &, const emAnything &,
	void *
)
{
	return strcoll(item1name.Get(),item2name.Get());
}


emStringRec * emStocksItemPanel::CategoryPanel::GetCategoryRec(
	emStocksRec::StockRec * stockRec
)
{
	switch (Type) {
		case CT_COUNTRY: return &stockRec->Country;
		case CT_SECTOR : return &stockRec->Sector;
		default        : return &stockRec->Collection;
	}
}


emTArrayRec<emStringRec> &
	emStocksItemPanel::CategoryPanel::GetCategoriesConfigRec()
{
	switch (Type) {
		case CT_COUNTRY: return ItemPanel.Config.VisibleCountries;
		case CT_SECTOR : return ItemPanel.Config.VisibleSectors;
		default        : return ItemPanel.Config.VisibleCollections;
	}
}
