//------------------------------------------------------------------------------
// emPdfServerModel.cpp
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

#include <emPdf/emPdfServerModel.h>
#include <emCore/emInstallInfo.h>


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


emPdfServerModel::JobHandle emPdfServerModel::StartRenderJob(
	PdfHandle pdfHandle, int page, double srcX, double srcY,
	double srcWidth, double srcHeight,  int tgtWidth, int tgtHeight,
	emColor bgColor, emImage * outputImage, double priority,
	emEngine * listenEngine
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
	job->BgColor=bgColor;
	job->OutputImage=outputImage;
	job->TgtW=tgtWidth;
	job->TgtH=tgtHeight;
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


emPdfServerModel::PageInfo::PageInfo()
{
	Width=1.0;
	Height=1.0;
}


emPdfServerModel::PageInfo::~PageInfo()
{
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


void emPdfServerModel::Poll(unsigned maxMilliSecs)
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

	endTime=emGetClockMS()+maxMilliSecs;

	if (ProcTerminating) {
		if (!Process.WaitForTermination(maxMilliSecs)) return;
		ProcTerminating=false;
	}

	ProcIdleClock=emGetClockMS();

	try {
		if (!Process.IsRunning()) {
			ProcRunId++;
			ProcPdfInstCount=0;
			ReadBuf.Empty();
			WriteBuf.Empty();
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
				emProcess::SF_SHARE_STDERR
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
	catch (emString errorMessage) {
		if (!FirstRunningJob) FailAllJobs(errorMessage);
		else FailAllRunningJobs(errorMessage);
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
	BgColor=0;
	OutputImage=NULL;
	TgtW=0;
	TgtH=0;
	ReadStage=0;
	ReadPos=0;
}


emPdfServerModel::RenderJob::~RenderJob()
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


void emPdfServerModel::TryStartJobs() throw(emString)
{
	Job * job;
	int n;

	for (;;) {
		for (n=0, job=FirstRunningJob; job; job=job->Next) {
			if (job->Type!=JT_CLOSE_JOB) n++;
		}
		if (n>=4) break;
		job=SearchBestNextJob();
		if (!job) break;
		switch (job->Type) {
		case JT_OPEN_JOB:
			TryStartOpenJob((OpenJob*)job);
			break;
		case JT_RENDER_JOB:
			TryStartRenderJob((RenderJob*)job);
			break;
		case JT_CLOSE_JOB:
			TryStartCloseJob((CloseJobStruct*)job);
			break;
		}
	}
}


void emPdfServerModel::TryStartOpenJob(OpenJob * openJob) throw(emString)
{
	RemoveJobFromList(openJob);
	if (openJob->Orphan) {
		delete openJob;
	}
	else {
		WriteLineToProc(
			emString::Format(
				"open %s",
				openJob->FilePath.Get()
			)
		);
		AddJobToRunningList(openJob);
		openJob->State=JS_RUNNING;
		if (openJob->ListenEngine) openJob->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartRenderJob(RenderJob * renderJob) throw(emString)
{
	RemoveJobFromList(renderJob);
	if (renderJob->Orphan) {
		delete renderJob;
	}
	else if (renderJob->ProcRunId!=ProcRunId) {
		renderJob->State=JS_ERROR;
		renderJob->ErrorText="PDF server process restarted";
		if (renderJob->ListenEngine) renderJob->ListenEngine->WakeUp();
	}
	else {
		WriteLineToProc(emString::Format(
			"render %d %d %.16lg %.16lg %.16lg %.16lg %d %d",
			renderJob->InstanceId,
			renderJob->Page,
			renderJob->SrcX,
			renderJob->SrcY,
			renderJob->SrcWidth,
			renderJob->SrcHeight,
			renderJob->TgtW,
			renderJob->TgtH
		));
		AddJobToRunningList(renderJob);
		renderJob->State=JS_RUNNING;
		if (renderJob->ListenEngine) renderJob->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryStartCloseJob(CloseJobStruct * closeJob) throw(emString)
{
	RemoveJobFromList(closeJob);
	if (closeJob->ProcRunId==ProcRunId) {
		WriteLineToProc(emString::Format(
			"close %d",
			closeJob->InstanceId
		));
		ProcPdfInstCount--;
	}
	if (closeJob->Orphan) {
		delete closeJob;
	}
	else {
		closeJob->State=JS_SUCCESS;
		if (closeJob->ListenEngine) closeJob->ListenEngine->WakeUp();
	}
}


void emPdfServerModel::TryFinishJobs() throw(emString)
{
	Job * job;

	for (;;) {
		job=FirstRunningJob;
		if (!job) break;
		if (job->Type==JT_OPEN_JOB) {
			if (!TryFinishOpenJob((OpenJob*)job)) break;
		}
		else if (job->Type==JT_RENDER_JOB) {
			if (!TryFinishRenderJob((RenderJob*)job)) break;
		}
		else {
			emFatalError("emPdfServerModel::TryFinishJobs: illegal job in running list");
		}
	}
}


bool emPdfServerModel::TryFinishOpenJob(OpenJob * job) throw(emString)
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
		args.Empty();
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
			throw emString::Format("PDF server protocol error (%d)",__LINE__);
		}
		job->Instance->ProcRunId=ProcRunId;
		job->Instance->InstanceId=i1;
	}
	else if (cmd=="pages:") {
		r=sscanf(args,"%d",&i1);
		if (r<1) {
			throw emString::Format("PDF server protocol error (%d)",__LINE__);
		}
		job->Instance->Pages.SetCount(i1);
	}
	else if (cmd=="pageinfo:") {
		r=sscanf(args,"%d %lf %lf %n",&i1,&d1,&d2,&pos);
		if (
			r<3 || pos<=0 || i1<0 ||
			i1>=job->Instance->Pages.GetCount()
		) {
			throw emString::Format("PDF server protocol error (%d)",__LINE__);
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
		throw emString::Format("PDF server protocol error (%d)",__LINE__);
	}

	return true;
}


bool emPdfServerModel::TryFinishRenderJob(RenderJob * job) throw(emString)
{
	int len,total,type,width,height,maxColor;
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
				throw emString::Format("PDF server protocol error (%d)",__LINE__);
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
		if (type!='6' || width!=job->TgtW || height!=job->TgtH || maxColor!=255) {
			throw emString::Format("PDF server protocol error (%d)",__LINE__);
		}
		job->ReadStage=1;
		progress=true;
	}

	if (ReadBuf.IsEmpty()) return progress;
	total=job->TgtW*job->TgtH*3;
	len=ReadBuf.GetCount();
	if (len>total-job->ReadPos) len=total-job->ReadPos;
	if (!job->Orphan) {
		if (
			job->OutputImage->GetWidth()!=job->TgtW ||
			job->OutputImage->GetHeight()!=job->TgtH ||
			job->OutputImage->GetChannelCount()!=3
		) {
			job->OutputImage->Setup(job->TgtW,job->TgtH,3);
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


bool emPdfServerModel::TryProcIO() throw(emString)
{
	char buf[256];
	bool progress;
	int r;

	progress=false;

	if (!WriteBuf.IsEmpty()) {
		r=Process.TryWrite(WriteBuf.Get(),WriteBuf.GetCount());
		if (r<0) throw emString("PDF server process died unexpectedly.");
		if (r>0) {
			WriteBuf.Remove(0,r);
			progress=true;
		}
	}

	while (ReadBuf.GetCount()<65536) {
		r=Process.TryRead(buf,sizeof(buf));
		if (r<0) throw emString("PDF server process died unexpectedly.");
		if (r==0) break;
		ReadBuf.Add(buf,r);
		progress=true;
	}

	return progress;
}


emPdfServerModel::Job * emPdfServerModel::SearchBestNextJob() const
{
	Job * job, * bestJob;

	bestJob=FirstWaitingJob;
	if (!bestJob) return NULL;
	for (job=bestJob->Next; job; job=job->Next) {
		switch (bestJob->Type) {
		case JT_OPEN_JOB:
			switch (job->Type) {
			case JT_OPEN_JOB:
				if (bestJob->Priority<job->Priority) bestJob=job;
				break;
			case JT_RENDER_JOB:
				break;
			case JT_CLOSE_JOB:
				bestJob=job;
				break;
			}
			break;
		case JT_RENDER_JOB:
			switch (job->Type) {
			case JT_OPEN_JOB:
				bestJob=job;
				break;
			case JT_RENDER_JOB:
				if (bestJob->Priority<job->Priority) bestJob=job;
				break;
			case JT_CLOSE_JOB:
				bestJob=job;
				break;
			}
			break;
		case JT_CLOSE_JOB:
			break;
		}
	}
	return bestJob;
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
		throw emString::Format("PDF server protocol error (%d)",__LINE__);
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
				throw emString::Format("PDF server protocol error (%d)",__LINE__);
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
		throw emString::Format("PDF server protocol error (%d)",__LINE__);
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
