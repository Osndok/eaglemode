//------------------------------------------------------------------------------
// emAvImageConverter_AVX2.cpp
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

#include <emAv/emAvImageConverter.h>

#if EM_HAVE_X86_INTRINSICS

#if defined(_MSC_VER)
#	include <immintrin.h>
#else
#	include <x86intrin.h>
#endif


#if defined(__GNUC__)
	__attribute__((target("avx2")))
#endif
void emAvImageConverter::ConvertI420_AVX2(int y1, int y2)
{
	const emByte * sy, * syBeg, * su, * sv;
	emByte * t;
	int duv,cu,cv,cy,cr,cg,cb,cr1,cg1,cb1,c;

	while (y1<y2) {
		y2--;
		t=Image->GetWritableMap()+((y2+1)*Width)*3;
		syBeg=Plane+y2*BPL;
		sy=syBeg+Width-2;
		su=Plane2+(y2>>1)*BPL2+((Width-2)>>1);
		sv=Plane3+(y2>>1)*BPL2+((Width-2)>>1);
		duv=0;
		if (y2&1) {
			if (y2<Height-1) duv=BPL2;
		}
		else {
			if (y2>0) duv=-BPL2;
		}
		cu=su[0]*3+su[duv];
		cv=sv[0]*3+sv[duv];
		cr=409*cv+(128*4-16*298*4-409*512);
		cg=-100*cu-208*cv+(128*4-16*298*4+100*512+208*512);
		cb=516*cu+(128*4-16*298*4-516*512);
		do {
			if (((t-(emByte*)NULL)&7)==0 && sy-6>=syBeg) {
				__m256i ar=_mm256_set_epi32(0,0,0,0,0,0,0,cr*32);
				__m256i ag=_mm256_set_epi32(0,0,0,0,0,0,0,cg*32);
				__m256i ab=_mm256_set_epi32(0,0,0,0,0,0,0,cb*32);
				su-=3;
				sv-=3;
				sy-=6;
				do {
					__m128i au1=_mm_cvtsi32_si128(((emUInt32*)su)[0]);
					__m128i av1=_mm_cvtsi32_si128(((emUInt32*)sv)[0]);
					__m128i au2=_mm_cvtsi32_si128(((emUInt32*)(su+duv))[0]);
					__m128i av2=_mm_cvtsi32_si128(((emUInt32*)(sv+duv))[0]);

					au1=_mm_cvtepu8_epi32(au1);
					av1=_mm_cvtepu8_epi32(av1);
					au2=_mm_cvtepu8_epi32(au2);
					av2=_mm_cvtepu8_epi32(av2);

					au2=_mm_add_epi32(au2,au1);
					av2=_mm_add_epi32(av2,av1);
					au1=_mm_add_epi32(au1,au1);
					av1=_mm_add_epi32(av1,av1);
					au1=_mm_add_epi32(au1,au2);
					av1=_mm_add_epi32(av1,av2);

					__m128i ayt=_mm_loadl_epi64((__m128i*)sy);

					__m256i auu=_mm256_broadcastsi128_si256(au1);
					__m256i avv=_mm256_broadcastsi128_si256(av1);

					__m256i ay=_mm256_cvtepu8_epi32(ayt);

					auu=_mm256_mullo_epi32(auu,_mm256_set_epi32(
						-100*32,-100*32,-100*32,-100*32,
						516*32,516*32,516*32,516*32
					));
					avv=_mm256_mullo_epi32(avv,_mm256_set_epi32(
						-208*32,-208*32,-208*32,-208*32,
						409*32,409*32,409*32,409*32
					));

					ay=_mm256_mullo_epi16(ay,_mm256_set_epi16(
						0,149,0,149,0,149,0,149,0,149,0,149,0,149,0,149
					));

					__m256i bb=_mm256_add_epi32(auu,_mm256_set_epi32(
						0,
						0,
						0,
						0,
						(128*4-16*298*4-516*512)*32,
						(128*4-16*298*4-516*512)*32,
						(128*4-16*298*4-516*512)*32,
						(128*4-16*298*4-516*512)*32
					));
					__m256i br=_mm256_add_epi32(avv,_mm256_set_epi32(
						(128*4-16*298*4+100*512+208*512)*32,
						(128*4-16*298*4+100*512+208*512)*32,
						(128*4-16*298*4+100*512+208*512)*32,
						(128*4-16*298*4+100*512+208*512)*32,
						(128*4-16*298*4-409*512)*32,
						(128*4-16*298*4-409*512)*32,
						(128*4-16*298*4-409*512)*32,
						(128*4-16*298*4-409*512)*32
					));

					ay=_mm256_slli_epi32(ay,9);

					__m256i bg=_mm256_add_epi32(br,bb);

					br=_mm256_permutevar8x32_epi32(br,_mm256_set_epi32(
						3,3,2,2,1,1,0,0
					));
					bg=_mm256_permutevar8x32_epi32(bg,_mm256_set_epi32(
						7,7,6,6,5,5,4,4
					));
					bb=_mm256_permutevar8x32_epi32(bb,_mm256_set_epi32(
						3,3,2,2,1,1,0,0
					));

					__m256i fr=_mm256_add_epi32(ay,br);
					__m256i fg=_mm256_add_epi32(ay,bg);
					__m256i fb=_mm256_add_epi32(ay,bb);

					ar=_mm256_permute2x128_si256(ar,br,0x03);
					ag=_mm256_permute2x128_si256(ag,bg,0x03);
					ab=_mm256_permute2x128_si256(ab,bb,0x03);

					ar=_mm256_alignr_epi8(ar,br,4);
					ag=_mm256_alignr_epi8(ag,bg,4);
					ab=_mm256_alignr_epi8(ab,bb,4);

					fr=_mm256_add_epi32(fr,ar);
					fg=_mm256_add_epi32(fg,ag);
					fr=_mm256_srli_epi32(fr,16);
					fb=_mm256_add_epi32(fb,ab);

					__m256i ac=_mm256_blend_epi16(fr,fg,0xaa);

					t-=24;

					ac=_mm256_packus_epi16(ac,fb);

					su-=4;
					sv-=4;

					ac=_mm256_shuffle_epi8(ac,_mm256_set_epi8(
						 2, 9, 1, 0, -1,-1,-1,-1, 15, 7, 6,13,  5, 4,11, 3,
						-1,-1,-1,-1, 15, 7, 6,13,  5, 4,11, 3,  2, 9, 1, 0
					));

					sy-=8;

					_mm_storel_epi64((__m128i*)(t+16),_mm256_extracti128_si256(ac,1));

					ac=_mm256_permutevar8x32_epi32(ac,_mm256_set_epi32(
						7,6,5,4,7,2,1,0
					));

					_mm_storeu_si128((__m128i*)t,_mm256_castsi256_si128(ac));

				} while (sy>=syBeg);

				cr=_mm_cvtsi128_si32(_mm256_castsi256_si128(ar))/32;
				cg=_mm_cvtsi128_si32(_mm256_castsi256_si128(ag))/32;
				cb=_mm_cvtsi128_si32(_mm256_castsi256_si128(ab))/32;
				su+=3;
				sv+=3;
				sy+=6;
			}
			else {
				t-=6;

				cr1=cr;
				cg1=cg;
				cb1=cb;
				cu=su[0]*3+su[duv];
				cv=sv[0]*3+sv[duv];
				cr=409*cv+(128*4-16*298*4-409*512);
				cg=-100*cu-208*cv+(128*4-16*298*4+100*512+208*512);
				cb=516*cu+(128*4-16*298*4-516*512);

				cy=sy[1]*(298*8);
				c=(cr+cr1+cy)>>11;
				if ((unsigned)c>255) c=(-c)>>16;
				t[3]=(emByte)c;
				c=(cg+cg1+cy)>>11;
				if ((unsigned)c>255) c=(-c)>>16;
				t[4]=(emByte)c;
				c=(cb+cb1+cy)>>11;
				if ((unsigned)c>255) c=(-c)>>16;
				t[5]=(emByte)c;

				cy=sy[0]*(298*4);
				c=(cr+cy)>>10;
				if ((unsigned)c>255) c=(-c)>>16;
				t[0]=(emByte)c;
				c=(cg+cy)>>10;
				if ((unsigned)c>255) c=(-c)>>16;
				t[1]=(emByte)c;
				c=(cb+cy)>>10;
				if ((unsigned)c>255) c=(-c)>>16;
				t[2]=(emByte)c;

				su--;
				sv--;
				sy-=2;
			}
		} while (sy>=syBeg);
	}
}


#endif
