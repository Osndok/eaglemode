/*------------------------------------------------------------------------------
// emAvServerProc_vlc.c
//
// Copyright (C) 2018 Oliver Hamann.
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



/*??????????????????????????????????????????????????????????????????????????????
????????????????????????????????????????????????????????????????????????????????
???                                                                         ????
???                                                                         ????
???          THIS SOURCE CODE IS STILL EXPERIMENTAL AND UNFINISHED!!!       ????
???                                                                         ????
???                                                                         ????
????????????????????????????????????????????????????????????????????????????????
??????????????????????????????????????????????????????????????????????????????*/



#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
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
/*============================= Helper Functions =============================*/
/*============================================================================*/

/*static void emAvCpDynStr(char * * pTgt, const char * src)
{
	if (*pTgt) {
		free(*pTgt);
		*pTgt=NULL;
	}
	if (src) {
		*pTgt=strdup(src);
	}
}*/


static void emAvCpDynStrArr(char * * * pTgt, const char * const * src)
{
	char * * tgt;
	int i;

	tgt=*pTgt;
	if (tgt) {
		for (i=0; tgt[i]; i++) free(tgt[i]);
		free(tgt);
		*pTgt=NULL;
	}
	if (src) {
		for (i=0; src[i]; i++);
		tgt=(char**)malloc(sizeof(char*)*(i+1));
		for (i=0; src[i]; i++) tgt[i]=strdup(src[i]);
		tgt[i]=NULL;
		*pTgt=tgt;
	}
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
			usleep(1000);
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
	CreateThread(NULL,0,emAvRcvThreadProc,NULL,0,&d);
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
	if (emAvPipeIn==-1) {
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
	pthread_mutex_t Mutex;
	libvlc_instance_t * VlcInstance;
	libvlc_media_t * VlcMedia;
	libvlc_media_player_t * VlcPlayer;
	emAvPlayState State;
	int NewMinShmSize;
	int ShmSize;
#if defined(_WIN32) || defined(__CYGWIN__)
	HANDLE ShmHdl;
#endif
	void * ShmPtr;
	char FormatChroma[64];
	int FormatWidth;
	int FormatHeight;
	int FormatPitch;
	int FormatLines;
	int CurrentAudioVisu;
	int MinShmSize;
	int Status;
	int SpeedParam;
	int PlayLength;
	int PlayPos;
	int AudioVolume;
	int AudioMute;
	int Warnings;
	char * * AudioVisus;
	char * * AudioChannels;
	char * * SpuChannels;
	int AudioVisu;
	int AudioChannel;
	int SpuChannel;
} emAvInstance;

#define EM_AV_WARNING_NO_AUDIO_DRIVER (1<<0)
#define EM_AV_WARNING_NO_AUDIO_CODEC  (1<<1)
#define EM_AV_WARNING_NO_VIDEO_CODEC  (1<<2)

#define EM_AV_MAX_INSTANCES 512
static emAvInstance * emAvInstances[EM_AV_MAX_INSTANCES];
static int emAvInstanceCount=0;

/*???:*/
#define HAVE_ONE_VLC_INSTANCE_PER_STREAM

#ifndef HAVE_ONE_VLC_INSTANCE_PER_STREAM
static libvlc_instance_t * TheVlcInstance=NULL;
#endif


static void emAvInitInstances()
{
	memset(emAvInstances,0,sizeof(emAvInstances));
}


static void emAvSendInfo(int instIndex)
{
	emAvInstance * inst;
	libvlc_media_track_t * * tracks;
	char * val;
	int i,n;

	inst=emAvInstances[instIndex];

	emAvBeginMsg(instIndex,"set");
	emAvContMsg(":info:");

	static struct {
		libvlc_meta_t meta;
		const char * name;
	} metas[]={
		{ libvlc_meta_Title      , "Title      " },
		{ libvlc_meta_Artist     , "Artist     " },
		{ libvlc_meta_Genre      , "Genre      " },
		{ libvlc_meta_Copyright  , "Copyright  " },
		{ libvlc_meta_Album      , "Album      " },
		{ libvlc_meta_TrackNumber, "TrackNumber" },
		{ libvlc_meta_Description, "Description" },
		{ libvlc_meta_Rating     , "Rating     " },
		{ libvlc_meta_Date       , "Date       " },
		{ libvlc_meta_Setting    , "Setting    " },
		{ libvlc_meta_URL        , "URL        " },
		{ libvlc_meta_Language   , "Language   " },
		{ libvlc_meta_NowPlaying , "NowPlaying " },
		{ libvlc_meta_Publisher  , "Publisher  " },
		{ libvlc_meta_EncodedBy  , "EncodedBy  " },
		{ libvlc_meta_ArtworkURL , "ArtworkURL " },
		{ libvlc_meta_TrackID    , "TrackID    " },
		{ libvlc_meta_TrackTotal , "TrackTotal " },
		{ libvlc_meta_Director   , "Director   " },
		{ libvlc_meta_Season     , "Season     " },
		{ libvlc_meta_Episode    , "Episode    " },
		{ libvlc_meta_ShowName   , "ShowName   " },
		{ libvlc_meta_Actors     , "Actor      " },
		{ (libvlc_meta_t)-1      , NULL          }
	};

	for (i=0; metas[i].name; i++) {
		val=libvlc_media_get_meta(inst->VlcMedia,metas[i].meta);
		if (i>0) emAvContMsg("\n");
		emAvContMsg("%s: %s",metas[i].name,val?val:"");
		if (val) free(val);
	}

	tracks=NULL;
	n=libvlc_media_tracks_get(inst->VlcMedia,&tracks);
	if (tracks) {
		for (i=0; i<n; i++) {
			if (tracks[i]->i_type==libvlc_track_video) {
				emAvContMsg(
					"\nVideo  : codec=%d size=%dx%d sar=%d/%d rate=%d/%d",
					tracks[i]->i_codec,
					tracks[i]->video->i_width,
					tracks[i]->video->i_height,
					tracks[i]->video->i_sar_num,
					tracks[i]->video->i_sar_den,
					tracks[i]->video->i_frame_rate_num,
					tracks[i]->video->i_frame_rate_den
				);
			}
			else if (tracks[i]->i_type==libvlc_track_audio) {
				emAvContMsg(
					"\nAudio  : codec=%d channels=%d, rate=%d",
					tracks[i]->i_codec,
					tracks[i]->audio->i_channels,
					tracks[i]->audio->i_rate
				);
			}
			else if (tracks[i]->i_type==libvlc_track_text) {
				emAvContMsg(
					"\nSubtitle  : codec=%d encoding=%s",
					tracks[i]->i_codec,
					tracks[i]->subtitle->psz_encoding
				);
			}
		}
		libvlc_media_tracks_release(tracks,n);
	}


/*
	emAvContMsg("Title  : \n");
	emAvContMsg("Comment: \n");
	emAvContMsg("Artist : \n");
	emAvContMsg("Genre  : \n");
	emAvContMsg("Album  : \n");
	emAvContMsg("Year   : \n");
	emAvContMsg("\n");
	emAvContMsg("Length : \n");
	emAvContMsg("System : \n");
	emAvContMsg("Audio  : \n");
	emAvContMsg("Video  :");
*/
	emAvEndMsg();
}


static void emAvPollProperties(int instIndex, int initialize)
{
	emAvInstance * inst;
	const char * str;
	libvlc_state_t vlcState;
	emAvPlayState state;
	int val,playing;
	float rate;

	inst=emAvInstances[instIndex];

	pthread_mutex_lock(&inst->Mutex);

	/* minshmsize */
	if (inst->MinShmSize<inst->NewMinShmSize || initialize) {
		inst->MinShmSize=inst->NewMinShmSize;
		emAvSendMsg(instIndex,"minshmsize","%d",inst->MinShmSize);
	}

	/* type */
	if (initialize) {
		emAvSendMsg(instIndex,"set","type:video");
	}

	/* state */
	vlcState=libvlc_media_get_state(inst->VlcMedia);
	playing=libvlc_media_player_is_playing(inst->VlcPlayer);
	rate=libvlc_media_player_get_rate(inst->VlcPlayer);
	if (vlcState==libvlc_Paused) state=EMAV_PAUSED;
	else if (!playing) state=EMAV_STOPPED;
	else if (rate<0.75F) state=EMAV_SLOW;
	else if (rate>1.25F) state=EMAV_FAST;
	else state=EMAV_NORMAL;
	if (initialize || inst->State!=state) {
		inst->State=state;
		if (state==EMAV_PAUSED) str="paused";
		else if (state==EMAV_NORMAL) str="normal";
		else if (state==EMAV_SLOW) str="slow";
		else if (state==EMAV_FAST) str="fast";
		else str="stopped";
		emAvSendMsg(instIndex,"set","state:%s",str);
	}

	/* length */
	val=(int)libvlc_media_player_get_length(inst->VlcPlayer);
	if (val<=0) {
		val=(int)libvlc_media_get_duration(inst->VlcMedia);
	}
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
		emAvSendMsg(instIndex,"set","aspect:%d",(4<<16)/3);
	}

	/* audio_volume */
	if (initialize) {
		emAvSendMsg(instIndex,"set","audio_volume:%d",100);
	}

	/* audio_mute */
	if (initialize) {
		emAvSendMsg(instIndex,"set","audio_mute:off");
	}

	/* info */
	if (initialize) {
		emAvSendInfo(instIndex);
	}

	/* warning */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":warning:");
		emAvEndMsg();
	}

	/* audio_visus */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_visus");
		emAvEndMsg();
	}

	/* audio_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_channels");
		emAvEndMsg();
	}

	/* audio_channel */
	if (initialize) {
		emAvSendMsg(instIndex,"set","audio_channel:");
	}

	/* spu_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":spu_channels");
		emAvEndMsg();
	}

	/* spu_channel */
	if (initialize) {
		emAvSendMsg(instIndex,"set","spu_channel:");
	}

	pthread_mutex_unlock(&inst->Mutex);
}


