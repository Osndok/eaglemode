/*------------------------------------------------------------------------------
// emPdfServerProc.c
//
// Copyright (C) 2011-2013,2017-2019,2022-2024 Oliver Hamann.
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
#ifdef _WIN32
#	include <fcntl.h>
#	include <io.h>
#	include <windows.h>
#endif


typedef struct {
	PopplerDocument * doc;
	int pageCount;
} emPdfInst;


static emPdfInst * * emPdfInstArray=NULL;
static int emPdfInstArraySize=0;


static void emPdfPrintQuoted(const char * str, int maxLen)
{
	int i,c;

	putchar('"');
	if (str) for (i=0; i<maxLen; i++) {
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


static void emPdfPrintQuotedFromUtf8(const char * str, int maxLen)
{
	char * normalized, * converted;
	const gchar * charset;
	int len;

	normalized=NULL;
	converted=NULL;

	if (str) {
		len=strlen(str);
		normalized=g_utf8_normalize(
			str,
			maxLen<len?maxLen:len,
			G_NORMALIZE_NFC
		);
		if (normalized) str=normalized;
		if (!g_get_charset(&charset)) {
			len=strlen(str);
			converted=g_convert_with_fallback(
				str,
				maxLen<len?maxLen:len,
				charset,
				"UTF-8",
				"?",
				NULL,
				NULL,
				NULL
			);
			if (converted) str=converted;
		}
	}

	emPdfPrintQuoted(str,maxLen);

	if (converted) g_free(converted);
	if (normalized) g_free(normalized);
}


static char * emPdfEncodeFileUri(const char * absFilePath)
{
	char * uri, * p;
	int c;

	uri=malloc(16+3*strlen(absFilePath));

	p=uri;
	strcpy(p,"file://");
	p+=strlen(p);
#ifdef _WIN32
	*p++='/';
#endif
	while (*absFilePath) {
		c=(unsigned char)*absFilePath++;
		if (
			c<32 ||
			c==' ' ||
			c=='#' ||
			c=='$' ||
			c=='%' ||
			c=='&' ||
			c=='+' ||
			c=='#' ||
			c==',' ||
			c==';' ||
			c=='=' ||
			c=='?' ||
			c=='@' ||
			c==127
		) {
			sprintf(p,"%%%02X",c);
			p+=3;
		}
#ifdef _WIN32
		else if (c=='\\') {
			*p++='/';
		}
#endif
		else {
			*p++=(char)c;
		}
	}
	*p=0;

	return uri;
}


static void emPdfOpen(const char * args)
{
	const char * filePath;
	const char * label;
	gchar * str;
	char genericLabel[256];
	char * uri;
	GError * err;
	emPdfInst * inst;
	PopplerPage * page;
	double width,height;
	time_t t;
	int i,instId;

	filePath=args;
	if (!*filePath) {
		fprintf(stderr,"emPdfServerProc: emPdfOpen: illegal arguments.\n");
		exit(1);
	}

#ifndef _WIN32
	char realFilePath[PATH_MAX];
	if (!realpath(filePath,realFilePath)) {
		printf(
			"error: Failed to read %s (%s)\n",
			filePath,
			strerror(errno)
		);
		return;
	}
	filePath=realFilePath;
#endif

	err=NULL;

#ifdef _WIN32
	/* On Windows, glib expects file names in UTF-8 instead of current locale. */
	gchar * filePathConverted=g_locale_to_utf8(
		filePath,strlen(filePath),NULL,NULL,&err
	);
	if (!filePathConverted) {
		printf(
			"error: Failed to convert file path %s (%s)\n",
			filePath,
			(err && err->message && err->message[0]) ? err->message : "unknown error"
		);
		if (err) g_error_free(err);
		return;
	}
	uri=emPdfEncodeFileUri(filePathConverted);
	g_free(filePathConverted);
#else
	uri=emPdfEncodeFileUri(filePath);
