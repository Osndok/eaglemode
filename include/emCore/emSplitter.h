//------------------------------------------------------------------------------
// emSplitter.h
//
// Copyright (C) 2005-2010,2014,2016 Oliver Hamann.
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

#ifndef emSplitter_h
#define emSplitter_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================= emSplitter =================================
//==============================================================================

class emSplitter : public emBorder {

public:

	// Class for a splitter panel. Such a panel can get two child panels
	// which are laid out automatically, either horizontally or vertically,
	// filling the whole content area. Between the two child panels is a
	// grip which can be dragged by the user, for making one of the panels
	// larger while the other gets smaller.

	emSplitter(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		bool vertical=false, double minPos=0.0, double maxPos=1.0,
		double pos=0.5
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   vertical    - See SetVertical.
		//   minPos      - See SetMinMaxPos.
		//   maxPos      - See SetMinMaxPos.
		//   pos         - See SetPos.

	virtual ~emSplitter();
		// Destructor.

	bool IsVertical() const;
	void SetVertical(bool vertical=true);
		// Whether the child panels are laid out left-right (false) or
		// on top of each other (true).

	double GetMinPos() const;
	double GetMaxPos() const;
	double GetPos() const;
	void SetMinMaxPos(double minPos, double maxPos);
	void SetPos(double pos);
		// Get/set minimum, maximum and current position of the grip.
		// The position ranges from 0.0 to 1.0 (0.0 = first child panel
		// collapsed, 1.0 = second child panel collapsed).

	const emSignal & GetPosSignal() const;
		// This signal is signaled after each change of the grip
		// position.

protected:

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual emCursor GetCursor() const;
	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;
	virtual void LayoutChildren();

private:

	void CalcGripRect(
		double contentX, double contentY, double contentW,
		double contentH, double * pX, double * pY, double * pW,
		double * pH
	) const;

	bool Vertical;
	double MinPos;
	double MaxPos;
	double Pos;
	emSignal PosSignal;
	bool Pressed;
	double MousePosInGrip;
	bool MouseInGrip;
};

inline bool emSplitter::IsVertical() const
{
	return Vertical;
}

inline double emSplitter::GetMinPos() const
{
	return MinPos;
}

inline double emSplitter::GetMaxPos() const
{
	return MaxPos;
}

inline double emSplitter::GetPos() const
{
	return Pos;
}

inline const emSignal & emSplitter::GetPosSignal() const
{
	return PosSignal;
}


#endif
