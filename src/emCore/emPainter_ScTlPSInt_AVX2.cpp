//------------------------------------------------------------------------------
// emPainter_ScTlPSInt_AVX2.cpp
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
// algorithms for emPainter::ScanlineTool::PaintScanlineIntAvx2..(..) with
// different settings. The preprocessor defines for these settings are:
//   HAVE_GC1:   0 or 1        - Whether to paint a gradient with color 1 (this
//                               can be combined with HAVE_C2 but not with
//                               HAVE_ALPHA).
//   HAVE_GC2:   0 or 1        - Whether to paint a gradient with color 2 (this
//                               can be combined with HAVE_C1 but not with
//                               HAVE_ALPHA).
//   HAVE_ALPHA: 0 or 1        - Whether to alpha blend (when no gradient)
//   CHANNELS:   1, 2, 3, or 4 - Number of channels in the input map.
//   PIXEL_FORMAT: 0 ... 3     - See OptimizedPixelFormatIndex.
//   HAVE_CVC:   0 or 1        - Whether canvas color is given (opaque).
//------------------------------------------------------------------------------

#if !defined(HAVE_GC1)
//==============================================================================
//====================== Top level include / Set HAVE_GC1 ======================
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
#	define PF_8888_0BGR 0
#	define PF_8888_0RGB 1
#	define PF_8888_BGR0 2
#	define PF_8888_RGB0 3
#	define METHOD_NAME_HAVE_GC1_0
#	define METHOD_NAME_HAVE_GC1_1 G1
#	define METHOD_NAME_HAVE_GC2_0
#	define METHOD_NAME_HAVE_GC2_1 G2
#	define METHOD_NAME_HAVE_ALPHA_0
#	define METHOD_NAME_HAVE_ALPHA_1 A
#	define METHOD_NAME_CHANNELS_1 Cs1
#	define METHOD_NAME_CHANNELS_2 Cs2
#	define METHOD_NAME_CHANNELS_3 Cs3
#	define METHOD_NAME_CHANNELS_4 Cs4
#	define METHOD_NAME_PIXEL_FORMAT_0 Pf0
#	define METHOD_NAME_PIXEL_FORMAT_1 Pf1
#	define METHOD_NAME_PIXEL_FORMAT_2 Pf2
#	define METHOD_NAME_PIXEL_FORMAT_3 Pf3
#	define METHOD_NAME_HAVE_CVC_0
#	define METHOD_NAME_HAVE_CVC_1 Cv

#	define HAVE_GC1 0
#	include "emPainter_ScTlPSInt_AVX2.cpp"
#	undef HAVE_GC1

#	define HAVE_GC1 1
#	include "emPainter_ScTlPSInt_AVX2.cpp"
#	undef HAVE_GC1

#endif


#elif !defined(HAVE_GC2)
//==============================================================================
//================================ Set HAVE_GC2 ================================
//==============================================================================

#define HAVE_GC2 0
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef HAVE_GC2

#define HAVE_GC2 1
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef HAVE_GC2


#elif !defined(HAVE_ALPHA)
//==============================================================================
//=============================== Set HAVE_ALPHA ===============================
//==============================================================================

#define HAVE_ALPHA 0
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef HAVE_ALPHA

// Alpha is relevant only if no gradient.
#if !HAVE_GC1 && !HAVE_GC2
#	define HAVE_ALPHA 1
#	include "emPainter_ScTlPSInt_AVX2.cpp"
#	undef HAVE_ALPHA
#endif


#elif !defined(CHANNELS)
//==============================================================================
//================================ Set CHANNELS ================================
//==============================================================================

#define CHANNELS 1
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 2
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 3
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef CHANNELS

#define CHANNELS 4
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef CHANNELS


#elif !defined(PIXEL_FORMAT)
//==============================================================================
//============================== Set PIXEL_FORMAT ==============================
//==============================================================================

#define PIXEL_FORMAT PF_8888_0BGR
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef PIXEL_FORMAT

#define PIXEL_FORMAT PF_8888_0RGB
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef PIXEL_FORMAT

#define PIXEL_FORMAT PF_8888_BGR0
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef PIXEL_FORMAT

#define PIXEL_FORMAT PF_8888_RGB0
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef PIXEL_FORMAT


#elif !defined(HAVE_CVC)
//==============================================================================
//================================ Set HAVE_CVC ================================
//==============================================================================

#define HAVE_CVC 0
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef HAVE_CVC

#define HAVE_CVC 1
#include "emPainter_ScTlPSInt_AVX2.cpp"
#undef HAVE_CVC


#else
//==============================================================================
//============== emPainter::ScanlineTool::PaintScanlineIntAvx2... ==============
//==============================================================================

