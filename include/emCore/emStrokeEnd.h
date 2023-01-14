//------------------------------------------------------------------------------
// emStrokeEnd.h
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

#ifndef emStrokeEnd_h
#define emStrokeEnd_h

#ifndef emColor_h
#include <emCore/emColor.h>
#endif


//==============================================================================
//================================ emStrokeEnd =================================
//==============================================================================

class emStrokeEnd {

public:

	// Class for specifying how to paint an end of a line.

	enum TypeEnum {
		// Data type for the shape or picture at the end of the line.

		BUTT,
			// Cut the line at the reference point.
			// ********
			// *******|
			// ******-+-
			// *******|
			// ********

		CAP,
			// Extend the line by half of the stroke width, rounded
			// or not depending on emStroke::IsRounded().
			//
			// If stroke not rounded:
			// ***********
			// *******|***
			// ******-+-**
			// *******|***
			// ***********
			// If stroke rounded:
			// ********
			// *******|**
			// ******-+-**
			// *******|**
			// ********

		ARROW,
			//     **
			//     ****
			//      *****
			// ************
			//      *****
			//     ****
			//     **

		CONTOUR_ARROW,
			//     **
			//     * **
			//      *  **
			// *******   **
			//      *  **
			//     * **
			//     **

		LINE_ARROW,
			//     **
			//       **
			//         **
			// ************
			//         **
			//       **
			//     **

		TRIANGLE,
			//     **
			//     ****
			//     ******
			// ************
			//     ******
			//     ****
			//     **

		CONTOUR_TRIANGLE,
			//     **
			//     * **
			//     *   **
			// *****     **
			//     *   **
			//     * **
			//     **

		SQUARE,
			//    ***********
			//    ***********
			//    ***********
			// **************
			//    ***********
			//    ***********
			//    ***********

		CONTOUR_SQUARE,
			//    ***********
			//    *         *
			//    *         *
			// ****         *
			//    *         *
			//    *         *
			//    ***********

		HALF_SQUARE,
			//    ******
			//    *
			//    *
			// ****
			//    *
			//    *
			//    ******

		CIRCLE,
			//       *****
			//     *********
			//    ***********
			// **************
			//    ***********
			//     *********
			//       *****

		CONTOUR_CIRCLE,
			//       *****
			//     **     **
			//    *         *
			// ****         *
			//    *         *
			//     **     **
			//       *****

		HALF_CIRCLE,
			//       ***
			//     **
			//    *
			// ****
			//    *
			//     **
			//       ***

		DIAMOND,
			//        ***
			//      *******
			//    ***********
			// ****************
			//    ***********
			//      *******
			//        ***

		CONTOUR_DIAMOND,
			//        ***
			//      **   **
			//    **       **
			// ***           **
			//    **       **
			//      **   **
			//        ***

		HALF_DIAMOND,
			//        **
			//      **
			//    **
			// ***
			//    **
			//      **
			//        **

		STROKE,
			//        *
			//        *
			//        *
			// ********
			//        *
			//        *
			//        *

		NO_END
			// Only for internal use by emPainter, not part of
			// public API.
	};

	emStrokeEnd(TypeEnum type=BUTT, emColor innerColor=emColor::WHITE,
	            double widthFactor=1.0, double lengthFactor=1.0);
		// Construct a stroke end.
		// Arguments:
		//   type          Type of the stroke end.
		//   innerColor    Inner color if it is a contour type.
		//   widthFactor   Factor for the width of the arrow or other
		//                 picture.
		//   lengthFactor  Factor for the length of the arrow or other
		//                 picture.

	TypeEnum GetType() const;
	void SetType(TypeEnum type);
		// Get or set the type of the stroke end.

	bool IsDecorated() const;
		// Ask whether there is an arrow or other picture. This is true
		// if the type is none of BUTT, CAP, and NO_END.

	emColor GetInnerColor() const;
	void SetInnerColor(emColor innerColor);
		// Get or set the inner color of contour types. This is only
		// CONTOUR_ARROW, CONTOUR_TRIANGLE, CONTOUR_SQUARE,
		// CONTOUR_CIRCLE, and CONTOUR_DIAMOND.

	double GetWidthFactor() const;
	void SetWidthFactor(double widthFactor);
		// Get or set the factor for the width of the arrow or other
		// picture. This is only for types other than BUTT, CAP, and
		// NO_END.

	double GetLengthFactor() const;
	void SetLengthFactor(double lengthFactor);
		// Get or set the factor for the length of the arrow or other
		// picture. This is only for types other than BUTT, CAP, and
		// NO_END.

private:

	friend class emPainter;

	TypeEnum Type;
	emColor InnerColor;
	double WidthFactor;
	double LengthFactor;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline emStrokeEnd::emStrokeEnd(
	TypeEnum type, emColor innerColor, double widthFactor,
	double lengthFactor
) :
	Type(type),
	InnerColor(innerColor),
	WidthFactor(widthFactor),
	LengthFactor(lengthFactor)
{
}

inline emStrokeEnd::TypeEnum emStrokeEnd::GetType() const
{
	return Type;
}

inline void emStrokeEnd::SetType(TypeEnum type)
{
	Type=type;
}

inline bool emStrokeEnd::IsDecorated() const
{
	return Type!=BUTT && Type!=CAP && Type!=NO_END;
}

inline emColor emStrokeEnd::GetInnerColor() const
{
	return InnerColor;
}

inline void emStrokeEnd::SetInnerColor(emColor innerColor)
{
	InnerColor=innerColor;
}

inline double emStrokeEnd::GetWidthFactor() const
{
	return WidthFactor;
}

inline void emStrokeEnd::SetWidthFactor(double widthFactor)
{
	WidthFactor=widthFactor;
}

inline double emStrokeEnd::GetLengthFactor() const
{
	return LengthFactor;
}

inline void emStrokeEnd::SetLengthFactor(double lengthFactor)
{
	LengthFactor=lengthFactor;
}


#endif
