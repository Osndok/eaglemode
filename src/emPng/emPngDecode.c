/*------------------------------------------------------------------------------
// emPngDecode.c
//
// Copyright (C) 2022 Oliver Hamann.
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

#include <emPng/emPngDecode.h>
#include <png.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


struct emPngDecodeInstance {
	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info_ptr;
	png_uint_32 width, height, bytes_per_pixel;
	int bit_depth, color_type, interlace_type;
	int number_of_passes;
	jmp_buf jmpbuffer;
	char errorText[256];
	int y,pass;
};


static void emPngCatStr(char * buf, size_t bufSize, const char * text)
{
	size_t l;

	l=strlen(buf);
	if (l<bufSize-1) {
		strncat(buf+l,text,bufSize-l-1);
	}
}


static void emPng_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	struct emPngDecodeInstance * inst;

	inst=(struct emPngDecodeInstance *)png_get_error_ptr(png_ptr);
	if (!error_msg) error_msg="PNG error";
	emPngCatStr(inst->errorText,sizeof(inst->errorText),error_msg);
	longjmp(inst->jmpbuffer,1);
}


static void emPng_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}


void * emPngStartDecoding(
	FILE * file, int * width, int * height, int * channelCount,
	int * passCount, char * infoBuf, size_t infoBufSize,
	char * errorBuf, size_t errorBufSize
)
{
	struct emPngDecodeInstance * inst;
	int rowbytes,originalPixelSize;

	inst=(struct emPngDecodeInstance*)malloc(sizeof(struct emPngDecodeInstance));
	memset(inst,0,sizeof(struct emPngDecodeInstance));

 	*infoBuf=0;
	*errorBuf=0;

	if (setjmp(inst->jmpbuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->errorText);
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->png_ptr=png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		(png_voidp)inst,
		emPng_error_fn,
		emPng_warning_fn
	);
	if (!inst->png_ptr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->info_ptr=png_create_info_struct(inst->png_ptr);
	if (!inst->info_ptr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->end_info_ptr=png_create_info_struct(inst->png_ptr);
	if (!inst->end_info_ptr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	png_init_io(inst->png_ptr, file);

	png_read_info(inst->png_ptr, inst->info_ptr);

	png_get_IHDR(
		inst->png_ptr,
		inst->info_ptr,
		&inst->width,
		&inst->height,
		&inst->bit_depth,
		&inst->color_type,
		&inst->interlace_type,
		NULL,
		NULL
	);

	originalPixelSize=inst->bit_depth;
	if ((inst->color_type&PNG_COLOR_MASK_PALETTE)==0) {
		originalPixelSize*=png_get_channels(inst->png_ptr,inst->info_ptr);
	}

	png_set_expand(inst->png_ptr);
	png_set_strip_16(inst->png_ptr);
	png_set_packing(inst->png_ptr);
	inst->number_of_passes=png_set_interlace_handling(inst->png_ptr);
	png_read_update_info(inst->png_ptr, inst->info_ptr);
	rowbytes=png_get_rowbytes(inst->png_ptr,inst->info_ptr);
	inst->bytes_per_pixel=rowbytes/inst->width;
	if (
		rowbytes%inst->width!=0 ||
		inst->bytes_per_pixel<1 || inst->bytes_per_pixel>4 ||
		inst->width<1 || inst->width>0x7fffff ||
		inst->height<1 || inst->height>0x7fffff
	) {
		emPngCatStr(errorBuf,errorBufSize,"Unsupported PNG format.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	*width=inst->width,
	*height=inst->height,
	*channelCount=inst->bytes_per_pixel;
	*passCount=inst->number_of_passes;

	snprintf(infoBuf,infoBufSize,"PNG %d-bit ",originalPixelSize);
	infoBuf[infoBufSize-1]=0;
	if ((inst->color_type&PNG_COLOR_MASK_COLOR)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"color");
	}
	else {
		emPngCatStr(infoBuf,infoBufSize,"grayscale");
	}
	if ((inst->color_type&PNG_COLOR_MASK_ALPHA)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"-alpha");
	}
	if ((inst->color_type&PNG_COLOR_MASK_PALETTE)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"-palette");
	}
	snprintf(
		infoBuf+strlen(infoBuf),
		infoBufSize-strlen(infoBuf),
		" (%d channels extracted)",
		(int)inst->bytes_per_pixel
	);
	infoBuf[infoBufSize-1]=0;

	return inst;
}


int emPngContinueDecoding(
	void * instance, unsigned char * rowBuf, char * commentBuf,
	size_t commentBufSize, char * errorBuf, size_t errorBufSize
)
{
	struct emPngDecodeInstance * inst;
	png_textp t;
	int e,i,n;

	inst = (struct emPngDecodeInstance*)instance;

	*commentBuf=0;
	*errorBuf=0;

	if (setjmp(inst->jmpbuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->errorText);
		return -1;
	}

	if (inst->y<(int)inst->height && inst->pass<inst->number_of_passes) {
		png_read_row(
			inst->png_ptr,
			rowBuf,
			NULL
		);
		inst->y++;
		if (inst->y>=(int)inst->height) {
			inst->y=0;
			inst->pass++;
		}
		return 0;
	}

	png_read_end(inst->png_ptr,inst->end_info_ptr);

	for (e=0; e<2; e++) {
		n=png_get_text(
			inst->png_ptr,
			e ? inst->end_info_ptr : inst->info_ptr,
			&t,
			NULL
		);
		for (i=0; i<n; i++) {
			if (
				t[i].text && *t[i].text && t[i].key && (
					strcasecmp(t[i].key,"Comment")==0 ||
					strcasecmp(t[i].key,"Description")==0
				)
			) {
				if (*commentBuf) emPngCatStr(commentBuf,commentBufSize,"\n");
				emPngCatStr(commentBuf,commentBufSize,t[i].text);
			}
		}
	}

	return 1;
}


void emPngQuitDecoding(void * instance)
{
	struct emPngDecodeInstance * inst;

	inst = (struct emPngDecodeInstance*)instance;
	if (inst) {
		if (inst->png_ptr) {
			png_destroy_read_struct(
				&inst->png_ptr,
				inst->info_ptr ? &inst->info_ptr : NULL,
				inst->end_info_ptr ? &inst->end_info_ptr : NULL
			);
		}
		free(inst);
	}
}