#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(PaintScanlineIntAvx2,CONCAT(
	CONCAT(
		CONCAT(
			CONCAT(METHOD_NAME_HAVE_GC1_,HAVE_GC1),
			CONCAT(METHOD_NAME_HAVE_GC2_,HAVE_GC2)
		),
		CONCAT(
			CONCAT(METHOD_NAME_HAVE_ALPHA_,HAVE_ALPHA),
			CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
		)
	),
	CONCAT(
		CONCAT(METHOD_NAME_PIXEL_FORMAT_,PIXEL_FORMAT),
		CONCAT(METHOD_NAME_HAVE_CVC_,HAVE_CVC)
	)
)) (
	const ScanlineTool & sct, int x, int y, int w,
	int opacityBeg, int opacity, int opacityEnd
)
{
	if (w>MaxInterpolationBytesAtOnce/CHANNELS) {
		PaintLargeScanlineInt(sct,x,y,w,opacityBeg,opacity,opacityEnd);
		return;
	}

	sct.Interpolate(sct,x,y,w);
	const emByte * s=(const emByte*)sct.InterpolationBuffer;

	__m128i sOb=_mm_cvtsi32_si128(opacityBeg);
	sOb=_mm_insert_epi32(sOb,opacityEnd,1);
	sOb=_mm_insert_epi32(sOb,opacity,2);
#	if HAVE_ALPHA
		sOb=_mm_mullo_epi32(sOb,_mm_set1_epi32(sct.Alpha));
		sOb=_mm_add_epi32(sOb,_mm_set1_epi32(0x800));
		sOb=_mm_add_epi32(sOb,_mm_srli_epi32(sOb,8));
		sOb=_mm_srli_epi32(sOb,8);
#	endif

#	if HAVE_GC1
		__m128i sC1=_mm_cvtsi32_si128(sct.Color1.Get());
		sC1=_mm_shuffle_epi8(sC1,_mm_set_epi8(
			 0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3
		));
		__m256i aC1=_mm256_cvtepu8_epi16(sC1);
#	endif
#	if HAVE_GC2
		__m128i sC2=_mm_cvtsi32_si128(sct.Color2.Get());
		sC2=_mm_shuffle_epi8(sC2,_mm_set_epi8(
			 0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3
		));
		__m256i aC2=_mm256_cvtepu8_epi16(sC2);
#	endif
#	if HAVE_CVC
		__m128i sCv=_mm_cvtsi32_si128(sct.CanvasColor.Get());
		sCv=_mm_shuffle_epi8(sCv,_mm_set_epi8(
			 0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3
		));
		__m256i aCv=_mm256_cvtepu8_epi16(sCv);
#	endif

	emUInt32 * p=(emUInt32*)(
		(char*)sct.Painter.Map+y*(size_t)sct.Painter.BytesPerRow
	) + x;
	emUInt32 * pEnd=p+w;

	__m128i sO=sOb;
	int n=w;
	if (n>4) {
		int r=(p-(emUInt32*)NULL)&3;
		if (!r && opacityBeg==opacity) goto L_2;
		n=4-r;
		sO=_mm_shuffle_epi32(sO,0xe8);
	}

	{
#		if CHANNELS==1
			__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
#		elif CHANNELS==2
			__m128i sC=_mm_insert_epi16(_mm_setzero_si128(),((emUInt16*)s)[0],0);
#		elif CHANNELS==3
			__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
			sC=_mm_insert_epi8(sC,s[1],1);
			sC=_mm_insert_epi8(sC,s[2],2);
#		else
			__m128i sC=_mm_cvtsi32_si128(((emUInt32*)s)[0]);
#		endif

		__m128i sQ=_mm_cvtsi32_si128(p[0]);

		if (n>=2) {
#			if CHANNELS==1
				sC=_mm_insert_epi8(sC,s[1],1);
#			elif CHANNELS==2
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[1],1);
#			elif CHANNELS==3
				sC=_mm_insert_epi8(sC,s[3],4);
				sC=_mm_insert_epi8(sC,s[4],5);
				sC=_mm_insert_epi8(sC,s[5],6);
#			else
				sC=_mm_insert_epi32(sC,((emUInt32*)s)[1],1);
#			endif

			sQ=_mm_insert_epi32(sQ,p[1],1);

			if (n>2) {
#				if CHANNELS==1
					sC=_mm_insert_epi8(sC,s[2],2);
#				elif CHANNELS==2
					sC=_mm_insert_epi16(sC,((emUInt16*)s)[2],2);
#				elif CHANNELS==3
					sC=_mm_insert_epi8(sC,s[6],8);
					sC=_mm_insert_epi8(sC,s[7],9);
					sC=_mm_insert_epi8(sC,s[8],10);
#				else
					sC=_mm_insert_epi32(sC,((emUInt32*)s)[2],2);
#				endif

				sQ=_mm_insert_epi32(sQ,p[2],2);

				sO=_mm_shuffle_epi32(sO,0xd8);

				if (n>3) {
#					if CHANNELS==1
						sC=_mm_insert_epi8(sC,s[3],3);
#					elif CHANNELS==2
						sC=_mm_insert_epi16(sC,((emUInt16*)s)[3],3);
#					elif CHANNELS==3
						sC=_mm_insert_epi8(sC,s[9],12);
						sC=_mm_insert_epi8(sC,s[10],13);
						sC=_mm_insert_epi8(sC,s[11],14);
#					else
						sC=_mm_insert_epi32(sC,((emUInt32*)s)[3],3);
#					endif

					sQ=_mm_insert_epi32(sQ,p[3],3);

					sO=_mm_shuffle_epi32(sO,0x94);
				}
			}
		}

#		if CHANNELS==1
			sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
				-1, 3, 3, 3, -1, 2, 2, 2, -1, 1, 1, 1, -1, 0, 0, 0
			));
#		elif CHANNELS==2
			sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
				 7, 6, 6, 6,  5, 4, 4, 4,  3, 2, 2, 2,  1, 0, 0, 0
			));
#		endif

		__m256i aO=_mm256_broadcastsi128_si256(sO);
		aO=_mm256_shuffle_epi8(aO,_mm256_set_epi8(
#			if HAVE_GC1 || HAVE_GC2
				-1,-1,-1,-1, -1,-1,13,12, -1,-1,-1,-1, -1,-1, 9, 8,
				-1,-1,-1,-1, -1,-1, 5, 4, -1,-1,-1,-1, -1,-1, 1, 0
#			else
				13,12,13,12, 13,12,13,12,  9, 8, 9, 8,  9, 8, 9, 8,
				 5, 4, 5, 4,  5, 4, 5, 4,  1, 0, 1, 0,  1, 0, 1, 0
#			endif
		));

		__m256i aC=_mm256_cvtepu8_epi16(sC);

#		if HAVE_GC1
			__m256i aO1=_mm256_mullo_epi32(_mm256_srli_epi64(aC1,48),aO);
			aO1=_mm256_add_epi32(aO1,_mm256_set1_epi32(0x80));
			aO1=_mm256_add_epi32(aO1,_mm256_srli_epi32(aO1,8));
			aO1=_mm256_shuffle_epi8(aO1,_mm256_set_epi8(
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1,
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1
			));
