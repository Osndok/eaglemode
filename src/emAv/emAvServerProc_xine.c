/*------------------------------------------------------------------------------
// emAvServerProc_xine.c
//
// Copyright (C) 2008,2010-2012 Oliver Hamann.
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xine.h>
#include <xine/xine_internal.h>


/*============================================================================*/
/*============================= Helper Functions =============================*/
/*============================================================================*/

static void emAvCpDynStr(char * * pTgt, const char * src)
{
	if (*pTgt) {
		free(*pTgt);
		*pTgt=NULL;
	}
	if (src) {
		*pTgt=strdup(src);
	}
}


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


static const char * emAv_get_xine_error(xine_stream_t * stream)
{
	static char buf[256];
	int e;

	e=xine_get_error(stream);
	switch (e) {
	case XINE_ERROR_NONE           : return "No error reported.";
	case XINE_ERROR_NO_INPUT_PLUGIN: return "No input plugin.";
	case XINE_ERROR_NO_DEMUX_PLUGIN: return "No demux plugin.";
	case XINE_ERROR_DEMUX_FAILED   : return "Demux failed.";
	case XINE_ERROR_MALFORMED_MRL  : return "Malformed MRL.";
	case XINE_ERROR_INPUT_FAILED   : return "Input failed.";
	default:
		sprintf(buf,"xine error code %d.",e);
		return buf;
	}
}


/*============================================================================*/
/*============================== emAv_vo_frame_t =============================*/
/*============================================================================*/

typedef struct {
	vo_frame_t base;
	char base_extra[256]; /* Improves binary compatibility a little. */
	int width;
	int height;
	int format;
	int aspect_ratio;
	int min_shm_size;
	int sizes[3];
	uint8_t * mem[3];
} emAv_vo_frame_t;


static void emAv_vo_frame_field(vo_frame_t * vo_img, int which_field)
{
}


static void emAv_vo_frame_dispose(vo_frame_t * vo_img)
{
	emAv_vo_frame_t * frame=(emAv_vo_frame_t*)vo_img;
	int i;

	for (i=0; i<3; i++) if (frame->mem[i]) free(frame->mem[i]);
	pthread_mutex_destroy(&frame->base.mutex);
	memset(frame,0,sizeof(emAv_vo_frame_t));
	free(frame);
}


/*============================================================================*/
/*============================== emAv_vo_driver_t ============================*/
/*============================================================================*/

typedef struct {
	vo_driver_t base;
	char base_extra[256]; /* Improves binary compatibility a little. */
	alphablend_t ov_alphablend;
	pthread_mutex_t mutex;
	int min_shm_size;
	int shm_size;
	void volatile * shm_ptr;
} emAv_vo_driver_t;


static uint32_t emAv_vo_driver_get_capabilities(vo_driver_t * self)
{
	return VO_CAP_YUY2|VO_CAP_YV12;
}


static vo_frame_t * emAv_vo_driver_alloc_frame(vo_driver_t * self)
{
	emAv_vo_frame_t * frame;

	frame=(emAv_vo_frame_t*)calloc(1,sizeof(emAv_vo_frame_t));
	pthread_mutex_init(&frame->base.mutex,NULL);
	frame->base.driver=self;
	frame->base.field=emAv_vo_frame_field;
	frame->base.dispose=emAv_vo_frame_dispose;
	return (vo_frame_t*)frame;
}


static void emAv_vo_driver_update_frame_format(
	vo_driver_t * self, vo_frame_t * vo_img, uint32_t width, uint32_t height,
	double ratio, int format, int flags
)
{
	emAv_vo_driver_t * driver=(emAv_vo_driver_t*)self;
	emAv_vo_frame_t * frame=(emAv_vo_frame_t*)vo_img;
	int i;

	if (
		frame->width!=(int)width || frame->height!=(int)height ||
		frame->format!=format
	) {
		frame->width=width;
		frame->height=height;
		frame->format=format;
		if (format==XINE_IMGFMT_YV12) {
			frame->base.pitches[0]=8*((width+7)/8);
			frame->base.pitches[1]=8*((width+15)/16);
			frame->base.pitches[2]=8*((width+15)/16);
			frame->sizes[0]=frame->base.pitches[0]*height;
			frame->sizes[1]=frame->base.pitches[1]*((height+1)/2);
			frame->sizes[2]=frame->base.pitches[2]*((height+1)/2);
			frame->min_shm_size=7*sizeof(int)+frame->sizes[0]+frame->sizes[1]+frame->sizes[2];
		}
		else { /* format==XINE_IMGFMT_YUY2 */
			frame->base.pitches[0]=8*((width+3)/4);
			frame->sizes[0]=frame->base.pitches[0]*height;
			frame->sizes[1]=0;
			frame->sizes[2]=0;
			frame->min_shm_size=6*sizeof(int)+frame->sizes[0];
		}
		for (i=0; i<3; i++) {
			frame->base.base[i]=NULL;
			if (frame->mem[i]) {
				free(frame->mem[i]);
				frame->mem[i]=NULL;
			}
			if (frame->sizes[i]>0) {
				frame->mem[i]=calloc(1,frame->sizes[i]+255);
				frame->base.base[i]=(uint8_t*)frame->mem[i];
				frame->base.base[i]+=(0-((size_t)frame->base.base[i]))&255;
			}
		}
		pthread_mutex_lock(&driver->mutex);
		if (driver->min_shm_size<frame->min_shm_size) driver->min_shm_size=frame->min_shm_size;
		pthread_mutex_unlock(&driver->mutex);
	}
	frame->aspect_ratio=(int)(ratio*65536.0+0.5);
}


