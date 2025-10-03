//------------------------------------------------------------------------------
// emSvgServerModel.cpp
//
// Copyright (C) 2010-2011,2014,2017-2019,2022-2024 Oliver Hamann.
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

#include <emSvg/emSvgServerModel.h>
#include <emCore/emInstallInfo.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#	include <emCore/emThread.h>
#	include <windows.h>
#else
#	include <sys/shm.h>
#endif


emRef<emSvgServerModel> emSvgServerModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emSvgServerModel,rootContext,"")
}


emSvgServerModel::SvgInstance::~SvgInstance()
{
	if (InstanceId!=-1 && SvgServerModel) {
		emRef<CloseJob> job=new CloseJob(ProcRunId,InstanceId);
		SvgServerModel->EnqueueJob(*job);
	}
}


emSvgServerModel::SvgInstance::SvgInstance(emSvgServerModel & svgServerModel)
	: SvgServerModel(&svgServerModel),
	ProcRunId(0),
	InstanceId(-1),
	Width(0.0),
	Height(0.0)
{
}


emSvgServerModel::OpenJob::OpenJob(const emString & filePath, double priority)
	: emJob(priority),
	FilePath(filePath)
{
}


emSvgServerModel::RenderJob::RenderJob(
	SvgInstance & svgInstance, double srcX, double srcY,
	double srcWidth, double srcHeight, emColor bgColor,
	int tgtWidth, int tgtHeight, double priority
)
	: emJob(priority),
	SvgInst(&svgInstance),
	SrcX(srcX),
	SrcY(srcY),
	SrcWidth(srcWidth),
	SrcHeight(srcHeight),
	BgColor(bgColor),
	TgtW(tgtWidth),
	TgtH(tgtHeight),
	ShmOffset(0)
{
}


void emSvgServerModel::EnqueueJob(emJob & job)
{
	JobQueue.EnqueueJob(job);
	WakeUp();
}


void emSvgServerModel::AbortJob(emJob & job)
{
	if (job.GetState() != emJob::ST_RUNNING) {
		JobQueue.AbortJob(job);
	}
}


void emSvgServerModel::Poll(unsigned maxMillisecs)
{
	emUInt64 endTime,now;
	int flags;

	endTime=emGetClockMS()+maxMillisecs;

	if (
		JobQueue.IsEmpty() &&
		ProcSvgInstCount==0 &&
		Process.IsRunning() &&
		!ProcTerminating &&
		emGetClockMS()-ProcIdleClock>=5000
	) {
		emDLog("emSvgServerModel: Terminating server process");
		Process.CloseWriting();
		ProcTerminating=true;
	}

	if (ProcTerminating) {
		if (!Process.WaitForTermination(maxMillisecs)) return;
		ProcTerminating=false;
	}

	if (JobQueue.IsEmpty()) return;

	ProcIdleClock=emGetClockMS();

	try {
		if (!Process.IsRunning()) {
			ProcRunId++;
			ProcSvgInstCount=0;
			ReadBuf.Clear();
			WriteBuf.Clear();
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
				emProcess::SF_SHARE_STDERR|
				emProcess::SF_NO_WINDOW
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
			if (!JobQueue.GetFirstRunningJob() && WriteBuf.IsEmpty()) break;
			now=emGetClockMS();
			if (now>=endTime) break;
			flags=emProcess::WF_WAIT_STDOUT;
			if (!WriteBuf.IsEmpty()) flags|=emProcess::WF_WAIT_STDIN;
			Process.WaitPipes(flags,(unsigned)(endTime-now));
		}
	}
	catch (const emException & exception) {
		if (!JobQueue.GetFirstRunningJob()) JobQueue.FailAllJobs(exception.GetText());
		else JobQueue.FailAllRunningJobs(exception.GetText());
		Process.SendTerminationSignal();
		ProcTerminating=true;
	}
}


emSvgServerModel::emSvgServerModel(emContext & context, const emString & name)
	: emModel(context,name),
	JobQueue(GetScheduler())
{
	ProcRunId=0;
	ProcSvgInstCount=0;
	ProcIdleClock=0;
	ProcTerminating=false;
	ReadBuf.SetTuningLevel(4);
	WriteBuf.SetTuningLevel(4);
	ShmSize=0;
#if defined(_WIN32) || defined(__CYGWIN__)
	ShmId[0]=0;
	ShmHdl=NULL;
#else
	ShmId=-1;
#endif
	ShmPtr=NULL;
	ShmAllocBegin=0;
	ShmAllocEnd=0;
	SetMinCommonLifetime(10);
	SetEnginePriority(LOW_PRIORITY);
}


emSvgServerModel::~emSvgServerModel()
{
	Process.Terminate();
	FreeShm();
}


bool emSvgServerModel::Cycle()
{
	bool busy;

	busy=emModel::Cycle();

	Poll(IsTimeSliceAtEnd()?0:10);

	if (
		!JobQueue.IsEmpty() || !WriteBuf.IsEmpty() ||
		(Process.IsRunning() && !ProcSvgInstCount)
	) busy=true;

	return busy;
}


