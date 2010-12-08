//------------------------------------------------------------------------------
// emEngine.h
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

#ifndef emEngine_h
#define emEngine_h

#ifndef emSignal_h
#include <emCore/emSignal.h>
#endif


//==============================================================================
//================================== emEngine ==================================
//==============================================================================

class emEngine : public emUncopyable {

public:

	// The classes emEngine, emSignal and emScheduler can be used for some
	// kind of CPU sharing and event handling, within one thread of
	// execution.
	//
	// The central point is the virtual method Cycle of emEngine. Derived
	// classes have to overload it for doing something useful in it.
	//
	// Polling
	// -------
	//
	// One possibility is that Cycle could be called by the scheduler on
	// every time slice. The size of a time slice is defined by the actual
	// derivative of emScheduler. It should be about 10 millisecs.
	//
	// Such a polling can be made by waking up the engine once through
	// calling WakeUp, and by always returning true from the Cycle call.
	//
	// Not polling
	// -----------
	//
	// For sparing CPU cycles, polling should be used in rare cases only.
	// Best is to let the engine sleep most times, and to wake it up
	// whenever there is something useful to do.
	//
	// After calling WakeUp on an engine, Cycle is called by the scheduler
	// within the current time slice. The Cycle call should return false for
	// falling asleep again.
	//
	// Through the wake-up mechanism, Cycle can be called multiple times
	// within one time slice. For example, this allows to have chains of
	// signals without any delay through the time slices.
	//
	// Signals
	// -------
	//
	// emSignal can be used for sending a unary event to the engines without
	// having references to them. But all receiving engines must share the
	// same scheduler, and the sender must have a reference to that
	// scheduler. Very often, the sender is even an engine.
	//
	// An engine can receive a signal event by calling IsSignaled from
	// within Cycle. To avoid polling, an engine can be woken up by signals.
	// Therefore any number of signals can be connected to an engine with
	// AddWakeUpSignal.
	//
	// Here is an example of how a sender can send two different signals A
	// and B to receivers:
	//
	// class MySender : public emEngine {
	// private:
	//   emSignal A;
	//   emSignal B;
	// public:
	//   inline MySender(emScheduler & scheduler) : emEngine(scheduler) {}
	//   inline const emSignal & GetSignalA() const { return A; }
	//   inline const emSignal & GetSignalB() const { return B; }
	//   inline void SendA() { Signal(A); }
	//   inline void SendB() { Signal(B); }
	// };
	//
	// class MyReceiver : public emEngine {
	// private:
	//   MySender & Sender;
	// public:
	//   inline MyReceiver(MySender & sender)
	//     : emEngine(sender.GetScheduler()), Sender(sender)
	//   {
	//     AddWakeUpSignal(Sender.GetSignalA());
	//     AddWakeUpSignal(Sender.GetSignalB());
	//   }
	// protected:
	//   virtual bool Cycle();
	// };
	//
	// bool MyReceiver::Cycle()
	// {
	//   if (IsSignaled(Sender.GetSignalA())) {
	//     //...got signal A...
	//   }
	//   if (IsSignaled(Sender.GetSignalB())) {
	//     //...got signal B...
	//   }
	//   return false;
	// }
	//
	// Minimum fairness
	// ----------------
	//
	// A call to Cycle should not block for a too long time. If an engine
	// wants to perform a long-time job, it should divide that job into
	// multiple time slices. The method IsTimeSliceAtEnd can be helpful
	// here. Example:
	//
	// bool JobEngine::Cycle()
	// {
	//   while (!JobFinished()) {
	//     WorkOnTheJobALittleBit();
	//     if (IsTimeSliceAtEnd()) break;
	//   }
	//   return !JobFinished();
	// }
	//
	// The above example provides just minimum fairness, because if there
	// are two such job engines, one would always eat the whole time slice
	// and the other would perform WorkOnTheJobALittleBit only once per time
	// slice.
	//
	// ??? More fairness for parallel jobs may be supported in the future.
	// ??? Maybe the return value of Cycle should be replaced by a method.
	//
	// There is a helper for serializing jobs: emPriSchedAgent

	emEngine(emScheduler & scheduler);
		// Construct this as a sleeping engine.
		// Arguments:
		//   scheduler - The scheduler which has to serve this engine.

	virtual ~emEngine();
		// Destructor.

	emScheduler & GetScheduler();
		// Get the scheduler.

