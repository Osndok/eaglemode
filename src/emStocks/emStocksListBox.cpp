//------------------------------------------------------------------------------
// emStocksListBox.cpp
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

#include <emStocks/emStocksListBox.h>
#include <emStocks/emStocksFetchPricesDialog.h>
#include <emCore/emAvlTreeSet.h>


emStocksListBox::emStocksListBox(
	ParentArg parent, const emString & name, emStocksFileModel & fileModel,
	emStocksConfig & config
)
	: emListBox(parent,name),
	FileModel(fileModel),
	Config(config),
	InterestToSet(emStocksFileModel::MEDIUM_INTEREST)
{
	emLook look;

	SetSelectionType(MULTI_SELECTION);
	SetBorderType(OBT_NONE,IBT_CUSTOM_RECT);
	SetPrefChildTallness(0.315);
	SetMinChildTallness(0.265);
	SetMaxChildTallness(0.365);
	SetAlignment(EM_ALIGN_TOP_LEFT);
	SetInnerSpace(0.02,0.05);
	SetBorderScaling(0.3);

	look=GetLook();
	look.SetBgColor(0x272d40ff);
	look.SetButtonBgColor(0x333B52ff);
	SetLook(look);

	AddWakeUpSignal(FileModel.GetChangeSignal());
	AddWakeUpSignal(Config.GetChangeSignal());
	AddWakeUpSignal(GetItemTriggerSignal());

	SelectedDate=FileModel.GetLatestPricesDate();
	if (SelectedDate.IsEmpty()) {
		SelectedDate=emStocksFileModel::GetCurrentDate();
	}

	if (FileModel.PricesFetchingDialog) {
		FileModel.PricesFetchingDialog->AddListBox(*this);
	}

	UpdateItems();
}


emStocksListBox::~emStocksListBox()
{
	if (CutStocksDialog) CutStocksDialog->Finish(emDialog::NEGATIVE);
	if (PasteStocksDialog) PasteStocksDialog->Finish(emDialog::NEGATIVE);
	if (DeleteStocksDialog) DeleteStocksDialog->Finish(emDialog::NEGATIVE);
	if (InterestDialog) InterestDialog->Finish(emDialog::NEGATIVE);
}


emStocksRec::StockRec * emStocksListBox::GetStockByItemIndex(
	int itemIndex
) const
{
	const emCrossPtr<emStocksRec::StockRec> * crossPtr=
		emCastAnything<emCrossPtr<emStocksRec::StockRec>>(
			GetItemData(itemIndex)
		)
	;
	return crossPtr ? *crossPtr : NULL;
}


int emStocksListBox::GetItemIndexByStock(
	const emStocksRec::StockRec * stockRec
) const
{
	return stockRec ? GetItemIndex(stockRec->Id.Get()) : -1;
}


void emStocksListBox::SetSelectedDate(const emString & selectedDate)
{
	if (SelectedDate!=selectedDate) {
		SelectedDate=selectedDate;
		Signal(SelectedDateSignal);
		UpdateItems();
	}
}


void emStocksListBox::GoBackInHistory()
{
	emString date;

	date=FileModel.GetPricesDateBefore(SelectedDate);
	if (!date.IsEmpty()) SetSelectedDate(date);
}


void emStocksListBox::GoForwardInHistory()
{
	emString date;

	date=FileModel.GetPricesDateAfter(SelectedDate);
	if (!date.IsEmpty()) SetSelectedDate(date);
}


void emStocksListBox::NewStock()
{
	emStocksRec::StockRec * stockRec;
	emPanel * panel;
	int i,j;

	i=FileModel.Stocks.GetCount();
	FileModel.Stocks.Insert(i);
	stockRec=&FileModel.Stocks[i];
	stockRec->Id=FileModel.InventStockId();
	if (stockRec->Interest.Get() > Config.MinVisibleInterest.Get()) {
		stockRec->Interest=Config.MinVisibleInterest.Get();
	}
	if (Config.VisibleCountries.GetCount() > 0) {
		stockRec->Country.Set(Config.VisibleCountries[0].Get());
	}
	if (Config.VisibleSectors.GetCount() > 0) {
		stockRec->Sector.Set(Config.VisibleSectors[0].Get());
	}
	if (Config.VisibleCollections.GetCount() > 0) {
		stockRec->Collection.Set(Config.VisibleCollections[0].Get());
	}

	UpdateItems();
	j=GetItemIndexByStock(stockRec);
	SetSelectedIndex(j);

	panel=GetItemPanel(j);
	if (panel) GetView().VisitFullsized(panel,true);
}


