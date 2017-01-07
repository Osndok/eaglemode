//------------------------------------------------------------------------------
// emButton.h
//
// Copyright (C) 2005-2010,2014,2016 Oliver Hamann.
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

#ifndef emButton_h
#define emButton_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================== emButton ==================================
//==============================================================================

class emButton : public emBorder {

public:

	// Class for a button. Buttons can be triggered (clicked) by the user to
	// perform a function. The label is shown in the button face.

	emButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.

	virtual ~emButton();
		// Destructor.

	bool IsNoEOI() const;
	void SetNoEOI(bool noEOI=true);
		// Whether clicking this button is not an "End Of Interaction".
		// If false (the default), GetView().SignalEOIDelayed() is
		// called on every click.

	const emSignal & GetClickSignal() const;
		// This signal is signaled when the button has been clicked.

	const emSignal & GetPressStateSignal() const;
		// This signal is signaled when the press state has changed (see
		// IsPressed()).

	bool IsPressed() const;
		// Whether the button is currently pressed or not.

	void Click(bool shift=false);
		// Perform a button click programmatically.

protected:

	virtual void Clicked();
		// Called when the button has been clicked.

	virtual void PressStateChanged();
		// Called when the press state has changed.

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo() const;
	virtual emString GetHowTo() const;

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

	virtual void PaintBoxSymbol(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

	virtual bool CheckMouse(double mx, double my) const;

	bool IsShownChecked() const;
	bool IsShownBoxed() const;
	bool IsShownRadioed() const;
	void SetShownChecked(bool shownChecked);
	void SetShownBoxed(bool shownBoxed);
	void SetShownRadioed(bool shownRadioed);
		// Yes, this class has the ability to paint all our button
		// types.

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
	virtual bool CheckMouse(double mx, double my) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	enum DoButtonFunc {
		BUTTON_FUNC_PAINT,
		BUTTON_FUNC_CHECK_MOUSE
	};
	void DoButton(
		DoButtonFunc func, const emPainter * painter,
		emColor canvasColor,
		double mx, double my, bool * pHit
	) const;

	emSignal ClickSignal;
	emSignal PressStateSignal;
	unsigned Pressed : 1;
	unsigned NoEOI : 1;
	unsigned ShownChecked : 1;
	unsigned ShownBoxed : 1;
	unsigned ShownRadioed : 1;

	static const char * HowToButton;
	static const char * HowToEOIButton;
};

inline bool emButton::IsNoEOI() const
{
	return NoEOI;
}

inline const emSignal & emButton::GetClickSignal() const
{
	return ClickSignal;
}

inline const emSignal & emButton::GetPressStateSignal() const
{
	return PressStateSignal;
}

inline bool emButton::IsPressed() const
{
	return Pressed;
}

inline bool emButton::IsShownChecked() const
{
	return ShownChecked;
}

inline bool emButton::IsShownBoxed() const
{
	return ShownBoxed;
}

inline bool emButton::IsShownRadioed() const
{
	return ShownRadioed;
}


#endif
