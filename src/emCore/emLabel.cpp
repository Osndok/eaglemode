//------------------------------------------------------------------------------
// emLabel.cpp
//
// Copyright (C) 2005-2011,2014,2016 Oliver Hamann.
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

#include <emCore/emLabel.h>


emLabel::emLabel(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emBorder(parent,name,caption,description,icon)
{
	SetOuterBorderType(OBT_MARGIN);
	SetLabelInBorder(false);
	SetFocusable(false);
}


void emLabel::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	PaintLabel(
		painter,
		x,y,w,h,
		IsEnabled() ?
			GetLook().GetFgColor()
		:
			GetLook().GetFgColor().GetTransparented(75.0F),
		canvasColor
	);
}