void emStocksListBox::CutStocks(bool ask)
{
	emString str;
	const emString * p;
	int i;

	if (GetSelectionCount()<=0) return;

	if (ask) {
		str=
			"Are you sure to delete the following selected stocks\n"
			"after copying them to the clipboard?\n"
		;
		for (i=0; i<GetItemCount(); i++) {
			if (!IsSelected(i)) continue;
			p=&GetStockByItemIndex(i)->Name.Get();
			str+="\n  ";
			if (p->IsEmpty()) str+="<unnamed>";
			else str+=*p;
		}

		if (CutStocksDialog) CutStocksDialog->Finish(emDialog::NEGATIVE);
		CutStocksDialog=new emDialog(GetViewContext());
		CutStocksDialog->SetRootTitle("Cut Stocks");
		CutStocksDialog->AddOKCancelButtons();
		new emLabel(
			CutStocksDialog->GetContentPanel(),
			"l",
			str
		);
		CutStocksDialog->EnableAutoDeletion();
		AddWakeUpSignal(CutStocksDialog->GetFinishSignal());
		return;
	}

	if (!CopyStocks()) return;

	DeleteStocks(false);
}


bool emStocksListBox::CopyStocks()
{
	emStocksRec stocksRec;
	emStocksRec::StockRec * stockRec;
	emRef<emClipboard> clipboard;
	emArray<char> buf;
	emString str;
	int i,j;

	if (GetSelectionCount()<=0) return false;

	for (i=0; i<FileModel.Stocks.GetCount(); i++) {
		stockRec=&FileModel.Stocks[i];

		j=GetItemIndex(stockRec->Id.Get());
		if (j<0 || !IsSelected(j)) continue;

		j=stocksRec.Stocks.GetCount();
		stocksRec.Stocks.Insert(j);
		stocksRec.Stocks[j].Copy(*stockRec);
	}

	buf.SetTuningLevel(4);
	stocksRec.SaveToMem(buf);
	str=emString(buf,buf.GetCount());

	clipboard=emClipboard::LookupInherited(GetView());
	if (!clipboard) {
		emDialog::ShowMessage(GetView(),"Error","No clipboard found.");
		return false;
	}
	clipboard->PutText(str);
	clipboard->PutText(str,true);
	return true;
}


void emStocksListBox::PasteStocks(bool ask)
{
	emRef<emClipboard> clipboard;
	emList<emString> invisible;
	emStocksRec stocksRec;
	emStocksRec::StockRec * stockRec;
	emString str;
	const emString * p;
	int i,j,n,m;

	clipboard=emClipboard::LookupInherited(GetView());
	if (!clipboard) {
		emDialog::ShowMessage(GetView(),"Error","No clipboard found.");
		return;
	}

	str=clipboard->GetText();
	try {
		stocksRec.TryLoadFromMem(str.Get(),str.GetLen());
	}
	catch (const emException & exception) {
		emDialog::ShowMessage(
			GetView(),
			"Error",
			emString::Format(
				"No valid stocks in clipboard (%s)",
				exception.GetText().Get()
			)
		);
		return;
	}

	if (ask) {
		str="Are you sure to insert the following stocks from the clipboard?\n";
		for (i=0; i<stocksRec.Stocks.GetCount(); i++) {
			p=&stocksRec.Stocks[i].Name.Get();
			str+="\n  ";
			if (p->IsEmpty()) str+="<unnamed>";
			else str+=*p;
		}

		if (PasteStocksDialog) PasteStocksDialog->Finish(emDialog::NEGATIVE);
		PasteStocksDialog=new emDialog(GetViewContext());
		PasteStocksDialog->SetRootTitle("Paste Stocks");
		PasteStocksDialog->AddOKCancelButtons();
		new emLabel(
			PasteStocksDialog->GetContentPanel(),
			"l",
			str
		);
		PasteStocksDialog->EnableAutoDeletion();
		AddWakeUpSignal(PasteStocksDialog->GetFinishSignal());
		return;
	}

	n=FileModel.Stocks.GetCount();
	m=stocksRec.Stocks.GetCount();
	for (i=0; i<m; i++) {
		stockRec=&stocksRec.Stocks[i];
		j=FileModel.GetStockIndexById(stockRec->Id.Get());
		if (j>=0) stockRec->Id=FileModel.InventStockId();
		FileModel.Stocks.SetCount(n+i+1);
		FileModel.Stocks[n+i].Copy(*stockRec);
		if (!IsVisibleStock(*stockRec)) {
			invisible.Add(stockRec->Name.Get());
		}
	}

	UpdateItems();
	ClearSelection();
	for (i=n; i<n+m; i++) {
		stockRec=&FileModel.Stocks[i];
		j=GetItemIndexByStock(stockRec);
		if (j>=0) Select(j);
	}

	GetView().VisitFullsized(this,false);

	if (!invisible.IsEmpty()) {
		str="The following pasted stocks are not visible due to filter settings:\n";
		for (p=invisible.GetFirst(); p; p=invisible.GetNext(p)) {
			str+="\n  ";
			if (p->IsEmpty()) str+="<unnamed>";
			else str+=*p;
		}
		emDialog::ShowMessage(GetView(),"Warning",str);
	}
}


