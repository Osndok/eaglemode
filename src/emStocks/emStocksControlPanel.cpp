//------------------------------------------------------------------------------
// emStocksControlPanel.cpp
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

#include <emStocks/emStocksControlPanel.h>
#include <emCore/emInstallInfo.h>


emStocksControlPanel::emStocksControlPanel(
	ParentArg parent, const emString & name,
	emStocksFileModel & fileModel,
	emStocksConfig & config,
	emStocksListBox & listBox
)
	: emLinearGroup(parent,name,"emStocks"),
	FileModel(&fileModel),
	Config(&config),
	ListBox(&listBox),
	UpdateControlsNeeded(true),
	ApiScript(NULL),
	ApiScriptInterpreter(NULL),
	ApiKey(NULL),
	WebBrowser(NULL),
	AutoUpdateDates(NULL),
	TriggeringOpensWebPage(NULL),
	ChartPeriod(NULL),
	MinVisibleInterest(NULL),
	VisibleCountries(NULL),
	VisibleSectors(NULL),
	VisibleCollections(NULL),
	Sorting(NULL),
	OwnedSharesFirst(NULL),
	FetchSharePrices(NULL),
	DeleteSharePrices(NULL),
	GoBackInHistory(NULL),
	GoForwardInHistory(NULL),
	SelectedDate(NULL),
	TotalPurchaseValue(NULL),
	TotalCurrentValue(NULL),
	TotalDifferenceValue(NULL),
	NewStock(NULL),
	CutStocks(NULL),
	CopyStocks(NULL),
	PasteStocks(NULL),
	DeleteStocks(NULL),
	SelectAll(NULL),
	ClearSelection(NULL),
	SetHighInterest(NULL),
	SetMediumInterest(NULL),
	SetLowInterest(NULL),
	ShowFirstWebPages(NULL),
	ShowAllWebPages(NULL),
	FindSelected(NULL),
	SearchText(NULL),
	FindNext(NULL),
	FindPrevious(NULL)
{
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(Config->GetChangeSignal());
	AddWakeUpSignal(ListBox->GetSelectionSignal());
	AddWakeUpSignal(ListBox->GetSelectedDateSignal());
	WakeUp();
}


emStocksControlPanel::~emStocksControlPanel()
{
}


bool emStocksControlPanel::Cycle()
{
	bool busy;

	busy=emLinearGroup::Cycle();

	if (!FileModel || !Config || !ListBox || !IsAutoExpanded()) {
		return busy;
	}

	if (IsSignaled(FileModel->GetChangeSignal())) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(Config->GetChangeSignal())) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(ListBox->GetSelectionSignal())) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(ListBox->GetSelectedDateSignal())) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(ApiKey->GetTextSignal())) {
		Config->ApiKey=ApiKey->GetText();
	}

	if (IsSignaled(AutoUpdateDates->GetCheckSignal())) {
		Config->AutoUpdateDates=AutoUpdateDates->IsChecked();
	}

	if (IsSignaled(TriggeringOpensWebPage->GetCheckSignal())) {
		Config->TriggeringOpensWebPage=TriggeringOpensWebPage->IsChecked();
	}

	if (IsSignaled(ChartPeriod->GetValueSignal())) {
		Config->ChartPeriod=ChartPeriod->GetValue();
	}

	if (IsSignaled(MinVisibleInterest->GetCheckSignal())) {
		Config->MinVisibleInterest=MinVisibleInterest->GetCheckIndex();
	}

	if (IsSignaled(Sorting->GetCheckSignal())) {
		Config->Sorting=Sorting->GetCheckIndex();
	}

	if (IsSignaled(OwnedSharesFirst->GetClickSignal())) {
		Config->OwnedSharesFirst=OwnedSharesFirst->IsChecked();
	}

	if (IsSignaled(FetchSharePrices->GetClickSignal())) {
		ListBox->StartToFetchSharePrices();
	}

	if (IsSignaled(DeleteSharePrices->GetClickSignal())) {
		ListBox->DeleteSharePrices();
	}

	if (IsSignaled(GoBackInHistory->GetClickSignal())) {
		ListBox->GoBackInHistory();
	}

	if (IsSignaled(GoForwardInHistory->GetClickSignal())) {
		ListBox->GoForwardInHistory();
	}

	if (IsSignaled(SelectedDate->GetTextSignal())) {
		ListBox->SetSelectedDate(SelectedDate->GetText());
	}

	if (IsSignaled(NewStock->GetClickSignal())) {
		ListBox->NewStock();
	}

	if (IsSignaled(CutStocks->GetClickSignal())) {
		ListBox->CutStocks();
	}

	if (IsSignaled(CopyStocks->GetClickSignal())) {
		ListBox->CopyStocks();
	}

	if (IsSignaled(PasteStocks->GetClickSignal())) {
		ListBox->PasteStocks();
	}

	if (IsSignaled(DeleteStocks->GetClickSignal())) {
		ListBox->DeleteStocks();
	}

	if (IsSignaled(SelectAll->GetClickSignal())) {
		ListBox->SelectAll();
	}

	if (IsSignaled(ClearSelection->GetClickSignal())) {
		ListBox->ClearSelection();
	}

	if (IsSignaled(SetLowInterest->GetClickSignal())) {
		ListBox->SetInterest(emStocksFileModel::LOW_INTEREST);
	}

	if (IsSignaled(SetMediumInterest->GetClickSignal())) {
		ListBox->SetInterest(emStocksFileModel::MEDIUM_INTEREST);
	}

	if (IsSignaled(SetHighInterest->GetClickSignal())) {
		ListBox->SetInterest(emStocksFileModel::HIGH_INTEREST);
	}

	if (IsSignaled(ShowFirstWebPages->GetClickSignal())) {
		ListBox->ShowFirstWebPages();
	}

	if (IsSignaled(ShowAllWebPages->GetClickSignal())) {
		ListBox->ShowAllWebPages();
	}

	if (IsSignaled(FindSelected->GetClickSignal())) {
		ListBox->FindSelected();
	}

	if (IsSignaled(SearchText->GetTextSignal())) {
		Config->SearchText=SearchText->GetText();
	}

	if (IsSignaled(FindNext->GetClickSignal())) {
		ListBox->FindNext();
	}

	if (IsSignaled(FindPrevious->GetClickSignal())) {
		ListBox->FindPrevious();
	}

	if (UpdateControlsNeeded) UpdateControls();

	return busy;
}


