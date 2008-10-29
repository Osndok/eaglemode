//------------------------------------------------------------------------------
// emTimer.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emCore/emTimer.h>


emTimer::emTimer(emScheduler & scheduler)
{
	if (scheduler.TimerStuff) {
		Central=(TimerCentral*)scheduler.TimerStuff;
		Central->RefCount++;
	}
	else {
		Central=new TimerCentral(scheduler);
		Central->RefCount=1;
		scheduler.TimerStuff=Central;
	}
	Node.Prev=NULL;
	Node.Next=NULL;
}


emTimer::~emTimer()
{
	if (Node.Prev) {
		Node.Prev->Next=Node.Next;
		Node.Next->Prev=Node.Prev;
	}
	Central->RefCount--;
	if (Central->RefCount<=0) {
		Central->GetScheduler().TimerStuff=NULL;
		delete Central;
	}
}


void emTimer::Start(emUInt64 millisecs, bool periodic)
{
	if (!periodic) Period=0;
	else if (!millisecs) Period=1;
	else Period=millisecs;
	if (Node.Prev) {
		Node.Prev->Next=Node.Next;
		Node.Next->Prev=Node.Prev;
	}
	Central->Insert(&Node,emGetClockMS()+millisecs);
}


void emTimer::Stop(bool abortSignal)
{
	if (Node.Prev) {
		Node.Prev->Next=Node.Next;
		Node.Next->Prev=Node.Prev;
		Node.Prev=NULL;
		Node.Next=NULL;
	}
	if (abortSignal) TimerSignal.Abort();
}


emTimer::TimerCentral::TimerCentral(emScheduler & scheduler)
	: emEngine(scheduler)
{
	InList.SigTime=0;
	InList.Prev=&InList;
	InList.Next=&InList;
	OutList.SigTime=0;
	OutList.Prev=&OutList;
	OutList.Next=&OutList;
	Busy=false;
	SetEnginePriority(emEngine::VERY_HIGH_PRIORITY);
}


void emTimer::TimerCentral::Insert(TimeNode * node, emUInt64 sigTime)
{
	TimeNode * p, * n;

	node->SigTime=sigTime;
	InList.SigTime=0;
	for (p=InList.Prev; p->SigTime>sigTime; p=p->Prev);
	n=p->Next;
	node->Prev=p;
	node->Next=n;
	p->Next=node;
	n->Prev=node;
	if (!Busy) {
		Busy=true;
		WakeUp();
	}
}


bool emTimer::TimerCentral::Cycle()
{
	TimeNode * i1, * i2, * o1, * o2;
	emTimer * t;
	emUInt64 ct, st;

	i1=InList.Next;
	if (i1!=&InList) {
		InList.SigTime=EM_UINT64_MAX;
		o2=&OutList;
		for (;;) {
			o2=o2->Next;
			if (o2!=&OutList) {
				if (o2->SigTime<=i1->SigTime) continue;
				i2=i1->Next;
				while (o2->SigTime>i2->SigTime) i2=i2->Next;
			}
			else {
				i2=&InList;
			}
			o1=o2->Prev;
			i1->Prev=o1;
			o1->Next=i1;
			i1=i2->Prev;
			o2->Prev=i1;
			i1->Next=o2;
			i1=i2;
			if (i1==&InList) break;
		}
		InList.Prev=&InList;
		InList.Next=&InList;
	}

	o1=OutList.Next;
	if (o1==&OutList) {
		Busy=false;
		return false;
	}

	// Remember to come to this point at most once per time slice.
	// (=> do not set Busy=false from here on...)

	ct=emGetClockMS();
	if (o1->SigTime>ct) {
		return true;
	}
	do {
		o2=o1->Next;
		t=(emTimer*)(((char*)o1)-offsetof(emTimer,Node));
		Signal(t->TimerSignal);
		if (t->Period) {
			st=o1->SigTime+t->Period;
			if (st<ct) st=ct;
			Insert(o1,st);
		}
		else {
			o1->Next=NULL;
			o1->Prev=NULL;
		}
		o1=o2;
	} while (o1->SigTime<=ct && o1!=&OutList);
	o1->Prev=&OutList;
	OutList.Next=o1;
	return true;
}
