//------------------------------------------------------------------------------
// emJob.h
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

#ifndef emJob_h
#define emJob_h

#ifndef emRef_h
#include <emCore/emRef.h>
#endif

#ifndef emSignal_h
#include <emCore/emSignal.h>
#endif

class emJobQueue;


//==============================================================================
//=================================== emJob ====================================
//==============================================================================

class emJob : public emRefTarget, public emUncopyable {

public:

	emJob(double priority=0.0);
	virtual ~emJob();

	double GetPriority() const;
	void SetPriority(double priority);

	enum StateEnum {
		ST_NOT_ENQUEUED,
		ST_WAITING,
		ST_RUNNING,
		ST_ABORTED,
		ST_SUCCESS,
		ST_ERROR
	};

	StateEnum GetState() const;

	const emSignal & GetStateSignal() const;

	const emString & GetErrorText() const;

	emJob * GetPrev() const;
	emJob * GetNext() const;

private:
	friend class emJobQueue;

	double Priority;
	StateEnum State;
	emSignal StateSignal;
	emString ErrorText;
	emJobQueue * Queue;
	emJob * Prev;
	emJob * Next;
};


//==============================================================================
//================================= emJobQueue =================================
//==============================================================================

class emJobQueue : public emUncopyable {

public:

	emJobQueue(emScheduler & scheduler);
	virtual ~emJobQueue();

	void EnqueueJob(emJob & job);

	bool IsEmpty() const;

	emJob * GetFirstWaitingJob() const;
	emJob * GetLastWaitingJob() const;
	emJob * GetFirstRunningJob() const;
	emJob * GetLastRunningJob() const;

	void InvalidateSortingOfWaitingJobs();
	virtual int CompareForSortingOfWaitingJobs(emJob & job1, emJob & job2) const;
	void UpdateSortingOfWaitingJobs();

	emJob * StartNextJob();
	void StartJob(emJob & job);
	void AbortJob(emJob & job);
	void SucceedJob(emJob & job);
	void FailJob(emJob & job, const emString & errorText);
	void FailAllRunningJobs(emString errorText);
	void FailAllJobs(emString errorText);

private:

	void RemoveJobFromList(emJob & job);
	static int CompareJobs(emJob * job1, emJob * job2, void * context);

	emScheduler & Scheduler;
	emJob * FirstWaitingJob;
	emJob * LastWaitingJob;
	emJob * FirstRunningJob;
	emJob * LastRunningJob;
	bool SortingOfWaitingJobsInvalid;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline double emJob::GetPriority() const
{
	return Priority;
}

inline emJob::StateEnum emJob::GetState() const
{
	return State;
}

inline const emSignal & emJob::GetStateSignal() const
{
	return StateSignal;
}

inline const emString & emJob::GetErrorText() const
{
	return ErrorText;
}

inline emJob * emJob::GetPrev() const
{
	return Prev;
}

inline emJob * emJob::GetNext() const
{
	return Next;
}

inline bool emJobQueue::IsEmpty() const
{
	return !FirstWaitingJob && !FirstRunningJob;
}

inline emJob * emJobQueue::GetFirstWaitingJob() const
{
	return FirstWaitingJob;
}

inline emJob * emJobQueue::GetLastWaitingJob() const
{
	return LastWaitingJob;
}

inline emJob * emJobQueue::GetFirstRunningJob() const
{
	return FirstRunningJob;
}

inline emJob * emJobQueue::GetLastRunningJob() const
{
	return LastRunningJob;
}


#endif
