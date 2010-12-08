//------------------------------------------------------------------------------
// emMainPanel.h
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#ifndef emMainPanel_h
#define emMainPanel_h

#ifndef emSubViewPanel_h
#include <emCore/emSubViewPanel.h>
#endif

#ifndef emMainConfig_h
#include <emMain/emMainConfig.h>
#endif


class emMainPanel : public emPanel {

public:

	emMainPanel(ParentArg parent, const emString & name,
	            double controlTallness);

	virtual ~emMainPanel();

	emView & GetControlView();
	emView & GetContentView();

	emColor GetControlEdgesColor() const;
	void SetControlEdgesColor(emColor controlEdgesColor);

protected:

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual void LayoutChildren();

private:

	void UpdateCoordinates();
	void UpdateFullscreen();
	void UpdateSliderHiding(bool restart);
	void DragSlider(double deltaY);
	void DoubleClickSlider();

	class SliderPanel : public emPanel {
	public:
		SliderPanel(emMainPanel & parent, const emString & name);
		virtual ~SliderPanel();
		void SetHidden(bool hidden);
		virtual void Input(emInputEvent & event,
		                   const emInputState & state,
		                   double mx, double my);
		virtual emCursor GetCursor();
		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor);
		emMainPanel & MainPanel;
		emImage SliderImage;
		bool MouseOver,Pressed,Hidden;
		double PressMY,PressSliderY;
	};
	friend class SliderPanel;

	emRef<emMainConfig> MainConfig;

	emColor ControlEdgesColor;
	emImage ControlEdgesImage;

	double ControlTallness;
	emSubViewPanel * ControlViewPanel;
	emSubViewPanel * ContentViewPanel;
	SliderPanel * Slider;

	double UnifiedSliderPos;
	double ControlX,ControlY,ControlW,ControlH;
	double ContentX,ContentY,ContentW,ContentH;
	double SliderX,SliderY,SliderW,SliderH;
	double SliderMinY,SliderMaxY;

	bool FullscreenOn;

	double OldMouseX,OldMouseY;
	emTimer SliderTimer;
};

inline emView & emMainPanel::GetControlView()
{
	return ControlViewPanel->GetSubView();
}

inline emView & emMainPanel::GetContentView()
{
	return ContentViewPanel->GetSubView();
}

inline emColor emMainPanel::GetControlEdgesColor() const
{
	return ControlEdgesColor;
}


#endif
