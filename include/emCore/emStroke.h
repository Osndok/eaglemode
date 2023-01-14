//------------------------------------------------------------------------------
// emStroke.h
//
// Copyright (C) 2022 Oliver Hamann.
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

#ifndef emStroke_h
#define emStroke_h

#ifndef emColor_h
#include <emCore/emColor.h>
#endif


//==============================================================================
//================================== emStroke ==================================
//==============================================================================

class emStroke {

public:

	// Class for specifying how to paint a line. This includes the color,
	// whether joins and caps are rounded, the dash type, and dash factors.
	// The rounding and dash features can also easily be specified by using
	// one of the following derived classes:
	//
	//   emRoundedStroke
	//   emDashedStroke
	//   emRoundedDashedStroke
	//   emDottedStroke
	//   emRoundedDottedStroke
	//   emDashDottedStroke
	//   emRoundedDashDottedStroke
	//
	// These stroke classes just exist for convenience and define only
	// constructors. All the attributes are already defined here in
	// emStroke, because emStroke ought to be a simple non-polymorphic type.

	enum DashTypeEnum {
		// Data type for the type of dash.

		SOLID,
			// Have a continuous line without dashes or dots.

		DASHED,
			// Have a dashed line.

		DOTTED,
			// Have a dotted line.

		DASH_DOTTED
			// Have a line with dashes and dots alternating.
	};

	emStroke(emColor color=emColor::BLACK, bool rounded=false,
	         DashTypeEnum dashType=SOLID, double dashLengthFactor=1.0,
	         double gapLengthFactor=1.0);
		// Construct a stroke.
		// Arguments:
		//   color             Color of the stroke.
		//   rounded           Whether joins and caps are rounded.
		//   dashType          Type of dash.
		//   dashLengthFactor  Length factor for dashes.
		//   gapLengthFactor   Length factor for gaps between dashes or
		//                     dots.

	emStroke(emUInt32 packedColor);
		// Construct from a packed color (allows implicit conversion).

	emColor GetColor() const;
	void SetColor(emColor color);
		// Get or set the color of the stroke.

	bool IsRounded() const;
	void SetRounded(bool rounded=true);
		// Get or set whether joins (polyline corners) and caps
		// (see emStrokeEnd::CAP) are rounded.

	DashTypeEnum GetDashType() const;
	void SetDashType(DashTypeEnum dashType);
		// Get or set the type of dash.

	double GetDashLengthFactor() const;
	void SetDashLengthFactor(double dashLengthFactor);
		// Get or set the length factor for dashes.

	double GetGapLengthFactor() const;
	void SetGapLengthFactor(double gapLengthFactor);
		// Get or set the length factor for gaps between dashes or dots.

private:

	friend class emPainter;

	emColor Color;
	bool Rounded;
	DashTypeEnum DashType;
	double DashLengthFactor;
	double GapLengthFactor;
};


//==============================================================================
//============================== emRoundedStroke ===============================
//==============================================================================

class emRoundedStroke : public emStroke {

public:

	emRoundedStroke(emColor color=emColor::BLACK);
		// Construct a stroke for a continues line with rounded joins
		// and caps.
		// Arguments:
		//   color  Color of the stroke.
};


//==============================================================================
//=============================== emDashedStroke ===============================
//==============================================================================

class emDashedStroke : public emStroke {

public:

	emDashedStroke(emColor color=emColor::BLACK,
	               double dashLengthFactor=1.0,
	               double gapLengthFactor=1.0);
		// Construct a stroke for a dashed line.
		// Arguments:
		//   color             Color of the stroke.
		//   dashLengthFactor  Length factor for dashes.
		//   gapLengthFactor   Length factor for gaps between dashes.
};


//==============================================================================
//=========================== emRoundedDashedStroke ============================
//==============================================================================

class emRoundedDashedStroke : public emStroke {

public:

	emRoundedDashedStroke(emColor color=emColor::BLACK,
	                      double dashLengthFactor=1.0,
	                      double gapLengthFactor=1.0);
		// Construct a stroke for a dashed line with rounded joins and
		// caps.
		// Arguments:
		//   color             Color of the stroke.
		//   dashLengthFactor  Length factor for dashes.
		//   gapLengthFactor   Length factor for gaps between dashes.
};


//==============================================================================
//=============================== emDottedStroke ===============================
//==============================================================================

class emDottedStroke : public emStroke {

public:

	emDottedStroke(emColor color=emColor::BLACK,
	               double gapLengthFactor=1.0);
		// Construct a stroke for a dotted line.
		// Arguments:
		//   color            Color of the stroke.
		//   gapLengthFactor  Length factor for gaps between dots.
};


//==============================================================================
//=========================== emRoundedDottedStroke ============================
//==============================================================================

class emRoundedDottedStroke : public emStroke {

public:

	emRoundedDottedStroke(emColor color=emColor::BLACK,
	                      double gapLengthFactor=1.0);
		// Construct a stroke for a dotted line with rounded joins and
		// caps.
		// Arguments:
		//   color            Color of the stroke.
		//   gapLengthFactor  Length factor for gaps between dots.
};


//==============================================================================
//============================= emDashDottedStroke =============================
//==============================================================================

class emDashDottedStroke : public emStroke {

public:

	emDashDottedStroke(emColor color=emColor::BLACK,
	                   double dashLengthFactor=1.0,
	                   double gapLengthFactor=1.0);
		// Construct a stroke for a dash-dotted line.
		// Arguments:
		//   color             Color of the stroke.
		//   dashLengthFactor  Length factor for dashes.
		//   gapLengthFactor   Length factor for gaps between dashes and
		//                     dots.
};


//==============================================================================
//========================= emRoundedDashDottedStroke ==========================
//==============================================================================

class emRoundedDashDottedStroke : public emStroke {

public:

	emRoundedDashDottedStroke(emColor color=emColor::BLACK,
	                          double dashLengthFactor=1.0,
	                          double gapLengthFactor=1.0);
		// Construct a stroke for a dash-dotted line with rounded joins and
		// caps.
		// Arguments:
		//   color             Color of the stroke.
		//   dashLengthFactor  Length factor for dashes.
		//   gapLengthFactor   Length factor for gaps between dashes and
		//                     dots.
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//---------------------------------- emStroke ----------------------------------

inline emStroke::emStroke(
	emColor color, bool rounded, DashTypeEnum dashType,
	double dashLengthFactor, double gapLengthFactor
) :
	Color(color),
	Rounded(rounded),
	DashType(dashType),
	DashLengthFactor(dashLengthFactor),
	GapLengthFactor(gapLengthFactor)
{
}

inline emStroke::emStroke(emUInt32 packedColor)
	: Color(packedColor),
	Rounded(false),
	DashType(SOLID),
	DashLengthFactor(1.0),
	GapLengthFactor(1.0)
{
}

inline emColor emStroke::GetColor() const
{
	return Color;
}

inline void emStroke::SetColor(emColor color)
{
	Color=color;
}

inline bool emStroke::IsRounded() const
{
	return Rounded;
}

inline void emStroke::SetRounded(bool rounded)
{
	Rounded=rounded;
}

inline emStroke::DashTypeEnum emStroke::GetDashType() const
{
	return DashType;
}

inline void emStroke::SetDashType(DashTypeEnum dashType)
{
	DashType=dashType;
}

inline double emStroke::GetDashLengthFactor() const
{
	return DashLengthFactor;
}

inline void emStroke::SetDashLengthFactor(double dashLengthFactor)
{
	DashLengthFactor=dashLengthFactor;
}

inline double emStroke::GetGapLengthFactor() const
{
	return GapLengthFactor;
}

inline void emStroke::SetGapLengthFactor(double gapLengthFactor)
{
	GapLengthFactor=gapLengthFactor;
}


//------------------------------ emRoundedStroke -------------------------------

inline emRoundedStroke::emRoundedStroke(emColor color)
	: emStroke(color,true)
{
}


//------------------------------- emDashedStroke -------------------------------

inline emDashedStroke::emDashedStroke(
	emColor color, double dashLengthFactor, double gapLengthFactor
) :
	emStroke(color,false,DASHED,dashLengthFactor,gapLengthFactor)
{
}


//--------------------------- emRoundedDashedStroke ----------------------------

inline emRoundedDashedStroke::emRoundedDashedStroke(
	emColor color, double dashLengthFactor, double gapLengthFactor
) :
	emStroke(color,true,DASHED,dashLengthFactor,gapLengthFactor)
{
}


//------------------------------- emDottedStroke -------------------------------

inline emDottedStroke::emDottedStroke(
	emColor color, double gapLengthFactor
) :
	emStroke(color,false,DOTTED,1.0,gapLengthFactor)
{
}


//--------------------------- emRoundedDottedStroke ----------------------------

inline emRoundedDottedStroke::emRoundedDottedStroke(
	emColor color, double gapLengthFactor
) :
	emStroke(color,true,DOTTED,1.0,gapLengthFactor)
{
}


//----------------------------- emDashDottedStroke -----------------------------

inline emDashDottedStroke::emDashDottedStroke(
	emColor color, double dashLengthFactor, double gapLengthFactor
) :
	emStroke(color,false,DASH_DOTTED,dashLengthFactor,gapLengthFactor)
{
}


//------------------------- emRoundedDashDottedStroke --------------------------

inline emRoundedDashDottedStroke::emRoundedDashDottedStroke(
	emColor color, double dashLengthFactor, double gapLengthFactor
) :
	emStroke(color,true,DASH_DOTTED,dashLengthFactor,gapLengthFactor)
{
}


#endif
