//------------------------------------------------------------------------------
// emAvFpPlugin.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emFpPlugin.h>
#include <emAv/emAvFilePanel.h>


extern "C" {
	emPanel * emAvFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		if (
			plugin->Properties.GetCount()!=1 ||
			plugin->Properties[0].Name.Get()!="ServerProc"
		) {
			*errorBuf=
				"emAvFpPlugin: One property required: \"ServerProc\""
			;
			return NULL;
		}
		return new emAvFilePanel(
			parent,name,
			emAvFileModel::Acquire(
				parent.GetRootContext(),
				path,
				emGetChildPath(
					emGetInstallPath(EM_IDT_LIB,"emAv","emAv"),
					plugin->Properties[0].Value.Get()
				)
			)
		);
	}
}