static void emAv_vo_driver_display_frame(
	vo_driver_t * self, vo_frame_t * vo_img
)
{
	emAv_vo_driver_t * driver=(emAv_vo_driver_t*)self;
	emAv_vo_frame_t * frame=(emAv_vo_frame_t*)vo_img;
	int volatile * pi;
	char volatile * pc;
	int i;

	pthread_mutex_lock(&driver->mutex);
	pi=(int volatile *)driver->shm_ptr;
	if (pi && driver->shm_size>=frame->min_shm_size && !pi[0]) {
		pi[1]=frame->width;
		pi[2]=frame->height;
		pi[3]=frame->aspect_ratio;
		if (frame->format==XINE_IMGFMT_YV12) {
			pi[4]=1;
			pi[5]=frame->base.pitches[0];
			pi[6]=frame->base.pitches[1];
			pc=(char volatile *)(pi+7);
			for (i=0; i<3; i++) {
				memcpy((void*)pc,frame->base.base[i],frame->sizes[i]);
				pc+=frame->sizes[i];
			}
		}
		else {
			pi[4]=2;
			pi[5]=frame->base.pitches[0];
			memcpy((void*)(pi+6),frame->base.base[0],frame->sizes[0]);
		}
		pi[0]=1;
	}
	pthread_mutex_unlock(&driver->mutex);

	frame->base.free(&frame->base);
}


static void emAv_vo_driver_overlay_blend(
	vo_driver_t * self, vo_frame_t * vo_img, vo_overlay_t * overlay
)
{
	emAv_vo_driver_t * driver=(emAv_vo_driver_t*)self;
	emAv_vo_frame_t * frame=(emAv_vo_frame_t*)vo_img;

	if (overlay->rle) {
		driver->ov_alphablend.offset_x=vo_img->overlay_offset_x;
		driver->ov_alphablend.offset_y=vo_img->overlay_offset_y;
		if (frame->format==XINE_IMGFMT_YV12) {
			_x_blend_yuv(
				frame->base.base,overlay,frame->width,frame->height,
				frame->base.pitches,&driver->ov_alphablend
			);
		}
		else {
			_x_blend_yuy2(
				frame->base.base[0],overlay,frame->width,frame->height,
				frame->base.pitches[0],&driver->ov_alphablend
			);
		}
	}
}


static int emAv_vo_driver_get_property(vo_driver_t * self, int property)
{
	return 0;
}


static int emAv_vo_driver_set_property(
	vo_driver_t * self, int property, int value
)
{
	return value;
}


static int emAv_vo_driver_gui_data_exchange(
	vo_driver_t * self, int data_type, void * data
)
{
	return 0;
}


static int emAv_vo_driver_redraw_needed(vo_driver_t * self)
{
	return 0;
}


static void emAv_vo_driver_dispose(vo_driver_t * self)
{
	emAv_vo_driver_t * driver=(emAv_vo_driver_t*)self;

	if (driver->shm_ptr) shmdt((const void*)driver->shm_ptr);
	pthread_mutex_destroy(&driver->mutex);
	_x_alphablend_free(&driver->ov_alphablend);
	memset(driver,0,sizeof(emAv_vo_driver_t));
	free(driver);
}


