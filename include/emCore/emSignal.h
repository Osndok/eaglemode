//------------------------------------------------------------------------------
// emSignal.h
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

#ifndef emSignal_h
#define emSignal_h

#ifndef emScheduler_h
#include <emCore/emScheduler.h>
#endif


//==============================================================================
//================================== emSignal ==================================
//==============================================================================

class emSignal : public emUncopyable {

public:

	// Class for a signal which can be sent (signaled) to all the engines
	// of an emScheduler.

	emSignal();
	~emSignal();

	void Signal(emScheduler & scheduler);
		// Signal this signal. Please even read the comments on
		// emEngine::AddWakeUpSignal, emEngine::Signal and
		// emEngine::IsSignaled.

	bool IsPending() const;
		// If a signal is signaled, it is always pending until the end
		// or beginning of the schedulers call to an emEngine::Cycle.

	void Abort();
		// Abort any signaling of this signal. No engine will see this
		// signal signaled before it is signaled again (but engines
		// could already have been woken up, except this is called in
		// the pending phase of the signal).

private:

	friend class emScheduler;
	friend class emEngine;

	struct Link {
		emEngine * Engine;
		Link * * ELThisPtr;
		Link * ELNext;
		emSignal * Signal;
		Link * * SLThisPtr;
		Link * SLNext;
		unsigned int RefCount;
	};

	emScheduler::SignalRingNode RNode;
		// Node for the list of pending signals (emScheduler::PSList).

	Link * ELFirst;
		// First element in the list of connected engines.

	emUInt64 Clock;
		// State of emScheduler::Clock after signaling this signal,
		// modified at end of pending phase.
};

inline void emSignal::Signal(emScheduler & scheduler)
{
	if (!RNode.Next) {
		RNode.Next=scheduler.PSList.Next;
		scheduler.PSList.Next=&RNode;
	}
}

inline bool emSignal::IsPending() const
{
	return RNode.Next!=NULL;
}


#endif