#			if CHANNELS==2 || CHANNELS==4
				__m256i aA=_mm256_shuffle_epi8(aC,_mm256_set_epi8(
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
				));
#			else
				__m256i aA=_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				);
#			endif
#		endif

#		if HAVE_GC2
			__m256i aO2=_mm256_mullo_epi32(_mm256_srli_epi64(aC2,48),aO);
			aO2=_mm256_add_epi32(aO2,_mm256_set1_epi32(0x80));
			aO2=_mm256_add_epi32(aO2,_mm256_srli_epi32(aO2,8));
			aO2=_mm256_shuffle_epi8(aO2,_mm256_set_epi8(
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1,
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1
			));
#		endif

#		if HAVE_GC1 && HAVE_GC2
			__m256i aD=_mm256_sub_epi16(aA,aC);
			__m256i aA1=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
			__m256i aA2=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
			aA=_mm256_add_epi8(aA1,aA2);
#			if CHANNELS==2 || CHANNELS==4
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				))) goto L_1;
#			endif
			__m256i aP=_mm256_mullo_epi16(aC1,aA1);
			__m256i aP2=_mm256_mullo_epi16(aC2,aA2);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP2=_mm256_add_epi16(aP2,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP2=_mm256_add_epi16(aP2,_mm256_srli_epi16(aP2,8));
			aP=_mm256_add_epi8(aP,aP2);
			aP=_mm256_srli_epi16(aP,8);
#		elif HAVE_GC1
			__m256i aD=_mm256_sub_epi16(aA,aC);
			aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
			if (_mm256_testz_si256(aA,_mm256_set_epi16(
				0,255,255,255,0,255,255,255,
				0,255,255,255,0,255,255,255
			))) goto L_1;
			__m256i aP=_mm256_mullo_epi16(aC1,aA);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP=_mm256_srli_epi16(aP,8);
#		elif HAVE_GC2
			__m256i aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
			if (_mm256_testz_si256(aA,_mm256_set_epi16(
				0,255,255,255,0,255,255,255,
				0,255,255,255,0,255,255,255
			))) goto L_1;
			__m256i aP=_mm256_mullo_epi16(aC2,aA);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP=_mm256_srli_epi16(aP,8);
#		else
#			if CHANNELS==1 || CHANNELS==3
				aC=_mm256_or_si256(aC,_mm256_set_epi16(
					255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0
				));
#			endif
			__m256i aP=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO);
			__m256i aA=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
				-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
				-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
			));
#			if CHANNELS==2 || CHANNELS==4
				if (_mm256_testz_si256(aA,aA)) goto L_1;
#			endif
#		endif

#		if HAVE_CVC
			__m256i aR=_mm256_mullo_epi16(aCv,aA);
			aR=_mm256_add_epi16(aR,_mm256_set1_epi16(0x80));
			aR=_mm256_add_epi16(aR,_mm256_srli_epi16(aR,8));
			aR=_mm256_shuffle_epi8(aR,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#				endif
			));
			aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#				endif
			));
			aP=_mm256_sub_epi32(aP,aR);
			aP=_mm256_permute4x64_epi64(aP,0xe8);
			sQ=_mm_add_epi32(sQ,_mm256_castsi256_si128(aP));
#		else
			__m256i aQ=_mm256_broadcastsi128_si256(sQ);
			aQ=_mm256_shuffle_epi8(aQ,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,14, -1,13,-1,12, -1,-1,-1,10, -1, 9,-1, 8,
					-1,-1,-1, 6, -1, 5,-1, 4, -1,-1,-1, 2, -1, 1,-1, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,12, -1,13,-1,14, -1,-1,-1, 8, -1, 9,-1,10,
					-1,-1,-1, 4, -1, 5,-1, 6, -1,-1,-1, 0, -1, 1,-1, 2
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,15, -1,14,-1,13, -1,-1,-1,11, -1,10,-1, 9,
					-1,-1,-1, 7, -1, 6,-1, 5, -1,-1,-1, 3, -1, 2,-1, 1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,13, -1,14,-1,15, -1,-1,-1, 9, -1,10,-1,11,
					-1,-1,-1, 5, -1, 6,-1, 7, -1,-1,-1, 1, -1, 2,-1, 3
#				endif
			));
			aA=_mm256_sub_epi16(_mm256_set_epi16(
				0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
			),aA);
			aQ=_mm256_mullo_epi16(aQ,aA);
			aQ=_mm256_add_epi16(aQ,_mm256_set1_epi16(0x80));
			aQ=_mm256_add_epi16(aQ,_mm256_srli_epi16(aQ,8));
			aQ=_mm256_srli_epi16(aQ,8);
			aP=_mm256_add_epi8(aP,aQ);
			aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#				endif
			));
			aP=_mm256_permute4x64_epi64(aP,0xe8);
			sQ=_mm256_castsi256_si128(aP);
#		endif

		p[0]=_mm_cvtsi128_si32(sQ);
		if (n>=2) {
			p[1]=_mm_extract_epi32(sQ,1);
			if (n>2) {
				p[2]=_mm_extract_epi32(sQ,2);
				if (n>3) {
					p[3]=_mm_extract_epi32(sQ,3);
				}
			}
		}
	}
#if CHANNELS==2 || CHANNELS==4 || HAVE_GC1!=HAVE_GC2
L_1:
#endif
	s+=CHANNELS*n;
	p+=n;
	if (p>=pEnd) return;

L_2:
	sOb=_mm_shuffle_epi32(sOb,0xf6);
	emUInt32 * pEnd4=pEnd-3;
	if (opacityEnd!=opacity) pEnd4--;
	if (p>=pEnd4) goto L_3;

#	if !HAVE_ALPHA
	if (
		opacity < 0x1000
#		if HAVE_GC1
			|| !sct.Color1.IsOpaque()
#		endif
#		if HAVE_GC2
			|| !sct.Color2.IsOpaque()
#		endif
	)
