//------------------------------------------------------------------------------
// emEngine.cpp
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

#include <emCore/emEngine.h>


emEngine::emEngine(emScheduler & scheduler)
	: Scheduler(scheduler)
{
	Scheduler.EngineCount++;
	RNode.Prev=NULL;
	RNode.Next=NULL;
	SLFirst=NULL;
	AwakeState=-1;
	Priority=DEFAULT_PRIORITY;
	Clock=Scheduler.Clock;
}


emEngine::~emEngine()
{
	while (SLFirst) RemoveLink(SLFirst);
	if (Scheduler.CurrentEngine==this) Scheduler.CurrentEngine=NULL;
	if (AwakeState>=0) {
		RNode.Next->Prev=RNode.Prev;
		RNode.Prev->Next=RNode.Next;
	}
	Scheduler.EngineCount--;
}


void emEngine::AddWakeUpSignal(const emSignal & signal)
{
	emSignal * sig;
	emSignal::Link * sl, * el;

	sig=(emSignal*)&signal;
	sl=SLFirst;
	el=sig->ELFirst;
	if (sl && el) {
		do {
			if (sl->Signal==sig) {
				sl->RefCount++;
				return;
			}
			sl=sl->SLNext;
			if (!sl) break;
			if (el->Engine==this) {
				el->RefCount++;
				return;
			}
			el=el->ELNext;
		} while (el);
	}
	sl=(emSignal::Link*)malloc(sizeof(emSignal::Link));
	sl->Engine=this;
	sl->ELThisPtr=&sig->ELFirst;
	sl->ELNext=sig->ELFirst;
	if (sl->ELNext) sl->ELNext->ELThisPtr=&sl->ELNext;
	sig->ELFirst=sl;
	sl->Signal=sig;
	sl->SLThisPtr=&SLFirst;
	sl->SLNext=SLFirst;
	if (sl->SLNext) sl->SLNext->SLThisPtr=&sl->SLNext;
	SLFirst=sl;
	sl->RefCount=1;
}


void emEngine::RemoveWakeUpSignal(const emSignal & signal)
{
	emSignal * sig;
	emSignal::Link * sl, * el;

	sig=(emSignal*)&signal;
	sl=SLFirst;
	el=sig->ELFirst;
	if (!sl || !el) return;
	for (;;) {
		if (sl->Signal==sig) {
			break;
		}
		sl=sl->SLNext;
		if (!sl) return;
		if (el->Engine==this) {
			sl=el;
			break;
		}
		el=el->ELNext;
		if (!el) return;
	}
	if (--sl->RefCount>0) return;
	RemoveLink(sl);
}


int emEngine::GetWakeUpSignalRefs(const emSignal & signal) const
{
	const emSignal::Link * sl, * el;

	sl=SLFirst;
	el=signal.ELFirst;
	if (sl && el) {
		do {
			if (sl->Signal==&signal) {
				return sl->RefCount;
			}
			sl=sl->SLNext;
			if (!sl) break;
			if (el->Engine==this) {
				return el->RefCount;
			}
			el=el->ELNext;
		} while (el);
	}
	return 0;
}


void emEngine::SetEnginePriority(PriorityType priority)
{
	emScheduler::EngineRingNode * l;

	if (Priority==priority) return;
	Priority=priority;
	if (AwakeState<0) return;
	RNode.Prev->Next=RNode.Next;
	RNode.Next->Prev=RNode.Prev;
	l=Scheduler.AwakeLists+(Priority*2+AwakeState);
	if (
		Scheduler.CurrentAwakeList<l &&
		AwakeState==Scheduler.TimeSlice
	) Scheduler.CurrentAwakeList=l;
	RNode.Next=l;
	RNode.Prev=l->Prev;
	l->Prev->Next=&RNode;
	l->Prev=&RNode;
}


void emEngine::WakeUpImp()
{
	emScheduler::EngineRingNode * l;

	if (AwakeState>=0) {
		RNode.Prev->Next=RNode.Next;
		RNode.Next->Prev=RNode.Prev;
	}
	AwakeState=Scheduler.TimeSlice;
	l=Scheduler.AwakeLists+(Priority*2+AwakeState);
	if (Scheduler.CurrentAwakeList<l) Scheduler.CurrentAwakeList=l;
	RNode.Next=l;
	RNode.Prev=l->Prev;
	l->Prev->Next=&RNode;
	l->Prev=&RNode;
}


void emEngine::RemoveLink(emSignal::Link * link)
{
	*link->SLThisPtr=link->SLNext;
	if (link->SLNext) link->SLNext->SLThisPtr=link->SLThisPtr;
	*link->ELThisPtr=link->ELNext;
	if (link->ELNext) link->ELNext->ELThisPtr=link->ELThisPtr;
	free((void*)link);
}
