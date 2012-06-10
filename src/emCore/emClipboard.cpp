//------------------------------------------------------------------------------
// emClipboard.cpp
//
// Copyright (C) 2005-2008,2011 Oliver Hamann.
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

#include <emCore/emVarModel.h>
#include <emCore/emClipboard.h>


//==============================================================================
//================================ emClipboard =================================
//==============================================================================

emRef<emClipboard> emClipboard::LookupInherited(emContext & context)
{
	return emVarModel<emRef<emClipboard> >::GetInherited(
		context,
		"emClipboard::InstalledRef",
		emRef<emClipboard>()
	);
}


emClipboard::emClipboard(emContext & context, const emString & name)
	: emModel(context,name)
{
}


void emClipboard::Install()
{
	emVarModel<emRef<emClipboard> >::Set(
		GetContext(),
		"emClipboard::InstalledRef",
		emRef<emClipboard>(this),
		UINT_MAX
	);
}


//==============================================================================
//============================= emPrivateClipboard =============================
//==============================================================================

void emPrivateClipboard::Install(emContext & context)
{
	emPrivateClipboard * m;
	emString name;

	m=(emPrivateClipboard*)context.Lookup(typeid(emPrivateClipboard),name);
	if (!m) {
		m=new emPrivateClipboard(context,name);
		m->Register();
	}
	m->emClipboard::Install();
}


emInt64 emPrivateClipboard::PutText(const emString & str, bool selection)
{
	if (selection) {
		SelText=str;
		SelId++;
		return SelId;
	}
	else {
		ClipText=str;
		return 0;
	}
}


void emPrivateClipboard::Clear(bool selection, emInt64 selectionId)
{
	if (selection) {
		if (SelId==selectionId) {
			SelText.Empty();
			SelId++;
		}
	}
	else {
		ClipText.Empty();
	}
}


emString emPrivateClipboard::GetText(bool selection)
{
	if (selection) {
		return SelText;
	}
	else {
		return ClipText;
	}
}


emPrivateClipboard::emPrivateClipboard(emContext & context, const emString & name)
	: emClipboard(context,name)
{
	SelId=1;
}


emPrivateClipboard::~emPrivateClipboard()
{
}
