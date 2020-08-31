/*------------------------------------------------------------------------------
// emAvServerProc_vlc.c
//
// Copyright (C) 2018-2020 Oliver Hamann.
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
//----------------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vlc/libvlc_version.h>
#include <vlc/vlc.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#	if defined(_WIN32)
#		include <fcntl.h>
#		include <io.h>
#	endif
#	include <windows.h>
#else
#	include <sys/shm.h>
#endif


/*============================================================================*/
/*================================= Logging ==================================*/
/*============================================================================*/

/*#define EM_AV_ENABLE_LOGGING*/

#if defined(EM_AV_ENABLE_LOGGING)
	void emAvLogFunc(const char * func, int line, const char * format, ...)
	{
		va_list args;
		char msg[256];

		va_start(args,format);
		vsnprintf(msg,sizeof(msg),format,args);
		msg[sizeof(msg)-1]=0;
		va_end(args);
		fprintf(stderr,"--- %s, %d: %s\n",func,line,msg);
	}
#	define EM_AV_LOG(...) emAvLogFunc(__FUNCTION__,__LINE__,__VA_ARGS__)
#else
#	define EM_AV_LOG(...)
#endif


/*============================================================================*/
/*============================== Timeout thread ==============================*/
/*============================================================================*/

static int emAvTimeoutThreadMilliseconds;
static int emAvTimeoutThreadDone;
static pthread_mutex_t emAvTimeoutMutex;
static pthread_t emAvTimeoutThread;


static void * emAvTimeoutThreadRoutine(void * arg)
{
	for (int i=0; ; i++) {
		pthread_mutex_lock(&emAvTimeoutMutex);
		int done=emAvTimeoutThreadDone;
		pthread_mutex_unlock(&emAvTimeoutMutex);
		if (done) break;
		if (i*10>emAvTimeoutThreadMilliseconds) {
			fprintf(stderr,"emAvServerProc_vlc: Function timed out: %s\n",(const char*)arg);
			exit(255);
		}
		usleep(10000);
	}
	return NULL;
}


static void emAvStartTimeoutThread(int milliseconds, const char * purpose)
{
	emAvTimeoutThreadMilliseconds=milliseconds;
	emAvTimeoutThreadDone=0;
	pthread_mutex_init(&emAvTimeoutMutex,NULL);
	pthread_create(&emAvTimeoutThread,NULL,emAvTimeoutThreadRoutine,(void*)purpose);
}


static void emAvStopTimeoutThread()
{
	pthread_mutex_lock(&emAvTimeoutMutex);
	emAvTimeoutThreadDone=1;
	pthread_mutex_unlock(&emAvTimeoutMutex);
	pthread_join(emAvTimeoutThread,NULL);
	pthread_mutex_destroy(&emAvTimeoutMutex);
}


/*============================================================================*/
/*============================= emAvVideoAdapter =============================*/
/*============================================================================*/

typedef struct {
	pthread_mutex_t Mutex;
	int MinShmSize;
	int ShmSize;
#if defined(_WIN32) || defined(__CYGWIN__)
	HANDLE ShmHdl;
#endif
	void * ShmPtr;
	int DummyMemSize;
	char * DummyMem;
	int OutputEnabled;
	int AudioVisuEnabled;
	int AudioVisuType;
	int Width;
	int Height;
	int Aspect;
	int Format;
	int Pitches[3];
	int Lines[3];
	int FrameCount;
	int Pause;
	int PauseCountDown;
} emAvVideoAdapter;


static const char * emAvAudioVisus[] = {
	"none",
	"colripp",
	NULL
};


emAvVideoAdapter * emAvCreateVideoAdapter()
{
	emAvVideoAdapter * v;

	v=(emAvVideoAdapter*)calloc(1,sizeof(emAvVideoAdapter));
	pthread_mutex_init(&v->Mutex,NULL);
	v->AudioVisuType=1;
	return v;
}


void emAvDeleteVideoAdapter(emAvVideoAdapter * v)
{
	if (v) {
		if (v->DummyMem) free(v->DummyMem);
		pthread_mutex_destroy(&v->Mutex);
		free(v);
	}
}


static unsigned emAvVideoFormatCb(
	void * * opaque, char * chroma, unsigned * width, unsigned * height,
	unsigned * pitches, unsigned * lines
)
{
	static const struct {
		const char * chroma;
		int format;
	} tab[] = {
		/* This is based on vlc_fourcc.h */
		{"YVU9",1},
		{"I410",1},
		{"I411",2},
		{"YV12",1},
		{"I420",1},
		{"I09L",1},
		{"I09B",1},
		{"I0AL",1},
		{"I0AB",1},
		{"I0CL",1},
		{"I0CB",1},
		{"I0FL",1},
		{"I0FB",1},
		{"I422",2},
		{"I29L",2},
		{"I29B",2},
		{"I2AL",2},
		{"I2AB",2},
		{"I2CL",2},
		{"I2CB",2},
		{"J420",1},
		{"J422",2},
		{"YUVP",2},
		{"I42A",2},
		{"I40A",1},
		{"NV12",1},
		{"NV21",1},
		{"NV16",2},
		{"NV61",2},
		{"P010",1},
		{"UYVY",2},
		{"VYUY",2},
		{"YUY2",2},
		{"YVYU",2},
		{"Y211",2},
		{"cyuv",2},
		{"v210",2},
		{"r420",1},
		{"GREY",1},
		{"VDV0",1},
		{"VDV2",2},
		{"VAOP",1},
		{"VAO0",1},
		{"DXA9",1},
		{"DXA0",1},
		{"DX11",1},
		{"DX10",1},
		{"CVPN",1},
		{"CVPY",2},
		{"CVPI",1},
		{NULL,  0} /* end of list, default format */
	};
	emAvVideoAdapter * v;
	int i,planeCount;

	/* Note: This function may be called multiple times with different chromas. */

	EM_AV_LOG("chroma=%s with=%u height=%u",chroma,*width,*height);

	if (
		(*width)<1 || (*width)>0x7fffff ||
		(*height)<1 || (*height)>0x7fffff ||
		(*width)*(unsigned long)(*height) > 0x3fffffff
	) {
		return 0;
	}

	v=(emAvVideoAdapter*)*opaque;
	pthread_mutex_lock(&v->Mutex);

	for (i=0; tab[i].chroma && memcmp(chroma,tab[i].chroma,4)!=0; i++);
	v->Format=tab[i].format;

	/* Here a format can be forced for testing. */
	/* v->Format=2; */

	switch (v->Format) {
	case 1:
		strcpy(chroma,"I420");
		pitches[0]=((*width)+31)&~31;
		pitches[1]=(((*width)+1)/2+31)&~31;
		pitches[2]=(((*width)+1)/2+31)&~31;
		lines[0]=((*height)+31)&~31;
		lines[1]=(((*height)+1)/2+31)&~31;
		lines[2]=(((*height)+1)/2+31)&~31;
		planeCount=3;
		break;
	case 2:
		strcpy(chroma,"YUY2");
		pitches[0]=(((*width)*2)+31)&~31;
		lines[0]=((*height)+31)&~31;
		planeCount=1;
		break;
	default:
		strcpy(chroma,"RV24");
		pitches[0]=(((*width)*3)+31)&~31;
		lines[0]=((*height)+31)&~31;
		planeCount=1;
		break;
	}

	v->Width=*width;
	v->Height=*height;

	v->MinShmSize=256;
	for (i=0; i<planeCount; i++) {
		v->Pitches[i]=pitches[i];
		v->Lines[i]=lines[i];
		v->MinShmSize+=pitches[i]*lines[i];
	}

	v->AudioVisuEnabled=0;

	pthread_mutex_unlock(&v->Mutex);
	return 1;
}


static void emAvVideoCleanupCb(void * opaque)
{
}


