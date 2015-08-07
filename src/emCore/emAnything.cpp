//------------------------------------------------------------------------------
// emAnything.cpp
//
// Copyright (C) 2015 Oliver Hamann.
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

#include <emCore/emAnything.h>


emAnything::emAnything(const emAnything & anything)
{
	Data=anything.Data;
	if (Data) Data->RefCount++;
}


emAnything::~emAnything()
{
	if (Data && !--Data->RefCount) delete Data;
}


emAnything & emAnything::operator = (const emAnything & anything)
{
	if (Data!=anything.Data) {
		if (Data && !--Data->RefCount) delete Data;
		Data=anything.Data;
		if (Data) Data->RefCount++;
	}
	return *this;
}


emAnything::AbstractSharedData::~AbstractSharedData()
{
}

