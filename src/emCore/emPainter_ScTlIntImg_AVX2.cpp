//------------------------------------------------------------------------------
// emPainter_ScTlIntImg_AVX2.cpp
//
// Copyright (C) 2020 Oliver Hamann.
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

//------------------------------------------------------------------------------
// This cpp file includes itself multiple times in order to expand the
// algorithms for emPainter::ScanlineTool::InterpolateImageAvx2..(..) with
// different settings. The preprocessor defines for these settings are:
//   EXTENSION:  0, 1, 2       - One of EXTEND_TILED, EXTEND_EDGE, EXTEND_ZERO
//   CHANNELS:   1, 2, 3, or 4 - Number of channels in the input map.
//------------------------------------------------------------------------------

#if !defined(EXTENSION)
//==============================================================================
//===================== Top level include / Set EXTENSION ======================
//==============================================================================

#include "emPainter_ScTl.h"

#if EM_HAVE_X86_INTRINSICS
#	if defined(_MSC_VER)
#		include <immintrin.h>
#	else
#		include <x86intrin.h>
#	endif
#	define CONCATIMPL(a,b) a##b
#	define CONCAT(a,b) CONCATIMPL(a,b)
#	define EXTEND_TILED  0
#	define EXTEND_EDGE   1
#	define EXTEND_ZERO   2
#	define METHOD_NAME_EXTENSION_0 Et
#	define METHOD_NAME_EXTENSION_1 Ee
#	define METHOD_NAME_EXTENSION_2 Ez
#	define METHOD_NAME_CHANNELS_1 Cs1
#	define METHOD_NAME_CHANNELS_2 Cs2
#	define METHOD_NAME_CHANNELS_3 Cs3
#	define METHOD_NAME_CHANNELS_4 Cs4

#	define EXTENSION EXTEND_TILED
#	include "emPainter_ScTlIntImg_AVX2.cpp"
#	undef EXTENSION

#	define EXTENSION EXTEND_EDGE
#	include "emPainter_ScTlIntImg_AVX2.cpp"
#	undef EXTENSION

#	define EXTENSION EXTEND_ZERO
#	include "emPainter_ScTlIntImg_AVX2.cpp"
#	undef EXTENSION

#endif

#elif !defined(CHANNELS)
//==============================================================================
//================================ Set CHANNELS ================================
//==============================================================================

#define CHANNELS 1
#include "emPainter_ScTlIntImg_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 2
#include "emPainter_ScTlIntImg_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 3
#include "emPainter_ScTlIntImg_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 4
#include "emPainter_ScTlIntImg_AVX2.cpp"
#undef CHANNELS


#else
//==============================================================================
//======================== Define General Helper Macros ========================
//==============================================================================

// DEFINE_AND_SET_IMAGE_Y(Y,Y_IN,DY,SY)
#if EXTENSION==EXTEND_TILED
#	define DEFINE_AND_SET_IMAGE_Y(Y,Y_IN,DY,SY) \
		ssize_t Y=((Y_IN)*DY)%SY; \
		if (Y<0) Y+=SY;
#elif EXTENSION==EXTEND_EDGE
#	define DEFINE_AND_SET_IMAGE_Y(Y,Y_IN,DY,SY) \
		ssize_t Y=(Y_IN)*DY; \
		ssize_t Y##Clipped=Y; \
		if ((size_t)Y##Clipped>=(size_t)SY) { \
			if (Y##Clipped<0) Y##Clipped=0; \
			else Y##Clipped=SY-DY; \
		}
#else
#	define DEFINE_AND_SET_IMAGE_Y(Y,Y_IN,DY,SY) \
		ssize_t Y=(Y_IN)*DY;
#endif


// DEFINE_AND_COPY_IMAGE_Y(Y,Y_SRC)
#if EXTENSION==EXTEND_TILED
#	define DEFINE_AND_COPY_IMAGE_Y(Y,Y_SRC) \
		ssize_t Y=Y_SRC;
#elif EXTENSION==EXTEND_EDGE
#	define DEFINE_AND_COPY_IMAGE_Y(Y,Y_SRC) \
		ssize_t Y=Y_SRC; \
		ssize_t Y##Clipped=Y_SRC##Clipped;
#else
#	define DEFINE_AND_COPY_IMAGE_Y(Y,Y_SRC) \
		ssize_t Y=Y_SRC;
#endif


// INCREMENT_IMAGE_Y(Y,DY,SY)
#if EXTENSION==EXTEND_TILED
#	define INCREMENT_IMAGE_Y(Y,DY,SY) \
		Y+=DY; \
		if (Y>=SY) Y=0;
#elif EXTENSION==EXTEND_EDGE
#	define INCREMENT_IMAGE_Y(Y,DY,SY) \
		Y+=DY; \
		Y##Clipped=Y; \
		if ((size_t)Y##Clipped>=(size_t)SY) { \
			if (Y##Clipped<0) Y##Clipped=0; \
			else Y##Clipped=SY-DY; \
		}
#else
#	define INCREMENT_IMAGE_Y(Y,DY,SY) \
		Y+=DY;
#endif


// DEFINE_AND_SET_IMAGE_ROW_PTR(ROW_PTR,Y,SX,SY,MAP)
#if EXTENSION==EXTEND_TILED
#	define DEFINE_AND_SET_IMAGE_ROW_PTR(ROW_PTR,Y,SX,SY,MAP) \
		const emByte * ROW_PTR=MAP+Y;
#elif EXTENSION==EXTEND_EDGE
#	define DEFINE_AND_SET_IMAGE_ROW_PTR(ROW_PTR,Y,SX,SY,MAP) \
		const emByte * ROW_PTR=MAP+Y##Clipped;
#else
#	define DEFINE_AND_SET_IMAGE_ROW_PTR(ROW_PTR,Y,SX,SY,MAP) \
		const emByte * ROW_PTR=MAP+Y; \
		int ROW_PTR##UsedSX=SX; \
		if ((size_t)Y>=(size_t)SY) ROW_PTR##UsedSX=0;
#endif


// DEFINE_AND_SET_IMAGE_X(X,X_IN,DX,SX)
#if EXTENSION==EXTEND_TILED
#	define DEFINE_AND_SET_IMAGE_X(X,X_IN,DX,SX) \
		ssize_t X=((X_IN)*DX)%SX; \
		if (X<0) X+=SX;
#elif EXTENSION==EXTEND_EDGE
#	define DEFINE_AND_SET_IMAGE_X(X,X_IN,DX,SX) \
		ssize_t X=(X_IN)*DX; \
		ssize_t X##Clipped=X; \
		if ((size_t)X##Clipped>=(size_t)SX) { \
			if (X##Clipped<0) X##Clipped=0; \
			else X##Clipped=SX-DX; \
		}
#else
#	define DEFINE_AND_SET_IMAGE_X(X,X_IN,DX,SX) \
		ssize_t X=(X_IN)*DX;
#endif


// INCREMENT_IMAGE_X(X,DX,SX)
#if EXTENSION==EXTEND_TILED
#	define INCREMENT_IMAGE_X(X,DX,SX) \
		X+=DX; \
		if (X>=SX) X=0;
#elif EXTENSION==EXTEND_EDGE
#	define INCREMENT_IMAGE_X(X,DX,SX) \
		X+=DX; \
		X##Clipped=X; \
		if ((size_t)X##Clipped>=(size_t)SX) { \
			if (X##Clipped<0) X##Clipped=0; \
			else X##Clipped=SX-DX; \
		}
#else
#	define INCREMENT_IMAGE_X(X,DX,SX) \
		X+=DX;
#endif


// ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(X,SX)
#if EXTENSION==EXTEND_TILED
#	define ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(X,SX) \
		(X+16<=SX)
#elif EXTENSION==EXTEND_EDGE
#	define ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(X,SX) \
		(X>=0 && X+16<=SX)
#else
#	define ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(X,SX) \
		(X>=0 && X+16<=SX)
		// No need to test ROW_PTR##UsedSX, because the ZeroPixels
		// array has been made large enough.
#endif


// DEFINE_AND_SET_IMAGE_PIX_PTR(PIX_PTR,ROW_PTR,X)
#ifndef ZERO_PIXELS_DEFINED
#	define ZERO_PIXELS_DEFINED
	struct alignas(32) ZeroPixelsStruct {
		emByte data[32];
	};
	static const ZeroPixelsStruct ZeroPixels = {0};
#endif
#if EXTENSION==EXTEND_TILED
#	define DEFINE_AND_SET_IMAGE_PIX_PTR(PIX_PTR,ROW_PTR,X) \
		const emByte * PIX_PTR=ROW_PTR+X;
#elif EXTENSION==EXTEND_EDGE
#	define DEFINE_AND_SET_IMAGE_PIX_PTR(PIX_PTR,ROW_PTR,X) \
		const emByte * PIX_PTR=ROW_PTR+X##Clipped;
#else
#	define DEFINE_AND_SET_IMAGE_PIX_PTR(PIX_PTR,ROW_PTR,X) \
		const emByte * PIX_PTR=ROW_PTR+X; \
		if ((size_t)X>=(size_t)ROW_PTR##UsedSX) PIX_PTR=ZeroPixels.data;
#endif


// PREMULFIN_COLOR_VEC8(C)
#if CHANNELS==1 || CHANNELS==3
#	define PREMULFIN_COLOR_VEC8(C)
#elif CHANNELS==2
#	define PREMULFIN_COLOR_VEC8(C) { \
		__m256i c=_mm256_cvtepu8_epi16(C); \
		__m256i a=_mm256_shuffle_epi8(c,_mm256_set_epi8( \
			15,14,15,14, 11,10,11,10,  7, 6, 7, 6,  3, 2, 3, 2, \
			15,14,15,14, 11,10,11,10,  7, 6, 7, 6,  3, 2, 3, 2 \
		)); \
		c=_mm256_or_si256(c,_mm256_set1_epi32(0x00ff0000)); \
		c=_mm256_mullo_epi16(c,a); \
		c=_mm256_add_epi16(c,_mm256_set1_epi16(0x80)); \
		c=_mm256_add_epi16(c,_mm256_srli_epi16(c,8)); \
		c=_mm256_srli_epi16(c,8); \
		C=_mm_packus_epi16(_mm256_castsi256_si128(c),_mm256_extracti128_si256(c,1)); \
	}
#else
#	define PREMULFIN_COLOR_VEC8(C) { \
		__m256i c=_mm256_cvtepu8_epi16(C); \
		__m256i a=_mm256_shuffle_epi8(c,_mm256_set_epi8( \
			15,14,15,14, 15,14,15,14,  7, 6, 7, 6,  7, 6, 7, 6, \
			15,14,15,14, 15,14,15,14,  7, 6, 7, 6,  7, 6, 7, 6 \
		)); \
		c=_mm256_or_si256(c,_mm256_set_epi16(255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0)); \
		c=_mm256_mullo_epi16(c,a); \
		c=_mm256_add_epi16(c,_mm256_set1_epi16(0x80)); \
		c=_mm256_add_epi16(c,_mm256_srli_epi16(c,8)); \
		c=_mm256_srli_epi16(c,8); \
		C=_mm_packus_epi16(_mm256_castsi256_si128(c),_mm256_extracti128_si256(c,1)); \
	}
#endif


// PREMULFIN_SHL_COLOR_VEC16(C,S)
#if CHANNELS==1 || CHANNELS==3
#	define PREMULFIN_SHL_COLOR_VEC16(C,S) { \
		C=_mm256_slli_epi16(C,S); \
	}
#elif CHANNELS==2
#	define PREMULFIN_SHL_COLOR_VEC16(C,S) { \
		__m256i a=_mm256_shuffle_epi8(C,_mm256_set_epi8( \
			15,14,15,14, 11,10,11,10,  7, 6, 7, 6,  3, 2, 3, 2, \
			15,14,15,14, 11,10,11,10,  7, 6, 7, 6,  3, 2, 3, 2 \
		)); \
		C=_mm256_or_si256(C,_mm256_set1_epi32(0x00ff0000)); \
		C=_mm256_mullo_epi16(C,a); \
		C=_mm256_add_epi16(C,_mm256_set1_epi16(0x80)); \
		C=_mm256_add_epi16(C,_mm256_srli_epi16(C,8)); \
		C=_mm256_srli_epi16(C,8); \
		C=_mm256_slli_epi16(C,S); \
	}
#else
#	define PREMULFIN_SHL_COLOR_VEC16(C,S) { \
		__m256i a=_mm256_shuffle_epi8(C,_mm256_set_epi8( \
			15,14,15,14, 15,14,15,14,  7, 6, 7, 6,  7, 6, 7, 6, \
			15,14,15,14, 15,14,15,14,  7, 6, 7, 6,  7, 6, 7, 6 \
		)); \
		C=_mm256_or_si256(C,_mm256_set_epi16(255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0)); \
		C=_mm256_mullo_epi16(C,a); \
		C=_mm256_add_epi16(C,_mm256_set1_epi16(0x80)); \
		C=_mm256_add_epi16(C,_mm256_srli_epi16(C,8)); \
		C=_mm256_srli_epi16(C,8); \
		C=_mm256_slli_epi16(C,S); \
	}
