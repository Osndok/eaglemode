//------------------------------------------------------------------------------
// emPsRenderer.cpp
//
// Copyright (C) 2006-2011 Oliver Hamann.
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

#include <emPs/emPsRenderer.h>


emRef<emPsRenderer> emPsRenderer::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emPsRenderer,rootContext,"");
}


emPsRenderer::JobHandle emPsRenderer::StartJob(
	const emPsDocument & document, int pageIndex,
	emImage & outputImage, double priority,
	emEngine * listenEngine
)
{
	Job * job;

	job=new Job;
	job->Document=document;
	job->PageIndex=pageIndex;
	job->Image=&outputImage;
	job->Priority=priority;
	job->ListenEngine=listenEngine;
	job->State=JS_WAITING;
	job->Prev=NULL;
	job->Next=NULL;
	AddToJobList(job);
	PSPriorityValid=false;
	WakeUp();
	return job;
}


void emPsRenderer::SetJobPriority(JobHandle jobHandle, double priority)
{
	Job * job;

	job=(Job*)jobHandle;
	if (job->Priority!=priority) {
		job->Priority=priority;
		if (job->State==JS_WAITING) {
			PSPriorityValid=false;
			WakeUp();
		}
	}
}


void emPsRenderer::CloseJob(JobHandle jobHandle)
{
	Job * job;

	job=(Job*)jobHandle;
	if (job->State!=JS_SUCCESS && job->State!=JS_ERROR) {
		job->ListenEngine=NULL;
		SetJobState(job,JS_ERROR);
	}
	delete job;
}


emPsRenderer::emPsRenderer(emContext & context, const emString & name)
	: emModel(context,name),
	Timer(GetScheduler()),
	PSAgent(*this)
{
	SetMinCommonLifetime(5);
	PSPriorityValid=false;
	FirstJob=NULL;
	LastJob=NULL;
	MainState=COLD_WAIT_JOB;
	CurrentJob=NULL;
	CurrentPageIndex=0;
	AddWakeUpSignal(Timer.GetSignal());
}


emPsRenderer::~emPsRenderer()
{
	while (FirstJob) CloseJob(FirstJob);
	if (CurrentJob) CloseJob(CurrentJob);
	Process.Terminate();
}