static void * emAvVideoLockCb(void * opaque, void * * planes)
{
	emAvVideoAdapter * v;
	int * pi;
	char * p;
	int showFrame;

	v=(emAvVideoAdapter*)opaque;
	pthread_mutex_lock(&v->Mutex);

	EM_AV_LOG("v->Format=%d",v->Format);

	showFrame=v->OutputEnabled;

	if (v->Pause) {
		/* In pause mode, VLC often still sends frames. Ignore more than
		   a few ones in order to spare CPU cycles... */
		if (v->PauseCountDown<-10) showFrame=0;
		else v->PauseCountDown--;
	}
	else {
		v->PauseCountDown=0;
	}

	pi=(int*)v->ShmPtr;
	if (!showFrame || !pi || v->ShmSize<v->MinShmSize || pi[0]) {
		EM_AV_LOG("*** Leaving out a frame ***");
		if (v->DummyMemSize<v->MinShmSize) {
			v->DummyMemSize=v->MinShmSize;
			v->DummyMem=realloc(v->DummyMem,v->DummyMemSize);
		}
		pi=(int*)v->DummyMem;
	}

	pi[1]=v->Width;
	pi[2]=v->Height;
	pi[3]=v->Aspect ? v->Aspect : (v->Width*65536+v->Height/2)/v->Height;
	pi[4]=v->Format;

	switch (v->Format) {
	case 1:
		pi[5]=v->Pitches[0];
		pi[6]=v->Pitches[1];
		p=(char*)(pi+10);
		pi[7]=(((char*)NULL)-p)&31;
		p+=pi[7];
		pi[8]=v->Pitches[0]*(v->Lines[0]-v->Height);
		pi[9]=v->Pitches[1]*(v->Lines[1]-(v->Height+1)/2);
		planes[0]=p;
		p+=v->Pitches[0]*v->Lines[0];
		planes[1]=p;
		p+=v->Pitches[1]*v->Lines[1];
		planes[2]=p;
		break;
	default:
		pi[5]=v->Pitches[0];
		p=(char*)(pi+7);
		pi[6]=(((char*)NULL)-p)&31;
		p+=pi[6];
		planes[0]=p;
		break;
	}

	return pi;
}


static void emAvVideoUnlockCb(void * opaque, void * picture, void * const * planes)
{
	emAvVideoAdapter * v;
	int * pi;

	EM_AV_LOG("");

	v=(emAvVideoAdapter*)opaque;

	/* It would be more correct to do this in the display callback. But if
	   there is really a time synchronization behind that call, we would
	   need some kind of double buffering. Copying from dummy mem to shared
	   mem would be a bad idea. */
	pi=(int*)picture;
	pi[0]=1;

	v->FrameCount++;

	pthread_mutex_unlock(&v->Mutex);
}


static void emAvVideoDisplayCb(void * opaque, void * picture)
{
}


static void emAvDummyAudioPlayCb(void *data, const void *samples,
                                 unsigned count, int64_t pts)
{
}


static void emAvPerformAudioVisualizationFrame(emAvVideoAdapter * v)
{
	int volatile * pi;
	unsigned char volatile * map, * p;
	int showFrame,pitch,x,y,c,cx,cy,cs;
	float lx,ly,lz,dx,dy,r,rr,rm,rrm,nx,ny,nz,a,sr,sg,sb,f,t;

	pthread_mutex_lock(&v->Mutex);

	if (!v->AudioVisuEnabled) {
		pthread_mutex_unlock(&v->Mutex);
		return;
	}

	v->Format=0;
	v->Width=320;
	v->Height=v->Width*3/4;
	v->Pitches[0]=v->Width*3;
	v->Lines[0]=v->Height;
	v->MinShmSize=256+v->Pitches[0]*v->Lines[0];

	showFrame=v->OutputEnabled;
	if (v->Pause) {
		if (v->PauseCountDown<-1) showFrame=0;
		else v->PauseCountDown--;
	}
	else {
		v->PauseCountDown=0;
	}

	if (v->AudioVisuType==0) showFrame=0;

	pi=(int*)v->ShmPtr;
	if (!showFrame || !pi || v->ShmSize<v->MinShmSize || pi[0]) {
		pthread_mutex_unlock(&v->Mutex);
		return;
	}

	pi[1]=v->Width;
	pi[2]=v->Height;
	pi[3]=(v->Width*65536+v->Height/2)/v->Height;
	pi[4]=v->Format;
	pi[5]=v->Pitches[0];
	pi[6]=0;
	map=(unsigned char*)(pi+7);
	memset((void*)map,0,v->Pitches[0]*v->Lines[0]);

	cx=v->Width/2;
	cy=v->Height/2;
	cs=v->Height/2;
	rm=(float)cs;
	rrm=rm*rm;
	lx=0.35F;
	ly=0.45F;
	lz=sqrtf(1.0F-lx*lx-ly*ly);
	pitch=v->Pitches[0];
	map+=cy*pitch+cx*3;
	for (y=0; y<cs; y++) {
		for (x=0; x<=y; x++) {
			dx=x+0.5F;
			dy=y+0.5F;
			rr=dx*dx+dy*dy;
			if (rr>=rrm) continue;

			r=sqrtf(rr);
			a=r*24.0F/rm;
			t=cosf(a-v->FrameCount*0.2F);
			nx=dx/r*t;
			ny=dy/r*t;
			nz=1.0F;
			t=sqrtf(nx*nx+ny*ny+nz*nz);
			nx/=t;
			ny/=t;
			nz/=t;

			sr=sinf(a-v->FrameCount*0.1F);
			sg=sinf(a*0.4F-v->FrameCount*0.03F);
			sb=sinf(a*0.3F-v->FrameCount*0.05F);

			f=(1.0F-r/rm)*255.0F;

#			define EMAV_AVFRAME_PIXEL(X,Y,NX,NY) \
				p=map+(Y)*pitch+(X)*3; \
				t=(NX)*lx+(NY)*ly+nz*lz; \
				c=(int)(f*t*(sr+t)); \
				p[0]=(unsigned char)((unsigned)c>255 ? (-c)>>16 : c); \
				c=(int)(f*t*(sg+t)); \
				p[1]=(unsigned char)((unsigned)c>255 ? (-c)>>16 : c); \
				c=(int)(f*t*(sb+t)); \
				p[2]=(unsigned char)((unsigned)c>255 ? (-c)>>16 : c);

			EMAV_AVFRAME_PIXEL(-x-1,-y-1,-nx,-ny)
			EMAV_AVFRAME_PIXEL( x  ,-y-1, nx,-ny)
			EMAV_AVFRAME_PIXEL(-x-1, y  ,-nx, ny)
			EMAV_AVFRAME_PIXEL( x  , y  , nx, ny)

			if (x>=y) break;

			EMAV_AVFRAME_PIXEL(-y-1,-x-1,-ny,-nx)
			EMAV_AVFRAME_PIXEL( y  ,-x-1, ny,-nx)
			EMAV_AVFRAME_PIXEL(-y-1, x  ,-ny, nx)
			EMAV_AVFRAME_PIXEL( y  , x  , ny, nx)
		}
	}

	pi[0]=1;
	v->FrameCount++;
	pthread_mutex_unlock(&v->Mutex);
}


/*============================================================================*/
/*================================== emAvPipe ================================*/
/*============================================================================*/

#define EM_AV_MAX_MSG_LEN 32768

static int emAvPipeIn=-1;
static int emAvPipeOut=-1;
static int emAvPipeBufSize=EM_AV_MAX_MSG_LEN*2;
static char * emAvPipeInBuf;
static char * emAvPipeOutBuf;
static int emAvPipeInBufPos=0;
static int emAvPipeInBufEnd=0;
static int emAvPipeOutBufFill=0;


#ifdef _WIN32
static pthread_mutex_t emAvRcvThreadMutex={0};
static char * emAvRcvBuf=NULL;
static int emAvRcvBufSize=0;
static int emAvRcvBufFill=0;
static int emAvRcvEnded=0;