static void emAvSetAudioVisu(int instIndex, int audioVisu)
{
	emAvInstance * inst;

	inst=emAvInstances[instIndex];
	if (!inst) return;

	inst->CurrentAudioVisu=audioVisu;
}


static const char *  emAvSetProperty(
	int instIndex, const char * name, const char * value
)
{
	emAvInstance * inst;
	emAvPlayState state;

	inst=emAvInstances[instIndex];
	if (!inst) return "Not an opened instance.";

	pthread_mutex_lock(&inst->Mutex);

	if (strcmp(name,"pos")==0) {
		inst->PlayPos=atoi(value);
		if (inst->State!=EMAV_STOPPED) {
			/*??? libvlc_media_player_set_position isn't better... */
			libvlc_media_player_set_time(inst->VlcPlayer,inst->PlayPos);
		}
	}
	else if (strcmp(name,"state")==0) {
		if (strcmp(value,"paused")==0) state=EMAV_PAUSED;
		else if (strcmp(value,"normal")==0) state=EMAV_NORMAL;
		else if (strcmp(value,"fast")==0) state=EMAV_FAST;
		else if (strcmp(value,"slow")==0) state=EMAV_SLOW;
		else state=EMAV_STOPPED;
		if (state==EMAV_STOPPED) {
			libvlc_media_player_stop(inst->VlcPlayer);
		}
		else if (state==EMAV_PAUSED) {
			libvlc_media_player_play(inst->VlcPlayer);
			libvlc_media_player_set_pause(inst->VlcPlayer,1);
			if (inst->State==EMAV_STOPPED && inst->PlayPos>0) {
				libvlc_media_player_set_time(inst->VlcPlayer,inst->PlayPos);
			}
		}
		else {
			libvlc_media_player_play(inst->VlcPlayer);
			libvlc_media_player_set_pause(inst->VlcPlayer,0);
			if (inst->State==EMAV_STOPPED && inst->PlayPos>0) {
				libvlc_media_player_set_time(inst->VlcPlayer,inst->PlayPos);
			}
			libvlc_media_player_set_rate(
				inst->VlcPlayer,
				state==EMAV_FAST ? 4.0F :
				state==EMAV_SLOW ? 0.25F :
				1.0F
			);
		}
		inst->State=state;
	}
	else if (strcmp(name,"audio_volume")==0) {
	}
	else if (strcmp(name,"audio_mute")==0) {
	}
	else if (strcmp(name,"audio_visu")==0) {
	}
	else if (strcmp(name,"audio_channel")==0) {
	}
	else if (strcmp(name,"spu_channel")==0) {
	}
	else {
		pthread_mutex_unlock(&inst->Mutex);
		return "Unknown property.";
	}

	pthread_mutex_unlock(&inst->Mutex);
	return NULL;
}


