//------------------------------------------------------------------------------
// emAvServerModel.cpp
//
// Copyright (C) 2008-2010 Oliver Hamann.
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
#include <emAv/emAvServerModel.h>
#include <emAv/emAvClient.h>


emRef<emAvServerModel> emAvServerModel::Acquire(
	emRootContext & rootContext, const emString & serverProcPath
)
{
	EM_IMPL_ACQUIRE_COMMON(emAvServerModel,rootContext,serverProcPath)
}


emAvServerModel::emAvServerModel(
	emContext & context, const emString & serverProcPath
)
	: emModel(context,serverProcPath),
	StateTimer(GetScheduler())
{
	int i;

	SetMinCommonLifetime(MIN_LIFETIME_MILLISECS/1000);

	for (i=0; i<MAX_INSTANCES; i++) Instances[i]=NULL;
	InstanceCount=0;
	State=STATE_IDLE;
	InBuf.SetTuningLevel(4);
	OutBuf.SetTuningLevel(4);
	InBufFill=0;
	OutBufFill=0;
	OutBufOverflowed=false;
}


emAvServerModel::~emAvServerModel()
{
	char buf[256];
	int i,r;

	if (ServerProc.IsRunning()) {
		ServerProc.CloseWriting();
		for (i=0; i<DTOR_PROC_TERM_TIMEOUT/10; i++) {
			if (!ServerProc.IsRunning()) break;
			try {
				r=ServerProc.TryRead(buf,sizeof(buf));
			}
			catch (emString) {
				break;
			}
			if (r>0) continue;
			if (r<0) break;
			ServerProc.WaitPipes(emProcess::WF_WAIT_STDOUT,10);
		}
		if (ServerProc.IsRunning()) {
			emDLog("emAvServerModel::~emAvServerModel: Server process did not terminate properly - sending a signal.");
			ServerProc.Terminate();
		}
		else {
			emDLog("emAvServerModel::~emAvServerModel: Server process terminated properly.");
		}
	}

	for (i=0; i<MAX_INSTANCES; i++) DeleteInstance(i);
}


bool emAvServerModel::Cycle()
{
	emString termProcReason;
	char buf[256];
	int i,r;

	switch (State) {

L_ENTER_IDLE:
		State=STATE_IDLE;
	case STATE_IDLE:
		if (InstanceCount>0 || OutBufFill>0) {
			try {
				ServerProc.TryStart(
					emArray<emString>(GetName()),
					emArray<emString>(),
					NULL,
					emProcess::SF_PIPE_STDIN|
					emProcess::SF_PIPE_STDOUT|
					emProcess::SF_SHARE_STDERR
				);
			}
			catch (emString errorMessage) {
				termProcReason=errorMessage;
				goto L_ENTER_TERM_PROC;
			}
			goto L_ENTER_BUSY;
		}
		break;

L_ENTER_BUSY:
		State=STATE_BUSY;
	case STATE_BUSY:
		if (!ServerProc.IsRunning()) {
			termProcReason="Server process died unexpectedly.";
			goto L_ENTER_TERM_PROC;
		}
		if (OutBufOverflowed) {
			termProcReason="Server process seems to hang (buffer overflowed).";
			goto L_ENTER_TERM_PROC;
		}
		try {
			TryDoPipeIO();
		}
		catch (emString errorMessage) {
			termProcReason=errorMessage;
			goto L_ENTER_TERM_PROC;
		}
		if (InstanceCount<=0 && OutBufFill<=0) {
			goto L_ENTER_HOLD_PROC;
		}
		TransferFrames();
		break;

L_ENTER_HOLD_PROC:
		StateTimer.Start(PROC_HOLD_MILLISECS);
		State=STATE_HOLD_PROC;
	case STATE_HOLD_PROC:
		if (InstanceCount>0 || OutBufFill>0) goto L_ENTER_BUSY;
		else if (!StateTimer.IsRunning()) {
			termProcReason="Server process no longer needed.";
			goto L_ENTER_TERM_PROC;
		}
		break;

L_ENTER_TERM_PROC:
		for (i=0; i<MAX_INSTANCES; i++) {
			if (Instances[i]) {
				if (Instances[i]->Client) {
					Instances[i]->Client->SetStreamErrored(termProcReason);
				}
				Instances[i]->OldProc=true;
			}
		}
		InBuf.Empty(true);
		OutBuf.Empty(true);
		InBufFill=0;
		OutBufFill=0;
		OutBufOverflowed=false;
		ServerProc.CloseWriting();
		StateTimer.Start(NORMAL_PROC_TERM_TIMEOUT);
		State=STATE_TERM_PROC;
	case STATE_TERM_PROC:
		while (ServerProc.IsRunning()) {
			try {
				r=ServerProc.TryRead(buf,sizeof(buf));
			}
			catch (emString) {
				break;
			}
			if (r<=0) break;
		}
		if (!ServerProc.IsRunning()) {
			emDLog("emAvServerModel::Cycle: Server process terminated properly.");
			for (i=0; i<MAX_INSTANCES; i++) {
				if (Instances[i] && Instances[i]->OldProc) {
					DeleteInstance(i);
				}
			}
			goto L_ENTER_IDLE;
		}
		if (!StateTimer.IsRunning()) {
			emDLog("emAvServerModel::Cycle: Server process did not terminate properly - sending a signal.");
			ServerProc.CloseReading();
			ServerProc.SendTerminationSignal();
			termProcReason="Server process hangs.";
			goto L_ENTER_TERM_PROC;
		}
		break;

	}

	return State!=STATE_IDLE;
}