bool emPsRenderer::Cycle()
{
	bool busy,readProceeded,writeProceeded;
	Job * job;
	int flags;

	busy=false;

	switch (MainState) {

L_ENTER_COLD_WAIT_JOB:
		CurrentDocument.Empty();
		PSAgent.ReleaseAccess();
		MainState=COLD_WAIT_JOB;
	case COLD_WAIT_JOB:
		if (FirstJob) goto L_ENTER_COLD_WAIT_ACCESS;
		break;

L_ENTER_COLD_WAIT_ACCESS:
		UpdatePSPriority();
		PSAgent.RequestAccess();
		MainState=COLD_WAIT_ACCESS;
	case COLD_WAIT_ACCESS:
		if (!FirstJob) goto L_ENTER_COLD_WAIT_JOB;
		if (PSAgent.HasAccess()) goto L_ENTER_PREPARE_PROCESS;
		UpdatePSPriority();
		break;

L_ENTER_PREPARE_PROCESS:
		job=SearchBestJob();
		if (!job) goto L_ENTER_COLD_WAIT_JOB;
		CurrentDocument=job->Document;
		try {
			TryStartProcess();
		}
		catch (emString errorMessage) {
			FailAllJobs(errorMessage);
			goto L_ENTER_COLD_WAIT_JOB;
		}
		PrepareWritingStartup();
		PrepareReadingStartup();
		Timer.Start(12000);
		MainState=PREPARE_PROCESS;
	case PREPARE_PROCESS:
		if (!Process.IsRunning()) {
			FailDocJobs("PostScript interpretation failed: Interpreter exited.");
			goto L_ENTER_QUIT_PROCESS;
		}
		try {
			TryRead();
			TryWrite();
		}
		catch (emString errorMessage) {
			FailDocJobs(errorMessage);
			goto L_ENTER_QUIT_PROCESS;
		}
		if (IsReadingFinished()) goto L_ENTER_RUN_JOB;
		if (!Timer.IsRunning()) {
			FailDocJobs("PostScript interpretation failed: Start-up timed out.");
			goto L_ENTER_QUIT_PROCESS;
		}
		busy=true;
		break;

L_ENTER_RUN_JOB:
		if (CurrentDocument.GetDataRefCount()<=1) goto L_ENTER_QUIT_PROCESS;
		job=SearchBestSameDocJob();
		if (!job) goto L_ENTER_QUIT_PROCESS;
		SetJobState(job,JS_RUNNING);
		CurrentPageIndex=CurrentJob->PageIndex;
		PrepareWritingPage();
		PrepareReadingPage();
		Timer.Start(8000);
		MainState=RUN_JOB;
	case RUN_JOB:
		if (!Process.IsRunning()) {
			FailDocJobs("PostScript interpretation failed: Interpreter exited.");
			goto L_ENTER_QUIT_PROCESS;
		}
		if (!Timer.IsRunning()) {
			FailDocJobs("PostScript interpretation failed: Page timed out.");
			goto L_ENTER_QUIT_PROCESS;
		}
		for (;;) {
			try {
				readProceeded=TryRead();
				writeProceeded=TryWrite();
			}
			catch (emString errorMessage) {
				FailDocJobs(errorMessage);
				goto L_ENTER_QUIT_PROCESS;
			}
			if (IsReadingFinished()) {
				if (CurrentJob) SetJobState(CurrentJob,JS_SUCCESS);
				goto L_ENTER_HOT_WAIT_JOB;
			}
			if (IsTimeSliceAtEnd()) break;
			if (!readProceeded && !writeProceeded) {
				flags=emProcess::WF_WAIT_STDOUT;
				if (!IsWritingFinished()) flags|=emProcess::WF_WAIT_STDIN;
				Process.WaitPipes(flags,10);
			}
		}
		busy=true;
		break;

L_ENTER_HOT_WAIT_JOB:
		PSAgent.ReleaseAccess();
		Timer.Start(3000);
		MainState=HOT_WAIT_JOB;
	case HOT_WAIT_JOB:
		if (CurrentDocument.GetDataRefCount()<=1) goto L_ENTER_QUIT_PROCESS;
		if (FirstJob) goto L_ENTER_HOT_WAIT_ACCESS;
		if (!Timer.IsRunning()) goto L_ENTER_QUIT_PROCESS;
		busy=true;
		break;

L_ENTER_HOT_WAIT_ACCESS:
		UpdatePSPriority();
		PSAgent.RequestAccess();
		MainState=HOT_WAIT_ACCESS;
	case HOT_WAIT_ACCESS:
		if (CurrentDocument.GetDataRefCount()<=1) goto L_ENTER_QUIT_PROCESS;
		if (!FirstJob) goto L_ENTER_QUIT_PROCESS;
		if (PSAgent.HasAccess()) goto L_ENTER_RUN_JOB;
		UpdatePSPriority();
		busy=true;
		break;

L_ENTER_QUIT_PROCESS:
		CurrentDocument.Empty();
		PSAgent.ReleaseAccess();
		Process.CloseWriting();
		Process.CloseReading();
		Process.SendTerminationSignal();
		Timer.Start(10000);
		MainState=QUIT_PROCESS;
	case QUIT_PROCESS:
		if (!Process.IsRunning()) goto L_ENTER_COLD_WAIT_JOB;
		if (!Timer.IsRunning()) {
			FailAllJobs(
				"Failed to terminate PostScript interpreter after previous job."
			);
			Timer.Start(10000);
		}
		busy=true;
		break;
	}

	return busy;
}


