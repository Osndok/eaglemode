//------------------------------------------------------------------------------
// emPdfServerModel.cpp
//
// Copyright (C) 2011,2014,2017-2019,2022-2024 Oliver Hamann.
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


emRef<emPdfServerModel> emPdfServerModel::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emPdfServerModel,rootContext,"")
}


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


emPdfServerModel::PageAreas::PageAreas(const PageAreas & pageAreas)
	: TextRects(pageAreas.TextRects),
	UriRects(pageAreas.UriRects),
	RefRects(pageAreas.RefRects)
{
}


emPdfServerModel::PageAreas::~PageAreas()
{
}


emPdfServerModel::PageAreas & emPdfServerModel::PageAreas::operator = (
	const PageAreas & pageAreas
)
{
	TextRects=pageAreas.TextRects;
	UriRects=pageAreas.UriRects;
	RefRects=pageAreas.RefRects;
	return *this;
}


emPdfServerModel::PdfInstance::~PdfInstance()
{
	if (PdfServerModel) {
		if (InstanceId!=-1) {
			emRef<CloseJob> job=new CloseJob(ProcRunId,InstanceId);
			PdfServerModel->EnqueueJob(*job);
		}
		PdfServerModel->ProcPdfInstCount--;
	}
}


emPdfServerModel::PdfInstance::PdfInstance(emPdfServerModel & pdfServerModel)
	: PdfServerModel(&pdfServerModel),
	ProcRunId(0),
	InstanceId(-1)
{
	pdfServerModel.ProcPdfInstCount++;
}


emPdfServerModel::PdfJobBase::~PdfJobBase()
{
}


emPdfServerModel::PdfJobBase::PdfJobBase(
	PdfInstance * pdfInstance, bool costly, int classPriority, double priority
)
	: emJob(priority),
	PdfInst(pdfInstance),
	Costly(costly),
	ClassPriority(classPriority)
{
}


void emPdfServerModel::PdfJobBase::SetPdfInstance(PdfInstance * pdfInstance)
{
	PdfInst=pdfInstance;
}


void emPdfServerModel::PdfJobBase::SetClassPriority(int classPriority)
{
	ClassPriority=classPriority;
}


emPdfServerModel::OpenJob::OpenJob(const emString & filePath, double priority)
	: PdfJobBase(NULL,true,3,priority),
	FilePath(filePath)
{
}


emPdfServerModel::OpenJob::~OpenJob()
{
}


bool emPdfServerModel::OpenJob::Send(emPdfServerModel & mdl, emString & err)
{
	mdl.WriteLineToProc(emString::Format("open %s",FilePath.Get()));
	return true;
}


