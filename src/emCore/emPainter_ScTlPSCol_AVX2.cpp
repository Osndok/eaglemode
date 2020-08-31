//------------------------------------------------------------------------------
// emPainter_ScTlPSCol_AVX2.cpp
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
// algorithms for emPainter::ScanlineTool::PaintScanlineColAvx2..(..) with
// different settings. The preprocessor defines for these settings are:
//   PIXEL_FORMAT: 0 ... 3     - See OptimizedPixelFormatIndex.
//   HAVE_CVC:   0 or 1        - Whether canvas color is given (opaque).
//------------------------------------------------------------------------------

#if !defined(PIXEL_FORMAT)
//==============================================================================
//==================== Top level include / Set PIXEL_FORMAT ====================
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
#	define METHOD_NAME_PIXEL_FORMAT_0 Pf0
#	define METHOD_NAME_PIXEL_FORMAT_1 Pf1
#	define METHOD_NAME_PIXEL_FORMAT_2 Pf2
#	define METHOD_NAME_PIXEL_FORMAT_3 Pf3
#	define METHOD_NAME_HAVE_CVC_0
#	define METHOD_NAME_HAVE_CVC_1 Cv

#	define PIXEL_FORMAT PF_8888_0BGR
#	include "emPainter_ScTlPSCol_AVX2.cpp"
#	undef PIXEL_FORMAT

#	define PIXEL_FORMAT PF_8888_0RGB
#	include "emPainter_ScTlPSCol_AVX2.cpp"
#	undef PIXEL_FORMAT

#	define PIXEL_FORMAT PF_8888_BGR0
#	include "emPainter_ScTlPSCol_AVX2.cpp"
#	undef PIXEL_FORMAT

#	define PIXEL_FORMAT PF_8888_RGB0
#	include "emPainter_ScTlPSCol_AVX2.cpp"
#	undef PIXEL_FORMAT

#endif


#elif !defined(HAVE_CVC)
//==============================================================================
//================================ Set HAVE_CVC ================================
//==============================================================================

#define HAVE_CVC 0
#include "emPainter_ScTlPSCol_AVX2.cpp"
#undef HAVE_CVC

#define HAVE_CVC 1
#include "emPainter_ScTlPSCol_AVX2.cpp"
#undef HAVE_CVC


#else
//==============================================================================
//============== emPainter::ScanlineTool::PaintScanlineColAvx2... ==============
//==============================================================================

#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emPainter::ScanlineTool::CONCAT(PaintScanlineColAvx2,CONCAT(
	CONCAT(METHOD_NAME_PIXEL_FORMAT_,PIXEL_FORMAT),
	CONCAT(METHOD_NAME_HAVE_CVC_,HAVE_CVC)
)) (
	const ScanlineTool & sct, int x, int y, int w,
	int opacityBeg, int opacity, int opacityEnd
)
{
	emUInt32 * p=(emUInt32*)(
		(char*)sct.Painter.Map+y*(size_t)sct.Painter.BytesPerRow
	) + x;

	__m128i sCvc1=_mm_cvtsi32_si128(sct.Color1.Get());
#	if HAVE_CVC
		sCvc1=_mm_insert_epi32(sCvc1,sct.CanvasColor.Get(),1);
#	endif
	sCvc1=_mm_shuffle_epi8(sCvc1,_mm_set_epi8(
#		if PIXEL_FORMAT==PF_8888_0BGR
			-1,-1,-1, 5, -1, 6,-1, 7, -1,-1,-1, 1, -1, 2,-1, 3
#		elif PIXEL_FORMAT==PF_8888_0RGB
			-1,-1,-1, 7, -1, 6,-1, 5, -1,-1,-1, 3, -1, 2,-1, 1
#		elif PIXEL_FORMAT==PF_8888_BGR0
			-1, 5,-1, 6, -1, 7,-1,-1, -1, 1,-1, 2, -1, 3,-1,-1
#		elif PIXEL_FORMAT==PF_8888_RGB0
			-1, 7,-1, 6, -1, 5,-1,-1, -1, 3,-1, 2, -1, 1,-1,-1
#		endif
	));

	unsigned alpha=(sct.Color1.GetAlpha()*opacityBeg+0x800)>>12;
	if (alpha>=255) {
		__m128i sC=_mm_packus_epi16(sCvc1,sCvc1);
		*p=_mm_cvtsi128_si32(sC);
	}
	else {
#		if HAVE_CVC
			__m128i sC=_mm_mullo_epi16(sCvc1,_mm_set1_epi16(alpha));
			sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
			sC=_mm_add_epi16(sC,_mm_srli_epi16(sC,8));
			__m128i sC2=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x0f0d0b09));
			sC=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x07050301));
			sC=_mm_sub_epi32(sC,sC2);
			*p+=_mm_cvtsi128_si32(sC);