void emStocksControlPanel::AutoExpand()
{
	emLabel * text;
	emLinearGroup * about, * prices, * search;
	emLinearLayout * aboutPrefsFilterSort,  * pricesCmdsFind;
	emLinearLayout * aboutPrefsFilter, * aboutPrefs;
	emLinearLayout * pricesCmds, * pr1, * pr2, * dsp;
	emPackGroup * prefs;
	emRasterLayout * prefs1, * prefs2;
	emRasterGroup * flt, * cmds;
	emTunnel * tunnel;

	emLinearGroup::AutoExpand();
	SetChildWeight(1,0.88);
	SetSpaceH(0.015);
	SetSpaceV(0.015);
	SetOrientationThresholdTallness(1.0);

	aboutPrefsFilterSort=new emLinearLayout(this,"apfs");
	aboutPrefsFilterSort->SetChildWeight(1,0.95);
	aboutPrefsFilterSort->SetSpaceH(0.028);
	aboutPrefsFilterSort->SetSpaceV(0.028);
	aboutPrefsFilterSort->SetOrientationThresholdTallness(1.0);

	pricesCmdsFind=new emLinearLayout(this,"pcf");
	pricesCmdsFind->SetChildWeight(1,0.22);
	pricesCmdsFind->SetSpaceH(0.032);
	pricesCmdsFind->SetSpaceV(0.032);
	pricesCmdsFind->SetOrientationThresholdTallness(0.3);

	aboutPrefsFilter=new emLinearLayout(aboutPrefsFilterSort,"apf");
	aboutPrefsFilter->SetChildWeight(1,0.83);
	aboutPrefsFilter->SetSpaceH(0.06);
	aboutPrefsFilter->SetSpaceV(0.06);
	aboutPrefsFilter->SetOrientationThresholdTallness(0.8);

	aboutPrefs=new emLinearLayout(aboutPrefsFilter,"ap");
	aboutPrefs->SetChildWeight(1,2.0);
	aboutPrefs->SetSpaceH(0.115);
	aboutPrefs->SetSpaceV(0.115);
	aboutPrefs->SetOrientationThresholdTallness(1.0);

	pricesCmds=new emLinearLayout(pricesCmdsFind,"pc");
	pricesCmds->SetChildWeight(1,2.1);
	pricesCmds->SetSpaceH(0.04);
	pricesCmds->SetSpaceV(0.04);
	pricesCmds->SetOrientationThresholdTallness(1.0);

	about=new emLinearGroup(aboutPrefs,"about","About emStocks");
	text=new emLabel(about,"text",emString::Format(
		"INTRODUCTION\n"
		"\n"
		"emStocks is a plugin application for managing a stock watchlist. Here you can\n"
		"add and watch stocks of which you want to sell or purchase shares. For each\n"
		"stock you can enter lots of information like country, section, comments, web\n"
		"page URLs, number of shares, trade price, trade date, expected dividend, and the\n"
		"desired price for a trade. Charts are showing the share prices on a daily basis.\n"
		"Colored bars in the charts give a quick overview of success or failure in\n"
		"achieving desired prices. Several filter and sorting functions help to find the\n"
		"next trade.\n"
		"\n"
		"Since emStocks stores only one share price per day for each stock, this software\n"
		"can be useful only for someone who does at most a few trades per month. It is\n"
		"not suitable for day trading.\n"
		"\n"
		"\n"
		"API SCRIPT\n"
		"\n"
		"An essential part of emStocks is its ability to fetch share prices from the\n"
		"internet. There are some web sites out there with so called stock APIs which\n"
		"allow to download share prices freely or for money. To request the prices of one\n"
		"stock from such a service, emStocks runs a so-called API script. This is a small\n"
		"program written in Perl or any other scripting language. It acts as an interface\n"
		"or adapter to the service. Here are the steps of how you can get it working:\n"
		"\n"
		"1.) Find a stock API on the internet and register there and get an API key. If\n"
		"you are lucky, then an API script for that service already exists. Please have a\n"
		"look at the following directory:\n"
		"\n"
		"  %s\n"
		"\n"
		"2.) If you don't find a suitable script there, then you have to write your own\n"
		"one. You may take the existing scripts as a start point. The script gets three\n"
		"arguments when it is run:\n"
		"\n"
		"  1. The symbol of the stock (stock ticker symbol).\n"
		"  2. The start date in the format YYYY-MM-DD.\n"
		"  3. The API key.\n"
		"\n"
		"As a result, the API script must print one line for each retrieved price to\n"
		"stdout, with the date as YYYY-MM-DD in front. The order does not matter and days\n"
		"can be left out (e.g. weekends). Example:\n"
		"\n"
		"  2022-12-15 23.45\n"
		"  2022-12-16 22.77\n"
		"  2022-12-19 24.14\n"
		"\n"
		"3.) Now go to the emStocks Preferences (in the control panel) and enter the path\n"
		"to the API script, the API script interpreter, and the API key. On Windows you\n"
		"will probably have to install the interpreter (e.g. install Strawberry Perl for\n"
		"Perl scripts). On other operating systems the interpreter may already be there.\n"
		"\n"
		"4.) Don't forget to set the correct symbols in the stocks. Each service usually\n"
		"has some interface for finding the symbol to a stock.\n"
		"\n"
		"5.) Now press the Fetch Prices button and see if it works: Charts and price\n"
		"fields should be filled or updated.\n"
		"\n"
		"\n"
		"FILES\n"
		"\n"
		"The list of stocks is stored in a file whose name ends with .emStocks. If you\n"
		"want to work with multiple lists, you can create multiple emStocks files. To\n"
		"create a new emStocks file, you can simply create an ordinary empty file\n"
		"anywhere in your file system and give it a name that ends with .emStocks. Then\n"
		"zoom into it and start adding stocks.\n"
		"\n"
		"  ***********************************************************************\n"
		"  *** Important: Please make daily backups of your emStocks file(s).  ***\n"
		"  *** emStocks has no undo function (except for the text fields), and ***\n"
		"  *** if the program crashes or is aborted while it saves to the      ***\n"
		"  *** file, the file can get damaged.                                 ***\n"
		"  ***********************************************************************\n"
		"\n"
		"The file for the current list is:\n"
		"\n"
		"  %s\n"
		"\n"
		"Oh and if it ever happens that you think stocks have vanished unexpectedly from\n"
		"the list, please check the filter settings first.",
		BreakPath(emGetInstallPath(EM_IDT_RES,"emStocks","scripts"),"\n  ",78).Get(),
		BreakPath(FileModel->GetFilePath(),"\n  ",78).Get()
	));
	text->SetLabelAlignment(EM_ALIGN_TOP_LEFT);

	prefs=new emPackGroup(aboutPrefs,"Preferences","Preferences");
	prefs->SetChildWeight(1,0.4);
	prefs->SetChildWeight(2,0.6);
	prefs->SetPrefChildTallness(0,0.4);
	prefs->SetPrefChildTallness(1,0.2);
	prefs->SetPrefChildTallness(2,0.2);
	prefs1=new emRasterLayout(prefs,"1");
	prefs1->SetPrefChildTallness(0.09);
	prefs2=new emRasterLayout(prefs,"2");
	prefs2->SetPrefChildTallness(0.09);

	ApiScript=new FileFieldPanel(
		prefs1,"ApiScript",*this,FT_SCRIPT,
		"API Script for Fetching Prices",
		"Path of the API script that connects to the internet for fetching prices. It is run by emStocks when\n"
		"fetching the share prices of a stock. The script gets three arguments:\n"
		"\n"
		"  1. The symbol of the stock (stock ticker symbol).\n"
		"  2. The start date in the format YYYY-MM-DD.\n"
		"  3. The API key.\n"
		"\n"
		"As a result, the API script must print one line for each retrieved price to stdout, with the date as\n"
		"YYYY-MM-DD in front. The order does not matter and days can be left out (e.g. weekends). Example:\n"
		"\n"
		"  2022-12-15 23.45\n"
		"  2022-12-16 22.77\n"
		"  2022-12-19 24.14"
	);

	ApiScriptInterpreter=new FileFieldPanel(
		prefs1,"ApiScriptInterpreter",*this,FT_INTERPRETER,
		"Interpreter for API Script",
		"Executable of the interpreter to be used to run the API script."
	);

	ApiKey=new emTextField(
		prefs1,"ApiKey","API Key",
		"The personal API key for the stock API. This is given as the third argument\n"
		"to the API script.\n"
		"\n"
		"WARNING: The API key is stored as plain text in a configuration file in the\n"
		"home directory. If the home directory is not encrypted and you want to keep\n"
		"the key more secret, then please do not enter it here and modify the API\n"
		"script in a way that it retrieves the key from a secret place, for example\n"
		"from an encrypted file system or a password manager."
	);
	ApiKey->SetEditable();
	AddWakeUpSignal(ApiKey->GetTextSignal());

	WebBrowser=new FileFieldPanel(
		prefs1,"WebBrowser",*this,FT_BROWSER,"Web Browser",
		"Executable of the preferred web browser"
	);

	AutoUpdateDates=new emCheckBox(
		prefs2,"AutoUpdateDates","Auto Update Dates",
		"Whether to update trade dates and inquiry dates automatically on\n"
		"change of related data. For example, if a purchase price is edited,\n"
		"then the purchase date is set to today."
	);
	AutoUpdateDates->SetNoEOI();
	AddWakeUpSignal(AutoUpdateDates->GetCheckSignal());

	TriggeringOpensWebPage=new emCheckBox(
		prefs2,"TriggeringOpensWebPage","Triggering Opens Web Page",
		"Whether a double-click on a stock (border or title) opens\n"
		"the first web page of that stock in the web browser."
	);
	TriggeringOpensWebPage->SetNoEOI();
	AddWakeUpSignal(TriggeringOpensWebPage->GetCheckSignal());

	ChartPeriod=new emScalarField(
		prefs,"ChartPeriod","Chart Period",
		"Time range of the charts."
	);
	ChartPeriod->SetEditable();
	ChartPeriod->SetMinMaxValues(
		emStocksConfig::PT_1_WEEK,
		emStocksConfig::PT_20_YEARS
	);
	ChartPeriod->SetTextOfValueFunc(ChartPeriodTextOfValue);
	ChartPeriod->SetTextBoxTallness(0.8);
	AddWakeUpSignal(ChartPeriod->GetValueSignal());

	flt=new emRasterGroup(aboutPrefsFilter,"Filter","Filter");
	flt->SetPrefChildTallness(0.5);

	MinVisibleInterest=new emRadioButton::LinearGroup(
		flt,"MinVisibleInterest","Minimum Visible Interest",
		"Minimum level of interest whose stocks should be visible."
	);
	new emRadioButton(
		MinVisibleInterest,"high","High","Hotkey: Shift+Alt+H"
	);
	new emRadioButton(
		MinVisibleInterest,"medium","Medium","Hotkey: Shift+Alt+M"
	);
	new emRadioButton(
		MinVisibleInterest,"low","Low","Hotkey: Shift+Alt+L"
	);
	AddWakeUpSignal(MinVisibleInterest->GetCheckSignal());

	VisibleCountries=new CategoryPanel(
		flt,"VisibleCountries",*this,CT_COUNTRY,Config->VisibleCountries,
		"Visible Countries",
		"Choice of countries whose stocks should be visible."
	);
	VisibleSectors=new CategoryPanel(
		flt,"VisibleSectors",*this,CT_SECTOR,Config->VisibleSectors,
		"Visible Sectors",
		"Choice of sectors whose stocks should be visible."
	);
	VisibleCollections=new CategoryPanel(
		flt,"VisibleCollections",*this,CT_COLLECTION,Config->VisibleCollections,
		"Visible Collections",
		"Choice of collections whose stocks should be visible."
	);

	Sorting=new emRadioButton::RasterGroup(
		aboutPrefsFilterSort,"Sorting","Sorting"
	);
	new emRadioButton(
		Sorting,"SortByName","Sort By Name",
		"Sort the stocks by name.\n"
		"\n"
		"Hotkey: Shift+Alt+N"
	);
	new emRadioButton(
		Sorting,"SortByTradeDate","Sort By Trade Date",
		"Sort the stocks by trade date.\n"
		"\n"
		"Hotkey: Shift+Alt+T"
	);
	new emRadioButton(
		Sorting,"SortByInquiryDate","Sort By Inquiry Date",
		"Sort the stocks by inquiry date.\n"
		"\n"
		"Hotkey: Shift+Alt+I"
	);
	new emRadioButton(
		Sorting,"SortByAchievement","Sort By Achievement",
		"Sort the stocks by the percentage difference between the price\n"
		"on the selected date and the desired price, in reverse order.\n"
		"Thus, this sorts by how far the desired price is reached on the\n"
		"selected date.\n"
		"\n"
		"Hotkey: Shift+Alt+A"
	);
	new emRadioButton(
		Sorting,"SortByOneWeekRise","Sort By One-Week Rise",
		"Sort the stocks by stock price increase over the past week.\n"
		"\n"
		"Hotkey: Shift+Alt+1"
	);
	new emRadioButton(
		Sorting,"SortByThreeWeekRise","Sort By Three-Week Rise",
		"Sort the stocks by stock price increase over the last three weeks.\n"
		"\n"
		"Hotkey: Shift+Alt+3"
	);
	new emRadioButton(
		Sorting,"SortByNineWeekRise","Sort By Nine-Week Rise",
		"Sort the stocks by stock price increase over the last nine weeks.\n"
		"\n"
		"Hotkey: Shift+Alt+9"
	);
	new emRadioButton(
		Sorting,"SortByDividend","Sort By Dividend",
		"Sort the stocks by dividend.\n"
		"\n"
		"Hotkey: Shift+Alt+D"
	);
	new emRadioButton(
		Sorting,"SortByPurchaseValue","Sort By Purchase Value",
		"Sort the stocks with owned shares by purchase value.\n"
		"\n"
		"Hotkey: Shift+Alt+P"
	);
	new emRadioButton(
		Sorting,"SortByValue","Sort By Value",
		"Sort the stocks with owned shares by value on the selected date.\n"
		"\n"
		"Hotkey: Shift+Alt+V"
	);
	new emRadioButton(
		Sorting,"SortByDifference","Sort By Difference",
		"Sort the stocks with owned shares by the difference between\n"
		"the purchase value and the value on the selected date.\n"
		"\n"
		"Hotkey: Shift+Alt+F"
	);
	AddWakeUpSignal(Sorting->GetCheckSignal());

	OwnedSharesFirst=new emCheckButton(
		Sorting,"OwnedSharesFirst","Owned Shares First",
		"Always have stocks with owned shares at the beginning,\n"
		"regardless of the other sort criterions.\n"
		"\n"
		"Hotkey: Shift+Alt+O"
	);
	AddWakeUpSignal(OwnedSharesFirst->GetClickSignal());

	prices=new emLinearGroup(pricesCmds,"Prices","Prices");
	prices->SetOrientationThresholdTallness(1.0);
	pr1=new emLinearLayout(prices,"pr1");
	pr1->SetChildWeight(0,2.1);
	pr1->SetChildWeight(1,0.4);
	pr2=new emLinearLayout(prices,"pr2");

	FetchSharePrices=new emButton(
		pr1,"FetchSharePrices","Fetch\nPrices",
		"Run the API script for each visible stock (that has a non-empty symbol)\n"
		"to request the daily stock prices from last retrieved date up to today.\n"
		"This fills the charts best possible. The selected date is automatically\n"
		"set to the latest day for which prices are retrieved.\n"
		"\n"
		"Hotkey: Ctrl+P"
	);
	FetchSharePrices->SetBorderScaling(0.5);
	FetchSharePrices->SetCaptionAlignment(EM_ALIGN_CENTER);
	AddWakeUpSignal(FetchSharePrices->GetClickSignal());

	dsp=new emLinearLayout(pr1,"DeleteSharePrices");
	dsp->SetOuterSpace(0.5,0.0);
	tunnel=new emTunnel(dsp,"DeleteSharePrices","Delete Prices");
	tunnel->SetChildTallness(0.2);
	tunnel->SetDepth(20.0);
	DeleteSharePrices=new emButton(
		tunnel,"DeleteSharePrices","Delete Prices",
		"Delete all saved prices of the visible stocks, so that they\n"
		"are fetched again when the Fetch Prices function is triggered.\n"
		"This can be useful for debugging and testing, or when the API\n"
		"script is changed or repaired."
	);
	AddWakeUpSignal(DeleteSharePrices->GetClickSignal());

	GoBackInHistory=new emButton(
		pr1,"GoBackInHistory","Go Back In History",
		"Set the selected date to the previous day for which\n"
		"any stock prices are known.\n"
		"\n"
		"Hotkey: Ctrl+J"
	);
	AddWakeUpSignal(GoBackInHistory->GetClickSignal());

	GoForwardInHistory=new emButton(
		pr1,"GoForwardInHistory","Go Forward In History",
		"Set the selected date to the next day for which\n"
		"any stock prices are known.\n"
		"\n"
		"Hotkey: Ctrl+K"
	);
	AddWakeUpSignal(GoForwardInHistory->GetClickSignal());

	SelectedDate=new emTextField(
		pr2,"SelectedDate","Selected Date",
		"Currently selected date for which stock prices are displayed."
	);
	SelectedDate->SetEditable();
	SelectedDate->SetValidateFunc(&ValidateDate,this);
	AddWakeUpSignal(SelectedDate->GetTextSignal());
	TotalPurchaseValue=new emTextField(
		pr2,"TotalPurchaseValue","Total Purchase Value",
		"Sum of the purchase values of the owned shares of the visible stocks."
	);
	TotalCurrentValue=new emTextField(
		pr2,"TotalCurrentValue","Total Value On Selected Date",
		"Sum of the values of the owned shares of the visible stocks on\n"
		"the selected date. This field remains empty if the price of any\n"
		"related stock is not known for the selected date."
	);
	TotalDifferenceValue=new emTextField(
		pr2,"TotalDifferenceValue","Total Difference Value",
		"Difference between total purchase value and total value on selected date."
	);

	cmds=new emRasterGroup(pricesCmds,"Commands","Commands");

	NewStock=new emButton(
		cmds,"NewStock","New",
		"Create a new empty stock.\n"
		"\n"
		"Hotkey: Ctrl+N"
	);
	AddWakeUpSignal(NewStock->GetClickSignal());

	CutStocks=new emButton(
		cmds,"CutStocks","Cut",
		"Move the selected stocks to the clipboard.\n"
		"\n"
		"Hotkey: Ctrl+X"
	);
	AddWakeUpSignal(CutStocks->GetClickSignal());

	CopyStocks=new emButton(
		cmds,"CopyStocks","Copy",
		"Copy the selected stocks to the clipboard.\n"
		"\n"
		"Hotkey: Ctrl+C"
	);
	AddWakeUpSignal(CopyStocks->GetClickSignal());

	PasteStocks=new emButton(
		cmds,"PasteStocks","Paste",
		"Insert stocks from the clipboard.\n"
		"\n"
		"Hotkey: Ctrl+V"
	);
	AddWakeUpSignal(PasteStocks->GetClickSignal());

	DeleteStocks=new emButton(
		cmds,"DeleteStocks","Delete",
		"Delete the selected stocks.\n"
		"\n"
		"Hotkey: Delete"
	);
	AddWakeUpSignal(DeleteStocks->GetClickSignal());

	SelectAll=new emButton(
		cmds,"SelectAll","Select All",
		"Select all visible stocks.\n"
		"\n"
		"Hotkey: Ctrl+A"
	);
	AddWakeUpSignal(SelectAll->GetClickSignal());

	ClearSelection=new emButton(
		cmds,"ClearSelection","Clear Selection",
		"Deselect all stocks.\n"
		"\n"
		"Hotkey: Shift+Ctrl+A"
	);
	AddWakeUpSignal(ClearSelection->GetClickSignal());

	SetHighInterest=new emButton(
		cmds,"SetHighInterest","Set High Interest",
		"Set the selected stocks to high interest.\n"
		"\n"
		"Hotkey: Alt+H"
	);
	AddWakeUpSignal(SetHighInterest->GetClickSignal());

	SetMediumInterest=new emButton(
		cmds,"SetMediumInterest","Set Medium Interest",
		"Set the selected stocks to medium interest.\n"
		"\n"
		"Hotkey: Alt+M"
	);
	AddWakeUpSignal(SetMediumInterest->GetClickSignal());

	SetLowInterest=new emButton(
		cmds,"SetLowInterest","Set Low Interest",
		"Set the selected stocks to low interest.\n"
		"\n"
		"Hotkey: Alt+L"
	);
	AddWakeUpSignal(SetLowInterest->GetClickSignal());

	ShowFirstWebPages=new emButton(
		cmds,"ShowFirstWebPages","Show First Web Pages",
		"Open the first web page of each selected stock in the web browser.\n"
		"\n"
		"Hotkey: Ctrl+W"
	);
	AddWakeUpSignal(ShowFirstWebPages->GetClickSignal());

	ShowAllWebPages=new emButton(
		cmds,"ShowAllWebPages","Show All Web Pages",
		"Open all web pages of the selected stocks in the web browser.\n"
		"\n"
		"Hotkey: Shift+Ctrl+W"
	);
	AddWakeUpSignal(ShowAllWebPages->GetClickSignal());

	search=new emLinearGroup(pricesCmdsFind,"Search","Search");

	FindSelected=new emButton(
		search,"FindSelected","Find Selected",
		"Set the search text to the currently selected text and\n"
		"find the next stock that contains that search text.\n"
		"\n"
		"Hotkey: Ctrl+H"
	);
	AddWakeUpSignal(FindSelected->GetClickSignal());

	SearchText=new emTextField(
		search,"SearchText","Search Text",
		"The text to be searched for by the find functions."
	);
	SearchText->SetEditable();
	AddWakeUpSignal(SearchText->GetTextSignal());

	FindNext=new emButton(
		search,"FindNext","Find Next",
		"Find the next stock that contains the search text.\n"
		"\n"
		"Hotkey: Ctrl+G"
	);
	AddWakeUpSignal(FindNext->GetClickSignal());

	FindPrevious=new emButton(
		search,"FindPrevious","Find Previous",
		"Find the previous stock that contains the search text.\n"
		"\n"
		"Hotkey: Shift+Ctrl+G"
	);
	AddWakeUpSignal(FindPrevious->GetClickSignal());

	UpdateControlsNeeded=true;
	WakeUp();
}


