//------------------------------------------------------------------------------
// emPainter_ScTlPSCol.cpp
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
// algorithms for emPainter::ScanlineTool::PaintScanlineCol..(..) with different
// settings. The preprocessor defines for these settings are:
//   PIXEL_SIZE: 1, 2, or 4    - Bytes per pixel in the output map.
//   HAVE_CVC:   0 or 1        - Whether canvas color is given (opaque).
//------------------------------------------------------------------------------

#if !defined(PIXEL_SIZE)
//==============================================================================
//===================== Top level include / Set PIXEL_SIZE =====================
//==============================================================================

#include "emPainter_ScTl.h"
#define CONCATIMPL(a,b) a##b
#define CONCAT(a,b) CONCATIMPL(a,b)
#define METHOD_NAME_PIXEL_SIZE_1 Ps1
#define METHOD_NAME_PIXEL_SIZE_2 Ps2
#define METHOD_NAME_PIXEL_SIZE_4 Ps4
#define METHOD_NAME_HAVE_CVC_0
#define METHOD_NAME_HAVE_CVC_1 Cv

#define PIXEL_SIZE 1
#include "emPainter_ScTlPSCol.cpp"
#undef PIXEL_SIZE

#define PIXEL_SIZE 2
#include "emPainter_ScTlPSCol.cpp"
#undef PIXEL_SIZE

#define PIXEL_SIZE 4
#include "emPainter_ScTlPSCol.cpp"
#undef PIXEL_SIZE


#elif !defined(HAVE_CVC)
//==============================================================================
//================================ Set HAVE_CVC ================================
//==============================================================================

#define HAVE_CVC 0
#include "emPainter_ScTlPSCol.cpp"
#undef HAVE_CVC

#define HAVE_CVC 1
#include "emPainter_ScTlPSCol.cpp"
#undef HAVE_CVC


#else
//==============================================================================
//================ emPainter::ScanlineTool::PaintScanlineCol... ================
//==============================================================================

void emPainter::ScanlineTool::CONCAT(PaintScanlineCol,CONCAT(
	CONCAT(METHOD_NAME_PIXEL_SIZE_,PIXEL_SIZE),
	CONCAT(METHOD_NAME_HAVE_CVC_,HAVE_CVC)
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

	Pixel * p=(Pixel*)(
		(char*)sct.Painter.Map+y*(size_t)sct.Painter.BytesPerRow
	) + x;

	const SharedPixelFormat & pf = *sct.Painter.PixelFormat;

	const Pixel * h1R=(const Pixel*)pf.RedHash  +sct.Color1.GetRed()  *256;
	const Pixel * h1G=(const Pixel*)pf.GreenHash+sct.Color1.GetGreen()*256;
	const Pixel * h1B=(const Pixel*)pf.BlueHash +sct.Color1.GetBlue() *256;

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

	unsigned alpha=(sct.Color1.GetAlpha()*opacityBeg+0x800)>>12;
	if (alpha>=255) {
		*p=h1R[255]+h1G[255]+h1B[255];
	}
	else {
		Pixel pix=h1R[alpha]+h1G[alpha]+h1B[alpha];
#		if HAVE_CVC
			pix-=hcR[alpha]+hcG[alpha]+hcB[alpha];
			*p+=pix;
#		else
			unsigned t=(255-alpha)*257;
			Pixel v=*p;
			*p=(Pixel)(
				(((((v>>rsh)&rmsk)*t+0x8073)>>16)<<rsh) +
				(((((v>>gsh)&gmsk)*t+0x8073)>>16)<<gsh) +
				(((((v>>bsh)&bmsk)*t+0x8073)>>16)<<bsh)
			) + pix;
#		endif
	}

	w-=2;
	if (w>=0) {
		p++;
		if (w>0) {
			Pixel * pEnd=p+w;
			alpha=(sct.Color1.GetAlpha()*opacity+0x800)>>12;
			if (alpha>=255) {
				Pixel pix=h1R[255]+h1G[255]+h1B[255];
				do {
					*p=pix;
					p++;
				} while(p<pEnd);
			}
			else {
				Pixel pix=h1R[alpha]+h1G[alpha]+h1B[alpha];
#				if HAVE_CVC
					pix-=hcR[alpha]+hcG[alpha]+hcB[alpha];
#				else
					unsigned t=(255-alpha)*257;
#				endif
				do {
#					if HAVE_CVC
						*p+=pix;
#					else
						Pixel v=*p;
						*p=(Pixel)(
							(((((v>>rsh)&rmsk)*t+0x8073)>>16)<<rsh) +
							(((((v>>gsh)&gmsk)*t+0x8073)>>16)<<gsh) +
							(((((v>>bsh)&bmsk)*t+0x8073)>>16)<<bsh)
						) + pix;
#					endif
					p++;
				} while(p<pEnd);
			}
		}

		alpha=(sct.Color1.GetAlpha()*opacityEnd+0x800)>>12;
		if (alpha>=255) {
			*p=h1R[255]+h1G[255]+h1B[255];
		}
		else {
			Pixel pix=h1R[alpha]+h1G[alpha]+h1B[alpha];
#			if HAVE_CVC
				pix-=hcR[alpha]+hcG[alpha]+hcB[alpha];
				*p+=pix;
#			else
				unsigned t=(255-alpha)*257;
				Pixel v=*p;
				*p=(Pixel)(
					(((((v>>rsh)&rmsk)*t+0x8073)>>16)<<rsh) +
					(((((v>>gsh)&gmsk)*t+0x8073)>>16)<<gsh) +
					(((((v>>bsh)&bmsk)*t+0x8073)>>16)<<bsh)
				) + pix;
#			endif
		}
	}
}


#endif