emPdfServerModel::PdfJobBase::RcvRes emPdfServerModel::OpenJob::TryReceive(
	emPdfServerModel & mdl, emString & err
)
{
	PdfInstance * inst;
	emString cmd,args;
	const char * p;
	double d1,d2;
	int l,i1,r,pos;

	args=mdl.ReadLineFromProc();
	if (args.IsEmpty()) return RCV_WAIT;
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

	inst=GetPdfInstance();
	if (!inst) {
		inst=new PdfInstance(mdl);
		SetPdfInstance(inst);
	}

	if (cmd=="error:") {
		err=args;
		return RCV_ERROR;
	}
	else if (cmd=="instance:") {
		r=sscanf(args,"%d",&i1);
		if (r<1) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		inst->ProcRunId=mdl.ProcRunId;
		inst->InstanceId=i1;
	}
	else if (cmd=="title:") {
		inst->Document.Title=Unquote(args);
	}
	else if (cmd=="author:") {
		inst->Document.Author=Unquote(args);
	}
	else if (cmd=="subject:") {
		inst->Document.Subject=Unquote(args);
	}
	else if (cmd=="keywords:") {
		inst->Document.Keywords=Unquote(args);
	}
	else if (cmd=="creator:") {
		inst->Document.Creator=Unquote(args);
	}
	else if (cmd=="producer:") {
		inst->Document.Producer=Unquote(args);
	}
	else if (cmd=="creation_date:") {
		inst->Document.CreationDate=atol(args.Get());
	}
	else if (cmd=="modification_date:") {
		inst->Document.ModificationDate=atol(args.Get());
	}
	else if (cmd=="version:") {
		inst->Document.Version=Unquote(args);
	}
	else if (cmd=="pages:") {
		r=sscanf(args,"%d",&i1);
		if (r<1) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		inst->Pages.SetCount(i1);
	}
	else if (cmd=="pageinfo:") {
		r=sscanf(args,"%d %lf %lf %n",&i1,&d1,&d2,&pos);
		if (
			r<3 || pos<=0 || i1<0 ||
			i1>=inst->Pages.GetCount()
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		inst->Pages.GetWritable(i1).Width=d1;
		inst->Pages.GetWritable(i1).Height=d2;
		inst->Pages.GetWritable(i1).Label=Unquote(args.Get()+pos);
	}
	else if (cmd=="ok") {
		inst->ProcRunId=mdl.ProcRunId;
		return RCV_SUCCESS;
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}

	return RCV_CONTINUE;
}


emPdfServerModel::GetAreasJob::GetAreasJob(
	PdfInstance & pdfInstance, int page, double priority
)
	: PdfJobBase(&pdfInstance,true,2,priority),
	Page(page)
{
}


emPdfServerModel::GetAreasJob::~GetAreasJob()
{
}


bool emPdfServerModel::GetAreasJob::Send(emPdfServerModel & mdl, emString & err)
{
	if (GetPdfInstance()->GetProcRunId()!=mdl.ProcRunId) {
		err="PDF server process restarted";
		return false;
	}
	mdl.WriteLineToProc(emString::Format(
		"get_areas %d %d",
		GetPdfInstance()->GetInstanceId(),
		Page
	));
	return true;
}


emPdfServerModel::PdfJobBase::RcvRes emPdfServerModel::GetAreasJob::TryReceive(
	emPdfServerModel & mdl, emString & err
)
{
	emString cmd,args;
	const char * p;
	int l,r,x1,y1,x2,y2,type,pos;

	args=mdl.ReadLineFromProc();
	if (args.IsEmpty()) return RCV_WAIT;
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
		err=args;
		return RCV_ERROR;
	}
	else if (cmd=="rect:") {
		r=sscanf(args,"%d %d %d %d %d%n",&x1,&y1,&x2,&y2,&type,&pos);
		if (
			r<5 || pos<=0 || type<0 || type>2 ||
			(type!=0 && args[pos]!=' ')
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		if (type==0) {
			Areas.TextRects.AddNew();
			TextRect & tr=Areas.TextRects.GetWritable(Areas.TextRects.GetCount()-1);
			tr.X1=x1;
			tr.Y1=y1;
			tr.X2=x2;
			tr.Y2=y2;
		}
		else if (type==1) {
			Areas.UriRects.AddNew();
			UriRect & ur=Areas.UriRects.GetWritable(Areas.UriRects.GetCount()-1);
			ur.X1=x1;
			ur.Y1=y1;
			ur.X2=x2;
			ur.Y2=y2;
			ur.Uri=Unquote(args.Get()+pos+1);
		}
		else if (type==2) {
			Areas.RefRects.AddNew();
			RefRect & rr=Areas.RefRects.GetWritable(Areas.RefRects.GetCount()-1);
			rr.X1=x1;
			rr.Y1=y1;
			rr.X2=x2;
			rr.Y2=y2;
			r=sscanf(args.Get()+pos+1,"%d %d",&rr.TargetPage,&rr.TargetY);
			if (r<2) {
				throw emException("PDF server protocol error (%d)",__LINE__);
			}
		}
		return RCV_CONTINUE;
	}
	else if (cmd=="ok") {
		return RCV_SUCCESS;
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}
}


emPdfServerModel::GetSelectedTextJob::GetSelectedTextJob(
	PdfInstance & pdfInstance, int page, SelectionStyle style,
	double selX1, double selY1, double selX2, double selY2, double priority
)
	: PdfJobBase(&pdfInstance,false,5,priority),
	Page(page),
	Style(style),
	SelX1(selX1),
	SelY1(selY1),
	SelX2(selX2),
	SelY2(selY2)
{
}


emPdfServerModel::GetSelectedTextJob::~GetSelectedTextJob()
{
}


bool emPdfServerModel::GetSelectedTextJob::Send(emPdfServerModel & mdl, emString & err)
{
	if (GetPdfInstance()->GetProcRunId()!=mdl.ProcRunId) {
		err="PDF server process restarted";
		return false;
	}
	mdl.WriteLineToProc(emString::Format(
		"get_selected_text %d %d %d %.16g %.16g %.16g %.16g",
		GetPdfInstance()->GetInstanceId(),
		Page,
		Style,
		SelX1,
		SelY1,
		SelX2,
		SelY2
	));
	return true;
}


emPdfServerModel::PdfJobBase::RcvRes emPdfServerModel::GetSelectedTextJob::TryReceive(
	emPdfServerModel & mdl, emString & err
)
{
	emString cmd,args;
	const char * p;
	int l;

	args=mdl.ReadLineFromProc();
	if (args.IsEmpty()) return RCV_WAIT;
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
		err=args;
		return RCV_ERROR;
	}
	else if (cmd=="selected_text:") {
		SelectedText=Unquote(args);
		return RCV_SUCCESS;
	}
	else {
		throw emException("PDF server protocol error (%d)",__LINE__);
	}
}


