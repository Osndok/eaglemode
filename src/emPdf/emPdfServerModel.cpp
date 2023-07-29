//------------------------------------------------------------------------------
// emPdfServerModel.cpp
//
// Copyright (C) 2011,2014,2017-2019,2022-2023 Oliver Hamann.
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

#include <emPdf/emPdfServerModel.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emList.h>


emPdfServerModel::DocumentInfo::DocumentInfo()
	: CreationDate(0),
	ModificationDate(0)
{
}


emPdfServerModel::DocumentInfo::~DocumentInfo()
{
}


emPdfServerModel::PageInfo::PageInfo()
{
	Width=1.0;
	Height=1.0;
}


emPdfServerModel::PageInfo::PageInfo(const PageInfo & pageInfo)
	: Width(pageInfo.Width),
	Height(pageInfo.Height),
	Label(pageInfo.Label)
{
}


emPdfServerModel::PageInfo::~PageInfo()
{
}


emPdfServerModel::PageInfo & emPdfServerModel::PageInfo::operator = (
	const PageInfo & pageInfo
)
{
	Width=pageInfo.Width;
	Height=pageInfo.Height;
	Label=pageInfo.Label;
	return *this;
}


emPdfServerModel::PageAreas::PageAreas()
{
	TextRects.SetTuningLevel(4);
	UriRects.SetTuningLevel(1);
	RefRects.SetTuningLevel(4);
}


emPdfServerModel::PageAreas::~PageAreas()
{
}


emRef<emPdfServerModel> emPdfServerModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emPdfServerModel,rootContext,"")
}