void emStocksControlPanel::AutoShrink()
{
	ApiScript=NULL;
	ApiScriptInterpreter=NULL;
	ApiKey=NULL;
	WebBrowser=NULL;
	AutoUpdateDates=NULL;
	TriggeringOpensWebPage=NULL;
	ChartPeriod=NULL;
	MinVisibleInterest=NULL;
	VisibleCountries=NULL;
	VisibleSectors=NULL;
	VisibleCollections=NULL;
	Sorting=NULL;
	OwnedSharesFirst=NULL;
	FetchSharePrices=NULL;
	DeleteSharePrices=NULL;
	GoBackInHistory=NULL;
	GoForwardInHistory=NULL;
	SelectedDate=NULL;
	TotalPurchaseValue=NULL;
	TotalCurrentValue=NULL;
	TotalDifferenceValue=NULL;
	NewStock=NULL;
	CutStocks=NULL;
	CopyStocks=NULL;
	PasteStocks=NULL;
	DeleteStocks=NULL;
	SelectAll=NULL;
	ClearSelection=NULL;
	SetHighInterest=NULL;
	SetMediumInterest=NULL;
	SetLowInterest=NULL;
	ShowFirstWebPages=NULL;
	ShowAllWebPages=NULL;
	FindSelected=NULL;
	SearchText=NULL;
	FindNext=NULL;
	FindPrevious=NULL;

	emLinearGroup::AutoShrink();
}


