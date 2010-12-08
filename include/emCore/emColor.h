//------------------------------------------------------------------------------
// emColor.h
//
// Copyright (C) 2001,2003-2008,2010 Oliver Hamann.
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

#ifndef emColor_h
#define emColor_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//================================== emColor ===================================
//==============================================================================

class emColor {

public:

	// Class for a 32-bit color value. A color has four components, each of
	// eight bits: red, green, blue and alpha. An alpha value of 255 means
	// opaque, 0 means totally transparent. A here so called "packed color"
	// is a 32-bit unsigned integer with:
	//   Bits 0-7  : Alpha component
	//   Bits 8-15 : Blue component
	//   Bits 16-23: Green component
	//   Bits 24-31: Red component

	enum {
		// Some predefined packed colors.
		BLACK  =0x000000ff,
		WHITE  =0xffffffff,
		RED    =0xff0000ff,
		GREEN  =0x00ff00ff,
		BLUE   =0x0000ffff,
		YELLOW =0xffff00ff,
		CYAN   =0x00ffffff,
		MAGENTA=0xff00ffff
	};

	emColor();
		// Performs no initialization! (emColor is a primitive type)

	emColor(emUInt32 packed);
		// Construct from a packed color.

	emColor(emByte red, emByte green, emByte blue, emByte alpha=255);
		// Construct from color components.

	emColor(const emColor & color);
		// Construct by copying a color.

	emColor(const emColor & rgb, emByte alpha);
		// Construct by copying a color, but override the alpha
		// component.

	void TryParse(const char * str) throw(emString);
		// Try to set this color by interpreting the given string as a
		// color. Throw an error message on failure. Currently, this
		// tries to interpret only X11 color strings like "#f8a91c" or
		// "Powder Blue". Future extension may even accept additional
		// formats.

	emColor & operator = (const emColor & color);
	emColor & operator = (emUInt32 packed);
		// Copy a color or a packed color.

	operator emUInt32 () const;
	emUInt32 Get() const;
		// Convert this color to a packed color.

	void Set(emUInt32 packed);
	void Set(emByte red, emByte green, emByte blue, emByte alpha=255);
	void Set(const emColor & color);
	void Set(const emColor & rgb, emByte alpha);
		// Like the constructors.

	emByte GetRed() const;
	emByte GetGreen() const;
	emByte GetBlue() const;
	emByte GetAlpha() const;
	void SetRed(emByte red);
	void SetGreen(emByte green);
	void SetBlue(emByte blue);
	void SetAlpha(emByte alpha);
		// Get or set individual components.

	bool IsTotallyTransparent() const;
		// Ask whether the alpha component is 0.

	bool IsOpaque() const;
		// Ask whether the alpha component is 255.

	bool IsGrey() const;
		// Ask whether the red, green and blue components are equal.

	emByte GetGrey() const;
	void SetGrey(emByte grey, emByte alpha=255);
		// Get or set in grey format.

	float GetHue() const;
	float GetSat() const;
	float GetVal() const;
	void SetHue(float hue);
	void SetSat(float sat);
	void SetVal(float val);
	void SetHSVA(float hue, float sat, float val, emByte alpha=255);
		// Get or set in Hue-Saturation-Value format. Hue is in degrees
		// (0.0-360.0). Sat and val are in percent (0.0-100.0).

	emColor GetBlended(const emColor & color, float weight) const;
		// Return a blending of this color and a given color. The given
		// weight of the given color is in percent.

	emColor GetLighted(float light) const;
		// Get a shaded or lighted version of this color. The light
		// parameter ranges from -100.0 to 100.0. -100.0 means black
		// (fully shaded), 0.0 means no change, 100.0 means white (fully
		// highlighted).

	emColor GetTransparented(float tp) const;
		// Get a more opaque or transparent version of this color. The
		// argument tp ranges from -100.0 to 100.0. -100.0 means fully
		// opaque, 0.0 means no change, 100.0 means fully transparent.

	bool operator == (const emColor & color) const;
	bool operator != (const emColor & color) const;
	bool operator == (emUInt32 packed) const;
	bool operator != (emUInt32 packed) const;
	friend bool operator == (emUInt32 packed, const emColor & color);
	friend bool operator != (emUInt32 packed, const emColor & color);
		// Compare colors.

private:
	union {
		emUInt32 Packed;
		struct {
#			if EM_BYTE_ORDER==4321
				emByte Red, Green, Blue, Alpha;
#			elif EM_BYTE_ORDER==1234
				emByte Alpha, Blue, Green, Red;
#			elif EM_BYTE_ORDER==3412
				emByte Green, Red, Alpha, Blue;
#			else
#				error unexpected value for EM_BYTE_ORDER
#			endif
		} Components;
	};
};

