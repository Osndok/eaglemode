//------------------------------------------------------------------------------
// emHmiDemoPieceGroup.h
//
// Copyright (C) 2012,2024 Oliver Hamann.
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

#ifndef emHmiDemoPieceGroup_h
#define emHmiDemoPieceGroup_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoPieceGroup : public emPanel {
public:
	emHmiDemoPieceGroup(
		ParentArg parent, const emString & name,
		int rx, int ry, int rw=1, int rh=1,
		emColor color=0x808080FF
	);
	virtual ~emHmiDemoPieceGroup();
	void SetColor(emColor color);
	emColor GetColor() const;
	int GetRX() const;
	int GetRY() const;
	int GetRW() const;
	int GetRH() const;
protected:
	virtual void LayoutChildren();
private:
	int RX,RY,RW,RH;
	emColor Color;
};


inline emColor emHmiDemoPieceGroup::GetColor() const
{
	return Color;
}

inline int emHmiDemoPieceGroup::GetRX() const
{
	return RX;
}

inline int emHmiDemoPieceGroup::GetRY() const
{
	return RY;
}

inline int emHmiDemoPieceGroup::GetRW() const
{
	return RW;
}

inline int emHmiDemoPieceGroup::GetRH() const
{
	return RH;
}


#endif
