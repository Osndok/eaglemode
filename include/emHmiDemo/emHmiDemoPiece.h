//------------------------------------------------------------------------------
// emHmiDemoPiece.h
//
// Copyright (C) 2012,2016,2024 Oliver Hamann.
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

#ifndef emHmiDemoPiece_h
#define emHmiDemoPiece_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoPiece : public emPanel {
public:
	emHmiDemoPiece(
		ParentArg parent, const emString & name,
		int rx, int ry, int rw=1, int rh=1,
		const emString & imageName="00",
		emColor color=0x808080FF
	);
	virtual ~emHmiDemoPiece();
	void SetColor(emColor color);
	emColor GetColor() const;
	emColor GetInnerColor() const;
	int GetRX() const;
	int GetRY() const;
	int GetRW() const;
	int GetRH() const;
protected:
	virtual bool IsOpaque() const;
	virtual void Paint(const emPainter & painter, emColor canvasColor) const;
private:
	void UpdateInnerColor();
	emImage Mask;
	emImage Image;
	int RX,RY,RW,RH;
	emColor Color;
	emColor InnerColor;
};


inline emColor emHmiDemoPiece::GetColor() const
{
	return Color;
}

inline emColor emHmiDemoPiece::GetInnerColor() const
{
	return InnerColor;
}

inline int emHmiDemoPiece::GetRX() const
{
	return RX;
}

inline int emHmiDemoPiece::GetRY() const
{
	return RY;
}

inline int emHmiDemoPiece::GetRW() const
{
	return RW;
}

inline int emHmiDemoPiece::GetRH() const
{
	return RH;
}


#endif
