//------------------------------------------------------------------------------
// emGifFpPlugin.cpp
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
#include <emGif/emGifFilePanel.h>
#include <emGif/emGifImageFileModel.h>


extern "C" {
	emPanel * emGifFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		if (plugin->Properties.GetCount()) {
			*errorBuf="emGifFpPlugin: No properties allowed.";
			return NULL;
		}
		return new emGifFilePanel(
			parent,name,
			emGifFileModel::Acquire(parent.GetRootContext(),path)
		);
	}

	bool emGifFpPluginModelFunc(
		emContext & context, const char * className,
		const emString & name, bool common, emFpPlugin * plugin,
		emRef<emModel> * pResult, emString * errorBuf
	)
	{
		if (
			strcmp(className,"emFileModel")==0 ||
			strcmp(className,"emGifFileModel")==0
		)
		{
			*pResult=emGifFileModel::Acquire(context,name,common);
			return true;
		}
		else if (
			strcmp(className,"emImageFileModel")==0 ||
			strcmp(className,"emGifImageFileModel")==0
		)
		{
			*pResult=emGifImageFileModel::Acquire(context,name,common);
			return true;
		}
		else {
			*errorBuf=emString::Format(
				"emGifFpPluginModelFunc: Unsupported class name: %s",
				className
			);
			return false;
		}
	}
}