	void WakeUp();
		// Wake up this engine. This means that Cycle will be called by
		// the scheduler within the current time slice. Even if WakeUp
		// is called from within Cycle, Cycle will be called again
		// within the current time slice.

	void AddWakeUpSignal(const emSignal & signal);
		// Wake up this engine whenever the given signal is signaled.
		// Waking up through a signal is like calling WakeUp. Adding the
		// same signal more than once does not cost any additional
		// resources. But the number of additions is counted.

	void RemoveWakeUpSignal(const emSignal & signal);
		// Remove the given signal from the set of signals waking up
		// this engine. If a signal has been added multiple times, it
		// would have to be removed for the same number of times, before
		// it would really be disconnected from the engine (this is for
		// solving conflicts between base classes and derived classes:
		// the derived class does not need to know whether the base
		// class has added a certain signal for its own purpose). There
		// is no need to remove signals before destruction of signals or
		// engines. This is solved by the destructors more efficient.

	int GetWakeUpSignalRefs(const emSignal & signal) const;
		// Get number of references to the given signal (how often the
		// signal has been added and not removed).

	void Signal(emSignal & signal);
		// Signal the given signal. This is just a short cut for
		// signal.Signal(GetScheduler()). It does not matter on which
		// emEngine the call is made, as long as they belong to the same
		// scheduler.

	bool IsSignaled(const emSignal & signal) const;
		// Ask whether the given signal has been signaled. This must be
		// called only from within the Cycle method of this engine. The
		// return value is true, if the signal has been signaled between
		// the beginning of the previous call to Cycle and the beginning
		// of the current call to Cycle.

	bool IsTimeSliceAtEnd() const;
		// Returns true if the current time slice is at its end.

	enum PriorityType {
		VERY_LOW_PRIORITY  = 0,
		LOW_PRIORITY       = 1,
		DEFAULT_PRIORITY   = 2,
		HIGH_PRIORITY      = 3,
		VERY_HIGH_PRIORITY = 4
	};
	void SetEnginePriority(PriorityType priority);
	PriorityType GetEnginePriority() const;
		// Set or get the priority. If two engines are to be called
		// within the same time slice, the one with the higher priority
		// is called first. And if the priority is equal, it's the order
		// of waking up the engines. VERY_LOW_PRIORITY and
		// VERY_HIGH_PRIORITY are meant to be used by driver
		// implementations only. A GUI driver should perform input at
		// VERY_HIGH_PRIORITY and output at VERY_LOW_PRIORITY.

protected:

	virtual bool Cycle() = 0;
		// This is the virtual Cycle method mentioned all above. It is
		// only to be called by the scheduler. There is no problem with
		// doing a 'delete this' within Cycle, from the schedulers point
		// of view.
		// The meaning of the return value is:
		//  false: Fall asleep if this engine has not been woken up
		//         since the beginning of this call to Cycle.
		//   true: Wake up this engine on next time slice if the engine
		//         will not be woken up earlier or if it has not been
		//         woken up since the beginning of this call to Cycle.

private:

	friend class emScheduler;
	friend class emSignal;

	void WakeUpImp();

	static void RemoveLink(emSignal::Link * link);

	emScheduler & Scheduler;
		// The scheduler.

	emScheduler::EngineRingNode RNode;
		// A node for this engine in a list of awake engines, garbage
		// when sleeping.

	emSignal::Link * SLFirst;
		// First element in the list of connected signals.

	emInt8 AwakeState;
		// -1 <=> Sleeping.
		// emScheduler::TimeSlice <=> Busy in current time slice
		// emScheduler::TimeSlice^1 <=> Busy in next time slice

	emUInt8 Priority;
		// The priority (0-4).

	emUInt64 Clock;
		// State of emScheduler::Clock after last call to Cycle().
};

inline emScheduler & emEngine::GetScheduler()
{
	return Scheduler;
}

inline void emEngine::WakeUp()
{
	if (AwakeState!=Scheduler.TimeSlice) WakeUpImp();
}

inline void emEngine::Signal(emSignal & signal)
{
	if (!signal.RNode.Next) {
		signal.RNode.Next=Scheduler.PSList.Next;
		Scheduler.PSList.Next=&signal.RNode;
	}
}

inline bool emEngine::IsSignaled(const emSignal & signal) const
{
	return signal.Clock>Clock;
}

inline bool emEngine::IsTimeSliceAtEnd() const
{
	return Scheduler.IsTimeSliceAtEnd();
}

inline emEngine::PriorityType emEngine::GetEnginePriority() const
{
	return (PriorityType)Priority;
}


#endif