static DWORD WINAPI emAvRcvThreadProc(LPVOID data)
{
	int f,l;

	pthread_mutex_lock(&emAvRcvThreadMutex);
	for (;;) {
		f=emAvRcvBufFill;
		if (f>=emAvRcvBufSize) {
			pthread_mutex_unlock(&emAvRcvThreadMutex);
			usleep(10000);
			pthread_mutex_lock(&emAvRcvThreadMutex);
			continue;
		}
		l=emAvRcvBufSize-f;
		pthread_mutex_unlock(&emAvRcvThreadMutex);
		l=read(emAvPipeIn,emAvRcvBuf+f,l);
		pthread_mutex_lock(&emAvRcvThreadMutex);
		if (l<0) {
			fprintf(
				stderr,
				"emAvServerProc_vlc: Could not read from pipe: %s\n",
				strerror(errno)
			);
			exit(255);
		}
		if (l==0) {
			emAvRcvEnded=1;
			break;
		}
		if (f!=emAvRcvBufFill) {
			memmove(emAvRcvBuf+emAvRcvBufFill,emAvRcvBuf+f,l);
		}
		emAvRcvBufFill+=l;
	}
	pthread_mutex_unlock(&emAvRcvThreadMutex);
	return 0;
}
#endif


static void emAvInitPipe()
{
	emAvPipeInBuf=(char*)malloc(emAvPipeBufSize);
	emAvPipeOutBuf=(char*)malloc(emAvPipeBufSize);

	emAvPipeIn=dup(STDIN_FILENO);
	if (emAvPipeIn==-1) {
		fprintf(
			stderr,
			"emAvServerProc_vlc: dup(STDIN_FILENO) failed: %s\n",
			strerror(errno)
		);
		exit(255);
	}
	close(STDIN_FILENO);

#ifdef _WIN32
	emAvRcvBufSize=65536;
	emAvRcvBuf=malloc(emAvRcvBufSize);
	pthread_mutex_init(&emAvRcvThreadMutex,NULL);
	DWORD d;
	if (!CreateThread(NULL,0,emAvRcvThreadProc,NULL,0,&d)) {
		fprintf(
			stderr,
			"emAvServerProc_vlc: Failed to create thread for pipe: %u\n",
			(unsigned)GetLastError()
		);
		exit(255);
	}
#else
	int f;
	if (
		(f=fcntl(emAvPipeIn,F_GETFL))<0 ||
		fcntl(emAvPipeIn,F_SETFL,f|O_NONBLOCK)<0
	) {
		fprintf(
			stderr,
			"emAvServerProc_vlc: Failed to set pipe read handle to non-blocking mode: %s\n",
			strerror(errno)
		);
		exit(255);
	}
#endif

	emAvPipeOut=dup(STDOUT_FILENO);
	if (emAvPipeOut==-1) {
		fprintf(
			stderr,
			"emAvServerProc_vlc: dup(STDOUT_FILENO) failed: %s\n",
			strerror(errno)
		);
		exit(255);
	}
	close(STDOUT_FILENO);
	if (dup2(STDERR_FILENO,STDOUT_FILENO)==-1) {
		fprintf(
			stderr,
			"emAvServerProc_vlc: dup2(STDERR_FILENO,STDOUT_FILENO) failed: %s\n",
			strerror(errno)
		);
		exit(255);
	}
}


static void emAvFlushPipe()
{
	int pos,len;

	for (pos=0; pos<emAvPipeOutBufFill; pos+=len) {
		len=write(emAvPipeOut,emAvPipeOutBuf+pos,emAvPipeOutBufFill-pos);
		if (len<0) {
			fprintf(
				stderr,
				"emAvServerProc_vlc: Could not write to pipe: %s\n",
				strerror(errno)
			);
			exit(255);
		}
	}
	emAvPipeOutBufFill=0;
}


static void emAvBeginMsg(int instIndex, const char * tag)
{
	if (emAvPipeBufSize-emAvPipeOutBufFill<EM_AV_MAX_MSG_LEN) {
		emAvFlushPipe();
	}
	emAvPipeOutBufFill+=
		sprintf(emAvPipeOutBuf+emAvPipeOutBufFill,"%d:%s",instIndex,tag)
	;
}


static void emAvContMsgV(const char * format, va_list args)
{
	char * p;
	int a,l,i;

	p=emAvPipeOutBuf+emAvPipeOutBufFill;
	a=emAvPipeBufSize-emAvPipeOutBufFill;
	l=vsnprintf(p,a,format,args);
	if (l<0 || l>a) l=a; /* just clip it... */
	for (i=0; i<l; i++) {
		if (p[i]==0x0a) p[i]=0x1a;
		if (p[i]==0x0d) p[i]=0x1d;
	}
	emAvPipeOutBufFill+=l;
}


static void emAvContMsg(const char * format, ...)
{
	va_list args;

	va_start(args,format);
	emAvContMsgV(format,args);
	va_end(args);
}


static void emAvEndMsg()
{
	if (emAvPipeOutBufFill>=emAvPipeBufSize) {
		emAvPipeOutBufFill=emAvPipeBufSize-1;
	}
	emAvPipeOutBuf[emAvPipeOutBufFill]='\n';
	emAvPipeOutBufFill++;
}


static void emAvSendMsg(
	int instIndex, const char * tag, const char * format, ...
)
{
	va_list args;

	emAvBeginMsg(instIndex,tag);
	if (format && *format) {
		emAvContMsg(":");
		va_start(args,format);
		emAvContMsgV(format,args);
		va_end(args);
	}
	emAvEndMsg();
}


static int emAvReceiveMsg(
	int * pInstIndex, const char * * pTag, const char * * pData
)
{
	char * p1, * p2, * p3, * p4, * pe;
	int l;

	/* Return value: -1 = pipe closed, 0 = try later again, 1 = got message. */

	for (;;) {
		p1=emAvPipeInBuf+emAvPipeInBufPos;
		pe=emAvPipeInBuf+emAvPipeInBufEnd;
		while (p1<pe && (unsigned char)*p1<=0x20) {
			emAvPipeInBufPos++;
			p1++;
		}
		for (p4=p1; p4<pe && *p4!=0x0a && *p4!=0x0d; p4++);
		if (p4<pe) {
			p2=(char*)memchr(p1,':',p4-p1);
			if (!p2) {
				fprintf(stderr,"emAvServerProc_vlc: Protocol error.\n");
				exit(255);
			}
			*p2++=0;
			p3=(char*)memchr(p2,':',p4-p2);
			if (p3) *p3++=0; else p3=p4;
			*p4=0;
			emAvPipeInBufPos+=(p4+1-p1);
			*pInstIndex=atoi(p1);
			*pTag=p2;
			*pData=p3;
			return 1;
		}
		if (emAvPipeInBufPos>0) {
			emAvPipeInBufEnd-=emAvPipeInBufPos;
			if (emAvPipeInBufEnd>0) {
				memmove(
					emAvPipeInBuf,
					emAvPipeInBuf+emAvPipeInBufPos,
					emAvPipeInBufEnd
				);
			}
			emAvPipeInBufPos=0;
		}
		l=emAvPipeBufSize-emAvPipeInBufEnd;
		if (l<=0) {
			fprintf(stderr,"emAvServerProc_vlc: Pipe input buffer too small.\n");
			exit(255);
		}
#ifdef _WIN32
		pthread_mutex_lock(&emAvRcvThreadMutex);
		if (emAvRcvBufFill>0) {
			if (l>emAvRcvBufFill) l=emAvRcvBufFill;
			memcpy(emAvPipeInBuf+emAvPipeInBufEnd,emAvRcvBuf,l);
			emAvRcvBufFill-=l;
			if (emAvRcvBufFill>0) memmove(emAvRcvBuf,emAvRcvBuf+l,emAvRcvBufFill);
		}
		else if (emAvRcvEnded) {
			l=0;
		}
		else {
			l=-1;
			errno=EAGAIN;
		}
		pthread_mutex_unlock(&emAvRcvThreadMutex);
#else
		l=read(emAvPipeIn,emAvPipeInBuf+emAvPipeInBufEnd,l);
#endif
		if (l<0) {
			if (errno==EAGAIN) return 0;
			fprintf(
				stderr,
				"emAvServerProc_vlc: Could not read from pipe: %s\n",
				strerror(errno)
			);
			exit(255);
		}
		if (l==0) return -1;
		emAvPipeInBufEnd+=l;
	}
}


