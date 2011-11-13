//------------------------------------------------------------------------------
// emGUIFramework.cpp
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#include <emCore/emGUIFramework.h>


extern "C" {
	typedef emScheduler * (*emGUIFramework_CreateSchedulerFunc)();
	typedef void (*emGUIFramework_InstallDriversFunc)(emRootContext * rootContext);
}


emGUIFramework::emGUIFramework()
{
	emGUIFramework_CreateSchedulerFunc createSchedulerFunc;
	emGUIFramework_InstallDriversFunc installDriversFunc;
	emString createSchedulerFuncName;
	emString installDriversFuncName;
	const char * lib;

	emSetFatalErrorGraphical(true);

	lib=getenv("EM_GUI_LIB");
	if (!lib) {
#		if defined(_WIN32)
			lib="emWnds";
#		else
			lib="emX11";
#		endif
	}

	createSchedulerFuncName=emString::Format("%sGUIFramework_CreateScheduler",lib);
	installDriversFuncName=emString::Format("%sGUIFramework_InstallDrivers",lib);

	try {
		createSchedulerFunc=(emGUIFramework_CreateSchedulerFunc)emTryResolveSymbol(
			lib,false,createSchedulerFuncName
		);
		installDriversFunc=(emGUIFramework_InstallDriversFunc)emTryResolveSymbol(
			lib,false,installDriversFuncName
		);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
		exit(255); // Just to avoid compiler warnings (never executed).
	}

	Scheduler=createSchedulerFunc();
	RootContext=new emRootContext(*Scheduler);
	installDriversFunc(RootContext);
	AutoTerminator=NULL;
}


emGUIFramework::~emGUIFramework()
{
	if (AutoTerminator) {
		delete AutoTerminator;
		AutoTerminator=NULL;
	}
	delete RootContext;
	RootContext=NULL;
	delete Scheduler;
	Scheduler=NULL;
}


void emGUIFramework::EnableAutoTermination(bool autoTermination)
{
	if (autoTermination) {
		if (!AutoTerminator) {
			AutoTerminator=new AutoTerminatorClass(*RootContext);
		}
	}
	else if (AutoTerminator) {
		delete AutoTerminator;
		AutoTerminator=NULL;
	}
}


emGUIFramework::AutoTerminatorClass::AutoTerminatorClass(emRootContext & rootContext)
	: emEngine(rootContext.GetScheduler())
{
	Screen=emScreen::LookupInherited(rootContext);
	AddWakeUpSignal(Screen->GetWindowsSignal());
}


emGUIFramework::AutoTerminatorClass::~AutoTerminatorClass()
{
}


bool emGUIFramework::AutoTerminatorClass::Cycle()
{
	if (IsSignaled(Screen->GetWindowsSignal())) {
		if (Screen->GetWindows().GetCount()<=0) {
			GetScheduler().InitiateTermination(0);
		}
	}
	return false;
}
