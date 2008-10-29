//------------------------------------------------------------------------------
// emTmpConvFramePanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emTmpConvFramePanel_h
#define emTmpConvFramePanel_h

#ifndef emTmpConvPanel_h
#include <emTmpConv/emTmpConvPanel.h>
#endif


class emTmpConvFramePanel : public emPanel {

public:

	emTmpConvFramePanel(ParentArg parent, const emString & name,
	                    emTmpConvModel * model);

	virtual ~emTmpConvFramePanel();

protected:

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void PaintInfo(
		const emPainter & painter,
		double x, double y, double w, double h,
		emColor canvasColor
	);

	double InnerScale;
	emColor BGColor;
	emTmpConvPanel * InnerPanel;
};


#endif