#endif


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAvx2Nearest... ===========
//==============================================================================

#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(InterpolateImageAvx2Nearest,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY;
	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;
	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX;

	int sw=((tx&0xffffff)+0x1000000+(w-1)*tdx)>>24;
	if (w==sw) {
		DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
		do {
			__m128i v;
			if (ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(imgX,imgSX)) {
				DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
				v=_mm_loadu_si128((__m128i*)p);
				INCREMENT_IMAGE_X(imgX,((16/CHANNELS)*CHANNELS),imgSX)
			}
			else {
				for (int i=0, j=bufEnd-buf; i<=16-CHANNELS; i+=CHANNELS) {
					v=_mm_srli_si128(v,CHANNELS);
					if (i<j) {
						DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
#						if CHANNELS==1
							v=_mm_insert_epi8(v,p[0],15);
#						elif CHANNELS==2
							v=_mm_insert_epi16(v,((emUInt16*)p)[0],7);
#						elif CHANNELS==3
							v=_mm_insert_epi16(v,p[0]|(p[1]<<8),6);
							v=_mm_insert_epi8(v,p[2],14);
#						else
							v=_mm_insert_epi32(v,((emUInt32*)p)[0],3);
#						endif
						INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
					}
				}
			}
			PREMULFIN_COLOR_VEC8(v);
			_mm_storeu_si128((__m128i*)buf,v);
			buf+=(16/CHANNELS)*CHANNELS;
		} while (buf<bufEnd);
	}
	else {
		do {
			__m128i v;
			for (int i=0, j=bufEnd-buf; i<=16-CHANNELS; i+=CHANNELS) {
				v=_mm_srli_si128(v,CHANNELS);
				if (i<j) {
					DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
#					if CHANNELS==1
						v=_mm_insert_epi8(v,p[0],15);
#					elif CHANNELS==2
						v=_mm_insert_epi16(v,((emUInt16*)p)[0],7);
#					elif CHANNELS==3
						v=_mm_insert_epi16(v,p[0]|(p[1]<<8),6);
						v=_mm_insert_epi8(v,p[2],14);
#					else
						v=_mm_insert_epi32(v,((emUInt32*)p)[0],3);
#					endif
					tx+=tdx;
				}
			}
			PREMULFIN_COLOR_VEC8(v);
			_mm_storeu_si128((__m128i*)buf,v);
			buf+=(16/CHANNELS)*CHANNELS;
		} while (buf<bufEnd);
	}
}


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAvx2Bilinear... ==========
//==============================================================================

#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(InterpolateImageAvx2Bilinear,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x800000;
	emUInt32 oy=((ty&0xffffff)+0x7fff)>>16;

	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row0,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row1,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX-0x800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)

	tx=(tx&0xffffff)-0x1000000-tdx;
	int tc=((tx+0x3000000+w*tdx)>>24)*CHANNELS;

	const emByte * p=(emByte*)sct.InterpolationBuffer+InterpolationBufferSize-tc*2-64;
	p-=(p-(emByte*)NULL)&31;
	const emInt16 * pvyBeg=(emInt16*)p;
	const emInt16 * pvy=pvyBeg;
	const emInt16 * pvyEnd=pvyBeg+tc;

	__m256i fy1=_mm256_set1_epi16(oy<<6);
	__m256i fy0=_mm256_sub_epi16(_mm256_set1_epi16(16384),fy1);

	do {
		__m128i svy0,svy1;
		if (ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(imgX,imgSX)) {
			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			svy0=_mm_loadu_si128((__m128i*)p0);
			svy1=_mm_loadu_si128((__m128i*)p1);
			INCREMENT_IMAGE_X(imgX,((16/CHANNELS)*CHANNELS),imgSX)
		}
		else {
			for (int i=0, j=pvyEnd-pvy; i<=16-CHANNELS; i+=CHANNELS) {
				svy0=_mm_srli_si128(svy0,CHANNELS);
				svy1=_mm_srli_si128(svy1,CHANNELS);
				if (i<j) {
					DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
#					if CHANNELS==1
						svy0=_mm_insert_epi8(svy0,p0[0],15);
						svy1=_mm_insert_epi8(svy1,p1[0],15);
#					elif CHANNELS==2
						svy0=_mm_insert_epi16(svy0,((emUInt16*)p0)[0],7);
						svy1=_mm_insert_epi16(svy1,((emUInt16*)p1)[0],7);
#					elif CHANNELS==3
						svy0=_mm_insert_epi16(svy0,p0[0]|(p0[1]<<8),6);
						svy0=_mm_insert_epi8(svy0,p0[2],14);
						svy1=_mm_insert_epi16(svy1,p1[0]|(p1[1]<<8),6);
						svy1=_mm_insert_epi8(svy1,p1[2],14);
#					else
						svy0=_mm_insert_epi32(svy0,((emUInt32*)p0)[0],3);
						svy1=_mm_insert_epi32(svy1,((emUInt32*)p1)[0],3);
#					endif
					INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
				}
			}
		}

		__m256i vy0=_mm256_cvtepu8_epi16(svy0);
		__m256i vy1=_mm256_cvtepu8_epi16(svy1);

		PREMULFIN_SHL_COLOR_VEC16(vy0,7)
		PREMULFIN_SHL_COLOR_VEC16(vy1,7)

		__m256i vy=_mm256_add_epi16(
			_mm256_mulhrs_epi16(vy0,fy0),
			_mm256_mulhrs_epi16(vy1,fy1)
		);

		_mm256_storeu_si256((__m256i*)pvy,vy);
		pvy+=(16/CHANNELS)*CHANNELS;
	} while (pvy<pvyEnd);

	_mm256_storeu_si256((__m256i*)pvy,_mm256_setzero_si256());

	pvy=pvyBeg;
	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;

	// Order of pixels in v01 and vff with 3-4 / 1-2 channels:
	//   v01:   2 1 1 0   /    4 3 2 1  3 2 1 0
	//   vff:   4 3 5 2   /    8 7 6 5  b a 9 4
#	if CHANNELS<=2
#		if CHANNELS==1
			__m256i vt=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#		else
			__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
#		endif
		__m256i v01=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(4,3,2,1,3,2,1,0));
		__m256i vff=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,7,6,5,0,0,0,4));
		pvy+=CHANNELS*8;
		int vn=7;
#	elif CHANNELS==3
		__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
		__m256i vta=_mm256_shuffle_epi8(vt,_mm256_set_epi8(
			-1,-1, 1, 0, -1,-1,-1,-1, -1,-1, 7, 6,  5, 4, 3, 2,
			-1,-1,11,10,  9, 8, 7, 6, -1,-1,-1,-1, 15,14,13,12
		));
		__m256i vtb=_mm256_permutevar8x32_epi32(vta,_mm256_set_epi32(7,0,3,2,3,2,7,0));
		vta=_mm256_blend_epi32(vt,vta,0x3f);
		__m256i v01=_mm256_blend_epi32(vt,vtb,0xfc);
		__m256i vff=_mm256_blend_epi32(vta,vtb,0x0f);
		pvy+=CHANNELS*5;
		int vn=4;
#	else
		__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v01=_mm256_permute4x64_epi64(vt,0x94);
		__m256i vff=_mm256_permute4x64_epi64(vt,0x36);
		pvy+=CHANNELS*4;
		int vn=3;
#	endif

	do {
		__m256i v01l,v01h,f1l,f1h;
		int cx=16/CHANNELS-1;

		do {
			tx+=tdx;
			if (tx>=0) {
				tx-=0x1000000;

#				if CHANNELS<=2
					v01=_mm256_alignr_epi8(vff,v01,4);
					vff=_mm256_permutevar8x32_epi32(vff,_mm256_set_epi32(1,7,6,5,0,3,2,4));
#				else
					v01=_mm256_alignr_epi8(vff,v01,8);
					vff=_mm256_permute4x64_epi64(vff,0x72);
#				endif

				vn--;
				if (vn<=0) {
#					if CHANNELS<=2
#						if CHANNELS==1
							vff=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#						else
							vff=_mm256_loadu_si256((__m256i*)pvy);
#						endif
						__m256i t=_mm256_permutevar8x32_epi32(vff,_mm256_set_epi32(3,2,1,0,2,1,0,3));
						v01=_mm256_blend_epi32(v01,t,0xfe);
						vff=_mm256_blend_epi32(vff,t,0x0f);
						pvy+=CHANNELS*8;
						vn+=8;
#					elif CHANNELS==3
						__m256i t=_mm256_loadu_si256((__m256i*)pvy);
						__m256i t1=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(7,6,4,3,7,6,1,0));
						__m256i t2=_mm256_shuffle_epi8(t,_mm256_set_epi8(
							-1,-1, 7, 6,  5, 4, 3, 2, -1,-1, 1, 0, -1,-1,-1,-1,
							-1,-1, 5, 4,  3, 2, 1, 0, -1,-1,11,10,  9, 8, 7, 6
						));
						vff=_mm256_blend_epi32(t1,t2,0xc3);
						__m256i t3=_mm256_permute4x64_epi64(t2,0x14);
						v01=_mm256_blend_epi32(v01,t3,0xfc);
						pvy+=CHANNELS*5;
						vn+=5;
#					else
						vff=_mm256_loadu_si256((__m256i*)pvy);
						__m256i t=_mm256_permute4x64_epi64(vff,0x41);
						v01=_mm256_blend_epi32(v01,t,0xfc);
						vff=_mm256_blend_epi32(vff,t,0x0f);
						pvy+=CHANNELS*4;
						vn+=4;
#					endif
				}
			}

			emUInt32 ox=(tx+0x1007fff)>>16;

			__m256i f1=_mm256_castsi128_si256(_mm_set1_epi16(ox));

			if (cx==7/CHANNELS) {
#				if CHANNELS==3
					v01l=_mm256_alignr_epi8(v01,v01h,4);
					f1l=_mm256_alignr_epi8(f1,f1h,4);
#				else
					v01l=v01h;
					f1l=f1h;
#				endif
			}

			v01h=_mm256_alignr_epi8(v01,v01h,CHANNELS*2);
			f1h=_mm256_alignr_epi8(f1,f1h,CHANNELS*2);
			cx--;
		} while (cx>=0);

#		if CHANNELS==3
			v01h=_mm256_srli_si256(v01h,2);
			f1h=_mm256_srli_si256(f1h,2);