void emStocksControlPanel::UpdateControls()
{
	emStocksRec::StockRec * stockRec;
	double d,totalPurchase,totalCurrent;
	bool totalPurchaseValid,totalCurrentValid;
	int i;

	UpdateControlsNeeded=false;
	if (!FileModel || !Config || !ListBox || !IsAutoExpanded()) return;

	ApiKey->SetText(Config->ApiKey);

	AutoUpdateDates->SetChecked(Config->AutoUpdateDates);

	TriggeringOpensWebPage->SetChecked(Config->TriggeringOpensWebPage);

	ChartPeriod->SetValue(Config->ChartPeriod.Get());

	MinVisibleInterest->SetCheckIndex(Config->MinVisibleInterest.Get());

	Sorting->SetCheckIndex(Config->Sorting.Get());

	OwnedSharesFirst->SetChecked(Config->OwnedSharesFirst);

	GoBackInHistory->SetEnableSwitch(
		!FileModel->GetPricesDateBefore(ListBox->GetSelectedDate()).IsEmpty()
	);

	GoForwardInHistory->SetEnableSwitch(
		!FileModel->GetPricesDateAfter(ListBox->GetSelectedDate()).IsEmpty()
	);

	SelectedDate->SetText(ListBox->GetSelectedDate());

	totalPurchase=totalCurrent=0.0;
	totalPurchaseValid=totalCurrentValid=true;
	for (i=0; i<FileModel->Stocks.GetCount(); i++) {
		stockRec=&FileModel->Stocks[i];
		if (!stockRec->OwningShares.Get()) continue;
		if (!ListBox->IsVisibleStock(*stockRec)) continue;
		if (stockRec->GetTradeValue(&d)) totalPurchase+=d;
		else totalPurchaseValid=false;
		if (stockRec->GetValueOfDate(&d,ListBox->GetSelectedDate())) totalCurrent+=d;
		else totalCurrentValid=false;
	}
	if (totalPurchaseValid) {
		TotalPurchaseValue->SetText(
			emStocksFileModel::PaymentPriceToString(totalPurchase)
		);
	}
	else {
		TotalPurchaseValue->SetText("");
	}
	if (totalCurrentValid) {
		TotalCurrentValue->SetText(
			emStocksFileModel::PaymentPriceToString(totalCurrent)
		);
	}
	else {
		TotalCurrentValue->SetText("");
	}
	if (totalPurchaseValid && totalCurrentValid) {
		TotalDifferenceValue->SetText(
			emStocksFileModel::PaymentPriceToString(totalCurrent-totalPurchase)
		);
	}
	else {
		TotalDifferenceValue->SetText("");
	}

	CutStocks->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	CopyStocks->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	DeleteStocks->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	SelectAll->SetEnableSwitch(ListBox->GetSelectionCount()<ListBox->GetItemCount());

	ClearSelection->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	SetHighInterest->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	SetMediumInterest->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	SetLowInterest->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	ShowFirstWebPages->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	ShowAllWebPages->SetEnableSwitch(ListBox->GetSelectionCount()>0);

	SearchText->SetText(Config->SearchText);

	FindNext->SetEnableSwitch(!Config->SearchText.Get().IsEmpty());

	FindPrevious->SetEnableSwitch(!Config->SearchText.Get().IsEmpty());
}


