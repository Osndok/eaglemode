//------------------------------------------------------------------------------
// emSvgServerModel.h
//
// Copyright (C) 2010 Oliver Hamann.
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

#ifndef emSvgServerModel_h
#define emSvgServerModel_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif


class emSvgServerModel : public emModel {

public:

	static emRef<emSvgServerModel> Acquire(emRootContext & rootContext);

	typedef void * JobHandle;
	typedef void * SvgHandle;

	JobHandle StartOpenJob(
		const emString & filePath, SvgHandle * svgHandleReturn,
		double priority=0.0, emEngine * listenEngine=NULL
	);

	JobHandle StartRenderJob(
		SvgHandle svgHandle, double srcX, double srcY, double srcWidth,
		double srcHeight, emColor bgColor, emImage * outputImage,
		double priority=0.0, emEngine * listenEngine=NULL
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

	double GetSvgWidth(SvgHandle svgHandle) const;
	double GetSvgHeight(SvgHandle svgHandle) const;
	const emString & GetSvgTitle(SvgHandle svgHandle) const;
	const emString & GetSvgDescription(SvgHandle svgHandle) const;

	void CloseSvg(SvgHandle svgHandle);

	void Poll(unsigned maxMilliSecs);

protected:

	emSvgServerModel(emContext & context, const emString & name);
	virtual ~emSvgServerModel();

	virtual bool Cycle();

private:

	struct SvgInstance {
		SvgInstance();
		~SvgInstance();
		emUInt64 ProcRunId;
		int InstanceId;
		double Width;
		double Height;
		emString Title;
		emString Description;
	};

	enum JobType {
		JT_OPEN_JOB,
		JT_RENDER_JOB,
		JT_CLOSE_JOB
	};

	struct Job {
		Job();
		virtual ~Job();
		JobType Type;
		JobState State;
		emString ErrorText;
		double Priority;
		emEngine * ListenEngine;
		bool Orphan;
		Job * Prev;
		Job * Next;
	};

	struct OpenJob : Job {
		OpenJob();
		virtual ~OpenJob();
		emString FilePath;
		SvgHandle * SvgHandleReturn;
	};

	struct RenderJob : Job {
		RenderJob();
		virtual ~RenderJob();
		emUInt64 ProcRunId;
		int InstanceId;
		double SrcX, SrcY, SrcWidth, SrcHeight;
		emColor BgColor;
		emImage * OutputImage;
		int TgtW, TgtH;
		int ShmOffset;
	};

	struct CloseJobStruct : Job {
		CloseJobStruct();
		virtual ~CloseJobStruct();
		emUInt64 ProcRunId;
		int InstanceId;
	};

	void TryStartJobs() throw(emString);
	void TryStartOpenJob(OpenJob * openJob) throw(emString);
	bool TryStartRenderJob(RenderJob * renderJob) throw(emString);
	void TryStartCloseJob(CloseJobStruct * closeJob) throw(emString);

	void TryFinishJobs() throw(emString);
	void TryFinishOpenJob(OpenJob * openJob, const char * args) throw(emString);
	void TryFinishRenderJob(RenderJob * renderJob) throw(emString);

	void TryWriteAttachShm() throw(emString);

	void FailAllRunningJobs(emString errorText);
	void FailAllJobs(emString errorText);

	void WriteLineToProc(const char * str);
	emString ReadLineFromProc();
	bool TryProcIO() throw(emString);

	void TryAllocShm(int size) throw(emString);
	void FreeShm();

	Job * SearchBestNextJob() const;

	void AddJobToWaitingList(Job * job);
	void AddJobToRunningList(Job * job);
	void RemoveJobFromList(Job * job);

	emProcess Process;
	emUInt64 ProcRunId;
	emUInt64 ProcSvgInstCount;
	emUInt64 ProcIdleClock;
	bool ProcTerminating;
	emArray<char> ReadBuf;
	emArray<char> WriteBuf;

	Job * FirstWaitingJob;
	Job * LastWaitingJob;
	Job * FirstRunningJob;
	Job * LastRunningJob;

	int ShmSize;
	int ShmId;
	emByte * ShmPtr;
	int ShmAllocBegin;
	int ShmAllocEnd;

	static const int MinShmSize;
};

inline void emSvgServerModel::SetJobPriority(JobHandle jobHandle, double priority)
{
	((Job*)jobHandle)->Priority=priority;
}

inline void emSvgServerModel::SetJobListenEngine(JobHandle jobHandle, emEngine * listenEngine)
{
	((Job*)jobHandle)->ListenEngine=listenEngine;
}

inline emSvgServerModel::JobState emSvgServerModel::GetJobState(JobHandle jobHandle) const
{
	return ((Job*)jobHandle)->State;
}

inline const emString & emSvgServerModel::GetJobErrorText(JobHandle jobHandle) const
{
	return ((Job*)jobHandle)->ErrorText;
}

inline double emSvgServerModel::GetSvgWidth(SvgHandle svgHandle) const
{
	return ((SvgInstance*)svgHandle)->Width;
}

inline double emSvgServerModel::GetSvgHeight(SvgHandle svgHandle) const
{
	return ((SvgInstance*)svgHandle)->Height;
}

inline const emString & emSvgServerModel::GetSvgTitle(SvgHandle svgHandle) const
{
	return ((SvgInstance*)svgHandle)->Title;
}

inline const emString & emSvgServerModel::GetSvgDescription(SvgHandle svgHandle) const
{
	return ((SvgInstance*)svgHandle)->Description;
}


#endif