void emStocksListBox::DeleteStocks(bool ask)
{
	emAvlTreeSet<int> set;
	emAvlTreeSet<int>::Iterator it;
	emString str;
	const emString * p;
	int i;

	if (GetSelectionCount()<=0) return;

	if (ask) {
		str="Are you sure to delete the following selected stocks?\n";
		for (i=0; i<GetItemCount(); i++) {
			if (!IsSelected(i)) continue;
			p=&GetStockByItemIndex(i)->Name.Get();
			str+="\n  ";
			if (p->IsEmpty()) str+="<unnamed>";
			else str+=*p;
		}

		if (DeleteStocksDialog) DeleteStocksDialog->Finish(emDialog::NEGATIVE);
		DeleteStocksDialog=new emDialog(GetViewContext());
		DeleteStocksDialog->SetRootTitle("Delete Stocks");
		DeleteStocksDialog->AddOKCancelButtons();
		new emLabel(
			DeleteStocksDialog->GetContentPanel(),
			"l",
			str
		);
		DeleteStocksDialog->EnableAutoDeletion();
		AddWakeUpSignal(DeleteStocksDialog->GetFinishSignal());
		return;
	}

	for (i=0; i<GetItemCount(); i++) {
		if (IsSelected(i)) {
			set.Insert(FileModel.GetStockIndexByStock(GetStockByItemIndex(i)));
		}
	}
	for (it.SetLast(set); it; --it) {
		FileModel.Stocks.Remove(**it);
	}
}


void emStocksListBox::StartToFetchSharePrices()
{
	emArray<emString> stockIds;
	emStocksRec::StockRec * stockRec;
	int i;

	for (i=0; i<GetItemCount(); i++) {
		stockRec = GetStockByItemIndex(i);
		if (stockRec) stockIds.Add(stockRec->Id.Get());
	}

	StartToFetchSharePrices(stockIds);
}


void emStocksListBox::StartToFetchSharePrices(
	const emArray<emString> & stockIds
)
{
	emString date;

	if (!FileModel.PricesFetchingDialog) {
		FileModel.PricesFetchingDialog=new emStocksFetchPricesDialog(
			GetView(),FileModel,Config.ApiScript,
			Config.ApiScriptInterpreter,Config.ApiKey
		);
	}
	else {
		FileModel.PricesFetchingDialog->Raise();
	}

	date=FileModel.GetLatestPricesDate();
	if (date.IsEmpty()) date=emStocksFileModel::GetCurrentDate();
	SetSelectedDate(date);

	FileModel.PricesFetchingDialog->AddListBox(*this);
	FileModel.PricesFetchingDialog->AddStockIds(stockIds);
}


void emStocksListBox::DeleteSharePrices()
{
	emStocksRec::StockRec * stockRec;
	int i;

	for (i=0; i<GetItemCount(); i++) {
		stockRec=GetStockByItemIndex(i);
		if (!stockRec) continue;
		stockRec->Prices.Set("");
		stockRec->LastPriceDate.Set("");
	}
}


void emStocksListBox::SetInterest(
	emStocksFileModel::InterestType interest, bool ask
)
{
	emStocksRec::StockRec * stockRec;
	int i;

	if (ask) {
		if (InterestDialog) InterestDialog->Finish(emDialog::NEGATIVE);
		InterestDialog=new emDialog(GetViewContext());
		InterestDialog->SetRootTitle("Set Interest");
		InterestDialog->AddOKCancelButtons();
		new emLabel(
			InterestDialog->GetContentPanel(),
			"l",
			"Are you sure to set the interest level of the selected stocks?"
		);
		InterestDialog->EnableAutoDeletion();
		AddWakeUpSignal(InterestDialog->GetFinishSignal());
		InterestToSet=interest;
		return;
	}

	for (i=0; i<GetItemCount(); i++) {
		if (IsSelected(i)) {
			stockRec=GetStockByItemIndex(i);
			if (stockRec) {
				stockRec->Interest=interest;
			}
		}
	}
}