emSvgServerModel::CloseJob::CloseJob(emUInt64 procRunId, int instanceId)
	: emJob(1E200),
	ProcRunId(procRunId),
	InstanceId(instanceId)
{
}


void emSvgServerModel::TryStartJobs()
{
	emJob * job;

	JobQueue.UpdateSortingOfWaitingJobs();

	while ((job=JobQueue.GetFirstWaitingJob())!=NULL) {
		if (OpenJob * openJob=dynamic_cast<OpenJob*>(job)) {
			TryStartOpenJob(*openJob);
		}
		else if (RenderJob * renderJob=dynamic_cast<RenderJob*>(job)) {
			if (!TryStartRenderJob(*renderJob)) break;
		}
		else if (CloseJob * closeJob=dynamic_cast<CloseJob*>(job)) {
			TryStartCloseJob(*closeJob);
		}
		else {
			JobQueue.FailJob(*job,"Unsupported job class");
		}
	}
}


void emSvgServerModel::TryStartOpenJob(OpenJob & openJob)
{
	WriteLineToProc(
		emString::Format(
			"open %s",
			openJob.FilePath.Get()
		)
	);
	JobQueue.StartJob(openJob);
}


bool emSvgServerModel::TryStartRenderJob(RenderJob & renderJob)
{
	emByte * t, * e;
	emUInt32 u;
	int size;

	if (renderJob.SvgInst->ProcRunId!=ProcRunId) {
		JobQueue.FailJob(renderJob,"SVG server process restarted");
		return true;
	}

	size=renderJob.TgtW*renderJob.TgtH*4;
	if (!JobQueue.GetFirstRunningJob() || ShmAllocBegin==ShmAllocEnd) {
		if (size>ShmSize) {
			if (JobQueue.GetFirstRunningJob()) return false;
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
	renderJob.ShmOffset=ShmAllocEnd;
	ShmAllocEnd+=size;

	t=ShmPtr+renderJob.ShmOffset;
	e=t+size;
	u=renderJob.BgColor.Get()>>8;
	while (t<e) {
		*(emUInt32*)t=u;
		t+=4;
	}

	WriteLineToProc(emString::Format(
		"render %d %.16g %.16g %.16g %.16g %d %d %d",
		renderJob.SvgInst->InstanceId,
		renderJob.SrcX,
		renderJob.SrcY,
		renderJob.SrcWidth,
		renderJob.SrcHeight,
		renderJob.ShmOffset,
		renderJob.TgtW,
		renderJob.TgtH
	));
	JobQueue.StartJob(renderJob);
	return true;
}


void emSvgServerModel::TryStartCloseJob(CloseJob & closeJob)
{
	if (closeJob.ProcRunId==ProcRunId) {
		WriteLineToProc(emString::Format(
			"close %d",
			closeJob.InstanceId
		));
		ProcSvgInstCount--;
	}
	JobQueue.SucceedJob(closeJob);
}


void emSvgServerModel::TryFinishJobs()
{
	emString cmd,args;
	const char * p;
	emJob * job;
	OpenJob * openJob;
	RenderJob * renderJob;
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
			args.Clear();
		}

		job=JobQueue.GetFirstRunningJob();
		if (job) {
			if (cmd=="error:") {
				JobQueue.FailJob(*job,args);
				continue;
			}
			if (cmd=="opened:") {
				openJob=dynamic_cast<OpenJob*>(job);
				if (openJob) {
					TryFinishOpenJob(*openJob,args);
					continue;
				}
			}
			if (cmd=="rendered") {
				renderJob=dynamic_cast<RenderJob*>(job);
				if (renderJob) {
					TryFinishRenderJob(*renderJob);
					continue;
				}
			}
		}

		throw emException("SVG server protocol error");
	}
}


