//------------------------------------------------------------------------------
// emX11Clipboard.h
//
// Copyright (C) 2005-2008,2010 Oliver Hamann.
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

#ifndef emX11Clipboard_h
#define emX11Clipboard_h

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif

#ifndef emX11Screen_h
#include <emX11/emX11Screen.h>
#endif


class emX11Clipboard : public emClipboard {

public:

	static void Install(emContext & context);

	virtual emInt64 PutText(const emString & str, bool selection=false);

	virtual void Clear(bool selection=false, emInt64 selectionId=0);

	virtual emString GetText(bool selection=false);

private:

	friend class emX11Screen;

	emX11Clipboard(emContext & context, const emString & name);
	virtual ~emX11Clipboard();

	XSelectionEvent * WaitSelectionEvent(Atom selection, Atom target);

	void HandleEvent(XEvent & event);

	void HandleSelectionClear(XSelectionClearEvent & sce);
	void HandleSelectionRequest(XSelectionRequestEvent & sre);
	void HandleSelectionNotify(XSelectionEvent & se);

	emArray<emByte> GetLargeWindowProperty(
		Display * display, Window window, Atom property, bool deleteProperty,
		Atom type, Atom * actual_type_return, int * actual_format_return,
		unsigned long * nitems_return
	);

	static emString Latin1ToUtf8(const emString & latin1);
	static emString Utf8ToLatin1(const emString & utf8);
		//??? These functions should move into a public module.

	emRef<emX11Screen> Screen;
	emThreadMiniMutex * XMutex;
	Display * Disp;
	::Window Win;
	Atom MY_XA_CLIPBOARD;
	Atom MY_XA_TARGETS;
	Atom MY_XA_TIMESTAMP;
	Atom MY_XA_UTF8_STRING;
	Atom SelAtom[2];
	emString LocalText[2];
	Time LocalTimestamp[2];
	emInt64 LocalSelectionId;
	XSelectionEvent LastSelectionEvent;
};


#endif
