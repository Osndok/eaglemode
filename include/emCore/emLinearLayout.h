//------------------------------------------------------------------------------
// emLinearLayout.h
//
// Copyright (C) 2015,2017 Oliver Hamann.
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

#ifndef emLinearLayout_h
#define emLinearLayout_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//=============================== emLinearLayout ===============================
//==============================================================================

class emLinearLayout : public emBorder {

public:

	// A panel of this class automatically lays out its child panels within
	// the content area, either horizontally or vertically. The child panels
	// can get different sizes by applying weights, minimums, maximums or
	// fixed tallnesses to the panels.
	//
	//                   Horizontal layout
	//   +---------------------------------------------+
	//   | +---+  +------------+  +-----+  +--------+  |
	//   | |   |  |            |  |     |  |        |  |
	//   | |   |  |            |  |     |  |        |  |
	//   | +---+  +------------+  +-----+  +--------+  |
	//   +---------------------------------------------+
	//
	//                   Vertical layout
	//                  +---------------+
	//                  | +-----------+ |
	//                  | |           | |
	//                  | |           | |
	//                  | |           | |
	//                  | |           | |
	//                  | +-----------+ |
	//                  |               |
	//                  | +-----------+ |
	//                  | |           | |
	//                  | +-----------+ |
	//                  |               |
	//                  | +-----------+ |
	//                  | |           | |
	//                  | |           | |
	//                  | +-----------+ |
	//                  +---------------+
	//
	// By default, a panel of this class is not focusable and has no border,
	// because it is meant as a pure layout programming tool. For other use,
	// please see the derived class emLinearGroup.

	emLinearLayout(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emBorder, but sets non-focusable.

	virtual ~emLinearLayout();
		// Destructor.

	void SetHorizontal();
		// Set horizontal layout.
		// (This is a short cut for SetOrientationThresholdTallness(1E100).)

	void SetVertical();
		// Set vertical layout.
		// (This is a short cut for SetOrientationThresholdTallness(0.0).)

	double GetOrientationThresholdTallness() const;
	void SetOrientationThresholdTallness(double tallness);
		// Threshold tallness for deciding the orientation of the of
		// layout. If the tallness of the content area of the panel is
		// greater than this threshold, the orientation is vertical,
		// otherwise it is horizontal.

	int GetMinCellCount() const;
	void SetMinCellCount(int minCellCount);
		// Minimum number of cells to be generated. The layout algorithm
		// behaves like if there were at least this number of child
		// panels. The additional cells are simply making up unused
		// space. The default is zero.

	double GetChildWeight(int index) const;
	void SetChildWeight(int index, double weight);
		// Get or set the weight of a child panel. The bigger the weight
		// of a child panel, the bigger is the proportion of the
		// available area given to that panel by the layout algorithm.
		// It is a simple linear relation. The index argument denotes a
		// child panel. Zero means first child, one means second, and so
		// on. The default weight is 1.0.

	void SetChildWeight(double weight);
		// Set the weight of all child panels to the given value.

	double GetMinChildTallness(int index) const;
	void SetMinChildTallness(int index, double minCT);
		// Get or set the minimum tallness (height/width ratio) of a
		// child panel. The index argument denotes a child panel. Zero
		// means first child, one means second, and so on. The default
		// minimum tallness is 0.0.

	void SetMinChildTallness(double minCT);
		// Set the minimum tallness of all child panels to the given
		// value.

	double GetMaxChildTallness(int index) const;
	void SetMaxChildTallness(int index, double maxCT);
		// Get or set the maximum tallness (height/width ratio) of a
		// child panel. The index argument denotes a child panel. Zero
		// means first child, one means second, and so on. The default
		// maximum tallness is 1E100.

	void SetMaxChildTallness(double maxCT);
		// Set the maximum tallness of all child panels to the given
		// value.

	void SetChildTallness(int index, double tallness);
		// Set a fixed tallness for a child panel. This actually sets
		// the minimum and maximum tallness to the given value.

	void SetChildTallness(double tallness);
		// Set a fixed tallness for all child panels.

	emAlignment GetAlignment() const;
	void SetAlignment(emAlignment alignment);
		// Alignment of the child panels as a whole within the available
		// space. This applies only if the available space cannot be
		// filled completely because of minimum, maximum or fixed
		// tallnesses of the child panels. The default is
		// EM_ALIGN_CENTER.

	double GetSpaceL() const;
	double GetSpaceT() const;
	double GetSpaceH() const;
	double GetSpaceV() const;
	double GetSpaceR() const;
	double GetSpaceB() const;
	void SetSpaceL(double l);
	void SetSpaceT(double t);
	void SetSpaceH(double h);
	void SetSpaceV(double v);
	void SetSpaceR(double r);
	void SetSpaceB(double b);
	void SetSpace(double l, double t, double h, double v, double r,
	              double b);
	void SetSpace(double lr, double tb, double h, double v);
	void SetInnerSpace(double h, double v);
	void SetOuterSpace(double l, double t, double r, double b);
	void SetOuterSpace(double lr, double tb);
		// Left, top, horizontal, vertical, right and bottom space (l,
		// t, h, v, r, b). This defines space between child panels, and
		// between child panels and borders. Here is a chart showing the
		// meaning of the six parameters:
		//
		//   +-------------------- border -------------------+
		//   |         t             t             t         |
		//   |     +-------+     +-------+     +-------+     |
		//   |  l  | panel |  h  | panel |  h  | panel |  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         b             b             b         |
		//   +-----------------------------------------------+
		//
		//
		//   +------ border -----+
		//   |         t         |
		//   |     +-------+     |
		//   |  l  | panel |  r  |
		//   |     +-------+     |
		//   |         v         |
		//   |     +-------+     |
		//   |  l  | panel |  r  |
		//   |     +-------+     |
		//   |         v         |
		//   |     +-------+     |
		//   |  l  | panel |  r  |
		//   |     +-------+     |
		//   |         b         |
		//   +-------------------+
		//
		// The method argument lr means to set l and r to the same
		// value, same with tb for t and b.
		//
		// The values are relative to the average size of the child
		// panels. For example, v=0.5 means that the vertical space
		// between child panels gets half as tall as an average child
		// panel. The default is zero for all parameters.

protected:

	virtual void LayoutChildren();
		// Lays out all child panels in the content area (except for an
		// auxiliary panel, which is laid out in the border).

private:

	double CalculateForce(int cells, double w, double h, bool horizontal);

	double OrientationThreshold;
	double DefaultWeight,DefaultMinTallness,DefaultMaxTallness;
	double SpaceL,SpaceT,SpaceH,SpaceV,SpaceR,SpaceB;
	emArray<double> WeightArray;
	emArray<double> MinCTArray;
	emArray<double> MaxCTArray;
	int MinCellCount;
	emAlignment Alignment;
};

inline double emLinearLayout::GetOrientationThresholdTallness() const
{
	return OrientationThreshold;
}

inline int emLinearLayout::GetMinCellCount() const
{
	return MinCellCount;
}

inline emAlignment emLinearLayout::GetAlignment() const
{
	return Alignment;
}

inline double emLinearLayout::GetSpaceL() const
{
	return SpaceL;
}

inline double emLinearLayout::GetSpaceT() const
{
	return SpaceT;
}

inline double emLinearLayout::GetSpaceH() const
{
	return SpaceH;
}

inline double emLinearLayout::GetSpaceV() const
{
	return SpaceV;
}

inline double emLinearLayout::GetSpaceR() const
{
	return SpaceR;
}

inline double emLinearLayout::GetSpaceB() const
{
	return SpaceB;
}


#endif
