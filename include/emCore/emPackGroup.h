//------------------------------------------------------------------------------
// emPackGroup.h
//
// Copyright (C) 2015 Oliver Hamann.
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

#ifndef emPackGroup_h
#define emPackGroup_h

#ifndef emPackLayout_h
#include <emCore/emPackLayout.h>
#endif


//==============================================================================
//================================ emPackGroup =================================
//==============================================================================

class emPackGroup : public emPackLayout {

public:

	// Class for a group of panels which are laid out by a pack algorithm.
	// This is just like emPackLayout, but it has a group border and it is
	// focusable.

	emPackGroup(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
};


#endif