inline emColor::emColor()
{
}

inline emColor::emColor(emUInt32 packed)
{
	Packed=packed;
}

inline emColor::emColor(emByte red, emByte green, emByte blue, emByte alpha)
{
	Components.Red=red;
	Components.Green=green;
	Components.Blue=blue;
	Components.Alpha=alpha;
}

inline emColor::emColor(const emColor & color)
{
	Packed=color.Packed;
}

inline emColor::emColor(const emColor & rgb, emByte alpha)
{
	// Did not work error-free with every compiler:
	//  Packed=rgb.Packed;
	//  Components.Alpha=alpha;
	// Therefore:
	Components.Red=rgb.Components.Red;
	Components.Green=rgb.Components.Green;
	Components.Blue=rgb.Components.Blue;
	Components.Alpha=alpha;
}

inline emColor & emColor::operator = (const emColor & color)
{
	Packed=color.Packed;
	return *this;
}

inline emColor & emColor::operator = (emUInt32 packed)
{
	Packed=packed;
	return *this;
}

inline emColor::operator emUInt32 () const
{
	return Packed;
}

inline emUInt32 emColor::Get() const
{
	return Packed;
}

inline void emColor::Set(emUInt32 packed)
{
	Packed=packed;
}

inline void emColor::Set(emByte red, emByte green, emByte blue, emByte alpha)
{
	Components.Red=red;
	Components.Green=green;
	Components.Blue=blue;
	Components.Alpha=alpha;
}

inline void emColor::Set(const emColor & color)
{
	Packed=color.Packed;
}

inline void emColor::Set(const emColor & rgb, emByte alpha)
{
	// Did not work error-free with every compiler:
	//  Packed=rgb.Packed;
	//  Components.Alpha=alpha;
	// Therefore:
	Components.Red=rgb.Components.Red;
	Components.Green=rgb.Components.Green;
	Components.Blue=rgb.Components.Blue;
	Components.Alpha=alpha;
}

inline emByte emColor::GetRed() const
{
	return Components.Red;
}

inline emByte emColor::GetGreen() const
{
	return Components.Green;
}

inline emByte emColor::GetBlue() const
{
	return Components.Blue;
}

inline emByte emColor::GetAlpha() const
{
	return Components.Alpha;
}

inline void emColor::SetRed(emByte red)
{
	Components.Red=red;
}

inline void emColor::SetGreen(emByte green)
{
	Components.Green=green;
}

inline void emColor::SetBlue(emByte blue)
{
	Components.Blue=blue;
}

inline void emColor::SetAlpha(emByte alpha)
{
	Components.Alpha=alpha;
}

inline bool emColor::IsTotallyTransparent() const
{
	return Components.Alpha==0;
}

inline bool emColor::IsOpaque() const
{
	return Components.Alpha==255;
}

inline bool emColor::IsGrey() const
{
	return
		Components.Red==Components.Green &&
		Components.Red==Components.Blue
	;
}

inline emByte emColor::GetGrey() const
{
	return (emByte)(
		(
			((int)Components.Red)+
			((int)Components.Green)+
			((int)Components.Blue)+
			1
		)/3
	);
}

inline void emColor::SetGrey(emByte grey, emByte alpha)
{
	Components.Red=grey;
	Components.Green=grey;
	Components.Blue=grey;
	Components.Alpha=alpha;
}

inline void emColor::SetHue(float hue)
{
	SetHSVA(hue,GetSat(),GetVal(),GetAlpha());
}

inline void emColor::SetSat(float sat)
{
	SetHSVA(GetHue(),sat,GetVal(),GetAlpha());
}

inline void emColor::SetVal(float val)
{
	SetHSVA(GetHue(),GetSat(),val,GetAlpha());
}

inline bool emColor::operator == (const emColor & color) const
{
	return Packed==color.Packed;
}

inline bool emColor::operator != (const emColor & color) const
{
	return Packed!=color.Packed;
}

inline bool emColor::operator == (emUInt32 packed) const
{
	return Packed==packed;
}

inline bool emColor::operator != (emUInt32 packed) const
{
	return Packed!=packed;
}

inline bool operator == (emUInt32 packed, const emColor & color)
{
	return packed==color.Packed;
}

inline bool operator != (emUInt32 packed, const emColor & color)
{
	return packed!=color.Packed;
}


#endif
