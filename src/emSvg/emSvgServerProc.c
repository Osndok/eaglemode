/*------------------------------------------------------------------------------
// emSvgServerProc.c
//
// Copyright (C) 2010-2011,2017-2020 Oliver Hamann.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <librsvg/rsvg.h>
#if defined(LIBRSVG_CHECK_VERSION)
#	if !LIBRSVG_CHECK_VERSION(2,36,2)
#		include <librsvg/rsvg-cairo.h>
#	endif
#else
#	include <librsvg/librsvg-features.h>
#	include <librsvg/rsvg-cairo.h>
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
#	if defined(_WIN32)
#		include <fcntl.h>
#		include <io.h>
#	endif
#	include <windows.h>
#else
#	include <sys/shm.h>
#endif


typedef struct {
	RsvgHandle * handle;
	RsvgDimensionData dimData;
} emSvgInst;


static emSvgInst * * emSvgInstArray=NULL;
static int emSvgInstArraySize=0;


#if defined(_WIN32) || defined(__CYGWIN__)
static HANDLE emSvgShmHdl=NULL;
#endif
static void * emSvgShmPtr=NULL;


static void emSvgAttachShm(const char * args)
{
#if defined(_WIN32) || defined(__CYGWIN__)
	if (emSvgShmPtr) {
		UnmapViewOfFile(emSvgShmPtr);
		emSvgShmPtr=NULL;
	}

	if (emSvgShmHdl) {
		CloseHandle(emSvgShmHdl);
		emSvgShmHdl=NULL;
	}

	if (*args) {
		emSvgShmHdl=OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,args);
		if (!emSvgShmHdl) {
			fprintf(
				stderr,
				"emSvgServerProc: Failed to attach shared memory (OpenFileMapping: 0x%lX)\n",
				(long)GetLastError()
			);
			exit(1);
		}
		emSvgShmPtr=MapViewOfFile(emSvgShmHdl,FILE_MAP_ALL_ACCESS,0,0,0);
		if (!emSvgShmPtr) {
			fprintf(
				stderr,
				"emSvgServerProc: Failed to attach shared memory (MapViewOfFile: 0x%lX)\n",
				(long)GetLastError()
			);
			exit(1);
		}
	}
#else
	int shmId;

	if (sscanf(args,"%d",&shmId)!=1) {
		fprintf(stderr,"emSvgServerProc: emSvgAttachShm: illegal arguments.\n");
		exit(1);
	}

	if (emSvgShmPtr) {
		shmdt(emSvgShmPtr);
		emSvgShmPtr=NULL;
	}

	if (shmId!=-1) {
		emSvgShmPtr=shmat(shmId,NULL,0);
		if (emSvgShmPtr==(void*)-1) {
			emSvgShmPtr=NULL;
			fprintf(
				stderr,
				"emSvgServerProc: Failed to attach shared memory segment (%s)\n",
				strerror(errno)
			);
			exit(1);
		}
	}
#endif
}


static void emSvgPrintQuoted(const char * str)
{
	int c;

	putchar('"');
	if (str) for (;;) {
		c=*str++;
		if (!c) break;
		if (c=='"' || c=='\\') printf("\\%c",c);
		else if (c=='\n') printf("\\n");
		else if (c=='\r') printf("\\r");
		else if (c=='\t') printf("\\t");
		else if ((unsigned char)c>=32) putchar(c);
	}
	putchar('"');
}


static void emSvgOpen(const char * args)
{
	const char * filePath, * title, * desc;
	char * msg, * p;
	GError * err;
	emSvgInst * inst;
	int instId;

	filePath=args;
	if (!*filePath) {
		fprintf(stderr,"emSvgServerProc: emSvgOpen: illegal arguments.\n");
		exit(1);
	}

	inst=(emSvgInst*)malloc(sizeof(emSvgInst));
	memset(inst,0,sizeof(emSvgInst));

	err=NULL;
#ifdef _WIN32
	/* On Windows, rsvg_handle_new_from_file(..) failed when having 8-bit
	   characters in file path (because glib expects UTF-8 instead of
	   current locale for file names on Windows). Simply load on our own to
	   get out of that problem: */
	FILE * file=fopen(filePath,"rb");
	if (!file) {
		printf("error: Failed to open %s (%s)\n",filePath,strerror(errno));
		free(inst);
		return;
	}
	fseek(file,0,SEEK_END);
	long fileSize=ftell(file);
	if (fileSize<0) {
		printf("error: Failed to get file size of %s (%s)\n",filePath,strerror(errno));
		fclose(file);
		free(inst);
		return;
	}
	rewind(file);
	guint8 * fileBuf=(guint8 *)malloc(fileSize);
	if (fread(fileBuf,1,fileSize,file)!=(size_t)fileSize) {
		printf("error: Failed to load %s (%s)\n",filePath,strerror(errno));
		free(fileBuf);
		fclose(file);
		free(inst);
		return;
	}
	fclose(file);
	inst->handle=rsvg_handle_new_from_data(fileBuf,fileSize,&err);
	free(fileBuf);
