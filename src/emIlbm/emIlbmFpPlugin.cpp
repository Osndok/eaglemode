//------------------------------------------------------------------------------
// emIlbmFpPlugin.cpp
//
// Copyright (C) 2006-2008,2025 Oliver Hamann.
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
#include <emIlbm/emIlbmImageFileModel.h>


extern "C" {
	emPanel * emIlbmFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		if (plugin->Properties.GetCount()) {
			*errorBuf="emIlbmFpPlugin: No properties allowed.";
			return NULL;
		}
		return new emImageFilePanel(
			parent,name,
			emIlbmImageFileModel::Acquire(
				parent.GetRootContext(),path
			)
		);
	}

	bool emIlbmFpPluginModelFunc(
		emContext & context, const char * className,
		const emString & name, bool common, emFpPlugin * plugin,
		emRef<emModel> * pResult, emString * errorBuf
	)
	{
		if (
			strcmp(className,"emFileModel")==0 ||
			strcmp(className,"emImageFileModel")==0 ||
			strcmp(className,"emIlbmImageFileModel")==0
		)
		{
			*pResult=emIlbmImageFileModel::Acquire(context,name,common);
			return true;
		}
		else {
			*errorBuf=emString::Format(
				"emIlbmFpPluginModelFunc: Unsupported class name: %s",
				className
			);
			return false;
		}
	}
}
