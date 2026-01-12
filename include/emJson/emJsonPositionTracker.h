//------------------------------------------------------------------------------
// emJsonPositionTracker.h
//
// Copyright (C) 2025 Oliver Hamann.
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

#ifndef emJsonPositionTracker_h
#define emJsonPositionTracker_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//=============================== emJsonPosition ===============================
//==============================================================================

class emJsonPosition {

public:

	emJsonPosition(unsigned line=1, unsigned column=1);

	emString ToString() const;

private:
	friend class emJsonPositionTracker;

	unsigned Line;
	unsigned Column;
};


//==============================================================================
//=========================== emJsonPositionTracker ============================
//==============================================================================

class emJsonPositionTracker {

public:

	emJsonPositionTracker();

	void Parse(char c);

	const emJsonPosition & GetPosition() const;

private:

	void ParseImpl(char c);

	enum StateEnum {
		READY,
		CARRIAGE_RETURN,
		BUFFERED
	};

	emJsonPosition Pos;
	emMBState MBState;
	StateEnum State;
	unsigned int ReadyMagic;
	char Buf[EM_MB_LEN_MAX];
	int Len;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline emJsonPosition::emJsonPosition(unsigned line, unsigned column)
	: Line(line),
	Column(column)
{
}

inline void emJsonPositionTracker::Parse(char c)
{
	// Optimization of: if (c<32 || c>=127 || State!=READY)
	if (((unsigned int)(unsigned char)c)-32 >= ReadyMagic) ParseImpl(c);
	else if (Pos.Column<UINT_MAX) Pos.Column++;
}

inline const emJsonPosition & emJsonPositionTracker::GetPosition() const
{
	return Pos;
}


#endif