void emPsRenderer::AddToJobList(Job * job)
{
	job->Prev=LastJob;
	job->Next=NULL;
	if (LastJob) LastJob->Next=job; else FirstJob=job;
	LastJob=job;
}


void emPsRenderer::RemoveFromJobList(Job * job)
{
	if (job->Prev) job->Prev->Next=job->Next;
	else FirstJob=job->Next;
	if (job->Next) job->Next->Prev=job->Prev;
	else LastJob=job->Prev;
	job->Prev=NULL;
	job->Next=NULL;
}


emPsRenderer::Job * emPsRenderer::SearchBestJob()
{
	Job * job, * bestJob;
	double bestPri;

	bestJob=FirstJob;
	if (bestJob) {
		bestPri=bestJob->Priority;
		for (job=bestJob->Next; job; job=job->Next) {
			if (bestPri<job->Priority) {
				bestPri=job->Priority;
				bestJob=job;
			}
		}
	}
	return bestJob;
}


emPsRenderer::Job * emPsRenderer::SearchBestSameDocJob()
{
	Job * job, * bestJob;

	for (bestJob=FirstJob; bestJob; bestJob=bestJob->Next) {
		if (CurrentDocument==bestJob->Document) break;
	}
	if (bestJob) {
		for (job=bestJob->Next; job; job=job->Next) {
			if (bestJob->Priority<job->Priority && bestJob->Document==job->Document) {
				bestJob=job;
			}
		}
	}
	return bestJob;
}


void emPsRenderer::SetJobState(Job * job, JobState state, emString errorText)
{
	switch (job->State) {
	case JS_WAITING:
		RemoveFromJobList(job);
		PSPriorityValid=false;
		WakeUp();
		break;
	case JS_RUNNING:
		CurrentJob=NULL;
		break;
	default:
		break;
	}

	job->State=state;
	job->ErrorText=errorText;
	if (job->ListenEngine) job->ListenEngine->WakeUp();

	switch (job->State) {
	case JS_WAITING:
		AddToJobList(job);
		PSPriorityValid=false;
		WakeUp();
		break;
	case JS_RUNNING:
		CurrentJob=job;
		break;
	default:
		break;
	}
}


void emPsRenderer::FailCurrentJob(emString errorMessage)
{
	if (CurrentJob) SetJobState(CurrentJob,JS_ERROR,errorMessage);
}


void emPsRenderer::FailDocJobs(emString errorMessage)
{
	Job * * pJob;
	Job * job;

	for (pJob=&FirstJob;;) {
		job=*pJob;
		if (!job) break;
		if (job->Document==CurrentDocument) {
			SetJobState(job,JS_ERROR,errorMessage);
		}
		else {
			pJob=&job->Next;
		}
	}
	if (CurrentJob) SetJobState(CurrentJob,JS_ERROR,errorMessage);
}


void emPsRenderer::FailAllJobs(emString errorMessage)
{
	while (FirstJob) SetJobState(FirstJob,JS_ERROR,errorMessage);
	if (CurrentJob) SetJobState(CurrentJob,JS_ERROR,errorMessage);
}


void emPsRenderer::UpdatePSPriority()
{
	Job * job;
	double pri;

	if (!PSPriorityValid) {
		job=SearchBestJob();
		if (job) pri=job->Priority;
		else pri=0.0;
		PSAgent.SetAccessPriority(pri);
		PSPriorityValid=true;
	}
}


