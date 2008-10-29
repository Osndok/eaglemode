//------------------------------------------------------------------------------
// emScheduler.cpp
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


//==============================================================================
//================================ emScheduler =================================
//==============================================================================

emScheduler::emScheduler()
{
	int i;

	PSList.Next=&PSList;
	for (i=0; i<10; i++) {
		AwakeLists[i].Next=&AwakeLists[i];
		AwakeLists[i].Prev=&AwakeLists[i];
	}
	CurrentAwakeList=NULL;
	CurrentEngine=NULL;
	EngineCount=0;
	TimeSlice=0;
	Clock=1;
	TimeSliceCounter=0;
	TimerStuff=NULL;
}


emScheduler::~emScheduler()
{
	if (EngineCount) {
		emFatalError("emScheduler::~emScheduler(): remaining emEngine");
	}
	if (PSList.Next!=&PSList) {
		emFatalError("emScheduler::~emScheduler(): remaining emSignal");
	}
}


void emScheduler::DoTimeSlice()
{
	SignalRingNode * sr1, * sr2, * sr3;
	EngineRingNode * l, * er;
	emSignal::Link * el;
	emEngine * e;
	emSignal * s;
	int nextTimeSlice;

	TimeSliceCounter++;
	nextTimeSlice=TimeSlice^1;
	CurrentAwakeList=AwakeLists+8+TimeSlice;
	for (;;) {
		Clock++;
		if ((sr1=PSList.Next)!=&PSList) {
			if ((sr2=sr1->Next)!=&PSList) {
				sr1->Next=&PSList;
				sr3=sr2->Next;
				sr2->Next=sr1;
				do {
					sr1=sr3->Next;
					sr3->Next=sr2;
					if (sr3==&PSList) break;
					sr2=sr1->Next;
					sr1->Next=sr3;
					if (sr1==&PSList) break;
					sr3=sr2->Next;
					sr2->Next=sr1;
				} while (sr2!=&PSList);
			}
			do {
				s=(emSignal*)(((char*)PSList.Next)-offsetof(emSignal,RNode));
				PSList.Next=s->RNode.Next;
				s->RNode.Next=NULL;
				s->Clock=Clock;
				el=s->ELFirst;
				if (el) {
					do {
						el->Engine->WakeUp();
						el=el->ELNext;
					} while (el);
				}
			} while (PSList.Next!=&PSList);
		}
		for (;;) {
			l=CurrentAwakeList;
			er=l->Next;
			if (er!=l) break;
			CurrentAwakeList-=2;
			if (CurrentAwakeList<AwakeLists) {
				TimeSlice=(emInt8)nextTimeSlice;
				CurrentAwakeList=NULL;
				CurrentEngine=NULL;
				return;
			}
		}
		e=(emEngine*)(((char*)er)-offsetof(emEngine,RNode));
		e->AwakeState=-1;
		e->RNode.Next->Prev=e->RNode.Prev;
		e->RNode.Prev->Next=e->RNode.Next;
		CurrentEngine=e;
		if (!e->Cycle()) {
			if ((e=CurrentEngine)==NULL) continue;
			e->Clock=Clock;
			continue;
		}
		if ((e=CurrentEngine)==NULL) continue;
		e->Clock=Clock;
		if (e->AwakeState>=0) continue;
		e->AwakeState=(emInt8)nextTimeSlice;
		l=AwakeLists+(e->Priority*2+nextTimeSlice);
		e->RNode.Next=l;
		e->RNode.Prev=l->Prev;
		l->Prev->Next=&e->RNode;
		l->Prev=&e->RNode;
	}
}


//==============================================================================
//============================ emStandardScheduler =============================
//==============================================================================

emStandardScheduler::emStandardScheduler()
{
	TerminationInitiated=false;
	ReturnCode=0;
	SyncTime=0;
	DeadlineTime=0;
}


int emStandardScheduler::Run()
{
	emUInt64 clk;

	TerminationInitiated=false;
	ReturnCode=0;
	SyncTime=0;
	do {
		clk=emGetClockMS();
		if (SyncTime>clk) emSleepMS((int)(SyncTime-clk));
		SyncTime+=10;
		if (SyncTime<clk) SyncTime=clk;
		DeadlineTime=SyncTime+50;
		DoTimeSlice();
	} while (!TerminationInitiated);
	return ReturnCode;
}


bool emStandardScheduler::IsTimeSliceAtEnd() const
{
	return emGetClockMS()>=DeadlineTime;
}


void emStandardScheduler::InitiateTermination(int returnCode)
{
	if (!TerminationInitiated) {
		ReturnCode=returnCode;
		TerminationInitiated=true;
	}
}