#	endif
	{
#		if HAVE_GC1 && HAVE_GC2
			__m128i sO1=_mm_blend_epi16(
				_mm256_castsi256_si128(aC1),
				_mm256_castsi256_si128(aC2),
				0xf0
			);
			sO1=_mm_srli_epi64(sO1,48);
			sO1=_mm_mullo_epi32(sO1,_mm_broadcastd_epi32(sOb));
			sO1=_mm_add_epi32(sO1,_mm_set1_epi32(0x80));
			sO1=_mm_add_epi32(sO1,_mm_srli_epi32(sO1,8));
			__m256i aO1=_mm256_broadcastw_epi16(_mm_srli_si128(sO1,1));
			__m256i aO2=_mm256_broadcastw_epi16(_mm_srli_si128(sO1,9));
#		elif HAVE_GC1
			__m128i sO1=_mm_srli_epi64(_mm256_castsi256_si128(aC1),48);
			sO1=_mm_mullo_epi32(sO1,sOb);
			sO1=_mm_add_epi32(sO1,_mm_set1_epi32(0x80));
			sO1=_mm_add_epi32(sO1,_mm_srli_epi32(sO1,8));
			sO1=_mm_srli_epi32(sO1,8);
			__m256i aO1=_mm256_broadcastw_epi16(sO1);
#		elif HAVE_GC2
			__m128i sO2=_mm_srli_epi64(_mm256_castsi256_si128(aC2),48);
			sO2=_mm_mullo_epi32(sO2,sOb);
			sO2=_mm_add_epi32(sO2,_mm_set1_epi32(0x80));
			sO2=_mm_add_epi32(sO2,_mm_srli_epi32(sO2,8));
			sO2=_mm_srli_epi32(sO2,8);
			__m256i aO2=_mm256_broadcastw_epi16(sO2);
#		else
			__m256i aO=_mm256_broadcastw_epi16(sOb);
#		endif

		do {
#			if CHANNELS==1
				__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
				sC=_mm_insert_epi8(sC,s[1],1);
				sC=_mm_insert_epi8(sC,s[2],2);
				sC=_mm_insert_epi8(sC,s[3],3);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					-1, 3, 3, 3, -1, 2, 2, 2, -1, 1, 1, 1, -1, 0, 0, 0
				));
#			elif CHANNELS==2
				__m128i sC=_mm_insert_epi16(_mm_setzero_si128(),((emUInt16*)s)[0],0);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[1],1);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[2],2);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[3],3);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					 7, 6, 6, 6,  5, 4, 4, 4,  3, 2, 2, 2,  1, 0, 0, 0
				));
#			elif CHANNELS==3
				__m128i sC=_mm_loadu_si128((__m128i*)s);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					-1,11,10, 9, -1, 8, 7, 6, -1, 5, 4, 3, -1, 2, 1, 0
				));
#			else
				__m128i sC=_mm_loadu_si128((__m128i*)s);
#			endif

			__m256i aC=_mm256_cvtepu8_epi16(sC);

#			if HAVE_GC1
#				if CHANNELS==2 || CHANNELS==4
					__m256i aA=_mm256_shuffle_epi8(aC,_mm256_set_epi8(
						-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
						-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
					));
#				else
					__m256i aA=_mm256_set_epi16(
						0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
					);
#				endif
#			endif

#			if HAVE_GC1 && HAVE_GC2
				__m256i aD=_mm256_sub_epi16(aA,aC);
				__m256i aA1=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
				__m256i aA2=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
				aA=_mm256_add_epi8(aA1,aA2);
#				if CHANNELS==2 || CHANNELS==4
					if (_mm256_testz_si256(aA,_mm256_set_epi16(
						0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
					))) continue;
#				endif
				__m256i aP=_mm256_mullo_epi16(aC1,aA1);
				__m256i aP2=_mm256_mullo_epi16(aC2,aA2);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP2=_mm256_add_epi16(aP2,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
				aP2=_mm256_add_epi16(aP2,_mm256_srli_epi16(aP2,8));
				aP=_mm256_add_epi8(aP,aP2);
				aP=_mm256_srli_epi16(aP,8);
#			elif HAVE_GC1
				__m256i aD=_mm256_sub_epi16(aA,aC);
				aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,
					0,255,255,255,0,255,255,255
				))) continue;
				__m256i aP=_mm256_mullo_epi16(aC1,aA);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
				aP=_mm256_srli_epi16(aP,8);
#			elif HAVE_GC2
				__m256i aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,
					0,255,255,255,0,255,255,255
				))) continue;
				__m256i aP=_mm256_mullo_epi16(aC2,aA);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
				aP=_mm256_srli_epi16(aP,8);
#			else
#				if CHANNELS==1 || CHANNELS==3
					aC=_mm256_or_si256(aC,_mm256_set_epi16(
						255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0
					));
#				endif
				__m256i aP=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO);
				__m256i aA=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
				));
#				if CHANNELS==2 || CHANNELS==4
					if (_mm256_testz_si256(aA,aA)) continue;
#				endif
#			endif

#			if HAVE_CVC
				__m256i aR=_mm256_mullo_epi16(aCv,aA);
				aR=_mm256_add_epi16(aR,_mm256_set1_epi16(0x80));
				aR=_mm256_add_epi16(aR,_mm256_srli_epi16(aR,8));
				aR=_mm256_shuffle_epi8(aR,_mm256_set_epi8(
#					if PIXEL_FORMAT==PF_8888_0BGR
						-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
						-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#					elif PIXEL_FORMAT==PF_8888_0RGB
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#					elif PIXEL_FORMAT==PF_8888_BGR0
						-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
						-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#					elif PIXEL_FORMAT==PF_8888_RGB0
						-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
						-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#					endif
				));
				aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#					if PIXEL_FORMAT==PF_8888_0BGR
						-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
						-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#					elif PIXEL_FORMAT==PF_8888_0RGB
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#					elif PIXEL_FORMAT==PF_8888_BGR0
						-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
						-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#					elif PIXEL_FORMAT==PF_8888_RGB0
						-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
						-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#					endif
				));
				aP=_mm256_sub_epi32(aP,aR);
				aP=_mm256_permute4x64_epi64(aP,0xe8);
				__m128i sQ=_mm_load_si128((__m128i*)p);
				sQ=_mm_add_epi32(sQ,_mm256_castsi256_si128(aP));
				_mm_store_si128((__m128i*)p,sQ);