#else
	inst->handle=rsvg_handle_new_from_file(filePath,&err);
#endif
	if (!inst->handle) {
		msg=strdup(
			(err && err->message && err->message[0]) ? err->message : "unknown error"
		);
		for (p=msg;;) {
			if ((p=strchr(p,'\n'))==NULL) break;
			*p=' ';
		}
		printf("error: Failed to read %s (%s)\n",filePath,msg);
		free(msg);
		if (err) g_error_free(err);
		free(inst);
		return;
	}

	rsvg_handle_get_dimensions(inst->handle,&inst->dimData);

	if (
		inst->dimData.width<=0 ||
		inst->dimData.height<=0 ||
		inst->dimData.width<inst->dimData.height/100 ||
		inst->dimData.height<inst->dimData.width/100
	) {
		printf(
			"error: Unsupported SVG image dimensions: %d x %d\n",
			inst->dimData.width,
			inst->dimData.height
		);
		g_object_unref(inst->handle);
		free(inst);
		return;
	}

	for (instId=0; ; instId++) {
		if (instId==emSvgInstArraySize) {
			emSvgInstArraySize+=256;
			emSvgInstArray=(emSvgInst**)realloc(
				emSvgInstArray,
				emSvgInstArraySize*sizeof(emSvgInst*)
			);
			memset(
				emSvgInstArray+instId,
				0,
				(emSvgInstArraySize-instId)*sizeof(emSvgInst*)
			);
		}
		if (!emSvgInstArray[instId]) break;
	}
	emSvgInstArray[instId]=inst;

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
	title = rsvg_handle_get_title(inst->handle);
	desc = rsvg_handle_get_desc(inst->handle);
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
#	pragma GCC diagnostic pop
#endif

	printf(
		"opened: %d %d %d ",
		instId,
		inst->dimData.width,
		inst->dimData.height
	);
	emSvgPrintQuoted(title);
	putchar(' ');
	emSvgPrintQuoted(desc);
	putchar('\n');
}


static void emSvgClose(const char * args)
{
	emSvgInst * inst;
	int instId;

	if (
		sscanf(args,"%d",&instId)!=1 ||
		instId<0 || instId>=emSvgInstArraySize ||
		!emSvgInstArray[instId]
	) {
		fprintf(stderr,"emSvgServerProc: emSvgClose: illegal arguments.\n");
		exit(1);
	}

	inst=emSvgInstArray[instId];
	emSvgInstArray[instId]=NULL;
	g_object_unref(inst->handle);
	free(inst);
}


