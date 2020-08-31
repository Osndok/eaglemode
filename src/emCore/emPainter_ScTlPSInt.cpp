//------------------------------------------------------------------------------
// emPainter_ScTlPSInt.cpp
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
// algorithms for emPainter::ScanlineTool::PaintScanlineInt..(..) with
// different settings. The preprocessor defines for these settings are:
//   HAVE_GC1:   0 or 1        - Whether to paint a gradient with color 1 (this
//                               can be combined with HAVE_C2 but not with
//                               HAVE_ALPHA).
//   HAVE_GC2:   0 or 1        - Whether to paint a gradient with color 2 (this
//                               can be combined with HAVE_C1 but not with
//                               HAVE_ALPHA).
//   HAVE_ALPHA: 0 or 1        - Whether to alpha blend (when no gradient)
//   CHANNELS:   1, 2, 3, or 4 - Number of channels in the input map.
//   PIXEL_SIZE: 1, 2, or 4    - Bytes per pixel in the output map.
//   HAVE_CVC:   0 or 1        - Whether canvas color is given (opaque).
//------------------------------------------------------------------------------

#if !defined(HAVE_GC1)
//==============================================================================
//====================== Top level include / Set HAVE_GC1 ======================
//==============================================================================

#include "emPainter_ScTl.h"
#define CONCATIMPL(a,b) a##b
#define CONCAT(a,b) CONCATIMPL(a,b)
#define METHOD_NAME_HAVE_GC1_0
#define METHOD_NAME_HAVE_GC1_1 G1
#define METHOD_NAME_HAVE_GC2_0
#define METHOD_NAME_HAVE_GC2_1 G2
#define METHOD_NAME_HAVE_ALPHA_0
#define METHOD_NAME_HAVE_ALPHA_1 A
#define METHOD_NAME_CHANNELS_1 Cs1
#define METHOD_NAME_CHANNELS_2 Cs2
#define METHOD_NAME_CHANNELS_3 Cs3
#define METHOD_NAME_CHANNELS_4 Cs4
#define METHOD_NAME_PIXEL_SIZE_1 Ps1
#define METHOD_NAME_PIXEL_SIZE_2 Ps2
#define METHOD_NAME_PIXEL_SIZE_4 Ps4
#define METHOD_NAME_HAVE_CVC_0
#define METHOD_NAME_HAVE_CVC_1 Cv

#define HAVE_GC1 0
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_GC1

#define HAVE_GC1 1
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_GC1


#elif !defined(HAVE_GC2)
//==============================================================================
//================================ Set HAVE_GC2 ================================
//==============================================================================

#define HAVE_GC2 0
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_GC2

#define HAVE_GC2 1
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_GC2


#elif !defined(HAVE_ALPHA)
//==============================================================================
//=============================== Set HAVE_ALPHA ===============================
//==============================================================================

#define HAVE_ALPHA 0
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_ALPHA

// Alpha is relevant only if no gradient.
#if !HAVE_GC1 && !HAVE_GC2
#	define HAVE_ALPHA 1
#	include "emPainter_ScTlPSInt.cpp"
#	undef HAVE_ALPHA
#endif


#elif !defined(CHANNELS)
//==============================================================================
//================================ Set CHANNELS ================================
//==============================================================================

#define CHANNELS 1
#include "emPainter_ScTlPSInt.cpp"
#undef CHANNELS

#define CHANNELS 2
#include "emPainter_ScTlPSInt.cpp"
#undef CHANNELS

#define CHANNELS 3
#include "emPainter_ScTlPSInt.cpp"
#undef CHANNELS

#define CHANNELS 4
#include "emPainter_ScTlPSInt.cpp"
#undef CHANNELS


#elif !defined(PIXEL_SIZE)
//==============================================================================
//=============================== Set PIXEL_SIZE ===============================
//==============================================================================

#define PIXEL_SIZE 1
#include "emPainter_ScTlPSInt.cpp"
#undef PIXEL_SIZE

#define PIXEL_SIZE 2
#include "emPainter_ScTlPSInt.cpp"
#undef PIXEL_SIZE

#define PIXEL_SIZE 4
#include "emPainter_ScTlPSInt.cpp"
#undef PIXEL_SIZE


#elif !defined(HAVE_CVC)
//==============================================================================
//================================ Set HAVE_CVC ================================
//==============================================================================

#define HAVE_CVC 0
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_CVC

#define HAVE_CVC 1
#include "emPainter_ScTlPSInt.cpp"
#undef HAVE_CVC


#else
//==============================================================================
//================ emPainter::ScanlineTool::PaintScanlineInt... ================
//==============================================================================

