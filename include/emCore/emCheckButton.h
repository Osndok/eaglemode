//------------------------------------------------------------------------------
// emCheckButton.h
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

#ifndef emCheckButton_h
#define emCheckButton_h

#ifndef emButton_h
#include <emCore/emButton.h>
#endif


//==============================================================================
//=============================== emCheckButton ================================
//==============================================================================

class emCheckButton : public emButton {

public:

	// Class for a check button. This is like emButton, but a check state
	// is managed and shown. The check state toggles on every click of the
	// button, for switching something on and off.

	emCheckButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emButton. The initial check state is false.

	virtual ~emCheckButton();
		// Destructor.

	const emSignal & GetCheckSignal() const;
		// This signal is signaled when the check state has changed.

	bool IsChecked() const;
	void SetChecked(bool checked=true);
		// Get/set the check state of this button.

protected:

	virtual void Clicked();
		// See emButton. This implements the toggling of the check
		// state.

	virtual void CheckChanged();
		// Called when the check state has changed.

	virtual emString GetHowTo() const;

private:

	emSignal CheckSignal;
	bool Checked;

	static const char * HowToCheckButton;
	static const char * HowToChecked;
	static const char * HowToNotChecked;
};

inline const emSignal & emCheckButton::GetCheckSignal() const
{
	return CheckSignal;
}

inline bool emCheckButton::IsChecked() const
{
	return Checked;
}


#endif
