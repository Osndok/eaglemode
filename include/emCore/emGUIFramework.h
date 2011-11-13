//------------------------------------------------------------------------------
// emGUIFramework.h
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

#ifndef emGUIFramework_h
#define emGUIFramework_h

#ifndef emScreen_h
#include <emCore/emScreen.h>
#endif


//==============================================================================
//============================ MAIN_OR_WINMAIN_HERE ============================
//==============================================================================

// The macro MAIN_OR_WINMAIN_HERE implements the normal main function or the
// Windows WinMain function. It calls the static function wrapped_main, which
// has to be defined by the application programmer. Example usage:
//
//   MAIN_OR_WINMAIN_HERE
//
//   static int wrapped_main(int argc, char * argv[])
//   {
//     ...
//   }

#if defined(_WIN32) && !defined(__CYGWIN__)
#	ifndef _INC_WINDOWS
#		include <windows.h>
#	endif
#	define MAIN_OR_WINMAIN_HERE \
		static int wrapped_main(int argc, char * argv[]); \
		int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) \
		{ \
			return wrapped_main(__argc,__argv); \
		}
#else
#	define MAIN_OR_WINMAIN_HERE \
		static int wrapped_main(int argc, char * argv[]); \
		int main(int argc, char * argv[]) \
		{ \
			return wrapped_main(argc,argv); \
		}
#endif


//==============================================================================
//=============================== emGUIFramework ===============================
//==============================================================================

class emGUIFramework {

public:

	// Framework for a program with graphical user interface. An object of
	// this class has an emScheduler and an emRootContext with an emScreen
	// and an emClipboard installed. Besides, the constructor calls
	// emSetFatalErrorGraphical(true).

	emGUIFramework();
		// Constructor.

	virtual ~emGUIFramework();
		// Destructor.

	emScheduler & GetScheduler();
		// Get the scheduler.

	emRootContext & GetRootContext();
		// Get the root context. It has an emScreen and an emClipboard.

	void EnableAutoTermination(bool autoTermination=true);
		// Whether to call GetScheduler().InitiateTermination(0)
		// automatically when the last window has been destructed.

	int Run();
		// Same as GetScheduler().Run()

private:

	class AutoTerminatorClass : public emEngine {
	public:
		AutoTerminatorClass(emRootContext & rootContext);
		virtual ~AutoTerminatorClass();
	protected:
		virtual bool Cycle();
	private:
		emRef<emScreen> Screen;
	};

	emScheduler * Scheduler;
	emRootContext * RootContext;
	AutoTerminatorClass * AutoTerminator;
};

inline emScheduler & emGUIFramework::GetScheduler()
{
	return *Scheduler;
}

inline emRootContext & emGUIFramework::GetRootContext()
{
	return *RootContext;
}

inline int emGUIFramework::Run()
{
	return Scheduler->Run();
}


#endif
