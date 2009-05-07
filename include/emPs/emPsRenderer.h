//------------------------------------------------------------------------------
// emPsRenderer.h
//
// Copyright (C) 2006-2009 Oliver Hamann.
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

#ifndef emPsRenderer_h
#define emPsRenderer_h

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif

#ifndef emPriSchedAgent_h
#include <emCore/emPriSchedAgent.h>
#endif

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emPsDocument_h
#include <emPs/emPsDocument.h>
#endif


class emPsRenderer : public emModel {

public:

	static emRef<emPsRenderer> Acquire(emRootContext & rootContext);

	typedef void * JobHandle;

	JobHandle StartJob(
		const emPsDocument & document, int pageIndex,
		emImage & outputImage, double priority=0.0,
		emEngine * listenEngine=NULL
	);

	void SetJobPriority(JobHandle jobHandle, double priority);

	void SetJobListenEngine(JobHandle jobHandle, emEngine * listenEngine);

	enum JobState {
		JS_WAITING,
		JS_RUNNING,
		JS_ERROR,
		JS_SUCCESS
	};
	JobState GetJobState(JobHandle jobHandle) const;

	const emString & GetJobErrorText(JobHandle jobHandle) const;

	void CloseJob(JobHandle jobHandle);

protected:

	emPsRenderer(emContext & context, const emString & name);
	virtual ~emPsRenderer();

	virtual bool Cycle();

private:

	struct Job {
		emPsDocument Document;
		int PageIndex;
		emImage * Image;
		double Priority;
		emEngine * ListenEngine;
		JobState State;
		emString ErrorText;
		Job * Prev;
		Job * Next;
	};

	enum MainStateType {
		COLD_WAIT_JOB,
		COLD_WAIT_ACCESS,
		PREPARE_PROCESS,
		RUN_JOB,
		HOT_WAIT_JOB,
		HOT_WAIT_ACCESS,
		QUIT_PROCESS
	};

	void AddToJobList(Job * job);
	void RemoveFromJobList(Job * job);

	Job * SearchBestJob();
	Job * SearchBestSameDocJob();

	void SetJobState(Job * job, JobState state, emString errorText="");
	void FailCurrentJob(emString errorMessage);
	void FailDocJobs(emString errorMessage);
	void FailAllJobs(emString errorMessage);

	void UpdatePSPriority();

	void TryStartProcess() throw(emString);

	void PrepareWritingStartup();
	void PrepareWritingPage();
	bool TryWrite() throw(emString);
	bool IsWritingFinished() const;

	void PrepareReadingStartup();
	void PrepareReadingPage();
	bool TryRead() throw(emString);
	int ParseImageHeader(const char * buf, int len);
	static int ParseImageDecimal(const char * buf, int len, int * pNumber);
	int ParseImageData(const char * buf, int len);
	bool IsReadingFinished() const;

	class PSAgentClass : public emPriSchedAgent {
	public:
		PSAgentClass(emPsRenderer & interpreter);
	protected:
		virtual void GotAccess();
	private:
		emPsRenderer & Renderer;
	};
	friend class PSAgentClass;

	emProcess Process;
	emTimer Timer;

	PSAgentClass PSAgent;
	bool PSPriorityValid;

	Job * FirstJob;
	Job * LastJob;

	MainStateType MainState;

	Job * CurrentJob;
	emPsDocument CurrentDocument;
	int CurrentPageIndex;

	enum WriterStateType {
		WRITING_STARTUP,
		WRITING_PAGE_SIZE,
		WRITING_PAGE,
		WRITING_SYNC,
		WRITING_FINISHED
	};
	WriterStateType WriterState;
	emString WriteCommand;
	int WriterPos;

	enum ReaderStateType {
		READING_IMAGE_HEADER,
		READING_IMAGE_DATA,
		READING_SYNC,
		READING_FINISHED
	};
	ReaderStateType ReaderState;
	char ReadBuffer[131072];
	int ReadBufferFill;
	int RdSyncSearchPos;
	int RdImgFormat,RdImgW,RdImgH,RdImgMaxVal,RdImgX,RdImgY;
	bool RdImgDone;

	static const char * const SyncString;
};

inline void emPsRenderer::SetJobListenEngine(
	JobHandle jobHandle, emEngine * listenEngine
)
{
	((Job*)jobHandle)->ListenEngine=listenEngine;
}

inline emPsRenderer::JobState emPsRenderer::GetJobState(
	JobHandle jobHandle
) const
{
	return ((Job*)jobHandle)->State;
}

inline const emString & emPsRenderer::GetJobErrorText(
	JobHandle jobHandle
) const
{
	return ((Job*)jobHandle)->ErrorText;
}

inline bool emPsRenderer::IsWritingFinished() const
{
	return WriterState==WRITING_FINISHED;
}

inline bool emPsRenderer::IsReadingFinished() const
{
	return ReaderState==READING_FINISHED;
}


#endif