void emPsRenderer::TryStartProcess() throw(emString)
{
#if defined(_WIN32)

	throw emString(
		"Ghostscript probably not available on this system. As a precaution, execution is not tried."
	);

#else

	emArray<emString> args;

	args.Add("gs");
	args.Add("-q");
	args.Add("-dNOPAUSE");
	args.Add("-dSAFER");
	args.Add("-sDEVICE=ppmraw");
	args.Add("-dTextAlphaBits=1");     // emPsPagePanel performs some
	args.Add("-dGraphicsAlphaBits=1"); // kind of anti-aliasing. Therefore
	args.Add("-dNOINTERPOLATE");       // it's disabled here.
	args.Add("-dAlignToPixels=0");
	args.Add("-r72.0x72.0"); // Dummy values (adapted for each page by commands).
	args.Add("-g612x792");   // Dummy values (adapted for each page by commands).
	args.Add("-sOutputFile=-");
	args.Add("-_"); // "-" or "-_"?

	Process.TryStart(
		args,
		emArray<emString>(),
		NULL,
		emProcess::SF_PIPE_STDIN|
		emProcess::SF_PIPE_STDOUT|
		emProcess::SF_SHARE_STDERR
	);

#endif
}


void emPsRenderer::PrepareWritingStartup()
{
	WriterState=WRITING_STARTUP;
	WriterPos=0;
	WriteCommand.Empty();
}


void emPsRenderer::PrepareWritingPage()
{
	double rx,ry,rt;
	int w,h,t;

	if (CurrentJob && CurrentJob->Image) {
		w=CurrentJob->Image->GetWidth();
		h=CurrentJob->Image->GetHeight();
	}
	else {
		w=10;
		h=10;
	}
	rx=w*72.0/CurrentDocument.GetPageWidth(CurrentPageIndex);
	ry=h*72.0/CurrentDocument.GetPageHeight(CurrentPageIndex);
	if (CurrentDocument.IsLandscapePage(CurrentPageIndex)) {
		rt=rx; rx=ry; ry=rt;
		t=w; w=h; h=t;
	}
	WriteCommand=emString::Format(
		"\nmark /HWSize [%d %d] /HWResolution [%f %f] currentdevice putdeviceprops pop\n",
		w,h,
		rx,ry
	);
	WriterState=WRITING_PAGE_SIZE;
	WriterPos=0;
}


bool emPsRenderer::TryWrite() throw(emString)
{
	const char * buf;
	int len;

	switch (WriterState) {
	case WRITING_STARTUP:
		buf=CurrentDocument.GetStartupScriptPtr();
		len=CurrentDocument.GetStartupScriptLen();
		if (WriterPos>=len) goto L_ENTER_WRITING_SYNC;
		break;

	case WRITING_PAGE_SIZE:
		buf=WriteCommand.Get();
		len=strlen(buf);
		if (WriterPos>=len) goto L_ENTER_WRITING_PAGE;
		break;

L_ENTER_WRITING_PAGE:
		WriterPos=0;
		WriterState=WRITING_PAGE;
	case WRITING_PAGE:
		buf=CurrentDocument.GetPageScriptPtr(CurrentPageIndex);
		len=CurrentDocument.GetPageScriptLen(CurrentPageIndex);
		if (WriterPos>=len) goto L_ENTER_WRITING_SYNC;
		break;

L_ENTER_WRITING_SYNC:
		WriteCommand=emString::Format(
			"\n(%s) print\nflush\n",
			SyncString
		);
		WriterPos=0;
		WriterState=WRITING_SYNC;
	case WRITING_SYNC:
		buf=WriteCommand.Get();
		len=strlen(buf);
		if (WriterPos>=len) WriterState=WRITING_FINISHED;
		break;

	default:
		buf=NULL;
		len=0;
	}

	len=Process.TryWrite(buf+WriterPos,len-WriterPos);
	if (len<0) {
		throw emString(
			"PostScript interpretation failed: Interpreter closed STDIN or exited."
		);
	}
	if (len==0) return false;
	WriterPos+=len;
	return true;
}


void emPsRenderer::PrepareReadingStartup()
{
	ReaderState=READING_SYNC;
	ReadBufferFill=0;
	RdSyncSearchPos=0;
}


void emPsRenderer::PrepareReadingPage()
{
	ReaderState=READING_IMAGE_HEADER;
	ReadBufferFill=0;
	RdSyncSearchPos=0;
}