#			else
				__m256i aQ=_mm256_broadcastsi128_si256(_mm_load_si128((__m128i*)p));
				aQ=_mm256_shuffle_epi8(aQ,_mm256_set_epi8(
#					if PIXEL_FORMAT==PF_8888_0BGR
						-1,-1,-1,14, -1,13,-1,12, -1,-1,-1,10, -1, 9,-1, 8,
						-1,-1,-1, 6, -1, 5,-1, 4, -1,-1,-1, 2, -1, 1,-1, 0
#					elif PIXEL_FORMAT==PF_8888_0RGB
						-1,-1,-1,12, -1,13,-1,14, -1,-1,-1, 8, -1, 9,-1,10,
						-1,-1,-1, 4, -1, 5,-1, 6, -1,-1,-1, 0, -1, 1,-1, 2
#					elif PIXEL_FORMAT==PF_8888_BGR0
						-1,-1,-1,15, -1,14,-1,13, -1,-1,-1,11, -1,10,-1, 9,
						-1,-1,-1, 7, -1, 6,-1, 5, -1,-1,-1, 3, -1, 2,-1, 1
#					elif PIXEL_FORMAT==PF_8888_RGB0
						-1,-1,-1,13, -1,14,-1,15, -1,-1,-1, 9, -1,10,-1,11,
						-1,-1,-1, 5, -1, 6,-1, 7, -1,-1,-1, 1, -1, 2,-1, 3
#					endif
				));
				aA=_mm256_sub_epi16(_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				),aA);
				aQ=_mm256_mullo_epi16(aQ,aA);
				aQ=_mm256_add_epi16(aQ,_mm256_set1_epi16(0x80));
				aQ=_mm256_add_epi16(aQ,_mm256_srli_epi16(aQ,8));
				aQ=_mm256_srli_epi16(aQ,8);
				aP=_mm256_add_epi8(aP,aQ);
				aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#					if PIXEL_FORMAT==PF_8888_0BGR
						-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
						-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#					elif PIXEL_FORMAT==PF_8888_0RGB
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
						-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#					elif PIXEL_FORMAT==PF_8888_BGR0
						-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
						-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#					elif PIXEL_FORMAT==PF_8888_RGB0
						-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
						-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#					endif
				));
				aP=_mm256_permute4x64_epi64(aP,0xe8);
				_mm_store_si128((__m128i*)p,_mm256_castsi256_si128(aP));
#			endif
		} while (s+=CHANNELS*4, p+=4, p<pEnd4);
	}
#	if !HAVE_ALPHA
	else {
		do {
#			if CHANNELS==3 && !HAVE_GC1 && !HAVE_GC2
				emUInt32 * pEnd16=pEnd4-12;
				if (p<pEnd16 && (((char*)p-(char*)NULL)&31)==0) {
					do {
						__m256i aC1=_mm256_broadcastsi128_si256(_mm_loadu_si128((__m128i*)s+0));
						__m256i aCm=_mm256_broadcastsi128_si256(_mm_loadu_si128((__m128i*)s+1));
						__m256i aC2=_mm256_broadcastsi128_si256(_mm_loadu_si128((__m128i*)s+2));

						aC1=_mm256_blend_epi32(aC1,aCm,0x30);
						aC2=_mm256_blend_epi32(aC2,aCm,0x0c);

						aC1=_mm256_shuffle_epi8(aC1,_mm256_set_epi8(
#							if PIXEL_FORMAT==PF_8888_0BGR
								-1, 7, 6, 5, -1, 4, 3, 2, -1, 1, 0,15, -1,14,13,12,
								-1,11,10, 9, -1, 8, 7, 6, -1, 5, 4, 3, -1, 2, 1, 0
#							elif PIXEL_FORMAT==PF_8888_0RGB
								-1, 5, 6, 7, -1, 2, 3, 4, -1,15, 0, 1, -1,12,13,14,
								-1, 9,10,11, -1, 6, 7, 8, -1, 3, 4, 5, -1, 0, 1, 2
#							elif PIXEL_FORMAT==PF_8888_BGR0
								 7, 6, 5,-1,  4, 3, 2,-1,  1, 0,15,-1, 14,13,12,-1,
								11,10, 9,-1,  8, 7, 6,-1,  5, 4, 3,-1,  2, 1, 0,-1
#							elif PIXEL_FORMAT==PF_8888_RGB0
								 5, 6, 7,-1,  2, 3, 4,-1, 15, 0, 1,-1, 12,13,14,-1,
								 9,10,11,-1,  6, 7, 8,-1,  3, 4, 5,-1,  0, 1, 2,-1
#							endif
						));
						aC2=_mm256_shuffle_epi8(aC2,_mm256_set_epi8(
#							if PIXEL_FORMAT==PF_8888_0BGR
								-1,15,14,13, -1,12,11,10, -1, 9, 8, 7, -1, 6, 5, 4,
								-1, 3, 2, 1, -1, 0,15,14, -1,13,12,11, -1,10, 9, 8
#							elif PIXEL_FORMAT==PF_8888_0RGB
								-1,13,14,15, -1,10,11,12, -1, 7, 8, 9, -1, 4, 5, 6,
								-1, 1, 2, 3, -1,14,15, 0, -1,11,12,13, -1, 8, 9,10
#							elif PIXEL_FORMAT==PF_8888_BGR0
								15,14,13,-1, 12,11,10,-1,  9, 8, 7,-1,  6, 5, 4,-1,
								 3, 2, 1,-1,  0,15,14,-1, 13,12,11,-1, 10, 9, 8,-1
#							elif PIXEL_FORMAT==PF_8888_RGB0
								13,14,15,-1, 10,11,12,-1,  7, 8, 9,-1,  4, 5, 6,-1,
								 1, 2, 3,-1, 14,15, 0,-1, 11,12,13,-1,  8, 9,10,-1
#							endif
						));

						_mm256_store_si256((__m256i*)p, aC1);
						_mm256_store_si256((__m256i*)p+1, aC2);
					} while (s+=CHANNELS*16, p+=16, p<pEnd16);
					if (p>=pEnd4) break;
				}
#			endif

#			if CHANNELS==1
				__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
				sC=_mm_insert_epi8(sC,s[1],1);
				sC=_mm_insert_epi8(sC,s[2],2);
				sC=_mm_insert_epi8(sC,s[3],3);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					-1, 3, 3, 3, -1, 2, 2, 2, -1, 1, 1, 1, -1, 0, 0, 0
				));
