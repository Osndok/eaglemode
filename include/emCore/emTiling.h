//------------------------------------------------------------------------------
// emTiling.h
//
// Copyright (C) 2005-2010,2014-2015 Oliver Hamann.
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

#ifndef emTiling_h
#define emTiling_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================== emTiling ==================================
//==============================================================================

class emTiling : public emBorder {

public:

	// *********************************************************************
	// *                             WARNING!!!                            *
	// *                                                                   *
	// * This class is deprecated and will be removed in a future version. *
	// * Please use emLinearLayout, emRasterLayout or emPackLayout instead.*
	// *********************************************************************
	//
	// A panel of this class automatically lays out any child panels within
	// the content area, just like in a rectangular tiling. By default, the
	// panel itself is not focusable and has no border, because it is meant
	// as a pure layout programming tool. For other use, see the derived
	// class emGroup.

	EM_DEPRECATED( // Because the whole class is deprecated!
		emTiling(
			ParentArg parent, const emString & name,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		)
	);
		// Like emBorder, but sets non-focusable.

	virtual ~emTiling();
		// Destructor.

	bool IsRowByRow() const;
	void SetRowByRow(bool rowByRow=true);
		// Whether to layout the child panels column-by-column (false,
		// the default) or row-by-row (true). Here is an example for the
		// order of 10 panels in a 4x3 grid:
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

	void SetChildTallness(double ct);
		// Set the demanded tallness of child panels (height/width
		// ratio). This method is a short cut for:
		//   SetPrefChildTallness(ct);
		//   SetChildTallnessForced();

	void SetPrefChildTallness(double pct);
		// Set the preferred tallness of child panels (height/width
		// ratio). The child panels may get another tallness, for
		// filling the available space best possible. This method is a
		// short cut for: SetPrefChildTallness(pct,0,true);

	double GetPrefChildTallness(int idx) const;
	void SetPrefChildTallness(double pct, int idx, bool allFurther=true);
		// Preferred tallness of child panels in the first row and in
		// the first column. The index idx denotes a cell. It is a cell
		// in the first row (idx>0) or in the first column (idx<0), or
		// both (idx==0). Other cells cannot be set, because they are
		// sized implicitly by the first row and column. Here is a chart
		// showing the meaning of idx, and the effect of different
		// tallnesses:
		//
		//   +--------+-------+-------------+-----+----
		//   | idx  0 | idx 1 |    idx 2    |idx 3| ...
		//   +--------+-------+-------------+-----+----
		//   |        |       |             |     |
		//   | idx -1 |       |             |     |
		//   |        |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   | idx -2 |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   |        |       |             |     |
		//   |        |       |             |     |
		//   | idx -3 |       |             |     |
		//   |        |       |             |     |
		//   |        |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   | ...    |       |             |     |
		//
		// Now to the important trick argument "allFurther": If true
		// (the default), all further cells in the row and/or column are
		// even set. For example, SetPrefChildTallness(1.0,3) means to
		// set the cells 3, 4, 5, ... INT_MAX. And
		// SetPrefChildTallness(1.0,-2) means to set the cells -2, -3,
		// -4, ... INT_MIN. And SetPrefChildTallness(1.0,-3,false) means
		// to set cell -3 only. And SetPrefChildTallness(1.0,0) means to
		// set all cells at once.
		//
		// There is no limit for idx except through the memory required
		// by internal arrays.
		//
		// The default preferred child tallness is: 0.2

	void SetChildTallnessForced(bool forced=true);
		// Whether the preferred child tallness has to be applied in any
		// case, instead of deviating for filling the whole available
		// space. This method is a short cut for:
		//   SetForcedChildTallnessColumn(forced ? 0 : -1);
		//   SetForcedChildTallnessRow(forced ? 0 : -1);

	int GetForcedChildTallnessColumn() const;
	int GetForcedChildTallnessRow() const;
	void SetForcedChildTallnessColumn(int column);
	void SetForcedChildTallnessRow(int row);
		// Index of a single column or row, in which the preferred child
		// tallness has to be applied in any case. -1 means to have no
		// such column or row (this is the default). Understand that if
		// both a row and a column are forced, all cells of the whole
		// grid are forced implicitly.

	emAlignment GetAlignment() const;
	void SetAlignment(emAlignment alignment);
		// Alignment of the child panels as a whole within the available
		// space. This applies only if the available space cannot be
		// filled completely, with respect to forced child tallness. The
		// default is EM_ALIGN_CENTER.

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
		//   |  l  + panel +  h  + panel +  h  + panel +  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         v             v             v         |
		//   |     +-------+     +-------+     +-------+     |
		//   |  l  + panel +  h  + panel +  h  + panel +  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         b             b             b         |
		//   +-----------------------------------------------+
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
		// Tiles all child panels in the content area (except for an
		// auxiliary panel, which is laid out in the border).

private:

	double SpaceL,SpaceT,SpaceH,SpaceV,SpaceR,SpaceB;
	double PCT;
	emArray<double> PCTPos, PCTNeg;
	int FixedColumnCount,FixedRowCount,MinCellCount,FCTColumn,FCTRow;
	emAlignment Alignment;
	bool RowByRow;

	friend class emGroup;
	emTiling(
		ParentArg parent, const emString & name,
		const emString & caption,
		const emString & description,
		const emImage & icon,
		int notWarningDeprecatedForInternalUse
	);
};

inline bool emTiling::IsRowByRow() const
{
	return RowByRow;
}

inline int emTiling::GetFixedColumnCount() const
{
	return FixedColumnCount;
}

inline int emTiling::GetFixedRowCount() const
{
	return FixedRowCount;
}

inline int emTiling::GetMinCellCount() const
{
	return MinCellCount;
}

inline int emTiling::GetForcedChildTallnessColumn() const
{
	return FCTColumn;
}

inline int emTiling::GetForcedChildTallnessRow() const
{
	return FCTRow;
}

inline emAlignment emTiling::GetAlignment() const
{
	return Alignment;
}

inline double emTiling::GetSpaceL() const
{
	return SpaceL;
}

inline double emTiling::GetSpaceT() const
{
	return SpaceT;
}

inline double emTiling::GetSpaceH() const
{
	return SpaceH;
}

inline double emTiling::GetSpaceV() const
{
	return SpaceV;
}

inline double emTiling::GetSpaceR() const
{
	return SpaceR;
}

inline double emTiling::GetSpaceB() const
{
	return SpaceB;
}


#endif