#		endif

		__m256i vx0=_mm256_permute2x128_si256(v01l,v01h,0x20);
		__m256i vx1=_mm256_permute2x128_si256(v01l,v01h,0x31);
		__m256i fx1=_mm256_permute2x128_si256(f1l,f1h,0x20);

		fx1=_mm256_slli_epi16(fx1,6);
		__m256i fx0=_mm256_sub_epi16(_mm256_set1_epi16(16384),fx1);

		__m256i vx=_mm256_add_epi16(
			_mm256_mulhrs_epi16(vx0,fx0),
			_mm256_mulhrs_epi16(vx1,fx1)
		);

		vx=_mm256_add_epi16(vx,_mm256_set1_epi16(0x10));
		vx=_mm256_srai_epi16(vx,5);
		__m128i svx=_mm_packus_epi16(
			_mm256_castsi256_si128(vx),
			_mm256_extracti128_si256(vx,1)
		);

		_mm_storeu_si128((__m128i*)buf,svx);

		buf+=(16/CHANNELS)*CHANNELS;
	} while (buf<bufEnd);
}


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAvx2Bicubic... ===========
//==============================================================================

#ifndef BICUBIC_FACTORS_TABLE_DEFINED
#	define BICUBIC_FACTORS_TABLE_DEFINED
	struct alignas(8) BicubicFactors {
		emInt16 f0;
		emInt16 f1;
		emInt16 f2;
		emInt16 f3;
	};
	static const BicubicFactors BicubicFactorsTable[257] = {
		// #include <stdio.h>
		// #include <math.h>
		// int main(int argc, char * argv[])
		// {
		//   for (int i=0; i<=256; i++) {
		//     double o=i/256.0;
		//     double s=1.0-o;
		//     double f=16384;
		//     int f0=(int)round((-0.5*s*o)*s*f);
		//     int f1=(int)round(((1 - 1.5*o)*o + 1)*s*f);
		//     int f2=(int)round(((1 - 1.5*s)*s + 1)*o*f);
		//     int f3=(int)round((-0.5*s*o)*o*f);
		//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",f0,f1,f2,f3);
		//   }
		//   return 0;
		// }
		{0,16384,0,0},{-32,16383,32,0},{-63,16382,66,0},{-94,16378,100,-1},
		{-124,16374,136,-2},{-154,16369,172,-3},{-183,16362,210,-4},{-212,16354,248,-6},
		{-240,16345,287,-8},{-268,16334,327,-10},{-295,16323,369,-12},{-322,16310,411,-14},
		{-349,16297,453,-17},{-375,16282,497,-20},{-400,16266,542,-23},{-425,16248,588,-26},
		{-450,16230,634,-30},{-474,16211,681,-34},{-498,16190,729,-38},{-521,16168,778,-42},
		{-544,16146,828,-46},{-566,16122,879,-51},{-588,16097,930,-55},{-610,16071,983,-60},
		{-631,16044,1036,-65},{-651,16016,1090,-70},{-672,15987,1144,-76},{-691,15957,1200,-82},
		{-711,15926,1256,-87},{-730,15894,1313,-93},{-748,15861,1370,-99},{-766,15827,1429,-106},
		{-784,15792,1488,-112},{-801,15756,1548,-119},{-818,15719,1608,-125},{-835,15681,1670,-132},
		{-851,15642,1732,-139},{-866,15603,1794,-146},{-882,15562,1858,-154},{-897,15520,1922,-161},
		{-911,15478,1986,-169},{-925,15434,2052,-176},{-939,15390,2117,-184},{-953,15345,2184,-192},
		{-966,15299,2251,-200},{-978,15252,2319,-209},{-991,15204,2387,-217},{-1002,15155,2456,-225},
		{-1014,15106,2526,-234},{-1025,15056,2596,-243},{-1036,15005,2667,-251},{-1047,14953,2738,-260},
		{-1057,14900,2810,-269},{-1066,14846,2882,-278},{-1076,14792,2955,-288},{-1085,14737,3029,-297},
		{-1094,14681,3103,-306},{-1102,14625,3177,-316},{-1110,14567,3252,-325},{-1118,14509,3328,-335},
		{-1125,14450,3404,-345},{-1133,14391,3480,-354},{-1139,14331,3557,-364},{-1146,14270,3634,-374},
		{-1152,14208,3712,-384},{-1158,14146,3790,-394},{-1163,14083,3869,-404},{-1169,14019,3948,-414},
		{-1174,13955,4027,-424},{-1178,13890,4107,-435},{-1182,13824,4188,-445},{-1187,13758,4268,-455},
		{-1190,13691,4349,-466},{-1194,13623,4431,-476},{-1197,13555,4512,-487},{-1200,13486,4595,-497},
		{-1202,13417,4677,-508},{-1205,13347,4760,-518},{-1207,13277,4843,-529},{-1208,13206,4926,-539},
		{-1210,13134,5010,-550},{-1211,13062,5094,-561},{-1212,12989,5178,-571},{-1213,12916,5263,-582},
		{-1213,12842,5348,-593},{-1214,12768,5433,-603},{-1214,12693,5518,-614},{-1213,12618,5604,-625},
		{-1213,12542,5690,-635},{-1212,12466,5776,-646},{-1211,12389,5862,-657},{-1210,12312,5949,-667},
		{-1208,12235,6035,-678},{-1207,12157,6122,-688},{-1205,12078,6209,-699},{-1202,11999,6297,-709},
		{-1200,11920,6384,-720},{-1197,11840,6472,-730},{-1195,11760,6559,-741},{-1192,11680,6647,-751},
		{-1188,11599,6735,-762},{-1185,11518,6823,-772},{-1181,11436,6911,-782},{-1177,11354,7000,-793},
		{-1173,11272,7088,-803},{-1169,11189,7177,-813},{-1165,11106,7265,-823},{-1160,11023,7354,-833},
		{-1155,10939,7443,-843},{-1150,10855,7531,-853},{-1145,10771,7620,-863},{-1140,10687,7709,-872},
		{-1134,10602,7798,-882},{-1128,10517,7887,-892},{-1122,10432,7976,-901},{-1116,10346,8065,-911},
		{-1110,10260,8154,-920},{-1104,10174,8242,-929},{-1097,10088,8331,-938},{-1091,10002,8420,-947},
		{-1084,9915,8509,-956},{-1077,9828,8597,-965},{-1070,9741,8686,-974},{-1062,9654,8775,-982},
		{-1055,9567,8863,-991},{-1047,9479,8951,-999},{-1040,9392,9040,-1008},{-1032,9304,9128,-1016},
		{-1024,9216,9216,-1024},{-1016,9128,9304,-1032},{-1008,9040,9392,-1040},{-999,8951,9479,-1047},
		{-991,8863,9567,-1055},{-982,8775,9654,-1062},{-974,8686,9741,-1070},{-965,8597,9828,-1077},
		{-956,8509,9915,-1084},{-947,8420,10002,-1091},{-938,8331,10088,-1097},{-929,8242,10174,-1104},
		{-920,8154,10260,-1110},{-911,8065,10346,-1116},{-901,7976,10432,-1122},{-892,7887,10517,-1128},
		{-882,7798,10602,-1134},{-872,7709,10687,-1140},{-863,7620,10771,-1145},{-853,7531,10855,-1150},
		{-843,7443,10939,-1155},{-833,7354,11023,-1160},{-823,7265,11106,-1165},{-813,7177,11189,-1169},
		{-803,7088,11272,-1173},{-793,7000,11354,-1177},{-782,6911,11436,-1181},{-772,6823,11518,-1185},
		{-762,6735,11599,-1188},{-751,6647,11680,-1192},{-741,6559,11760,-1195},{-730,6472,11840,-1197},
		{-720,6384,11920,-1200},{-709,6297,11999,-1202},{-699,6209,12078,-1205},{-688,6122,12157,-1207},
		{-678,6035,12235,-1208},{-667,5949,12312,-1210},{-657,5862,12389,-1211},{-646,5776,12466,-1212},
		{-635,5690,12542,-1213},{-625,5604,12618,-1213},{-614,5518,12693,-1214},{-603,5433,12768,-1214},
		{-593,5348,12842,-1213},{-582,5263,12916,-1213},{-571,5178,12989,-1212},{-561,5094,13062,-1211},
		{-550,5010,13134,-1210},{-539,4926,13206,-1208},{-529,4843,13277,-1207},{-518,4760,13347,-1205},
		{-508,4677,13417,-1202},{-497,4595,13486,-1200},{-487,4512,13555,-1197},{-476,4431,13623,-1194},
		{-466,4349,13691,-1190},{-455,4268,13758,-1187},{-445,4188,13824,-1182},{-435,4107,13890,-1178},
		{-424,4027,13955,-1174},{-414,3948,14019,-1169},{-404,3869,14083,-1163},{-394,3790,14146,-1158},
		{-384,3712,14208,-1152},{-374,3634,14270,-1146},{-364,3557,14331,-1139},{-354,3480,14391,-1133},
		{-345,3404,14450,-1125},{-335,3328,14509,-1118},{-325,3252,14567,-1110},{-316,3177,14625,-1102},
		{-306,3103,14681,-1094},{-297,3029,14737,-1085},{-288,2955,14792,-1076},{-278,2882,14846,-1066},
		{-269,2810,14900,-1057},{-260,2738,14953,-1047},{-251,2667,15005,-1036},{-243,2596,15056,-1025},
		{-234,2526,15106,-1014},{-225,2456,15155,-1002},{-217,2387,15204,-991},{-209,2319,15252,-978},
		{-200,2251,15299,-966},{-192,2184,15345,-953},{-184,2117,15390,-939},{-176,2052,15434,-925},
		{-169,1986,15478,-911},{-161,1922,15520,-897},{-154,1858,15562,-882},{-146,1794,15603,-866},
		{-139,1732,15642,-851},{-132,1670,15681,-835},{-125,1608,15719,-818},{-119,1548,15756,-801},
		{-112,1488,15792,-784},{-106,1429,15827,-766},{-99,1370,15861,-748},{-93,1313,15894,-730},
		{-87,1256,15926,-711},{-82,1200,15957,-691},{-76,1144,15987,-672},{-70,1090,16016,-651},
		{-65,1036,16044,-631},{-60,983,16071,-610},{-55,930,16097,-588},{-51,879,16122,-566},
		{-46,828,16146,-544},{-42,778,16168,-521},{-38,729,16190,-498},{-34,681,16211,-474},
		{-30,634,16230,-450},{-26,588,16248,-425},{-23,542,16266,-400},{-20,497,16282,-375},
		{-17,453,16297,-349},{-14,411,16310,-322},{-12,369,16323,-295},{-10,327,16334,-268},
		{-8,287,16345,-240},{-6,248,16354,-212},{-4,210,16362,-183},{-3,172,16369,-154},
		{-2,136,16374,-124},{-1,100,16378,-94},{0,66,16382,-63},{0,32,16383,-32},
		{0,0,16384,0}
	};
#endif