/*============================================================================*/
/*================================ emAvInstance ==============================*/
/*============================================================================*/

typedef enum {
	EMAV_STOPPED,
	EMAV_PAUSED,
	EMAV_NORMAL,
	EMAV_SLOW,
	EMAV_FAST
} emAvPlayState;

typedef struct {
	libvlc_instance_t * VlcInstance;
	libvlc_media_t * VlcMedia;
	libvlc_media_player_t * VlcPlayer;
	emAvVideoAdapter * VideoAdapter;
	emAvPlayState State;
	int MinShmSize;
	int PlayLength;
	int PlayPos;
	int AudioVolume;
	int AudioMute;
	int TrackCount;
	libvlc_media_track_t * * Tracks;
	libvlc_track_description_t * AudioDescs;
	libvlc_track_description_t * VideoDescs;
	libvlc_track_description_t * SpuDescs;
	int AudioTrackId;
	int SpuTrackId;
} emAvInstance;

#define EM_AV_MAX_INSTANCES 100
static emAvInstance * emAvInstances[EM_AV_MAX_INSTANCES];
static int emAvInstanceCount=0;

/* #define HAVE_ONE_VLC_INSTANCE_PER_STREAM */

#ifndef HAVE_ONE_VLC_INSTANCE_PER_STREAM
static libvlc_instance_t * TheVlcInstance=NULL;
#endif


static void emAvInitInstances()
{
	memset(emAvInstances,0,sizeof(emAvInstances));
}


static void emAvReleaseTracks(emAvInstance * inst)
{
	if (inst->VideoDescs) {
		libvlc_track_description_list_release(inst->VideoDescs);
		inst->VideoDescs=NULL;
	}
	if (inst->AudioDescs) {
		libvlc_track_description_list_release(inst->AudioDescs);
		inst->AudioDescs=NULL;
	}
	if (inst->SpuDescs) {
		libvlc_track_description_list_release(inst->SpuDescs);
		inst->SpuDescs=NULL;
	}
	if (inst->Tracks) {
		libvlc_media_tracks_release(inst->Tracks,inst->TrackCount);
		inst->TrackCount=0;
		inst->Tracks=NULL;
	}
}


static void emAvFetchTracks(emAvInstance * inst)
{
	emAvReleaseTracks(inst);
	inst->TrackCount=libvlc_media_tracks_get(inst->VlcMedia,&inst->Tracks);
	inst->AudioDescs=libvlc_audio_get_track_description(inst->VlcPlayer);
	inst->VideoDescs=libvlc_video_get_track_description(inst->VlcPlayer);
	inst->SpuDescs=libvlc_video_get_spu_description(inst->VlcPlayer);
}


static int emAvIsTracksInfoComplete(emAvInstance * inst)
{
	int i,foundAudio,foundVideo,foundAspect;

	foundVideo=foundAudio=foundAspect=0;
	for (i=0; i<inst->TrackCount; i++) {
		if (inst->Tracks[i]->i_type==libvlc_track_video) {
			foundVideo++;
			if (
				inst->Tracks[i]->video->i_width > 0 &&
				inst->Tracks[i]->video->i_height > 0 &&
				inst->Tracks[i]->video->i_sar_num > 0 &&
				inst->Tracks[i]->video->i_sar_den > 0
			) foundAspect++;
		}
		else if (inst->Tracks[i]->i_type==libvlc_track_audio) {
			foundAudio++;
		}
	}
	if (!foundAudio && !foundVideo) return 0;
	if (foundVideo && !foundAspect) return 0;

	if (
		libvlc_media_player_get_length(inst->VlcPlayer) <= 0 &&
		libvlc_media_get_duration(inst->VlcMedia) <=0
	) return 0;

	return 1;
}


static void emAvSendInfo(int instIndex)
{
	emAvInstance * inst;
	const libvlc_media_track_t * track;
	const char * codec;
	char * val, * p, * q, * e;
	int i,j,rate,d,length_time,min,sec,w,h,t;

	inst=emAvInstances[instIndex];

	emAvBeginMsg(instIndex,"set");
	emAvContMsg(":info:");

	static struct {
		libvlc_meta_t meta;
		const char * name;
		int always;
	} metas[]={
		{ libvlc_meta_Title      , "Title      ", 1 },
		{ libvlc_meta_Artist     , "Artist     ", 1 },
		{ libvlc_meta_Genre      , "Genre      ", 1 },
		{ libvlc_meta_Copyright  , "Copyright  ", 1 },
		{ libvlc_meta_Album      , "Album      ", 1 },
		{ libvlc_meta_TrackNumber, "TrackNumber", 0 },
		{ libvlc_meta_Description, "Description", 1 },
		{ libvlc_meta_Rating     , "Rating     ", 0 },
		{ libvlc_meta_Date       , "Date       ", 0 },
		{ libvlc_meta_Setting    , "Setting    ", 0 },
		{ libvlc_meta_URL        , "URL        ", 0 },
		{ libvlc_meta_Language   , "Language   ", 0 },
		{ libvlc_meta_NowPlaying , "NowPlaying ", 0 },
		{ libvlc_meta_Publisher  , "Publisher  ", 0 },
		{ libvlc_meta_EncodedBy  , "EncodedBy  ", 0 },
		{ libvlc_meta_ArtworkURL , "ArtworkURL ", 0 },
		{ libvlc_meta_TrackID    , "TrackID    ", 0 },
		{ libvlc_meta_TrackTotal , "TrackTotal ", 0 },
		{ libvlc_meta_Director   , "Director   ", 0 },
		{ libvlc_meta_Season     , "Season     ", 0 },
		{ libvlc_meta_Episode    , "Episode    ", 0 },
		{ libvlc_meta_ShowName   , "ShowName   ", 0 },
		{ libvlc_meta_Actors     , "Actors     ", 0 },
		{ libvlc_meta_AlbumArtist, "AlbumArtist", 0 },
		{ libvlc_meta_DiscNumber , "DiscNumber ", 0 },
		{ libvlc_meta_DiscTotal  , "DiscTotal  ", 0 },
		{ (libvlc_meta_t)-1      , NULL         , 0 }
	};

	for (i=0; metas[i].name; i++) {
		val=libvlc_media_get_meta(inst->VlcMedia,metas[i].meta);
		if (val || metas[i].always) {
			emAvContMsg("%s: ",metas[i].name);
			if (val) {
				for (e=val+strlen(val); e>val && (unsigned char)e[-1]<=0x20; e--);
				for (p=val,j=0; p<e; j++) {
					for (q=p; q<e && *q!=0x0a; q++);
					*q=0;
					if (q>p && q[-1]==0x0d) q[-1]=0;
					if (j) emAvContMsg("\n             ");
					emAvContMsg("%s",p);
					p=q+1;
					if (p<e && *p==0x0d) p++;
				}
				free(val);
			}
			emAvContMsg("\n");
		}
	}

	emAvContMsg("Length     :");
	length_time=(int)libvlc_media_player_get_length(inst->VlcPlayer);
	if (length_time<=0) {
		length_time=(int)libvlc_media_get_duration(inst->VlcMedia);
	}
	if (length_time>0) {
		sec=(length_time+500)/1000;
		min=sec/60;
		sec%=60;
		if (min>=30) emAvContMsg(" %d minutes",min+(sec+30)/60);
		else if (min) emAvContMsg(" %d minutes, %d seconds",min,sec);
		else emAvContMsg(" %d seconds",sec);
	}

	for (i=0; i<inst->TrackCount; i++) {
		track=inst->Tracks[i];
		codec=libvlc_media_get_codec_description(track->i_type,track->i_codec);
		if (!codec) codec="unknown";
		if (track->i_type==libvlc_track_video) {
			rate=track->video->i_frame_rate_num;
			d=track->video->i_frame_rate_den;
			if (d) rate=(rate+(d/2))/d;
			w=track->video->i_width;
			h=track->video->i_height;
			switch (track->video->i_orientation) {
			case libvlc_video_orient_left_top:
			case libvlc_video_orient_left_bottom:
			case libvlc_video_orient_right_top:
			case libvlc_video_orient_right_bottom:
				t=w; w=h; h=t;
				break;
			default:
				break;
			}
			emAvContMsg(
				"\nVideo      : %s, %dx%d pixels, %d Hz",
				codec,w,h,rate
			);
			EM_AV_LOG(
				"Orientation: %s",
				track->video->i_orientation==libvlc_video_orient_top_left ? "top_left" :
				track->video->i_orientation==libvlc_video_orient_top_right ? "top_right" :
				track->video->i_orientation==libvlc_video_orient_bottom_left ? "bottom_left" :
				track->video->i_orientation==libvlc_video_orient_bottom_right ? "bottom_right" :
				track->video->i_orientation==libvlc_video_orient_left_top ? "left_top" :
				track->video->i_orientation==libvlc_video_orient_left_bottom ? "left_bottom" :
				track->video->i_orientation==libvlc_video_orient_right_top ? "right_top" :
				track->video->i_orientation==libvlc_video_orient_right_bottom ? "right_bottom" :
				"other"
			);
		}
		else if (track->i_type==libvlc_track_audio) {
			emAvContMsg(
				"\nAudio      : %s, %d channels, %d Hz",
				codec,
				track->audio->i_channels,
				track->audio->i_rate
			);
		}
		else if (track->i_type==libvlc_track_text) {
			emAvContMsg(
				"\nSubtitle   : %s, encoding=%s",
				codec,
				track->subtitle->psz_encoding
			);
		}
	}

	emAvEndMsg();
}


