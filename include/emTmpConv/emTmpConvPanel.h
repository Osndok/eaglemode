//------------------------------------------------------------------------------
// emTmpConvPanel.h
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

#ifndef emTmpConvPanel_h
#define emTmpConvPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emTmpConvModelClient_h
#include <emTmpConv/emTmpConvModelClient.h>
#endif


class emTmpConvPanel : public emPanel {

public:

	emTmpConvPanel(
		ParentArg parent, const emString & name, emTmpConvModel * model,
		double minViewPercentForTriggering=50.0,
		double minViewPercentForHolding=25.0
	);
	virtual ~emTmpConvPanel();

	emTmpConvModel * GetModel();

	virtual emString GetTitle();

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

	virtual bool IsHopeForSeeking();

private:

	emTmpConvModel::ConversionState GetVirtualConversionState();
	void UpdateModelClientAndChildPanel();

	emTmpConvModelClient ModelClient;
	emPanel * ChildPanel;
	double MinViewPercentForTriggering;
	double MinViewPercentForHolding;
};

inline emTmpConvModel * emTmpConvPanel::GetModel()
{
	return (emTmpConvModel*)ModelClient.GetModel();
}


#endif