void emStocksListBox::ShowFirstWebPages() const
{
	emArray<emString> webPages;
	emStocksRec::StockRec * stockRec;
	int i;

	for (i=0; i<GetItemCount(); i++) {
		if (!IsSelected(i)) continue;
		stockRec=GetStockByItemIndex(i);
		if (!stockRec) continue;
		if (stockRec->WebPages.GetCount()<=0) continue;
		if (stockRec->WebPages[0].Get().IsEmpty()) continue;
		webPages.Add(stockRec->WebPages[0].Get());
	}
	if (!webPages.IsEmpty()) ShowWebPages(webPages);
}


void emStocksListBox::ShowAllWebPages() const
{
	emArray<emString> webPages;
	emStocksRec::StockRec * stockRec;
	int i,j;

	for (i=0; i<GetItemCount(); i++) {
		if (!IsSelected(i)) continue;
		stockRec=GetStockByItemIndex(i);
		if (!stockRec) continue;
		for (j=0; j<stockRec->WebPages.GetCount(); j++) {
			if (stockRec->WebPages[j].Get().IsEmpty()) continue;
			webPages.Add(stockRec->WebPages[j].Get());
		}
	}
	if (!webPages.IsEmpty()) ShowWebPages(webPages);
}


void emStocksListBox::ShowWebPages(const emArray<emString> & webPages) const
{
	emArray<emString> args;
	int i;

	if (Config.WebBrowser.Get().IsEmpty()) {
		emDialog::ShowMessage(GetView(),"Error","Web browser is not configured.");
		return;
	}

	args.Add(Config.WebBrowser.Get());
	for (i=0; i<webPages.GetCount(); i++) {
		args.Add(webPages[i]);
	}

	try {
		emProcess::TryStartUnmanaged(args);
	}
	catch (const emException & exception) {
		emDialog::ShowMessage(GetView(),"Error",exception.GetText());
	}
}


void emStocksListBox::FindSelected()
{
	emRef<emClipboard> clipboard;
	emString text;

	clipboard=emClipboard::LookupInherited(GetView());
	if (!clipboard) {
		emDialog::ShowMessage(GetView(),"Error","No clipboard available.");
		return;
	}

	text=clipboard->GetText(true);
	if (text.IsEmpty()) {
		text=clipboard->GetText(false);
		if (text.IsEmpty()) {
			if (GetScreen()) GetScreen()->Beep();
			return;
		}
	}

	Config.SearchText=text;
	FindNext();
}



void emStocksListBox::FindNext()
{
	emStocksRec::StockRec * s;
	emPanel * p;
	int i,j;

	if (GetItemCount()<=0) return;

	for (i=GetItemCount()-1; i>=0; i--) {
		p=GetItemPanel(i);
		if (p && p->IsInActivePath()) break;
	}

	if (i<0) i=GetItemCount()-1;
	for (j=i;;) {
		j=(j+1)%GetItemCount();
		s=GetStockByItemIndex(j);
		if (s && s->IsMatchingSearchText(Config.SearchText.Get())) {
			break;
		}
		if (j==i) {
			if (GetScreen()) GetScreen()->Beep();
			return;
		}
	}
	p=GetItemPanel(j);
	if (!p) return;
	GetView().VisitFullsized(p,true);
}


void emStocksListBox::FindPrevious()
{
	emStocksRec::StockRec * s;
	emPanel * p;
	int i,j;

	if (GetItemCount()<=0) return;

	for (i=GetItemCount()-1; i>=0; i--) {
		p=GetItemPanel(i);
		if (p && p->IsInActivePath()) break;
	}

	if (i<0) i=0;
	for (j=i;;) {
		j=(j+GetItemCount()-1)%GetItemCount();
		s=GetStockByItemIndex(j);
		if (s && s->IsMatchingSearchText(Config.SearchText.Get())) {
			break;
		}
		if (j==i) {
			if (GetScreen()) GetScreen()->Beep();
			return;
		}
	}
	p=GetItemPanel(j);
	if (!p) return;
	GetView().VisitFullsized(p,true);
}



