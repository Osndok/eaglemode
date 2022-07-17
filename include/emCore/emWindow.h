//------------------------------------------------------------------------------
// emWindow.h
//
// Copyright (C) 2005-2010,2016-2018,2022 Oliver Hamann.
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

#ifndef emWindow_h
#define emWindow_h

#ifndef emScreen_h
#include <emCore/emScreen.h>
#endif

#ifndef emView_h
#include <emCore/emView.h>
#endif

class emWindowPort;


//==============================================================================
//================================== emWindow ==================================
//==============================================================================

class emWindow : public emView {

public:

	// An emWindow is an emView which is shown as a window on a screen. See
	// also class emScreen. Such a screen interface has to be installed in a
	// context before windows can be constructed. This should be done at
	// program start, maybe with the help of an emGUIFramework.

	typedef int WindowFlags;
		// Data type for the feature flags of a window. Possible flags
		// are:
	enum {
		WF_MODAL       = (1<<0),
			// Ancestor windows of same screen cannot get input
			// events or close signal. This flag has no effect if
			// the are no such ancestors.
		WF_UNDECORATED = (1<<1),
			// The window has no border and not title bar.
		WF_POPUP       = (1<<2),
			// Like WF_UNDECORATED, but the close signal is
			// generated as soon as the focus is lost.
		WF_MAXIMIZED   = (1<<3),
			// The window covers the whole work area of the desktop.
			// This flag may have no effect when combined with
			// WF_UNDECORATED, WF_POPUP or WF_FULLSCREEN.
		WF_FULLSCREEN  = (1<<4),
			// The window covers the whole screen. This flag may
			// have no effect when combined with WF_UNDECORATED or
			// WF_POPUP.
		WF_AUTO_DELETE = (1<<5)
			// The window deletes itself automatically three time
			// slices after the close signal is signaled.
	};

	emWindow(
		emContext & parentContext,
		ViewFlags viewFlags=0,
		WindowFlags windowFlags=0,
		const emString & wmResName="emWindow"
	);
		// Construct a window. If the parent context or a higher
		// ancestor context is a window (without another emScreen in
		// between), this new window will be a slave of that ancestor
		// window ("Transient Window" in X11 speaking, "Owned Window" in
		// Windows speaking). The new window is not shown before the end
		// of the current time slice (by typical emWindowPort
		// derivatives). This gives a chance to prepare things like the
		// size and position of the window without any flicker on the
		// screen. If the position or size is not set before the window
		// is shown, a default is invented at the moment of showing the
		// window (the default depends on the emWindowPort derivative,
		// which may even forward the job to the window manager).
		// Arguments:
		//   parentContext - Parent context for this new context.
		//   viewFlags     - Initial view features.
		//   windowFlags   - Initial window features.
		//   wmResName     - A resource name for the window. This is
		//                   reported to the window manager.

	virtual ~emWindow();
		// Destructor.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// This means emCrossPtr<emWindow> is possible.

	emScreen & GetScreen() const;
		// Get the screen where this window is shown.

	WindowFlags GetWindowFlags() const;
	void SetWindowFlags(WindowFlags windowFlags);
		// Get or set the features of this window.
		// CAUTION: Changing WF_MODAL, WF_UNDECORATED or WF_POPUP may
		// reset position and size of the window. Any call to
		// SetViewPosSize(..), SetWinPosSize(..) or companions should be
		// made AFTER setting the window flags.

	const emSignal & GetWindowFlagsSignal() const;
		// This signal is signaled when the features of this window have
		// changed.

	const emString & GetWMResName() const;
		// Get the resource name of this window.

	emWindowPort & GetWindowPort() const;
		// Get the window port of this window.

	const emSignal & GetCloseSignal() const;
		// Get the close signal. It is signaled when the window should
		// be deleted.

	void SignalClosing();
		// Signal the close signal.