static void emAvPollProperties(int instIndex, int initialize)
{
	emAvInstance * inst;
	const libvlc_track_description_t * desc;
	const char * str;
	char tmp[256];
	emAvPlayState state;
	libvlc_state_t vlcState;
	int id,sz,foundAudio,foundVideo,param,w,h,t,i,n,d,val,playing;
	float rate;

	inst=emAvInstances[instIndex];

	/* minshmsize */
	sz=0;
	pthread_mutex_lock(&inst->VideoAdapter->Mutex);
	if (inst->VideoAdapter->OutputEnabled) {
		sz=inst->VideoAdapter->MinShmSize;
	}
	pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
	if (inst->MinShmSize<sz || initialize) {
		inst->MinShmSize=sz;
		emAvSendMsg(instIndex,"minshmsize","%d",sz);
	}

	/* type */
	if (initialize) {
		for (foundVideo=0, foundAudio=0, i=0; i<inst->TrackCount; i++) {
			if (inst->Tracks[i]->i_type==libvlc_track_video) foundVideo++;
			if (inst->Tracks[i]->i_type==libvlc_track_audio) foundAudio++;
		}
		str=(foundVideo || !foundAudio)?"video":"audio";
		emAvSendMsg(instIndex,"set","type:%s",str);
		if (!foundVideo && foundAudio) {
			pthread_mutex_lock(&inst->VideoAdapter->Mutex);
			inst->VideoAdapter->AudioVisuEnabled=1;
			pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
		}
	}

	/* state */
	vlcState=libvlc_media_get_state(inst->VlcMedia);
	playing=libvlc_media_player_is_playing(inst->VlcPlayer);
	rate=libvlc_media_player_get_rate(inst->VlcPlayer);
	EM_AV_LOG("vlcState=%d playing=%d rate=%f",(int)vlcState,playing,rate);
	if (vlcState==libvlc_Paused) state=EMAV_PAUSED;
	else if (!playing) state=EMAV_STOPPED;
	else if (rate<0.75F) state=EMAV_SLOW;
	else if (rate>1.25F) state=EMAV_FAST;
	else state=EMAV_NORMAL;
	if (initialize || inst->State!=state) {
		inst->State=state;
		if (state==EMAV_STOPPED) str="stopped";
		else if (state==EMAV_PAUSED) str="paused";
		else if (state==EMAV_SLOW) str="slow";
		else if (state==EMAV_FAST) str="fast";
		else str="normal";
		emAvSendMsg(instIndex,"set","state:%s",str);
	}

	/* length */
	val=(int)libvlc_media_player_get_length(inst->VlcPlayer);
	if (val<=0) val=(int)libvlc_media_get_duration(inst->VlcMedia);
	if (val<0) val=0;
	if (initialize || inst->PlayLength!=val) {
		inst->PlayLength=val;
		emAvSendMsg(instIndex,"set","length:%d",val);
	}

	/* pos */
	val=(int)libvlc_media_player_get_time(inst->VlcPlayer);
	if (val<0) val=0;
	if (inst->State==EMAV_STOPPED) val=inst->PlayPos;
	if (initialize || inst->PlayPos!=val) {
		inst->PlayPos=val;
		emAvSendMsg(instIndex,"set","pos:%d",val);
	}

	/* aspect */
	if (initialize) {
		for (d=0, n=0, w=0, h=0, i=0; i<inst->TrackCount; i++) {
			if (inst->Tracks[i]->i_type==libvlc_track_video) {
				w=inst->Tracks[i]->video->i_width;
				h=inst->Tracks[i]->video->i_height;
				n=inst->Tracks[i]->video->i_sar_num;
				d=inst->Tracks[i]->video->i_sar_den;
				switch (inst->Tracks[i]->video->i_orientation) {
				case libvlc_video_orient_left_top:
				case libvlc_video_orient_left_bottom:
				case libvlc_video_orient_right_top:
				case libvlc_video_orient_right_bottom:
					t=w; w=h; h=t;
					t=n; n=d; d=t;
					break;
				default:
					break;
				}
				if (w && h) break;
			}
		}
		if (!n || !d) { n=1; d=1; }
		if (!w || !h) { w=4; h=3; }
		n=(int)(((double)w)*n*65536.0/h/d+0.5);
		emAvSendMsg(instIndex,"set","aspect:%d",n);
		if (inst->VideoAdapter) {
			pthread_mutex_lock(&inst->VideoAdapter->Mutex);
			inst->VideoAdapter->Aspect=n;
			pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
		}
	}

	/* audio_volume */
	param=libvlc_audio_get_volume(inst->VlcPlayer);
	if (inst->AudioVolume!=param || initialize) {
		inst->AudioVolume=param;
		emAvSendMsg(instIndex,"set","audio_volume:%d",param);
	}

	/* audio_mute */
	param=libvlc_audio_get_mute(inst->VlcPlayer);
	param=(param>0);
	if (inst->AudioMute!=param || initialize) {
		inst->AudioMute=param;
		emAvSendMsg(instIndex,"set","audio_mute:%s",param?"on":"off");
	}

	/* info */
	if (initialize) {
		emAvSendInfo(instIndex);
	}

	/* warning */
	if (initialize) {
		emAvSendMsg(instIndex,"set","warning:");
	}

	/* audio_visus */
	if (initialize) {
		pthread_mutex_lock(&inst->VideoAdapter->Mutex);
		param=inst->VideoAdapter->AudioVisuEnabled;
		pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_visus");
		for (i=0; param && emAvAudioVisus[i]; i++) {
			emAvContMsg(":%s",emAvAudioVisus[i]);
		}
		emAvEndMsg();
	}

	/* audio_visu */
	if (initialize) {
		pthread_mutex_lock(&inst->VideoAdapter->Mutex);
		param=inst->VideoAdapter->AudioVisuType;
		pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
		emAvSendMsg(instIndex,"set","audio_visu:%s",emAvAudioVisus[param]);
	}

	/* audio_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_channels");
		for (desc=inst->AudioDescs, i=1; desc; desc=desc->p_next, i++) {
			str=desc->psz_name;
			if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
			emAvContMsg(":%s",str);
		}
		emAvEndMsg();
	}

	/* audio_channel */
	id=libvlc_audio_get_track(inst->VlcPlayer);
	if (initialize || inst->AudioTrackId!=id) {
		inst->AudioTrackId=id;
		for (str="", desc=inst->AudioDescs, i=1; desc; desc=desc->p_next, i++) {
			if (desc->i_id==id) {
				str=desc->psz_name;
				if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
				break;
			}
		}
		emAvSendMsg(instIndex,"set","audio_channel:%s",str);
	}

	/* spu_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":spu_channels");
		for (desc=inst->SpuDescs, i=1; desc; desc=desc->p_next, i++) {
			str=desc->psz_name;
			if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
			emAvContMsg(":%s",str);
		}
		emAvEndMsg();
	}

	/* spu_channel */
	id=libvlc_video_get_spu(inst->VlcPlayer);
	if (initialize || inst->SpuTrackId!=id) {
		inst->SpuTrackId=id;
		for (str="", desc=inst->SpuDescs, i=1; desc; desc=desc->p_next, i++) {
			if (desc->i_id==id) {
				str=desc->psz_name;
				if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
				break;
			}
		}
		emAvSendMsg(instIndex,"set","spu_channel:%s",str);
	}
}


