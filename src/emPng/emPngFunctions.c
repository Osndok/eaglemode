/*------------------------------------------------------------------------------
// emPngFunctions.c
//
// Copyright (C) 2022,2025 Oliver Hamann.
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

#include <emPng/emPngFunctions.h>
#include <png.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


struct emPngErrorBuffer {
	jmp_buf JmpBuffer;
	char ErrorText[256];
};


struct emPngDecodeInstance {
	struct emPngErrorBuffer Err;
	png_structp PngPtr;
	png_infop InfoPtr;
	png_infop EndInfoPtr;
	png_uint_32 Width,Height,BytesPerPixel;
	int BitDepth,ColorType,InterlaceType;
	int NumberOfPasses;
	int Y,Pass;
};


struct emPngEncodeInstance {
	struct emPngErrorBuffer Err;
	png_structp PngPtr;
	png_infop InfoPtr;
	int Width,Height,BitDepth,PixelBits;
	int Y;
};


static void emPngCatStr(char * buf, size_t bufSize, const char * text)
{
	size_t l;

	l=strlen(buf);
	if (l<bufSize-1) {
		strncat(buf+l,text,bufSize-l-1);
	}
}


static void emPng_error_fn(png_structp pngPtr, png_const_charp error_msg)
{
	struct emPngErrorBuffer * err;

	err=(struct emPngErrorBuffer *)png_get_error_ptr(pngPtr);
	if (!error_msg) error_msg="PNG error";
	emPngCatStr(err->ErrorText,sizeof(err->ErrorText),error_msg);
	longjmp(err->JmpBuffer,1);
}


static void emPng_warning_fn(png_structp pngPtr, png_const_charp warning_msg)
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

	if (setjmp(inst->Err.JmpBuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->Err.ErrorText);
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->PngPtr=png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		(png_voidp)&inst->Err,
		emPng_error_fn,
		emPng_warning_fn
	);
	if (!inst->PngPtr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->InfoPtr=png_create_info_struct(inst->PngPtr);
	if (!inst->InfoPtr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	inst->EndInfoPtr=png_create_info_struct(inst->PngPtr);
	if (!inst->EndInfoPtr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	png_init_io(inst->PngPtr, file);

	png_read_info(inst->PngPtr, inst->InfoPtr);

	png_get_IHDR(
		inst->PngPtr,
		inst->InfoPtr,
		&inst->Width,
		&inst->Height,
		&inst->BitDepth,
		&inst->ColorType,
		&inst->InterlaceType,
		NULL,
		NULL
	);

	originalPixelSize=inst->BitDepth;
	if ((inst->ColorType&PNG_COLOR_MASK_PALETTE)==0) {
		originalPixelSize*=png_get_channels(inst->PngPtr,inst->InfoPtr);
	}

	png_set_expand(inst->PngPtr);
	png_set_strip_16(inst->PngPtr);
	png_set_packing(inst->PngPtr);
	inst->NumberOfPasses=png_set_interlace_handling(inst->PngPtr);
	png_read_update_info(inst->PngPtr, inst->InfoPtr);
	rowbytes=png_get_rowbytes(inst->PngPtr,inst->InfoPtr);
	inst->BytesPerPixel=rowbytes/inst->Width;
	if (
		rowbytes%inst->Width!=0 ||
		inst->BytesPerPixel<1 || inst->BytesPerPixel>4 ||
		inst->Width<1 || inst->Width>0x7fffff ||
		inst->Height<1 || inst->Height>0x7fffff
	) {
		emPngCatStr(errorBuf,errorBufSize,"Unsupported PNG format.");
		emPngQuitDecoding(inst);
		return NULL;
	}

	*width=inst->Width,
	*height=inst->Height,
	*channelCount=inst->BytesPerPixel;
	*passCount=inst->NumberOfPasses;

	snprintf(infoBuf,infoBufSize,"PNG %d-bit ",originalPixelSize);
	infoBuf[infoBufSize-1]=0;
	if ((inst->ColorType&PNG_COLOR_MASK_COLOR)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"color");
	}
	else {
		emPngCatStr(infoBuf,infoBufSize,"grayscale");
	}
	if ((inst->ColorType&PNG_COLOR_MASK_ALPHA)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"-alpha");
	}
	if ((inst->ColorType&PNG_COLOR_MASK_PALETTE)!=0) {
		emPngCatStr(infoBuf,infoBufSize,"-palette");
	}
	snprintf(
		infoBuf+strlen(infoBuf),
		infoBufSize-strlen(infoBuf),
		" (%d channels extracted)",
		(int)inst->BytesPerPixel
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

	if (setjmp(inst->Err.JmpBuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->Err.ErrorText);
		return -1;
	}

	if (inst->Y<(int)inst->Height && inst->Pass<inst->NumberOfPasses) {
		png_read_row(
			inst->PngPtr,
			rowBuf,
			NULL
		);
		inst->Y++;
		if (inst->Y>=(int)inst->Height) {
			inst->Y=0;
			inst->Pass++;
		}
		return 0;
	}

	png_read_end(inst->PngPtr,inst->EndInfoPtr);

	for (e=0; e<2; e++) {
		n=png_get_text(
			inst->PngPtr,
			e ? inst->EndInfoPtr : inst->InfoPtr,
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
		if (inst->PngPtr) {
			png_destroy_read_struct(
				&inst->PngPtr,
				inst->InfoPtr ? &inst->InfoPtr : NULL,
				inst->EndInfoPtr ? &inst->EndInfoPtr : NULL
			);
		}
		free(inst);
	}
}


void * emPngStartEncoding(
	FILE * file, int width, int height, int bitDepth, int pixelBits,
	unsigned char * palette, int palSize, const char * comment,
	char * errorBuf, size_t errorBufSize
)
{
	struct emPngEncodeInstance * inst;
	unsigned char * p;
	png_color palrgb[256];
	png_byte pala[256];
	png_text pngText;
	int i,j,colorType;

	inst=(struct emPngEncodeInstance*)malloc(sizeof(struct emPngEncodeInstance));
	memset(inst,0,sizeof(struct emPngEncodeInstance));
	inst->Width=width;
	inst->Height=height;
	inst->BitDepth=bitDepth;
	inst->PixelBits=pixelBits;

	*errorBuf=0;

	if (setjmp(inst->Err.JmpBuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->Err.ErrorText);
		emPngQuitEncoding(inst);
		return NULL;
	}

	inst->PngPtr=png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		(png_voidp)&inst->Err,
		emPng_error_fn,
		emPng_warning_fn
	);
	if (!inst->PngPtr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitEncoding(inst);
		return NULL;
	}

	inst->InfoPtr=png_create_info_struct(inst->PngPtr);
	if (!inst->InfoPtr) {
		emPngCatStr(errorBuf,errorBufSize,"PNG lib failed.");
		emPngQuitEncoding(inst);
		return NULL;
	}

	png_set_compression_level(inst->PngPtr,9);

	png_init_io(inst->PngPtr,file);

	if (palSize>0)          colorType=PNG_COLOR_TYPE_PALETTE;
	else if (pixelBits==8)  colorType=PNG_COLOR_TYPE_GRAY;
	else if (pixelBits==16) colorType=PNG_COLOR_TYPE_GRAY_ALPHA;
	else if (pixelBits==24) colorType=PNG_COLOR_TYPE_RGB;
	else                    colorType=PNG_COLOR_TYPE_RGB_ALPHA;

	png_set_IHDR(
		inst->PngPtr,inst->InfoPtr,
		width,height,
		bitDepth,colorType,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE
	);

	if (comment && *comment) {
		pngText.key=(char*)"Comment";
		pngText.text=(char*)comment;
		pngText.compression=PNG_TEXT_COMPRESSION_NONE;
		png_set_text(inst->PngPtr,inst->InfoPtr,&pngText,1);
	}

	if (colorType==PNG_COLOR_TYPE_PALETTE) {
		p=palette;
		for (i=0, j=0; i<palSize; i++) {
			palrgb[i].red=p[0];
			palrgb[i].green=p[1];
			palrgb[i].blue=p[2];
			pala[i]=p[3];
			if (pala[i]!=255) j=i+1;
			p+=4;
		}
		png_set_PLTE(inst->PngPtr,inst->InfoPtr,palrgb,palSize);
		if (j) png_set_tRNS(inst->PngPtr,inst->InfoPtr,pala,j,NULL);
	}

	png_write_info(inst->PngPtr,inst->InfoPtr);

	return inst;
}


int emPngContinueEncoding(
	void * instance, const unsigned char * rowBuf, char * errorBuf,
	size_t errorBufSize
)
{
	struct emPngEncodeInstance * inst;

	inst = (struct emPngEncodeInstance*)instance;

	*errorBuf=0;

	if (setjmp(inst->Err.JmpBuffer)) {
		emPngCatStr(errorBuf,errorBufSize,inst->Err.ErrorText);
		return -1;
	}

	if (inst->Y<inst->Height) {
		png_write_row(inst->PngPtr,rowBuf);
		inst->Y++;
		if (inst->Y<inst->Height) return 0;
	}

	png_write_end(inst->PngPtr,NULL);

	return 1;
}


void emPngQuitEncoding(void * instance)
{
	struct emPngEncodeInstance * inst;

	inst = (struct emPngEncodeInstance*)instance;
	if (inst) {
		if (inst->PngPtr) {
			png_destroy_write_struct(
				&inst->PngPtr,
				inst->InfoPtr ? &inst->InfoPtr : NULL
			);
		}
		free(inst);
	}
}