	void SetViewPos(double x, double y);
	void SetViewSize(double w, double h);
	void SetViewPosSize(double x, double y, double w, double h);
		// Set position and size of this window by the coordinates of
		// the view port. This sets the coordinates you can get with
		// GetHomeX, GetHomeY, GetHomeWidth and GetHomeHeight from the
		// base class. On X11, the position may not be exact.

	void SetWinPos(double x, double y);
	void SetWinSize(double w, double h);
	void SetWinPosSize(double x, double y, double w, double h);
		// Set position and size of this window by the coordinates of
		// the whole window rectangle including the borders. On X11, the
		// size may not be exact.

	void SetWinPosViewSize(double x, double y, double w, double h);
		// Like SetWinPos and SetViewSize.

	bool SetWinPosViewSize(const char * geometry);
		// Like SetWinPosViewSize, but parse a typical X11 geometry
		// option string (e.g. "800x600+100-100"). As expected, the
		// position or size is not set if it is not specified (e.g.
		// "320x200" sets the size only). On parse error, false is
		// returned.

	void GetBorderSizes(double * pL, double * pT, double * pR,
	                    double * pB) const;
		// Get the best known size (thickness) of the left, top, right
		// and bottom edges of the window border.

	int GetMonitorIndex() const;
		// Get the index of the display monitor on which this window is.

	const emImage & GetWindowIcon() const;
	void SetWindowIcon(const emImage & windowIcon);
		// Get or set the icon to be shown for this window. The default
		// is to inherit the icon from an ancestor window at
		// construction. But if there is no ancestor window, the default
		// is an empty image which means to have no specific icon.

	void Raise();
		// Bring this window to the top of the stacking order. The
		// window is even restored from any iconified state.

	void InhibitScreensaver();
	void AllowScreensaver();
		// Inhibit or re-allow the screensaver. This should implement an
		// internal counter for calls to InhibitScreensaver() which have
		// not yet been taken back by calls to AllowScreensaver().
		// Inhibiting only works while the window is shown quite large
		// and not iconified.

protected:

	virtual void InvalidateTitle();
		// The title of the view is taken for the title of the window.
		// This method has been overloaded just to update the window
		// title.

private:

	friend class emWindowPort;

	class AutoDeleteEngineClass : public emEngine {
	public:
		AutoDeleteEngineClass(emWindow * window);
	protected:
		virtual bool Cycle();
	private:
		emWindow * Window;
		int CountDown;
	};

	emRef<emScreen> Screen;
	emCrossPtrList CrossPtrList;
	WindowFlags WFlags;
	emString WMResName;
	emImage WindowIcon;
	emWindowPort * WindowPort;
	emSignal WindowFlagsSignal;
	emSignal CloseSignal;
	AutoDeleteEngineClass AutoDeleteEngine;
};


//==============================================================================
//================================ emWindowPort ================================
//==============================================================================

class emWindowPort : public emViewPort {

public:

	// Abstract base class for the connection between an emWindow and the
	// operating system or the hardware or whatever. When an emWindow is
	// constructed, its constructor creates an emWindowPort by calling
	// CreateWindowPort on the emScreen.

	emWindowPort(emWindow & window);
		// Only to be called through overloaded versions of
		// emScreen::CreateWindowPort.

	virtual ~emWindowPort();

	emWindow & GetWindow() const;

protected:

	emWindow::WindowFlags GetWindowFlags() const;

	const emString & GetWMResName() const;

	emString GetWindowTitle() const;

	const emImage & GetWindowIcon() const;

	virtual void WindowFlagsChanged() = 0;

	enum PosSizeArgSpec {
		PSAS_IGNORE,
		PSAS_VIEW,
		PSAS_WINDOW
	};
	virtual void SetPosSize(
		double x, double y, PosSizeArgSpec posSpec,
		double w, double h, PosSizeArgSpec sizeSpec
	) = 0;
		// Should call SetViewGeometry(...) immediately with appropriate
		// values.

	virtual void GetBorderSizes(
		double * pL, double * pT, double * pR, double * pB
	) const = 0;

