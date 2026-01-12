//------------------------------------------------------------------------------
// emWebpFpPlugin.cpp
//
// Copyright (C) 2021,2025 Oliver Hamann.
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
#include <emWebp/emWebpImageFileModel.h>


extern "C" {
	emPanel * emWebpFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		if (plugin->Properties.GetCount()) {
			*errorBuf="emWebpFpPlugin: No properties allowed.";
			return NULL;
		}
		return new emImageFilePanel(
			parent,name,
			emWebpImageFileModel::Acquire(
				parent.GetRootContext(),path
			)
		);
	}

	bool emWebpFpPluginModelFunc(
		emContext & context, const char * className,
		const emString & name, bool common, emFpPlugin * plugin,
		emRef<emModel> * pResult, emString * errorBuf
	)
	{
		if (
			strcmp(className,"emFileModel")==0 ||
			strcmp(className,"emImageFileModel")==0 ||
			strcmp(className,"emWebpImageFileModel")==0
		)
		{
			*pResult=emWebpImageFileModel::Acquire(context,name,common);
			return true;
		}
		else {
			*errorBuf=emString::Format(
				"emWebpFpPluginModelFunc: Unsupported class name: %s",
				className
			);
			return false;
		}
	}
}
