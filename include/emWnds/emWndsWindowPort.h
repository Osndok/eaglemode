//------------------------------------------------------------------------------
// emWndsWindowPort.h
//
// Copyright (C) 2006-2008,2010-2011,2015-2017 Oliver Hamann.
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

#ifndef emWndsWindowPort_h
#define emWndsWindowPort_h

#ifndef emClipRects_h
#include <emCore/emClipRects.h>
#endif

#ifndef emWndsScreen_h
#include <emWnds/emWndsScreen.h>
#endif


class emWndsWindowPort : public emWindowPort, private emEngine {

protected:

	virtual void WindowFlagsChanged();
	virtual void SetPosSize(
		double x, double y, PosSizeArgSpec posSpec,
		double w, double h, PosSizeArgSpec sizeSpec
	);
	virtual void GetBorderSizes(
		double * pL, double * pT, double * pR, double * pB
	) const;
	virtual void RequestFocus();
	virtual void Raise();
	virtual void InhibitScreensaver();
	virtual void AllowScreensaver();
	virtual emUInt64 GetInputClockMS() const;
	virtual void InvalidateTitle();
	virtual void InvalidateIcon();
	virtual void InvalidateCursor();
	virtual void InvalidatePainting(double x, double y,
	                                double w, double h);

private:

	friend class emWndsScreen;

	emWndsWindowPort(emWindow & window);
	virtual ~emWndsWindowPort();

	void PreConstruct();
	void PostConstruct();

	LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam,
	                   bool goodCall);

	bool FlushInputState();

	void RestoreCursor();

	virtual bool Cycle();

	void UpdatePainting();

	bool IsScreensaverReallyToInhibit();

	bool IsAncestorOf(emWndsWindowPort * wp);

	void SetModalState(bool modalState);

	void SetIconProperty(const emImage & icon);

	static HICON Image2Icon(const emImage & image, int width, int height);

	emWndsScreen & Screen;
	emWndsWindowPort * Owner;
	HWND HWnd;
	emWindow::WindowFlags WindowFlags;
	int MinPaneW,MinPaneH;
	int PaneX,PaneY,PaneW,PaneH;
	int RestoreX,RestoreY,RestoreW,RestoreH;
	double ClipX1,ClipY1,ClipX2,ClipY2;
	emString Title;
	emCursor Cursor;
	bool CursorShown;
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
	emClipRects<int> InvalidRects;
	emUInt64 InputStateClock;
	emInputKey LastButtonPress;
	emUInt64 LastButtonPressTime;
	double LastButtonPressX;
	double LastButtonPressY;
	int LastButtonPressRepeat;
	emInputKey RepeatKey;
	int KeyRepeat;
	int MouseWheelRemainder;
	int MouseHWheelRemainder;
	bool ModalState;
	int ModalDescendants;
	HICON BigIcon;
	HICON SmallIcon;
	int ScreensaverInhibitCount;
};


#endif