#		else
			__m128i sC=_mm_cvtsi32_si128(*p);
			sC=_mm_cvtepu8_epi16(sC);
			sC=_mm_unpacklo_epi16(sCvc1,sC);
			sC=_mm_madd_epi16(sC,_mm_set1_epi32(alpha+((255-alpha)<<16)));
			sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
			sC=_mm_add_epi16(sC,_mm_srli_epi16(sC,8));
			sC=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x0d090501));
			*p=_mm_cvtsi128_si32(sC);
#		endif
	}

	w-=2;
	if (w>=0) {
		p++;
		if (w>0) {
			alpha=(sct.Color1.GetAlpha()*opacity+0x800)>>12;
			if (alpha>=255) {
				__m128i sC=_mm_packus_epi16(sCvc1,sCvc1);
				emUInt32 * e1=p+w-1;
				if (p<e1) {
					if (((char*)p-(char*)NULL)&7) {
						*p=_mm_cvtsi128_si32(sC);
						p++;
						if (p>=e1) goto L_CP1A;
					}
					sC=_mm_broadcastd_epi32(sC);
					emUInt32 * e3=e1-2;
					if (p<e3) {
						if (((char*)p-(char*)NULL)&15) {
							_mm_storel_epi64((__m128i*)p,sC);
							p+=2;
							if (p>=e3) goto L_CP2A;
						}
						emUInt32 * e7=e3-4;
						if (p<e7) {
							if (((char*)p-(char*)NULL)&31) {
								_mm_store_si128((__m128i*)p,sC);
								p+=4;
								if (p>=e7) goto L_CP3A;
							}
							__m256i aC=_mm256_broadcastsi128_si256(sC);
							emUInt32 * e15=e7-8;
							if (p<e15) {
								emUInt32 * e31=e15-16;
								if (p<e31) {
									do {
										_mm256_store_si256((__m256i*)p,aC);
										_mm256_store_si256((__m256i*)p+1,aC);
										_mm256_store_si256((__m256i*)p+2,aC);
										_mm256_store_si256((__m256i*)p+3,aC);
										p+=32;
									} while (p<e31);
									if (p>=e15) goto L_CP5B;
								}
								_mm256_store_si256((__m256i*)p,aC);
								_mm256_store_si256((__m256i*)p+1,aC);
								p+=16;
L_CP5B:
								if (p>=e7) goto L_CP4B;
							}
							_mm256_store_si256((__m256i*)p,aC);
							p+=8;
L_CP4B:
							if (p>=e3) goto L_CP3B;
						}
L_CP3A:
						_mm_store_si128((__m128i*)p,sC);
						p+=4;
L_CP3B:
						if (p>=e1) goto L_CP2B;
					}
L_CP2A:
					_mm_storel_epi64((__m128i*)p,sC);
					p+=2;
L_CP2B:
					if (p>e1) goto L_CP1B;
				}
L_CP1A:
				*p=_mm_cvtsi128_si32(sC);
				p++;
L_CP1B:
				;
			}
			else {
#				if HAVE_CVC
					__m128i sC=_mm_mullo_epi16(sCvc1,_mm_set1_epi16(alpha));
					sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
					sC=_mm_add_epi16(sC,_mm_srli_epi16(sC,8));
					__m128i sC2=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x0f0d0b09));
					sC=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x07050301));
					sC=_mm_sub_epi32(sC,sC2);
					emUInt32 * e1=p+w-1;
					if (p<e1) {
						if (((char*)p-(char*)NULL)&7) {
							*p+=_mm_cvtsi128_si32(sC);
							p++;
							if (p>=e1) goto L_AD1A;
						}
						emUInt32 * e3=e1-2;
						if (p<e3) {
							if (((char*)p-(char*)NULL)&15) {
								__m128i v=_mm_loadl_epi64((__m128i*)p);
								v=_mm_add_epi32(v,sC);
								_mm_storel_epi64((__m128i*)p,v);
								p+=2;
								if (p>=e3) goto L_AD2A;
							}
							emUInt32 * e7=e3-4;
							if (p<e7) {
								if (((char*)p-(char*)NULL)&31) {
									__m128i v=_mm_load_si128((__m128i*)p);
									v=_mm_add_epi32(v,sC);
									_mm_store_si128((__m128i*)p,v);
									p+=4;
									if (p>=e7) goto L_AD3A;
								}
								__m256i aC=_mm256_broadcastsi128_si256(sC);
								emUInt32 * e15=e7-8;
								if (p<e15) {
									emUInt32 * e31=e15-16;
									if (p<e31) {
										do {
											__m256i v0=_mm256_load_si256((__m256i*)p);
											__m256i v1=_mm256_load_si256((__m256i*)p+1);
											__m256i v2=_mm256_load_si256((__m256i*)p+2);
											__m256i v3=_mm256_load_si256((__m256i*)p+3);
											v0=_mm256_add_epi32(v0,aC);
											v1=_mm256_add_epi32(v1,aC);
											v2=_mm256_add_epi32(v2,aC);
											v3=_mm256_add_epi32(v3,aC);
											_mm256_store_si256((__m256i*)p,v0);
											_mm256_store_si256((__m256i*)p+1,v1);
											_mm256_store_si256((__m256i*)p+2,v2);
											_mm256_store_si256((__m256i*)p+3,v3);
											p+=32;
										} while (p<e31);
										if (p>=e15) goto L_AD5B;
									}
									{
										__m256i v0=_mm256_load_si256((__m256i*)p);
										__m256i v1=_mm256_load_si256((__m256i*)p+1);
										v0=_mm256_add_epi32(v0,aC);
										v1=_mm256_add_epi32(v1,aC);
										_mm256_store_si256((__m256i*)p,v0);
										_mm256_store_si256((__m256i*)p+1,v1);
									}
									p+=16;
L_AD5B:
									if (p>=e7) goto L_AD4B;
								}
								{
									__m256i v=_mm256_load_si256((__m256i*)p);
									v=_mm256_add_epi32(v,aC);
									_mm256_store_si256((__m256i*)p,v);
								}
								p+=8;
L_AD4B:
								if (p>=e3) goto L_AD3B;
							}
L_AD3A:
							{
								__m128i v=_mm_load_si128((__m128i*)p);
								v=_mm_add_epi32(v,sC);
								_mm_store_si128((__m128i*)p,v);
							}
							p+=4;
L_AD3B:
							if (p>=e1) goto L_AD2B;
						}
L_AD2A:
						{
							__m128i v=_mm_loadl_epi64((__m128i*)p);
							v=_mm_add_epi32(v,sC);
							_mm_storel_epi64((__m128i*)p,v);
						}
						p+=2;
L_AD2B:
						if (p>e1) goto L_AD1B;
					}