emAvServerModel::Instance * emAvServerModel::TryOpenInstance(
	const char * audioDrv, const char * videoDrv, const char * filePath
) throw(emString)
{
	Instance * inst;
	int i;

	for (i=0; ; i++) {
		if (i>=MAX_INSTANCES) {
			throw emString("Too many emAvServer clients.");
		}
		if (!Instances[i]) break;
	}
	inst=new Instance;
	inst->Index=i;
	inst->OldProc=false;
	inst->Client=NULL;
	inst->ShmAttachState=SA_DETACHED;
	inst->MinShmSize=0;
	inst->ShmSize=0;
	inst->ShmId=-1;
	inst->ShmAddr=NULL;
	Instances[i]=inst;
	InstanceCount++;
	if (State==STATE_IDLE) WakeUp();
	SendMessage(
		inst,
		"open",
		emString::Format("%s:%s:%s",audioDrv,videoDrv,filePath)
	);
	return inst;
}


void emAvServerModel::DeleteInstance(int index)
{
	Instance * inst;

	inst=Instances[index];
	if (!inst) return;
	DeleteShm(inst);
	delete inst;
	Instances[index]=NULL;
	InstanceCount--;
}


void emAvServerModel::SendMessage(
	Instance * inst, const char * tag, const char * data
)
{
	char instStr[64];
	int newFill,l1,l2,l3;
	char * p;

	if (OutBufOverflowed) return;

	emDLog("emAvServerModel: client->server: %d:%s:%s",inst->Index,tag,data);

	sprintf(instStr,"%d",inst->Index);
	l1=strlen(instStr);
	l2=strlen(tag);
	l3=data ? strlen(data) : 0;

	newFill=OutBufFill+l1+1+l2;
	if (data) newFill+=1+l3;
	newFill+=1;

	if (newFill>MAX_OUT_BUF_SIZE) {
		OutBufOverflowed=true;
		return;
	}

	if (OutBuf.GetCount()<newFill) OutBuf.SetCount(newFill,true);

	p=OutBuf.GetWritable()+OutBufFill;
	memcpy(p,instStr,l1);
	p+=l1;
	*p++=':';
	memcpy(p,tag,l2);
	p+=l2;
	if (data) {
		*p++=':';
		memcpy(p,data,l3);
		p+=l3;
	}
	*p++='\n';

	OutBufFill=newFill;
}