#			elif CHANNELS==2
				__m128i sC=_mm_insert_epi16(_mm_setzero_si128(),((emUInt16*)s)[0],0);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[1],1);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[2],2);
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[3],3);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					 7, 6, 6, 6,  5, 4, 4, 4,  3, 2, 2, 2,  1, 0, 0, 0
				));
#			elif CHANNELS==3
				__m128i sC=_mm_loadu_si128((__m128i*)s);
				sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
					-1,11,10, 9, -1, 8, 7, 6, -1, 5, 4, 3, -1, 2, 1, 0
				));
#			else
				__m128i sC=_mm_loadu_si128((__m128i*)s);
#			endif

			__m256i aC=_mm256_cvtepu8_epi16(sC);

#			if HAVE_GC1 || !HAVE_GC2
#				if CHANNELS==2 || CHANNELS==4
					__m256i aA=_mm256_shuffle_epi8(aC,_mm256_set_epi8(
						-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
						-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
					));
#					if (HAVE_GC1 && HAVE_GC2) || (!HAVE_GC1 && !HAVE_GC2)
						if (_mm256_testz_si256(aA,aA)) continue;
#					endif
#				elif HAVE_GC1
					__m256i aA=_mm256_set_epi16(
						0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
					);
#				endif
#			endif

#			if HAVE_GC1 && HAVE_GC2
				__m256i aP=_mm256_mullo_epi16(aC1,_mm256_sub_epi16(aA,aC));
				__m256i aP2=_mm256_mullo_epi16(aC2,aC);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP2=_mm256_add_epi16(aP2,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
				aP2=_mm256_add_epi16(aP2,_mm256_srli_epi16(aP2,8));
				aP=_mm256_add_epi8(aP,aP2);
#			elif HAVE_GC1
				aA=_mm256_sub_epi16(aA,aC);
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				))) continue;
				__m256i aP=_mm256_mullo_epi16(aC1,aA);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
#			elif HAVE_GC2
				__m256i aA=aC;
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				))) continue;
				__m256i aP=_mm256_mullo_epi16(aC2,aA);
				aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
				aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
#			else
				__m256i aP=_mm256_slli_epi16(aC,8);
#			endif

#			if (HAVE_GC1 && !HAVE_GC2) || (!HAVE_GC1 && HAVE_GC2) || CHANNELS==2 || CHANNELS==4
				if (!_mm256_testc_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				))) {
#					if HAVE_CVC
						__m256i aR=_mm256_mullo_epi16(aCv,aA);
						aR=_mm256_add_epi16(aR,_mm256_set1_epi16(0x80));
						aR=_mm256_add_epi16(aR,_mm256_srli_epi16(aR,8));
						aR=_mm256_shuffle_epi8(aR,_mm256_set_epi8(
#							if PIXEL_FORMAT==PF_8888_0BGR
								-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
								-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#							elif PIXEL_FORMAT==PF_8888_0RGB
								-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
								-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#							elif PIXEL_FORMAT==PF_8888_BGR0
								-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
								-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#							elif PIXEL_FORMAT==PF_8888_RGB0
								-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
								-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#							endif
						));
						aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#							if PIXEL_FORMAT==PF_8888_0BGR
								-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
								-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#							elif PIXEL_FORMAT==PF_8888_0RGB
								-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
								-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#							elif PIXEL_FORMAT==PF_8888_BGR0
								-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
								-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#							elif PIXEL_FORMAT==PF_8888_RGB0
								-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
								-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#							endif
						));
						aP=_mm256_sub_epi32(aP,aR);
						aP=_mm256_permute4x64_epi64(aP,0xe8);
						__m128i sQ=_mm_load_si128((__m128i*)p);
						sQ=_mm_add_epi32(sQ,_mm256_castsi256_si128(aP));
						_mm_store_si128((__m128i*)p,sQ);
						continue;
#					else
						__m256i aQ=_mm256_broadcastsi128_si256(_mm_load_si128((__m128i*)p));
						aQ=_mm256_shuffle_epi8(aQ,_mm256_set_epi8(
#							if PIXEL_FORMAT==PF_8888_0BGR
								-1,-1,-1,14, -1,13,-1,12, -1,-1,-1,10, -1, 9,-1, 8,
								-1,-1,-1, 6, -1, 5,-1, 4, -1,-1,-1, 2, -1, 1,-1, 0
#							elif PIXEL_FORMAT==PF_8888_0RGB
								-1,-1,-1,12, -1,13,-1,14, -1,-1,-1, 8, -1, 9,-1,10,
								-1,-1,-1, 4, -1, 5,-1, 6, -1,-1,-1, 0, -1, 1,-1, 2
#							elif PIXEL_FORMAT==PF_8888_BGR0
								-1,-1,-1,15, -1,14,-1,13, -1,-1,-1,11, -1,10,-1, 9,
								-1,-1,-1, 7, -1, 6,-1, 5, -1,-1,-1, 3, -1, 2,-1, 1
#							elif PIXEL_FORMAT==PF_8888_RGB0
								-1,-1,-1,13, -1,14,-1,15, -1,-1,-1, 9, -1,10,-1,11,
								-1,-1,-1, 5, -1, 6,-1, 7, -1,-1,-1, 1, -1, 2,-1, 3
#							endif
						));
						aA=_mm256_sub_epi16(_mm256_set_epi16(
							0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
						),aA);
						aQ=_mm256_mullo_epi16(aQ,aA);
						aQ=_mm256_add_epi16(aQ,_mm256_set1_epi16(0x80));
						aQ=_mm256_add_epi16(aQ,_mm256_srli_epi16(aQ,8));
						aP=_mm256_add_epi8(aP,aQ);
