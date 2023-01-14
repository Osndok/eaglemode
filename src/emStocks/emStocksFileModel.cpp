//------------------------------------------------------------------------------
// emStocksFileModel.cpp
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

#include <emStocks/emStocksFileModel.h>


emRef<emStocksFileModel> emStocksFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emStocksFileModel,context,name,common)
}


emStocksFileModel::emStocksFileModel(
	emContext & context, const emString & name
)
	: emRecFileModel(context,name),
	emStocksRec(),
	SaveTimer(GetScheduler())
{
	PostConstruct(*this);
	SetListenedRec(this);
	AddWakeUpSignal(SaveTimer.GetSignal());
}


emStocksFileModel::~emStocksFileModel()
{
	if (SaveTimer.IsRunning()) {
		Save(true);
	}
}


bool emStocksFileModel::Cycle()
{
	if (IsSignaled(SaveTimer.GetSignal())) {
		Save(true);
	}
	return emRecFileModel::Cycle();
}


void emStocksFileModel::OnRecChanged()
{
	SaveTimer.Start(15000);
}
