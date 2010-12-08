//------------------------------------------------------------------------------
// emTimer.h
//
// Copyright (C) 2006-2008,2010 Oliver Hamann.
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

#ifndef emTimer_h
#define emTimer_h

#ifndef emEngine_h
#include <emCore/emEngine.h>
#endif


//==============================================================================
//================================== emTimer ===================================
//==============================================================================

class emTimer : public emUncopyable {

public:

	// Class for a timer which signals an emSignal on each timer event.

	emTimer(emScheduler & scheduler);
		// Construct a timer. The given scheduler is used for signaling
		// the timer signal (and for an internal engine which serves all
		// the timers on that scheduler).

	virtual ~emTimer();
		// Destructor.

	emScheduler & GetScheduler();
		// Get the scheduler.

	const emSignal & GetSignal() const;
		// This signal is signaled on each timer event.

	void Start(emUInt64 millisecs, bool periodic=false);
		// Start the timer.
		// Arguments:
		//   millisecs - Time in milliseconds after which a timer event
		//               shall be generated.
		//   periodic  - If true, additional timer events are generated
		//               periodically. The given number of milliseconds
		//               is the period duration. The timer tries to keep
		//               up that rate on average, even if timer events
		//               are delayed through blockings of the scheduler,
		//               but it is limited to the rate of scheduler time
		//               slices by all means.

	bool IsRunning() const;
		// Ask whether the timer is still active.

	void Stop(bool abortSignal);
		// Stop the timer.
		// Arguments:
		//   abortSignal - Whether to perform an Abort() on the signal,
		//                 but remember that engines could already have
		//                 been woken up. Say false for an asynchronous
		//                 stop, or true for a synchronous stop.

private:

	struct TimeNode {
		emUInt64 SigTime;
		TimeNode * Prev;
		TimeNode * Next;
	};

	class TimerCentral : public emEngine {
	public:
		TimerCentral(emScheduler & scheduler);
		void Insert(TimeNode * node, emUInt64 sigTime);
		int RefCount;
	protected:
		virtual bool Cycle();
	private:
		TimeNode InList;
		TimeNode OutList;
		bool Busy;
	};

	friend class TimerCentral;

	TimerCentral * Central;
	emSignal TimerSignal;
	emUInt64 Period;
	TimeNode Node;
};

inline emScheduler & emTimer::GetScheduler()
{
	return Central->GetScheduler();
}

inline const emSignal & emTimer::GetSignal() const
{
	return TimerSignal;
}

inline bool emTimer::IsRunning() const
{
	return Node.Prev!=NULL;
}


#endif
