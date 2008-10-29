//------------------------------------------------------------------------------
// emScreen.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emCore/emVarModel.h>
#include <emCore/emScreen.h>
#include <emCore/emWindow.h>


emRef<emScreen> emScreen::LookupInherited(emContext & context)
{
	return emVarModel<emRef<emScreen> >::GetInherited(
		context,
		"emScreen::InstalledRef",
		emRef<emScreen>()
	);
}


emScreen::emScreen(emContext & context, const emString & name)
	: emModel(context,name)
{
	Windows.SetTuningLevel(4);
}


emScreen::~emScreen()
{
}


void emScreen::LeaveFullscreenModes(emWindow * exceptForWindow)
{
	emWindow * w;
	int i;

	for (i=Windows.GetCount()-1; i>=0; i--) {
		w=Windows[i];
		if (
			(w->GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0 &&
			w!=exceptForWindow
		) {
			w->SetWindowFlags(w->GetWindowFlags()&~emWindow::WF_FULLSCREEN);
		}
	}
}


void emScreen::Install()
{
	emVarModel<emRef<emScreen> >::Set(
		GetContext(),
		"emScreen::InstalledRef",
		emRef<emScreen>(this),
		UINT_MAX
	);
}
