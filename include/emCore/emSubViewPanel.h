//------------------------------------------------------------------------------
// emSubViewPanel.h
//
// Copyright (C) 2006-2008,2010-2011 Oliver Hamann.
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

#ifndef emSubViewPanel_h
#define emSubViewPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//=============================== emSubViewPanel ===============================
//==============================================================================

class emSubViewPanel : public emPanel {

public:

	// Class for a panel which shows an emView as a sub-view. This is good
	// for splitting a view or window into multiple views. All input and
	// output of the panel is connected to the sub-view via an internal
	// emViewPort. The outer view(s) must not be zoomable.

	emSubViewPanel(ParentArg parent, const emString & name);
		// Constructor. This creates the sub-view with default features
		// and without any panel. The caller may change the features of
		// the sub-view and create a root panel in it.

	virtual ~emSubViewPanel();
		// Destructor. Deletes the sub-view and its panels.

	emView & GetSubView();
		// Get the sub-view.

	virtual emString GetTitle();

	virtual double GetTouchEventPriority(double touchX, double touchY);

protected:

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual emCursor GetCursor();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void InvalidatePaintingOnView(double x, double y, double w, double h);

	class SubViewClass : public emView {
	public:
		SubViewClass(emSubViewPanel & superPanel);
		virtual ~SubViewClass();
	protected:
		virtual void InvalidateTitle();
	private:
		emSubViewPanel & SuperPanel;
	};

	class SubViewPortClass : public emViewPort {
	public:
		SubViewPortClass(emSubViewPanel & superPanel);
		void SetViewGeometry(double x, double y, double w, double h,
		                     double pixelTallness);
		void SetViewFocused(bool focused);
		void InputToView(emInputEvent & event,
		                 const emInputState & state);
		emCursor GetViewCursor();
		void PaintView(const emPainter & painter, emColor canvasColor);
	protected:
		virtual void RequestFocus();
		virtual bool IsSoftKeyboardShown();
		virtual void ShowSoftKeyboard(bool show);
		virtual emUInt64 GetInputClockMS();
		virtual void InvalidateCursor();
		virtual void InvalidatePainting(double x, double y, double w,
		                                double h);
	private:
		emSubViewPanel & SuperPanel;
	};

	friend class SubViewClass;
	friend class SubViewPortClass;

	SubViewClass * SubView;
	SubViewPortClass * SubViewPort;
};

inline emView & emSubViewPanel::GetSubView()
{
	return *SubView;
}

inline void emSubViewPanel::InvalidatePaintingOnView(
	double x, double y, double w, double h
)
{
	GetView().InvalidatePainting(x,y,w,h);
}

inline void emSubViewPanel::SubViewPortClass::SetViewGeometry(
	double x, double y, double w, double h, double pixelTallness
)
{
	emViewPort::SetViewGeometry(x,y,w,h,pixelTallness);
}

inline void emSubViewPanel::SubViewPortClass::SetViewFocused(
	bool focused
)
{
	emViewPort::SetViewFocused(focused);
}

inline void emSubViewPanel::SubViewPortClass::InputToView(
	emInputEvent & event, const emInputState & state
)
{
	emViewPort::InputToView(event,state);
}

inline emCursor emSubViewPanel::SubViewPortClass::GetViewCursor()
{
	return emViewPort::GetViewCursor();
}

inline void emSubViewPanel::SubViewPortClass::PaintView(
	const emPainter & painter, emColor canvasColor
)
{
	emViewPort::PaintView(painter,canvasColor);
}


#endif
