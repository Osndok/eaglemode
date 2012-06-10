//------------------------------------------------------------------------------
// emX11Screen.h
//
// Copyright (C) 2005-2011 Oliver Hamann.
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

#ifndef emX11Screen_h
#define emX11Screen_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef _XUTIL_H_
#include <X11/Xutil.h>
#endif

#ifndef emX11ExtDynamic_h
#include <emX11/emX11ExtDynamic.h>
#endif

#ifndef emWindow_h
#include <emCore/emWindow.h>
#endif

#ifndef emThread_h
#include <emCore/emThread.h>
#endif

class emX11WindowPort;
class emX11Clipboard;


class emX11Screen : public emScreen {

public:

	static void Install(emContext & context);

	virtual double GetWidth();
	virtual double GetHeight();

	virtual void GetVisibleRect(double * pX, double * pY,
	                            double * pW, double * pH);

	virtual double GetDPI();

	virtual void MoveMousePointer(double dx, double dy);

	virtual void Beep();

	virtual void DisableScreensaver();
	virtual void EnableScreensaver();

protected:

	virtual emWindowPort * CreateWindowPort(emWindow & window);

private:

	friend class emX11WindowPort;
	friend class emX11Clipboard;

	emX11Screen(emContext & context, const emString & name);
	virtual ~emX11Screen();

	struct CursorMapElement {
		int CursorId;
		::Cursor XCursor;
	};

	virtual bool Cycle();

	void UpdateKeymapAndInputState();
	void UpdateInputStateFromKeymap();

	void UpdateLastKnownTime(const XEvent & event);

	void WaitBufs();

	static Bool WaitPredicate(Display * display, XEvent * event, XPointer arg);

	static int CompareCurMapElemAgainstKey(
		const CursorMapElement * obj, void * key, void * context
	);

	::Cursor GetXCursor(int cursorId);

	::Cursor CreateXCursor(
		int srcWidth, int srcHeight, const unsigned char * srcBits,
		int mskWidth, int mskHeight, const unsigned char * mskBits,
		int hotX, int hotY
	);

	static emInputKey ConvertKey(KeySym ks, int * pVariant=NULL);

	static int ErrorHandler(Display * display, XErrorEvent * event);

	class WaitCursorThread : private emThread
	{
	public:
		WaitCursorThread(emThreadMiniMutex & xMutex, Display * disp);
		~WaitCursorThread();
		void AddWindow(::Window win);
		void RemoveWindow(::Window win);
		void SignOfLife();
		bool CursorToRestore();
	private:
		virtual int Run(void * arg);
		emThreadMiniMutex & XMutex;
		emThreadMiniMutex DataMutex;
		Display * Disp;
		emThreadEvent QuitEvent;
		emArray<Window> Windows;
		emUInt64 Clock;
		bool CursorChanged;
	};

	emThreadMiniMutex XMutex; // (XInitThreads was too buggy for me...)
	Display * Disp;
	WaitCursorThread * WCThread;
	XIM       InputMethod;
	int       Scrn;
	int       Width,Height;
	double    DPI;
	double    PixelTallness;
	Window    RootWin;
	Visual *  Visu;
	int       VisuDepth;
	Colormap  Colmap;
	Atom      WM_PROTOCOLS;
	Atom      WM_DELETE_WINDOW;
	Atom      _NET_WM_ICON;
	bool      HaveXF86VidMode;
	bool      UsingXShm;
	int       ShmCompletionEventType;
	int       BufWidth,BufHeight;
	XImage *  BufImg[2];
	XShmSegmentInfo BufSeg[2];
	bool      BufActive[2];
	bool      BufSegAutoRemoved;
	emPainter BufPainter[2];
	emArray<CursorMapElement> CursorMap;
	emInputState InputState;
	emUInt64 InputStateClock;
	char      Keymap[32];
	double    MouseWarpX,MouseWarpY;
	Time      LastKnownTime;
	emArray<emX11WindowPort*> WinPorts;
	emX11WindowPort * GrabbingWinPort;
	emX11Clipboard * Clipboard;
	int       ScreensaverDisableCounter;
	emTimer   ScreensaverDisableTimer;

	static emThreadMiniMutex ErrorHandlerMutex;
	static bool ErrorHandlerCalled;
};


#endif