static const char *  emAvSetProperty(
	int instIndex, const char * name, const char * value
)
{
	emAvInstance * inst;
	const libvlc_track_description_t * desc;
	const char * str;
	char tmp[256];
	emAvPlayState state;
	int i,id;

	EM_AV_LOG("%s=%s",name,value);

	inst=emAvInstances[instIndex];
	if (!inst) return "Not an opened instance.";

	if (strcmp(name,"pos")==0) {
		inst->PlayPos=atoi(value);
		if (inst->State!=EMAV_STOPPED) {
			pthread_mutex_lock(&inst->VideoAdapter->Mutex);
			inst->VideoAdapter->PauseCountDown=0;
			pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
			libvlc_media_player_set_time(inst->VlcPlayer,inst->PlayPos);
		}
	}
	else if (strcmp(name,"state")==0) {
		if (strcmp(value,"paused")==0) state=EMAV_PAUSED;
		else if (strcmp(value,"normal")==0) state=EMAV_NORMAL;
		else if (strcmp(value,"fast")==0) state=EMAV_FAST;
		else if (strcmp(value,"slow")==0) state=EMAV_SLOW;
		else state=EMAV_STOPPED;
		pthread_mutex_lock(&inst->VideoAdapter->Mutex);
		inst->VideoAdapter->Pause=(state==EMAV_PAUSED);
		pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
		if (state==EMAV_STOPPED) {
			emAvStartTimeoutThread(3000,"libvlc_media_player_stop in emAvSetProperty");
			libvlc_media_player_stop(inst->VlcPlayer);
			emAvStopTimeoutThread();
		}
		else {
			libvlc_media_player_play(inst->VlcPlayer);
			libvlc_media_player_set_pause(inst->VlcPlayer,(state==EMAV_PAUSED));
			if (inst->State==EMAV_STOPPED && inst->PlayPos>0) {
				libvlc_media_player_set_time(inst->VlcPlayer,inst->PlayPos);
			}
			if (state!=EMAV_PAUSED) {
				libvlc_media_player_set_rate(
					inst->VlcPlayer,
					state==EMAV_FAST ? 4.0F :
					state==EMAV_SLOW ? 0.25F :
					1.0F
				);
			}
		}
		inst->State=state;
		if (state!=EMAV_STOPPED) {
			/* Bugfix/workaround: wait until playing state arrived at the front. */
			for (i=0; i<200; i++) {
				if (libvlc_media_player_is_playing(inst->VlcPlayer)) break;
				usleep(10000);
			}
		}
	}
	else if (strcmp(name,"audio_volume")==0) {
		inst->AudioVolume=atoi(value);
		libvlc_audio_set_volume(inst->VlcPlayer,inst->AudioVolume);
	}
	else if (strcmp(name,"audio_mute")==0) {
		inst->AudioMute=(strcmp(value,"on")==0 ? 1 : 0);
		libvlc_audio_set_mute(inst->VlcPlayer,inst->AudioMute);
	}
	else if (strcmp(name,"audio_visu")==0) {
		for (i=0; emAvAudioVisus[i]; i++) {
			if (strcmp(emAvAudioVisus[i],value)==0) {
				pthread_mutex_lock(&inst->VideoAdapter->Mutex);
				inst->VideoAdapter->AudioVisuType=i;
				pthread_mutex_unlock(&inst->VideoAdapter->Mutex);
				break;
			}
		}
	}
	else if (strcmp(name,"audio_channel")==0) {
		for (id=-1, desc=inst->AudioDescs, i=1; desc; desc=desc->p_next, i++) {
			str=desc->psz_name;
			if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
			if (strcmp(str,value)==0) { id=desc->i_id; break; }
		}
		if (inst->AudioTrackId!=id) {
			inst->AudioTrackId=id;
			libvlc_audio_set_track(inst->VlcPlayer,id);
		}
	}
	else if (strcmp(name,"spu_channel")==0) {
		for (id=-1, desc=inst->SpuDescs, i=1; desc; desc=desc->p_next, i++) {
			str=desc->psz_name;
			if (!str || !*str) { sprintf(tmp,"%d",i); str=tmp; }
			if (strcmp(str,value)==0) { id=desc->i_id; break; }
		}
		if (inst->SpuTrackId!=id) {
			inst->SpuTrackId=id;
			libvlc_video_set_spu(inst->VlcPlayer,id);
		}
	}
	else {
		return "Unknown property.";
	}

	return NULL;
}


static void emAvDetachShm(int instIndex)
{
	emAvInstance * inst;
	emAvVideoAdapter * v;

	EM_AV_LOG("");

	inst=emAvInstances[instIndex];
	if (!inst) return;
	v=inst->VideoAdapter;
	if (!v) return;

	pthread_mutex_lock(&v->Mutex);

	if (v->ShmPtr) {
#if defined(_WIN32) || defined(__CYGWIN__)
		UnmapViewOfFile(v->ShmPtr);
		v->ShmPtr=NULL;
		CloseHandle(v->ShmHdl);
		v->ShmHdl=NULL;
#else
		shmdt((const void*)v->ShmPtr);
		v->ShmPtr=NULL;
#endif
		v->ShmSize=0;
	}

	pthread_mutex_unlock(&v->Mutex);
}


static const char * emAvAttachShm(int instIndex, const char * params)
{
	emAvInstance * inst;
	emAvVideoAdapter * v;
	const char * err;
	int shmSize;
#if defined(_WIN32) || defined(__CYGWIN__)
	char shmId[256];
	const char * p;
#else
	int shmId;
#endif

	EM_AV_LOG("%s",params);

	inst=emAvInstances[instIndex];
	if (!inst) return "Not an opened instance.";
	v=inst->VideoAdapter;
	if (!v) return "No video adapter.";

	pthread_mutex_lock(&v->Mutex);
	err=NULL;

#if defined(_WIN32) || defined(__CYGWIN__)

	shmId[0]=0;
	shmSize=0;
	p=strrchr(params,':');
	if (p) {
		memcpy(shmId,params,p-params);
		shmId[p-params]=0;
		sscanf(p+1,"%d",&shmSize);
	}
	if (shmId[0]==0 || shmSize<=0) {
		err="Illegal shm parameters.";
	}
	else {
		if (v->ShmPtr) {
			UnmapViewOfFile(v->ShmPtr);
			v->ShmPtr=NULL;
			CloseHandle(v->ShmHdl);
			v->ShmHdl=NULL;
			v->ShmSize=0;
		}
		v->ShmHdl=OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,shmId);
		if (!v->ShmHdl) {
			err="Failed to attach shared memory (OpenFileMapping failed)";
		}
		else {
			v->ShmPtr=MapViewOfFile(v->ShmHdl,FILE_MAP_ALL_ACCESS,0,0,0);
			if (!v->ShmPtr) {
				CloseHandle(v->ShmHdl);
				v->ShmHdl=NULL;
				err="Failed to attach shared memory (MapViewOfFile failed)";
			}
			else {
				v->ShmSize=shmSize;
			}
		}
	}

