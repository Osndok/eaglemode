//------------------------------------------------------------------------------
// emMainContentPanel.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef emMainContentPanel_h
#define emMainContentPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


class emMainContentPanel : public emPanel {

public:

	emMainContentPanel(ParentArg parent, const emString & name);

	virtual ~emMainContentPanel();

	virtual emString GetTitle();

	virtual void Layout(double layoutX, double layoutY,
	                    double layoutWidth, double layoutHeight,
	                    emColor canvasColor=0);

protected:

	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void UpdateCoordinates();
	void UpdateChildLayout();
	void PaintEagle(const emPainter & painter);

	emImage SkyImage;
	double EagleShiftX,EagleShiftY,EagleScaleX,EagleScaleY;
};


#endif
