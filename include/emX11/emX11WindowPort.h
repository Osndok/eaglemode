//------------------------------------------------------------------------------
// emX11WindowPort.h
//
// Copyright (C) 2005-2012 Oliver Hamann.
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

#ifndef emX11WindowPort_h
#define emX11WindowPort_h

#ifndef emClipRects_h
#include <emCore/emClipRects.h>
#endif

#ifndef emX11Screen_h
#include <emX11/emX11Screen.h>
#endif


class emX11WindowPort : public emWindowPort, private emEngine {

protected:

	virtual void WindowFlagsChanged();
	virtual void SetPosSize(
		double x, double y, PosSizeArgSpec posSpec,
		double w, double h, PosSizeArgSpec sizeSpec
	);
	virtual void GetBorderSizes(
		double * pL, double * pT, double * pR, double * pB
	);
	virtual void RequestFocus();
	virtual void Raise();
	virtual emUInt64 GetInputClockMS();
	virtual void InvalidateTitle();
	virtual void InvalidateIcon();
	virtual void InvalidateCursor();
	virtual void InvalidatePainting(double x, double y,
	                                double w, double h);

private:

	friend class emX11Screen;

	emX11WindowPort(emWindow & window);
	virtual ~emX11WindowPort();

	void PreConstruct();
	void PostConstruct();

	void HandleEvent(XEvent & event);

	bool FlushInputState();

	virtual bool Cycle();

	void UpdatePainting();

	bool MakeViewable();

	emX11WindowPort * SearchOwnedPopupAt(double x, double y);

	bool IsAncestorOf(emX11WindowPort * wp);

	void SetModalState(bool modalState);
	void FocusModalDescendant(bool flash=false);

	void Flash();

	void SetIconProperty(const emImage & icon);

	void SendLaunchFeedback();

	void RestoreCursor();

	void GetAbsWinGeometry(Display * disp, ::Window win,
	                       int * pX, int *pY, int * pW, int *pH);

	emX11Screen & Screen;
	emThreadMiniMutex & XMutex;
	Display * Disp;
	emX11WindowPort * Owner;
	::Window Win;
	XIC InputContext;
	GC Gc;
	int MinPaneW,MinPaneH;
	int PaneX,PaneY,PaneW,PaneH;
	int BorderL,BorderT,BorderR,BorderB;
	double ClipX1,ClipY1,ClipX2,ClipY2;
	emString Title;
	emCursor Cursor;
	bool PostConstructed;
	bool Mapped;
	bool Focused;
	bool PosForced;
	bool PosPending;
	bool SizeForced;
	bool SizePending;
	bool TitlePending;
	bool IconPending;
	bool CursorPending;
	bool LaunchFeedbackSent;
	emClipRects<int> InvalidRects;
	emUInt64 InputStateClock;
	emInputKey LastButtonPress;
	Time LastButtonPressTime;
	int LastButtonPressX;
	int LastButtonPressY;
	int LastButtonPressRepeat;
	emInputKey RepeatKey;
	int KeyRepeat;
	XComposeStatus ComposeStatus;
	emTimer * FullscreenUpdateTimer;
	bool ModalState;
	int ModalDescendants;
};


#endif