L_AD1A:
					*p+=_mm_cvtsi128_si32(sC);
					p++;
L_AD1B:
					;
#				else
					__m128i sC=_mm_mullo_epi16(sCvc1,_mm_set1_epi16(alpha));
					__m128i sBeta=_mm_set1_epi16(255-alpha);
					__m128i sShuf=_mm_set_epi32(-1,-1,0x0f0d0b09,0x07050301);
					sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
					emUInt32 * e1=p+w-1;
					do {
						if (p<e1 && (((char*)p-(char*)NULL)&7)==0) {
							sC=_mm_broadcastq_epi64(sC);
							emUInt32 * e3=e1-2;
							do {
								if (p<e3 && (((char*)p-(char*)NULL)&15)==0) {
									__m256i aBeta=_mm256_broadcastsi128_si256(sBeta);
									__m256i aShuf=_mm256_broadcastsi128_si256(sShuf);
									__m256i aC=_mm256_broadcastsi128_si256(sC);
									emUInt32 * e7=e3-4;
									do {
										if (p<e7 && (((char*)p-(char*)NULL)&31)==0) {
											do {
												__m256i v=_mm256_load_si256((__m256i*)p);
												__m256i v1=_mm256_unpacklo_epi8(v,_mm256_setzero_si256());
												__m256i v2=_mm256_unpackhi_epi8(v,_mm256_setzero_si256());
												v1=_mm256_mullo_epi16(v1,aBeta);
												v2=_mm256_mullo_epi16(v2,aBeta);
												v1=_mm256_add_epi16(v1,aC);
												v2=_mm256_add_epi16(v2,aC);
												v1=_mm256_add_epi16(v1,_mm256_srli_epi16(v1,8));
												v2=_mm256_add_epi16(v2,_mm256_srli_epi16(v2,8));
												v1=_mm256_srli_epi16(v1,8);
												v2=_mm256_srli_epi16(v2,8);
												v=_mm256_packus_epi16(v1,v2);
												_mm256_store_si256((__m256i*)p,v);
												p+=8;
											} while (p<e7);
											if (p>=e3) break;
										}
										__m256i v=_mm256_cvtepu8_epi16(_mm_load_si128((__m128i*)p));
										v=_mm256_mullo_epi16(v,aBeta);
										v=_mm256_add_epi16(v,aC);
										v=_mm256_add_epi16(v,_mm256_srli_epi16(v,8));
										v=_mm256_shuffle_epi8(v,aShuf);
										v=_mm256_permute4x64_epi64(v,0x08);
										_mm_store_si128((__m128i*)p,_mm256_castsi256_si128(v));
										p+=4;
									} while (p<e3);
									if (p>=e1) break;
								}
								__m128i v=_mm_loadl_epi64((__m128i*)p);
								v=_mm_cvtepu8_epi16(v);
								v=_mm_mullo_epi16(v,sBeta);
								v=_mm_add_epi16(v,sC);
								v=_mm_add_epi16(v,_mm_srli_epi16(v,8));
								v=_mm_shuffle_epi8(v,sShuf);
								_mm_storel_epi64((__m128i*)p,v);
								p+=2;
							} while (p<e1);
							if (p>e1) break;
						}
						__m128i v=_mm_cvtsi32_si128(*p);
						v=_mm_cvtepu8_epi16(v);
						v=_mm_mullo_epi16(v,sBeta);
						v=_mm_add_epi16(v,sC);
						v=_mm_add_epi16(v,_mm_srli_epi16(v,8));
						v=_mm_shuffle_epi8(v,sShuf);
						*p=_mm_cvtsi128_si32(v);
						p++;
					} while(p<=e1);
