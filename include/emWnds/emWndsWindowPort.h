//------------------------------------------------------------------------------
// emWndsWindowPort.h
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

#ifndef emWndsWindowPort_h
#define emWndsWindowPort_h

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
	);
	virtual void RequestFocus();
	virtual void Raise();
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

	virtual bool Cycle();

	void UpdatePainting();
	void ClearInvRectList();
	void MergeToInvRectList(int x1, int y1, int x2, int y2);

	bool IsAncestorOf(emWndsWindowPort * wp);

	void SetModalState(bool modalState);

	void SetIconProperty(const emImage & icon);

	static HICON Image2Icon(const emImage & image, int width, int height);

	struct InvRect {
		InvRect * Next;
		int x1,y1,x2,y2;
	};

	emWndsScreen & Screen;
	emWndsWindowPort * Owner;
	HWND HWnd;
	int MinPaneW,MinPaneH;
	int PaneX,PaneY,PaneW,PaneH;
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
	InvRect InvRectHeap[100];
	InvRect * InvRectFreeList;
	InvRect * InvRectList;
	emUInt64 InputStateClock;
	emInputKey LastButtonPress;
	emUInt64 LastButtonPressTime;
	double LastButtonPressX;
	double LastButtonPressY;
	int LastButtonPressRepeat;
	emInputKey RepeatKey;
	int KeyRepeat;
	int MouseWheelRemainder;
	bool ModalState;
	int ModalDescendants;
	HICON BigIcon;
	HICON SmallIcon;
};


#endif
