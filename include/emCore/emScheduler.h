//------------------------------------------------------------------------------
// emScheduler.h
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

#ifndef emScheduler_h
#define emScheduler_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif

class emEngine;


//==============================================================================
//================================ emScheduler =================================
//==============================================================================

class emScheduler : public emUncopyable {

public:

	// Abstract base class for a scheduler on emEngine objects.

	emScheduler();
	virtual ~emScheduler();

	virtual int Run() = 0;
		// Do all the time slices until InitiateTermination() is called.

	virtual bool IsTimeSliceAtEnd() const = 0;
		// See emEngine::IsTimeSliceAtEnd().

	virtual void InitiateTermination(int returnCode) = 0;
		// Tell the scheduler to terminate round about after the current
		// time slice.

	emUInt64 GetTimeSliceCounter() const;
		// This is incremented by one on each time slice.

protected:

	void DoTimeSlice();
		// This has to be called by the derived class on each time
		// slice. It performs all the scheduling for one time slice, but
		// it does not wait for IsTimeSliceAtEnd.

private:

	friend class emSignal;
	friend class emEngine;
	friend class emTimer;

	struct SignalRingNode {
		// Node for a circular single-linked list of pending signals.
		SignalRingNode * Next;
	};

	struct EngineRingNode {
		// Node for a circular double-linked list of awake engines.
		EngineRingNode * Prev;
		EngineRingNode * Next;
	};

	SignalRingNode PSList;
		// Circular single-linked list of pending signals. The order is
		// reversed before processing.

	EngineRingNode AwakeLists[10];
		// Circular double-linked lists of awake engines. Index is:
		// Priority*2+TimeSlice

	EngineRingNode * CurrentAwakeList;
		// The list currently processed for calling the engines.

	emEngine * CurrentEngine;
		// The engine currently called, or NULL.

	emUInt32 EngineCount;
		// Total number of engines.

	emInt8 TimeSlice;
		// Whether the current time slice is even (0) or odd (1).

	emUInt64 Clock;
		// Incremented on each loop-run of handling pending signals and
		// calling emEngine::Cycle.

	emUInt64 TimeSliceCounter;
		// Incremented on each time slice.

	void * TimerStuff;
		// A little hack for the implementation of emTimer.
};

inline emUInt64 emScheduler::GetTimeSliceCounter() const
{
	return TimeSliceCounter;
}


//==============================================================================
//============================ emStandardScheduler =============================
//==============================================================================

class emStandardScheduler : public emScheduler {

public:

	// Class for a standard scheduler. It tries to make the time slices 10
	// millisecs long, but IsTimeSliceAtEnd() allows to have 50 millisecs
	// per time slice (for reducing the graphics frame rate when busy).

	emStandardScheduler();

	virtual int Run();
	virtual bool IsTimeSliceAtEnd() const;
	virtual void InitiateTermination(int returnCode);

private:

	bool TerminationInitiated;
	int ReturnCode;
	emUInt64 SyncTime, DeadlineTime;
};


#endif