#					endif
				}
#			endif
			aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#				endif
			));
			aP=_mm256_permute4x64_epi64(aP,0xe8);
			_mm_store_si128((__m128i*)p,_mm256_castsi256_si128(aP));

		} while (s+=CHANNELS*4, p+=4, p<pEnd4);
	}
#	endif

	if (p>=pEnd) return;

L_3:
	{
#		if CHANNELS==1
			__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
#		elif CHANNELS==2
			__m128i sC=_mm_insert_epi16(_mm_setzero_si128(),((emUInt16*)s)[0],0);
#		elif CHANNELS==3
			__m128i sC=_mm_insert_epi8(_mm_setzero_si128(),s[0],0);
			sC=_mm_insert_epi8(sC,s[1],1);
			sC=_mm_insert_epi8(sC,s[2],2);
#		else
			__m128i sC=_mm_cvtsi32_si128(((emUInt32*)s)[0]);
#		endif

		__m128i sQ=_mm_cvtsi32_si128(p[0]);

		sO=_mm_shufflelo_epi16(sOb,0x0a);

		n=pEnd-p;
		if (n>=2) {
#			if CHANNELS==1
				sC=_mm_insert_epi8(sC,s[1],1);
#			elif CHANNELS==2
				sC=_mm_insert_epi16(sC,((emUInt16*)s)[1],1);
#			elif CHANNELS==3
				sC=_mm_insert_epi8(sC,s[3],4);
				sC=_mm_insert_epi8(sC,s[4],5);
				sC=_mm_insert_epi8(sC,s[5],6);
#			else
				sC=_mm_insert_epi32(sC,((emUInt32*)s)[1],1);
#			endif

			sQ=_mm_insert_epi32(sQ,p[1],1);

			sO=_mm_shuffle_epi32(sO,0xe1);

			if (n>2) {
#				if CHANNELS==1
					sC=_mm_insert_epi8(sC,s[2],2);
#				elif CHANNELS==2
					sC=_mm_insert_epi16(sC,((emUInt16*)s)[2],2);
#				elif CHANNELS==3
					sC=_mm_insert_epi8(sC,s[6],8);
					sC=_mm_insert_epi8(sC,s[7],9);
					sC=_mm_insert_epi8(sC,s[8],10);
#				else
					sC=_mm_insert_epi32(sC,((emUInt32*)s)[2],2);
#				endif

				sQ=_mm_insert_epi32(sQ,p[2],2);

				sO=_mm_shuffle_epi32(sO,0xd0);

				if (n>3) {
#					if CHANNELS==1
						sC=_mm_insert_epi8(sC,s[3],3);
#					elif CHANNELS==2
						sC=_mm_insert_epi16(sC,((emUInt16*)s)[3],3);
#					elif CHANNELS==3
						sC=_mm_insert_epi8(sC,s[9],12);
						sC=_mm_insert_epi8(sC,s[10],13);
						sC=_mm_insert_epi8(sC,s[11],14);
#					else
						sC=_mm_insert_epi32(sC,((emUInt32*)s)[3],3);
#					endif

					sQ=_mm_insert_epi32(sQ,p[3],3);

					sO=_mm_shuffle_epi32(sO,0x80);
				}
			}
		}

#		if CHANNELS==1
			sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
				-1, 3, 3, 3, -1, 2, 2, 2, -1, 1, 1, 1, -1, 0, 0, 0
			));
#		elif CHANNELS==2
			sC=_mm_shuffle_epi8(sC,_mm_set_epi8(
				 7, 6, 6, 6,  5, 4, 4, 4,  3, 2, 2, 2,  1, 0, 0, 0
			));
#		endif

		__m256i aO=_mm256_cvtepu32_epi64(sO);
#		if HAVE_GC1 || HAVE_GC2
			aO=_mm256_srli_epi32(aO,16);
#		else
			aO=_mm256_shuffle_epi32(aO,0xa0);
#		endif

		__m256i aC=_mm256_cvtepu8_epi16(sC);

#		if HAVE_GC1
			__m256i aO1=_mm256_mullo_epi32(_mm256_srli_epi64(aC1,48),aO);
			aO1=_mm256_add_epi32(aO1,_mm256_set1_epi32(0x80));
			aO1=_mm256_add_epi32(aO1,_mm256_srli_epi32(aO1,8));
			aO1=_mm256_shuffle_epi8(aO1,_mm256_set_epi8(
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1,
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1
			));
#			if CHANNELS==2 || CHANNELS==4
				__m256i aA=_mm256_shuffle_epi8(aC,_mm256_set_epi8(
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
					-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
				));
#			else
				__m256i aA=_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				);
#			endif
#		endif

#		if HAVE_GC2
			__m256i aO2=_mm256_mullo_epi32(_mm256_srli_epi64(aC2,48),aO);
			aO2=_mm256_add_epi32(aO2,_mm256_set1_epi32(0x80));
			aO2=_mm256_add_epi32(aO2,_mm256_srli_epi32(aO2,8));
			aO2=_mm256_shuffle_epi8(aO2,_mm256_set_epi8(
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1,
				10, 9,10, 9, 10, 9,10, 9,  2, 1, 2, 1,  2, 1, 2, 1
			));
#		endif

#		if HAVE_GC1 && HAVE_GC2
			__m256i aD=_mm256_sub_epi16(aA,aC);
			__m256i aA1=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
			__m256i aA2=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
			aA=_mm256_add_epi8(aA1,aA2);
