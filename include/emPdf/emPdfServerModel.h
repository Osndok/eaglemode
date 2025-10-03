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

#ifndef emJob_h
#include <emCore/emJob.h>
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

	class OpenJob;

	class PdfInstance : public emRefTarget, public emUncopyable {
	public:
		virtual ~PdfInstance();
		emUInt64 GetProcRunId() const;
		int GetInstanceId() const;
		const DocumentInfo & GetDocumentInfo() const;
		int GetPageCount() const;
		const PageInfo & GetPageInfo(int page) const;
	private:
		friend class OpenJob;
		PdfInstance(emPdfServerModel & pdfServerModel);
		emCrossPtr<emPdfServerModel> PdfServerModel;
		emUInt64 ProcRunId;
		int InstanceId;
		DocumentInfo Document;
		emArray<PageInfo> Pages;
	};

	class PdfJobBase : public emJob {
	public:
		virtual ~PdfJobBase();
		const emRef<PdfInstance>& GetPdfInstance() const;
		int GetClassPriority() const;
	protected:
		friend class emPdfServerModel;
		PdfJobBase(PdfInstance * pdfInstance, bool costly,
		           int classPriority, double priority);
		void SetPdfInstance(PdfInstance * pdfInstance);
		void SetClassPriority(int classPriority);
		virtual bool Send(emPdfServerModel & mdl, emString & err) = 0;
		enum RcvRes {
			RCV_WAIT,
			RCV_CONTINUE,
			RCV_SUCCESS,
			RCV_ERROR
		};
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err) = 0;
	private:
		emRef<PdfInstance> PdfInst;
		bool Costly;
		int ClassPriority;
	};

	class OpenJob : public PdfJobBase {
	public:
		OpenJob(const emString & filePath, double priority=0.0);
		virtual ~OpenJob();
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err);
	private:
		emString FilePath;
	};

	class GetAreasJob : public PdfJobBase {
	public:
		GetAreasJob(PdfInstance & pdfInstance, int page, double priority=0.0);
		virtual ~GetAreasJob();
		const PageAreas & GetAreas() const;
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err);
	private:
		int Page;
		PageAreas Areas;
	};

	class GetSelectedTextJob : public PdfJobBase {
	public:
		GetSelectedTextJob(
			PdfInstance & pdfInstance, int page, SelectionStyle style,
			double selX1, double selY1, double selX2, double selY2,
			double priority=0.0
		);
		virtual ~GetSelectedTextJob();
		const emString & GetSelectedText() const;
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err);
	private:
		int Page;
		SelectionStyle Style;
		double SelX1, SelY1, SelX2, SelY2;
		emString SelectedText;
	};

	class RenderJob : public PdfJobBase {
	public:
		RenderJob(
			PdfInstance & pdfInstance, int page, double srcX, double srcY,
			double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
			double priority=0.0
		);
		virtual ~RenderJob();
		double GetSrcX() const;
		double GetSrcY() const;
		double GetSrcWidth() const;
		double GetSrcHeight() const;
		const emImage & GetImage() const;
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err);
		int Page;
		double SrcX, SrcY, SrcWidth, SrcHeight;
		int TgtW, TgtH;
		emImage Image;
		int ReadStage;
		int ReadPos;
		bool IsRenderSelectionJob;
	};

	class RenderSelectionJob : public RenderJob {
	public:
		RenderSelectionJob(
			PdfInstance & pdfInstance, int page, double srcX, double srcY,
			double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
			SelectionStyle style, double selX1, double selY1, double selX2,
			double selY2, double priority=0.0
		);
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
	private:
		SelectionStyle Style;
		double SelX1, SelY1, SelX2, SelY2;
	};

	void EnqueueJob(PdfJobBase & job);
	void AbortJob(PdfJobBase & job);

	void Poll(unsigned maxMillisecs);

protected:

	emPdfServerModel(emContext & context, const emString & name);
	virtual ~emPdfServerModel();

	virtual bool Cycle();

private:

	class CloseJob : public PdfJobBase {
	public:
		CloseJob(emUInt64 procRunId, int instanceId);
	protected:
		virtual bool Send(emPdfServerModel & mdl, emString & err);
		virtual RcvRes TryReceive(emPdfServerModel & mdl, emString & err);
	private:
		emUInt64 ProcRunId;
		int InstanceId;
	};

	class PdfJobQueue : public emJobQueue {
	public:
		PdfJobQueue(emScheduler & scheduler);
		virtual int CompareForSortingOfWaitingJobs(emJob & job1, emJob & job2) const;
	};

	friend PdfInstance;
	friend OpenJob;
	friend GetAreasJob;
	friend GetSelectedTextJob;
	friend RenderJob;
	friend RenderSelectionJob;
	friend CloseJob;

	void TryStartJobs();
	void TryFinishJobs();

	void WriteLineToProc(const char * str);
	emString ReadLineFromProc();
	bool TryProcIO();

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

	PdfJobQueue JobQueue;
};

inline bool emPdfServerModel::TextRect::Contains(int x, int y) const
{
	return x>=X1 && x<X2 && y>=Y1 && y<Y2;
}

inline emUInt64 emPdfServerModel::PdfInstance::GetProcRunId() const
{
	return ProcRunId;
}

inline int emPdfServerModel::PdfInstance::GetInstanceId() const
{
	return InstanceId;
}

inline const emPdfServerModel::DocumentInfo &
	emPdfServerModel::PdfInstance::GetDocumentInfo() const
{
	return Document;
}

inline int emPdfServerModel::PdfInstance::GetPageCount() const
{
	return Pages.GetCount();
}

inline const emPdfServerModel::PageInfo &
	emPdfServerModel::PdfInstance::GetPageInfo(int page) const
{
	return Pages[page];
}

inline const emRef<emPdfServerModel::PdfInstance>&
	emPdfServerModel::PdfJobBase::GetPdfInstance() const
{
	return PdfInst;
}

inline int emPdfServerModel::PdfJobBase::GetClassPriority() const
{
	return ClassPriority;
}

inline const emPdfServerModel::PageAreas &
	emPdfServerModel::GetAreasJob::GetAreas() const
{
	return Areas;
}

inline const emString &
	emPdfServerModel::GetSelectedTextJob::GetSelectedText() const
{
	return SelectedText;
}

inline double emPdfServerModel::RenderJob::GetSrcX() const
{
	return SrcX;
}

inline double emPdfServerModel::RenderJob::GetSrcY() const
{
	return SrcY;
}

inline double emPdfServerModel::RenderJob::GetSrcWidth() const
{
	return SrcWidth;
}

inline double emPdfServerModel::RenderJob::GetSrcHeight() const
{
	return SrcHeight;
}

inline const emImage & emPdfServerModel::RenderJob::GetImage() const
{
	return Image;
}


#endif
