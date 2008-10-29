//------------------------------------------------------------------------------
// emTmpConvFpPlugin.cpp
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

#include <emCore/emFpPlugin.h>
#include <emTmpConv/emTmpConvFramePanel.h>


extern "C" {
	emPanel * emTmpConvFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		emString outFileEnding;
		emString command;
		bool outFileEndingFound;
		bool commandFound;
		emFpPlugin::PropertyRec * prop;
		int i;

		outFileEndingFound=false;
		commandFound=false;
		for (i=0; i<plugin->Properties.GetCount(); i++) {
			prop=&plugin->Properties[i];
			if (!outFileEndingFound && prop->Name.Get()=="OutFileEnding") {
				outFileEnding=prop->Value;
				outFileEndingFound=true;
			}
			else if (!commandFound && prop->Name.Get()=="Command") {
				command=prop->Value;
				commandFound=true;
			}
			else {
				*errorBuf=emString::Format(
					"emTmpConvFpPlugin: Unsupported or duplicated property: %s",
					prop->Name.Get().Get()
				);
				return NULL;
			}
		}
		if (!outFileEndingFound) {
			*errorBuf="emTmpConvFpPlugin: Missing property: OutFileEnding";
			return NULL;
		}
		if (!commandFound) {
			*errorBuf="emTmpConvFpPlugin: Missing property: Command";
			return NULL;
		}
		return new emTmpConvFramePanel(
			parent,
			name,
			emTmpConvModel::Acquire(
				parent.GetRootContext(),
				path,
				command,
				outFileEnding
			)
		);
	}
}