emPdfServerModel::RenderJob::RenderJob(
	PdfInstance & pdfInstance, int page, double srcX, double srcY,
	double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
	double priority
)
	: PdfJobBase(&pdfInstance,true,1,priority),
	Page(page),
	SrcX(srcX),
	SrcY(srcY),
	SrcWidth(srcWidth),
	SrcHeight(srcHeight),
	TgtW(tgtWidth),
	TgtH(tgtHeight),
	ReadStage(0),
	ReadPos(0),
	IsRenderSelectionJob(false)
{
}


emPdfServerModel::RenderJob::~RenderJob()
{
}


bool emPdfServerModel::RenderJob::Send(emPdfServerModel & mdl, emString & err)
{
	if (GetPdfInstance()->GetProcRunId()!=mdl.ProcRunId) {
		err="PDF server process restarted";
		return false;
	}
	mdl.WriteLineToProc(emString::Format(
		"render %d %d %.16g %.16g %.16g %.16g %d %d",
		GetPdfInstance()->GetInstanceId(),
		Page,
		SrcX,
		SrcY,
		SrcWidth,
		SrcHeight,
		TgtW,
		TgtH
	));
	return true;
}


emPdfServerModel::PdfJobBase::RcvRes emPdfServerModel::RenderJob::TryReceive(
	emPdfServerModel & mdl, emString & err
)
{
	int len,total,type,channels,width,height,maxColor;
	emString line;
	const char * p;

	if (ReadStage==0) {
		if (mdl.ReadBuf.IsEmpty()) return RCV_WAIT;
		if (mdl.ReadBuf[0]!='P') {
			line=mdl.ReadLineFromProc();
			if (line.IsEmpty()) return RCV_WAIT;
			p="error: ";
			len=strlen(p);
			if (line.GetSubString(0,len)!=p) {
				throw emException("PDF server protocol error (%d)",__LINE__);
			}
			line.Remove(0,len);
			err=line;
			return RCV_ERROR;
		}
		len=TryParsePnmHeader(
			mdl.ReadBuf.Get(),mdl.ReadBuf.GetCount(),&type,&width,&height,&maxColor
		);
		if (len<=0) return RCV_WAIT;
		emDLog("emPdfServerModel: Receiving: P%c %d %d %d ...",type,width,height,maxColor);
		mdl.ReadBuf.Remove(0,len);
		if (
			type!=(IsRenderSelectionJob?'X':'6') ||
			width!=TgtW || height!=TgtH || maxColor!=255
		) {
			throw emException("PDF server protocol error (%d)",__LINE__);
		}
		ReadStage=1;
	}

	if (mdl.ReadBuf.IsEmpty()) return RCV_WAIT;
	channels=IsRenderSelectionJob?2:3;
	total=TgtW*TgtH*channels;
	len=mdl.ReadBuf.GetCount();
	if (len>total-ReadPos) len=total-ReadPos;
	if (GetRefCount()>1) {
		if (
			Image.GetWidth()!=TgtW ||
			Image.GetHeight()!=TgtH ||
			Image.GetChannelCount()!=channels
		) {
			Image.Setup(TgtW,TgtH,channels);
		}
		memcpy(
			Image.GetWritableMap()+ReadPos,
			mdl.ReadBuf.Get(),
			len
		);
	}
	mdl.ReadBuf.Remove(0,len);
	ReadPos+=len;
	if (ReadPos>=total) return RCV_SUCCESS;
	return RCV_CONTINUE;
}


emPdfServerModel::RenderSelectionJob::RenderSelectionJob(
	PdfInstance & pdfInstance, int page, double srcX, double srcY,
	double srcWidth, double srcHeight, int tgtWidth, int tgtHeight,
	SelectionStyle style, double selX1, double selY1, double selX2,
	double selY2, double priority
)
	: RenderJob(
		pdfInstance,page,srcX,srcY,srcWidth,srcHeight,
		tgtWidth,tgtHeight,priority
	),
	Style(style),
	SelX1(selX1),
	SelY1(selY1),
	SelX2(selX2),
	SelY2(selY2)
{
	SetClassPriority(4);
	IsRenderSelectionJob=true;
}


