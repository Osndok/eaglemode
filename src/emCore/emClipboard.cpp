//------------------------------------------------------------------------------
// emClipboard.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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
