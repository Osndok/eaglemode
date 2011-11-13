/*------------------------------------------------------------------------------
// emPdfServerProc.c
//
// Copyright (C) 2011 Oliver Hamann.
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
#include <gtk/gtk.h>
#include <poppler.h>


typedef struct {
	PopplerDocument * doc;
	int pageCount;
} emPdfInst;


static emPdfInst * * emPdfInstArray=NULL;
static int emPdfInstArraySize=0;


static void emPdfPrintQuoted(const char * str)
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


static void emPdfOpen(const char * args)
{
	const char * filePath;
	const char * label;
	char genericLabel[256];
	char uri[64+PATH_MAX];
	GError * err;
	emPdfInst * inst;
	PopplerPage * page;
	double width,height;
	int i,instId;

	filePath=args;
	if (!*filePath) {
		fprintf(stderr,"emPdfServerProc: emPdfOpen: illegal arguments.\n");
		exit(1);
	}

	strcpy(uri,"file:");
	if (!realpath(filePath,uri+strlen(uri))) {
		printf(
			"error: Failed to read %s (%s)\n",
			filePath,
			strerror(errno)
		);
		return;
	}

	inst=(emPdfInst*)malloc(sizeof(emPdfInst));
	memset(inst,0,sizeof(emPdfInst));

	err=NULL;
	inst->doc=poppler_document_new_from_file(uri,NULL,&err);
	if (!inst->doc) {
		printf(
			"error: Failed to read %s (%s)\n",
			filePath,
			(err && err->message && err->message[0]) ? err->message : "unknown error"
		);
		if (err) g_error_free(err);
		free(inst);
		return;
	}

	for (instId=0; ; instId++) {
		if (instId==emPdfInstArraySize) {
			emPdfInstArraySize+=256;
			emPdfInstArray=(emPdfInst**)realloc(
				emPdfInstArray,
				emPdfInstArraySize*sizeof(emPdfInst*)
			);
			memset(
				emPdfInstArray+instId,
				0,
				(emPdfInstArraySize-instId)*sizeof(emPdfInst*)
			);
		}
		if (!emPdfInstArray[instId]) break;
	}
	emPdfInstArray[instId]=inst;

	printf("instance: %d\n",instId);

#if 0
	/* ???: Not in old version. */
	printf("title: %s\n",poppler_document_get_title(inst->doc));
	printf("author: %s\n",poppler_document_get_author(inst->doc));
	printf("subject: %s\n",poppler_document_get_subject(inst->doc));
	printf("keyword: %s\n",poppler_document_get_keyword(inst->doc));
	printf("creator: %s\n",poppler_document_get_creator(inst->doc));
	printf("producer: %s\n",poppler_document_get_produer(inst->doc));
	time_t t1=poppler_document_get_creation_date(inst->doc);
	time_t t2=poppler_document_get_modification_date(inst->doc);
	gboolean lin=poppler_document_is_linearized(inst->doc);
	PopplerPageLayout layout=poppler_document_get_page_layout(inst->doc);
	PopplerPageMode pageMode=poppler_document_get_page_mode(inst->doc);
	PopplerPermissions perm=poppler_document_get_permissions(inst->doc);
	gchar * metadata=poppler_document_get_metadata(inst->doc);
#endif

	inst->pageCount=poppler_document_get_n_pages(inst->doc);

	printf("pages: %d\n",inst->pageCount);

	for (i=0; i<inst->pageCount; i++) {
		sprintf(genericLabel,"%d",i+1);
		page = poppler_document_get_page(inst->doc,i);
		if (page) {
			poppler_page_get_size(page,&width,&height);
#if 0
			/* ???: Not in old version. */
			label=poppler_page_get_label(page);
#else
			label=NULL;
#endif
			if (!label || !*label) label=genericLabel;
			g_object_unref(page);
		}
		else {
			width=1000;
			height=1000;
			label=genericLabel;
		}
		printf("pageinfo: %d %.16lg %.16lg ",i,width,height);
		emPdfPrintQuoted(label);
		putchar('\n');
	}
	printf("ok\n");
}


