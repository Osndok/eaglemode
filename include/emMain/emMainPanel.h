//------------------------------------------------------------------------------
// emMainPanel.h
//
// Copyright (C) 2007-2010,2016 Oliver Hamann.
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

	emView & GetControlView() const;
	emView & GetContentView() const;

	emColor GetControlEdgesColor() const;
	void SetControlEdgesColor(emColor controlEdgesColor);

	bool HasStartupOverlay() const;
	void SetStartupOverlay(bool startupOverlay);

protected:

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual bool IsOpaque() const;
	virtual void Paint(const emPainter & painter, emColor canvasColor) const;
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
		virtual emCursor GetCursor() const;
		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor) const;
		emMainPanel & MainPanel;
		emImage SliderImage;
		bool MouseOver,Pressed,Hidden;
		double PressMY,PressSliderY;
	};
	friend class SliderPanel;

	class StartupOverlayPanel : public emPanel {
	public:
		StartupOverlayPanel(ParentArg parent, const emString & name);
		virtual ~StartupOverlayPanel();
		virtual void Input(emInputEvent & event,
		                   const emInputState & state,
		                   double mx, double my);
		virtual emCursor GetCursor() const;
		virtual bool IsOpaque() const;
		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor) const;
	};

	emRef<emMainConfig> MainConfig;

	emColor ControlEdgesColor;
	emImage ControlEdgesImage;

	double ControlTallness;
	emSubViewPanel * ControlViewPanel;
	emSubViewPanel * ContentViewPanel;
	SliderPanel * Slider;
	StartupOverlayPanel * StartupOverlay;

	double UnifiedSliderPos;
	double ControlX,ControlY,ControlW,ControlH;
	double ContentX,ContentY,ContentW,ContentH;
	double SliderX,SliderY,SliderW,SliderH;
	double SliderMinY,SliderMaxY;

	bool FullscreenOn;

	double OldMouseX,OldMouseY;
	emTimer SliderTimer;
};

inline emView & emMainPanel::GetControlView() const
{
	return ControlViewPanel->GetSubView();
}

inline emView & emMainPanel::GetContentView() const
{
	return ContentViewPanel->GetSubView();
}

inline emColor emMainPanel::GetControlEdgesColor() const
{
	return ControlEdgesColor;
}

inline bool emMainPanel::HasStartupOverlay() const
{
	return StartupOverlay!=NULL;
}

#endif
