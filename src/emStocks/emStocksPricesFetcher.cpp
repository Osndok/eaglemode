//------------------------------------------------------------------------------
// emStocksPricesFetcher.cpp
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

#include <emStocks/emStocksPricesFetcher.h>


emStocksPricesFetcher::emStocksPricesFetcher(
	emStocksFileModel & fileModel, const emString & apiScript,
	const emString & apiScriptInterpreter, const emString & apiKey
)
	: emEngine(fileModel.GetScheduler()),
	FileModel(&fileModel),
	FileModelClient(&fileModel),
	ApiScript(apiScript),
	ApiScriptInterpreter(apiScriptInterpreter),
	ApiKey(apiKey),
	CurrentIndex(0),
	CurrentProcessActive(false),
	CurrentStockUpdated(false)
{
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(FileModel->GetFileStateSignal());
}


emStocksPricesFetcher::~emStocksPricesFetcher()
{
}


void emStocksPricesFetcher::AddListBox(emStocksListBox & listBox)
{
	const emCrossPtr<emStocksListBox> * c;

	for (c=ListBoxes.GetFirst(); c; c=ListBoxes.GetNext(c)) {
		if (c->Get() == &listBox) return;
	}
	ListBoxes.Add(emCrossPtr<emStocksListBox>(&listBox));
}


void emStocksPricesFetcher::AddStockIds(const emArray<emString> & stockIds)
{
	int i;

	for (i=0; i<stockIds.GetCount(); i++) {
		if (!StockRecsMap.Contains(stockIds[i])) {
			StockIds.Add(stockIds[i]);
			StockRecsMap.Insert(stockIds[i],NULL);
		}
	}
	Error.Clear();
	Signal(ChangeSignal);
	WakeUp();
}


const emString * emStocksPricesFetcher::GetCurrentStockId() const
{
	if (CurrentIndex<0 || CurrentIndex>=StockIds.GetCount()) return NULL;
	return &StockIds[CurrentIndex];
}


emStocksRec::StockRec * emStocksPricesFetcher::GetCurrentStockRec() const
{
	if (CurrentIndex<0 || CurrentIndex>=StockIds.GetCount()) return NULL;
	return GetStockRec(StockIds[CurrentIndex]);
}


double emStocksPricesFetcher::GetProgressInPercent() const
{
	if (CurrentIndex<0 || CurrentIndex>=StockIds.GetCount()) return 100.0;
	return (CurrentIndex + 0.5) * 100.0 / StockIds.GetCount();
}


bool emStocksPricesFetcher::HasFinished() const
{
	return CurrentIndex<0 || CurrentIndex>=StockIds.GetCount();
}


bool emStocksPricesFetcher::Cycle()
{
	switch (FileModel->GetFileState()) {
		case emFileModel::FS_LOADED:
		case emFileModel::FS_UNSAVED:
			break;
		default:
			return false;
	}

	if (CurrentProcessActive) PollProcess();
	if (!CurrentProcessActive) StartProcess();

	return CurrentProcessActive;
}


void emStocksPricesFetcher::StartProcess()
{
	emStocksRec::StockRec * stockRec;
	emArray<emString> args;

	if (CurrentProcessActive) return;

	for (;;) {
		if (CurrentIndex>=StockIds.GetCount()) {
			return;
		}

		stockRec=GetCurrentStockRec();
		if (!stockRec || stockRec->Symbol.Get().IsEmpty()) {
			CurrentIndex++;
			Signal(ChangeSignal);
			continue;
		}

		break;
	}

	CurrentSymbol=stockRec->Symbol.Get();
	OutBuffer.Clear();
	ErrBuffer.Clear();
	CurrentProcessActive=true;

	CalculateDate();

	if (ApiScript.IsEmpty()) {
		SetFailed("API script is not set.");
		return;
	}

	if (!ApiScriptInterpreter.IsEmpty()) args.Add(ApiScriptInterpreter);
	args.Add(ApiScript);
	args.Add(CurrentSymbol);
	args.Add(CurrentStartDate);
	args.Add(ApiKey);
	try {
		CurrentProcess.TryStart(
			args,
			emArray<emString>(),
			NULL,
			emProcess::SF_PIPE_STDOUT|
			emProcess::SF_PIPE_STDERR|
			emProcess::SF_NO_WINDOW|
			emProcess::SF_USE_CTRL_BREAK
		);
	}
	catch (const emException & exception) {
		SetFailed(exception.GetText());
	}
}