static void emAvDetachShm(int instIndex)
{
	emAvInstance * inst;

	inst=emAvInstances[instIndex];
	if (!inst) return;

	pthread_mutex_lock(&inst->Mutex);

	if (inst->ShmPtr) {
#if defined(_WIN32) || defined(__CYGWIN__)
		UnmapViewOfFile(inst->ShmPtr);
		inst->ShmPtr=NULL;
		CloseHandle(inst->ShmHdl);
		inst->ShmHdl=NULL;
#else
		shmdt((const void*)inst->ShmPtr);
		inst->ShmPtr=NULL;
#endif
		inst->ShmSize=0;
	}

	pthread_mutex_unlock(&inst->Mutex);
}


static const char * emAvAttachShm(int instIndex, const char * params)
{
	emAvInstance * inst;
	const char * err;
	int shmSize;
#if defined(_WIN32) || defined(__CYGWIN__)
	char shmId[256];
	const char * p;
#else
	int shmId;
#endif

	err=NULL;
	inst=emAvInstances[instIndex];
	if (!inst) {
		err="Not an opened instance.";
	}
	else {
		pthread_mutex_lock(&inst->Mutex);

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
			if (inst->ShmPtr) {
				UnmapViewOfFile(inst->ShmPtr);
				inst->ShmPtr=NULL;
				CloseHandle(inst->ShmHdl);
				inst->ShmHdl=NULL;
				inst->ShmSize=0;
			}
			inst->ShmHdl=OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,shmId);
			if (!inst->ShmHdl) {
				err="Failed to attach shared memory (OpenFileMapping failed)";
			}
			else {
				inst->ShmPtr=MapViewOfFile(inst->ShmHdl,FILE_MAP_ALL_ACCESS,0,0,0);
				if (!inst->ShmPtr) {
					CloseHandle(inst->ShmHdl);
					inst->ShmHdl=NULL;
					err="Failed to attach shared memory (MapViewOfFile failed)";
				}
				else {
					inst->ShmSize=shmSize;
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
			if (inst->ShmPtr) {
				shmdt((const void*)inst->ShmPtr);
				inst->ShmPtr=NULL;
				inst->ShmSize=0;
			}
			inst->ShmPtr=shmat(shmId,NULL,0);
			if (inst->ShmPtr==(void*)-1) {
				inst->ShmPtr=NULL;
				err="Failed to attach shm.";
			}
			else {
				inst->ShmSize=shmSize;
			}
		}

#endif

		pthread_mutex_unlock(&inst->Mutex);
	}

	return err;
}


