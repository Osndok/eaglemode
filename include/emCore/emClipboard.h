//------------------------------------------------------------------------------
// emClipboard.h
//
// Copyright (C) 2005-2008,2010-2011 Oliver Hamann.
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

#ifndef emClipboard_h
#define emClipboard_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//================================ emClipboard =================================
//==============================================================================

class emClipboard : public emModel {

public:

	// This model class acts as an interface to the common clipboard and the
	// common selection. It is an abstract base class for an interface
	// implementation. Such an implementation should also define a public
	// method like this:
	//
	//   static void Install(emContext & context);
	//
	// That method should find or create an instance of the interface
	// implementation within the given context, and it should call the
	// protected method Install for registering it as the interface to be
	// returned by LookupInherited. The main program or the emGUIFramework
	// implementation should call the public Install method on the root
	// context at program start.

	static emRef<emClipboard> LookupInherited(emContext & context);
		// Get a reference to the clipboard interface.
		// Arguments:
		//   context - The context where the clipboard interface has
		//             been installed, or any descendant context.
		//             Typically, it should have been installed in the
		//             root context, so you can give any context here.
		// Returns:
		//   The reference to the interface, or a NULL reference if not
		//   found.

	virtual emInt64 PutText(const emString & str, bool selection=false) = 0;
		// Put a text to the clipboard or selection.
		// Arguments:
		//   str       - The text.
		//   selection - Whether to put the text to the clipboard
		//               (false) or to the selection (true).
		// Returns:
		//   An identification number for the new selection (see Clear).
		//   If the text is put to the clipboard, the return value has
		//   no meaning.

	virtual void Clear(bool selection=false, emInt64 selectionId=0) = 0;
		// Clear the clipboard or selection.
		// Arguments:
		//   selection   - Whether to clear the clipboard (false) or the
		//                 selection (true).
		//   selectionId - If the selection is to be cleared:
		//                 Identification number retrieved with PutText.
		//                 If anyone else has modified the selection in
		//                 between, the call is ignored and the
		//                 selection is not cleared.

	virtual emString GetText(bool selection=false) = 0;
		// Get the text from the clipboard or selection.
		// Arguments:
		//   selection - Whether to ask the clipboard (false) or the
		//               selection (true).
		// Returns:
		//   The text, or an empty string if cleared.

protected:

	emClipboard(emContext & context, const emString & name);
		// See emModel.

	void Install();
		// Register this interface so that it can be found by
		// LookupInherited.
};


//==============================================================================
//============================= emPrivateClipboard =============================
//==============================================================================

class emPrivateClipboard : public emClipboard {

public:

	// This is a simple implementation for a clipboard which is not shared
	// with anything.

	static void Install(emContext & context);

	virtual emInt64 PutText(const emString & str, bool selection=false);

	virtual void Clear(bool selection=false, emInt64 selectionId=0);

	virtual emString GetText(bool selection=false);

private:

	emPrivateClipboard(emContext & context, const emString & name);
	virtual ~emPrivateClipboard();

	emString ClipText;
	emString SelText;
	emInt64 SelId;
};


#endif