#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(InterpolateImageAvx2Bicubic,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x1800000;
	emUInt32 oy=((ty&0xffffff)+0x7fff)>>16;
	const BicubicFactors & fy=BicubicFactorsTable[oy];

	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row0,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row1,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row2,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row3,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX-0x1800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)

	tx=(tx&0xffffff)-0x1000000-tdx;
	int tc=((tx+0x5000000+w*tdx)>>24)*CHANNELS;

	const emByte * p=(emByte*)sct.InterpolationBuffer+InterpolationBufferSize-tc*2-64;
	p-=(p-(emByte*)NULL)&31;
	const emInt16 * pvyBeg=(emInt16*)p;
	const emInt16 * pvy=pvyBeg;
	const emInt16 * pvyEnd=pvyBeg+tc;

	__m128i sfy=_mm_loadl_epi64((__m128i*)&fy);
	sfy=_mm_unpacklo_epi16(sfy,sfy);
	__m256i afy=_mm256_broadcastsi128_si256(sfy);
	__m256i fy0=_mm256_shuffle_epi32(afy,0x00);
	__m256i fy1=_mm256_shuffle_epi32(afy,0x55);
	__m256i fy2=_mm256_shuffle_epi32(afy,0xaa);
	__m256i fy3=_mm256_shuffle_epi32(afy,0xff);

	do {
		__m128i svy0,svy1,svy2,svy3;
		if (ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(imgX,imgSX)) {
			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			svy0=_mm_loadu_si128((__m128i*)p0);
			svy1=_mm_loadu_si128((__m128i*)p1);
			svy2=_mm_loadu_si128((__m128i*)p2);
			svy3=_mm_loadu_si128((__m128i*)p3);
			INCREMENT_IMAGE_X(imgX,((16/CHANNELS)*CHANNELS),imgSX)
		}
		else {
			for (int i=0, j=pvyEnd-pvy; i<=16-CHANNELS; i+=CHANNELS) {
				svy0=_mm_srli_si128(svy0,CHANNELS);
				svy1=_mm_srli_si128(svy1,CHANNELS);
				svy2=_mm_srli_si128(svy2,CHANNELS);
				svy3=_mm_srli_si128(svy3,CHANNELS);
				if (i<j) {
					DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
#					if CHANNELS==1
						svy0=_mm_insert_epi8(svy0,p0[0],15);
						svy1=_mm_insert_epi8(svy1,p1[0],15);
						svy2=_mm_insert_epi8(svy2,p2[0],15);
						svy3=_mm_insert_epi8(svy3,p3[0],15);
#					elif CHANNELS==2
						svy0=_mm_insert_epi16(svy0,((emUInt16*)p0)[0],7);
						svy1=_mm_insert_epi16(svy1,((emUInt16*)p1)[0],7);
						svy2=_mm_insert_epi16(svy2,((emUInt16*)p2)[0],7);
						svy3=_mm_insert_epi16(svy3,((emUInt16*)p3)[0],7);
#					elif CHANNELS==3
						svy0=_mm_insert_epi16(svy0,p0[0]|(p0[1]<<8),6);
						svy0=_mm_insert_epi8(svy0,p0[2],14);
						svy1=_mm_insert_epi16(svy1,p1[0]|(p1[1]<<8),6);
						svy1=_mm_insert_epi8(svy1,p1[2],14);
						svy2=_mm_insert_epi16(svy2,p2[0]|(p2[1]<<8),6);
						svy2=_mm_insert_epi8(svy2,p2[2],14);
						svy3=_mm_insert_epi16(svy3,p3[0]|(p3[1]<<8),6);
						svy3=_mm_insert_epi8(svy3,p3[2],14);
#					else
						svy0=_mm_insert_epi32(svy0,((emUInt32*)p0)[0],3);
						svy1=_mm_insert_epi32(svy1,((emUInt32*)p1)[0],3);
						svy2=_mm_insert_epi32(svy2,((emUInt32*)p2)[0],3);
						svy3=_mm_insert_epi32(svy3,((emUInt32*)p3)[0],3);
#					endif
					INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
				}
			}
		}

		__m256i vy0=_mm256_cvtepu8_epi16(svy0);
		__m256i vy1=_mm256_cvtepu8_epi16(svy1);
		__m256i vy2=_mm256_cvtepu8_epi16(svy2);
		__m256i vy3=_mm256_cvtepu8_epi16(svy3);

		PREMULFIN_SHL_COLOR_VEC16(vy0,7)
		PREMULFIN_SHL_COLOR_VEC16(vy1,7)
		PREMULFIN_SHL_COLOR_VEC16(vy2,7)
		PREMULFIN_SHL_COLOR_VEC16(vy3,7)

		__m256i vy=_mm256_add_epi16(
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vy0,fy0),
				_mm256_mulhrs_epi16(vy1,fy1)
			),
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vy2,fy2),
				_mm256_mulhrs_epi16(vy3,fy3)
			)
		);

		_mm256_storeu_si256((__m256i*)pvy,vy);
		pvy+=(16/CHANNELS)*CHANNELS;
	} while (pvy<pvyEnd);

	_mm256_storeu_si256((__m256i*)pvy,_mm256_setzero_si256());

	pvy=pvyBeg;
	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;

	// Order of pixels in v02 and v13 with 3-4 / 1-2 channels:
	//   v02:   6 2 4 0   /   14 12 10  2   8  6  4  0
	//   v13:   7 3 5 1   /   15 13 11  3   9  7  5  1
#	if CHANNELS<=2
#		if CHANNELS==1
			__m256i vt=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#		else
			__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
#		endif
		__m256i v02=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,2,0,6,4,0));
		__m256i v13=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,3,0,7,5,1));
		pvy+=CHANNELS*8;
		int vn=5;
#	elif CHANNELS==3
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_permutevar8x32_epi32(v02,_mm256_set_epi32(5,4,4,3,7,6,2,1));
		v02=_mm256_blend_epi32(v02,v13,0xfc);
		v13=_mm256_blend_epi32(_mm256_srli_si256(v13,2),_mm256_srli_si256(v13,10),0xf0);
		pvy+=CHANNELS*5;
		int vn=2;
#	else
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_srli_si256(v02,8);
		pvy+=CHANNELS*4;
		int vn=1;
#	endif

	do {
		__m256i v02l,v02h,v13l,v13h,f02l,f02h,f13l,f13h;
		int cx=16/CHANNELS-1;

		do {
			tx+=tdx;
			if (tx>=0) {
				tx-=0x1000000;

				__m256i oldV02=v02;
				v02=v13;
#				if CHANNELS<=2
					v13=_mm256_permutevar8x32_epi32(oldV02,_mm256_set_epi32(0,7,6,1,5,3,2,4));
#				else
					v13=_mm256_permute4x64_epi64(oldV02,0x1e);
#				endif

				vn--;
				if (vn<=0) {
#					if CHANNELS<=2
#						if CHANNELS==1
							__m256i t=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#						else
							__m256i t=_mm256_loadu_si256((__m256i*)pvy);
#						endif
						__m256i ta=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,7,0,5,3,1,0));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,0,0,6,4,2,0));
						v02=_mm256_blend_epi32(v02,ta,0xee);
						v13=_mm256_blend_epi32(v13,tb,0xfe);
						pvy+=CHANNELS*8;
						vn+=8;
#					elif CHANNELS==3
						__m256i t=_mm256_loadu_si256((__m256i*)pvy);
						__m256i ta=_mm256_shuffle_epi8(t,_mm256_set_epi8(
							-1,-1, 7, 6,  5, 4, 3, 2, -1,-1,-1,-1, -1,-1,-1,-1,
							-1,-1,11,10,  9, 8, 7, 6, -1,-1,-1,-1, -1,-1,-1,-1
						));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(7,6,1,0,4,3,2,1));
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*5;
						vn+=5;
#					else
						__m256i ta=_mm256_loadu_si256((__m256i*)pvy);
						__m256i tb=_mm256_permute4x64_epi64(ta,0x0a);
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*4;
						vn+=4;
#					endif
				}
			}

			emUInt32 ox=(tx+0x1007fff)>>16;
			const BicubicFactors & fx=BicubicFactorsTable[ox];

			__m256i f02=_mm256_cvtepu16_epi64(_mm_loadl_epi64((__m128i*)&fx));
#			if CHANNELS>=2
				f02=_mm256_shuffle_epi8(f02,_mm256_set_epi8(
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0,
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0
				));
#			endif

			__m256i f13=_mm256_srli_si256(f02,8);

			if (cx==7/CHANNELS) {
#				if CHANNELS==3
					v02l=_mm256_alignr_epi8(v02,v02h,4);
					v13l=_mm256_alignr_epi8(v13,v13h,4);
					f02l=_mm256_alignr_epi8(f02,f02h,4);
					f13l=_mm256_alignr_epi8(f13,f13h,4);
#				else
					v02l=v02h;
					v13l=v13h;
					f02l=f02h;
					f13l=f13h;
#				endif
			}

			v02h=_mm256_alignr_epi8(v02,v02h,CHANNELS*2);
			v13h=_mm256_alignr_epi8(v13,v13h,CHANNELS*2);
			f02h=_mm256_alignr_epi8(f02,f02h,CHANNELS*2);
			f13h=_mm256_alignr_epi8(f13,f13h,CHANNELS*2);
			cx--;
		} while (cx>=0);

#		if CHANNELS==3
			v02h=_mm256_srli_si256(v02h,2);
			v13h=_mm256_srli_si256(v13h,2);
			f02h=_mm256_srli_si256(f02h,2);
			f13h=_mm256_srli_si256(f13h,2);
#		endif

		__m256i vx0=_mm256_permute2x128_si256(v02l,v02h,0x20);
		__m256i vx1=_mm256_permute2x128_si256(v13l,v13h,0x20);
		__m256i vx2=_mm256_permute2x128_si256(v02l,v02h,0x31);
		__m256i vx3=_mm256_permute2x128_si256(v13l,v13h,0x31);
		__m256i fx0=_mm256_permute2x128_si256(f02l,f02h,0x20);
		__m256i fx1=_mm256_permute2x128_si256(f13l,f13h,0x20);
		__m256i fx2=_mm256_permute2x128_si256(f02l,f02h,0x31);
		__m256i fx3=_mm256_permute2x128_si256(f13l,f13h,0x31);

		__m256i vx=_mm256_add_epi16(
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vx0,fx0),
				_mm256_mulhrs_epi16(vx1,fx1)
			),
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vx2,fx2),
				_mm256_mulhrs_epi16(vx3,fx3)
			)
		);

		vx=_mm256_add_epi16(vx,_mm256_set1_epi16(0x10));
		vx=_mm256_srai_epi16(vx,5);
		vx=_mm256_max_epi16(vx,_mm256_setzero_si256());
		__m128i svx=_mm_packus_epi16(
			_mm256_castsi256_si128(vx),
			_mm256_extracti128_si256(vx,1)
		);
#		if CHANNELS==2 || CHANNELS==4
			svx=_mm_min_epu8(svx,_mm_shuffle_epi8(svx,_mm_set_epi8(
#				if CHANNELS==2
					15,15,13,13,11,11,9,9,7,7,5,5,3,3,1,1
#				else
					15,15,15,15,11,11,11,11,7,7,7,7,3,3,3,3
#				endif
			)));
#		endif

		_mm_storeu_si128((__m128i*)buf,svx);

		buf+=(16/CHANNELS)*CHANNELS;
	} while (buf<bufEnd);
}


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAvx2Lanczos... ===========
//==============================================================================

