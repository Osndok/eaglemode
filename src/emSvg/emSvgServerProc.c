/*------------------------------------------------------------------------------
// emSvgServerProc.c
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
//----------------------------------------------------------------------------*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/shm.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>


typedef struct {
	RsvgHandle * handle;
	RsvgDimensionData dimData;
} emSvgInst;


static emSvgInst * * emSvgInstArray=NULL;
static int emSvgInstArraySize=0;


static void * emSvgShmPtr=NULL;


static void emSvgAttachShm(const char * args)
{
	int shmId;

	if (sscanf(args,"%d",&shmId)!=1) {
		fprintf(stderr,"emSvgServerProc: emSvgAttachShm: illegal arguments.\n");
		exit(1);
	}

	if (emSvgShmPtr) shmdt(emSvgShmPtr);

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
	const char * filePath;
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
	inst->handle=rsvg_handle_new_from_file(filePath,&err);
	if (!inst->handle) {
		printf(
			"error: Failed to read %s (%s)\n",
			filePath,
			(err && err->message && err->message[0]) ? err->message : "unknown error"
		);
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

	printf(
		"opened: %d %d %d ",
		instId,
		inst->dimData.width,
		inst->dimData.height
	);
	emSvgPrintQuoted(rsvg_handle_get_title(inst->handle));
	putchar(' ');
	emSvgPrintQuoted(rsvg_handle_get_desc(inst->handle));
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


int main(int argc, char * argv[])
{
	char buf[1024];
	char * args;
	int len;

	rsvg_init();
	setlocale(LC_NUMERIC,"C");

	while (fgets(buf,sizeof(buf),stdin)) {
		len=strlen(buf);
		if (len>0 && buf[len-1]=='\n') buf[--len]=0;
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

	rsvg_term();

	return 0;
}