void emPainter::ScanlineTool::CONCAT(PaintScanlineInt,CONCAT(
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
		CONCAT(METHOD_NAME_PIXEL_SIZE_,PIXEL_SIZE),
		CONCAT(METHOD_NAME_HAVE_CVC_,HAVE_CVC)
	)
)) (
	const ScanlineTool & sct, int x, int y, int w,
	int opacityBeg, int opacity, int opacityEnd
)
{
#	if PIXEL_SIZE==1
		typedef emUInt8 Pixel;
#	elif PIXEL_SIZE==2
		typedef emUInt16 Pixel;
#	else
		typedef emUInt32 Pixel;
#	endif

	if (w>MaxInterpolationBytesAtOnce/CHANNELS) {
		PaintLargeScanlineInt(sct,x,y,w,opacityBeg,opacity,opacityEnd);
		return;
	}

	sct.Interpolate(sct,x,y,w);
	const emByte * s=(const emByte*)sct.InterpolationBuffer;

	const SharedPixelFormat & pf = *sct.Painter.PixelFormat;

#	if HAVE_GC1 && HAVE_GC2
		unsigned c1R=sct.Color1.GetRed();
		unsigned c1G=sct.Color1.GetGreen();
		unsigned c1B=sct.Color1.GetBlue();
		unsigned c2R=sct.Color2.GetRed();
		unsigned c2G=sct.Color2.GetGreen();
		unsigned c2B=sct.Color2.GetBlue();
		const Pixel * hR=(const Pixel*)pf.RedHash  +255*256;
		const Pixel * hG=(const Pixel*)pf.GreenHash+255*256;
		const Pixel * hB=(const Pixel*)pf.BlueHash +255*256;
#	elif HAVE_GC1
		const Pixel * h1R=(const Pixel*)pf.RedHash  +sct.Color1.GetRed()  *256;
		const Pixel * h1G=(const Pixel*)pf.GreenHash+sct.Color1.GetGreen()*256;
		const Pixel * h1B=(const Pixel*)pf.BlueHash +sct.Color1.GetBlue() *256;
#	elif HAVE_GC2
		const Pixel * h2R=(const Pixel*)pf.RedHash  +sct.Color2.GetRed()  *256;
		const Pixel * h2G=(const Pixel*)pf.GreenHash+sct.Color2.GetGreen()*256;
		const Pixel * h2B=(const Pixel*)pf.BlueHash +sct.Color2.GetBlue() *256;
#	else
		const Pixel * hR=(const Pixel*)pf.RedHash  +255*256;
		const Pixel * hG=(const Pixel*)pf.GreenHash+255*256;
		const Pixel * hB=(const Pixel*)pf.BlueHash +255*256;
#	endif

#	if HAVE_CVC
		const Pixel * hcR=(const Pixel*)pf.RedHash  +sct.CanvasColor.GetRed()  *256;
		const Pixel * hcG=(const Pixel*)pf.GreenHash+sct.CanvasColor.GetGreen()*256;
		const Pixel * hcB=(const Pixel*)pf.BlueHash +sct.CanvasColor.GetBlue() *256;
#	else
		int rsh=sct.Painter.PixelFormat->RedShift;
		int gsh=sct.Painter.PixelFormat->GreenShift;
		int bsh=sct.Painter.PixelFormat->BlueShift;
		Pixel rmsk=(Pixel)sct.Painter.PixelFormat->RedRange;
		Pixel gmsk=(Pixel)sct.Painter.PixelFormat->GreenRange;
		Pixel bmsk=(Pixel)sct.Painter.PixelFormat->BlueRange;
#	endif

	Pixel * p=(Pixel*)(
		(char*)sct.Painter.Map+y*(size_t)sct.Painter.BytesPerRow
	) + x;
	Pixel * pLast=p+w-1;
	Pixel * pStop=p;
	int o=opacityBeg;
	for (;;) {

#		if HAVE_ALPHA
			o=(o*sct.Alpha+127)/255;
#		endif
#		if HAVE_GC1
			int o1=(o*sct.Color1.GetAlpha()+127)/255;
#		endif
#		if HAVE_GC2
			int o2=(o*sct.Color2.GetAlpha()+127)/255;
#		endif

		if (
#			if HAVE_GC1 && HAVE_GC2
				o1 < 0x1000 || o2 < 0x1000
#			elif HAVE_GC1
				o1 < 0x1000
#			elif HAVE_GC2
				o2 < 0x1000
#			else
				o < 0x1000
#			endif
		) {
#			if !HAVE_GC1 && !HAVE_GC2 && (CHANNELS==1 || CHANNELS==3)
				unsigned a=(255*o+0x800)>>12;
#				if !HAVE_CVC
					unsigned t=(255-a)*257;
#				endif
#			endif
			do {
#				if HAVE_GC1 || !HAVE_GC2
#					if CHANNELS==2 || CHANNELS==4
						unsigned a=s[CHANNELS-1];
#					elif HAVE_GC1 || HAVE_GC2
						unsigned a=255;
#					endif
#				endif

#				if CHANNELS==3 || CHANNELS==4
#					if HAVE_GC1 && HAVE_GC2
						unsigned r=s[0];
						unsigned g=s[1];
						unsigned b=s[2];
						unsigned ar1=((a-r)*o1+0x800)>>12;
						unsigned ag1=((a-g)*o1+0x800)>>12;
						unsigned ab1=((a-b)*o1+0x800)>>12;
						unsigned ar2=(r*o2+0x800)>>12;
						unsigned ag2=(g*o2+0x800)>>12;
						unsigned ab2=(b*o2+0x800)>>12;
						unsigned ar=ar1+ar2;
						unsigned ag=ag1+ag2;
						unsigned ab=ab1+ab2;
#						if CHANNELS==2 || CHANNELS==4
							if (!(ar+ag+ab)) continue;
#						endif
						Pixel pix =
							hR[((c1R*ar1+c2R*ar2)*257+0x8073)>>16] +
							hG[((c1G*ag1+c2G*ag2)*257+0x8073)>>16] +
							hB[((c1B*ab1+c2B*ab2)*257+0x8073)>>16]
						;
#					elif HAVE_GC1
						unsigned ar=((a-s[0])*o1+0x800)>>12;
						unsigned ag=((a-s[1])*o1+0x800)>>12;
						unsigned ab=((a-s[2])*o1+0x800)>>12;
						if (!(ar+ag+ab)) continue;
						Pixel pix=h1R[ar]+h1G[ag]+h1B[ab];
#					elif HAVE_GC2
						unsigned ar=(s[0]*o2+0x800)>>12;
						unsigned ag=(s[1]*o2+0x800)>>12;
						unsigned ab=(s[2]*o2+0x800)>>12;
						if (!(ar+ag+ab)) continue;
						Pixel pix=h2R[ar]+h2G[ag]+h2B[ab];
#					else
#						if CHANNELS==2 || CHANNELS==4
							a=(a*o+0x800)>>12;
							if (!a) continue;
#						endif
						Pixel pix=
							hR[(s[0]*o+0x800)>>12]+
							hG[(s[1]*o+0x800)>>12]+
							hB[(s[2]*o+0x800)>>12]
						;
#					endif
#				else
#					if HAVE_GC1 && HAVE_GC2
						unsigned g=s[0];
						unsigned a1=((a-g)*o1+0x800)>>12;
						unsigned a2=(g*o2+0x800)>>12;
						a=a1+a2;
#						if CHANNELS==2 || CHANNELS==4
							if (!a) continue;
#						endif
						Pixel pix =
							hR[((c1R*a1+c2R*a2)*257+0x8073)>>16] +
							hG[((c1G*a1+c2G*a2)*257+0x8073)>>16] +
							hB[((c1B*a1+c2B*a2)*257+0x8073)>>16]
						;
#					elif HAVE_GC1
						a=((a-s[0])*o1+0x800)>>12;
						if (!a) continue;
						Pixel pix=h1R[a]+h1G[a]+h1B[a];
#					elif HAVE_GC2
						unsigned a=(s[0]*o2+0x800)>>12;
						if (!a) continue;
						Pixel pix=h2R[a]+h2G[a]+h2B[a];
#					else
#						if CHANNELS==2 || CHANNELS==4
							a=(a*o+0x800)>>12;
							if (!a) continue;
#						endif
						unsigned g=(s[0]*o+0x800)>>12;
						Pixel pix=hR[g]+hG[g]+hB[g];
#					endif
#				endif

#				if (HAVE_GC1 || HAVE_GC2) && (CHANNELS==3 || CHANNELS==4)
#					if HAVE_CVC
						pix-=hcR[ar]+hcG[ag]+hcB[ab];
						*p+=pix;
#					else
						Pixel v=*p;
						*p=(Pixel)(
							(((((v>>rsh)&rmsk)*((255-ar)*257)+0x8073)>>16)<<rsh) +
							(((((v>>gsh)&gmsk)*((255-ag)*257)+0x8073)>>16)<<gsh) +
							(((((v>>bsh)&bmsk)*((255-ab)*257)+0x8073)>>16)<<bsh)
						) + pix;
#					endif
#				else
#					if HAVE_CVC
						pix-=hcR[a]+hcG[a]+hcB[a];
						*p+=pix;
#					else
#						if HAVE_GC1 || HAVE_GC2 || CHANNELS==2 || CHANNELS==4
							unsigned t=(255-a)*257;
#						endif
						Pixel v=*p;
						*p=(Pixel)(
							(((((v>>rsh)&rmsk)*t+0x8073)>>16)<<rsh) +
							(((((v>>gsh)&gmsk)*t+0x8073)>>16)<<gsh) +
							(((((v>>bsh)&bmsk)*t+0x8073)>>16)<<bsh)
						) + pix;
#					endif
#				endif
			} while (s+=CHANNELS, p++, p<pStop);
		}
		else {
			do {
#				if HAVE_GC1 || !HAVE_GC2
#					if CHANNELS==2 || CHANNELS==4
						unsigned a=s[CHANNELS-1];
#						if (HAVE_GC1 && HAVE_GC2) || (!HAVE_GC1 && !HAVE_GC2)
							if (!a) continue;
#						endif
#					elif HAVE_GC1
						unsigned a=255;
#					endif
#				endif

#				if CHANNELS==3 || CHANNELS==4
					unsigned r=s[0];
					unsigned g=s[1];
					unsigned b=s[2];
#					if HAVE_GC1 && HAVE_GC2
						Pixel pix =
							hR[((c1R*(a-r)+c2R*r)*257+0x8073)>>16] +
							hG[((c1G*(a-g)+c2G*g)*257+0x8073)>>16] +
							hB[((c1B*(a-b)+c2B*b)*257+0x8073)>>16]
						;
#					elif HAVE_GC1
						unsigned ar=a-r;
						unsigned ag=a-g;
						unsigned ab=a-b;
						unsigned argb=ar+ag+ab;
						if (!argb) continue;
						Pixel pix=h1R[ar]+h1G[ag]+h1B[ab];
#					elif HAVE_GC2
						unsigned ar=r;
						unsigned ag=g;
						unsigned ab=b;
						unsigned argb=ar+ag+ab;
						if (!argb) continue;
						Pixel pix=h2R[ar]+h2G[ag]+h2B[ab];
#					else
						Pixel pix=hR[r]+hG[g]+hB[b];
#					endif
#				else
					unsigned g=s[0];
#					if HAVE_GC1 && HAVE_GC2
						Pixel pix =
							hR[((c1R*(a-g)+c2R*g)*257+0x8073)>>16] +
							hG[((c1G*(a-g)+c2G*g)*257+0x8073)>>16] +
							hB[((c1B*(a-g)+c2B*g)*257+0x8073)>>16]
						;
#					elif HAVE_GC1
						a-=g;
						if (!a) continue;
						Pixel pix=h1R[a]+h1G[a]+h1B[a];
#					elif HAVE_GC2
						unsigned a=g;
						if (!a) continue;
						Pixel pix=h2R[a]+h2G[a]+h2B[a];
#					else
						Pixel pix=hR[g]+hG[g]+hB[g];
#					endif
#				endif

#				if ((HAVE_GC1 && HAVE_GC2) || (!HAVE_GC1 && !HAVE_GC2)) && (CHANNELS==1 || CHANNELS==3)
					*p=pix;
#				elif ((HAVE_GC1 && !HAVE_GC2) || (!HAVE_GC1 && HAVE_GC2)) && (CHANNELS==3 || CHANNELS==4)
					if (argb>=3*255) {
						*p=pix;
					}
					else {
#						if HAVE_CVC
							pix-=hcR[ar]+hcG[ag]+hcB[ab];
							*p+=pix;
#						else
							Pixel v=*p;
							*p=(Pixel)(
								(((((v>>rsh)&rmsk)*((255-ar)*257)+0x8073)>>16)<<rsh) +
								(((((v>>gsh)&gmsk)*((255-ag)*257)+0x8073)>>16)<<gsh) +
								(((((v>>bsh)&bmsk)*((255-ab)*257)+0x8073)>>16)<<bsh)
							) + pix;
#						endif
					}
#				else
					if (a>=255) {
						*p=pix;
					}
					else {
#						if HAVE_CVC
							pix-=hcR[a]+hcG[a]+hcB[a];
							*p+=pix;
#						else
							unsigned t=(255-a)*257;
							Pixel v=*p;
							*p=(Pixel)(
								(((((v>>rsh)&rmsk)*t+0x8073)>>16)<<rsh) +
								(((((v>>gsh)&gmsk)*t+0x8073)>>16)<<gsh) +
								(((((v>>bsh)&bmsk)*t+0x8073)>>16)<<bsh)
							) + pix;
#						endif
					}
#				endif
			} while (s+=CHANNELS, p++, p<pStop);
		}

		if (p>pLast) break;
		if (p==pLast) {
			o=opacityEnd;
		}
		else {
			o=opacity;
			pStop=pLast;
		}
	}
}


#endif
