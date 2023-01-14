//------------------------------------------------------------------------------
// emStocksFpPlugin.cpp
//
// Copyright (C) 2021 Oliver Hamann.
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

#include <emCore/emFpPlugin.h>
#include <emStocks/emStocksFilePanel.h>


extern "C" {
	emPanel * emStocksFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		if (plugin->Properties.GetCount()) {
			*errorBuf="emStocksFpPlugin: No properties allowed.";
			return NULL;
		}
		return new emStocksFilePanel(
			parent,name,
			emStocksFileModel::Acquire(
				parent.GetRootContext(),path
			)
		);
	}
}