void emAvServerModel::TryDoPipeIO() throw(emString)
{
	char * p, * p2, * p3, * p4, * pb, * pe;
	bool progress;
	int l,r,i;

	if (InBuf.GetCount()<MAX_IN_BUF_SIZE) InBuf.SetCount(MAX_IN_BUF_SIZE,true);

	do {
		progress=false;

		l=InBuf.GetCount()-InBufFill;
		if (l>0) {
			p=InBuf.GetWritable()+InBufFill;
			r=ServerProc.TryRead(p,l);
			if (r) {
				if (r<0) {
					throw emString(
						"Server process died unexpectedly or closed STDOUT."
					);
				}
				InBufFill+=r;
				progress=true;
			}
		}

		if (progress) {
			pb=InBuf.GetWritable();
			pe=pb+InBufFill;
			p=pb;
			for (;;) {
				while (p<pe && (unsigned char)*p<=0x20) p++;
				for (p4=p; p4<pe; p4++) {
					if (*p4==0x0a || *p4==0x0d) break;
				}
				if (p4>=pe) {
					if (p>pb) {
						InBufFill-=(p-pb);
						if (InBufFill>0) memmove(pb,p,InBufFill);
					}
					break;
				}
				p2=(char*)memchr(p,':',p4-p);
				if (!p2) throw emString("emAvServerModel: Protocol error.\n");
				*p2++=0;
				p3=(char*)memchr(p2,':',p4-p2);
				if (p3) *p3++=0; else p3=p4;
				*p4++=0;
				for (i=0; p3[i]; i++) {
					if (p3[i]==0x1a) p3[i]=0x0a;
					if (p3[i]==0x1d) p3[i]=0x0d;
				}
				HandleMessage(atoi(p),p2,p3);
				p=p4;
			}
		}

		l=OutBufFill;
		if (l>0) {
			p=OutBuf.GetWritable();
			r=ServerProc.TryWrite(p,l);
			if (r) {
				if (r<0) {
					throw emString(
						"Server process died unexpectedly or closed STDIN."
					);
				}
				if (r<l) memmove(p,p+r,l-r);
				OutBufFill-=r;
				progress=true;
			}
		}

	} while (progress);
}


void emAvServerModel::HandleMessage(
	int instIndex, const char * tag, const char * data
)
{
	emString name,value;
	const char * p;
	Instance * inst;

	emDLog("emAvServerModel: server->client: %d:%s:%s",instIndex,tag,data);

	if (instIndex<0 || instIndex>=MAX_INSTANCES) return;
	inst=Instances[instIndex];
	if (!inst) return;

	if (strcmp(tag,"set")==0) {
		if (inst->Client) {
			p=strchr(data,':');
			if (!p) { name=data; value=""; }
			else { name=emString(data,p-data); value=p+1; }
			inst->Client->SetProperty(name,value,true);
		}
	}
	else if (strcmp(tag,"ok")==0) {
		if (strlen(data)>=4 && memcmp(data,"set:",4)==0) {
			if (inst->Client) {
				name=data+4;
				inst->Client->PropertyOKFromServer(name);
			}
		}
		else if (strcmp(data,"open")==0) {
			if (inst->Client) {
				inst->Client->SetStreamOpened();
			}
		}
		else if (strcmp(data,"close")==0) {
			inst->ShmAttachState=SA_DETACHED;
			if (!inst->Client) {
				DeleteInstance(instIndex);
			}
		}
		else if (strcmp(data,"attachshm")==0) {
			inst->ShmAttachState=SA_ATTACHED;
			UpdateShm(inst);
		}
		else if (strcmp(data,"detachshm")==0) {
			inst->ShmAttachState=SA_DETACHED;
			UpdateShm(inst);
		}
		else {
			emDLog(
				"emAvServerModel::HandleMessage: Unsupported ok tag \"%s\".",
				data
			);
		}
	}
	else if (strcmp(tag,"minshmsize")==0) {
		inst->MinShmSize=atoi(data);
		UpdateShm(inst);
	}
	else if (strcmp(tag,"error")==0) {
		if (inst->Client) inst->Client->SetStreamErrored(data);
	}
	else {
		emDLog(
			"emAvServerModel::HandleMessage: Unsupported tag \"%s\".",
			name.Get()
		);
	}
}


