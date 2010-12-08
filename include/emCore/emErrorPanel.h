//------------------------------------------------------------------------------
// emErrorPanel.h
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

#ifndef emErrorPanel_h
#define emErrorPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//================================ emErrorPanel ================================
//==============================================================================

class emErrorPanel : public emPanel {

public:

	// Class for a red panel showing an error message.

	emErrorPanel(ParentArg parent, const emString & name,
	             const emString & errorMessage);

	virtual ~emErrorPanel();

protected:

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	emString ErrorMessage;
};


#endif
