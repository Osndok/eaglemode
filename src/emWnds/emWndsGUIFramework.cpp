//------------------------------------------------------------------------------
// emWndsGUIFramework.cpp
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#include <emWnds/emWndsScheduler.h>
#include <emWnds/emWndsScreen.h>
#include <emWnds/emWndsClipboard.h>


extern "C" {
	emScheduler * emWndsGUIFramework_CreateScheduler()
	{
		return new emWndsScheduler();
	}

	void emWndsGUIFramework_InstallDrivers(emRootContext * rootContext)
	{
		emWndsScreen::Install(*rootContext);
		emWndsClipboard::Install(*rootContext);
	}
}