void emStocksPricesFetcher::PollProcess()
{
	char tmp[128]; // Don't make too large, otherwise too many AddPrice calls in a time slice.
	emStocksRec::StockRec * stockRec;
	int lenOut,lenErr;

	if (!CurrentProcessActive) return;

	for (;;) {
		if (IsTimeSliceAtEnd()) return;

		try {
			lenOut=CurrentProcess.TryRead(tmp,sizeof(tmp));
		}
		catch (const emException & exception) {
			SetFailed(exception.GetText());
			return;
		}

		if (lenOut<=0) break;

		OutBuffer.Add(tmp,lenOut);
		ProcessOutBufferLines();
		if (OutBuffer.GetCount()>100000) {
			SetFailed("API script printed a too long line.");
			return;
		}
	}

	for (;;) {
		try {
			lenErr=CurrentProcess.TryReadErr(tmp,sizeof(tmp));
		}
		catch (const emException & exception) {
			SetFailed(exception.GetText());
			return;
		}

		if (lenErr<=0) break;

		ErrBuffer.Add(tmp,lenErr);
		if (ErrBuffer.GetCount()>100000) {
			SetFailed("API script printed too much data on stderr.");
			return;
		}
	}

	if (lenOut>=0 || lenErr>=0) return;

	if (CurrentProcess.IsRunning()) return;

	ErrBuffer.Add('\0');

	if (CurrentProcess.GetExitStatus()!=0) {
		SetFailed(emString::Format(
			"API script failed for \"%s\":\n%s",
			CurrentSymbol.Get(),
			ErrBuffer.Get()
		));
		return;
	}

	if (!CurrentStockUpdated) {
		stockRec=GetCurrentStockRec();
		if (stockRec) {
			NoDataStocks+=emString::Format(
				"  %s - %s\n",
				stockRec->Symbol.Get().Get(),
				stockRec->Name.Get().Get()
			);
		}
	}

	if (!NoDataStocks.IsEmpty() && CurrentIndex+1>=StockIds.GetCount()) {
		SetFailed(
			"Could not fetch any new data for:\n"+
			NoDataStocks
		);
		return;
	}

	CurrentIndex++;
	CurrentSymbol.Clear();
	CurrentStartDate.Clear();
	CurrentProcessActive=false;
	CurrentStockUpdated=false;
	OutBuffer.Clear();
	ErrBuffer.Clear();
	if (CurrentIndex>=StockIds.GetCount()) Clear();
	Signal(ChangeSignal);
}


void emStocksPricesFetcher::SetFailed(const emString & error)
{
	Clear();
	Error=error;
	Signal(ChangeSignal);
}


void emStocksPricesFetcher::Clear()
{
	StockIds.Clear();
	StockRecsMap.Clear();
	CurrentIndex=0;
	CurrentSymbol.Clear();
	CurrentStartDate.Clear();
	CurrentProcess.Terminate();
	CurrentProcessActive=false;
	CurrentStockUpdated=false;
	OutBuffer.Clear();
	ErrBuffer.Clear();
	NoDataStocks.Clear();
	Error.Clear();
}


emStocksRec::StockRec * emStocksPricesFetcher::GetStockRec(
	const emString & stockId
) const
{
	const emCrossPtr<emStocksRec::StockRec> * value;

	value=StockRecsMap.GetValue(stockId);
	if (value && !*value) {
		((emStocksPricesFetcher*)this)->UpdateStockRecsMapValues();
		value=StockRecsMap.GetValue(stockId);
	}
	return value ? *value : NULL;
}