#			if CHANNELS==2 || CHANNELS==4
				if (_mm256_testz_si256(aA,_mm256_set_epi16(
					0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
				))) return;
#			endif
			__m256i aP=_mm256_mullo_epi16(aC1,aA1);
			__m256i aP2=_mm256_mullo_epi16(aC2,aA2);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP2=_mm256_add_epi16(aP2,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP2=_mm256_add_epi16(aP2,_mm256_srli_epi16(aP2,8));
			aP=_mm256_add_epi8(aP,aP2);
			aP=_mm256_srli_epi16(aP,8);
#		elif HAVE_GC1
			__m256i aD=_mm256_sub_epi16(aA,aC);
			aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aD,3),aO1);
			if (_mm256_testz_si256(aA,_mm256_set_epi16(
				0,255,255,255,0,255,255,255,
				0,255,255,255,0,255,255,255
			))) return;
			__m256i aP=_mm256_mullo_epi16(aC1,aA);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP=_mm256_srli_epi16(aP,8);
#		elif HAVE_GC2
			__m256i aA=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO2);
			if (_mm256_testz_si256(aA,_mm256_set_epi16(
				0,255,255,255,0,255,255,255,
				0,255,255,255,0,255,255,255
			))) return;
			__m256i aP=_mm256_mullo_epi16(aC2,aA);
			aP=_mm256_add_epi16(aP,_mm256_set1_epi16(0x80));
			aP=_mm256_add_epi16(aP,_mm256_srli_epi16(aP,8));
			aP=_mm256_srli_epi16(aP,8);
#		else
#			if CHANNELS==1 || CHANNELS==3
				aC=_mm256_or_si256(aC,_mm256_set_epi16(
					255,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0
				));
#			endif
			__m256i aP=_mm256_mulhrs_epi16(_mm256_slli_epi16(aC,3),aO);
			__m256i aA=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
				-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6,
				-1,-1,-1,14, -1,14,-1,14, -1,-1,-1, 6, -1, 6,-1, 6
			));
#			if CHANNELS==2 || CHANNELS==4
				if (_mm256_testz_si256(aA,aA)) return;
#			endif
#		endif

#		if HAVE_CVC
			__m256i aR=_mm256_mullo_epi16(aCv,aA);
			aR=_mm256_add_epi16(aR,_mm256_set1_epi16(0x80));
			aR=_mm256_add_epi16(aR,_mm256_srli_epi16(aR,8));
			aR=_mm256_shuffle_epi8(aR,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,13,11, 9, -1, 5, 3, 1
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 9,11,13, -1, 1, 3, 5
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 13,11, 9,-1,  5, 3, 1,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  9,11,13,-1,  1, 3, 5,-1
#				endif
			));
			aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#				endif
			));
			aP=_mm256_sub_epi32(aP,aR);
			aP=_mm256_permute4x64_epi64(aP,0xe8);
			sQ=_mm_add_epi32(sQ,_mm256_castsi256_si128(aP));
#		else
			__m256i aQ=_mm256_broadcastsi128_si256(sQ);
			aQ=_mm256_shuffle_epi8(aQ,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,14, -1,13,-1,12, -1,-1,-1,10, -1, 9,-1, 8,
					-1,-1,-1, 6, -1, 5,-1, 4, -1,-1,-1, 2, -1, 1,-1, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,12, -1,13,-1,14, -1,-1,-1, 8, -1, 9,-1,10,
					-1,-1,-1, 4, -1, 5,-1, 6, -1,-1,-1, 0, -1, 1,-1, 2
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,15, -1,14,-1,13, -1,-1,-1,11, -1,10,-1, 9,
					-1,-1,-1, 7, -1, 6,-1, 5, -1,-1,-1, 3, -1, 2,-1, 1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,13, -1,14,-1,15, -1,-1,-1, 9, -1,10,-1,11,
					-1,-1,-1, 5, -1, 6,-1, 7, -1,-1,-1, 1, -1, 2,-1, 3
#				endif
			));
			aA=_mm256_sub_epi16(_mm256_set_epi16(
				0,255,255,255,0,255,255,255,0,255,255,255,0,255,255,255
			),aA);
			aQ=_mm256_mullo_epi16(aQ,aA);
			aQ=_mm256_add_epi16(aQ,_mm256_set1_epi16(0x80));
			aQ=_mm256_add_epi16(aQ,_mm256_srli_epi16(aQ,8));
			aQ=_mm256_srli_epi16(aQ,8);
			aP=_mm256_add_epi8(aP,aQ);
			aP=_mm256_shuffle_epi8(aP,_mm256_set_epi8(
#				if PIXEL_FORMAT==PF_8888_0BGR
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0,
					-1,-1,-1,-1, -1,-1,-1,-1, -1,12,10, 8, -1, 4, 2, 0
#				elif PIXEL_FORMAT==PF_8888_0RGB
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4,
					-1,-1,-1,-1, -1,-1,-1,-1, -1, 8,10,12, -1, 0, 2, 4
#				elif PIXEL_FORMAT==PF_8888_BGR0
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1,
					-1,-1,-1,-1, -1,-1,-1,-1, 12,10, 8,-1,  4, 2, 0,-1
#				elif PIXEL_FORMAT==PF_8888_RGB0
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1,
					-1,-1,-1,-1, -1,-1,-1,-1,  8,10,12,-1,  0, 2, 4,-1
#				endif
			));
			aP=_mm256_permute4x64_epi64(aP,0xe8);
			sQ=_mm256_castsi256_si128(aP);
#		endif

		p[0]=_mm_cvtsi128_si32(sQ);
		if (n>=2) {
			p[1]=_mm_extract_epi32(sQ,1);
			if (n>2) {
				p[2]=_mm_extract_epi32(sQ,2);
				if (n>3) {
					p[3]=_mm_extract_epi32(sQ,3);
				}
			}
		}
	}
}


#endif