bool emPdfServerModel::RenderSelectionJob::Send(emPdfServerModel & mdl, emString & err)
{
	if (GetPdfInstance()->GetProcRunId()!=mdl.ProcRunId) {
		err="PDF server process restarted";
		return false;
	}
	mdl.WriteLineToProc(emString::Format(
		"render_selection %d %d %.16g %.16g %.16g %.16g %d %d %d %.16g %.16g %.16g %.16g",
		GetPdfInstance()->GetInstanceId(),
		Page,
		SrcX,
		SrcY,
		SrcWidth,
		SrcHeight,
		TgtW,
		TgtH,
		Style,
		SelX1,
		SelY1,
		SelX2,
		SelY2
	));
	return true;
}


void emPdfServerModel::EnqueueJob(PdfJobBase & job)
{
	JobQueue.EnqueueJob(job);
	WakeUp();
}


void emPdfServerModel::AbortJob(PdfJobBase & job)
{
	if (job.GetState() != emJob::ST_RUNNING) {
		JobQueue.AbortJob(job);
	}
}


void emPdfServerModel::Poll(unsigned maxMillisecs)
{
	emUInt64 endTime,now;
	int flags;

	if (JobQueue.IsEmpty()) {
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


emPdfServerModel::emPdfServerModel(emContext & context, const emString & name)
	: emModel(context,name),
	JobQueue(GetScheduler())
{
	ProcRunId=0;
	ProcPdfInstCount=0;
	ProcIdleClock=0;
	ProcTerminating=false;
	ReadBuf.SetTuningLevel(4);
	WriteBuf.SetTuningLevel(4);
	SetMinCommonLifetime(10);
	SetEnginePriority(LOW_PRIORITY);
}


emPdfServerModel::~emPdfServerModel()
{
	Process.Terminate();
}


bool emPdfServerModel::Cycle()
{
	bool busy;

	busy=emModel::Cycle();

	Poll(IsTimeSliceAtEnd()?0:10);

	if (
		!JobQueue.IsEmpty() || !WriteBuf.IsEmpty() ||
		(Process.IsRunning() && !ProcPdfInstCount)
	) busy=true;

	return busy;
}


emPdfServerModel::CloseJob::CloseJob(emUInt64 procRunId, int instanceId)
	: PdfJobBase(NULL,false,6,0.0),
	ProcRunId(procRunId),
	InstanceId(instanceId)
{
}


bool emPdfServerModel::CloseJob::Send(emPdfServerModel & mdl, emString & err)
{
	if (ProcRunId!=mdl.ProcRunId) {
		err="PDF server process restarted";
		return false;
	}
	mdl.WriteLineToProc(emString::Format("close %d",InstanceId));
	return true;
}


emPdfServerModel::PdfJobBase::RcvRes emPdfServerModel::CloseJob::TryReceive(
	emPdfServerModel &, emString &
)
{
	return RCV_SUCCESS;
}


emPdfServerModel::PdfJobQueue::PdfJobQueue(emScheduler & scheduler)
	: emJobQueue(scheduler)
{
}


int emPdfServerModel::PdfJobQueue::CompareForSortingOfWaitingJobs(
	emJob & job1, emJob & job2
) const
{
	int c1,c2;

	c1=((const PdfJobBase&)job1).GetClassPriority();
	c2=((const PdfJobBase&)job2).GetClassPriority();
	if (c1!=c2) return c2-c1;
	return emJobQueue::CompareForSortingOfWaitingJobs(job1,job2);
}


void emPdfServerModel::TryStartJobs()
{
	PdfJobBase * job;
	int costlyJobs;
	emString err;

	costlyJobs=0;
	for (
		job=(PdfJobBase*)JobQueue.GetFirstRunningJob();
		job;
		job=(PdfJobBase*)job->GetNext()
	) {
		if (job->Costly) costlyJobs++;
	}

	while (costlyJobs<4) {
		job=(PdfJobBase*)JobQueue.StartNextJob();
		if (!job) break;
		err.Clear();
		if (job->Send(*this,err)) {
			if (job->Costly) costlyJobs++;
		}
		else {
			JobQueue.FailJob(*job,err);
		}
	}
}


void emPdfServerModel::TryFinishJobs()
{
	PdfJobBase * job;
	PdfJobBase::RcvRes res;
	emString err;

	for (;;) {
		job=(PdfJobBase*)JobQueue.GetFirstRunningJob();
		if (!job) break;
		err.Clear();
		res=job->TryReceive(*this,err);
		if (res==PdfJobBase::RCV_SUCCESS) {
			JobQueue.SucceedJob(*job);
		}
		else if (res==PdfJobBase::RCV_ERROR) {
			JobQueue.FailJob(*job,err);
		}
		else if (res==PdfJobBase::RCV_WAIT) {
			break;
		}
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