void emSvgServerModel::TryFinishOpenJob(OpenJob & openJob, const char * args)
{
	int instId,pos,r;
	double width,height;
	emString title,desc,str;
	emRef<SvgInstance> inst;
	char c;

	pos=-1;
	r=sscanf(args,"%d %lf %lf %n",&instId,&width,&height,&pos);
	if (r<3 || pos<=0) {
		throw emException("SVG server protocol error");
	}

	args+=pos;
	for (r=0; ; r++) {
		do { c=*args++; } while (c && c!='"');
		if (!c) break;
		str.Clear();
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

	inst=new SvgInstance(*this);
	inst->ProcRunId=ProcRunId;
	inst->InstanceId=instId;
	inst->Width=width;
	inst->Height=height;
	inst->Title=title;
	inst->Description=desc;

	openJob.SvgInst=inst;
	JobQueue.SucceedJob(openJob);
}


void emSvgServerModel::TryFinishRenderJob(RenderJob & renderJob)
{
	emByte * s, * t, * e;
	emUInt32 u;
	int size;

	size=renderJob.TgtW*renderJob.TgtH*4;
	ShmAllocBegin=renderJob.ShmOffset+size;
	if (renderJob.GetRefCount() > 1) {
		renderJob.Image.Setup(renderJob.TgtW,renderJob.TgtH,3);
		s=ShmPtr+renderJob.ShmOffset;
		e=s+size;
		t=renderJob.Image.GetWritableMap();
		while (s<e) {
			u=*(emUInt32*)s;
			t[0]=(emByte)(u>>16);
			t[1]=(emByte)(u>>8);
			t[2]=(emByte)u;
			t+=3;
			s+=4;
		}
	}
	JobQueue.SucceedJob(renderJob);
}


void emSvgServerModel::TryWriteAttachShm()
{
#if defined(_WIN32) || defined(__CYGWIN__)
	WriteLineToProc(emString::Format("attachshm %s",ShmId));
#else
	WriteLineToProc(emString::Format("attachshm %d",ShmId));
#endif
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


bool emSvgServerModel::TryProcIO()
{
	char buf[256];
	bool progress;
	int r;

	progress=false;

	if (!WriteBuf.IsEmpty()) {
		r=Process.TryWrite(WriteBuf.Get(),WriteBuf.GetCount());
		if (r<0) throw emException("SVG server process died unexpectedly.");
		if (r>0) {
			WriteBuf.Remove(0,r);
			progress=true;
		}
	}

	for (;;) {
		r=Process.TryRead(buf,sizeof(buf));
		if (r<0) throw emException("SVG server process died unexpectedly.");
		if (r==0) break;
		ReadBuf.Add(buf,r);
		progress=true;
	}

	return progress;
}


void emSvgServerModel::TryAllocShm(int size)
{
	FreeShm();

#if defined(_WIN32) || defined(__CYGWIN__)

	static emThreadMiniMutex sharedCounterMutex;
	static unsigned long sharedCounter=0;
	unsigned long counter;

	sharedCounterMutex.Lock();
	counter=sharedCounter++;
	sharedCounterMutex.Unlock();

	sprintf(
		ShmId,
		"Local\\emSvg.%lX.%lX.%lX.%lX",
		(unsigned long)GetCurrentProcessId(),
		counter,
		(unsigned long)GetTickCount(),
		(unsigned long)emGetUInt64Random(0,0xffffffff) //???
	);
	emDLog("emSvgServerModel: ShmId=%s",ShmId);

	SetLastError(ERROR_SUCCESS);
	ShmHdl=CreateFileMapping(
		INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,
		0,size,ShmId
	);
	if (!ShmHdl || GetLastError()==ERROR_ALREADY_EXISTS) {
		if (ShmHdl) {
			CloseHandle(ShmHdl);
			ShmHdl=NULL;
		}
		ShmId[0]=0;
		throw emException(
			"Failed to create shared memory segment: CreateFileMapping: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

	ShmPtr=(emByte*)MapViewOfFile(ShmHdl,FILE_MAP_ALL_ACCESS,0,0,0);
	if (!ShmPtr) {
		CloseHandle(ShmHdl);
		ShmHdl=NULL;
		ShmId[0]=0;
		throw emException(
			"Failed to create shared memory segment: MapViewOfFile: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

#else

	ShmId=shmget(IPC_PRIVATE,size,IPC_CREAT|0600);
	if (ShmId==-1) {
		throw emException(
			"Failed to create shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}

	ShmPtr=(emByte*)shmat(ShmId,NULL,0);
	if (ShmPtr==(emByte*)-1) {
		ShmPtr=NULL;
		shmctl(ShmId,IPC_RMID,NULL);
		ShmId=-1;
		throw emException(
			"Failed to attach shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}

#if defined(__linux__)
	if (shmctl(ShmId,IPC_RMID,NULL)!=0) {
		emFatalError(
			"emSvgServerModel: shmctl failed: %s",
			emGetErrorText(errno).Get()
		);
	}
#endif

#endif

	ShmSize=size;
}


void emSvgServerModel::FreeShm()
{
#if defined(_WIN32) || defined(__CYGWIN__)
	if (ShmPtr) {
		UnmapViewOfFile(ShmPtr);
		ShmPtr=NULL;
	}
	if (ShmHdl) {
		CloseHandle(ShmHdl);
		ShmHdl=NULL;
	}
	ShmId[0]=0;
#else
	if (ShmPtr) {
		shmdt(ShmPtr);
		ShmPtr=NULL;
	}
	if (ShmId!=-1) {
#if !defined(__linux__)
		if (shmctl(ShmId,IPC_RMID,NULL)!=0) {
			emFatalError(
				"emSvgServerModel: shmctl failed: %s",
				emGetErrorText(errno).Get()
			);
		}
#endif
		ShmId=-1;
	}
#endif
	ShmSize=0;
	ShmAllocBegin=0;
	ShmAllocEnd=0;
}


const int emSvgServerModel::MinShmSize=1000*1000*4;
