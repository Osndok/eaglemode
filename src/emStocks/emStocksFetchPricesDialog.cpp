//------------------------------------------------------------------------------
// emStocksFetchPricesDialog.cpp
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

#include <emStocks/emStocksFetchPricesDialog.h>


emStocksFetchPricesDialog::emStocksFetchPricesDialog(
	emContext & parentContext, emStocksFileModel & fileModel,
	const emString & apiScript, const emString & apiScriptInterpreter,
	const emString & apiKey
)
	: emDialog(parentContext),
	Fetcher(fileModel,apiScript,apiScriptInterpreter,apiKey)
{
	double minW,minH,w,h;
	emContext * context;
	emWindow * window;

	SetRootTitle("Fetching Prices");
	SetWindowFlags(GetWindowFlags()&~WF_MODAL);

	minW=600;
	minH=200;
	w=minW;
	h=minH;
	for (context=GetParentContext(); context; context=context->GetParentContext()) {
		window=dynamic_cast<emWindow*>(context);
		if (window) {
			w=window->GetHomeWidth()*0.4;
			h=window->GetHomeHeight()*0.4;
			if (w>h*minW/minH) w=h*minW/minH;
			if (w<minW) w=minW;
			w=round(w);
			h=round(w*minH/minW);
			break;
		}
	}
	SetViewSize(w,h);

	AddNegativeButton("Abort");
	EnableAutoDeletion();
	Label=new emLabel(GetContentPanel(),"label");
	ProgressBar=new ProgressBarPanel(GetContentPanel(),"progress");
	GetContentPanel()->SetOrientationThresholdTallness(0.02);
	AddWakeUpSignal(Fetcher.GetChangeSignal());
}


emStocksFetchPricesDialog::~emStocksFetchPricesDialog()
{
}


bool emStocksFetchPricesDialog::Cycle()
{
	if (IsSignaled(Fetcher.GetChangeSignal())) {
		UpdateControls();
		if (Fetcher.HasFinished()) {
			if (!Fetcher.GetError().IsEmpty()) {
				emDialog::ShowMessage(
					*GetParentContext(),
					"Error",
					Fetcher.GetError()
				);
			}
			Finish(0);
		}
	}

	return emDialog::Cycle();
}


void emStocksFetchPricesDialog::UpdateControls()
{
	emStocksRec::StockRec * stockRec;
	emString str;

	if (!Fetcher.GetError().IsEmpty()) {
		str=emString::Format("Error: %s",Fetcher.GetError().Get());
	}
	else if (Fetcher.HasFinished()) {
		str="Done";
		ProgressBar->SetProgressInPercent(100.0);
	}
	else {
		stockRec=Fetcher.GetCurrentStockRec();
		if (stockRec) {
			str=stockRec->Name.Get();
		}
		else {
			str="";
		}
		ProgressBar->SetProgressInPercent(Fetcher.GetProgressInPercent());
	}
	Label->SetCaption(str);
}


emStocksFetchPricesDialog::ProgressBarPanel::ProgressBarPanel(
	ParentArg parent, const emString & name
)
	: emBorder(parent,name),
	ProgressInPercent(0.0)
{
	emLook look;

	SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
	look=GetLook();
	look.SetOutputBgColor(emColor(43,49,70));
	look.SetOutputFgColor(emColor(109,158,204));
	SetLook(look);
}


emStocksFetchPricesDialog::ProgressBarPanel::~ProgressBarPanel()
{
}


void emStocksFetchPricesDialog::ProgressBarPanel::SetProgressInPercent(
	double progressInPercent
)
{
	if (ProgressInPercent!=progressInPercent) {
		ProgressInPercent=progressInPercent;
		InvalidatePainting();
	}
}


void emStocksFetchPricesDialog::ProgressBarPanel::PaintContent(
	const emPainter & painter, double x, double y, double w,
	double h, emColor canvasColor
) const
{
	double d;

	d=emMin(w,h)*0.1;
	x+=d;
	y+=d;
	w-=2*d;
	h-=2*d;
	painter.PaintRect(x,y,w*ProgressInPercent/100.0,h,GetLook().GetOutputFgColor());
}
