//------------------------------------------------------------------------------
// emScreen.h
//
// Copyright (C) 2005-2011,2016-2017 Oliver Hamann.
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

#ifndef emScreen_h
#define emScreen_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

class emWindow;
class emWindowPort;


//==============================================================================
//================================== emScreen ==================================
//==============================================================================

class emScreen : public emModel {

public:

	// This model class acts as an interface to the screen or desktop. It is
	// an abstract base class for an interface implementation. Such an
	// implementation should also define a public method like this:
	//
	//   static void Install(emContext & context);
	//
	// That method should find or create an instance of the interface
	// implementation within the given context, and it should call the
	// protected method Install for registering it as the interface to be
	// returned by LookupInherited. The main program or the emGUIFramework
	// implementation should call the public Install method on the root
	// context at program start.

	static emRef<emScreen> LookupInherited(emContext & context);
		// Get a reference to the screen interface.
		// Arguments:
		//   context - The context where the screen interface has been
		//             installed, or any descendant context. Typically,
		//             it should have been installed in the root
		//             context, so you can give any context here.
		// Returns:
		//   The reference to the interface, or a NULL reference if not
		//   found.

	virtual void GetDesktopRect(
		double * pX, double * pY, double * pW, double * pH
	) const = 0;
		// Get the pixel coordinates of this (virtual) desktop.
		// Arguments:
		//   pX - Pointer for returning the X-position of the rectangle.
		//   pY - Pointer for returning the Y-position of the rectangle.
		//   pW - Pointer for returning the width of the rectangle.
		//   pH - Pointer for returning the height of the rectangle.

	virtual int GetMonitorCount() const = 0;
		// Get the number of display monitors showing this (virtual)
		// desktop.

	virtual void GetMonitorRect(
		int index, double * pX, double * pY, double * pW, double * pH
	) const = 0;
		// Get the pixel coordinates of a display monitor on this
		// (virtual) desktop.
		// Arguments:
		//   index - Index of the monitor (0...GetMonitorCount()).
		//   pX    - Pointer for returning the X-position of the rectangle.
		//   pY    - Pointer for returning the Y-position of the rectangle.
		//   pW    - Pointer for returning the width of the rectangle.
		//   pH    - Pointer for returning the height of the rectangle.

	int GetMonitorIndexOfRect(double x, double y, double w, double h) const;
		// Get the index of the monitor which contains the given
		// rectangle or the biggest part of it. If the rectangle is
		// beyond all monitors, the index of the primary monitor is
		// returned (which is 0).

	virtual double GetDPI() const = 0;
		// Get pixels per inch of the primary monitor (horizontally).

	const emSignal & GetGeometrySignal() const;
		// This signal is signaled on any change in the results of
		// GetDesktopRect, GetMonitorCount, GetMonitorRect and GetDPI.

	virtual void MoveMousePointer(double dx, double dy) = 0;
		// Move the mouse pointer programmatically.
		// Arguments:
		//   dx - Amount of pixels to move the pointer in X direction.
		//   dy - Amount of pixels to move the pointer in Y direction.

	virtual void Beep() = 0;
		// Give an acoustic warning to the user, if possible.

	const emArray<emWindow*> & GetWindows() const;
		// Get an array of pointers to all windows on the screen, which
		// have been created within the context of this screen
		// interface. Other windows are not included.

	const emSignal & GetWindowsSignal() const;
		// This signal is signaled when the array returned by GetWindows
		// has changed.

	void LeaveFullscreenModes(emWindow * exceptForWindow=NULL);
		// Make sure that no window is in fullscreen mode, except for a
		// given one, optionally. This concerns only windows that have
		// been created within the context of this screen interface.

protected:

	friend class emWindow;

	emScreen(emContext & context, const emString & name);
		// See emModel.

	virtual ~emScreen();

	void Install();
		// Register this interface so that it can be found by
		// LookupInherited.

	void SignalGeometrySignal();
		// Signal a change in the results of GetDesktopRect,
		// GetMonitorCount, GetMonitorRect or GetDPI.

	virtual emWindowPort * CreateWindowPort(emWindow & window) = 0;
		// Create a window port implementation for a new window on the
		// screen. (This is called by the constructor of emWindow)

private:
	emArray<emWindow*> Windows;
	emSignal GeometrySignal;
	emSignal WindowsSignal;
};

inline const emSignal & emScreen::GetGeometrySignal() const
{
	return GeometrySignal;
}

inline const emArray<emWindow*> & emScreen::GetWindows() const
{
	return Windows;
}

inline const emSignal & emScreen::GetWindowsSignal() const
{
	return WindowsSignal;
}

inline void emScreen::SignalGeometrySignal()
{
	Signal(GeometrySignal);
}


#endif