#else

	shmId=-1;
	shmSize=0;
	sscanf(params,"%d:%d",&shmId,&shmSize);
	if (shmId<0 || shmSize<=0) {
		err="Illegal shm parameters.";
	}
	else {
		if (v->ShmPtr) {
			shmdt((const void*)v->ShmPtr);
			v->ShmPtr=NULL;
			v->ShmSize=0;
		}
		v->ShmPtr=shmat(shmId,NULL,0);
		if (v->ShmPtr==(void*)-1) {
			v->ShmPtr=NULL;
			err="Failed to attach shm.";
		}
		else {
			v->ShmSize=shmSize;
		}
	}

#endif

	pthread_mutex_unlock(&v->Mutex);

	return err;
}


static void emAvUpdateAudioVisu(emAvInstance * inst)
{
	if (inst->State!=EMAV_STOPPED && inst->VideoAdapter) {
		emAvPerformAudioVisualizationFrame(inst->VideoAdapter);
	}
}


static void emAvCloseInstance(int instIndex)
{
	emAvInstance * inst;

	EM_AV_LOG("");

	inst=emAvInstances[instIndex];
	if (!inst) return;

	if (inst->VlcPlayer && inst->State!=EMAV_STOPPED) {
		emAvStartTimeoutThread(3000,"libvlc_media_player_stop in emAvCloseInstance");
		libvlc_media_player_stop(inst->VlcPlayer);
		emAvStopTimeoutThread();
	}

	emAvDetachShm(instIndex);

	emAvReleaseTracks(inst);

	if (inst->VlcPlayer) {
		emAvStartTimeoutThread(3000,"libvlc_media_player_release in emAvCloseInstance");
		libvlc_media_player_release(inst->VlcPlayer);
		emAvStopTimeoutThread();
		inst->VlcPlayer=NULL;
	}

	if (inst->VlcMedia) {
		libvlc_media_release(inst->VlcMedia);
		inst->VlcMedia=NULL;
	}

	if (inst->VideoAdapter) {
		emAvDeleteVideoAdapter(inst->VideoAdapter);
		inst->VideoAdapter=NULL;
	}

#ifdef HAVE_ONE_VLC_INSTANCE_PER_STREAM
	if (inst->VlcInstance) libvlc_release(inst->VlcInstance);
#endif

	free(inst);
	emAvInstances[instIndex]=NULL;
	emAvInstanceCount--;

#ifndef HAVE_ONE_VLC_INSTANCE_PER_STREAM
	if (emAvInstanceCount<=0 && TheVlcInstance) {
		libvlc_release(TheVlcInstance);
		TheVlcInstance=NULL;
	}
#endif
}


static const char * emAvOpenInstance(
	int instIndex, const char * audioDrv, const char * videoDrv, const char * file
)
{
	static char errBuf[512];
	const char * version, * msg;
	emAvInstance * inst;
	int major,minor,audioEnabled,videoEnabled,pass,i,fc;

	EM_AV_LOG("%s,%s,%s",audioDrv,videoDrv,file);

	if (emAvInstances[instIndex]) return "Instance already open.";

	version=libvlc_get_version();
	sscanf(version,"%d.%d",&major,&minor);
	if (major!=LIBVLC_VERSION_MAJOR || minor<LIBVLC_VERSION_MINOR) {
		return "Possibly incompatible libvlc version - please recompile emAvServerProc_vlc.";
	}

	audioEnabled=(strcmp(audioDrv,"auto")==0);
	videoEnabled=(strcmp(videoDrv,"emAv")==0);

	inst=(emAvInstance*)calloc(1,sizeof(emAvInstance));
	emAvInstances[instIndex]=inst;
	emAvInstanceCount++;

	inst->VideoAdapter=emAvCreateVideoAdapter();

#ifndef HAVE_ONE_VLC_INSTANCE_PER_STREAM
	inst->VlcInstance=TheVlcInstance;
	if (!inst->VlcInstance) {
#endif
		/* Don't do a libvlc_clearerr() before libvlc_new(..). */
		inst->VlcInstance=libvlc_new(0,NULL);
		if (!inst->VlcInstance) {
			msg=libvlc_errmsg();
			snprintf(
				errBuf,sizeof(errBuf),"libvlc_new failed: %s",
				msg?msg:
#ifdef _WIN32
				"unknown error (maybe VLC plugins not installed?)"
#else
				"unknown error (maybe vlc-plugin-base not installed?)"
#endif
			);
			errBuf[sizeof(errBuf)-1]=0;
			emAvCloseInstance(instIndex);
			return errBuf;
		}
#ifndef HAVE_ONE_VLC_INSTANCE_PER_STREAM
		TheVlcInstance=inst->VlcInstance;
	}
#endif

	libvlc_clearerr();
#ifdef _WIN32
	/* On Windows, libvlc_media_new_path seems to expect UTF-8 for the
	   file path always. Bug? Workaround: */
	wchar_t * wbuf=NULL;
	int wlen=0;
	for (;;) {
		wlen=MultiByteToWideChar(CP_ACP,0,file,-1,wbuf,wlen);
		if (wlen<=0 || wbuf) break;
		wbuf=(wchar_t*)calloc(wlen+1,sizeof(wchar_t));
	}
	if (wlen<=0) {
		snprintf(errBuf,sizeof(errBuf),
		         "MultiByteToWideChar on file path failed: %u",
		         (unsigned)GetLastError());
		errBuf[sizeof(errBuf)-1]=0;
		if (wbuf) free(wbuf);
		emAvCloseInstance(instIndex);
		return errBuf;
	}
	char * ubuf=NULL;
	int ulen=0;
	for (;;) {
		ulen=WideCharToMultiByte(CP_UTF8,0,wbuf,wlen,ubuf,ulen,NULL,NULL);
		if (ulen<=0 || ubuf) break;
		ubuf=(char*)calloc(ulen+1,sizeof(char));
	}
	free(wbuf);
	if (ulen<=0) {
		snprintf(errBuf,sizeof(errBuf),
		         "WideCharToMultiByte on file path failed: %u",
		         (unsigned)GetLastError());
		errBuf[sizeof(errBuf)-1]=0;
		if (ubuf) free(ubuf);
		emAvCloseInstance(instIndex);
		return errBuf;
	}
	inst->VlcMedia=libvlc_media_new_path(inst->VlcInstance,ubuf);
	free(ubuf);
#else
	inst->VlcMedia=libvlc_media_new_path(inst->VlcInstance,file);
#endif
	if (!inst->VlcMedia) {
		msg=libvlc_errmsg();
		snprintf(
			errBuf,sizeof(errBuf),"failed to open %s: %s",
			file,msg?msg:"unknown error"
		);
		errBuf[sizeof(errBuf)-1]=0;
		emAvCloseInstance(instIndex);
		return errBuf;
	}

	libvlc_clearerr();
	if (libvlc_media_parse_with_options(
		inst->VlcMedia,
		libvlc_media_parse_local,
		-1
	)!=0) {
		msg=libvlc_errmsg();
		snprintf(
			errBuf,sizeof(errBuf),"failed to parse %s: %s",
			file,msg?msg:"unknown error"
		);
		errBuf[sizeof(errBuf)-1]=0;
		emAvCloseInstance(instIndex);
		return errBuf;
	}
	for (;;) {
		libvlc_media_parsed_status_t status=libvlc_media_get_parsed_status(inst->VlcMedia);
		if (status) {
			if (status==libvlc_media_parsed_status_done) {
				break;
			}
			else if (status==libvlc_media_parsed_status_skipped) {
				msg="skipped (not local)";
			}
			else if (status==libvlc_media_parsed_status_failed) {
				msg=libvlc_errmsg();
				if (!msg) msg="unknown error";
			}
			else if (status==libvlc_media_parsed_status_timeout) {
				msg="timeout";
			}
			else {
				msg="unknown parse status";
			}
			snprintf(errBuf,sizeof(errBuf),"failed to parse %s: %s",file,msg);
			errBuf[sizeof(errBuf)-1]=0;
			emAvCloseInstance(instIndex);
			return errBuf;
		}
		usleep(10000);
	}

	for (pass=1; ; pass++) {

		libvlc_clearerr();
		inst->VlcPlayer=libvlc_media_player_new_from_media(inst->VlcMedia);
		if (!inst->VlcPlayer) {
			msg=libvlc_errmsg();
			snprintf(
				errBuf,sizeof(errBuf),"failed to open player for %s: %s",
				file,msg?msg:"unknown error"
			);
			errBuf[sizeof(errBuf)-1]=0;
			emAvCloseInstance(instIndex);
			return errBuf;
		}

		libvlc_video_set_callbacks(
			inst->VlcPlayer,
			emAvVideoLockCb,
			emAvVideoUnlockCb,
			emAvVideoDisplayCb,
			inst->VideoAdapter
		);

		libvlc_video_set_format_callbacks(
			inst->VlcPlayer,
			emAvVideoFormatCb,
			emAvVideoCleanupCb
		);

		if (!audioEnabled) {
			libvlc_audio_set_callbacks(
				inst->VlcPlayer,emAvDummyAudioPlayCb,NULL,NULL,NULL,NULL,NULL
			);
		}

		inst->VideoAdapter->OutputEnabled=videoEnabled;

		emAvFetchTracks(inst);

		if (pass>1 || emAvIsTracksInfoComplete(inst)) break;

		/* For some file formats, libvlc_media_parse_with_options does
		   not obtain all essential infos (i.e. whether there is a video
		   track, length, aspect ratio). In that case, we start playing
		   without any output in order to get the desired infos. The
		   player must be recreated afterwards when to undo
		   libvlc_audio_set_callbacks. */

		inst->VideoAdapter->OutputEnabled=false;
		if (audioEnabled) {
			libvlc_audio_set_callbacks(
				inst->VlcPlayer,emAvDummyAudioPlayCb,NULL,NULL,NULL,NULL,NULL
			);
		}

		inst->VideoAdapter->FrameCount=0;

		EM_AV_LOG("Playing to no output in order to get track infos");
		libvlc_media_player_play(inst->VlcPlayer);

		for (i=0; ; i++) {
			emAvFetchTracks(inst);
			if (emAvIsTracksInfoComplete(inst)) break;

			/* Hint: Waiting for a video format callback is not enough,
			   but waiting for two video lock callbacks mostly is. */

			pthread_mutex_lock(&inst->VideoAdapter->Mutex);
			fc=inst->VideoAdapter->FrameCount;
			pthread_mutex_unlock(&inst->VideoAdapter->Mutex);

			if (fc>=2 || i>200) break;
			usleep(10000);
		}

		emAvStartTimeoutThread(3000,"libvlc_media_player_stop in emAvOpenInstance");
		libvlc_media_player_stop(inst->VlcPlayer);
		emAvStopTimeoutThread();
		EM_AV_LOG("Playing to no output stopped");

		if (!audioEnabled) {
			inst->VideoAdapter->OutputEnabled=videoEnabled;
			break;
		}

		/* Recreate player in order to undo libvlc_audio_set_callbacks(..) */
		emAvStartTimeoutThread(3000,"libvlc_media_player_release in emAvOpenInstance");
		libvlc_media_player_release(inst->VlcPlayer);
		emAvStopTimeoutThread();
		inst->VlcPlayer=NULL;
	}

	emAvPollProperties(instIndex,1);

	return NULL;
}


