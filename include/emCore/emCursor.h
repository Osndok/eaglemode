//------------------------------------------------------------------------------
// emCursor.h
//
// Copyright (C) 2005-2012 Oliver Hamann.
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

#ifndef emCursor_h
#define emCursor_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//================================== emCursor ==================================
//==============================================================================

class emCursor {

public:

	// Class for a mouse cursor.

	//??? Allow custom cursors (e.g. construct from an image and
	//??? hot spot coordinates, or from a file).

	enum {
		// Possible values for the cursor id.
		NORMAL                   = 0,
		INVISIBLE                = 1,
		WAIT                     = 2,
		CROSSHAIR                = 3,
		TEXT                     = 4,
		HAND                     = 5,
		LEFT_RIGHT_ARROW         = 6,
		UP_DOWN_ARROW            = 7,
		LEFT_RIGHT_UP_DOWN_ARROW = 8
	};

	emCursor();
		// Construct a normal cursor.

	emCursor(int cursorId);
		// Construct from a cursor id.

	emCursor(const emCursor & cursor);
		// Copy constructor.

	emCursor & operator = (const emCursor & cursor);
	emCursor & operator = (int cursorId);
		// Copy operators.

	operator int () const;
	int Get() const;
		// Get the cursor id.

	const char * ToString() const;
		// Convert to string representation.

private:

	int CursorId;
};

inline emCursor::emCursor()
{
	CursorId=NORMAL;
}

inline emCursor::emCursor(int cursorId)
{
	CursorId=cursorId;
}

inline emCursor::emCursor(const emCursor & cursor)
{
	CursorId=cursor.CursorId;
}

inline emCursor & emCursor::operator = (const emCursor & cursor)
{
	CursorId=cursor.CursorId;
	return *this;
}

inline emCursor & emCursor::operator = (int cursorId)
{
	CursorId=cursorId;
	return *this;
}

inline emCursor::operator int () const
{
	return CursorId;
}

inline int emCursor::Get() const
{
	return CursorId;
}


#endif