void emAvServerModel::UpdateShm(Instance * inst)
{
	if (inst->ShmAttachState==SA_DETACHED) {
		if (inst->ShmSize<inst->MinShmSize) {
			DeleteShm(inst);
			inst->ShmSize=inst->MinShmSize;
		}
		if (inst->ShmSize>0 && inst->ShmId==-1 && inst->Client) {
			try {
				TryCreateShm(inst);
			}
			catch (emString errorMessage) {
				if (inst->Client) inst->Client->SetStreamErrored(errorMessage);
				return;
			}
			SendMessage(
				inst,"attachshm",
				emString::Format("%d:%d",inst->ShmId,inst->ShmSize)
			);
			inst->ShmAttachState=SA_ATTACHING;
		}
	}
	else if (inst->ShmAttachState==SA_ATTACHED) {
		if (inst->ShmSize<inst->MinShmSize || !inst->Client) {
			SendMessage(inst,"detachshm","");
			inst->ShmAttachState=SA_DETACHING;
		}
	}
}


void emAvServerModel::TryCreateShm(Instance * inst) throw(emString)
{
#if defined(__CYGWIN__)
	throw emString("shmget not tried as cygwin may abort the process.");
#endif
	inst->ShmId=shmget(IPC_PRIVATE,inst->ShmSize,IPC_CREAT|0600);
	if (inst->ShmId==-1) {
		throw emString::Format(
			"Failed to create shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}
	inst->ShmAddr=(int*)shmat(inst->ShmId,0,0);
	if (inst->ShmAddr==(int*)-1) {
		inst->ShmAddr=NULL;
		shmctl(inst->ShmId,IPC_RMID,0);
		inst->ShmId=-1;
		throw emString::Format(
			"Failed to attach shared memory segment: %s",
			emGetErrorText(errno).Get()
		);
	}
#if defined(__linux__)
	shmctl(inst->ShmId,IPC_RMID,0);
	if (shmctl(inst->ShmId,IPC_RMID,0)!=0) {
		emFatalError(
			"emAvServerModel: shmctl failed: %s",
			emGetErrorText(errno).Get()
		);
	}
#endif
	inst->ShmAddr[0]=0;
}


void emAvServerModel::DeleteShm(Instance * inst)
{
	if (inst->ShmId!=-1) {
		if (inst->ShmAttachState!=SA_DETACHED && ServerProc.IsRunning()) {
			emFatalError(
				"emAvServerModel: DeleteShm called while server not detached."
			);
		}
		if (inst->ShmAddr) {
			shmdt(inst->ShmAddr);
			inst->ShmAddr=NULL;
		}
#if !defined(__linux__)
		if (shmctl(inst->ShmId,IPC_RMID,0)!=0) {
			emFatalError(
				"emAvServerModel: shmctl failed: %s",
				emGetErrorText(errno).Get()
			);
		}
#endif
		inst->ShmId=-1;
	}
	inst->ShmSize=0;
}


void emAvServerModel::TransferFrames()
{
	Instance * inst;
	int i;

	//??? Too much polling - better have a list!
	for (i=0; i<MAX_INSTANCES; i++) {
		inst=Instances[i];
		if (inst && inst->ShmAddr && inst->ShmAddr[0]) {
			TransferFrame(inst);
			inst->ShmAddr[0]=0;
		}
	}
}


void emAvServerModel::TransferFrame(Instance * inst)
{
	int * shm;
	emByte * src, * src2, * src3, * map;
	emByte * t1, * t2, * t3, * s1, * s2, * s3, * s4;
	int width,height,aspectRatio,format,bpl,bpl2,width2,height2;
	int w,h,cy,cu,cv,cr,cg,cb,c;

	shm=inst->ShmAddr;

	width=shm[1];
	if (width<1 || width>4096) goto L_BAD_DATA;

	height=shm[2];
	if (height<1 || height>4096) goto L_BAD_DATA;

	aspectRatio=shm[3];
	if (aspectRatio<65536/64 || aspectRatio>65536*64) {
		aspectRatio=(width*65536+height/2)/height;
	}

	if (
		!inst->Image.IsEmpty() &&
		inst->Image.GetDataRefCount()>1 &&
		inst->Client
	) {
		//??? Dirty trick to avoid image map reallocation.
		inst->Client->ShowFrame(emImage(),65536.0/aspectRatio);
	}

	format=shm[4];

	if (format==0) { // RGB
		bpl=shm[5];
		if (bpl<3*width) goto L_BAD_DATA;
		if ((int)sizeof(int)*6+bpl*height>inst->ShmSize) goto L_BAD_DATA;
		w=width;
		h=height;
		if (
			inst->Image.GetWidth()!=w || inst->Image.GetHeight()!=h ||
			inst->Image.GetChannelCount()!=3
		) {
			inst->Image.Setup(w,h,3);
		}
		src=(emByte*)(shm+6);
		map=inst->Image.GetWritableMap();
		do {
			h--;
			memcpy(map+w*3*h,src+bpl*h,w*3);
		} while(h>0);
	}
	else if (format==1) { // YV12
		bpl=shm[5];
		bpl2=shm[6];
		height2=(height+1)/2;
		width2=(width+1)/2;
		if (bpl<width) goto L_BAD_DATA;
		if (bpl2<width2) goto L_BAD_DATA;
		if ((int)sizeof(int)*7+bpl*height+2*bpl2*height2>inst->ShmSize) goto L_BAD_DATA;
		w=width&~1;
		h=height&~1;
		if (w<=0 || h<=0) goto L_BAD_DATA;
		if (
			inst->Image.GetWidth()!=w || inst->Image.GetHeight()!=h ||
			inst->Image.GetChannelCount()!=3
		) {
			inst->Image.Setup(w,h,3);
		}
		w>>=1; h>>=1;
		src=(emByte*)(shm+7);
		src2=src+bpl*height;
		src3=src2+bpl2*height2;
		map=inst->Image.GetWritableMap();
		do {
			h--;
			s1=src+h*2*bpl;
			s2=s1+bpl;
			s3=src2+h*bpl2;
			s4=src3+h*bpl2;
			t1=map+h*2*w*6;
			t2=t1+w*6;
			t3=t2+w*6;
			do {
				cu=s3[0]-128;
				cv=s4[0]-128;
				cr=409*cv+128;
				cg=-100*cu-208*cv+128;
				cb=516*cu+128;

				cy=(s1[0]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[0]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[1]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[2]=(emByte)c;

				cy=(s1[1]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[3]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[4]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[5]=(emByte)c;

				cy=(s2[0]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[0]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[1]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[2]=(emByte)c;

				cy=(s2[1]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[3]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[4]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t2[5]=(emByte)c;

				s1+=2;
				s2+=2;
				s3++;
				s4++;
				t1+=6;
				t2+=6;
			} while (t2<t3);
		} while(h>0);
	}
	else if (format==2) { // YUY2
		bpl=shm[5];
		if (bpl<2*width) goto L_BAD_DATA;
		if ((int)sizeof(int)*6+bpl*height>inst->ShmSize) goto L_BAD_DATA;
		w=width&~1;
		h=height;
		if (w<=0) goto L_BAD_DATA;
		if (
			inst->Image.GetWidth()!=w || inst->Image.GetHeight()!=h ||
			inst->Image.GetChannelCount()!=3
		) {
			inst->Image.Setup(w,h,3);
		}
		w>>=1;
		src=(emByte*)(shm+6);
		map=inst->Image.GetWritableMap();
		do {
			h--;
			s1=src+h*bpl;
			t1=map+h*w*6;
			t2=t1+w*6;
			do {
				cu=s1[1]-128;
				cv=s1[3]-128;
				cr=409*cv+128;
				cg=-100*cu-208*cv+128;
				cb=516*cu+128;

				cy=(s1[0]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[0]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[1]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[2]=(emByte)c;

				cy=(s1[2]-16)*298;
				c=(cr+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[3]=(emByte)c;
				c=(cg+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[4]=(emByte)c;
				c=(cb+cy)>>8;
				if ((unsigned)c>255) c=(-c)>>16;
				t1[5]=(emByte)c;

				s1+=4;
				t1+=6;
			} while (t1<t2);
		} while(h>0);
	}
	else {
		goto L_BAD_DATA;
	}

	if (inst->Client) {
		inst->Client->ShowFrame(inst->Image,65536.0/aspectRatio);
	}
	return;

L_BAD_DATA:
	emDLog("emAvServerModel::TransferFrame: Bad data!\n");
	inst->Image.Empty();
	if (inst->Client) inst->Client->ShowFrame(inst->Image,3.0/4.0);
}