#endif

	inst=(emPdfInst*)malloc(sizeof(emPdfInst));
	memset(inst,0,sizeof(emPdfInst));

	inst->doc=poppler_document_new_from_file(uri,NULL,&err);
	free(uri);
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

	str=poppler_document_get_title(inst->doc);
	if (str) {
		printf("title: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	str=poppler_document_get_author(inst->doc);
	if (str) {
		printf("author: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	str=poppler_document_get_subject(inst->doc);
	if (str) {
		printf("subject: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	str=poppler_document_get_keywords(inst->doc);
	if (str) {
		printf("keywords: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	str=poppler_document_get_creator(inst->doc);
	if (str) {
		printf("creator: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	str=poppler_document_get_producer(inst->doc);
	if (str) {
		printf("producer: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	t=poppler_document_get_creation_date(inst->doc);
	if (t!=-1) {
		printf("creation_date: %ld\n",(long)t);
	}

	t=poppler_document_get_modification_date(inst->doc);
	if (t!=-1) {
		printf("modification_date: %ld\n",(long)t);
	}

	str=poppler_document_get_pdf_version_string(inst->doc);
	if (str) {
		printf("version: ");
		emPdfPrintQuotedFromUtf8(str,8192);
		putchar('\n');
		g_free(str);
	}

	inst->pageCount=poppler_document_get_n_pages(inst->doc);

	printf("pages: %d\n",inst->pageCount);

	for (i=0; i<inst->pageCount; i++) {
		width=1000;
		height=1000;
		sprintf(genericLabel,"%d",i+1);
		label=genericLabel;
		str=NULL;
		page = poppler_document_get_page(inst->doc,i);
		if (page) {
			poppler_page_get_size(page,&width,&height);
			str=poppler_page_get_label(page);
			if (str && *str) label=str;
			g_object_unref(page);
		}
		printf("pageinfo: %d %.16g %.16g ",i,width,height);
		emPdfPrintQuotedFromUtf8(label,256);
		if (str) g_free(str);
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


static void emPdfGetAreas(const char * args)
{
	PopplerRectangle selection;
	cairo_region_t * region;
	cairo_rectangle_int_t rect;
	GList * list, * node;
	PopplerLinkMapping * mapping;
	PopplerDest * dest, * foundDest;
	emPdfInst * inst;
	PopplerPage * page;
	double width,height;
	int i,n,instId,pageIndex,targetPage,targetY;

	if (
		sscanf(args,"%d %d",&instId,&pageIndex)!=2 ||
		instId<0 || instId>=emPdfInstArraySize ||
		!emPdfInstArray[instId] ||
		pageIndex<0 || pageIndex>=emPdfInstArray[instId]->pageCount
	) {
		printf("error: emPdfGetAreas: illegal arguments.\n");
		return;
	}

	inst=emPdfInstArray[instId];
	page=poppler_document_get_page(inst->doc,pageIndex);
	if (!page) {
		printf("ok\n");
		return;
	}

	poppler_page_get_size(page,&width,&height);
	selection.x1=0.0;
	selection.y1=0.0;
	selection.x2=width;
	selection.y2=height;
	region=poppler_page_get_selected_region(
		page,1.0,POPPLER_SELECTION_GLYPH,&selection
	);
	if (region) {
		n=cairo_region_num_rectangles(region);
		for (i=0; i<n; i++) {
			cairo_region_get_rectangle(region,i,&rect);
			printf(
				"rect: %d %d %d %d 0\n",
				rect.x,rect.y,
				rect.x+rect.width,rect.y+rect.height
			);
		}
		cairo_region_destroy(region);
	}

	list=poppler_page_get_link_mapping(page);
	if (list) {
		for (node=list; node; node=node->next) {
			mapping=(PopplerLinkMapping *)node->data;
			switch (mapping->action->type) {
				case POPPLER_ACTION_GOTO_DEST:
					dest=mapping->action->goto_dest.dest;
					foundDest=NULL;
					targetPage=-1;
					targetY=0;
					for (;;) {
						switch (dest->type) {
							case POPPLER_DEST_XYZ:
							case POPPLER_DEST_FITH:
							case POPPLER_DEST_FITR:
							case POPPLER_DEST_FITBH:
								targetPage=dest->page_num-1;
								targetY=(int)(height-dest->top+0.5);
								break;
							case POPPLER_DEST_FIT:
							case POPPLER_DEST_FITV:
							case POPPLER_DEST_FITB:
							case POPPLER_DEST_FITBV:
								targetPage=dest->page_num-1;
								break;
							case POPPLER_DEST_NAMED:
								if (foundDest) break;
								foundDest=poppler_document_find_dest(
									inst->doc,dest->named_dest
								);
								if (foundDest) {
									dest=foundDest;
									continue;
								}
								break;
							default:
								break;
						}
						break;
					}
					if (targetPage>=0 && targetPage<inst->pageCount) {
						printf(
							"rect: %d %d %d %d 2 %d %d\n",
							(int)(mapping->area.x1+0.5),
							(int)(height-mapping->area.y2+0.5),
							(int)(mapping->area.x2+0.5),
							(int)(height-mapping->area.y1+0.5),
							targetPage,targetY
						);
					}
					if (foundDest) {
						poppler_dest_free(foundDest);
					}
					break;
				case POPPLER_ACTION_URI:
					if(mapping->action->uri.uri) {
						printf(
							"rect: %d %d %d %d 1 ",
							(int)(mapping->area.x1+0.5),
							(int)(height-mapping->area.y2+0.5),
							(int)(mapping->area.x2+0.5),
							(int)(height-mapping->area.y1+0.5)
						);
						emPdfPrintQuotedFromUtf8(mapping->action->uri.uri,1024);
						putchar('\n');
					}
					break;
				default:
					break;
			}
		}
		poppler_page_free_link_mapping(list);
	}

	g_object_unref(page);

	printf("ok\n");
}


static void emPdfGetSelectedText(const char * args)
{
	static const PopplerSelectionStyle styles[]={
		POPPLER_SELECTION_GLYPH,
		POPPLER_SELECTION_WORD,
		POPPLER_SELECTION_LINE
	};
	PopplerRectangle selection;
	char * text;
	PopplerPage * page;
	double selX1,selY1,selX2,selY2;
	int instId,pageIndex,style;

	if (
		sscanf(
			args,"%d %d %d %lg %lg %lg %lg",
			&instId,&pageIndex,&style,&selX1,&selY1,&selX2,&selY2
		)!=7 ||
		instId<0 || instId>=emPdfInstArraySize ||
		!emPdfInstArray[instId] ||
		pageIndex<0 || pageIndex>=emPdfInstArray[instId]->pageCount ||
		style<0 || style>2
	) {
		printf("error: emPdfGetSelectedText: illegal arguments.\n");
		return;
	}

	text=NULL;
	page=poppler_document_get_page(emPdfInstArray[instId]->doc,pageIndex);
	if (page) {
		selection.x1=selX1;
		selection.y1=selY1;
		selection.x2=selX2;
		selection.y2=selY2;
		text=poppler_page_get_selected_text(page,styles[style],&selection);
	}

	printf("selected_text: ");
	emPdfPrintQuotedFromUtf8(text?text:"",INT_MAX);
	putchar('\n');

	if (text) g_free(text);
	if (page) g_object_unref(page);
}


static void emPdfRender(const char * args, int renderSelection)
{
	static const PopplerSelectionStyle styles[]={
		POPPLER_SELECTION_GLYPH,
		POPPLER_SELECTION_WORD,
		POPPLER_SELECTION_LINE
	};
	PopplerRectangle selection;
	PopplerColor fgColor,bgColor;
	unsigned char * buf, * ps, * pt, * pe;
	PopplerPage * page;
	cairo_surface_t * surface;
	cairo_t * cr;
	emPdfInst * inst;
	FILE * f;
	double srcX, srcY, srcW, srcH, selX1, selY1, selX2, selY2;
	int instId, pageIndex, outW, outH, style;
	unsigned int v;

	if (renderSelection) {
		if (
			sscanf(
				args,"%d %d %lg %lg %lg %lg %d %d %d %lg %lg %lg %lg",
				&instId,&pageIndex,&srcX,&srcY,&srcW,&srcH,&outW,&outH,
				&style,&selX1,&selY1,&selX2,&selY2
			)!=13
		) {
			printf("error: emPdfRender: illegal arguments.\n");
			return;
		}
	}
	else {
		if (
			sscanf(
				args,"%d %d %lg %lg %lg %lg %d %d",
				&instId,&pageIndex,&srcX,&srcY,&srcW,&srcH,&outW,&outH
			)!=8
		) {
			printf("error: emPdfRender: illegal arguments.\n");
			return;
		}
		style=0;
		selX1=selY1=selX2=selY2=0.0;
	}

	if (
		instId<0 || instId>=emPdfInstArraySize ||
		!emPdfInstArray[instId] ||
		pageIndex<0 || pageIndex>=emPdfInstArray[instId]->pageCount ||
		srcW<=0.0 || srcH<=0.0 ||
		outW<=0 || outH<=0 ||
		style<0 || style>2
	) {
		printf("error: emPdfRender: illegal arguments.\n");
		return;
	}

	inst=emPdfInstArray[instId];

	buf=(unsigned char*)malloc(outW*outH*4);
	memset(buf,renderSelection?0x00:0xff,outW*outH*4);

	surface=cairo_image_surface_create_for_data(
		buf,
		renderSelection ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
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
		if (renderSelection) {
			selection.x1=selX1;
			selection.y1=selY1;
			selection.x2=selX2;
			selection.y2=selY2;
			fgColor.red  =0xFFFF;
			fgColor.green=0xFFFF;
			fgColor.blue =0xFFFF;
			bgColor.red  =0x0000;
			bgColor.green=0x0000;
			bgColor.blue =0x0000;
			poppler_page_render_selection(
				page,cr,&selection,NULL,styles[style],&fgColor,&bgColor
			);
		}
		else {
			poppler_page_render(page,cr);
		}
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
	ps=buf;
	pt=buf;
	pe=buf+outW*outH*4;
	if (renderSelection) {
		fprintf(f,"PX\n%d\n%d\n255\n",outW,outH);
		while (ps<pe) {
			v=*(unsigned int*)ps;
			pt[0]=(unsigned char)v;
			pt[1]=(unsigned char)(v>>24);
			ps+=4;
			pt+=2;
		}
	}
	else {
		fprintf(f,"P6\n%d\n%d\n255\n",outW,outH);
		while (ps<pe) {
			v=*(unsigned int*)ps;
			pt[0]=(unsigned char)(v>>16);
			pt[1]=(unsigned char)(v>>8);
			pt[2]=(unsigned char)v;
			ps+=4;
			pt+=3;
		}
	}
	fwrite(buf,pt-buf,1,f);
	if (f!=stdout) fclose(f);

	free(buf);
}


static int emPdfServe(int argc, char * argv[])
{
	char * buf,* args;
	int bufSize,len;

	gtk_init_check(&argc,&argv);
	setlocale(LC_NUMERIC,"C");

	bufSize=16384;
	buf=malloc(bufSize);

	while (fgets(buf,bufSize,stdin)) {
		len=strlen(buf);
		while (len>0 && (unsigned char)buf[len-1]<32) buf[--len]=0;
		args=strchr(buf,' ');
		if (args) *args++=0;
		else args=buf+len;
		if      (strcmp(buf,"open")==0) emPdfOpen(args);
		else if (strcmp(buf,"get_areas")==0) emPdfGetAreas(args);
		else if (strcmp(buf,"get_selected_text")==0) emPdfGetSelectedText(args);
		else if (strcmp(buf,"render")==0) emPdfRender(args, 0);
		else if (strcmp(buf,"render_selection")==0) emPdfRender(args, 1);
		else if (strcmp(buf,"close")==0) emPdfClose(args);
		else {
			fprintf(stderr,"emPdfServerProc: illegal command.\n");
			exit(1);
		}
		fflush(stdout);
	}

	free(buf);

	return 0;
}


#ifdef _WIN32
static DWORD WINAPI emPdfServeThreadProc(LPVOID data)
{
	return emPdfServe(__argc,__argv);
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

	hdl=CreateThread(NULL,0,emPdfServeThreadProc,NULL,0,&d);
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
	return emPdfServe(argc,argv);
#endif
}