static void emPdfClose(const char * args)
{
	emPdfInst * inst;
	int instId;

	if (
		sscanf(args,"%d",&instId)!=1 ||
		instId<0 || instId>=emPdfInstArraySize
	) {
		fprintf(stderr,"emPdfServerProc: emPdfClose: illegal arguments.\n");
		exit(1);
	}

	inst=emPdfInstArray[instId];
	if (inst) {
		emPdfInstArray[instId]=NULL;
		g_object_unref(inst->doc);
		free(inst);
	}
}


static void emPdfRender(const char * args)
{
	unsigned char * buf, * ps, * pt, * pe;
	PopplerPage * page;
	cairo_surface_t * surface;
	cairo_t * cr;
	emPdfInst * inst;
	FILE * f;
	double srcX, srcY, srcW, srcH;
	int instId, pageIndex, outW, outH;
	unsigned int v;

	if (
		sscanf(
			args,"%d %d %lg %lg %lg %lg %d %d",
			&instId,&pageIndex,&srcX,&srcY,&srcW,&srcH,&outW,&outH
		)!=8 ||
		instId<0 || instId>=emPdfInstArraySize ||
		!emPdfInstArray[instId] ||
		pageIndex<0 || pageIndex>=emPdfInstArray[instId]->pageCount ||
		srcW<=0.0 || srcH<=0.0 ||
		outW<=0 || outH<=0
	) {
		printf("error: emPdfRender: illegal arguments.\n");
		return;
	}

	inst=emPdfInstArray[instId];

	buf=(unsigned char*)malloc(outW*outH*4);
	memset(buf,0xff,outW*outH*4);

	surface=cairo_image_surface_create_for_data(
		buf,
		CAIRO_FORMAT_RGB24,
		outW,
		outH,
		outW*4
	);
	if (cairo_surface_status(surface)!=CAIRO_STATUS_SUCCESS) {
		printf(
			"error: PDF rendering failed (bad surface status: %d).\n",
			(int)cairo_surface_status(surface)
		);
		free(buf);
		return;
	}

	cr=cairo_create(surface);
	if (cairo_status(cr)!=CAIRO_STATUS_SUCCESS) {
		printf(
			"error: PDF rendering failed (bad context status: %d).\n",
			(int)cairo_status(cr)
		);
		cairo_surface_destroy(surface);
		free(buf);
		return;
	}

	cairo_scale(cr,outW/srcW,outH/srcH);
	cairo_translate(cr,-srcX,-srcY);

	page = poppler_document_get_page(inst->doc,pageIndex);
	if (page) {
		poppler_page_render(page,cr);
		g_object_unref(page);
	}
	else {
		memset(buf,0x88,outW*outH*4);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

#if defined(EM_PDF_DEBUG_RENDER_TO_FILE)
	f=fopen("/tmp/emPdfTest.ppm","wb");
#else
	f=stdout;
#endif
	fprintf(f,"P6\n%d\n%d\n255\n",outW,outH);
	ps=buf;
	pt=buf;
	pe=buf+outW*outH*4;
	while (ps<pe) {
		v=*(unsigned int*)ps;
		pt[0]=(unsigned char)(v>>16);
		pt[1]=(unsigned char)(v>>8);
		pt[2]=(unsigned char)v;
		ps+=4;
		pt+=3;
	}
	fwrite(buf,outW*outH,3,f);
	if (f!=stdout) fclose(f);

	free(buf);
}


int main(int argc, char * argv[])
{
	char buf[1024];
	char * args;
	int len;

	gtk_init_check(&argc,&argv);
	setlocale(LC_NUMERIC,"C");

	while (fgets(buf,sizeof(buf),stdin)) {
		len=strlen(buf);
		if (len>0 && buf[len-1]=='\n') buf[--len]=0;
		args=strchr(buf,' ');
		if (args) *args++=0;
		else args=buf+len;
		if      (strcmp(buf,"open")==0) emPdfOpen(args);
		else if (strcmp(buf,"close")==0) emPdfClose(args);
		else if (strcmp(buf,"render")==0) emPdfRender(args);
		else {
			fprintf(stderr,"emPdfServerProc: illegal command.\n");
			exit(1);
		}
		fflush(stdout);
	}

	return 0;
}