emPdfServerModel::JobHandle emPdfServerModel::StartOpenJob(
	const emString & filePath, PdfHandle * pdfHandleReturn,
	double priority, emEngine * listenEngine
)
{
	OpenJob * job;

	job=new OpenJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->FilePath=filePath;
	job->Instance=new PdfInstance;
	job->PdfHandleReturn=pdfHandleReturn;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


emPdfServerModel::JobHandle emPdfServerModel::StartGetAreasJob(
	PdfHandle pdfHandle, int page, PageAreas * outputAreas,
	double priority, emEngine * listenEngine
)
{
	GetAreasJob * job;

	job=new GetAreasJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->ProcRunId=((PdfInstance*)pdfHandle)->ProcRunId;
	job->InstanceId=((PdfInstance*)pdfHandle)->InstanceId;
	job->Page=page;
	job->OutputAreas=outputAreas;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


emPdfServerModel::JobHandle emPdfServerModel::StartGetSelectedTextJob(
	PdfHandle pdfHandle, int page, SelectionStyle style, double selX1,
	double selY1, double selX2, double selY2, emString * outputString,
	double priority, emEngine * listenEngine
)
{
	GetSelectedTextJob * job;

	job=new GetSelectedTextJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->ProcRunId=((PdfInstance*)pdfHandle)->ProcRunId;
	job->InstanceId=((PdfInstance*)pdfHandle)->InstanceId;
	job->Page=page;
	job->Style=style;
	job->SelX1=selX1;
	job->SelY1=selY1;
	job->SelX2=selX2;
	job->SelY2=selY2;
	job->OutputString=outputString;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


emPdfServerModel::JobHandle emPdfServerModel::StartRenderJob(
	PdfHandle pdfHandle, int page, double srcX, double srcY,
	double srcWidth, double srcHeight,  int tgtWidth, int tgtHeight,
	emImage * outputImage, double priority, emEngine * listenEngine
)
{
	RenderJob * job;

	job=new RenderJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->ProcRunId=((PdfInstance*)pdfHandle)->ProcRunId;
	job->InstanceId=((PdfInstance*)pdfHandle)->InstanceId;
	job->Page=page;
	job->SrcX=srcX;
	job->SrcY=srcY;
	job->SrcWidth=srcWidth;
	job->SrcHeight=srcHeight;
	job->TgtW=tgtWidth;
	job->TgtH=tgtHeight;
	job->OutputImage=outputImage;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


emPdfServerModel::JobHandle emPdfServerModel::StartRenderSelectionJob(
	PdfHandle pdfHandle, int page, double srcX, double srcY, double srcWidth,
	double srcHeight, int tgtWidth, int tgtHeight, SelectionStyle style,
	double selX1, double selY1, double selX2, double selY2,
	emImage * outputImage, double priority, emEngine * listenEngine
)
{
	RenderSelectionJob * job;

	job=new RenderSelectionJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->ProcRunId=((PdfInstance*)pdfHandle)->ProcRunId;
	job->InstanceId=((PdfInstance*)pdfHandle)->InstanceId;
	job->Page=page;
	job->SrcX=srcX;
	job->SrcY=srcY;
	job->SrcWidth=srcWidth;
	job->SrcHeight=srcHeight;
	job->TgtW=tgtWidth;
	job->TgtH=tgtHeight;
	job->OutputImage=outputImage;
	job->Style=style;
	job->SelX1=selX1;
	job->SelY1=selY1;
	job->SelX2=selX2;
	job->SelY2=selY2;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


void emPdfServerModel::CloseJob(JobHandle jobHandle)
{
	Job * job;

	job=(Job*)jobHandle;
	if (job->State==JS_RUNNING) {
		job->ListenEngine=NULL;
		job->Orphan=true;
	}
	else {
		if (job->State==JS_WAITING) RemoveJobFromList(job);
		delete job;
	}
}


void emPdfServerModel::ClosePdf(PdfHandle pdfHandle)
{
	CloseJobStruct * job;
	PdfInstance * inst;

	inst=(PdfInstance*)pdfHandle;
	if (inst->ProcRunId==ProcRunId) {
		job=new CloseJobStruct;
		job->ProcRunId=inst->ProcRunId;
		job->InstanceId=inst->InstanceId;
		job->Orphan=true;
		AddJobToWaitingList(job);
		WakeUp();
	}
	delete inst;
}


void emPdfServerModel::Poll(unsigned maxMillisecs)
{
	emUInt64 endTime,now;
	int flags;

	if (!FirstRunningJob && !FirstWaitingJob) {
		if (
			ProcPdfInstCount==0 &&
			Process.IsRunning() &&
			!ProcTerminating &&
			emGetClockMS()-ProcIdleClock>=5000
		) {
			emDLog("emPdfServerModel: Terminating server process");
			Process.CloseWriting();
			ProcTerminating=true;
		}
		return;
	}

	endTime=emGetClockMS()+maxMillisecs;

	if (ProcTerminating) {
		if (!Process.WaitForTermination(maxMillisecs)) return;
		ProcTerminating=false;
	}

	ProcIdleClock=emGetClockMS();

	try {
		if (!Process.IsRunning()) {
			ProcRunId++;
			ProcPdfInstCount=0;
			ReadBuf.Clear();
			WriteBuf.Clear();
			emDLog("emPdfServerModel: Starting server process");
			Process.TryStart(
				emArray<emString>(
					emGetChildPath(
						emGetInstallPath(EM_IDT_LIB,"emPdf","emPdf"),
						"emPdfServerProc"
					)
				),
				emArray<emString>(),
				NULL,
				emProcess::SF_PIPE_STDIN|
				emProcess::SF_PIPE_STDOUT|
				emProcess::SF_SHARE_STDERR|
				emProcess::SF_NO_WINDOW
			);
		}
		TryStartJobs();
		for (;;) {
			while (TryProcIO()) {
				TryFinishJobs();
				TryStartJobs();
			}
			if (!FirstRunningJob && WriteBuf.IsEmpty()) break;
			now=emGetClockMS();
			if (now>=endTime) break;
			flags=emProcess::WF_WAIT_STDOUT;
			if (!WriteBuf.IsEmpty()) flags|=emProcess::WF_WAIT_STDIN;
			Process.WaitPipes(flags,(unsigned)(endTime-now));
		}
	}
	catch (const emException & exception) {
		if (!FirstRunningJob) FailAllJobs(exception.GetText());
		else FailAllRunningJobs(exception.GetText());
		Process.SendTerminationSignal();
		ProcTerminating=true;
	}
}


emPdfServerModel::emPdfServerModel(emContext & context, const emString & name)
	: emModel(context,name)
{
	ProcRunId=0;
	ProcPdfInstCount=0;
	ProcIdleClock=0;
	ProcTerminating=false;
	ReadBuf.SetTuningLevel(4);
	WriteBuf.SetTuningLevel(4);
	FirstWaitingJob=NULL;
	LastWaitingJob=NULL;
	FirstRunningJob=NULL;
	LastRunningJob=NULL;
	SetMinCommonLifetime(10);
	SetEnginePriority(LOW_PRIORITY);
}


emPdfServerModel::~emPdfServerModel()
{
	Job * job;

	for (;;) {
		job=FirstRunningJob;
		if (!job) job=FirstWaitingJob;
		if (!job) break;
		if (!job->Orphan) {
			emFatalError("emPdfServerModel::~emPdfServerModel: Job not closed.");
		}
		RemoveJobFromList(job);
		delete job;
	}
	Process.Terminate();
}


bool emPdfServerModel::Cycle()
{
	bool busy;

	busy=emModel::Cycle();

	Poll(IsTimeSliceAtEnd()?0:10);

	if (
		FirstRunningJob || FirstWaitingJob || !WriteBuf.IsEmpty() ||
		(Process.IsRunning() && !ProcPdfInstCount)
	) busy=true;

	return busy;
}


emPdfServerModel::PdfInstance::PdfInstance()
{
	ProcRunId=0;
	InstanceId=-1;
}


emPdfServerModel::PdfInstance::~PdfInstance()
{
}


emPdfServerModel::Job::Job()
{
	State=JS_WAITING;
	Priority=0.0;
	ListenEngine=NULL;
	Orphan=false;
	Prev=NULL;
	Next=NULL;
}


emPdfServerModel::Job::~Job()
{
}


emPdfServerModel::OpenJob::OpenJob()
{
	Type=JT_OPEN_JOB;
	Instance=NULL;
	PdfHandleReturn=NULL;
}


emPdfServerModel::OpenJob::~OpenJob()
{
	if (Instance) delete Instance;
}


emPdfServerModel::GetAreasJob::GetAreasJob()
{
	Type=JT_GET_AREAS_JOB;
	ProcRunId=0;
	InstanceId=-1;
	Page=0;
	OutputAreas=NULL;
}


emPdfServerModel::GetAreasJob::~GetAreasJob()
{
}


emPdfServerModel::GetSelectedTextJob::GetSelectedTextJob()
{
	Type=JT_GET_SELECTED_TEXT_JOB;
	ProcRunId=0;
	InstanceId=-1;
	Page=0;
	Style=SEL_GLYPHS;
	SelX1=SelY1=SelX2=SelY2=0.0;
	OutputString=NULL;
}


emPdfServerModel::GetSelectedTextJob::~GetSelectedTextJob()
{
}


emPdfServerModel::RenderJob::RenderJob()
{
	Type=JT_RENDER_JOB;
	ProcRunId=0;
	InstanceId=-1;
	Page=0;
	SrcX=0.0;
	SrcY=0.0;
	SrcWidth=0.0;
	SrcHeight=0.0;
	TgtW=0;
	TgtH=0;
	OutputImage=NULL;
	ReadStage=0;
	ReadPos=0;
}


emPdfServerModel::RenderJob::~RenderJob()
{
}


emPdfServerModel::RenderSelectionJob::RenderSelectionJob()
{
	Type=JT_RENDER_SELECTION_JOB;
	Style=SEL_GLYPHS;
	SelX1=SelY1=SelX2=SelY2=0.0;
}


emPdfServerModel::RenderSelectionJob::~RenderSelectionJob()
{
}


emPdfServerModel::CloseJobStruct::CloseJobStruct()
{
	Type=JT_CLOSE_JOB;
	ProcRunId=0;
	InstanceId=-1;
}


emPdfServerModel::CloseJobStruct::~CloseJobStruct()
{
}


void emPdfServerModel::TryStartJobs()
{
	Job * job;
	int costlyJobs;

	emSortDoubleLinkedList(
		(void**)(void*)&FirstWaitingJob,
		(void**)(void*)&LastWaitingJob,
		offsetof(Job,Next),
		offsetof(Job,Prev),
		(int(*)(void*,void*,void*))CompareJobs,
		NULL
	);

	costlyJobs=0;
	for (job=FirstRunningJob; job; job=job->Next) {
		switch (job->Type) {
		case JT_OPEN_JOB:
		case JT_GET_AREAS_JOB:
		case JT_RENDER_JOB:
		case JT_RENDER_SELECTION_JOB:
			costlyJobs++;
			break;
		default:
			break;
		}
	}

	while (FirstWaitingJob && costlyJobs<4) {
		job=FirstWaitingJob;
		RemoveJobFromList(job);
		switch (job->Type) {
		case JT_OPEN_JOB:
			TryStartOpenJob((OpenJob*)job);
			costlyJobs++;
			break;
		case JT_GET_AREAS_JOB:
			TryStartGetAreasJob((GetAreasJob*)job);
			costlyJobs++;
			break;
		case JT_GET_SELECTED_TEXT_JOB:
			TryStartGetSelectedTextJob((GetSelectedTextJob*)job);
			break;
		case JT_RENDER_JOB:
			TryStartRenderJob((RenderJob*)job);
			costlyJobs++;
			break;
		case JT_RENDER_SELECTION_JOB:
			TryStartRenderSelectionJob((RenderSelectionJob*)job);
			costlyJobs++;
			break;
		case JT_CLOSE_JOB:
			TryStartCloseJob((CloseJobStruct*)job);
			break;
		}
	}
}


void emPdfServerModel::TryStartOpenJob(OpenJob * job)
{
	if (job->Orphan) {
		delete job;
	}
	else {
		WriteLineToProc(
			emString::Format(
				"open %s",
				job->FilePath.Get()
			)
		);
		AddJobToRunningList(job);
		job->State=JS_RUNNING;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartGetAreasJob(GetAreasJob * job)
{
	if (job->Orphan) {
		delete job;
	}
	else if (job->ProcRunId!=ProcRunId) {
		job->State=JS_ERROR;
		job->ErrorText="PDF server process restarted";
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else {
		WriteLineToProc(emString::Format(
			"get_areas %d %d",
			job->InstanceId,
			job->Page
		));
		AddJobToRunningList(job);
		job->State=JS_RUNNING;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartGetSelectedTextJob(GetSelectedTextJob * job)
{
	if (job->Orphan) {
		delete job;
	}
	else if (job->ProcRunId!=ProcRunId) {
		job->State=JS_ERROR;
		job->ErrorText="PDF server process restarted";
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else {
		WriteLineToProc(emString::Format(
			"get_selected_text %d %d %d %.16g %.16g %.16g %.16g",
			job->InstanceId,
			job->Page,
			job->Style,
			job->SelX1,
			job->SelY1,
			job->SelX2,
			job->SelY2
		));
		AddJobToRunningList(job);
		job->State=JS_RUNNING;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartRenderJob(RenderJob * job)
{
	if (job->Orphan) {
		delete job;
	}
	else if (job->ProcRunId!=ProcRunId) {
		job->State=JS_ERROR;
		job->ErrorText="PDF server process restarted";
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else {
		WriteLineToProc(emString::Format(
			"render %d %d %.16g %.16g %.16g %.16g %d %d",
			job->InstanceId,
			job->Page,
			job->SrcX,
			job->SrcY,
			job->SrcWidth,
			job->SrcHeight,
			job->TgtW,
			job->TgtH
		));
		AddJobToRunningList(job);
		job->State=JS_RUNNING;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartRenderSelectionJob(RenderSelectionJob * job)
{
	if (job->Orphan) {
		delete job;
	}
	else if (job->ProcRunId!=ProcRunId) {
		job->State=JS_ERROR;
		job->ErrorText="PDF server process restarted";
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else {
		WriteLineToProc(emString::Format(
			"render_selection %d %d %.16g %.16g %.16g %.16g %d %d %d %.16g %.16g %.16g %.16g",
			job->InstanceId,
			job->Page,
			job->SrcX,
			job->SrcY,
			job->SrcWidth,
			job->SrcHeight,
			job->TgtW,
			job->TgtH,
			job->Style,
			job->SelX1,
			job->SelY1,
			job->SelX2,
			job->SelY2
		));
		AddJobToRunningList(job);
		job->State=JS_RUNNING;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartCloseJob(CloseJobStruct * job)
{
	if (job->ProcRunId==ProcRunId) {
		WriteLineToProc(emString::Format(
			"close %d",
			job->InstanceId
		));
		ProcPdfInstCount--;
	}
	if (job->Orphan) {
		delete job;
	}
	else {
		job->State=JS_SUCCESS;
		if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryFinishJobs()
{
	Job * job;

	for (;;) {
		job=FirstRunningJob;
		if (!job) break;
		if (job->Type==JT_OPEN_JOB) {
			if (!TryFinishOpenJob((OpenJob*)job)) break;
		}
		else if (job->Type==JT_GET_AREAS_JOB) {
			if (!TryFinishGetAreasJob((GetAreasJob*)job)) break;
		}
		else if (job->Type==JT_GET_SELECTED_TEXT_JOB) {
			if (!TryFinishGetSelectedTextJob((GetSelectedTextJob*)job)) break;
		}
		else if (job->Type==JT_RENDER_JOB) {
			if (!TryFinishRenderJob((RenderJob*)job,false)) break;
		}
		else if (job->Type==JT_RENDER_SELECTION_JOB) {
			if (!TryFinishRenderJob((RenderJob*)job,true)) break;
		}
		else {
			emFatalError("emPdfServerModel::TryFinishJobs: illegal job in running list");
		}
	}
}


bool emPdfServerModel::TryFinishOpenJob(OpenJob * job)
{
	emString cmd,args;
	const char * p;
	double d1,d2;
	int l,i1,r,pos;

	args=ReadLineFromProc();
	if (args.IsEmpty()) return false;
	p=strchr(args.Get(),' ');
	if (p) {
		l=p-args.Get();
		cmd=args.GetSubString(0,l);
		args.Remove(0,l+1);
	}
	else {
		cmd=args;
		args.Clear();
	}

	if (cmd=="error:") {
		RemoveJobFromList(job);
		job->State=JS_ERROR;
		job->ErrorText=args;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else if (cmd=="instance:") {
		r=sscanf(args,"%d",&i1);
		if (r<1) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		job->Instance->ProcRunId=ProcRunId;
		job->Instance->InstanceId=i1;
	}
	else if (cmd=="title:") {
		job->Instance->Document.Title=Unquote(args);
	}
	else if (cmd=="author:") {
		job->Instance->Document.Author=Unquote(args);
	}
	else if (cmd=="subject:") {
		job->Instance->Document.Subject=Unquote(args);
	}
	else if (cmd=="keywords:") {
		job->Instance->Document.Keywords=Unquote(args);
	}
	else if (cmd=="creator:") {
		job->Instance->Document.Creator=Unquote(args);
	}
	else if (cmd=="producer:") {
		job->Instance->Document.Producer=Unquote(args);
	}
	else if (cmd=="creation_date:") {
		job->Instance->Document.CreationDate=atol(args.Get());
	}
	else if (cmd=="modification_date:") {
		job->Instance->Document.ModificationDate=atol(args.Get());
	}
	else if (cmd=="version:") {
		job->Instance->Document.Version=Unquote(args);
	}
	else if (cmd=="pages:") {
		r=sscanf(args,"%d",&i1);
		if (r<1) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		job->Instance->Pages.SetCount(i1);
	}
	else if (cmd=="pageinfo:") {
		r=sscanf(args,"%d %lf %lf %n",&i1,&d1,&d2,&pos);
		if (
			r<3 || pos<=0 || i1<0 ||
			i1>=job->Instance->Pages.GetCount()
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		job->Instance->Pages.GetWritable(i1).Width=d1;
		job->Instance->Pages.GetWritable(i1).Height=d2;
		job->Instance->Pages.GetWritable(i1).Label=Unquote(args.Get()+pos);
	}
	else if (cmd=="ok") {
		RemoveJobFromList(job);
		job->State=JS_SUCCESS;
		job->Instance->ProcRunId=ProcRunId;
		if (job->Orphan) {
			delete job;
		}
		else {
			if (job->PdfHandleReturn) {
				*job->PdfHandleReturn=job->Instance;
				job->Instance=NULL;
				ProcPdfInstCount++;
			}
			if (job->ListenEngine) job->ListenEngine->WakeUp();
		}
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}

	return true;
}


bool emPdfServerModel::TryFinishGetAreasJob(GetAreasJob * job)
{
	PageAreas * areas;
	emString cmd,args;
	const char * p;
	int l,r,x1,y1,x2,y2,type,pos;

	args=ReadLineFromProc();
	if (args.IsEmpty()) return false;
	p=strchr(args.Get(),' ');
	if (p) {
		l=p-args.Get();
		cmd=args.GetSubString(0,l);
		args.Remove(0,l+1);
	}
	else {
		cmd=args;
		args.Clear();
	}

	if (cmd=="error:") {
		RemoveJobFromList(job);
		job->State=JS_ERROR;
		job->ErrorText=args;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else if (cmd=="rect:") {
		r=sscanf(args,"%d %d %d %d %d%n",&x1,&y1,&x2,&y2,&type,&pos);
		if (
			r<5 || pos<=0 || type<0 || type>2 ||
			(type!=0 && args[pos]!=' ')
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		areas=job->OutputAreas;
		if (type==0 && areas) {
			areas->TextRects.AddNew();
			TextRect & tr=areas->TextRects.GetWritable(areas->TextRects.GetCount()-1);
			tr.X1=x1;
			tr.Y1=y1;
			tr.X2=x2;
			tr.Y2=y2;
		}
		else if (type==1 && areas) {
			areas->UriRects.AddNew();
			UriRect & ur=areas->UriRects.GetWritable(areas->UriRects.GetCount()-1);
			ur.X1=x1;
			ur.Y1=y1;
			ur.X2=x2;
			ur.Y2=y2;
			ur.Uri=Unquote(args.Get()+pos+1);
		}
		else if (type==2 && areas) {
			areas->RefRects.AddNew();
			RefRect & rr=areas->RefRects.GetWritable(areas->RefRects.GetCount()-1);
			rr.X1=x1;
			rr.Y1=y1;
			rr.X2=x2;
			rr.Y2=y2;
			r=sscanf(args.Get()+pos+1,"%d %d",&rr.TargetPage,&rr.TargetY);
			if (r<2) {
				throw emException("PDF server protocol error (%d)",__LINE__);
			}
		}
	}
	else if (cmd=="ok") {
		RemoveJobFromList(job);
		job->State=JS_SUCCESS;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}

	return true;
}


bool emPdfServerModel::TryFinishGetSelectedTextJob(GetSelectedTextJob * job)
{
	emString cmd,args;
	const char * p;
	int l;

	args=ReadLineFromProc();
	if (args.IsEmpty()) return false;
	p=strchr(args.Get(),' ');
	if (p) {
		l=p-args.Get();
		cmd=args.GetSubString(0,l);
		args.Remove(0,l+1);
	}
	else {
		cmd=args;
		args.Clear();
	}

	if (cmd=="error:") {
		RemoveJobFromList(job);
		job->State=JS_ERROR;
		job->ErrorText=args;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	else if (cmd=="selected_text:") {
		RemoveJobFromList(job);
		job->State=JS_SUCCESS;
		if (job->Orphan) {
			delete job;
		}
		else {
			if (job->OutputString) *job->OutputString=Unquote(args);
			if (job->ListenEngine) job->ListenEngine->WakeUp();
		}
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}

	return true;
}


bool emPdfServerModel::TryFinishRenderJob(
	RenderJob * job, bool isRenderSelectionJob
)
{
	int len,total,type,channels,width,height,maxColor;
	emString line;
	const char * p;
	bool progress;

	progress=false;
	if (job->ReadStage==0) {
		if (ReadBuf.IsEmpty()) return progress;
		if (ReadBuf[0]!='P') {
			line=ReadLineFromProc();
			if (line.IsEmpty()) return progress;
			progress=true;
			p="error: ";
			len=strlen(p);
			if (line.GetSubString(0,len)!=p) {
				throw emException("PDF server protocol error (%d)",__LINE__);
			}
			line.Remove(0,len);
			RemoveJobFromList(job);
			job->State=JS_ERROR;
			job->ErrorText=line;
			if (job->Orphan) delete job;
			else if (job->ListenEngine) job->ListenEngine->WakeUp();
			return progress;
		}
		len=TryParsePnmHeader(
			ReadBuf.Get(),ReadBuf.GetCount(),&type,&width,&height,&maxColor
		);
		if (len<=0) return progress;
		emDLog("emPdfServerModel: Receiving: P%c %d %d %d ...",type,width,height,maxColor);
		ReadBuf.Remove(0,len);
		if (
			type!=(isRenderSelectionJob?'X':'6') ||
			width!=job->TgtW || height!=job->TgtH || maxColor!=255
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		job->ReadStage=1;
		progress=true;
	}

	if (ReadBuf.IsEmpty()) return progress;
	channels=isRenderSelectionJob?2:3;
	total=job->TgtW*job->TgtH*channels;
	len=ReadBuf.GetCount();
	if (len>total-job->ReadPos) len=total-job->ReadPos;
	if (!job->Orphan) {
		if (
			job->OutputImage->GetWidth()!=job->TgtW ||
			job->OutputImage->GetHeight()!=job->TgtH ||
			job->OutputImage->GetChannelCount()!=channels
		) {
			job->OutputImage->Setup(job->TgtW,job->TgtH,channels);
		}
		memcpy(
			job->OutputImage->GetWritableMap()+job->ReadPos,
			ReadBuf.Get(),
			len
		);
	}
	ReadBuf.Remove(0,len);
	job->ReadPos+=len;
	progress=true;
	if (job->ReadPos>=total) {
		RemoveJobFromList(job);
		job->State=JS_SUCCESS;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
	return progress;
}


void emPdfServerModel::FailAllRunningJobs(emString errorText)
{
	Job * job;

	for (;;) {
		job=FirstRunningJob;
		if (!job) break;
		RemoveJobFromList(job);
		job->State=JS_ERROR;
		job->ErrorText=errorText;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::FailAllJobs(emString errorText)
{
	Job * job;

	FailAllRunningJobs(errorText);
	for (;;) {
		job=FirstWaitingJob;
		if (!job) break;
		RemoveJobFromList(job);
		job->State=JS_ERROR;
		job->ErrorText=errorText;
		if (job->Orphan) delete job;
		else if (job->ListenEngine) job->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::WriteLineToProc(const char * str)
{
	emDLog("emPdfServerModel: Sending: %s",str);
	WriteBuf.Add(str,strlen(str));
	WriteBuf.Add((char)'\n');
}


emString emPdfServerModel::ReadLineFromProc()
{
	emString res;
	char * p;
	int l;

	if (!ReadBuf.IsEmpty()) {
		p=(char*)memchr(ReadBuf.Get(),'\n',ReadBuf.GetCount());
		if (p) {
			l=p-ReadBuf.Get();
			res=emString(ReadBuf.Get(),l);
			ReadBuf.Remove(0,l+1);
		}
	}
	if (!res.IsEmpty()) emDLog("emPdfServerModel: Receiving: %s",res.Get());
	return res;
}


bool emPdfServerModel::TryProcIO()
{
	char buf[256];
	bool progress;
	int r;

	progress=false;

	if (!WriteBuf.IsEmpty()) {
		r=Process.TryWrite(WriteBuf.Get(),WriteBuf.GetCount());
		if (r<0) throw emException("PDF server process died unexpectedly.");
		if (r>0) {
			WriteBuf.Remove(0,r);
			progress=true;
		}
	}

	while (ReadBuf.GetCount()<65536) {
		r=Process.TryRead(buf,sizeof(buf));
		if (r<0) throw emException("PDF server process died unexpectedly.");
		if (r==0) break;
		ReadBuf.Add(buf,r);
		progress=true;
	}

	return progress;
}


int emPdfServerModel::CompareJobs(Job * job1, Job * job2, void * context)
{
	int tp1, tp2;
	double d;

	tp1=GetJobTypePriority(job1->Type);
	tp2=GetJobTypePriority(job2->Type);
	if (tp1!=tp2) return tp2-tp1;

	d=job2->Priority-job1->Priority;
	return d>0.0 ? 1 : d<0.0 ? -1 : 0;
}


int emPdfServerModel::GetJobTypePriority(JobType type)
{
	switch (type) {
		case JT_OPEN_JOB:              return 3;
		case JT_GET_AREAS_JOB:         return 2;
		case JT_GET_SELECTED_TEXT_JOB: return 5;
		case JT_RENDER_JOB:            return 1;
		case JT_RENDER_SELECTION_JOB:  return 4;
		case JT_CLOSE_JOB:             return 6;
		default:                       return 0;
	}
}


void emPdfServerModel::AddJobToWaitingList(Job * job)
{
	job->Prev=LastWaitingJob;
	job->Next=NULL;
	if (LastWaitingJob) LastWaitingJob->Next=job;
	else FirstWaitingJob=job;
	LastWaitingJob=job;
}


void emPdfServerModel::AddJobToRunningList(Job * job)
{
	job->Prev=LastRunningJob;
	job->Next=NULL;
	if (LastRunningJob) LastRunningJob->Next=job;
	else FirstRunningJob=job;
	LastRunningJob=job;
}


void emPdfServerModel::RemoveJobFromList(Job * job)
{
	if (job->Prev) job->Prev->Next=job->Next;
	else if (FirstWaitingJob==job) FirstWaitingJob=job->Next;
	else if (FirstRunningJob==job) FirstRunningJob=job->Next;
	if (job->Next) job->Next->Prev=job->Prev;
	else if (LastWaitingJob==job) LastWaitingJob=job->Prev;
	else if (LastRunningJob==job) LastRunningJob=job->Prev;
	job->Prev=NULL;
	job->Next=NULL;
}


int emPdfServerModel::TryParsePnmHeader(
	const char * src, int len, int * pType, int * pWidth,
	int * pHeight, int * pMaxColor
)
{
	int val[3];
	int pos;
	int i,c;

	pos=0;
	*pType=0;
	*pWidth=0;
	*pHeight=0;
	*pMaxColor=0;
	if (pos>=len) return 0;
	if (src[pos++]!='P') {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}
	if (pos>=len) return 0;
	*pType=(unsigned char)src[pos++];
	for (i=0; i<3; i++) {
		for (;;) {
			if (pos>=len) return 0;
			c=(unsigned char)src[pos++];
			if (c>='0' && c<='9') break;
			if (c=='#') {
				while (pos<len && src[pos++]!='\n');
			}
			else if (c>32) {
				throw emException("PDF server protocol error (%d)",__LINE__);
			}
		}
		val[i]=c-'0';
		for (;;) {
			if (pos>=len) return 0;
			c=(unsigned char)src[pos++];
			if (c<'0' || c>'9') break;
			val[i]=val[i]*10+(c-'0');
		}
	}
	if (c>32) {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}
	*pWidth=val[0];
	*pHeight=val[1];
	*pMaxColor=val[2];
	return pos;
}


emString emPdfServerModel::Unquote(const char * str)
{
	emString res;
	char c;

	for (;;) {
		c=*str++;
		if (!c || c=='"') break;
	}
	if (c=='"') {
		for (;;) {
			c=*str++;
			if (!c) break;
			if (c=='"') break;
			if (c=='\\') {
				c=*str++;
				if (!c) break;
				if (c=='n') c='\n';
				else if (c=='r') c='\r';
				else if (c=='t') c='\t';
			}
			res+=c;
		}
	}
	return res;
}
