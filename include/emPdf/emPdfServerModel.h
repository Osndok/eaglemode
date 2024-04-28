//------------------------------------------------------------------------------
// emPdfServerModel.h
//
// Copyright (C) 2011,2014,2018,2022-2024 Oliver Hamann.
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

	typedef void * JobHandle;
	typedef void * PdfHandle;

	enum SelectionStyle {
		SEL_GLYPHS=0,
		SEL_WORDS =1,
		SEL_LINES =2
	};

	enum JobState {
		JS_WAITING,
		JS_RUNNING,
		JS_ERROR,
		JS_SUCCESS
	};

	struct DocumentInfo {
		DocumentInfo();
		~DocumentInfo();
		emString Title;
		emString Author;
		emString Subject;
		emString Keywords;
		emString Creator;
		emString Producer;
		time_t CreationDate;
		time_t ModificationDate;
		emString Version;
	};

	struct PageInfo {
		PageInfo();
		PageInfo(const PageInfo & pageInfo);
		~PageInfo();
		PageInfo & operator = (const PageInfo & pageInfo);
		double Width;
		double Height;
		emString Label;
	};

	struct TextRect {
		int X1,Y1,X2,Y2;
		bool Contains(int x, int y) const;
	};

	struct UriRect : TextRect {
		emString Uri;
	};

	struct RefRect : TextRect {
		int TargetPage;
		int TargetY;
	};

	struct PageAreas {
		PageAreas();
		PageAreas(const PageAreas & pageAreas);
		~PageAreas();
		PageAreas & operator = (const PageAreas & pageAreas);
		emArray<TextRect> TextRects;
		emArray<UriRect> UriRects;
		emArray<RefRect> RefRects;
	};

	static emRef<emPdfServerModel> Acquire(emRootContext & rootContext);

	JobHandle StartOpenJob(
		const emString & filePath, PdfHandle * pdfHandleReturn,
		double priority=0.0, emEngine * listenEngine=NULL
	);

	JobHandle StartGetAreasJob(
		PdfHandle pdfHandle, int page, PageAreas * outputAreas,
		double priority=0.0, emEngine * listenEngine=NULL
	);

	JobHandle StartGetSelectedTextJob(
		PdfHandle pdfHandle, int page, SelectionStyle style,
		double selX1, double selY1, double selX2, double selY2,
		emString * outputString, double priority=0.0,
		emEngine * listenEngine=NULL
	);

	JobHandle StartRenderJob(
		PdfHandle pdfHandle, int page, double srcX, double srcY,
		double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
		emImage * outputImage, double priority=0.0,
		emEngine * listenEngine=NULL
	);

	JobHandle StartRenderSelectionJob(
		PdfHandle pdfHandle, int page, double srcX, double srcY,
		double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
		SelectionStyle style, double selX1, double selY1, double selX2,
		double selY2, emImage * outputImage, double priority=0.0,
		emEngine * listenEngine=NULL
	);

	void SetJobPriority(JobHandle jobHandle, double priority);

	void SetJobListenEngine(JobHandle jobHandle, emEngine * listenEngine);

	JobState GetJobState(JobHandle jobHandle) const;

	const emString & GetJobErrorText(JobHandle jobHandle) const;

	void CloseJob(JobHandle jobHandle);

	const DocumentInfo & GetDocumentInfo(PdfHandle pdfHandle) const;

	int GetPageCount(PdfHandle pdfHandle) const;

	const PageInfo & GetPageInfo(PdfHandle pdfHandle, int page) const;

	void ClosePdf(PdfHandle pdfHandle);

	void Poll(unsigned maxMillisecs);

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
		DocumentInfo Document;
		emArray<PageInfo> Pages;
	};

	enum JobType {
		JT_OPEN_JOB,
		JT_GET_AREAS_JOB,
		JT_GET_SELECTED_TEXT_JOB,
		JT_RENDER_JOB,
		JT_RENDER_SELECTION_JOB,
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

	struct GetAreasJob : Job {
		GetAreasJob();
		virtual ~GetAreasJob();
		emUInt64 ProcRunId;
		int InstanceId;
		int Page;
		PageAreas * OutputAreas;
	};

	struct GetSelectedTextJob : Job {
		GetSelectedTextJob();
		virtual ~GetSelectedTextJob();
		emUInt64 ProcRunId;
		int InstanceId;
		int Page;
		SelectionStyle Style;
		double SelX1, SelY1, SelX2, SelY2;
		emString * OutputString;
	};

	struct RenderJob : Job {
		RenderJob();
		virtual ~RenderJob();
		emUInt64 ProcRunId;
		int InstanceId;
		int Page;
		double SrcX, SrcY, SrcWidth, SrcHeight;
		int TgtW, TgtH;
		emImage * OutputImage;
		int ReadStage;
		int ReadPos;
	};

	struct RenderSelectionJob : RenderJob {
		RenderSelectionJob();
		virtual ~RenderSelectionJob();
		SelectionStyle Style;
		double SelX1, SelY1, SelX2, SelY2;
	};

	struct CloseJobStruct : Job {
		CloseJobStruct();
		virtual ~CloseJobStruct();
		emUInt64 ProcRunId;
		int InstanceId;
	};

	void TryStartJobs();
	void TryStartOpenJob(OpenJob * job);
	void TryStartGetAreasJob(GetAreasJob * job);
	void TryStartGetSelectedTextJob(GetSelectedTextJob * job);
	void TryStartRenderJob(RenderJob * job);
	void TryStartRenderSelectionJob(RenderSelectionJob * job);
	void TryStartCloseJob(CloseJobStruct * job);

	void TryFinishJobs();
	bool TryFinishOpenJob(OpenJob * job);
	bool TryFinishGetAreasJob(GetAreasJob * job);
	bool TryFinishGetSelectedTextJob(GetSelectedTextJob * job);
	bool TryFinishRenderJob(RenderJob * job, bool isRenderSelectionJob);

	void FailAllRunningJobs(emString errorText);
	void FailAllJobs(emString errorText);

	void WriteLineToProc(const char * str);
	emString ReadLineFromProc();
	bool TryProcIO();

	static int CompareJobs(Job * job1, Job * job2, void * context);
	static int GetJobTypePriority(JobType type);

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

inline bool emPdfServerModel::TextRect::Contains(int x, int y) const
{
	return x>=X1 && x<X2 && y>=Y1 && y<Y2;
}

inline void emPdfServerModel::SetJobPriority(JobHandle jobHandle, double priority)
{
	((Job*)jobHandle)->Priority=priority;
}

inline void emPdfServerModel::SetJobListenEngine(
	JobHandle jobHandle, emEngine * listenEngine
)
{
	((Job*)jobHandle)->ListenEngine=listenEngine;
}

inline emPdfServerModel::JobState emPdfServerModel::GetJobState(
	JobHandle jobHandle
) const
{
	return ((Job*)jobHandle)->State;
}

inline const emString & emPdfServerModel::GetJobErrorText(JobHandle jobHandle) const
{
	return ((Job*)jobHandle)->ErrorText;
}

inline const emPdfServerModel::DocumentInfo & emPdfServerModel::GetDocumentInfo(
	PdfHandle pdfHandle
) const
{
	return ((PdfInstance*)pdfHandle)->Document;
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
