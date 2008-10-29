//------------------------------------------------------------------------------
// emPriSchedAgent.cpp
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

#include <emCore/emPriSchedAgent.h>


emPriSchedAgent::emPriSchedAgent(
	emContext & context, const emString & resourceName, double priority
)
{
	PriSched=PriSchedModel::Acquire(context,resourceName);
	Priority=priority;
	ThisPtrInList=NULL;
	NextInList=NULL;
}


emPriSchedAgent::~emPriSchedAgent()
{
	ReleaseAccess();
}


void emPriSchedAgent::SetAccessPriority(double priority)
{
	Priority=priority;
}


void emPriSchedAgent::RequestAccess()
{
	if (!ThisPtrInList) {
		NextInList=PriSched->List;
		if (NextInList) {
			NextInList->ThisPtrInList=&NextInList;
		}
		PriSched->List=this;
		ThisPtrInList=&PriSched->List;
	}
	if (PriSched->Active==this) PriSched->Active=NULL;
	if (!PriSched->Active) PriSched->WakeUp();
}


void emPriSchedAgent::ReleaseAccess()
{
	if (ThisPtrInList) {
		*ThisPtrInList=NextInList;
		if (NextInList) {
			NextInList->ThisPtrInList=ThisPtrInList;
			NextInList=NULL;
		}
		ThisPtrInList=NULL;
	}
	if (PriSched->Active==this) {
		PriSched->Active=NULL;
		PriSched->WakeUp();
	}
}


emRef<emPriSchedAgent::PriSchedModel> emPriSchedAgent::PriSchedModel::Acquire(
	emContext & context, const emString & name
)
{
	EM_IMPL_ACQUIRE_COMMON(emPriSchedAgent::PriSchedModel,context,name)
}


emPriSchedAgent::PriSchedModel::PriSchedModel(
	emContext & context, const emString & name
)
	: emModel(context,name)
{
	List=NULL;
	Active=NULL;
	SetEnginePriority(LOW_PRIORITY);
}


bool emPriSchedAgent::PriSchedModel::Cycle()
{
	emPriSchedAgent * p, * best;
	double bestPri;

	if (!List || Active) return false;
	p=List;
	best=p;
	bestPri=p->Priority;
	for (;;) {
		p=p->NextInList;
		if (!p) break;
		if (p->Priority<bestPri) continue;
		bestPri=p->Priority;
		best=p;
	}
	*best->ThisPtrInList=best->NextInList;
	if (best->NextInList) {
		best->NextInList->ThisPtrInList=best->ThisPtrInList;
		best->NextInList=NULL;
	}
	best->ThisPtrInList=NULL;
	Active=best;
	Active->GotAccess();
	return false;
}
