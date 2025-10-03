//------------------------------------------------------------------------------
// emJob.cpp
//
// Copyright (C) 2024 Oliver Hamann.
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

#include <emCore/emJob.h>
#include <emCore/emList.h>


//==============================================================================
//=================================== emJob ====================================
//==============================================================================

emJob::emJob(double priority)
	: Priority(priority),
	State(ST_NOT_ENQUEUED),
	Queue(NULL),
	Prev(NULL),
	Next(NULL)
{
}


emJob::~emJob()
{
	if (Queue) {
		emFatalError("emJob destructed while still referenced by an emJobQueue");
	}
}


void emJob::SetPriority(double priority)
{
	if (Priority!=priority) {
		Priority=priority;
		if (State==ST_WAITING) {
			Queue->InvalidateSortingOfWaitingJobs();
		}
	}
}


//==============================================================================
//================================= emJobQueue =================================
//==============================================================================

emJobQueue::emJobQueue(emScheduler & scheduler)
	: Scheduler(scheduler),
	FirstWaitingJob(NULL),
	LastWaitingJob(NULL),
	FirstRunningJob(NULL),
	LastRunningJob(NULL),
	SortingOfWaitingJobsInvalid(false)
{
}


emJobQueue::~emJobQueue()
{
	while (FirstRunningJob) AbortJob(*FirstRunningJob);
	while (FirstWaitingJob) AbortJob(*FirstWaitingJob);
}


void emJobQueue::EnqueueJob(emJob & job)
{
	if (job.Queue)
		emFatalError("emJobQueue::EnqueueJob: job already in a queue");

	job.Alloc();

	job.Queue=this;

	job.Prev=LastWaitingJob;
	job.Next=NULL;
	if (LastWaitingJob) LastWaitingJob->Next=&job;
	else FirstWaitingJob=&job;
	LastWaitingJob=&job;

	job.State=emJob::ST_WAITING;
	job.ErrorText.Clear();
	job.StateSignal.Signal(Scheduler);

	InvalidateSortingOfWaitingJobs();
}


void emJobQueue::InvalidateSortingOfWaitingJobs()
{
	SortingOfWaitingJobsInvalid=true;
}


int emJobQueue::CompareForSortingOfWaitingJobs(emJob & job1, emJob & job2) const
{
	return job2.Priority>job1.Priority ? 1 : job2.Priority<job1.Priority ? -1 : 0;
}


void emJobQueue::UpdateSortingOfWaitingJobs()
{
	if (SortingOfWaitingJobsInvalid) {
		emSortDoubleLinkedList(
			(void**)(void*)&FirstWaitingJob,
			(void**)(void*)&LastWaitingJob,
			offsetof(emJob,Next),
			offsetof(emJob,Prev),
			(int(*)(void*,void*,void*))CompareJobs,
			this
		);
		SortingOfWaitingJobsInvalid=false;
	}
}


emJob * emJobQueue::StartNextJob()
{
	emJob * job;

	if (!FirstWaitingJob)
		return NULL;

	UpdateSortingOfWaitingJobs();

	job=FirstWaitingJob;
	StartJob(*job);
	return job;
}


void emJobQueue::StartJob(emJob & job)
{
	if (job.State == emJob::ST_RUNNING) return;

	if (job.Queue) {
		if (job.Queue!=this) {
			emFatalError("emJobQueue::StartJob: job is in a different queue");
		}
		RemoveJobFromList(job);
	}
	else {
		job.Alloc();
		job.Queue=this;
	}

	job.Prev=LastRunningJob;
	job.Next=NULL;
	if (LastRunningJob) LastRunningJob->Next=&job;
	else FirstRunningJob=&job;
	LastRunningJob=&job;

	job.State=emJob::ST_RUNNING;
	job.StateSignal.Signal(Scheduler);
}


void emJobQueue::AbortJob(emJob & job)
{
	job.State=emJob::ST_ABORTED;
	job.StateSignal.Signal(Scheduler);

	if (job.Queue) {
		if (job.Queue!=this) {
			emFatalError("emJobQueue::AbortJob: job is in a different queue");
		}
		RemoveJobFromList(job);
		job.Queue=NULL;
		job.Free();
	}
}


void emJobQueue::SucceedJob(emJob & job)
{
	job.State=emJob::ST_SUCCESS;
	job.StateSignal.Signal(Scheduler);

	if (job.Queue) {
		if (job.Queue!=this) {
			emFatalError("emJobQueue::SucceedJob: job is in a different queue");
		}
		RemoveJobFromList(job);
		job.Queue=NULL;
		job.Free();
	}
}


void emJobQueue::FailJob(emJob & job, const emString & errorText)
{
	job.State=emJob::ST_ERROR;
	job.ErrorText=errorText;
	job.StateSignal.Signal(Scheduler);

	if (job.Queue) {
		if (job.Queue!=this) {
			emFatalError("emJobQueue::FailJob: job is in a different queue");
		}
		RemoveJobFromList(job);
		job.Queue=NULL;
		job.Free();
	}
}


void emJobQueue::FailAllRunningJobs(emString errorText)
{
	while (FirstRunningJob) {
		FailJob(*FirstRunningJob,errorText);
	}
}


void emJobQueue::FailAllJobs(emString errorText)
{
	FailAllRunningJobs(errorText);
	while (FirstWaitingJob) {
		FailJob(*FirstWaitingJob,errorText);
	}
}


void emJobQueue::RemoveJobFromList(emJob & job)
{
	if (job.Prev) job.Prev->Next=job.Next;
	else if (FirstWaitingJob==&job) FirstWaitingJob=job.Next;
	else if (FirstRunningJob==&job) FirstRunningJob=job.Next;
	if (job.Next) job.Next->Prev=job.Prev;
	else if (LastWaitingJob==&job) LastWaitingJob=job.Prev;
	else if (LastRunningJob==&job) LastRunningJob=job.Prev;
	job.Prev=NULL;
	job.Next=NULL;
}


int emJobQueue::CompareJobs(emJob * job1, emJob * job2, void * context)
{
	emJobQueue * queue = (emJobQueue *)context;
	return queue->CompareForSortingOfWaitingJobs(*job1,*job2);
}