static void * emAvVideoLockCb(void * opaque, void * * planes)
{
	emAvInstance * inst;
	int minShmSize;
	int * pi;

	/*??? "Those planes must be aligned on 32-bytes boundaries" */

	inst=(emAvInstance*)opaque;
	pthread_mutex_lock(&inst->Mutex);

	minShmSize=inst->FormatPitch*inst->FormatLines+256;
	if (!inst->ShmPtr || inst->ShmSize<minShmSize) {
		*planes=NULL;
		return NULL;
	}

	pi=(int*)inst->ShmPtr;
	if (pi[0]) {
		*planes=NULL;
		return NULL;
	}
	pi[1]=inst->FormatWidth;
	pi[2]=inst->FormatHeight;
	pi[3]=65536*inst->FormatWidth/inst->FormatHeight;
	pi[4]=0;
	pi[5]=inst->FormatPitch;
	*planes=(void*)(pi+6);

	return *planes;
}


static void emAvVideoUnlockCb(void * opaque, void * picture, void * const * planes)
{
	emAvInstance * inst;
	int * pi;

	inst=(emAvInstance*)opaque;

	if (picture) {
		pi=(int*)inst->ShmPtr;

		int x,y;
		for (y=0; y<inst->FormatHeight; y++) {
			for (x=0; x<inst->FormatWidth; x++) {
				char * p=(char*)(pi+6);
				p+=y*inst->FormatPitch;
				p+=3*x;
				char t=p[0];
				p[0]=p[2];
				p[2]=t;
			}
		}

		pi[0]=1;
	}
	pthread_mutex_unlock(&inst->Mutex);
}


static void emAvVideoDisplayCb(void * opaque, void * picture)
{
}


