//------------------------------------------------------------------------------
// emTextFpPlugin.cpp
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
#include <emText/emTextFilePanel.h>


extern "C" {
	emPanel * emTextFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		bool alt, altFound;
		emFpPlugin::PropertyRec * prop;
		int i;

		alt=false;
		altFound=false;
		for (i=0; i<plugin->Properties.GetCount(); i++) {
			prop=&plugin->Properties[i];
			if (!altFound && prop->Name.Get()=="AlternativeView") {
				if (strcasecmp(prop->Value.Get().Get(),"yes")==0) {
					alt=true;
				}
				else if (strcasecmp(prop->Value.Get().Get(),"no")==0) {
					alt=false;
				}
				else {
					*errorBuf=
						"emTextFpPlugin: Illegal value for property \"AlternativeView\" (must be \"yes\" or \"no\")."
					;
					return NULL;
				}
				altFound=true;
			}
			else {
				*errorBuf=emString::Format(
					"emTextFpPlugin: Unsupported or duplicated property: %s",
					prop->Name.Get().Get()
				);
				return NULL;
			}
		}

		return new emTextFilePanel(
			parent,
			name,
			emTextFileModel::Acquire(parent.GetRootContext(),path),
			true,
			alt
		);
	}
}
