//------------------------------------------------------------------------------
// emCursor.cpp
//
// Copyright (C) 2011 Oliver Hamann.
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

#include <emCore/emCursor.h>


const char * emCursor::ToString() const
{
	const char * p;

	switch (CursorId) {
	case NORMAL:
		p="NORMAL";
		break;
	case INVISIBLE:
		p="INVISIBLE";
		break;
	case WAIT:
		p="WAIT";
		break;
	case CROSSHAIR:
		p="CROSSHAIR";
		break;
	case TEXT:
		p="TEXT";
		break;
	case HAND:
		p="HAND";
		break;
	case LEFT_RIGHT_ARROW:
		p="LEFT_RIGHT_ARROW";
		break;
	case UP_DOWN_ARROW:
		p="UP_DOWN_ARROW";
		break;
	case LEFT_RIGHT_UP_DOWN_ARROW:
		p="LEFT_RIGHT_UP_DOWN_ARROW";
		break;
	default:
		p="UNKNOWN";
		break;
	};

	return p;
}