bool emPsRenderer::TryRead() throw(emString)
{
	int len,syncLen,eat,r;
	bool syncFound;
	const char * p;

	if (ReadBufferFill>=(int)sizeof(ReadBuffer)) {
		throw emString("PostScript interpretation failed: Read buffer too small.");
	}
	len=Process.TryRead(
		ReadBuffer+ReadBufferFill,
		sizeof(ReadBuffer)-ReadBufferFill
	);
	if (len<0) {
		throw emString(
			"PostScript interpretation failed: Interpreter closed STDOUT or exited."
		);
	}
	if (len==0) return false;
	ReadBufferFill+=len;

	syncLen=strlen(SyncString);
	syncFound=false;
	while (RdSyncSearchPos+syncLen<=ReadBufferFill) {
		p=(const char*)memchr(
			ReadBuffer+RdSyncSearchPos,
			SyncString[0],
			ReadBufferFill-RdSyncSearchPos
		);
		if (!p) {
			RdSyncSearchPos=ReadBufferFill;
			break;
		}
		RdSyncSearchPos=p-ReadBuffer;
		if (RdSyncSearchPos+syncLen>ReadBufferFill) break;
		if (memcmp(ReadBuffer+RdSyncSearchPos,SyncString,syncLen)==0) {
			syncFound=true;
			break;
		}
		RdSyncSearchPos++;
	}

	eat=0;
	len=RdSyncSearchPos;
	switch (ReaderState) {
	case READING_IMAGE_HEADER:
		while (len>0) {
			r=ParseImageHeader(ReadBuffer+eat,len-eat);
			if (r<0) {
				eat++;
			}
			else if (r==0) {
				if (len-eat>1024) eat++;
				else break;
			}
			else {
				eat+=r;
				goto L_ENTER_READING_IMAGE_DATA;
			}
		}
		break;

L_ENTER_READING_IMAGE_DATA:
		RdImgX=0;
		RdImgY=0;
		RdImgDone=false;
		ReaderState=READING_IMAGE_DATA;
	case READING_IMAGE_DATA:
		while (len>0) {
			r=ParseImageData(ReadBuffer+eat,len-eat);
			if (r<0) {
				throw emString(
					"PostScript interpretation failed: Image data confusion."
				);
			}
			else if (r==0) {
				if (len-eat>1024) {
					throw emString(
						"PostScript interpretation failed: Image parser faulty."
					);
				}
				break;
			}
			else {
				eat+=r;
			}
			if (RdImgDone) goto L_ENTER_READING_SYNC;
		}
		break;

L_ENTER_READING_SYNC:
		ReaderState=READING_SYNC;
	case READING_SYNC:
		if (syncFound) {
			ReaderState=READING_FINISHED;
			eat=ReadBufferFill;
		}
		else {
			eat=RdSyncSearchPos;
		}
		break;

	default:
		eat=ReadBufferFill;
		break;
	}

	if (syncFound && ReaderState!=READING_FINISHED) {
		throw emString(
			"PostScript interpretation failed: Unsupported document structure."
		);
	}

	if (eat>0) {
		ReadBufferFill-=eat;
		RdSyncSearchPos-=eat;
		if (RdSyncSearchPos<0) RdSyncSearchPos=0;
		if (ReadBufferFill>0) {
			memmove(ReadBuffer,ReadBuffer+eat,ReadBufferFill);
		}
		else {
			ReadBufferFill=0;
		}
	}

	return true;
}


