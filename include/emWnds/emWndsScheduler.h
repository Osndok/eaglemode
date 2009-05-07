//------------------------------------------------------------------------------
// emWndsScheduler.h
//
// Copyright (C) 2007-2009 Oliver Hamann.
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

#ifndef emWndsScheduler_h
#define emWndsScheduler_h

#ifndef emScheduler_h
#include <emCore/emScheduler.h>
#endif

#ifndef emThread_h
#include <emCore/emThread.h>
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif


class emWndsScheduler : public emScheduler {

public:

	// This scheduler works like emStandardScheduler, but in addition it
	// dispatches the Windows messages. Doing that dispatching in an
	// emEngine would be a bad idea: while the user moves or resizes a
	// window, DefWindowProc blocks and runs its own message loop.

	emWndsScheduler();
	virtual ~emWndsScheduler();

	virtual int Run();

	virtual bool IsTimeSliceAtEnd() const;

	virtual void InitiateTermination(int returnCode);

	enum StateType {
		NOT_RUNNING,
		HANDLING_MESSAGES,
		CALLING_ENGINES
	};
	StateType GetState() const;

private:

	void DoTimeSlice();

#if defined(_WIN64)
	typedef UINT_PTR TimerIdType;
#else
	typedef UINT TimerIdType;
#endif

	static VOID CALLBACK TimerProc(
		HWND hwnd, UINT uMsg, TimerIdType idEvent, DWORD dwTime
	);

	static emThreadMiniMutex InstanceListMutex;
	static emWndsScheduler * InstanceList;
	emWndsScheduler * NextInstance;
	StateType State;
	TimerIdType TimerId;
	bool TerminationInitiated;
	emUInt64 DeadlineTime;
};

inline emWndsScheduler::StateType emWndsScheduler::GetState() const
{
	return State;
}


#endif