bool emStocksListBox::IsVisibleStock(
	const emStocksRec::StockRec & stockRec
) const
{
	return
		stockRec.Interest.Get() <= Config.MinVisibleInterest.Get() &&
		emStocksConfig::IsInVisibleCategories(
			Config.VisibleCountries,stockRec.Country.Get().Get()
		) &&
		emStocksConfig::IsInVisibleCategories(
			Config.VisibleSectors,stockRec.Sector.Get().Get()
		) &&
		emStocksConfig::IsInVisibleCategories(
			Config.VisibleCollections,stockRec.Collection.Get().Get()
		)
	;
}


bool emStocksListBox::Cycle()
{
	emStocksRec::StockRec * stockRec;
	bool busy;

	busy=emListBox::Cycle();

	if (IsSignaled(FileModel.GetChangeSignal())) {
		UpdateItems();
	}

	if (IsSignaled(Config.GetChangeSignal())) {
		UpdateItems();
	}

	if (IsSignaled(GetItemTriggerSignal())) {
		stockRec=emStocksListBox::GetStockByItemIndex(GetTriggeredItemIndex());
		if (stockRec) {
			if (
				Config.TriggeringOpensWebPage.Get() &&
				stockRec->WebPages.GetCount()>0 &&
				!stockRec->WebPages[0].Get().IsEmpty()
			) {
				ShowWebPages(emArray<emString>(stockRec->WebPages[0].Get()));
			}
		}
	}

	if (CutStocksDialog && IsSignaled(CutStocksDialog->GetFinishSignal())) {
		if (CutStocksDialog->GetResult()==emDialog::POSITIVE) {
			CutStocks(false);
		}
	}

	if (PasteStocksDialog && IsSignaled(PasteStocksDialog->GetFinishSignal())) {
		if (PasteStocksDialog->GetResult()==emDialog::POSITIVE) {
			PasteStocks(false);
		}
	}

	if (DeleteStocksDialog && IsSignaled(DeleteStocksDialog->GetFinishSignal())) {
		if (DeleteStocksDialog->GetResult()==emDialog::POSITIVE) {
			DeleteStocks(false);
		}
	}

	if (InterestDialog && IsSignaled(InterestDialog->GetFinishSignal())) {
		if (InterestDialog->GetResult()==emDialog::POSITIVE) {
			SetInterest(InterestToSet,false);
		}
	}

	return busy;
}


void emStocksListBox::Paint(const emPainter & painter, emColor canvasColor) const
{
	emListBox::Paint(painter,canvasColor);

	if (GetItemCount()==0) {
		painter.PaintTextBoxed(
			0,0,1.0,GetHeight(),"empty stock list",GetHeight()*0.08,0xFFFFFF40
		);
	}
}


void emStocksListBox::CreateItemPanel(const emString & name, int itemIndex)
{
	emStocksItemPanel * itemPanel;
	emStocksRec::StockRec * stockRec;

	itemPanel = new emStocksItemPanel(*this,name,itemIndex,FileModel,Config);

	stockRec=GetStockByItemIndex(itemIndex);
	if (stockRec) itemPanel->SetStockRec(stockRec);
}


void emStocksListBox::UpdateItems()
{
	emStocksRec::StockRec * stockRec;
	int i,oldCount,visibleCount;

	oldCount=GetItemCount();

	for (i=0; i<GetItemCount(); ) {
		stockRec = GetStockByItemIndex(i);
		if (stockRec && IsVisibleStock(*stockRec)) i++;
		else RemoveItem(i);
	}

	for (visibleCount=0, i=0; i<FileModel.Stocks.GetCount(); i++) {
		if (IsVisibleStock(FileModel.Stocks[i])) visibleCount++;
	}
	if (visibleCount>GetItemCount()) {
		for (i=0; i<FileModel.Stocks.GetCount(); i++) {
			stockRec=&FileModel.Stocks[i];
			if (IsVisibleStock(*stockRec) && GetItemIndex(stockRec->Id.Get())<0) {
				AddItem(
					stockRec->Id.Get(),
					stockRec->Name.Get(),
					emCastAnything<emCrossPtr<emStocksRec::StockRec> >(
						emCrossPtr<emStocksRec::StockRec>(stockRec)
					)
				);
			}
		}
	}

	SortItems(CompareItems,this);

	if (oldCount!=GetItemCount()) InvalidatePainting();
}