void emStocksControlPanel::ChartPeriodTextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	const char * p;

	switch (value) {
		case emStocksConfig::PT_1_WEEK  : p="1\nweek"  ; break;
		case emStocksConfig::PT_2_WEEKS : p="2\nweeks" ; break;
		case emStocksConfig::PT_1_MONTH : p="1\nmonth" ; break;
		case emStocksConfig::PT_3_MONTHS: p="3\nmonths"; break;
		case emStocksConfig::PT_6_MONTHS: p="6\nmonths"; break;
		case emStocksConfig::PT_1_YEAR  : p="1\nyear"  ; break;
		case emStocksConfig::PT_3_YEARS : p="3\nyears" ; break;
		case emStocksConfig::PT_5_YEARS : p="5\nyears" ; break;
		case emStocksConfig::PT_10_YEARS: p="10\nyears"; break;
		case emStocksConfig::PT_20_YEARS: p="20\nyears"; break;
		default                            : p="unknown" ; break;
	}
	snprintf(buf,bufSize,"%s",p);
	buf[bufSize-1]=0;
}


bool emStocksControlPanel::ValidateDate(
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


emString emStocksControlPanel::BreakPath(emString path, const emString & brkStr, int len)
{
	int l,i;

	l=path.GetLen();
	if (l<=len) return path;
	for (i=len-1; i>=0; i--) {
		if (
			path[i]=='/'
#if defined(_WIN32)
			|| path[i]=='\\'
#endif
		) break;
	}
	if (i>0) i=i+1; else i=len;
	return
		path.GetSubString(0,i)+
		brkStr+
		BreakPath(path.GetSubString(i,l-i),brkStr,len)
	;
}


emStocksControlPanel::FileFieldPanel::FileFieldPanel(
	ParentArg parent, const emString & name,
	emStocksControlPanel & controlPanel, FileFieldType type,
	const emString & caption, const emString & description,
	const emImage & icon
) :
	emLinearGroup(parent,name,caption,description,icon),
	ControlPanel(controlPanel),
	Type(type),
	TextField(NULL),
	FileSelectionBox(NULL),
	UpdateControlsNeeded(false)
{
	SetBorderType(OBT_INSTRUMENT,IBT_INPUT_FIELD);
	SetOrientationThresholdTallness(0.5);
	SetChildWeight(1,0.2);
	AddWakeUpSignal(ControlPanel.Config->GetChangeSignal());
}


emStocksControlPanel::FileFieldPanel::~FileFieldPanel()
{
}


bool emStocksControlPanel::FileFieldPanel::Cycle()
{
	emStringRec * fileRec;
	bool busy;

	busy=emLinearGroup::Cycle();

	fileRec=GetFileRec();
	if (!fileRec || !IsAutoExpanded()) return busy;

	if (IsSignaled(ControlPanel.Config->GetChangeSignal())) {
		UpdateControlsNeeded=true;
	}

	if (IsSignaled(TextField->GetTextSignal())) {
		fileRec->Set(TextField->GetText());
	}

	if (
		IsSignaled(FileSelectionBox->GetSelectionSignal()) &&
		fileRec->Get() != FileSelectionBox->GetSelectedPath() &&
		emIsRegularFile(FileSelectionBox->GetSelectedPath())
	) {
		fileRec->Set(FileSelectionBox->GetSelectedPath());
	}

	if (UpdateControlsNeeded) UpdateControls();

	return busy;
}


void emStocksControlPanel::FileFieldPanel::AutoExpand()
{
	emLook look;

	emLinearGroup::AutoExpand();

	TextField=new emTextField(this,"t");
	TextField->SetBorderType(OBT_NONE,IBT_NONE);
	TextField->SetEditable();
	AddWakeUpSignal(TextField->GetTextSignal());

	FileSelectionBox=new emFileSelectionBox(this,"b","Browse");
	FileSelectionBox->SetBorderType(OBT_NONE,IBT_CUSTOM_RECT);
	look=FileSelectionBox->GetLook();
	look.SetBgColor(look.GetInputBgColor());
	look.SetFgColor(look.GetInputFgColor());
	FileSelectionBox->SetLook(look);
	AddWakeUpSignal(FileSelectionBox->GetSelectionSignal());

	UpdateControlsNeeded=true;
	WakeUp();
}


void emStocksControlPanel::FileFieldPanel::AutoShrink()
{
	TextField=NULL;
	FileSelectionBox=NULL;
	emLinearGroup::AutoShrink();
}


void emStocksControlPanel::FileFieldPanel::UpdateControls()
{
	emStringRec * fileRec;

	UpdateControlsNeeded=false;

	fileRec=GetFileRec();
	if (!fileRec || !IsAutoExpanded()) return;

	TextField->SetText(fileRec->Get());

	if (fileRec->Get().IsEmpty() || emGetParentPath(fileRec->Get()).IsEmpty()) {
		switch (Type) {
			case FT_SCRIPT:
				FileSelectionBox->SetSelectedPath(
					emGetInstallPath(EM_IDT_RES,"emStocks","scripts")
				);
				break;
			default:
				FileSelectionBox->SetSelectedPath(
					emGetInstallPath(EM_IDT_HOME,"emStocks")
				);
				break;
		}
	}
	else {
		FileSelectionBox->SetSelectedPath(fileRec->Get());
	}
}


emStringRec * emStocksControlPanel::FileFieldPanel::GetFileRec()
{
	switch (Type) {
		case FT_SCRIPT     : return &ControlPanel.Config->ApiScript;
		case FT_INTERPRETER: return &ControlPanel.Config->ApiScriptInterpreter;
		default            : return &ControlPanel.Config->WebBrowser;
	}
}


emStocksControlPanel::CategoryPanel::CategoryPanel(
	ParentArg parent, const emString & name, emStocksControlPanel & controlPanel,
	CategoryType type, emTArrayRec<emStringRec> & categoriesConfigRec,
	const emString & caption, const emString & description, const emImage & icon
) :
	emListBox(parent,name,caption,description,icon,MULTI_SELECTION),
	emRecListener(&categoriesConfigRec),
	ControlPanel(controlPanel),
	Type(type),
	CategoriesConfigRec(categoriesConfigRec),
	NameOfAll("ALL_g0@p#$sKz8@L%vHZ"),
	UpdateItemsNeeded(false),
	UpdateSelectionNeeded(false),
	HaveListBoxContent(false)
{
	AddWakeUpSignal(GetSelectionSignal());
	AddWakeUpSignal(ControlPanel.FileModel->GetChangeSignal());
}


emStocksControlPanel::CategoryPanel::~CategoryPanel()
{
}


bool emStocksControlPanel::CategoryPanel::Cycle()
{
	bool busy;

	busy=emListBox::Cycle();

	if (IsSignaled(ControlPanel.FileModel->GetChangeSignal())) {
		UpdateItemsNeeded=true;
	}

	if (IsSignaled(GetSelectionSignal())) {
		UpdateFromSelection();
	}

	if (UpdateItemsNeeded) UpdateItems();
	if (UpdateSelectionNeeded) UpdateSelection();

	return busy;
}


void emStocksControlPanel::CategoryPanel::Notice(NoticeFlags flags)
{
	bool b;

	emListBox::Notice(flags);

	if (flags&NF_VIEWING_CHANGED) {
		b=(GetViewCondition() > 500.0);
		if (HaveListBoxContent!=b) {
			HaveListBoxContent=b;
			UpdateItemsNeeded=true;
			WakeUp();
		}
	}
}


void emStocksControlPanel::CategoryPanel::OnRecChanged()
{
	UpdateSelectionNeeded=true;
	WakeUp();
}


void emStocksControlPanel::CategoryPanel::UpdateItems()
{
	const emString * cat;
	bool listChanged;
	int i,j;

	UpdateItemsNeeded=false;

	if (!HaveListBoxContent) {
		ClearItems();
		return;
	}

	emAnything obsolete = emCastAnything<bool>(true);
	for (i=GetItemCount()-1; i>=0; i--) {
		SetItemData(i,obsolete);
	}

	listChanged=false;

	for (i=ControlPanel.FileModel->Stocks.GetCount()-1; i>=-1; i--) {
		if (i<0) {
			cat=&NameOfAll;
		}
		else {
			cat=&GetCategoryRec(&ControlPanel.FileModel->Stocks[i])->Get();
		}

		j=GetItemIndex(*cat);
		if (j>=0) {
			SetItemData(j,emAnything());
		}
		else {
			AddItem(*cat,*cat==NameOfAll?"<all>":cat->IsEmpty()?emString("<blank>"):*cat);
			listChanged=true;
		}
	}

	for (i=GetItemCount()-1; i>=0; i--) {
		if (emCastAnything<bool>(GetItemData(i))) {
			RemoveItem(i);
			listChanged=true;
		}
	}

	if (listChanged) {
		SortItems(CompareItems,this);
		UpdateSelectionNeeded=true;
	}
}


void emStocksControlPanel::CategoryPanel::UpdateSelection()
{
	const char * name;
	int i;

	UpdateSelectionNeeded=false;
	if (!HaveListBoxContent) return;

	if (CategoriesConfigRec.GetCount()<=0) {
		SetSelectedIndex(GetItemIndex(NameOfAll));
		return;
	}

	for (i=GetItemCount()-1; i>=0; i--) {
			name=GetItemName(i).Get();
			if (emStocksConfig::IsInVisibleCategories(CategoriesConfigRec,name)) {
				Select(i);
			}
			else {
				Deselect(i);
			}
	}
}


void emStocksControlPanel::CategoryPanel::UpdateFromSelection()
{
	int i,j;

	if (!HaveListBoxContent) return;

	if (
		GetSelectionCount()<=0 ||
		GetSelectionCount()>=GetItemCount()-1 ||
		IsSelected(GetItemIndex(NameOfAll))
	) {
		SetSelectedIndex(GetItemIndex(NameOfAll));
		CategoriesConfigRec.SetCount(0);
		return;
	}

	for (i=0,j=0; i<GetItemCount(); i++) {
		if (!IsSelected(i)) continue;
		if (j>=CategoriesConfigRec.GetCount()) CategoriesConfigRec.SetCount(j+1);
		CategoriesConfigRec[j++]=GetItemName(i);
	}
	CategoriesConfigRec.SetCount(j);
}


int emStocksControlPanel::CategoryPanel::CompareItems(
	const emString & item1name, const emString &, const emAnything &,
	const emString & item2name, const emString &, const emAnything &,
	void * context
)
{
	const emStocksControlPanel::CategoryPanel & cp =
		*(const emStocksControlPanel::CategoryPanel*)context
	;
	if (item1name==cp.NameOfAll) return item2name==cp.NameOfAll ? 0 : -1;
	if (item2name==cp.NameOfAll) return 1;
	return strcoll(item1name.Get(),item2name.Get());
}


emStringRec * emStocksControlPanel::CategoryPanel::GetCategoryRec(
	emStocksRec::StockRec * stockRec
)
{
	switch (Type) {
		case CT_COUNTRY: return &stockRec->Country;
		case CT_SECTOR : return &stockRec->Sector;
		default        : return &stockRec->Collection;
	}
}