static unsigned emAvVideoFormatCb(
	void * * opaque, char * chroma, unsigned * width, unsigned * height,
	unsigned * pitches, unsigned * lines
)
{
	emAvInstance * inst;
	int minShmSize;

	inst=(emAvInstance*)*opaque;

	pthread_mutex_lock(&inst->Mutex);

	strcpy(chroma,"RV24");
	*pitches=(((*width)*3)+31)&~31;
	*lines=((*height)+31)&~31;

	strcpy(inst->FormatChroma,chroma);
	inst->FormatWidth=*width;
	inst->FormatHeight=*height;
	inst->FormatPitch=*pitches;
	inst->FormatLines=*lines;

	minShmSize=inst->FormatPitch*inst->FormatLines+256;
	if (inst->NewMinShmSize<minShmSize) {
		inst->NewMinShmSize=minShmSize;
	}

/*
	strcpy(chroma,"RV24");
	*width=256;
	*height=192;
	*pitches=256*3;
	*lines=192;
*/

	pthread_mutex_unlock(&inst->Mutex);
	return 1;
/* Possible formats (chromas) seem to be:
GREY, I240, RV16, RV15, RV24, RV32, YUY2, YUYV, UYVY, I41N, I422,
I420, I411, I410, MJPG
*/

/*
	libvlc_video_set_format(inst->VlcPlayer,"RV24",256,192,256*3);
*/
}


static void emAvVideoCleanupCb(void * opaque)
{
	emAvInstance * inst;

	inst=(emAvInstance*)opaque;
}


static void emAvCloseInstance(int instIndex)
{
	emAvInstance * inst;

	inst=emAvInstances[instIndex];
	if (!inst) return;

	emAvDetachShm(instIndex);

	emAvSetAudioVisu(instIndex,0);

	if (inst->VlcPlayer) {
		libvlc_media_player_release(inst->VlcPlayer);
		inst->VlcPlayer=NULL;
	}

	if (inst->VlcMedia) {
		libvlc_media_release(inst->VlcMedia);
		inst->VlcMedia=NULL;
	}

#ifdef HAVE_ONE_VLC_INSTANCE_PER_STREAM
	if (inst->VlcInstance) libvlc_release(inst->VlcInstance);
#endif

	emAvCpDynStrArr(&inst->AudioVisus,NULL);
	emAvCpDynStrArr(&inst->AudioChannels,NULL);
	emAvCpDynStrArr(&inst->SpuChannels,NULL);

	pthread_mutex_destroy(&inst->Mutex);

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
	const char * msg;
	emAvInstance * inst;

	if (emAvInstances[instIndex]) return "Instance already open.";

	/*??? Maybe check version here. See libvlc_get_version and LIBVLC_VERSION_* */

	inst=(emAvInstance*)calloc(1,sizeof(emAvInstance));
	emAvInstances[instIndex]=inst;
	emAvInstanceCount++;

	pthread_mutex_init(&inst->Mutex,NULL);

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
				msg?msg:"unknown error"
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
	inst->VlcMedia=libvlc_media_new_path(inst->VlcInstance,file);
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

#if 1
	libvlc_media_parse(inst->VlcMedia);
#elif 1
	/*???:*/
	while (!libvlc_media_is_parsed(inst->VlcMedia)) {
		usleep(1000);
	}
#else /*??? newer version*/
	libvlc_clearerr();
	if (libvlc_media_parse_with_options(
		inst->VlcMedia,
		libvlc_media_parse_local|libvlc_media_fetch_local,
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
	/*???:*/
	while (!libvlc_media_is_parsed(inst->VlcMedia)) {
		usleep(1000);
	}
#endif

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
		inst
	);

	libvlc_video_set_format_callbacks(
		inst->VlcPlayer,
		emAvVideoFormatCb,
		emAvVideoCleanupCb
	);

	inst->Warnings|=EM_AV_WARNING_NO_AUDIO_DRIVER;

	emAvPollProperties(instIndex,1);

	return NULL;
}


static void emAvHandleMsg(int instIndex, const char * tag, const char * data)
{
	const char * err, * p1, * p2, * p3;

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
	static const int dtInc=1000;
	static const int dtMax=50000;
	static const int tPoll=200000;
	const char * tag, * data;
	int i,t,dt,instIndex;

	emAvInitPipe();
	emAvInitInstances();
	for (t=0, dt=0;;) {
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
			emAvFlushPipe();
			dt+=dtInc;
			if (dt>dtMax) dt=dtMax;
			usleep(dt);
			t+=dt;
		}
		for (i=0; i<EM_AV_MAX_INSTANCES; i++) {
			if (emAvInstances[i]) {
				/*???emAv_raw_visual_update_out_params(
					emAvInstances[i]->MyRawVisual,
					emAvInstances[i]->Stream
				);*/
			}
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