static emAv_vo_driver_t * emAv_vo_driver_create(xine_t * xine)
{
	emAv_vo_driver_t * driver;

	driver=(emAv_vo_driver_t*)calloc(1,sizeof(emAv_vo_driver_t));
	_x_alphablend_init(&driver->ov_alphablend,xine);
	pthread_mutex_init(&driver->mutex,NULL);
	driver->base.get_capabilities    =emAv_vo_driver_get_capabilities;
	driver->base.alloc_frame         =emAv_vo_driver_alloc_frame;
	driver->base.update_frame_format =emAv_vo_driver_update_frame_format;
	driver->base.display_frame       =emAv_vo_driver_display_frame;
	driver->base.overlay_blend       =emAv_vo_driver_overlay_blend;
	driver->base.get_property        =emAv_vo_driver_get_property;
	driver->base.set_property        =emAv_vo_driver_set_property;
	driver->base.gui_data_exchange   =emAv_vo_driver_gui_data_exchange;
	driver->base.redraw_needed       =emAv_vo_driver_redraw_needed;
	driver->base.dispose             =emAv_vo_driver_dispose;
	return driver;
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


static void emAvInitPipe()
{
	int f;

	emAvPipeInBuf=(char*)malloc(emAvPipeBufSize);
	emAvPipeOutBuf=(char*)malloc(emAvPipeBufSize);

	emAvPipeIn=dup(STDIN_FILENO);
	if (emAvPipeIn==-1) {
		fprintf(
			stderr,
			"emAvServerProc_xine: dup(STDIN_FILENO) failed: %s\n",
			strerror(errno)
		);
		exit(255);
	}
	close(STDIN_FILENO);

	if (
		(f=fcntl(emAvPipeIn,F_GETFL))<0 ||
		fcntl(emAvPipeIn,F_SETFL,f|O_NONBLOCK)<0
	) {
		fprintf(
			stderr,
			"emAvServerProc_xine: Failed to set pipe read handle to non-blocking mode: %s\n",
			strerror(errno)
		);
		exit(255);
	}

	emAvPipeOut=dup(STDOUT_FILENO);
	if (emAvPipeIn==-1) {
		fprintf(
			stderr,
			"emAvServerProc_xine: dup(STDOUT_FILENO) failed: %s\n",
			strerror(errno)
		);
		exit(255);
	}
	close(STDOUT_FILENO);
	if (dup2(STDERR_FILENO,STDOUT_FILENO)!=STDOUT_FILENO) {
		fprintf(
			stderr,
			"emAvServerProc_xine: dup2(STDERR_FILENO,STDOUT_FILENO) failed: %s\n",
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
				"emAvServerProc_xine: Could not write to pipe: %s\n",
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
				fprintf(stderr,"emAvServerProc_xine: Protocol error.\n");
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
			fprintf(stderr,"emAvServerProc_xine: Pipe input buffer too small.\n");
			exit(255);
		}
		l=read(emAvPipeIn,emAvPipeInBuf+emAvPipeInBufEnd,l);
		if (l<0) {
			if (errno==EAGAIN) return 0;
			fprintf(
				stderr,
				"emAvServerProc_xine: Could not read from pipe: %s\n",
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

typedef struct {
	xine_t * Xine;
	xine_audio_port_t * AudioPort;
	xine_video_port_t * VideoPort;
	emAv_vo_driver_t * MyVoDrv;
	xine_stream_t * Stream;
	xine_post_t * CurrentAudioVisuPost;
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

#define HAVE_ONE_XINE_PER_STREAM

#ifndef HAVE_ONE_XINE_PER_STREAM
static xine_t * TheXine=NULL;
#endif


static void emAvInitInstances()
{
	memset(emAvInstances,0,sizeof(emAvInstances));
}


static void emAvSendInfo(int instIndex)
{
#if 0 /* developer's view... */
	static struct {
		int param;
		const char * name;
	} params[]={
		{ XINE_PARAM_SPEED                 , "PARAM_SPEED                  " },
		{ XINE_PARAM_AV_OFFSET             , "PARAM_AV_OFFSET              " },
		{ XINE_PARAM_AUDIO_CHANNEL_LOGICAL , "PARAM_AUDIO_CHANNEL_LOGICAL  " },
		{ XINE_PARAM_SPU_CHANNEL           , "PARAM_SPU_CHANNEL            " },
		{ XINE_PARAM_VIDEO_CHANNEL         , "PARAM_VIDEO_CHANNEL          " },
		{ XINE_PARAM_AUDIO_VOLUME          , "PARAM_AUDIO_VOLUME           " },
		{ XINE_PARAM_AUDIO_MUTE            , "PARAM_AUDIO_MUTE             " },
		{ XINE_PARAM_AUDIO_COMPR_LEVEL     , "PARAM_AUDIO_COMPR_LEVEL      " },
		{ XINE_PARAM_AUDIO_AMP_LEVEL       , "PARAM_AUDIO_AMP_LEVEL        " },
		{ XINE_PARAM_AUDIO_REPORT_LEVEL    , "PARAM_AUDIO_REPORT_LEVEL     " },
		{ XINE_PARAM_VERBOSITY             , "PARAM_VERBOSITY              " },
		{ XINE_PARAM_SPU_OFFSET            , "PARAM_SPU_OFFSET             " },
		{ XINE_PARAM_IGNORE_VIDEO          , "PARAM_IGNORE_VIDEO           " },
		{ XINE_PARAM_IGNORE_AUDIO          , "PARAM_IGNORE_AUDIO           " },
		{ XINE_PARAM_IGNORE_SPU            , "PARAM_IGNORE_SPU             " },
		{ XINE_PARAM_BROADCASTER_PORT      , "PARAM_BROADCASTER_PORT       " },
		{ XINE_PARAM_METRONOM_PREBUFFER    , "PARAM_METRONOM_PREBUFFER     " },
		{ XINE_PARAM_EQ_30HZ               , "PARAM_EQ_30HZ                " },
		{ XINE_PARAM_EQ_60HZ               , "PARAM_EQ_60HZ                " },
		{ XINE_PARAM_EQ_125HZ              , "PARAM_EQ_125HZ               " },
		{ XINE_PARAM_EQ_250HZ              , "PARAM_EQ_250HZ               " },
		{ XINE_PARAM_EQ_500HZ              , "PARAM_EQ_500HZ               " },
		{ XINE_PARAM_EQ_1000HZ             , "PARAM_EQ_1000HZ              " },
		{ XINE_PARAM_EQ_2000HZ             , "PARAM_EQ_2000HZ              " },
		{ XINE_PARAM_EQ_4000HZ             , "PARAM_EQ_4000HZ              " },
		{ XINE_PARAM_EQ_8000HZ             , "PARAM_EQ_8000HZ              " },
		{ XINE_PARAM_EQ_16000HZ            , "PARAM_EQ_16000HZ             " },
		{ XINE_PARAM_AUDIO_CLOSE_DEVICE    , "PARAM_AUDIO_CLOSE_DEVICE     " },
		{ XINE_PARAM_AUDIO_AMP_MUTE        , "PARAM_AUDIO_AMP_MUTE         " },
		{ XINE_PARAM_FINE_SPEED            , "PARAM_FINE_SPEED             " },
		{ -1                               , NULL                            }
	};
	static struct {
		int param;
		const char * name;
	} stream_infos[]={
		{ XINE_STREAM_INFO_BITRATE          , "STREAM_INFO_BITRATE          " },
		{ XINE_STREAM_INFO_SEEKABLE         , "STREAM_INFO_SEEKABLE         " },
		{ XINE_STREAM_INFO_VIDEO_WIDTH      , "STREAM_INFO_VIDEO_WIDTH      " },
		{ XINE_STREAM_INFO_VIDEO_HEIGHT     , "STREAM_INFO_VIDEO_HEIGHT     " },
		{ XINE_STREAM_INFO_VIDEO_RATIO      , "STREAM_INFO_VIDEO_RATIO      " },
		{ XINE_STREAM_INFO_VIDEO_CHANNELS   , "STREAM_INFO_VIDEO_CHANNELS   " },
		{ XINE_STREAM_INFO_VIDEO_STREAMS    , "STREAM_INFO_VIDEO_STREAMS    " },
		{ XINE_STREAM_INFO_VIDEO_BITRATE    , "STREAM_INFO_VIDEO_BITRATE    " },
		{ XINE_STREAM_INFO_VIDEO_FOURCC     , "STREAM_INFO_VIDEO_FOURCC     " },
		{ XINE_STREAM_INFO_VIDEO_HANDLED    , "STREAM_INFO_VIDEO_HANDLED    " },
		{ XINE_STREAM_INFO_FRAME_DURATION   , "STREAM_INFO_FRAME_DURATION   " },
		{ XINE_STREAM_INFO_AUDIO_CHANNELS   , "STREAM_INFO_AUDIO_CHANNELS   " },
		{ XINE_STREAM_INFO_AUDIO_BITS       , "STREAM_INFO_AUDIO_BITS       " },
		{ XINE_STREAM_INFO_AUDIO_SAMPLERATE , "STREAM_INFO_AUDIO_SAMPLERATE " },
		{ XINE_STREAM_INFO_AUDIO_BITRATE    , "STREAM_INFO_AUDIO_BITRATE    " },
		{ XINE_STREAM_INFO_AUDIO_FOURCC     , "STREAM_INFO_AUDIO_FOURCC     " },
		{ XINE_STREAM_INFO_AUDIO_HANDLED    , "STREAM_INFO_AUDIO_HANDLED    " },
		{ XINE_STREAM_INFO_HAS_CHAPTERS     , "STREAM_INFO_HAS_CHAPTERS     " },
		{ XINE_STREAM_INFO_HAS_VIDEO        , "STREAM_INFO_HAS_VIDEO        " },
		{ XINE_STREAM_INFO_HAS_AUDIO        , "STREAM_INFO_HAS_AUDIO        " },
		{ XINE_STREAM_INFO_IGNORE_VIDEO     , "STREAM_INFO_IGNORE_VIDEO     " },
		{ XINE_STREAM_INFO_IGNORE_AUDIO     , "STREAM_INFO_IGNORE_AUDIO     " },
		{ XINE_STREAM_INFO_IGNORE_SPU       , "STREAM_INFO_IGNORE_SPU       " },
		{ XINE_STREAM_INFO_VIDEO_HAS_STILL  , "STREAM_INFO_VIDEO_HAS_STILL  " },
		{ XINE_STREAM_INFO_MAX_AUDIO_CHANNEL, "STREAM_INFO_MAX_AUDIO_CHANNEL" },
		{ XINE_STREAM_INFO_MAX_SPU_CHANNEL  , "STREAM_INFO_MAX_SPU_CHANNEL  " },
		{ XINE_STREAM_INFO_AUDIO_MODE       , "STREAM_INFO_AUDIO_MODE       " },
		{ XINE_STREAM_INFO_SKIPPED_FRAMES   , "STREAM_INFO_SKIPPED_FRAMES   " },
		{ XINE_STREAM_INFO_DISCARDED_FRAMES , "STREAM_INFO_DISCARDED_FRAMES " },
		{ -1                                , NULL                            }
	};
	static struct {
		int param;
		const char * name;
	} meta_infos[]={
		{ XINE_META_INFO_TITLE              , "META_INFO_TITLE              " },
		{ XINE_META_INFO_COMMENT            , "META_INFO_COMMENT            " },
		{ XINE_META_INFO_ARTIST             , "META_INFO_ARTIST             " },
		{ XINE_META_INFO_GENRE              , "META_INFO_GENRE              " },
		{ XINE_META_INFO_ALBUM              , "META_INFO_ALBUM              " },
		{ XINE_META_INFO_YEAR               , "META_INFO_YEAR               " },
		{ XINE_META_INFO_VIDEOCODEC         , "META_INFO_VIDEOCODEC         " },
		{ XINE_META_INFO_AUDIOCODEC         , "META_INFO_AUDIOCODEC         " },
		{ XINE_META_INFO_SYSTEMLAYER        , "META_INFO_SYSTEMLAYER        " },
		{ XINE_META_INFO_INPUT_PLUGIN       , "META_INFO_INPUT_PLUGIN       " },
		{ XINE_META_INFO_CDINDEX_DISCID     , "META_INFO_CDINDEX_DISCID     " },
		{ XINE_META_INFO_TRACK_NUMBER       , "META_INFO_TRACK_NUMBER       " },
		{ -1                                , NULL                            }
	};
	emAvInstance * inst;
	xine_stream_t * s;
	const char * str;
	uint32_t u;
	int i;

	inst=emAvInstances[instIndex];
	s=inst->Stream;

	emAvBeginMsg(instIndex,"set");
	emAvContMsg(":info:");

	for (i=0; params[i].name; i++) {
		u=xine_get_param(s,params[i].param);
		emAvContMsg("%s = %u\n",params[i].name,u);
	}

	for (i=0; stream_infos[i].name; i++) {
		u=xine_get_stream_info(s,stream_infos[i].param);
		emAvContMsg("%s = %u\n",stream_infos[i].name,u);
	}

	for (i=0; meta_infos[i].name; i++) {
		str=xine_get_meta_info(s,meta_infos[i].param);
		if (!str) str="";
		emAvContMsg("%s = %s\n",meta_infos[i].name,str);
	}

	emAvEndMsg();

#else

	emAvInstance * inst;
	xine_stream_t * s;
	const char * str, * sep;
	uint32_t u,u2;
	int pos_stream,pos_time,length_time,min,sec;

	inst=emAvInstances[instIndex];
	s=inst->Stream;

	emAvBeginMsg(instIndex,"set");
	emAvContMsg(":info:");

	str=xine_get_meta_info(s,XINE_META_INFO_TITLE);
	emAvContMsg("Title  : %s\n",str?str:"");

	str=xine_get_meta_info(s,XINE_META_INFO_COMMENT);
	emAvContMsg("Comment: %s\n",str?str:"");

	str=xine_get_meta_info(s,XINE_META_INFO_ARTIST);
	emAvContMsg("Artist : %s\n",str?str:"");

	str=xine_get_meta_info(s,XINE_META_INFO_GENRE);
	emAvContMsg("Genre  : %s\n",str?str:"");

	str=xine_get_meta_info(s,XINE_META_INFO_ALBUM);
	emAvContMsg("Album  : %s\n",str?str:"");

	str=xine_get_meta_info(s,XINE_META_INFO_YEAR);
	emAvContMsg("Year   : %s\n",str?str:"");

	emAvContMsg("\n");

	emAvContMsg("Length :");
	if (
		xine_get_pos_length(s,&pos_stream,&pos_time,&length_time) &&
		length_time>0
	) {
		sec=(length_time+500)/1000;
		min=sec/60;
		sec%=60;
		if (min>=30) emAvContMsg(" %d minutes",min+(sec+30)/60);
		else if (min) emAvContMsg(" %d minutes, %d seconds",min,sec);
		else emAvContMsg(" %d seconds",sec);
	}
	emAvContMsg("\n");

	emAvContMsg("System :");
	sep="";
	str=xine_get_meta_info(s,XINE_META_INFO_SYSTEMLAYER);
	if (str && *str) {
		emAvContMsg(" %s",str);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_BITRATE);
	if (u) emAvContMsg("%s %u bits/sec",sep,u);
	emAvContMsg("\n");

	emAvContMsg("Audio  :");
	sep="";
	str=xine_get_meta_info(s,XINE_META_INFO_AUDIOCODEC);
	if (str && *str) {
		emAvContMsg(" %s",str);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_AUDIO_BITRATE);
	if (u) {
		emAvContMsg("%s %u bits/sec",sep,u);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_AUDIO_BITS);
	if (u) {
		emAvContMsg("%s %u-bit",sep,u);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_AUDIO_SAMPLERATE);
	if (u) emAvContMsg("%s %u Hz",sep,u);
	emAvContMsg("\n");

	emAvContMsg("Video  :");
	sep="";
	str=xine_get_meta_info(s,XINE_META_INFO_VIDEOCODEC);
	if (str && *str) {
		emAvContMsg(" %s",str);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_VIDEO_BITRATE);
	if (u) {
		emAvContMsg("%s %u bits/sec",sep,u);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_VIDEO_WIDTH);
	u2=xine_get_stream_info(s,XINE_STREAM_INFO_VIDEO_HEIGHT);
	if (u && u2) {
		emAvContMsg("%s %ux%u pixels",sep,u,u2);
		sep=",";
	}
	u=xine_get_stream_info(s,XINE_STREAM_INFO_FRAME_DURATION);
	if (u) emAvContMsg("%s %u Hz",sep,(90000+u/2)/u);

	emAvEndMsg();
#endif
}


static void emAvPollProperties(int instIndex, int initialize)
{
	emAvInstance * inst;
	const char * str, * sep;
	int status,param,pos_stream,pos_time,length_time,sz,w,h,i,n;

	inst=emAvInstances[instIndex];

	/* minshmsize */
	if (inst->MyVoDrv) {
		pthread_mutex_lock(&inst->MyVoDrv->mutex);
		sz=inst->MyVoDrv->min_shm_size;
		pthread_mutex_unlock(&inst->MyVoDrv->mutex);
	}
	else {
		sz=0;
	}
	if (inst->MinShmSize<sz || initialize) {
		inst->MinShmSize=sz;
		emAvSendMsg(instIndex,"minshmsize","%d",sz);
	}

	/* type */
	if (initialize) {
		param=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_HAS_VIDEO);
		emAvSendMsg(instIndex,"set","type:%s",param?"video":"audio");
	}

	/* state */
	status=xine_get_status(inst->Stream);
	param=xine_get_param(inst->Stream,XINE_PARAM_SPEED);
	if (inst->Status!=status || inst->SpeedParam!=param || initialize) {
		inst->Status=status;
		inst->SpeedParam=param;
		if (status!=XINE_STATUS_PLAY) str="stopped";
		else if (param==XINE_SPEED_PAUSE) str="paused";
		else if (param==XINE_SPEED_SLOW_4) str="slow";
		else if (param==XINE_SPEED_FAST_4) str="fast";
		else str="normal";
		emAvSendMsg(instIndex,"set","state:%s",str);
	}

	/* length, pos */
	if (xine_get_pos_length(inst->Stream,&pos_stream,&pos_time,&length_time)) {
		if ((inst->PlayLength!=length_time && length_time>0) || initialize) {
			inst->PlayLength=length_time;
			emAvSendMsg(instIndex,"set","length:%d",length_time);
		}
		if (inst->PlayPos!=pos_time || initialize) {
			inst->PlayPos=pos_time;
			emAvSendMsg(instIndex,"set","pos:%d",pos_time);
		}
	}

	/* aspect */
	if (initialize) {
		param=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_VIDEO_RATIO);
		if (param>0) {
			param=(int)(param/10000.0*65536.0+0.5);
		}
		else {
			w=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_VIDEO_WIDTH);
			h=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_VIDEO_HEIGHT);
			if (h) param=(w<<16)/h;
			if (param<=0) param=(4<<16)/3;
		}
		emAvSendMsg(instIndex,"set","aspect:%d",param);
	}

	/* audio_volume */
	param=xine_get_param(inst->Stream,XINE_PARAM_AUDIO_AMP_LEVEL);
	if (inst->AudioVolume!=param || initialize) {
		inst->AudioVolume=param;
		emAvSendMsg(instIndex,"set","audio_volume:%d",param);
	}

	/* audio_mute */
	param=xine_get_param(inst->Stream,XINE_PARAM_AUDIO_AMP_MUTE);
	if (inst->AudioMute!=param || initialize) {
		inst->AudioMute=param;
		emAvSendMsg(instIndex,"set","audio_mute:%s",param?"on":"off");
	}

	/* info */
	if (initialize) {
		emAvSendInfo(instIndex);
	}

	/* warning */
	param=inst->Warnings;
	if (xine_get_status(inst->Stream)==XINE_STATUS_PLAY) {
		param&=~(EM_AV_WARNING_NO_AUDIO_CODEC|EM_AV_WARNING_NO_VIDEO_CODEC);
		if (
			!xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_AUDIO_HANDLED) &&
			xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_HAS_AUDIO)
		) {
			param|=EM_AV_WARNING_NO_AUDIO_CODEC;
		}
		if (
			!xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_VIDEO_HANDLED) &&
			xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_HAS_VIDEO)
		) {
			param|=EM_AV_WARNING_NO_VIDEO_CODEC;
		}
	}
	if (inst->Warnings!=param || initialize) {
		inst->Warnings=param;
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":warning:");
		sep="";
		if (param&EM_AV_WARNING_NO_VIDEO_CODEC) {
			emAvContMsg("No suitable video codec available.");
			sep="\n";
		}
		if (param&EM_AV_WARNING_NO_AUDIO_DRIVER) {
			emAvContMsg("%sFailed to prepare an audio driver.",sep);
			sep="\n";
		}
		else if (param&EM_AV_WARNING_NO_AUDIO_CODEC) {
			emAvContMsg("%sNo suitable audio codec available.",sep);
		}
		emAvEndMsg();
	}

	/* audio_visus */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_visus");
		for (i=0; inst->AudioVisus[i]; i++) {
			emAvContMsg(":%s",inst->AudioVisus[i]);
		}
		emAvEndMsg();
	}

	/* audio_visu */
	if (inst->AudioVisu!=inst->CurrentAudioVisu || initialize) {
		inst->AudioVisu=inst->CurrentAudioVisu;
		emAvSendMsg(
			instIndex,"set","audio_visu:%s",
			inst->AudioVisus[inst->AudioVisu]
		);
	}

	/* audio_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":audio_channels");
		for (i=0; inst->AudioChannels[i]; i++) {
			emAvContMsg(":%s",inst->AudioChannels[i]);
		}
		emAvEndMsg();
	}

	/* audio_channel */
	param=xine_get_stream_info(inst->Stream,XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
	for (n=0; inst->AudioChannels[n]; n++);
	if (param<-2 || param>=n-2) param=-2;
	if (inst->AudioChannel!=param || initialize) {
		inst->AudioChannel=param;
		emAvSendMsg(
			instIndex,"set","audio_channel:%s",
			param<n-2 ? inst->AudioChannels[param+2] : ""
		);
	}

	/* spu_channels */
	if (initialize) {
		emAvBeginMsg(instIndex,"set");
		emAvContMsg(":spu_channels");
		for (i=0; inst->SpuChannels[i]; i++) {
			emAvContMsg(":%s",inst->SpuChannels[i]);
		}
		emAvEndMsg();
	}

	/* spu_channel */
	param=xine_get_stream_info(inst->Stream,XINE_PARAM_SPU_CHANNEL);
	for (n=0; inst->SpuChannels[n]; n++);
	if (param<-2 || param>=n-2) param=-2;
	if (inst->SpuChannel!=param || initialize) {
		inst->SpuChannel=param;
		emAvSendMsg(
			instIndex,"set","spu_channel:%s",
			param<n-2 ? inst->SpuChannels[param+2] : ""
		);
	}
}


