//------------------------------------------------------------------------------
// emStocksFilePanel.cpp
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

#include <emStocks/emStocksFilePanel.h>
#include <emStocks/emStocksControlPanel.h>


emStocksFilePanel::emStocksFilePanel(
	ParentArg parent, const emString & name, emStocksFileModel * fileModel
)
	: emFilePanel(parent,name,fileModel,true),
	FileModel(fileModel),
	Config(emStocksConfig::Acquire(GetViewContext())),
	ListBox(NULL),
	BgColor(0x131520ff)
{
	AddWakeUpSignal(GetVirFileStateSignal());
}


emStocksFilePanel::~emStocksFilePanel()
{
}


void emStocksFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	FileModel=dynamic_cast<emStocksFileModel*>(fileModel);
	emFilePanel::SetFileModel(FileModel,updateFileModel);
}


emString emStocksFilePanel::GetIconFileName() const
{
	return "documents.tga";
}


bool emStocksFilePanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();

	if (IsSignaled(GetVirFileStateSignal())) {
		UpdateControls();
	}

	return busy;
}


void emStocksFilePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (IsVFSGood() && ListBox) {
		if (event.IsKey(EM_KEY_H) && state.IsShiftAltMod()) {
			Config->MinVisibleInterest=emStocksFileModel::HIGH_INTEREST;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_M) && state.IsShiftAltMod()) {
			Config->MinVisibleInterest=emStocksFileModel::MEDIUM_INTEREST;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_L) && state.IsShiftAltMod()) {
			Config->MinVisibleInterest=emStocksFileModel::LOW_INTEREST;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_N) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_NAME;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_T) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_TRADE_DATE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_I) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_INQUIRY_DATE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_A) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_ACHIEVEMENT;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_1) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_ONE_WEEK_RISE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_3) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_THREE_WEEK_RISE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_9) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_NINE_WEEK_RISE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_D) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_DIVIDEND;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_P) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_PURCHASE_VALUE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_V) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_VALUE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_F) && state.IsShiftAltMod()) {
			Config->Sorting=emStocksConfig::SORT_BY_DIFFERENCE;
			event.Eat();
		}
		if (event.IsKey(EM_KEY_O) && state.IsShiftAltMod()) {
			Config->OwnedSharesFirst=!Config->OwnedSharesFirst.Get();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_J) && state.IsCtrlMod()) {
			ListBox->GoBackInHistory();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_K) && state.IsCtrlMod()) {
			ListBox->GoForwardInHistory();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_N) && state.IsCtrlMod()) {
			ListBox->NewStock();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_X) && state.IsCtrlMod()) {
			ListBox->CutStocks();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_C) && state.IsCtrlMod()) {
			ListBox->CopyStocks();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_V) && state.IsCtrlMod()) {
			ListBox->PasteStocks();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsNoMod()) {
			ListBox->DeleteStocks();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_P) && state.IsCtrlMod()) {
			ListBox->StartToFetchSharePrices();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_H) && state.IsAltMod()) {
			ListBox->SetInterest(emStocksFileModel::HIGH_INTEREST);
			event.Eat();
		}
		if (event.IsKey(EM_KEY_M) && state.IsAltMod()) {
			ListBox->SetInterest(emStocksFileModel::MEDIUM_INTEREST);
			event.Eat();
		}
		if (event.IsKey(EM_KEY_L) && state.IsAltMod()) {
			ListBox->SetInterest(emStocksFileModel::LOW_INTEREST);
			event.Eat();
		}
		if (event.IsKey(EM_KEY_W) && state.IsCtrlMod()) {
			ListBox->ShowFirstWebPages();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_W) && state.IsShiftCtrlMod()) {
			ListBox->ShowAllWebPages();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_H) && state.IsCtrlMod()) {
			ListBox->FindSelected();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_G) && state.IsCtrlMod()) {
			ListBox->FindNext();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_G) && state.IsShiftCtrlMod()) {
			ListBox->FindPrevious();
			event.Eat();
		}
	}

	emFilePanel::Input(event,state,mx,my);
}


bool emStocksFilePanel::IsOpaque() const
{
	if (!IsVFSGood()) {
		return emFilePanel::IsOpaque();
	}
	return false;
}


void emStocksFilePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	if (!IsVFSGood()) {
		emFilePanel::Paint(painter,canvasColor);
	}
	else {
		painter.Clear(BgColor,canvasColor);
	}
}


void emStocksFilePanel::LayoutChildren()
{
	if (ListBox) {
		ListBox->Layout(0.0,0.0,1.0,GetHeight(),BgColor);
	}
}


emPanel * emStocksFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (FileModel && ListBox) {
		return new emStocksControlPanel(parent,name,*FileModel,*Config,*ListBox);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emStocksFilePanel::UpdateControls()
{
	if (IsVFSGood()) {
		if (!ListBox) {
			ListBox=new emStocksListBox(this,"",*FileModel,*Config);
			AddWakeUpSignal(ListBox->GetSelectedDateSignal());
			InvalidateControlPanel();
			if (IsActive()) {
				ListBox->Layout(0.0,0.0,1.0,GetHeight(),BgColor);
				ListBox->Activate(IsActivatedAdherent());
			}
			SetFocusable(false);
		}
	}
	else {
		if (ListBox) {
			SetFocusable(true);
			delete ListBox;
			ListBox=NULL;
			InvalidateControlPanel();
		}
	}
}
