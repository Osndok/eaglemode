//------------------------------------------------------------------------------
// emJsonPositionTracker.cpp
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

#include <emJson/emJsonPositionTracker.h>


//==============================================================================
//=============================== emJsonPosition ===============================
//==============================================================================

emString emJsonPosition::ToString() const
{
	if (Line<UINT_MAX && Column<UINT_MAX) {
		return emString::Format("line %u, column %u",Line,Column);
	}
	else {
		return "extreme position";
	}
}


//==============================================================================
//=========================== emJsonPositionTracker ============================
//==============================================================================

emJsonPositionTracker::emJsonPositionTracker()
	: State(READY),
	ReadyMagic(127-32),
	Len(0)
{
}


void emJsonPositionTracker::ParseImpl(char c)
{
	if (State==READY) {
L_READY:
		if ((unsigned char)c<32) {
			switch (c) {
			case '\t':
				Pos.Column=((Pos.Column-1+8)&~7)+1;
				break;
			case '\n':
				Pos.Column=1;
				if (Pos.Line<UINT_MAX) Pos.Line++;
				break;
			case '\r':
				Pos.Column=1;
				if (Pos.Line<UINT_MAX) Pos.Line++;
				State=CARRIAGE_RETURN;
				ReadyMagic=0;
				break;
			default:
				break;
			}
		}
		else if ((unsigned char)c<127) {
			if (Pos.Column<UINT_MAX) Pos.Column++;
		}
		else if (c==127) {
			if (Pos.Column>1 && Pos.Column<UINT_MAX) Pos.Column--;
		}
		else {
			Len=0;
			State=BUFFERED;
			ReadyMagic=0;
			goto L_BUFFERED;
		}
	}
	else if (State==CARRIAGE_RETURN) {
		State=READY;
		ReadyMagic=127-32;
		if (c!='\n') goto L_READY;
	}
	else {
L_BUFFERED:
		Buf[Len++]=c;
		for (;;) {
			int ucs4=0;
			int r=emDecodeCharStrictly(&ucs4,Buf,Len,&MBState);
			if (r==-2 && Len<(int)sizeof(Buf)) break;
			if (r!=0 && Pos.Column<UINT_MAX) Pos.Column++;
			if (r<=0) r=1;
			Len-=r;
			if (Len<=0) {
				State=READY;
				ReadyMagic=127-32;
				break;
			}
			memmove(Buf,Buf+r,Len);
		}
	}
}