int emPsRenderer::ParseImageHeader(const char * buf, int len)
{
	int i,r;

	i=0;

	if (i>=len) return 0;
	if (buf[i++]!='P') return -1;

	if (i>=len) return 0;
	RdImgFormat=buf[i++]-'0';
	if (RdImgFormat<1 || RdImgFormat>6) return -1;

	r=ParseImageDecimal(buf+i,len-i,&RdImgW);
	if (r<=0) return r;
	if (RdImgW<1) return -1;
	i+=r;

	r=ParseImageDecimal(buf+i,len-i,&RdImgH);
	if (r<=0) return r;
	if (RdImgH<1) return -1;
	i+=r;

	if (RdImgFormat!=1 && RdImgFormat!=4) {
		r=ParseImageDecimal(buf+i,len-i,&RdImgMaxVal);
		if (r<=0) return r;
		if (RdImgMaxVal<1 || RdImgMaxVal>65535) return -1;
		i+=r;
	}
	else {
		RdImgMaxVal=1;
	}

	if (i>=len) return 0;
	if (buf[i++]!=0x0a) return -1;
	return i;
}


int emPsRenderer::ParseImageDecimal(
	const char * buf, int len, int * pNumber
)
{
	int i,c,n;

	for (i=0;;) {
		if (i>=len) return 0;
		c=(unsigned char)buf[i++];
		if (c>='0' && c<='9') break;
		if (c=='#') {
			do {
				if (i>=len) return 0;
				c=(unsigned char)buf[i++];
			} while (c!=0x0a && c!=0x0d);
		}
		else if (c>0x20) return -1;
	}
	n=c-'0';
	for (;;) {
		if (i>=len) return 0;
		c=(unsigned char)buf[i++];
		if (c>='0' && c<='9') {
			n=n*10+(c-'0');
		}
		else {
			*pNumber=n;
			return i-1;
		}
	}
}


int emPsRenderer::ParseImageData(const char * buf, int len)
{
	emImage * img;
	int eat,w,d;
	bool landscape;
	emByte * p;
	const emByte * s, * se;

	if (RdImgFormat!=6 || RdImgMaxVal!=255) return -1;

	if (CurrentJob) {
		landscape=CurrentDocument.IsLandscapePage(CurrentPageIndex);
		img=CurrentJob->Image;
		if (img) {
			if (landscape) {
				if (img->GetWidth()!=RdImgH || img->GetHeight()!=RdImgW) {
					return -1;
				}
			}
			else {
				if (img->GetWidth()!=RdImgW || img->GetHeight()!=RdImgH) {
					return -1;
				}
			}
			if (img->GetChannelCount()!=3) {
				emFatalError("emPsRenderer: Output image must have 3 channels.");
			}
		}
	}
	else {
		img=NULL;
		landscape=false;
	}

	for (eat=0;;) {
		w=(len-eat)/3;
		if (w>RdImgW-RdImgX) w=RdImgW-RdImgX;
		if (w<=0) break;
		if (img) {
			if (landscape) {
				s=(const emByte*)buf+eat;
				se=s+3*w;
				p=img->GetWritableMap()+(RdImgX*RdImgH+RdImgH-1-RdImgY)*3;
				d=RdImgH*3;
				do {
					p[0]=s[0];
					p[1]=s[1];
					p[2]=s[2];
					p+=d;
					s+=3;
				} while (s<se);
			}
			else {
				memcpy(
					img->GetWritableMap()+(RdImgY*RdImgW+RdImgX)*3,
					buf+eat,
					w*3
				);
			}
		}
		eat+=w*3;
		RdImgX+=w;
		if (RdImgX>=RdImgW) {
			RdImgX=0;
			RdImgY++;
			if (RdImgY>=RdImgH) {
				RdImgDone=true;
				break;
			}
		}
	}
	return eat;
}


emPsRenderer::PSAgentClass::PSAgentClass(emPsRenderer & interpreter)
	: emPriSchedAgent(interpreter.GetRootContext(),"cpu"),
	Renderer(interpreter)

{
}


void emPsRenderer::PSAgentClass::GotAccess()
{
	Renderer.WakeUp();
}


const char * const emPsRenderer::SyncString=
	"SYNC823JVG73LS0GJ7B2TX2M49GZWK2D" // Just random
;