	virtual void Raise() = 0;

	virtual void InhibitScreensaver() = 0;
	virtual void AllowScreensaver() = 0;

	virtual void InvalidateTitle() = 0;

	virtual void InvalidateIcon() = 0;

	void SignalWindowClosing();

private:
	friend class emWindow;
	emWindow & Window;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//---------------------------------- emWindow ----------------------------------

inline void emWindow::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline emScreen & emWindow::GetScreen() const
{
	return *(Screen.Get());
}

inline emWindow::WindowFlags emWindow::GetWindowFlags() const
{
	return WFlags;
}

inline const emString & emWindow::GetWMResName() const
{
	return WMResName;
}

inline emWindowPort & emWindow::GetWindowPort() const
{
	return *WindowPort;
}

inline const emSignal & emWindow::GetCloseSignal() const
{
	return CloseSignal;
}

inline void emWindow::SignalClosing()
{
	Signal(CloseSignal);
}

inline void emWindow::SetViewPos(double x, double y)
{
	WindowPort->SetPosSize(
		x,y,emWindowPort::PSAS_VIEW,
		0,0,emWindowPort::PSAS_IGNORE
	);
}

inline void emWindow::SetViewSize(double w, double h)
{
	WindowPort->SetPosSize(
		0,0,emWindowPort::PSAS_IGNORE,
		w,h,emWindowPort::PSAS_VIEW
	);
}

inline void emWindow::SetViewPosSize(double x, double y, double w, double h)
{
	WindowPort->SetPosSize(
		x,y,emWindowPort::PSAS_VIEW,
		w,h,emWindowPort::PSAS_VIEW
	);
}

inline void emWindow::SetWinPos(double x, double y)
{
	WindowPort->SetPosSize(
		x,y,emWindowPort::PSAS_WINDOW,
		0,0,emWindowPort::PSAS_IGNORE
	);
}

inline void emWindow::SetWinSize(double w, double h)
{
	WindowPort->SetPosSize(
		0,0,emWindowPort::PSAS_IGNORE,
		w,h,emWindowPort::PSAS_WINDOW
	);
}

inline void emWindow::SetWinPosSize(double x, double y, double w, double h)
{
	WindowPort->SetPosSize(
		x,y,emWindowPort::PSAS_WINDOW,
		w,h,emWindowPort::PSAS_WINDOW
	);
}

inline void emWindow::SetWinPosViewSize(double x, double y, double w, double h)
{
	WindowPort->SetPosSize(
		x,y,emWindowPort::PSAS_WINDOW,
		w,h,emWindowPort::PSAS_VIEW
	);
}

inline void emWindow::GetBorderSizes(
	double * pL, double * pT, double * pR, double * pB
) const
{
	WindowPort->GetBorderSizes(pL,pT,pR,pB);
}

inline const emImage & emWindow::GetWindowIcon() const
{
	return WindowIcon;
}

inline void emWindow::Raise()
{
	WindowPort->Raise();
}

inline void emWindow::InhibitScreensaver()
{
	WindowPort->InhibitScreensaver();
}

inline void emWindow::AllowScreensaver()
{
	WindowPort->AllowScreensaver();
}


//-------------------------------- emWindowPort --------------------------------

inline emWindow & emWindowPort::GetWindow() const
{
	return Window;
}

inline emWindow::WindowFlags emWindowPort::GetWindowFlags() const
{
	return Window.GetWindowFlags();
}

inline const emSignal & emWindow::GetWindowFlagsSignal() const
{
	return WindowFlagsSignal;
}

inline const emString & emWindowPort::GetWMResName() const
{
	return Window.GetWMResName();
}

inline emString emWindowPort::GetWindowTitle() const
{
	return Window.GetTitle();
}

inline const emImage & emWindowPort::GetWindowIcon() const
{
	return Window.GetWindowIcon();
}

inline void emWindowPort::SignalWindowClosing()
{
	Window.SignalClosing();
}


#endif
