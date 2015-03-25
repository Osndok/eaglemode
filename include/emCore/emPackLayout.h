//------------------------------------------------------------------------------
// emPackLayout.h
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

#ifndef emPackLayout_h
#define emPackLayout_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================ emPackLayout ================================
//==============================================================================

class emPackLayout : public emBorder {

public:

	// A panel of this class automatically lays out its child panels within
	// the content area by a special "pack" algorithm, which allows to give
	// each child panel a weight (area proportion) and a preferred tallness
	// (height/width ratio). This should be used only for up to about seven
	// child panels. With more, the algorithm may not find an optimal
	// layout.
	//
	//                    Pack layout
	//   +---------------------------------------------+
	//   | +------------------+  +---------+  +-----+  |
	//   | |                  |  |         |  |     |  |
	//   | |                  |  |         |  |     |  |
	//   | |                  |  |         |  |     |  |
	//   | |                  |  |         |  +-----+  |
	//   | |                  |  |         |           |
	//   | |                  |  |         |  +-----+  |
	//   | |                  |  |         |  |     |  |
	//   | +------------------+  |         |  |     |  |
	//   |                       |         |  |     |  |
	//   | +------------------+  |         |  |     |  |
	//   | |                  |  |         |  |     |  |
	//   | |                  |  |         |  |     |  |
	//   | +------------------+  +---------+  +-----+  |
	//   +---------------------------------------------+
	//
	// The algorithm recursively divides the available area and the child
	// panel set into two areas and two child panel sets until each area has
	// only one panel. Thereby, the order of child panels is kept. The area
	// proportions are calculated from the given weights. The decision where
	// (at which child panel) and how (horizontally or vertically) the
	// dividing in each recursive step is made, is determined by iterating
	// multiple possibilities until a solution is found which satisfies the
	// preferred tallnesses best possible. Because this would be too time
	// consuming for large sets, the algorithm reduces the number of
	// iterations in a recursive step depending on the size of the set (down
	// to zero for large sets). This way, an optimum is guaranteed only for
	// up to seven child panels.
	//
	// By default, a panel of this class is not focusable and has no border,
	// because it is meant as a pure layout programming tool. For other use,
	// please see the derived class emPackGroup.

	emPackLayout(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emBorder, but sets non-focusable.

	virtual ~emPackLayout();
		// Destructor.

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

	double GetPrefChildTallness(int index) const;
	void SetPrefChildTallness(int index, double pct);
		// Get or set the preferred tallness (height/width ratio) of a
		// child panel. The index argument denotes a child panel. Zero
		// means first child, one means second, and so on. The default
		// preferred tallness is 0.2.

	void SetPrefChildTallness(double pct);
		// Set the preferred tallness of all child panels to the given
		// value.

protected:

	virtual void LayoutChildren();
		// Lays out all child panels in the content area (except for an
		// auxiliary panel, which is laid out in the border).

private:

	struct TmpPanelInfo {
		double PCT;
		double CumulativeWeight;
		double CumulativeLogPCT;
		emPanel * Panel;
	};

	struct TmpInfo {
		TmpPanelInfo * TPIs;
		emColor CanvasColor;
	};

	int CountCells();
	void FillTPIs(int count);
	double GetTPIWeightSum(int index, int count) const;
	double GetTPILogPCTSum(int index, int count) const;

	double RateCell(int index, double w, double h);

	double Pack1(
		int index,
		double x, double y, double w, double h,
		bool execute
	);

	double Pack2(
		int index,
		double x, double y, double w, double h,
		double bestError, bool execute
	);

	double Pack3(
		int index,
		double x, double y, double w, double h,
		double bestError, bool execute
	);

	double PackN(
		int index, int count,
		double x, double y, double w, double h,
		double bestError, bool execute
	);

	double RateHorizontally(
		int index, int count, int div,
		double x, double y, double w1, double w2, double h,
		double bestError
	);

	double RateVertically(
		int index, int count, int div,
		double x, double y, double w, double h1, double h2,
		double bestError
	);

	double DefaultWeight;
	double DefaultPCT;
	emArray<double> WeightArray;
	emArray<double> PCTArray;
	int MinCellCount;
	TmpInfo * TI;
	int Ratings;
};


inline int emPackLayout::GetMinCellCount() const
{
	return MinCellCount;
}


#endif
