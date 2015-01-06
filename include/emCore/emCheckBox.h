//------------------------------------------------------------------------------
// emCheckBox.h
//
// Copyright (C) 2005-2010,2014 Oliver Hamann.
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

#ifndef emCheckBox_h
#define emCheckBox_h

#ifndef emCheckButton_h
#include <emCore/emCheckButton.h>
#endif


//==============================================================================
//================================= emCheckBox =================================
//==============================================================================

class emCheckBox : public emCheckButton {

public:

	// This is like emCheckButton, but with a different visualization:
	// Instead of a push button, a small check box is shown with the label
	// on the right.

	emCheckBox(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
};


#endif
