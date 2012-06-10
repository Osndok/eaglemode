//------------------------------------------------------------------------------
// emScreen.h
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

	virtual double GetWidth() = 0;
	virtual double GetHeight() = 0;
		// Get the resolution of the screen (or virtual desktop) in
		// pixels.

	virtual void GetVisibleRect(double * pX, double * pY,
	                            double * pW, double * pH) = 0;
		// Get the pixel coordinates of the visible rectangle of the
		// screen. If it is not a virtual desktop, the result is equal
		// to: 0.0, 0.0, GetWidth(), GetHeight()
		// Arguments:
		//   pX - Pointer for returning the X-position of the rectangle.
		//   pY - Pointer for returning the Y-position of the rectangle.
		//   pW - Pointer for returning the width of the rectangle.
		//   pH - Pointer for returning the height of the rectangle.

	virtual double GetDPI() = 0;
		// Get pixels per inch (horizontally).

	virtual void MoveMousePointer(double dx, double dy) = 0;
		// Move the mouse pointer programmatically.
		// Arguments:
		//   dx - Amount of pixels to move the pointer in X direction.
		//   dy - Amount of pixels to move the pointer in Y direction.

	virtual void Beep() = 0;
		// Give an acoustic warning to the user, if possible.

	virtual void DisableScreensaver() = 0;
	virtual void EnableScreensaver() = 0;
		// Disable or re-enable the screensaver. This should implement
		// an internal counter for calls to DisableScreensaver() which
		// have not yet been taken back by calls to EnableScreensaver().

	const emArray<emWindow*> & GetWindows();
		// Get an array of pointers to all windows on the screen, which
		// have been created within the context of this screen
		// interface. Other windows are not included.

	const emSignal & GetWindowsSignal();
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

	virtual emWindowPort * CreateWindowPort(emWindow & window) = 0;
		// Create a window port implementation for a new window on the
		// screen. (This is called by the constructor of emWindow)

private:
	emArray<emWindow*> Windows;
	emSignal WindowsSignal;
};

inline const emArray<emWindow*> & emScreen::GetWindows()
{
	return Windows;
}

inline const emSignal & emScreen::GetWindowsSignal()
{
	return WindowsSignal;
}


#endif