#ifndef LANCZOS_FACTORS_TABLE_DEFINED
#	define LANCZOS_FACTORS_TABLE_DEFINED
	struct alignas(8) LanczosFactors {
		emInt16 f0;
		emInt16 f1;
		emInt16 f2;
		emInt16 f3;
	};
	static const LanczosFactors LanczosFactorsTable[257] = {
		// #include <stdio.h>
		// #include <math.h>
		// int main(int argc, char * argv[])
		// {
		//   for (int i=0; i<=256; i++) {
		//     double f=16384;
		//     double radius=2.5;
		//     double v[4];
		//     for (int j=0; j<4; j++) {
		//       double d=fabs((j-1-i/256.0)*M_PI);
		//       if (d<1E-10) v[j]=1.0/radius;
		//       else v[j]=sin(d)*sin(d/radius)/(d*d);
		//     }
		//     int f0=(int)round(f*v[0]/(v[0]+v[1]+v[2]+v[3]));
		//     int f1=(int)round(f*v[1]/(v[0]+v[1]+v[2]+v[3]));
		//     int f2=(int)round(f*v[2]/(v[0]+v[1]+v[2]+v[3]));
		//     int f3=(int)round(f*v[3]/(v[0]+v[1]+v[2]+v[3]));
		//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",f0,f1,f2,f3);
		//   }
		//   return 0;
		// }
		{0,16384,0,0},{-48,16391,49,-8},{-96,16397,98,-15},{-143,16402,148,-23},
		{-189,16406,199,-31},{-235,16408,250,-40},{-280,16410,302,-48},{-325,16411,355,-56},
		{-369,16411,408,-65},{-413,16409,462,-74},{-456,16407,516,-83},{-499,16403,572,-92},
		{-540,16399,627,-102},{-582,16393,684,-111},{-622,16387,741,-121},{-662,16379,798,-131},
		{-702,16370,857,-141},{-741,16361,916,-151},{-779,16350,975,-162},{-817,16338,1035,-172},
		{-854,16325,1096,-183},{-891,16311,1157,-194},{-927,16296,1219,-205},{-962,16280,1282,-216},
		{-997,16263,1345,-227},{-1031,16246,1408,-239},{-1065,16226,1473,-250},{-1098,16206,1538,-262},
		{-1130,16185,1603,-274},{-1162,16163,1669,-286},{-1194,16140,1736,-298},{-1224,16116,1803,-311},
		{-1254,16091,1870,-323},{-1284,16065,1939,-336},{-1313,16038,2007,-349},{-1341,16010,2077,-362},
		{-1369,15981,2147,-375},{-1396,15951,2217,-388},{-1422,15920,2288,-401},{-1448,15888,2360,-415},
		{-1474,15855,2432,-429},{-1499,15821,2504,-442},{-1523,15786,2577,-456},{-1547,15750,2651,-470},
		{-1570,15713,2725,-484},{-1592,15676,2799,-499},{-1614,15637,2874,-513},{-1635,15597,2950,-528},
		{-1656,15557,3026,-542},{-1676,15515,3102,-557},{-1696,15473,3179,-572},{-1715,15429,3257,-587},
		{-1734,15385,3334,-602},{-1752,15340,3413,-617},{-1769,15294,3491,-632},{-1786,15247,3571,-648},
		{-1803,15200,3650,-663},{-1818,15151,3730,-679},{-1834,15101,3811,-694},{-1848,15051,3891,-710},
		{-1863,15000,3973,-726},{-1876,14948,4054,-742},{-1889,14895,4136,-758},{-1902,14841,4219,-774},
		{-1914,14787,4301,-790},{-1926,14731,4384,-806},{-1937,14675,4468,-823},{-1947,14618,4552,-839},
		{-1957,14561,4636,-855},{-1967,14502,4720,-872},{-1976,14443,4805,-888},{-1984,14383,4890,-905},
		{-1992,14322,4976,-921},{-2000,14260,5061,-938},{-2007,14198,5147,-955},{-2014,14135,5234,-971},
		{-2020,14071,5320,-988},{-2025,14007,5407,-1005},{-2030,13942,5494,-1022},{-2035,13876,5582,-1038},
		{-2039,13809,5669,-1055},{-2043,13742,5757,-1072},{-2046,13674,5845,-1089},{-2049,13605,5934,-1106},
		{-2052,13536,6022,-1122},{-2054,13466,6111,-1139},{-2055,13396,6200,-1156},{-2056,13325,6289,-1173},
		{-2057,13253,6378,-1190},{-2058,13181,6467,-1207},{-2057,13108,6557,-1223},{-2057,13034,6647,-1240},
		{-2056,12960,6737,-1257},{-2055,12885,6827,-1273},{-2053,12810,6917,-1290},{-2051,12734,7007,-1306},
		{-2048,12658,7097,-1323},{-2045,12581,7188,-1339},{-2042,12504,7278,-1356},{-2039,12426,7369,-1372},
		{-2035,12347,7460,-1388},{-2030,12268,7550,-1405},{-2025,12189,7641,-1421},{-2020,12109,7732,-1437},
		{-2015,12029,7823,-1453},{-2009,11948,7914,-1468},{-2003,11867,8005,-1484},{-1997,11785,8096,-1500},
		{-1990,11703,8186,-1515},{-1983,11620,8277,-1531},{-1976,11538,8368,-1546},{-1968,11454,8459,-1561},
		{-1960,11371,8550,-1576},{-1952,11287,8640,-1591},{-1943,11202,8731,-1606},{-1934,11117,8822,-1621},
		{-1925,11032,8912,-1635},{-1916,10947,9002,-1649},{-1906,10861,9093,-1664},{-1896,10775,9183,-1678},
		{-1886,10688,9273,-1691},{-1876,10602,9363,-1705},{-1865,10515,9453,-1719},{-1854,10428,9542,-1732},
		{-1843,10340,9632,-1745},{-1831,10252,9721,-1758},{-1820,10164,9810,-1771},{-1808,10076,9899,-1783},
		{-1796,9988,9988,-1796},{-1783,9899,10076,-1808},{-1771,9810,10164,-1820},{-1758,9721,10252,-1831},
		{-1745,9632,10340,-1843},{-1732,9542,10428,-1854},{-1719,9453,10515,-1865},{-1705,9363,10602,-1876},
		{-1691,9273,10688,-1886},{-1678,9183,10775,-1896},{-1664,9093,10861,-1906},{-1649,9002,10947,-1916},
		{-1635,8912,11032,-1925},{-1621,8822,11117,-1934},{-1606,8731,11202,-1943},{-1591,8640,11287,-1952},
		{-1576,8550,11371,-1960},{-1561,8459,11454,-1968},{-1546,8368,11538,-1976},{-1531,8277,11620,-1983},
		{-1515,8186,11703,-1990},{-1500,8096,11785,-1997},{-1484,8005,11867,-2003},{-1468,7914,11948,-2009},
		{-1453,7823,12029,-2015},{-1437,7732,12109,-2020},{-1421,7641,12189,-2025},{-1405,7550,12268,-2030},
		{-1388,7460,12347,-2035},{-1372,7369,12426,-2039},{-1356,7278,12504,-2042},{-1339,7188,12581,-2045},
		{-1323,7097,12658,-2048},{-1306,7007,12734,-2051},{-1290,6917,12810,-2053},{-1273,6827,12885,-2055},
		{-1257,6737,12960,-2056},{-1240,6647,13034,-2057},{-1223,6557,13108,-2057},{-1207,6467,13181,-2058},
		{-1190,6378,13253,-2057},{-1173,6289,13325,-2056},{-1156,6200,13396,-2055},{-1139,6111,13466,-2054},
		{-1122,6022,13536,-2052},{-1106,5934,13605,-2049},{-1089,5845,13674,-2046},{-1072,5757,13742,-2043},
		{-1055,5669,13809,-2039},{-1038,5582,13876,-2035},{-1022,5494,13942,-2030},{-1005,5407,14007,-2025},
		{-988,5320,14071,-2020},{-971,5234,14135,-2014},{-955,5147,14198,-2007},{-938,5061,14260,-2000},
		{-921,4976,14322,-1992},{-905,4890,14383,-1984},{-888,4805,14443,-1976},{-872,4720,14502,-1967},
		{-855,4636,14561,-1957},{-839,4552,14618,-1947},{-823,4468,14675,-1937},{-806,4384,14731,-1926},
		{-790,4301,14787,-1914},{-774,4219,14841,-1902},{-758,4136,14895,-1889},{-742,4054,14948,-1876},
		{-726,3973,15000,-1863},{-710,3891,15051,-1848},{-694,3811,15101,-1834},{-679,3730,15151,-1818},
		{-663,3650,15200,-1803},{-648,3571,15247,-1786},{-632,3491,15294,-1769},{-617,3413,15340,-1752},
		{-602,3334,15385,-1734},{-587,3257,15429,-1715},{-572,3179,15473,-1696},{-557,3102,15515,-1676},
		{-542,3026,15557,-1656},{-528,2950,15597,-1635},{-513,2874,15637,-1614},{-499,2799,15676,-1592},
		{-484,2725,15713,-1570},{-470,2651,15750,-1547},{-456,2577,15786,-1523},{-442,2504,15821,-1499},
		{-429,2432,15855,-1474},{-415,2360,15888,-1448},{-401,2288,15920,-1422},{-388,2217,15951,-1396},
		{-375,2147,15981,-1369},{-362,2077,16010,-1341},{-349,2007,16038,-1313},{-336,1939,16065,-1284},
		{-323,1870,16091,-1254},{-311,1803,16116,-1224},{-298,1736,16140,-1194},{-286,1669,16163,-1162},
		{-274,1603,16185,-1130},{-262,1538,16206,-1098},{-250,1473,16226,-1065},{-239,1408,16246,-1031},
		{-227,1345,16263,-997},{-216,1282,16280,-962},{-205,1219,16296,-927},{-194,1157,16311,-891},
		{-183,1096,16325,-854},{-172,1035,16338,-817},{-162,975,16350,-779},{-151,916,16361,-741},
		{-141,857,16370,-702},{-131,798,16379,-662},{-121,741,16387,-622},{-111,684,16393,-582},
		{-102,627,16399,-540},{-92,572,16403,-499},{-83,516,16407,-456},{-74,462,16409,-413},
		{-65,408,16411,-369},{-56,355,16411,-325},{-48,302,16410,-280},{-40,250,16408,-235},
		{-31,199,16406,-189},{-23,148,16402,-143},{-15,98,16397,-96},{-8,49,16391,-48},
		{0,0,16384,0}
	};
#endif


#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(InterpolateImageAvx2Lanczos,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x1800000;
	emUInt32 oy=((ty&0xffffff)+0x7fff)>>16;
	const LanczosFactors & fy=LanczosFactorsTable[oy];

	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row0,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row1,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row2,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row3,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX-0x1800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)

	tx=(tx&0xffffff)-0x1000000-tdx;
	int tc=((tx+0x5000000+w*tdx)>>24)*CHANNELS;

	const emByte * p=(emByte*)sct.InterpolationBuffer+InterpolationBufferSize-tc*2-64;
	p-=(p-(emByte*)NULL)&31;
	const emInt16 * pvyBeg=(emInt16*)p;
	const emInt16 * pvy=pvyBeg;
	const emInt16 * pvyEnd=pvyBeg+tc;

	__m128i sfy=_mm_loadl_epi64((__m128i*)&fy);
	sfy=_mm_unpacklo_epi16(sfy,sfy);
	__m256i afy=_mm256_broadcastsi128_si256(sfy);
	__m256i fy0=_mm256_shuffle_epi32(afy,0x00);
	__m256i fy1=_mm256_shuffle_epi32(afy,0x55);
	__m256i fy2=_mm256_shuffle_epi32(afy,0xaa);
	__m256i fy3=_mm256_shuffle_epi32(afy,0xff);

	do {
		__m128i svy0,svy1,svy2,svy3;
		if (ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(imgX,imgSX)) {
			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			svy0=_mm_loadu_si128((__m128i*)p0);
			svy1=_mm_loadu_si128((__m128i*)p1);
			svy2=_mm_loadu_si128((__m128i*)p2);
			svy3=_mm_loadu_si128((__m128i*)p3);
			INCREMENT_IMAGE_X(imgX,((16/CHANNELS)*CHANNELS),imgSX)
		}
		else {
			for (int i=0, j=pvyEnd-pvy; i<=16-CHANNELS; i+=CHANNELS) {
				svy0=_mm_srli_si128(svy0,CHANNELS);
				svy1=_mm_srli_si128(svy1,CHANNELS);
				svy2=_mm_srli_si128(svy2,CHANNELS);
				svy3=_mm_srli_si128(svy3,CHANNELS);
				if (i<j) {
					DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
#					if CHANNELS==1
						svy0=_mm_insert_epi8(svy0,p0[0],15);
						svy1=_mm_insert_epi8(svy1,p1[0],15);
						svy2=_mm_insert_epi8(svy2,p2[0],15);
						svy3=_mm_insert_epi8(svy3,p3[0],15);
#					elif CHANNELS==2
						svy0=_mm_insert_epi16(svy0,((emUInt16*)p0)[0],7);
						svy1=_mm_insert_epi16(svy1,((emUInt16*)p1)[0],7);
						svy2=_mm_insert_epi16(svy2,((emUInt16*)p2)[0],7);
						svy3=_mm_insert_epi16(svy3,((emUInt16*)p3)[0],7);
#					elif CHANNELS==3
						svy0=_mm_insert_epi16(svy0,p0[0]|(p0[1]<<8),6);
						svy0=_mm_insert_epi8(svy0,p0[2],14);
						svy1=_mm_insert_epi16(svy1,p1[0]|(p1[1]<<8),6);
						svy1=_mm_insert_epi8(svy1,p1[2],14);
						svy2=_mm_insert_epi16(svy2,p2[0]|(p2[1]<<8),6);
						svy2=_mm_insert_epi8(svy2,p2[2],14);
						svy3=_mm_insert_epi16(svy3,p3[0]|(p3[1]<<8),6);
						svy3=_mm_insert_epi8(svy3,p3[2],14);
#					else
						svy0=_mm_insert_epi32(svy0,((emUInt32*)p0)[0],3);
						svy1=_mm_insert_epi32(svy1,((emUInt32*)p1)[0],3);
						svy2=_mm_insert_epi32(svy2,((emUInt32*)p2)[0],3);
						svy3=_mm_insert_epi32(svy3,((emUInt32*)p3)[0],3);
#					endif
					INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
				}
			}
		}

		__m256i vy0=_mm256_cvtepu8_epi16(svy0);
		__m256i vy1=_mm256_cvtepu8_epi16(svy1);
		__m256i vy2=_mm256_cvtepu8_epi16(svy2);
		__m256i vy3=_mm256_cvtepu8_epi16(svy3);

		PREMULFIN_SHL_COLOR_VEC16(vy0,7)
		PREMULFIN_SHL_COLOR_VEC16(vy1,7)
		PREMULFIN_SHL_COLOR_VEC16(vy2,7)
		PREMULFIN_SHL_COLOR_VEC16(vy3,7)

		__m256i vy=_mm256_add_epi16(
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vy0,fy0),
				_mm256_mulhrs_epi16(vy1,fy1)
			),
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vy2,fy2),
				_mm256_mulhrs_epi16(vy3,fy3)
			)
		);

		_mm256_storeu_si256((__m256i*)pvy,vy);
		pvy+=(16/CHANNELS)*CHANNELS;
	} while (pvy<pvyEnd);

	_mm256_storeu_si256((__m256i*)pvy,_mm256_setzero_si256());

	pvy=pvyBeg;
	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;

	// Order of pixels in v02 and v13 with 3-4 / 1-2 channels:
	//   v02:   6 2 4 0   /   14 12 10  2   8  6  4  0
	//   v13:   7 3 5 1   /   15 13 11  3   9  7  5  1
