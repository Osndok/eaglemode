//------------------------------------------------------------------------------
// emPdfServerModel.h
//
// Copyright (C) 2011 Oliver Hamann.
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

#ifndef emPdfServerModel_h
#define emPdfServerModel_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif


class emPdfServerModel : public emModel {

public:

	static emRef<emPdfServerModel> Acquire(emRootContext & rootContext);

	typedef void * JobHandle;
	typedef void * PdfHandle;

	JobHandle StartOpenJob(
		const emString & filePath, PdfHandle * pdfHandleReturn,
		double priority=0.0, emEngine * listenEngine=NULL
	);

	JobHandle StartRenderJob(
		PdfHandle pdfHandle, int page, double srcX, double srcY,
		double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
		emColor bgColor, emImage * outputImage, double priority=0.0,
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

	int GetPageCount(PdfHandle pdfHandle) const;

	struct PageInfo {
		PageInfo();
		~PageInfo();
		double Width;
		double Height;
		emString Label;
	};

	const PageInfo & GetPageInfo(PdfHandle pdfHandle, int page) const;

	void ClosePdf(PdfHandle pdfHandle);

	void Poll(unsigned maxMilliSecs);

protected:

	emPdfServerModel(emContext & context, const emString & name);
	virtual ~emPdfServerModel();

	virtual bool Cycle();

private:

	struct PdfInstance {
		PdfInstance();
		~PdfInstance();
		emUInt64 ProcRunId;
		int InstanceId;
		emArray<PageInfo> Pages;
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
		PdfInstance * Instance;
		PdfHandle * PdfHandleReturn;
	};

	struct RenderJob : Job {
		RenderJob();
		virtual ~RenderJob();
		emUInt64 ProcRunId;
		int InstanceId;
		int Page;
		double SrcX, SrcY, SrcWidth, SrcHeight;
		emColor BgColor;
		emImage * OutputImage;
		int TgtW, TgtH;
		int ReadStage;
		int ReadPos;
	};

	struct CloseJobStruct : Job {
		CloseJobStruct();
		virtual ~CloseJobStruct();
		emUInt64 ProcRunId;
		int InstanceId;
	};

	void TryStartJobs() throw(emString);
	void TryStartOpenJob(OpenJob * openJob) throw(emString);
	void TryStartRenderJob(RenderJob * renderJob) throw(emString);
	void TryStartCloseJob(CloseJobStruct * closeJob) throw(emString);

	void TryFinishJobs() throw(emString);
	bool TryFinishOpenJob(OpenJob * job) throw(emString);
	bool TryFinishRenderJob(RenderJob * job) throw(emString);

	void FailAllRunningJobs(emString errorText);
	void FailAllJobs(emString errorText);

	void WriteLineToProc(const char * str);
	emString ReadLineFromProc();
	bool TryProcIO() throw(emString);

	Job * SearchBestNextJob() const;

	void AddJobToWaitingList(Job * job);
	void AddJobToRunningList(Job * job);
	void RemoveJobFromList(Job * job);

	static int TryParsePnmHeader(
		const char * src, int len, int * pType, int * pWidth,
		int * pHeight, int * pMaxColor
	);
	static emString Unquote(const char * str);

	emProcess Process;
	emUInt64 ProcRunId;
	emUInt64 ProcPdfInstCount;
	emUInt64 ProcIdleClock;
	bool ProcTerminating;
	emArray<char> ReadBuf;
	emArray<char> WriteBuf;

	Job * FirstWaitingJob;
	Job * LastWaitingJob;
	Job * FirstRunningJob;
	Job * LastRunningJob;
};

inline void emPdfServerModel::SetJobPriority(JobHandle jobHandle, double priority)
{
	((Job*)jobHandle)->Priority=priority;
}

inline void emPdfServerModel::SetJobListenEngine(JobHandle jobHandle, emEngine * listenEngine)
{
	((Job*)jobHandle)->ListenEngine=listenEngine;
}

inline emPdfServerModel::JobState emPdfServerModel::GetJobState(JobHandle jobHandle) const
{
	return ((Job*)jobHandle)->State;
}

inline const emString & emPdfServerModel::GetJobErrorText(JobHandle jobHandle) const
{
	return ((Job*)jobHandle)->ErrorText;
}

inline int emPdfServerModel::GetPageCount(PdfHandle pdfHandle) const
{
	return ((PdfInstance*)pdfHandle)->Pages.GetCount();
}

inline const emPdfServerModel::PageInfo & emPdfServerModel::GetPageInfo(
	PdfHandle pdfHandle, int page
) const
{
	return ((PdfInstance*)pdfHandle)->Pages[page];
}


#endif