static void emAvSetAudioVisu(int instIndex, int audioVisu)
{
	emAvInstance * inst;
	xine_audio_port_t * audio_ports[2];
	xine_video_port_t * video_ports[2];

	inst=emAvInstances[instIndex];
	if (!inst) return;

	if (inst->CurrentAudioVisuPost) {
		xine_post_wire_audio_port(
			xine_get_audio_source(inst->Stream),
			inst->AudioPort
		);
		xine_post_dispose(inst->Xine,inst->CurrentAudioVisuPost);
		inst->CurrentAudioVisuPost=NULL;
	}

	inst->CurrentAudioVisu=audioVisu;

	if (audioVisu<=0) return;
	if (!inst->AudioPort) return;
	if (!inst->VideoPort) return;
	if (!inst->Stream) return;
	if (!inst->MyVoDrv) return;

	audio_ports[0]=inst->AudioPort;
	audio_ports[1]=NULL;
	video_ports[0]=inst->VideoPort;
	video_ports[1]=NULL;
	inst->CurrentAudioVisuPost=xine_post_init(
		inst->Xine,
		inst->AudioVisus[audioVisu],
		0,
		audio_ports,
		video_ports
	);
	if (!inst->CurrentAudioVisuPost) return;

	if (!xine_post_wire_audio_port(
		xine_get_audio_source(inst->Stream),
		inst->CurrentAudioVisuPost->audio_input[0]
	)) {
		xine_post_dispose(inst->Xine,inst->CurrentAudioVisuPost);
		inst->CurrentAudioVisuPost=NULL;
	}
}


