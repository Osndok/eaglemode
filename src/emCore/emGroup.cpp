//------------------------------------------------------------------------------
// emGroup.cpp
//
// Copyright (C) 2005-2011,2014-2015 Oliver Hamann.
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

#include <emCore/emGroup.h>


emGroup::emGroup(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTiling(parent,name,caption,description,icon,0)
{
	SetFocusable(true);
	SetBorderType(OBT_GROUP,IBT_GROUP);
}


emGroup::emGroup(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon,
	int notWarningDeprecatedForInternalUse
)
	: emTiling(parent,name,caption,description,icon,0)
{
	SetFocusable(true);
	SetBorderType(OBT_GROUP,IBT_GROUP);
}

