//------------------------------------------------------------------------------
// emStocksPricesFetcher.h
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

#ifndef emStocksPricesFetcher_h
#define emStocksPricesFetcher_h

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emAvlTreeMap_h
#include <emCore/emAvlTreeMap.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif

#ifndef emStocksListBox_h
#include <emStocks/emStocksListBox.h>
#endif


class emStocksPricesFetcher : public emEngine
{
public:

	emStocksPricesFetcher(
		emStocksFileModel & fileModel, const emString & apiScript,
		const emString & apiScriptInterpreter,
		const emString & apiKey
	);

	virtual ~emStocksPricesFetcher();

	void AddListBox(emStocksListBox & listBox);

	void AddStockIds(const emArray<emString> & stockIds);

	const emString * GetCurrentStockId() const;
	emStocksRec::StockRec * GetCurrentStockRec() const;

	double GetProgressInPercent() const;

	bool HasFinished() const;

	const emString & GetError() const;

	const emSignal & GetChangeSignal() const;

protected:

	virtual bool Cycle();

private:

	void StartProcess();
	void PollProcess();
	void SetFailed(const emString & error);
	void Clear();
	emStocksRec::StockRec * GetStockRec(const emString & stockId) const;
	void UpdateStockRecsMapValues();
	void CalculateDate();
	void ProcessOutBufferLines();
	void ProcessOutBufferLine(const char * str);
	void AddPrice(const char * date, const char * price);

	emRef<emStocksFileModel> FileModel;
	emAbsoluteFileModelClient FileModelClient;
	emList<emCrossPtr<emStocksListBox>> ListBoxes;
	emString ApiScript;
	emString ApiScriptInterpreter;
	emString ApiKey;
	emArray<emString> StockIds;
	emAvlTreeMap<emString,emCrossPtr<emStocksRec::StockRec> > StockRecsMap;
	int CurrentIndex;
	emString CurrentSymbol;
	emString CurrentStartDate;
	emProcess CurrentProcess;
	bool CurrentProcessActive;
	bool CurrentStockUpdated;
	emArray<char> OutBuffer;
	emArray<char> ErrBuffer;
	emString NoDataStocks;
	emString Error;
	emSignal ChangeSignal;
};


inline const emString & emStocksPricesFetcher::GetError() const
{
	return Error;
}

inline const emSignal & emStocksPricesFetcher::GetChangeSignal() const
{
	return ChangeSignal;
}


#endif