#	if CHANNELS<=2
#		if CHANNELS==1
			__m256i vt=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#		else
			__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
#		endif
		__m256i v02=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,2,0,6,4,0));
		__m256i v13=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,3,0,7,5,1));
		pvy+=CHANNELS*8;
		int vn=5;
#	elif CHANNELS==3
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_permutevar8x32_epi32(v02,_mm256_set_epi32(5,4,4,3,7,6,2,1));
		v02=_mm256_blend_epi32(v02,v13,0xfc);
		v13=_mm256_blend_epi32(_mm256_srli_si256(v13,2),_mm256_srli_si256(v13,10),0xf0);
		pvy+=CHANNELS*5;
		int vn=2;
#	else
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_srli_si256(v02,8);
		pvy+=CHANNELS*4;
		int vn=1;
#	endif

	do {
		__m256i v02l,v02h,v13l,v13h,f02l,f02h,f13l,f13h;
		int cx=16/CHANNELS-1;

		do {
			tx+=tdx;
			if (tx>=0) {
				tx-=0x1000000;

				__m256i oldV02=v02;
				v02=v13;
#				if CHANNELS<=2
					v13=_mm256_permutevar8x32_epi32(oldV02,_mm256_set_epi32(0,7,6,1,5,3,2,4));
#				else
					v13=_mm256_permute4x64_epi64(oldV02,0x1e);
#				endif

				vn--;
				if (vn<=0) {
#					if CHANNELS<=2
#						if CHANNELS==1
							__m256i t=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#						else
							__m256i t=_mm256_loadu_si256((__m256i*)pvy);
#						endif
						__m256i ta=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,7,0,5,3,1,0));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,0,0,6,4,2,0));
						v02=_mm256_blend_epi32(v02,ta,0xee);
						v13=_mm256_blend_epi32(v13,tb,0xfe);
						pvy+=CHANNELS*8;
						vn+=8;
#					elif CHANNELS==3
						__m256i t=_mm256_loadu_si256((__m256i*)pvy);
						__m256i ta=_mm256_shuffle_epi8(t,_mm256_set_epi8(
							-1,-1, 7, 6,  5, 4, 3, 2, -1,-1,-1,-1, -1,-1,-1,-1,
							-1,-1,11,10,  9, 8, 7, 6, -1,-1,-1,-1, -1,-1,-1,-1
						));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(7,6,1,0,4,3,2,1));
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*5;
						vn+=5;
#					else
						__m256i ta=_mm256_loadu_si256((__m256i*)pvy);
						__m256i tb=_mm256_permute4x64_epi64(ta,0x0a);
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*4;
						vn+=4;
#					endif
				}
			}

			emUInt32 ox=(tx+0x1007fff)>>16;
			const LanczosFactors & fx=LanczosFactorsTable[ox];

			__m256i f02=_mm256_cvtepu16_epi64(_mm_loadl_epi64((__m128i*)&fx));
#			if CHANNELS>=2
				f02=_mm256_shuffle_epi8(f02,_mm256_set_epi8(
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0,
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0
				));
#			endif

			__m256i f13=_mm256_srli_si256(f02,8);

			if (cx==7/CHANNELS) {
#				if CHANNELS==3
					v02l=_mm256_alignr_epi8(v02,v02h,4);
					v13l=_mm256_alignr_epi8(v13,v13h,4);
					f02l=_mm256_alignr_epi8(f02,f02h,4);
					f13l=_mm256_alignr_epi8(f13,f13h,4);
#				else
					v02l=v02h;
					v13l=v13h;
					f02l=f02h;
					f13l=f13h;
#				endif
			}

			v02h=_mm256_alignr_epi8(v02,v02h,CHANNELS*2);
			v13h=_mm256_alignr_epi8(v13,v13h,CHANNELS*2);
			f02h=_mm256_alignr_epi8(f02,f02h,CHANNELS*2);
			f13h=_mm256_alignr_epi8(f13,f13h,CHANNELS*2);
			cx--;
		} while (cx>=0);

#		if CHANNELS==3
			v02h=_mm256_srli_si256(v02h,2);
			v13h=_mm256_srli_si256(v13h,2);
			f02h=_mm256_srli_si256(f02h,2);
			f13h=_mm256_srli_si256(f13h,2);
#		endif

		__m256i vx0=_mm256_permute2x128_si256(v02l,v02h,0x20);
		__m256i vx1=_mm256_permute2x128_si256(v13l,v13h,0x20);
		__m256i vx2=_mm256_permute2x128_si256(v02l,v02h,0x31);
		__m256i vx3=_mm256_permute2x128_si256(v13l,v13h,0x31);
		__m256i fx0=_mm256_permute2x128_si256(f02l,f02h,0x20);
		__m256i fx1=_mm256_permute2x128_si256(f13l,f13h,0x20);
		__m256i fx2=_mm256_permute2x128_si256(f02l,f02h,0x31);
		__m256i fx3=_mm256_permute2x128_si256(f13l,f13h,0x31);

		__m256i vx=_mm256_add_epi16(
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vx0,fx0),
				_mm256_mulhrs_epi16(vx1,fx1)
			),
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(vx2,fx2),
				_mm256_mulhrs_epi16(vx3,fx3)
			)
		);

		vx=_mm256_add_epi16(vx,_mm256_set1_epi16(0x10));
		vx=_mm256_srai_epi16(vx,5);
		vx=_mm256_max_epi16(vx,_mm256_setzero_si256());
		__m128i svx=_mm_packus_epi16(
			_mm256_castsi256_si128(vx),
			_mm256_extracti128_si256(vx,1)
		);
#		if CHANNELS==2 || CHANNELS==4
			svx=_mm_min_epu8(svx,_mm_shuffle_epi8(svx,_mm_set_epi8(
#				if CHANNELS==2
					15,15,13,13,11,11,9,9,7,7,5,5,3,3,1,1
#				else
					15,15,15,15,11,11,11,11,7,7,7,7,3,3,3,3
#				endif
			)));
#		endif

		_mm_storeu_si128((__m128i*)buf,svx);

		buf+=(16/CHANNELS)*CHANNELS;
	} while (buf<bufEnd);
}


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAvx2Adaptive... ==========
//==============================================================================

//---------------------------- AdaptiveFactorsTable ----------------------------