static void emAvHandleMsg(int instIndex, const char * tag, const char * data)
{
	const char * err, * p1, * p2, * p3;

	EM_AV_LOG(tag);

	if (instIndex<0 || instIndex>=EM_AV_MAX_INSTANCES) {
		emAvSendMsg(instIndex,"error","Instance index out of range.");
	}
	else if (strcmp(tag,"open")==0) {
		p1=data;
		if ((p2=strchr(p1,':'))==NULL || (p3=strchr(p2+1,':'))==NULL) {
			emAvSendMsg(instIndex,"error","Illegal open request.");
			return;
		}
		*((char*)p2++)=0;
		*((char*)p3++)=0;
		err=emAvOpenInstance(instIndex,p1,p2,p3);
		if (err) emAvSendMsg(instIndex,"error","%s",err);
		else emAvSendMsg(instIndex,"ok",tag);
	}
	else if (strcmp(tag,"close")==0) {
		emAvCloseInstance(instIndex);
		emAvSendMsg(instIndex,"ok",tag);
	}
	else if (strcmp(tag,"attachshm")==0) {
		err=emAvAttachShm(instIndex,data);
		if (err) emAvSendMsg(instIndex,"error","%s",err);
		else emAvSendMsg(instIndex,"ok",tag);
	}
	else if (strcmp(tag,"detachshm")==0) {
		emAvDetachShm(instIndex);
		emAvSendMsg(instIndex,"ok",tag);
	}
	else if (strcmp(tag,"set")==0) {
		p1=data;
		p2=strchr(p1,':');
		if (p2) *((char*)p2++)=0; else p2="";
		err=emAvSetProperty(instIndex,p1,p2);
		if (err) emAvSendMsg(instIndex,"error","%s",err);
		else emAvSendMsg(instIndex,"ok","set:%s",p1);
	}
	else {
		emAvSendMsg(instIndex,"error","Unknown tag: %s",tag);
	}
}


/*============================================================================*/
/*==================================== main ==================================*/
/*============================================================================*/

int emAvServe(int argc, char * argv[])
{
	static const int dtInc=  1000;
	static const int dtMax= 25000;
	static const int tPoll=200000;
	static const int tAV  = 50000;
	const char * tag, * data;
	int i,t,t2,dt,instIndex;

	emAvInitPipe();
	emAvInitInstances();
	for (t=0, t2=0, dt=0;;) {
		i=emAvReceiveMsg(&instIndex,&tag,&data);
		if (i) {
			if (i<0) break;
			emAvHandleMsg(instIndex,tag,data);
			dt=0;
		}
		else {
			if (t>=tPoll) {
				t=0;
				for (i=0; i<EM_AV_MAX_INSTANCES; i++) {
					if (emAvInstances[i]) emAvPollProperties(i,0);
				}
			}
			if (t2>=tAV) {
				t2=0;
				for (i=0; i<EM_AV_MAX_INSTANCES; i++) {
					if (emAvInstances[i]) emAvUpdateAudioVisu(emAvInstances[i]);
				}
			}
			emAvFlushPipe();
			dt+=dtInc;
			if (dt>dtMax) dt=dtMax;
			usleep(dt);
			t+=dt;
			t2+=dt;
		}
	}
	for (i=0; i<EM_AV_MAX_INSTANCES; i++) {
		if (emAvInstances[i]) emAvCloseInstance(i);
	}
	return 0;
}


#ifdef _WIN32
static DWORD WINAPI emAvServeThreadProc(LPVOID data)
{
	return emAvServe(__argc,__argv);
}
#endif


int main(int argc, char * argv[])
{
#ifdef _WIN32
	HANDLE hdl;
	DWORD d;
	MSG msg;

	setmode(fileno(stdout),O_BINARY);
	setmode(fileno(stdin),O_BINARY);
	setbuf(stderr,NULL);

	hdl=CreateThread(NULL,0,emAvServeThreadProc,NULL,0,&d);
	do {
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message==WM_QUIT) ExitProcess(143);
		}
		d=MsgWaitForMultipleObjects(1,&hdl,FALSE,INFINITE,QS_ALLPOSTMESSAGE);
	} while(d==WAIT_OBJECT_0+1);
	WaitForSingleObject(hdl,INFINITE);
	GetExitCodeThread(hdl,&d);
	return d;
#else
	return emAvServe(argc,argv);
#endif
}
