//------------------------------------------------------------------------------
// emSvgServerModel.cpp
//
// Copyright (C) 2010-2011 Oliver Hamann.
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

#include <sys/shm.h>
#include <emSvg/emSvgServerModel.h>
#include <emCore/emInstallInfo.h>


emRef<emSvgServerModel> emSvgServerModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emSvgServerModel,rootContext,"")
}


emSvgServerModel::JobHandle emSvgServerModel::StartOpenJob(
	const emString & filePath, SvgHandle * svgHandleReturn,
	double priority, emEngine * listenEngine
)
{
	OpenJob * job;

	job=new OpenJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->FilePath=filePath;
	job->SvgHandleReturn=svgHandleReturn;
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


emSvgServerModel::JobHandle emSvgServerModel::StartRenderJob(
	SvgHandle svgHandle, double srcX, double srcY, double srcWidth,
	double srcHeight, emColor bgColor, emImage * outputImage,
	double priority, emEngine * listenEngine
)
{
	RenderJob * job;

	job=new RenderJob;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->ProcRunId=((SvgInstance*)svgHandle)->ProcRunId;
	job->InstanceId=((SvgInstance*)svgHandle)->InstanceId;
	job->SrcX=srcX;
	job->SrcY=srcY;
	job->SrcWidth=srcWidth;
	job->SrcHeight=srcHeight;
	job->BgColor=bgColor;
	job->OutputImage=outputImage;
	job->TgtW=outputImage->GetWidth();
	job->TgtH=outputImage->GetHeight();
	AddJobToWaitingList(job);
	WakeUp();
	return job;
}


void emSvgServerModel::CloseJob(JobHandle jobHandle)
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


void emSvgServerModel::CloseSvg(SvgHandle svgHandle)
{
	CloseJobStruct * job;
	SvgInstance * inst;

	inst=(SvgInstance*)svgHandle;
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


void emSvgServerModel::Poll(unsigned maxMilliSecs)
{
	emUInt64 endTime,now;
	int flags;

	if (!FirstRunningJob && !FirstWaitingJob) {
		if (
			ProcSvgInstCount==0 &&
			Process.IsRunning() &&
			!ProcTerminating &&
			emGetClockMS()-ProcIdleClock>=5000
		) {
			emDLog("emSvgServerModel: Terminating server process");
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
			ProcSvgInstCount=0;
			ReadBuf.Empty();
			WriteBuf.Empty();
			emDLog("emSvgServerModel: Starting server process");
			Process.TryStart(
				emArray<emString>(
					emGetChildPath(
						emGetInstallPath(EM_IDT_LIB,"emSvg","emSvg"),
						"emSvgServerProc"
					)
				),
				emArray<emString>(),
				NULL,
				emProcess::SF_PIPE_STDIN|
				emProcess::SF_PIPE_STDOUT|
				emProcess::SF_SHARE_STDERR
			);
			if (ShmSize<MinShmSize) {
				TryAllocShm(MinShmSize);
			}
			TryWriteAttachShm();
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


emSvgServerModel::emSvgServerModel(emContext & context, const emString & name)
	: emModel(context,name)
{
	ProcRunId=0;
	ProcSvgInstCount=0;
	ProcIdleClock=0;
	ProcTerminating=false;
	ReadBuf.SetTuningLevel(4);
	WriteBuf.SetTuningLevel(4);
	FirstWaitingJob=NULL;
	LastWaitingJob=NULL;
	FirstRunningJob=NULL;
	LastRunningJob=NULL;
	ShmSize=0;
	ShmId=-1;
	ShmPtr=NULL;
	ShmAllocBegin=0;
	ShmAllocEnd=0;
	SetMinCommonLifetime(10);
	SetEnginePriority(LOW_PRIORITY);
}


emSvgServerModel::~emSvgServerModel()
{
	Job * job;

	for (;;) {
		job=FirstRunningJob;
		if (!job) job=FirstWaitingJob;
		if (!job) break;
		if (!job->Orphan) {
			emFatalError("emSvgServerModel::~emSvgServerModel: Job not closed.");
		}
		RemoveJobFromList(job);
		delete job;
	}
	Process.Terminate();
	FreeShm();
}


bool emSvgServerModel::Cycle()
{
	bool busy;

	busy=emModel::Cycle();

	Poll(IsTimeSliceAtEnd()?0:10);

	if (
		FirstRunningJob || FirstWaitingJob || !WriteBuf.IsEmpty() ||
		(Process.IsRunning() && !ProcSvgInstCount)
	) busy=true;

	return busy;
}


emSvgServerModel::SvgInstance::SvgInstance()
{
	ProcRunId=0;
	InstanceId=-1;
	Width=0.0;
	Height=0.0;
}


emSvgServerModel::SvgInstance::~SvgInstance()
{
}


emSvgServerModel::Job::Job()
{
	State=JS_WAITING;
	Priority=0.0;
	ListenEngine=NULL;
	Orphan=false;
	Prev=NULL;
	Next=NULL;
}


emSvgServerModel::Job::~Job()
{
}


emSvgServerModel::OpenJob::OpenJob()
{
	Type=JT_OPEN_JOB;
	SvgHandleReturn=NULL;
}


emSvgServerModel::OpenJob::~OpenJob()
{
}


emSvgServerModel::RenderJob::RenderJob()
{
	Type=JT_RENDER_JOB;
	ProcRunId=0;
	InstanceId=-1;
	SrcX=0.0;
	SrcY=0.0;
	SrcWidth=0.0;
	SrcHeight=0.0;
	BgColor=0;
	OutputImage=NULL;
	TgtW=0;
	TgtH=0;
	ShmOffset=0;
}


emSvgServerModel::RenderJob::~RenderJob()
{
}


emSvgServerModel::CloseJobStruct::CloseJobStruct()
{
	Type=JT_CLOSE_JOB;
	ProcRunId=0;
	InstanceId=-1;
}


emSvgServerModel::CloseJobStruct::~CloseJobStruct()
{
}


void emSvgServerModel::TryStartJobs() throw(emString)
{
	Job * job;

	while ((job=SearchBestNextJob())!=NULL) {
		if (job->Type==JT_OPEN_JOB) {
			TryStartOpenJob((OpenJob*)job);
		}
		else if (job->Type==JT_RENDER_JOB) {
			if (!TryStartRenderJob((RenderJob*)job)) break;
		}
		else if (job->Type==JT_CLOSE_JOB) {
			TryStartCloseJob((CloseJobStruct*)job);
		}
	}
}


void emSvgServerModel::TryStartOpenJob(OpenJob * openJob) throw(emString)
{
	if (openJob->Orphan) {
		RemoveJobFromList(openJob);
		delete openJob;
		return;
	}
	WriteLineToProc(
		emString::Format(
			"open %s",
			openJob->FilePath.Get()
		)
	);
	RemoveJobFromList(openJob);
	AddJobToRunningList(openJob);
	openJob->State=JS_RUNNING;
	if (openJob->ListenEngine) openJob->ListenEngine->WakeUp();
}


bool emSvgServerModel::TryStartRenderJob(RenderJob * renderJob) throw(emString)
{
	emByte * t, * e;
	emUInt32 u;
	int size;

	if (renderJob->Orphan) {
		RemoveJobFromList(renderJob);
		delete renderJob;
		return true;
	}

	if (renderJob->ProcRunId!=ProcRunId) {
		RemoveJobFromList(renderJob);
		renderJob->State=JS_ERROR;
		renderJob->ErrorText="SVG server process restarted";
		if (renderJob->ListenEngine) renderJob->ListenEngine->WakeUp();
		return true;
	}

	size=renderJob->TgtW*renderJob->TgtH*4;
	if (!FirstRunningJob || ShmAllocBegin==ShmAllocEnd) {
		if (size>ShmSize) {
			if (FirstRunningJob) return false;
			TryAllocShm(size);
			TryWriteAttachShm();
		}
		ShmAllocBegin=0;
		ShmAllocEnd=0;
	}
	else if (ShmAllocEnd<ShmAllocBegin) {
		if (ShmAllocEnd+size>=ShmAllocBegin) return false;
	}
	else if (ShmAllocEnd+size>ShmSize) {
		if (size>=ShmAllocBegin) return false;
		ShmAllocEnd=0;
	}
	renderJob->ShmOffset=ShmAllocEnd;
	ShmAllocEnd+=size;

	t=ShmPtr+renderJob->ShmOffset;
	e=t+size;
	u=renderJob->BgColor.Get()>>8;
	while (t<e) {
		*(emUInt32*)t=u;
		t+=4;
	}

	WriteLineToProc(emString::Format(
		"render %d %.16lg %.16lg %.16lg %.16lg %d %d %d",
		renderJob->InstanceId,
		renderJob->SrcX,
		renderJob->SrcY,
		renderJob->SrcWidth,
		renderJob->SrcHeight,
		renderJob->ShmOffset,
		renderJob->TgtW,
		renderJob->TgtH
	));
	RemoveJobFromList(renderJob);
	AddJobToRunningList(renderJob);
	renderJob->State=JS_RUNNING;
	if (renderJob->ListenEngine) renderJob->ListenEngine->WakeUp();
	return true;
}


void emSvgServerModel::TryStartCloseJob(CloseJobStruct * closeJob) throw(emString)
{
	if (closeJob->ProcRunId==ProcRunId) {
		WriteLineToProc(emString::Format(
			"close %d",
			closeJob->InstanceId
		));
		ProcSvgInstCount--;
	}
	RemoveJobFromList(closeJob);
	if (closeJob->Orphan) {
		delete closeJob;
		return;
	}
	closeJob->State=JS_SUCCESS;
	if (closeJob->ListenEngine) closeJob->ListenEngine->WakeUp();
}


void emSvgServerModel::TryFinishJobs() throw(emString)
{
	emString cmd,args;
	const char * p;
	Job * job;
	int l;

	for (;;) {
		args=ReadLineFromProc();
		if (args.IsEmpty()) break;
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
		job=FirstRunningJob;
		if (cmd=="error:" && job) {
			RemoveJobFromList(job);
			job->State=JS_ERROR;
			job->ErrorText=args;
			if (job->Orphan) delete job;
			else if (job->ListenEngine) job->ListenEngine->WakeUp();
		}
		else if (cmd=="opened:" && job && job->Type==JT_OPEN_JOB) {
			TryFinishOpenJob((OpenJob*)job,args);
		}
		else if (cmd=="rendered" && job && job->Type==JT_RENDER_JOB) {
			TryFinishRenderJob((RenderJob*)job);
		}
		else {
			throw emString("SVG server protocol error");
		}
	}
}


void emSvgServerModel::TryFinishOpenJob(
	OpenJob * openJob, const char * args
) throw(emString)
{
	int instId,pos,r;
	double width,height;
	emString title,desc,str;
	SvgInstance * inst;
	char c;

	pos=-1;
	r=sscanf(args,"%d %lf %lf %n",&instId,&width,&height,&pos);
	if (r<3 || pos<=0) {
		throw emString("SVG server protocol error");
	}

	args+=pos;
	for (r=0; ; r++) {
		do { c=*args++; } while (c && c!='"');
		if (!c) break;
		str.Empty();
		for (;;) {
			c=*args++;
			if (!c || c=='"') break;
			if (c=='\\') {
				c=*args++;
				if (!c) break;
				if (c=='n') c='\n';
				else if (c=='r') c='\r';
				else if (c=='t') c='\t';
			}
			str+=c;
		}
		if (!r) title=str; else desc=str;
		if (!c) break;
	}

	ProcSvgInstCount++;

	inst=new SvgInstance;
	inst->ProcRunId=ProcRunId;
	inst->InstanceId=instId;
	inst->Width=width;
	inst->Height=height;
	inst->Title=title;
	inst->Description=desc;

	if (!openJob->Orphan && openJob->SvgHandleReturn) {
		*openJob->SvgHandleReturn=inst;
	}
	else {
		CloseSvg(inst);
	}

	RemoveJobFromList(openJob);
	openJob->State=JS_SUCCESS;
	if (openJob->Orphan) delete openJob;
	else if (openJob->ListenEngine) openJob->ListenEngine->WakeUp();
}


void emSvgServerModel::TryFinishRenderJob(RenderJob * renderJob) throw(emString)
{
	emByte * s, * t, * e;
	emImage * img;
	emUInt32 u;
	int size;

	size=renderJob->TgtW*renderJob->TgtH*4;
	ShmAllocBegin=renderJob->ShmOffset+size;
	if (
		!renderJob->Orphan &&
		(img=renderJob->OutputImage)!=NULL  &&
		img->GetWidth()==renderJob->TgtW &&
		img->GetHeight()==renderJob->TgtH &&
		img->GetChannelCount()==3
	) {
		s=ShmPtr+renderJob->ShmOffset;
		e=s+size;
		t=img->GetWritableMap();
		while (s<e) {
			u=*(emUInt32*)s;
			t[0]=(emByte)(u>>16);
			t[1]=(emByte)(u>>8);
			t[2]=(emByte)u;
			t+=3;
			s+=4;
		}
	}
	RemoveJobFromList(renderJob);
	renderJob->State=JS_SUCCESS;
	if (renderJob->Orphan) delete renderJob;
	else if (renderJob->ListenEngine) renderJob->ListenEngine->WakeUp();
}


void emSvgServerModel::TryWriteAttachShm() throw(emString)
{
	WriteLineToProc(emString::Format("attachshm %d",ShmId));
}


void emSvgServerModel::FailAllRunningJobs(emString errorText)
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


void emSvgServerModel::FailAllJobs(emString errorText)
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


void emSvgServerModel::WriteLineToProc(const char * str)
{
	emDLog("emSvgServerModel: Sending: %s",str);
	WriteBuf.Add(str,strlen(str));
	WriteBuf.Add((char)'\n');
}


emString emSvgServerModel::ReadLineFromProc()
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
	if (!res.IsEmpty()) emDLog("emSvgServerModel: Receiving: %s",res.Get());
	return res;
}


bool emSvgServerModel::TryProcIO() throw(emString)
{
	char buf[256];
	bool progress;
	int r;

	progress=false;

	if (!WriteBuf.IsEmpty()) {
		r=Process.TryWrite(WriteBuf.Get(),WriteBuf.GetCount());
		if (r<0) throw emString("SVG server process died unexpectedly.");
		if (r>0) {
			WriteBuf.Remove(0,r);
			progress=true;
		}
	}

	for (;;) {
		r=Process.TryRead(buf,sizeof(buf));
		if (r<0) throw emString("SVG server process died unexpectedly.");
		if (r==0) break;
		ReadBuf.Add(buf,r);
		progress=true;
	}

	return progress;
}


void emSvgServerModel::TryAllocShm(int size) throw(emString)
{
	FreeShm();
#if defined(__CYGWIN__)
	throw emString("shmget not tried as cygwin may abort the process.");
#endif
	ShmId=shmget(IPC_PRIVATE,size,IPC_CREAT|0600);
	if (ShmId==-1) {
		throw emString::Format(
			"Failed to create shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}
	ShmPtr=(emByte*)shmat(ShmId,0,0);
	if (ShmPtr==(emByte*)-1) {
		ShmPtr=NULL;
		shmctl(ShmId,IPC_RMID,0);
		ShmId=-1;
		throw emString::Format(
			"Failed to attach shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}
#if defined(__linux__)
	shmctl(ShmId,IPC_RMID,0);
	if (shmctl(ShmId,IPC_RMID,0)!=0) {
		emFatalError(
			"emSvgServerModel: shmctl failed: %s",
			emGetErrorText(errno).Get()
		);
	}
#endif
	ShmSize=size;
}


void emSvgServerModel::FreeShm()
{
	if (ShmPtr) {
		shmdt(ShmPtr);
		ShmPtr=NULL;
	}
	if (ShmId!=-1) {
#if !defined(__linux__)
		if (shmctl(ShmId,IPC_RMID,0)!=0) {
			emFatalError(
				"emSvgServerModel: shmctl failed: %s",
				emGetErrorText(errno).Get()
			);
		}
#endif
		ShmId=-1;
	}
	ShmSize=0;
	ShmAllocBegin=0;
	ShmAllocEnd=0;
}


emSvgServerModel::Job * emSvgServerModel::SearchBestNextJob() const
{
	Job * job, * bestJob;

	bestJob=FirstWaitingJob;
	if (!bestJob) return NULL;
	for (job=bestJob->Next; job; job=job->Next) {
		switch (bestJob->Type) {
		case JT_OPEN_JOB:
			if (
				job->Type!=JT_OPEN_JOB ||
				job->Priority>bestJob->Priority
			) bestJob=job;
			break;
		case JT_RENDER_JOB:
			if (
				job->Type==JT_RENDER_JOB &&
				job->Priority>bestJob->Priority
			) bestJob=job;
			break;
		case JT_CLOSE_JOB:
			if (job->Type==JT_RENDER_JOB) bestJob=job;
			break;
		}
	}
	return bestJob;
}


void emSvgServerModel::AddJobToWaitingList(Job * job)
{
	job->Prev=LastWaitingJob;
	job->Next=NULL;
	if (LastWaitingJob) LastWaitingJob->Next=job;
	else FirstWaitingJob=job;
	LastWaitingJob=job;
}


void emSvgServerModel::AddJobToRunningList(Job * job)
{
	job->Prev=LastRunningJob;
	job->Next=NULL;
	if (LastRunningJob) LastRunningJob->Next=job;
	else FirstRunningJob=job;
	LastRunningJob=job;
}


void emSvgServerModel::RemoveJobFromList(Job * job)
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


const int emSvgServerModel::MinShmSize=1000*1000*4;
