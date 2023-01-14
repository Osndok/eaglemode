//------------------------------------------------------------------------------
// emStocksFetchPricesDialog.h
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

#ifndef emStocksFetchPricesDialog_h
#define emStocksFetchPricesDialog_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emStocksPricesFetcher_h
#include <emStocks/emStocksPricesFetcher.h>
#endif


class emStocksFetchPricesDialog : public emDialog {

public:

	emStocksFetchPricesDialog(
		emContext & parentContext, emStocksFileModel & fileModel,
		const emString & apiScript,
		const emString & apiScriptInterpreter,
		const emString & apiKey
	);

	virtual ~emStocksFetchPricesDialog();

	void AddListBox(emStocksListBox & listBox);

	void AddStockIds(const emArray<emString> & stockIds);

protected:

	virtual bool Cycle();

private:

	void UpdateControls();

	class ProgressBarPanel : public emBorder {
	public:
		ProgressBarPanel(ParentArg parent, const emString & name);
		virtual ~ProgressBarPanel();
		void SetProgressInPercent(double progressInPercent);
	protected:
		virtual void PaintContent(
			const emPainter & painter, double x, double y, double w,
			double h, emColor canvasColor
		) const;
	private:
		double ProgressInPercent;
	};

	emStocksPricesFetcher Fetcher;
	emLabel * Label;
	ProgressBarPanel * ProgressBar;
};


inline void emStocksFetchPricesDialog::AddListBox(emStocksListBox & listBox)
{
	Fetcher.AddListBox(listBox);
}

inline void emStocksFetchPricesDialog::AddStockIds(
	const emArray<emString> & stockIds
)
{
	Fetcher.AddStockIds(stockIds);
}


#endif