int emStocksListBox::CompareItems(
	const emString &, const emString &, const emAnything & item1data,
	const emString &, const emString &, const emAnything & item2data,
	void * context
)
{
	const emCrossPtr<emStocksRec::StockRec> * crossPtr;
	const emStocksRec::StockRec * s1, * s2;
	emStocksListBox * lb;
	double f,f1,f2;
	int d,i1,i2;
	bool b1,b2;

	lb=(emStocksListBox*)context;

	crossPtr=emCastAnything<emCrossPtr<emStocksRec::StockRec>>(item1data);
	if (!crossPtr) return 1;
	s1=*crossPtr;

	crossPtr=emCastAnything<emCrossPtr<emStocksRec::StockRec>>(item2data);
	if (!crossPtr) return -1;
	s2=*crossPtr;

	if (lb->Config.OwnedSharesFirst.Get()) {
		if (s1->OwningShares.Get() != s2->OwningShares.Get()) {
			return s1->OwningShares.Get() ? -1 : 1;
		}
	}

	switch (lb->Config.Sorting.Get()) {
		case emStocksConfig::SORT_BY_TRADE_DATE:
			f1=emStocksFileModel::CompareDates(s1->TradeDate.Get(),s2->TradeDate.Get());
			f2=0.0;
			b1=b2=true;
			break;
		case emStocksConfig::SORT_BY_INQUIRY_DATE:
			f1=emStocksFileModel::CompareDates(s1->InquiryDate.Get(),s2->InquiryDate.Get());
			f2=0.0;
			b1=b2=true;
			break;
		case emStocksConfig::SORT_BY_ACHIEVEMENT:
			b1=s1->GetAchievementOfDate(&f1,lb->GetSelectedDate());
			b2=s2->GetAchievementOfDate(&f2,lb->GetSelectedDate());
			break;
		case emStocksConfig::SORT_BY_ONE_WEEK_RISE:
			b1=s1->GetRiseUntilDate(&f1,lb->GetSelectedDate(),7);
			b2=s2->GetRiseUntilDate(&f2,lb->GetSelectedDate(),7);
			break;
		case emStocksConfig::SORT_BY_THREE_WEEK_RISE:
			b1=s1->GetRiseUntilDate(&f1,lb->GetSelectedDate(),7*3);
			b2=s2->GetRiseUntilDate(&f2,lb->GetSelectedDate(),7*3);
			break;
		case emStocksConfig::SORT_BY_NINE_WEEK_RISE:
			b1=s1->GetRiseUntilDate(&f1,lb->GetSelectedDate(),7*9);
			b2=s2->GetRiseUntilDate(&f2,lb->GetSelectedDate(),7*9);
			break;
		case emStocksConfig::SORT_BY_DIVIDEND:
			b1=!s1->ExpectedDividend.Get().IsEmpty();
			f1=b1?atof(s1->ExpectedDividend.Get().Get()):0.0;
			b2=!s2->ExpectedDividend.Get().IsEmpty();
			f2=b2?atof(s2->ExpectedDividend.Get().Get()):0.0;
			break;
		case emStocksConfig::SORT_BY_PURCHASE_VALUE:
			b1=s1->GetTradeValue(&f1);
			b2=s2->GetTradeValue(&f2);
			break;
		case emStocksConfig::SORT_BY_VALUE:
			b1=s1->GetValueOfDate(&f1,lb->GetSelectedDate());
			b2=s2->GetValueOfDate(&f2,lb->GetSelectedDate());
			break;
		case emStocksConfig::SORT_BY_DIFFERENCE:
			b1=s1->GetDifferenceValueOfDate(&f1,lb->GetSelectedDate());
			b2=s2->GetDifferenceValueOfDate(&f2,lb->GetSelectedDate());
			break;
		default:
			f1=f2=0.0;
			b1=b2=false;
			break;
	}

	if (b1 != b2) return b1 ? 1 : -1;
	if (b1) {
		f=f1-f2;
		d=f<0.0?-1:f>0.0?1:0;
	}
	else {
		d=0;
	}

	if (!d) d=strcoll(s1->Name.Get(),s2->Name.Get());
	if (!d) {
		d=strcmp(s1->Name.Get(),s2->Name.Get());
		if (!d) {
			i1=atoi(s1->Id.Get());
			i2=atoi(s2->Id.Get());
			d=i1>i2?1:i1<i2?-1:0;
			if (!d) d=strcmp(s1->Id.Get(),s2->Id.Get());
		}
	}
	return d;
}