static void emSvgRender(const char * args)
{
	cairo_surface_t * surface;
	cairo_t * cr;
	emSvgInst * inst;
	double srcX, srcY, srcW, srcH;
	int instId, shmOffset, shmW, shmH;

	if (
		sscanf(
			args,"%d %lg %lg %lg %lg %d %d %d",
			&instId,&srcX,&srcY,&srcW,&srcH,&shmOffset,&shmW,&shmH
		)!=8 ||
		instId<0 || instId>=emSvgInstArraySize ||
		!emSvgInstArray[instId] ||
		srcW<=0.0 || srcH<=0.0 ||
		shmOffset<0 ||
		shmW<=0 || shmH<=0
	) {
		printf("error: emSvgRender: illegal arguments.\n");
		return;
	}

	inst=emSvgInstArray[instId];

#if defined(EM_SVG_DEBUG_BY_WRITING_PNG)
	surface=cairo_image_surface_create(CAIRO_FORMAT_RGB24,shmW,shmH);
#else
	if (!emSvgShmPtr) {
		printf("error: emSvgRender: no shared memory segment attached.\n");
		return;
	}
	surface=cairo_image_surface_create_for_data(
		((unsigned char*)emSvgShmPtr)+shmOffset,
		CAIRO_FORMAT_RGB24,
		shmW,
		shmH,
		shmW*4
	);
#endif
	if (cairo_surface_status(surface)!=CAIRO_STATUS_SUCCESS) {
		printf(
			"error: SVG rendering failed (bad surface status: %d).\n",
			(int)cairo_surface_status(surface)
		);
		return;
	}

	cr=cairo_create(surface);
	if (cairo_status(cr)!=CAIRO_STATUS_SUCCESS) {
		printf(
			"error: SVG rendering failed (bad context status: %d).\n",
			(int)cairo_status(cr)
		);
		cairo_surface_destroy(surface);
		return;
	}

	cairo_scale(cr,shmW/srcW,shmH/srcH);
	cairo_translate(cr,-srcX,-srcY);

	rsvg_handle_render_cairo(inst->handle,cr);

#if defined(EM_SVG_DEBUG_BY_WRITING_PNG)
	cairo_surface_write_to_png(surface,"/tmp/emSvgTest.png");
#endif

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	printf("rendered\n");
}


int emSvgServe(int argc, char * argv[])
{
	char * buf,* args;
	int bufSize,len;

#if defined(LIBRSVG_CHECK_VERSION)
#	if !LIBRSVG_CHECK_VERSION(2,35,0)
	rsvg_init();
#	elif defined(GLIB_CHECK_VERSION)
#		if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#		endif
#	else
	g_type_init();
#	endif
#else
	rsvg_init();
#endif

	setlocale(LC_NUMERIC,"C");

	bufSize=16384;
	buf=malloc(bufSize);

	while (fgets(buf,bufSize,stdin)) {
		len=strlen(buf);
		while (len>0 && (unsigned char)buf[len-1]<32) buf[--len]=0;
		args=strchr(buf,' ');
		if (args) *args++=0;
		else args=buf+len;
		if      (strcmp(buf,"attachshm")==0) emSvgAttachShm(args);
		else if (strcmp(buf,"open")==0) emSvgOpen(args);
		else if (strcmp(buf,"close")==0) emSvgClose(args);
		else if (strcmp(buf,"render")==0) emSvgRender(args);
		else {
			fprintf(stderr,"emSvgServerProc: illegal command.\n");
			exit(1);
		}
		fflush(stdout);
	}

	free(buf);

#if defined(LIBRSVG_CHECK_VERSION)
#	if !LIBRSVG_CHECK_VERSION(2,35,0)
	rsvg_term();
#	endif
#else
	rsvg_term();
#endif

	return 0;
}


#ifdef _WIN32
static DWORD WINAPI emSvgServeThreadProc(LPVOID data)
{
	return emSvgServe(__argc,__argv);
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

	hdl=CreateThread(NULL,0,emSvgServeThreadProc,NULL,0,&d);
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
	return emSvgServe(argc,argv);
#endif
}