#ifndef ADAPTIVE_FACTORS_TABLE_DEFINED
#	define ADAPTIVE_FACTORS_TABLE_DEFINED
	struct alignas(8) AdaptiveFactors {
		emInt16 fv1;
		emInt16 fv2;
		emInt16 fs1;
		emInt16 fs2;
	};
	static const AdaptiveFactors AdaptiveFactorsTable[257] = {
		// #include <stdio.h>
		// #include <math.h>
		// int main(int argc, char * argv[])
		// {
		//   for (int i=0; i<=256; i++) {
		//     double f=-32768.0; // Negative because +32768 is beyond signed 16-Bit
		//     double o=i/256.0;
		//     int fv1=(int)round((2*o*o*o-3*o*o+1)*f);
		//     int fv2=(int)round((-2*o*o*o+3*o*o)*f);
		//     int fs1=(int)round((o*o*o-2*o*o+o)*f);
		//     int fs2=(int)round((o*o*o-o*o)*f);
		//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",fv1,fv2,fs1,fs2);
		//   }
		//   return 0;
		// }
		{-32768,0,0,0},{-32767,-1,-127,0},{-32762,-6,-252,2},{-32755,-13,-375,4},
		{-32744,-24,-496,8},{-32731,-37,-615,12},{-32715,-53,-732,18},{-32696,-72,-848,24},
		{-32674,-94,-961,31},{-32649,-119,-1072,39},{-32622,-146,-1182,48},{-32592,-176,-1290,58},
		{-32559,-209,-1395,69},{-32523,-245,-1499,80},{-32485,-283,-1601,93},{-32444,-324,-1702,106},
		{-32400,-368,-1800,120},{-32354,-414,-1897,135},{-32305,-463,-1991,151},{-32253,-515,-2084,167},
		{-32199,-569,-2176,184},{-32143,-625,-2265,202},{-32084,-684,-2353,221},{-32022,-746,-2439,241},
		{-31958,-810,-2523,261},{-31892,-876,-2606,282},{-31823,-945,-2686,304},{-31751,-1017,-2765,326},
		{-31678,-1090,-2843,349},{-31602,-1166,-2919,373},{-31523,-1245,-2993,397},{-31443,-1325,-3065,422},
		{-31360,-1408,-3136,448},{-31275,-1493,-3205,474},{-31188,-1580,-3273,501},{-31098,-1670,-3339,529},
		{-31006,-1762,-3403,557},{-30912,-1856,-3466,586},{-30816,-1952,-3527,615},{-30718,-2050,-3587,645},
		{-30618,-2150,-3645,675},{-30516,-2252,-3702,706},{-30411,-2357,-3757,737},{-30305,-2463,-3810,769},
		{-30197,-2571,-3862,802},{-30086,-2682,-3913,835},{-29974,-2794,-3962,868},{-29860,-2908,-4010,902},
		{-29744,-3024,-4056,936},{-29626,-3142,-4101,971},{-29506,-3262,-4144,1006},{-29385,-3383,-4186,1041},
		{-29261,-3507,-4227,1077},{-29136,-3632,-4266,1114},{-29009,-3759,-4304,1150},{-28880,-3888,-4340,1188},
		{-28750,-4018,-4375,1225},{-28618,-4150,-4409,1263},{-28484,-4284,-4441,1301},{-28349,-4419,-4472,1339},
		{-28212,-4556,-4502,1378},{-28073,-4695,-4530,1417},{-27933,-4835,-4557,1457},{-27791,-4977,-4583,1496},
		{-27648,-5120,-4608,1536},{-27503,-5265,-4631,1576},{-27357,-5411,-4654,1616},{-27209,-5559,-4674,1657},
		{-27060,-5708,-4694,1698},{-26910,-5858,-4713,1739},{-26758,-6010,-4730,1780},{-26605,-6163,-4746,1821},
		{-26450,-6318,-4761,1863},{-26294,-6474,-4775,1905},{-26137,-6631,-4787,1947},{-25978,-6790,-4799,1989},
		{-25819,-6949,-4809,2031},{-25658,-7110,-4819,2073},{-25496,-7272,-4827,2115},{-25332,-7436,-4834,2158},
		{-25168,-7600,-4840,2200},{-25002,-7766,-4845,2243},{-24836,-7932,-4849,2285},{-24668,-8100,-4852,2328},
		{-24499,-8269,-4854,2370},{-24329,-8439,-4854,2413},{-24159,-8609,-4854,2456},{-23987,-8781,-4853,2498},
		{-23814,-8954,-4851,2541},{-23640,-9128,-4848,2584},{-23466,-9302,-4844,2626},{-23290,-9478,-4839,2669},
		{-23114,-9654,-4833,2711},{-22937,-9831,-4826,2753},{-22758,-10010,-4818,2796},{-22580,-10188,-4810,2838},
		{-22400,-10368,-4800,2880},{-22220,-10548,-4790,2922},{-22039,-10729,-4778,2964},{-21857,-10911,-4766,3005},
		{-21674,-11094,-4753,3047},{-21491,-11277,-4739,3088},{-21307,-11461,-4725,3129},{-21123,-11645,-4709,3170},
		{-20938,-11830,-4693,3211},{-20752,-12016,-4676,3252},{-20566,-12202,-4658,3292},{-20380,-12388,-4640,3332},
		{-20193,-12575,-4620,3372},{-20005,-12763,-4600,3411},{-19817,-12951,-4580,3450},{-19629,-13139,-4558,3489},
		{-19440,-13328,-4536,3528},{-19251,-13517,-4513,3566},{-19061,-13707,-4490,3604},{-18871,-13897,-4465,3642},
		{-18681,-14087,-4441,3679},{-18491,-14277,-4415,3716},{-18300,-14468,-4389,3753},{-18109,-14659,-4362,3789},
		{-17918,-14850,-4335,3825},{-17727,-15041,-4307,3860},{-17535,-15233,-4279,3895},{-17344,-15424,-4250,3930},
		{-17152,-15616,-4220,3964},{-16960,-15808,-4190,3998},{-16768,-16000,-4159,4031},{-16576,-16192,-4128,4064},
		{-16384,-16384,-4096,4096},{-16192,-16576,-4064,4128},{-16000,-16768,-4031,4159},{-15808,-16960,-3998,4190},
		{-15616,-17152,-3964,4220},{-15424,-17344,-3930,4250},{-15233,-17535,-3895,4279},{-15041,-17727,-3860,4307},
		{-14850,-17918,-3825,4335},{-14659,-18109,-3789,4362},{-14468,-18300,-3753,4389},{-14277,-18491,-3716,4415},
		{-14087,-18681,-3679,4441},{-13897,-18871,-3642,4465},{-13707,-19061,-3604,4490},{-13517,-19251,-3566,4513},
		{-13328,-19440,-3528,4536},{-13139,-19629,-3489,4558},{-12951,-19817,-3450,4580},{-12763,-20005,-3411,4600},
		{-12575,-20193,-3372,4620},{-12388,-20380,-3332,4640},{-12202,-20566,-3292,4658},{-12016,-20752,-3252,4676},
		{-11830,-20938,-3211,4693},{-11645,-21123,-3170,4709},{-11461,-21307,-3129,4725},{-11277,-21491,-3088,4739},
		{-11094,-21674,-3047,4753},{-10911,-21857,-3005,4766},{-10729,-22039,-2964,4778},{-10548,-22220,-2922,4790},
		{-10368,-22400,-2880,4800},{-10188,-22580,-2838,4810},{-10010,-22758,-2796,4818},{-9831,-22937,-2753,4826},
		{-9654,-23114,-2711,4833},{-9478,-23290,-2669,4839},{-9302,-23466,-2626,4844},{-9128,-23640,-2584,4848},
		{-8954,-23814,-2541,4851},{-8781,-23987,-2498,4853},{-8609,-24159,-2456,4854},{-8439,-24329,-2413,4854},
		{-8269,-24499,-2370,4854},{-8100,-24668,-2328,4852},{-7932,-24836,-2285,4849},{-7766,-25002,-2243,4845},
		{-7600,-25168,-2200,4840},{-7436,-25332,-2158,4834},{-7272,-25496,-2115,4827},{-7110,-25658,-2073,4819},
		{-6949,-25819,-2031,4809},{-6790,-25978,-1989,4799},{-6631,-26137,-1947,4787},{-6474,-26294,-1905,4775},
		{-6318,-26450,-1863,4761},{-6163,-26605,-1821,4746},{-6010,-26758,-1780,4730},{-5858,-26910,-1739,4713},
		{-5708,-27060,-1698,4694},{-5559,-27209,-1657,4674},{-5411,-27357,-1616,4654},{-5265,-27503,-1576,4631},
		{-5120,-27648,-1536,4608},{-4977,-27791,-1496,4583},{-4835,-27933,-1457,4557},{-4695,-28073,-1417,4530},
		{-4556,-28212,-1378,4502},{-4419,-28349,-1339,4472},{-4284,-28484,-1301,4441},{-4150,-28618,-1263,4409},
		{-4018,-28750,-1225,4375},{-3888,-28880,-1188,4340},{-3759,-29009,-1150,4304},{-3632,-29136,-1114,4266},
		{-3507,-29261,-1077,4227},{-3383,-29385,-1041,4186},{-3262,-29506,-1006,4144},{-3142,-29626,-971,4101},
		{-3024,-29744,-936,4056},{-2908,-29860,-902,4010},{-2794,-29974,-868,3962},{-2682,-30086,-835,3913},
		{-2571,-30197,-802,3862},{-2463,-30305,-769,3810},{-2357,-30411,-737,3757},{-2252,-30516,-706,3702},
		{-2150,-30618,-675,3645},{-2050,-30718,-645,3587},{-1952,-30816,-615,3527},{-1856,-30912,-586,3466},
		{-1762,-31006,-557,3403},{-1670,-31098,-529,3339},{-1580,-31188,-501,3273},{-1493,-31275,-474,3205},
		{-1408,-31360,-448,3136},{-1325,-31443,-422,3065},{-1245,-31523,-397,2993},{-1166,-31602,-373,2919},
		{-1090,-31678,-349,2843},{-1017,-31751,-326,2765},{-945,-31823,-304,2686},{-876,-31892,-282,2606},
		{-810,-31958,-261,2523},{-746,-32022,-241,2439},{-684,-32084,-221,2353},{-625,-32143,-202,2265},
		{-569,-32199,-184,2176},{-515,-32253,-167,2084},{-463,-32305,-151,1991},{-414,-32354,-135,1897},
		{-368,-32400,-120,1800},{-324,-32444,-106,1702},{-283,-32485,-93,1601},{-245,-32523,-80,1499},
		{-209,-32559,-69,1395},{-176,-32592,-58,1290},{-146,-32622,-48,1182},{-119,-32649,-39,1072},
		{-94,-32674,-31,961},{-72,-32696,-24,848},{-53,-32715,-18,732},{-37,-32731,-12,615},
		{-24,-32744,-8,496},{-13,-32755,-4,375},{-6,-32762,-2,252},{-1,-32767,0,127},
		{0,-32768,0,0}
	};
#endif


//----------------- Subroutine: InterpolateFourVectorsAdaptive -----------------

#ifndef INTERPOLATE_FOUR_VECTORS_ADAPTIVE_DEFINED
#	define INTERPOLATE_FOUR_VECTORS_ADAPTIVE_DEFINED

#	if defined(__GNUC__)
		__attribute__((target("avx2")))
#	endif
	static inline __m256i InterpolateFourVectorsAdaptive(
		__m256i v0, __m256i v1, __m256i v2, __m256i v3,
		__m256i fv1, __m256i fv2, __m256i fs1, __m256i fs2
	)
	{
		__m256i neg=_mm256_or_si256(
			_mm256_cmpgt_epi16(v2,v1),
			_mm256_set1_epi16(1)
		);

		v0=_mm256_sign_epi16(v0,neg);
		v1=_mm256_sign_epi16(v1,neg);
		v2=_mm256_sign_epi16(v2,neg);
		v3=_mm256_sign_epi16(v3,neg);

		__m256i s01=_mm256_sub_epi16(v1,v0);
		__m256i s12=_mm256_sub_epi16(v2,v1);
		__m256i s21=_mm256_sub_epi16(v1,v2);
		__m256i s23=_mm256_sub_epi16(v3,v2);

		__m256i s01x2=_mm256_add_epi16(s01,s01);
		__m256i s12x2=_mm256_add_epi16(s12,s12);
		__m256i s23x2=_mm256_add_epi16(s23,s23);

		__m256i s1=_mm256_min_epi16(
			_mm256_max_epi16(s01x2,s12),
			_mm256_max_epi16(s12x2,s01)
		);
		__m256i s2=_mm256_min_epi16(
			_mm256_max_epi16(s23x2,s12),
			_mm256_max_epi16(s12x2,s23)
		);

		__m256i q1=_mm256_sub_epi16(s1,s23x2);
		__m256i q2=_mm256_sub_epi16(s2,s01x2);

		s1=_mm256_add_epi16(
			s1,
			_mm256_min_epi16(
				_mm256_setzero_si256(),
				_mm256_max_epi16(q1,s1)
			)
		);
		s2=_mm256_add_epi16(
			s2,
			_mm256_min_epi16(
				_mm256_setzero_si256(),
				_mm256_max_epi16(q2,s2)
			)
		);

		s1=_mm256_min_epi16(_mm256_setzero_si256(),s1);
		s2=_mm256_min_epi16(_mm256_setzero_si256(),s2);

		__m256i s21p7=_mm256_add_epi16(s21,_mm256_set1_epi16(7));

		v1=_mm256_add_epi16(
			v1,
			_mm256_max_epi16(
				_mm256_setzero_si256(),
				_mm256_min_epi16(
					_mm256_min_epi16(s21,s01),
					_mm256_srai_epi16(_mm256_add_epi16(s01,s21p7),4)
				)
			)
		);
		v2=_mm256_sub_epi16(
			v2,
			_mm256_max_epi16(
				_mm256_setzero_si256(),
				_mm256_min_epi16(
					_mm256_min_epi16(s21,s23),
					_mm256_srai_epi16(_mm256_add_epi16(s23,s21p7),4)
				)
			)
		);

		__m256i v=_mm256_add_epi16(
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(v1,fv1),
				_mm256_mulhrs_epi16(v2,fv2)
			),
			_mm256_add_epi16(
				_mm256_mulhrs_epi16(s1,fs1),
				_mm256_mulhrs_epi16(s2,fs2)
			)
		);

		v=_mm256_sign_epi16(v,neg);
		return v;
	}
#endif


//---------- emPainter::ScanlineTool::InterpolateImageAvx2Adaptive... ----------