#				endif
			}
		}

		alpha=(sct.Color1.GetAlpha()*opacityEnd+0x800)>>12;
		if (alpha>=255) {
			__m128i sC=_mm_packus_epi16(sCvc1,sCvc1);
			*p=_mm_cvtsi128_si32(sC);
		}
		else {
#			if HAVE_CVC
				__m128i sC=_mm_mullo_epi16(sCvc1,_mm_set1_epi16(alpha));
				sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
				sC=_mm_add_epi16(sC,_mm_srli_epi16(sC,8));
				__m128i sC2=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x0f0d0b09));
				sC=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x07050301));
				sC=_mm_sub_epi32(sC,sC2);
				*p+=_mm_cvtsi128_si32(sC);
#			else
				__m128i sC=_mm_cvtsi32_si128(*p);
				sC=_mm_cvtepu8_epi16(sC);
				sC=_mm_unpacklo_epi16(sCvc1,sC);
				sC=_mm_madd_epi16(sC,_mm_set1_epi32(alpha+((255-alpha)<<16)));
				sC=_mm_add_epi16(sC,_mm_set1_epi16(0x80));
				sC=_mm_add_epi16(sC,_mm_srli_epi16(sC,8));
				sC=_mm_shuffle_epi8(sC,_mm_set1_epi32(0x0d090501));
				*p=_mm_cvtsi128_si32(sC);
#			endif
		}
	}
}


#endif