static const char *  emAvSetProperty(
	int instIndex, const char * name, const char * value
)
{
	emAvInstance * inst;
	int param,i;

	inst=emAvInstances[instIndex];
	if (!inst) return "Not an opened instance.";

	if (strcmp(name,"pos")==0) {
		inst->PlayPos=atoi(value);
		if (xine_get_status(inst->Stream)==XINE_STATUS_PLAY) {
			xine_play(inst->Stream,0,inst->PlayPos);
			xine_set_param(inst->Stream,XINE_PARAM_SPEED,inst->SpeedParam);
		}
	}
	else if (strcmp(name,"state")==0) {
		if (strcmp(value,"stopped")==0) {
			inst->Status=XINE_STATUS_STOP;
			inst->SpeedParam=XINE_SPEED_NORMAL;
			xine_stop(inst->Stream);
		}
		else {
			if (strcmp(value,"paused")==0) param=XINE_SPEED_PAUSE;
			else if (strcmp(value,"slow")==0) param=XINE_SPEED_SLOW_4;
			else if (strcmp(value,"fast")==0) param=XINE_SPEED_FAST_4;
			else if (strcmp(value,"normal")==0) param=XINE_SPEED_NORMAL;
			else return "Illegal property value";
			inst->Status=XINE_STATUS_PLAY;
			inst->SpeedParam=param;
			if (xine_get_status(inst->Stream)!=XINE_STATUS_PLAY) {
				xine_play(inst->Stream,0,inst->PlayPos);
			}
			xine_set_param(inst->Stream,XINE_PARAM_SPEED,param);
		}
	}
	else if (strcmp(name,"audio_volume")==0) {
		inst->AudioVolume=atoi(value);
		xine_set_param(inst->Stream,XINE_PARAM_AUDIO_AMP_LEVEL,inst->AudioVolume);
	}
	else if (strcmp(name,"audio_mute")==0) {
		inst->AudioMute=(strcmp(value,"on")==0 ? 1 : 0);
		xine_set_param(inst->Stream,XINE_PARAM_AUDIO_AMP_MUTE,inst->AudioMute);
	}
	else if (strcmp(name,"audio_visu")==0) {
		for (i=0; inst->AudioVisus[i]; i++) {
			if (strcmp(inst->AudioVisus[i],value)==0) {
				inst->AudioVisu=i;
				emAvSetAudioVisu(instIndex,i);
				break;
			}
		}
	}
	else if (strcmp(name,"audio_channel")==0) {
		for (i=0; inst->AudioChannels[i]; i++) {
			if (strcmp(inst->AudioChannels[i],value)==0) {
				inst->AudioChannel=i-2;
				xine_set_param(
					inst->Stream,XINE_PARAM_AUDIO_CHANNEL_LOGICAL,
					inst->AudioChannel
				);
				break;
			}
		}
	}
	else if (strcmp(name,"spu_channel")==0) {
		for (i=0; inst->SpuChannels[i]; i++) {
			if (strcmp(inst->SpuChannels[i],value)==0) {
				inst->SpuChannel=i-2;
				xine_set_param(
					inst->Stream,XINE_PARAM_SPU_CHANNEL,
					inst->SpuChannel
				);
				break;
			}
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

	inst=emAvInstances[instIndex];
	if (!inst) return;

	if (!inst->MyVoDrv) return;
	pthread_mutex_lock(&inst->MyVoDrv->mutex);
	if (inst->MyVoDrv->shm_ptr) {
		shmdt((const void*)inst->MyVoDrv->shm_ptr);
		inst->MyVoDrv->shm_ptr=NULL;
		inst->MyVoDrv->shm_size=0;
	}
	pthread_mutex_unlock(&inst->MyVoDrv->mutex);
}


static const char * emAvAttachShm(int instIndex, int shmId, int shmSize)
{
	emAvInstance * inst;
	void * shmPtr;

	inst=emAvInstances[instIndex];
	if (!inst) return "Not an opened instance.";

	if (!inst->MyVoDrv) return "No suitable video driver for shm.";

	if (shmId<0 || shmSize<=0) return "Illegal shm parameters.";

	shmPtr=shmat(shmId,NULL,0);
	if (shmPtr==(void*)-1) return "Failed to attach shm.";

	pthread_mutex_lock(&inst->MyVoDrv->mutex);
	if (inst->MyVoDrv->shm_ptr) shmdt((const void*)inst->MyVoDrv->shm_ptr);
	inst->MyVoDrv->shm_ptr=shmPtr;
	inst->MyVoDrv->shm_size=shmSize;
	pthread_mutex_unlock(&inst->MyVoDrv->mutex);

	return NULL;
}


static void emAvCloseInstance(int instIndex)
{
	emAvInstance * inst;

	inst=emAvInstances[instIndex];
	if (!inst) return;

	if (inst->Stream) xine_stop(inst->Stream);

	emAvDetachShm(instIndex);

	emAvSetAudioVisu(instIndex,0);

	if (inst->Stream) {
		xine_close(inst->Stream);
		xine_dispose(inst->Stream);
	}

	if (inst->AudioPort) xine_close_audio_driver(inst->Xine,inst->AudioPort);

	if (inst->VideoPort) xine_close_video_driver(inst->Xine,inst->VideoPort);
	else if (inst->MyVoDrv) inst->MyVoDrv->base.dispose(&inst->MyVoDrv->base);

#ifdef HAVE_ONE_XINE_PER_STREAM
	if (inst->Xine) xine_exit(inst->Xine);
#endif

	emAvCpDynStrArr(&inst->AudioVisus,NULL);
	emAvCpDynStrArr(&inst->AudioChannels,NULL);
	emAvCpDynStrArr(&inst->SpuChannels,NULL);

	free(inst);
	emAvInstances[instIndex]=NULL;
	emAvInstanceCount--;

#ifndef HAVE_ONE_XINE_PER_STREAM
	if (emAvInstanceCount<=0 && TheXine) {
		xine_exit(TheXine);
		TheXine=NULL;
	}
#endif
}


static const char * emAvOpenInstance(
	int instIndex, const char * audioDrv, const char * videoDrv, const char * file
)
{
	static char errBuf[512];
	char lang[XINE_LANG_MAX];
	char chName[64+XINE_LANG_MAX];
	const char * const * avplugins;
	emAvInstance * inst;
	int i,n;

	if (emAvInstances[instIndex]) return "Instance already open.";

	if (!xine_check_version(XINE_MAJOR_VERSION,XINE_MINOR_VERSION,XINE_SUB_VERSION)) {
		return "xine_check_version failed - please recompile emAvServerProc_xine.";
	}

	inst=(emAvInstance*)calloc(1,sizeof(emAvInstance));
	emAvInstances[instIndex]=inst;
	emAvInstanceCount++;

#ifndef HAVE_ONE_XINE_PER_STREAM
	inst->Xine=TheXine;
	if (!inst->Xine) {
#endif
		inst->Xine=xine_new();
		if (!inst->Xine) {
			emAvCloseInstance(instIndex);
			return "xine_new failed.";
		}
		xine_init(inst->Xine);
		xine_engine_set_param(inst->Xine,XINE_ENGINE_PARAM_VERBOSITY,XINE_VERBOSITY_NONE);
#ifndef HAVE_ONE_XINE_PER_STREAM
		TheXine=inst->Xine;
	}
#endif

	if (strcmp(audioDrv,"auto")==0) audioDrv=NULL;
#if defined(__linux__)
	if (!audioDrv) {
		audioDrv="alsa";
		inst->AudioPort=xine_open_audio_driver(inst->Xine,audioDrv,NULL);
		if (!inst->AudioPort) {
			audioDrv=NULL;
			inst->AudioPort=xine_open_audio_driver(inst->Xine,audioDrv,NULL);
		}
	}
	else {
		inst->AudioPort=xine_open_audio_driver(inst->Xine,audioDrv,NULL);
	}
#else
	inst->AudioPort=xine_open_audio_driver(inst->Xine,audioDrv,NULL);
#endif
	if (!inst->AudioPort) {
		inst->Warnings|=EM_AV_WARNING_NO_AUDIO_DRIVER;
		inst->AudioPort=xine_open_audio_driver(inst->Xine,"none",NULL);
		if (!inst->AudioPort) {
			emAvCloseInstance(instIndex);
			return "Failed to prepare audio driver \"none\".";
		}
	}

	if (strcmp(videoDrv,"emAv")==0) {
		inst->MyVoDrv=emAv_vo_driver_create(inst->Xine);
		inst->VideoPort=_x_vo_new_port(inst->Xine,&inst->MyVoDrv->base,0);
		if (!inst->VideoPort) {
			emAvCloseInstance(instIndex);
			return "Failed to prepare video driver.";
		}
	}
	else {
		inst->VideoPort=xine_open_video_driver(
			inst->Xine,videoDrv,XINE_VISUAL_TYPE_NONE,NULL
		);
		if (!inst->VideoPort) {
			emAvCloseInstance(instIndex);
			return "Failed to prepare video driver.";
		}
	}

	inst->Stream=xine_stream_new(inst->Xine,inst->AudioPort,inst->VideoPort);
	if (!inst->Stream) {
		sprintf(
			errBuf,
			"xine_stream_new failed: %s",
			emAv_get_xine_error(inst->Stream)
		);
		emAvCloseInstance(instIndex);
		return errBuf;
	}

	xine_set_param(inst->Stream,XINE_PARAM_VERBOSITY,XINE_VERBOSITY_NONE);

	if (!xine_open(inst->Stream,file)) {
		sprintf(
			errBuf,
			"xine_open failed: %s",
			emAv_get_xine_error(inst->Stream)
		);
		emAvCloseInstance(instIndex);
		return errBuf;
	}

	avplugins=NULL;
	if (
		!xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_HAS_VIDEO) &&
		xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_HAS_AUDIO)
	) {
		avplugins=xine_list_post_plugins_typed(
			inst->Xine,XINE_POST_TYPE_AUDIO_VISUALIZATION
		);
	}
	for (n=0; avplugins && avplugins[n]; n++);
	inst->AudioVisus=(char**)calloc(n+2,sizeof(char*));
	emAvCpDynStr(inst->AudioVisus+0,"none");
	for (i=0; i<n; i++) emAvCpDynStr(inst->AudioVisus+1+i,avplugins[i]);
	for (i=n; i>1 && strcmp(inst->AudioVisus[i],"goom")!=0; i--);
	emAvSetAudioVisu(instIndex,i);

	n=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);
	if (n>1024) n=1024; else if (n<0) n=0;
	emAvCpDynStrArr(&inst->AudioChannels,NULL);
	inst->AudioChannels=(char**)calloc(n+3,sizeof(char*));
	emAvCpDynStr(inst->AudioChannels+0,"none");
	if (n>0) {
		emAvCpDynStr(inst->AudioChannels+1,"auto");
		for (i=0; i<n; i++) {
			sprintf(chName,"%d",i);
			if (xine_get_audio_lang(inst->Stream,i,lang)) {
				sprintf(chName+strlen(chName)," - %s",lang);
			}
			emAvCpDynStr(inst->AudioChannels+2+i,chName);
		}
	}

	n=xine_get_stream_info(inst->Stream,XINE_STREAM_INFO_MAX_SPU_CHANNEL);
	if (n>1024) n=1024; else if (n<0) n=0;
	emAvCpDynStrArr(&inst->SpuChannels,NULL);
	inst->SpuChannels=(char**)calloc(n+3,sizeof(char*));
	emAvCpDynStr(inst->SpuChannels+0,"none");
	if (n>0) {
		emAvCpDynStr(inst->SpuChannels+1,"auto");
		for (i=0; i<n; i++) {
			sprintf(chName,"%d",i);
			if (xine_get_spu_lang(inst->Stream,i,lang)) {
				sprintf(chName+strlen(chName)," - %s",lang);
			}
			emAvCpDynStr(inst->SpuChannels+2+i,chName);
		}
	}

	emAvPollProperties(instIndex,1);

	return NULL;
}


static void emAvHandleMsg(int instIndex, const char * tag, const char * data)
{
	const char * err, * p1, * p2, * p3;
	int shmId,shmSize;

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
		shmId=-1;
		shmSize=0;
		sscanf(data,"%d:%d",&shmId,&shmSize);
		err=emAvAttachShm(instIndex,shmId,shmSize);
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

int main(int argc, char * argv[])
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
	}
	for (i=0; i<EM_AV_MAX_INSTANCES; i++) {
		if (emAvInstances[i]) emAvCloseInstance(i);
	}
	return 0;
}
