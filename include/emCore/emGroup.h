//------------------------------------------------------------------------------
// emGroup.h
//
// Copyright (C) 2005-2010,2014-2015 Oliver Hamann.
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

#ifndef emGroup_h
#define emGroup_h

#ifndef emTiling_h
#include <emCore/emTiling.h>
#endif


//==============================================================================
//================================== emGroup ===================================
//==============================================================================

class emGroup : public emTiling {

public:

	// *********************************************************************
	// *                             WARNING!!!                            *
	// *                                                                   *
	// * This class is deprecated and will be removed in a future version. *
	// * Please use emLinearGroup, emRasterGroup or emPackGroup instead.   *
	// *********************************************************************
	//
	// Class for a group of panels. Any user-created child panels are laid
	// out automatically. This is just like emTiling, but it has a group
	// border and it is focusable.

	EM_DEPRECATED( // Because the whole class is deprecated!
		emGroup(
			ParentArg parent, const emString & name,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		)
	);

protected:
	// Only for emRadioButton::Group:
	emGroup(
		ParentArg parent, const emString & name,
		const emString & caption,
		const emString & description,
		const emImage & icon,
		int notWarningDeprecatedForInternalUse
	);
};


#endif
