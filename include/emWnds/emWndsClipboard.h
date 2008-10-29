//------------------------------------------------------------------------------
// emWndsClipboard.h
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

#ifndef emWndsClipboard_h
#define emWndsClipboard_h

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif


class emWndsClipboard : public emClipboard {

public:

	static void Install(emContext & context);

	virtual emInt64 PutText(const emString & str, bool selection=false);

	virtual void Clear(bool selection=false, emInt64 selectionId=0);

	virtual emString GetText(bool selection=false);

private:

	emWndsClipboard(emContext & context, const emString & name);
	virtual ~emWndsClipboard();

	emString SelectionText;
	emInt64 CurrentSelectionId;
};


#endif
