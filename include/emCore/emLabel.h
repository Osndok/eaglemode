//------------------------------------------------------------------------------
// emLabel.h
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

#ifndef emLabel_h
#define emLabel_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================== emLabel ===================================
//==============================================================================

class emLabel : public emBorder {

public:

	// A panel of this class simply shows the label as the content, and it
	// is not focusable by default.

	emLabel(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emBorder, but it performs:
		//  SetOuterBorderType(OBT_MARGIN);
		//  SetLabelInBorder(false);
		//  SetFocusable(false);

protected:

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;
		// Paints the label.
};


#endif
