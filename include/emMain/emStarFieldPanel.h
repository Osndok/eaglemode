//------------------------------------------------------------------------------
// emStarFieldPanel.h
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

#ifndef emStarFieldPanel_h
#define emStarFieldPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


class emStarFieldPanel : public emPanel {

public:

	emStarFieldPanel(ParentArg parent, const emString & name,
	                 int depth=0, emUInt32 randomSeed=0);

	virtual ~emStarFieldPanel();

	virtual emString GetTitle();

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void PaintOverlay(const emPainter & painter);
	void UpdateChildren();
	emUInt32 GetRandom();
	double GetRandom(double minVal, double maxVal);

	class OverlayPanel : public emPanel {
	public:
		OverlayPanel(emStarFieldPanel * parent, const emString & name);
	protected:
		virtual void Input(emInputEvent & event,
		                   const emInputState & state,
		                   double mx, double my);
		virtual bool IsOpaque();
		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor);
	};
	friend class OverlayPanel;

	class TicTacToePanel : public emPanel {
	public:
		TicTacToePanel(emStarFieldPanel * parent,
		               const emString & name);
		virtual emString GetTitle();
	protected:
		virtual void Input(emInputEvent & event,
		                   const emInputState & state,
		                   double mx, double my);
		virtual bool IsOpaque();
		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor);
	private:
		static int DeepCheckState(int state, int turn);
		static int CheckState(int state);
			// -1 = not ended, 0 = draw, 1 = human won,
			// 2 = computer won
		emUInt32 GetRandom();
		int State;
			// s=(State>>((x+y*3)*2))&3
			// s: 0 = empty, 1 = human, 2 = computer
		int Starter;
		emUInt32 RandomSeed;
	};

	struct Star {
		double X,Y,Radius;
		emColor Color;
	};

	int Depth;
	emUInt32 RandomSeed;
	emUInt32 ChildRandomSeed[4];
	int StarCount;
	Star * Stars;
	emImage StarShape;

	static const emColor BgColor;
	static const double MinPanelSize;
	static const double MinStarRadius;
};


#endif