#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(InterpolateImageAvx2Adaptive,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x1800000;
	emUInt32 oy=((ty&0xffffff)+0x7fff)>>16;
	const AdaptiveFactors & fy=AdaptiveFactorsTable[oy];

	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row0,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row1,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row2,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row3,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX-0x1800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)

	tx=(tx&0xffffff)-0x1000000-tdx;
	int tc=((tx+0x5000000+w*tdx)>>24)*CHANNELS;

	const emByte * p=(emByte*)sct.InterpolationBuffer+InterpolationBufferSize-tc*2-64;
	p-=(p-(emByte*)NULL)&31;
	const emInt16 * pvyBeg=(emInt16*)p;
	const emInt16 * pvy=pvyBeg;
	const emInt16 * pvyEnd=pvyBeg+tc;

	__m128i sfy=_mm_loadl_epi64((__m128i*)&fy);
	sfy=_mm_unpacklo_epi16(sfy,sfy);
	__m256i afy=_mm256_broadcastsi128_si256(sfy);
	__m256i fy0=_mm256_shuffle_epi32(afy,0x00);
	__m256i fy1=_mm256_shuffle_epi32(afy,0x55);
	__m256i fy2=_mm256_shuffle_epi32(afy,0xaa);
	__m256i fy3=_mm256_shuffle_epi32(afy,0xff);

	do {
		__m128i svy0,svy1,svy2,svy3;
		if (ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X(imgX,imgSX)) {
			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			svy0=_mm_loadu_si128((__m128i*)p0);
			svy1=_mm_loadu_si128((__m128i*)p1);
			svy2=_mm_loadu_si128((__m128i*)p2);
			svy3=_mm_loadu_si128((__m128i*)p3);
			INCREMENT_IMAGE_X(imgX,((16/CHANNELS)*CHANNELS),imgSX)
		}
		else {
			for (int i=0, j=pvyEnd-pvy; i<=16-CHANNELS; i+=CHANNELS) {
				svy0=_mm_srli_si128(svy0,CHANNELS);
				svy1=_mm_srli_si128(svy1,CHANNELS);
				svy2=_mm_srli_si128(svy2,CHANNELS);
				svy3=_mm_srli_si128(svy3,CHANNELS);
				if (i<j) {
					DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
#					if CHANNELS==1
						svy0=_mm_insert_epi8(svy0,p0[0],15);
						svy1=_mm_insert_epi8(svy1,p1[0],15);
						svy2=_mm_insert_epi8(svy2,p2[0],15);
						svy3=_mm_insert_epi8(svy3,p3[0],15);
#					elif CHANNELS==2
						svy0=_mm_insert_epi16(svy0,((emUInt16*)p0)[0],7);
						svy1=_mm_insert_epi16(svy1,((emUInt16*)p1)[0],7);
						svy2=_mm_insert_epi16(svy2,((emUInt16*)p2)[0],7);
						svy3=_mm_insert_epi16(svy3,((emUInt16*)p3)[0],7);
#					elif CHANNELS==3
						svy0=_mm_insert_epi16(svy0,p0[0]|(p0[1]<<8),6);
						svy0=_mm_insert_epi8(svy0,p0[2],14);
						svy1=_mm_insert_epi16(svy1,p1[0]|(p1[1]<<8),6);
						svy1=_mm_insert_epi8(svy1,p1[2],14);
						svy2=_mm_insert_epi16(svy2,p2[0]|(p2[1]<<8),6);
						svy2=_mm_insert_epi8(svy2,p2[2],14);
						svy3=_mm_insert_epi16(svy3,p3[0]|(p3[1]<<8),6);
						svy3=_mm_insert_epi8(svy3,p3[2],14);
#					else
						svy0=_mm_insert_epi32(svy0,((emUInt32*)p0)[0],3);
						svy1=_mm_insert_epi32(svy1,((emUInt32*)p1)[0],3);
						svy2=_mm_insert_epi32(svy2,((emUInt32*)p2)[0],3);
						svy3=_mm_insert_epi32(svy3,((emUInt32*)p3)[0],3);
#					endif
					INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
				}
			}
		}

		__m256i vy0=_mm256_cvtepu8_epi16(svy0);
		__m256i vy1=_mm256_cvtepu8_epi16(svy1);
		__m256i vy2=_mm256_cvtepu8_epi16(svy2);
		__m256i vy3=_mm256_cvtepu8_epi16(svy3);

		PREMULFIN_SHL_COLOR_VEC16(vy0,5)
		PREMULFIN_SHL_COLOR_VEC16(vy1,5)
		PREMULFIN_SHL_COLOR_VEC16(vy2,5)
		PREMULFIN_SHL_COLOR_VEC16(vy3,5)

		__m256i vy=InterpolateFourVectorsAdaptive(vy0,vy1,vy2,vy3,fy0,fy1,fy2,fy3);

		_mm256_storeu_si256((__m256i*)pvy,vy);
		pvy+=(16/CHANNELS)*CHANNELS;
	} while (pvy<pvyEnd);

	_mm256_storeu_si256((__m256i*)pvy,_mm256_setzero_si256());

	pvy=pvyBeg;
	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;

	// Order of pixels in v02 and v13 with 3-4 / 1-2 channels:
	//   v02:   6 2 4 0   /   14 12 10  2   8  6  4  0
	//   v13:   7 3 5 1   /   15 13 11  3   9  7  5  1
#	if CHANNELS<=2
#		if CHANNELS==1
			__m256i vt=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#		else
			__m256i vt=_mm256_loadu_si256((__m256i*)pvy);
#		endif
		__m256i v02=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,2,0,6,4,0));
		__m256i v13=_mm256_permutevar8x32_epi32(vt,_mm256_set_epi32(0,0,0,3,0,7,5,1));
		pvy+=CHANNELS*8;
		int vn=5;
#	elif CHANNELS==3
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_permutevar8x32_epi32(v02,_mm256_set_epi32(5,4,4,3,7,6,2,1));
		v02=_mm256_blend_epi32(v02,v13,0xfc);
		v13=_mm256_blend_epi32(_mm256_srli_si256(v13,2),_mm256_srli_si256(v13,10),0xf0);
		pvy+=CHANNELS*5;
		int vn=2;
#	else
		__m256i v02=_mm256_loadu_si256((__m256i*)pvy);
		__m256i v13=_mm256_srli_si256(v02,8);
		pvy+=CHANNELS*4;
		int vn=1;
#	endif

	do {
		__m256i v02l,v02h,v13l,v13h,f02l,f02h,f13l,f13h;
		int cx=16/CHANNELS-1;

		do {
			tx+=tdx;
			if (tx>=0) {
				tx-=0x1000000;

				__m256i oldV02=v02;
				v02=v13;
#				if CHANNELS<=2
					v13=_mm256_permutevar8x32_epi32(oldV02,_mm256_set_epi32(0,7,6,1,5,3,2,4));
#				else
					v13=_mm256_permute4x64_epi64(oldV02,0x1e);
#				endif

				vn--;
				if (vn<=0) {
#					if CHANNELS<=2
#						if CHANNELS==1
							__m256i t=_mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i*)pvy));
#						else
							__m256i t=_mm256_loadu_si256((__m256i*)pvy);
#						endif
						__m256i ta=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,7,0,5,3,1,0));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(0,0,0,0,6,4,2,0));
						v02=_mm256_blend_epi32(v02,ta,0xee);
						v13=_mm256_blend_epi32(v13,tb,0xfe);
						pvy+=CHANNELS*8;
						vn+=8;
#					elif CHANNELS==3
						__m256i t=_mm256_loadu_si256((__m256i*)pvy);
						__m256i ta=_mm256_shuffle_epi8(t,_mm256_set_epi8(
							-1,-1, 7, 6,  5, 4, 3, 2, -1,-1,-1,-1, -1,-1,-1,-1,
							-1,-1,11,10,  9, 8, 7, 6, -1,-1,-1,-1, -1,-1,-1,-1
						));
						__m256i tb=_mm256_permutevar8x32_epi32(t,_mm256_set_epi32(7,6,1,0,4,3,2,1));
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*5;
						vn+=5;
#					else
						__m256i ta=_mm256_loadu_si256((__m256i*)pvy);
						__m256i tb=_mm256_permute4x64_epi64(ta,0x0a);
						v02=_mm256_blend_epi32(v02,ta,0xcc);
						v13=_mm256_blend_epi32(v13,tb,0xfc);
						pvy+=CHANNELS*4;
						vn+=4;
#					endif
				}
			}

			emUInt32 ox=(tx+0x1007fff)>>16;
			const AdaptiveFactors & fx=AdaptiveFactorsTable[ox];

			__m256i f02=_mm256_cvtepu16_epi64(_mm_loadl_epi64((__m128i*)&fx));
#			if CHANNELS>=2
				f02=_mm256_shuffle_epi8(f02,_mm256_set_epi8(
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0,
					9, 8, 9, 8, 9, 8, 9, 8, 1, 0, 1, 0, 1, 0, 1, 0
				));
#			endif

			__m256i f13=_mm256_srli_si256(f02,8);

			if (cx==7/CHANNELS) {
#				if CHANNELS==3
					v02l=_mm256_alignr_epi8(v02,v02h,4);
					v13l=_mm256_alignr_epi8(v13,v13h,4);
					f02l=_mm256_alignr_epi8(f02,f02h,4);
					f13l=_mm256_alignr_epi8(f13,f13h,4);
#				else
					v02l=v02h;
					v13l=v13h;
					f02l=f02h;
					f13l=f13h;
#				endif
			}

			v02h=_mm256_alignr_epi8(v02,v02h,CHANNELS*2);
			v13h=_mm256_alignr_epi8(v13,v13h,CHANNELS*2);
			f02h=_mm256_alignr_epi8(f02,f02h,CHANNELS*2);
			f13h=_mm256_alignr_epi8(f13,f13h,CHANNELS*2);
			cx--;
		} while (cx>=0);

#		if CHANNELS==3
			v02h=_mm256_srli_si256(v02h,2);
			v13h=_mm256_srli_si256(v13h,2);
			f02h=_mm256_srli_si256(f02h,2);
			f13h=_mm256_srli_si256(f13h,2);
#		endif

		__m256i vx0=_mm256_permute2x128_si256(v02l,v02h,0x20);
		__m256i vx1=_mm256_permute2x128_si256(v13l,v13h,0x20);
		__m256i vx2=_mm256_permute2x128_si256(v02l,v02h,0x31);
		__m256i vx3=_mm256_permute2x128_si256(v13l,v13h,0x31);
		__m256i fx0=_mm256_permute2x128_si256(f02l,f02h,0x20);
		__m256i fx1=_mm256_permute2x128_si256(f13l,f13h,0x20);
		__m256i fx2=_mm256_permute2x128_si256(f02l,f02h,0x31);
		__m256i fx3=_mm256_permute2x128_si256(f13l,f13h,0x31);

		__m256i vx=InterpolateFourVectorsAdaptive(vx0,vx1,vx2,vx3,fx0,fx1,fx2,fx3);

		vx=_mm256_add_epi16(vx,_mm256_set1_epi16(0x10));
		vx=_mm256_srai_epi16(vx,5);
		vx=_mm256_max_epi16(vx,_mm256_setzero_si256());
		__m128i svx=_mm_packus_epi16(
			_mm256_castsi256_si128(vx),
			_mm256_extracti128_si256(vx,1)
		);
#		if CHANNELS==2 || CHANNELS==4
			svx=_mm_min_epu8(svx,_mm_shuffle_epi8(svx,_mm_set_epi8(
#				if CHANNELS==2
					15,15,13,13,11,11,9,9,7,7,5,5,3,3,1,1
#				else
					15,15,15,15,11,11,11,11,7,7,7,7,3,3,3,3
#				endif
			)));
#		endif

		_mm_storeu_si128((__m128i*)buf,svx);

		buf+=(16/CHANNELS)*CHANNELS;
	} while (buf<bufEnd);
}


//==============================================================================
//======================= Undefine General Helper Macros =======================
//==============================================================================

#undef DEFINE_AND_SET_IMAGE_Y
#undef DEFINE_AND_COPY_IMAGE_Y
#undef INCREMENT_IMAGE_Y
#undef DEFINE_AND_SET_IMAGE_ROW_PTR
#undef DEFINE_AND_SET_IMAGE_X
#undef INCREMENT_IMAGE_X
#undef ARE_THERE_16_CONSECUTIVE_BYTES_AT_IMAGE_X
#undef DEFINE_AND_SET_IMAGE_PIX_PTR
#undef PREMULFIN_COLOR_VEC8
#undef PREMULFIN_SHL_COLOR_VEC16


#endif