void emStocksPricesFetcher::UpdateStockRecsMapValues()
{
	const emAvlTreeMap<
		emString,emCrossPtr<emStocksRec::StockRec>
	>::Element * element;
	emStocksRec::StockRec * stockRec;
	int i;

	for (i=0; i<FileModel->Stocks.GetCount(); i++) {
		stockRec=&FileModel->Stocks[i];
		element=StockRecsMap.Get(stockRec->Id.Get());
		if (element && !element->Value) {
			StockRecsMap.SetValue(element,stockRec);
		}
	}
}


void emStocksPricesFetcher::CalculateDate()
{
	emStocksRec::StockRec * stockRec;
	emString currentDate;
	int d;

	currentDate=emStocksFileModel::GetCurrentDate();

	stockRec=GetCurrentStockRec();
	if (!stockRec || stockRec->LastPriceDate.Get().IsEmpty()) {
		d=emStocksRec::StockRec::MAX_NUM_PRICES;
	}
	else {
		d=emStocksFileModel::GetDateDifference(
			stockRec->LastPriceDate.Get(),currentDate
		);
		d+=1;
		d=emMin(d,(int)emStocksRec::StockRec::MAX_NUM_PRICES);
		d=emMax(d,1);
	}

	CurrentStartDate=emStocksFileModel::AddDaysToDate(1-d,currentDate);
}


void emStocksPricesFetcher::ProcessOutBufferLines()
{
	char * beg, * pos, * end, * brk;

	beg=OutBuffer.GetWritable();
	end=beg+OutBuffer.GetCount();
	pos=beg;

	for (;;) {
		for (brk=pos; brk<end && *brk!=0x0d && *brk!=0x0a; brk++);
		if (brk>=end) break;
		*brk=0;
		ProcessOutBufferLine(pos);
		do { brk++; } while (brk<end && (*brk==0x0d || *brk==0x0a));
		pos=brk;
	}

	if (pos>beg) OutBuffer.Remove(0,pos-beg);
}


void emStocksPricesFetcher::ProcessOutBufferLine(const char * str)
{
	emString date;
	emString price;
	int ymd[3];
	int i,d;

	while (*str && (unsigned char)*str<=0x20) str++;

	for (i=0; ; i++) {
		if (*str<'0' || *str>'9') return;
		d=*str++ - '0';
		while (*str>='0' && *str<='9') d=d*10+(*str++ - '0');
		ymd[i]=d;
		if (i>=2) break;
		if (*str!='-') return;
		str++;
	}
	date=emString::Format("%04d-%02d-%02d",ymd[0],ymd[1],ymd[2]);

	if (emStocksFileModel::CompareDates(date, CurrentStartDate) < 0) {
		return;
	}

	while (*str && (*str<'0' || *str>'9') && *str!='-' && *str!='.') str++;
	if (!*str) return;
	price=emStocksFileModel::SharePriceToString(atof(str));

	AddPrice(date,price);
}


void emStocksPricesFetcher::AddPrice(const char * date, const char * price)
{
	emStocksRec::StockRec * stockRec;
	emStocksListBox * listBox;
	const emCrossPtr<emStocksListBox> * c;
	emString latestBefore;

	stockRec=GetCurrentStockRec();
	if (!stockRec) return;

	if (
		stockRec->LastPriceDate.Get().IsEmpty() ||
		emStocksFileModel::CompareDates(date, stockRec->LastPriceDate.Get()) > 0
	) {
		latestBefore=FileModel->GetLatestPricesDate();
		if (emStocksFileModel::CompareDates(date, latestBefore) > 0) {
			for (c=ListBoxes.GetFirst(); c; c=ListBoxes.GetNext(c)) {
				listBox=c->Get();
				if (!listBox) continue;
				if (emStocksFileModel::CompareDates(
					latestBefore, listBox->GetSelectedDate()) <= 0
				) {
					listBox->SetSelectedDate(date);
				}
			}
		}
	}

	stockRec->AddPrice(date,price);
	CurrentStockUpdated=true;
}
