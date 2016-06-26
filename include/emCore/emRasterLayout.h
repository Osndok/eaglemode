//------------------------------------------------------------------------------
// emRasterLayout.h
//
// Copyright (C) 2015 Oliver Hamann.
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

#ifndef emRasterLayout_h
#define emRasterLayout_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//=============================== emRasterLayout ===============================
//==============================================================================

class emRasterLayout : public emBorder {

public:

	// A panel of this class automatically lays out its child panels within
	// the content area, so that they have the same size and are arranged in
	// a rectangular raster.
	//
	//                    Raster layout
	//   +---------------------------------------------+
	//   | +-------+  +-------+  +-------+  +-------+  |
	//   | |       |  |       |  |       |  |       |  |
	//   | |       |  |       |  |       |  |       |  |
	//   | +-------+  +-------+  +-------+  +-------+  |
	//   |                                             |
	//   | +-------+  +-------+  +-------+  +-------+  |
	//   | |       |  |       |  |       |  |       |  |
	//   | |       |  |       |  |       |  |       |  |
	//   | +-------+  +-------+  +-------+  +-------+  |
	//   |                                             |
	//   | +-------+  +-------+  +-------+             |
	//   | |       |  |       |  |       |             |
	//   | |       |  |       |  |       |             |
	//   | +-------+  +-------+  +-------+             |
	//   +---------------------------------------------+
	//
	// By default, a panel of this class is not focusable and has no border,
	// because it is meant as a pure layout programming tool. For other use,
	// please see the derived class emRasterGroup.

	emRasterLayout(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emBorder, but sets non-focusable.

	virtual ~emRasterLayout();
		// Destructor.

	bool IsRowByRow() const;
	void SetRowByRow(bool rowByRow=true);
		// Whether to layout the child panels column-by-column (false,
		// the default) or row-by-row (true). Here is an example for the
		// order of 10 panels in a 4x3 raster:
		//   column-by-column      row-by-row
		//      1  4  7 10         1  2  3  4
		//      2  5  8            5  6  7  8
		//      3  6  9            9 10

	int GetFixedColumnCount() const;
	int GetFixedRowCount() const;
	void SetFixedColumnCount(int fixedColumnCount);
	void SetFixedRowCount(int fixedRowCount);
		// Whether to have a fixed number of columns or rows, and how
		// many. A value less or equal zero means to have no fixed
		// number of columns or rows and to let the layout algorithm
		// decide it. If both are fixed and if there are more child
		// panels than cells, the row count is increased accordingly.
		// The default is zero for both.

	int GetMinCellCount() const;
	void SetMinCellCount(int minCellCount);
		// Minimum number of cells to be generated. The layout algorithm
		// behaves like if there were at least this number of child
		// panels. The additional cells are simply making up unused
		// space. The default is zero.

	double GetPrefChildTallness() const;
	void SetPrefChildTallness(double prefCT);
		// Preferred tallness of child panels (height/width ratio). The
		// child panels may get another tallness, for filling the
		// available space best possible. The default is 0.1

	double GetMinChildTallness() const;
	void SetMinChildTallness(double minCT);
		// Minimum tallness of child panels. The default is 0.0.

	double GetMaxChildTallness() const;
	void SetMaxChildTallness(double maxCT);
		// Maximum tallness of child panels. The default is 1E100.

	void SetChildTallness(double tallness);
		// Set preferred, minimum and maximum child tallness to
		// the given value.

	bool IsStrictRaster() const;
	void SetStrictRaster(bool strictRaster=true);
		// Whether to make the raster layout more strict. The default is
		// false. Normally, the layout algorithm chooses the number of
		// rows and columns so that the child panels are shown as large
		// as possible. Depending on minimum and maximum tallness, the
		// available space may not be filled completely either
		// horizontally or vertically. With "strict raster", the
		// available space is best possible filled vertically in a
		// column-by-column layout or horizontally in a row-by-row
		// layout, respectively. This may result in more rows or columns
		// than originally needed, but it may look better or cleaner,
		// especially when the set of child panels is extensible.

	emAlignment GetAlignment() const;
	void SetAlignment(emAlignment alignment);
		// Alignment of the child panels as a whole within the available
		// space. This applies only if the available space cannot be
		// filled completely because of a minimum, maximum or fixed
		// tallness of the child panels. The default is EM_ALIGN_CENTER.

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
		//   |         v             v             v         |
		//   |     +-------+     +-------+     +-------+     |
		//   |  l  | panel |  h  | panel |  h  | panel |  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         b             b             b         |
		//   +-----------------------------------------------+
		//
		// The method argument lr means to set l and r to the same
		// value, same with tb for t and b.
		//
		// The values are relative to the size of the child panels. For
		// example, v=0.5 means that the vertical space between child
		// panels gets half as tall as a child panel. The default is
		// zero for all parameters.

protected:

	virtual void LayoutChildren();
		// Lays out all child panels in the content area (except for an
		// auxiliary panel, which is laid out in the border).

private:

	double PrefCT,MinCT,MaxCT;
	double SpaceL,SpaceT,SpaceH,SpaceV,SpaceR,SpaceB;
	int FixedColumnCount,FixedRowCount,MinCellCount;
	emAlignment Alignment;
	bool StrictRaster;
	bool RowByRow;
};

inline bool emRasterLayout::IsRowByRow() const
{
	return RowByRow;
}

inline int emRasterLayout::GetFixedColumnCount() const
{
	return FixedColumnCount;
}

inline int emRasterLayout::GetFixedRowCount() const
{
	return FixedRowCount;
}

inline int emRasterLayout::GetMinCellCount() const
{
	return MinCellCount;
}

inline double emRasterLayout::GetPrefChildTallness() const
{
	return PrefCT;
}

inline double emRasterLayout::GetMinChildTallness() const
{
	return MinCT;
}

inline double emRasterLayout::GetMaxChildTallness() const
{
	return MaxCT;
}

inline bool emRasterLayout::IsStrictRaster() const
{
	return StrictRaster;
}

inline emAlignment emRasterLayout::GetAlignment() const
{
	return Alignment;
}

inline double emRasterLayout::GetSpaceL() const
{
	return SpaceL;
}

inline double emRasterLayout::GetSpaceT() const
{
	return SpaceT;
}

inline double emRasterLayout::GetSpaceH() const
{
	return SpaceH;
}

inline double emRasterLayout::GetSpaceV() const
{
	return SpaceV;
}

inline double emRasterLayout::GetSpaceR() const
{
	return SpaceR;
}

inline double emRasterLayout::GetSpaceB() const
{
	return SpaceB;
}


#endif
