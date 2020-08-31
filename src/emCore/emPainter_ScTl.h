//------------------------------------------------------------------------------
// emPainter_ScTl.h
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

#ifndef emPainter_ScTl_h
#define emPainter_ScTl_h

#ifndef emPainter_h
#include <emCore/emPainter.h>
#endif


class emPainter::ScanlineTool {

public:

	ScanlineTool(const emPainter & painter);

	bool Init(const emTexture & texture, emColor canvasColor);

	void (*PaintScanline)(const ScanlineTool & sct, int x, int y, int w,
	                      int opacityBeg, int opacity, int opacityEnd);

private:

	void (*Interpolate)(const ScanlineTool & sct, int x, int y, int w);

	static void PaintLargeScanlineInt(
		const ScanlineTool & sct, int x, int y, int w,
		int opacityBeg, int opacity, int opacityEnd
	);

	const emPainter & Painter;
	int Alpha;
	emColor CanvasColor,Color1,Color2;
	int Channels;
	const emByte * ImgMap;
	int ImgW,ImgH;
	ssize_t ImgDX,ImgDY;
	ssize_t ImgSX,ImgSY;
	emInt64 TX,TY,TDX,TDY;
	emUInt32 ODX,ODY;

	enum { MaxInterpolationBytesAtOnce=1024 };

	enum { InterpolationBufferSize=MaxInterpolationBytesAtOnce*2+128 };
		// Lots of algorithms need more than MaxInterpolationBytesAtOnce...

	emUInt64 InterpolationBuffer[(InterpolationBufferSize+7)/8];

#	define DECLARE_INTERPOLATE(NAME) \
		static void Interpolate##NAME(const ScanlineTool & sct, int x, int y, int w);

#	define DECLARE_INTERPOLATE_CS(NAME) \
		DECLARE_INTERPOLATE(NAME##Cs1) \
		DECLARE_INTERPOLATE(NAME##Cs2) \
		DECLARE_INTERPOLATE(NAME##Cs3) \
		DECLARE_INTERPOLATE(NAME##Cs4)

#	define DECLARE_INTERPOLATE_EXCS(NAME) \
		DECLARE_INTERPOLATE_CS(NAME##Et) \
		DECLARE_INTERPOLATE_CS(NAME##Ee) \
		DECLARE_INTERPOLATE_CS(NAME##Ez)

	DECLARE_INTERPOLATE(LinearGradient) //  1 InterpolateLinearGradient function
	DECLARE_INTERPOLATE(RadialGradient) //  1 InterpolateRadialGradient function
	DECLARE_INTERPOLATE_EXCS(ImageNearest)     // 12 InterpolateImageNear.. functions
	DECLARE_INTERPOLATE_EXCS(ImageAreaSampled) // 12 InterpolateImageArea.. functions
	DECLARE_INTERPOLATE_EXCS(ImageBilinear)    // 12 InterpolateImageBili.. functions
	DECLARE_INTERPOLATE_EXCS(ImageBicubic)     // 12 InterpolateImageBicu.. functions
	DECLARE_INTERPOLATE_EXCS(ImageLanczos)     // 12 InterpolateImageLanc.. functions
	DECLARE_INTERPOLATE_EXCS(ImageAdaptive)    // 12 InterpolateImageAdap.. functions
#	if EM_HAVE_X86_INTRINSICS
		DECLARE_INTERPOLATE_EXCS(ImageAvx2Nearest)     // 12 InterpolateImageAvx2Near.. functions
		DECLARE_INTERPOLATE_EXCS(ImageAvx2Bilinear)    // 12 InterpolateImageAvx2Bili.. functions
		DECLARE_INTERPOLATE_EXCS(ImageAvx2Bicubic)     // 12 InterpolateImageAvx2Bicu.. functions
		DECLARE_INTERPOLATE_EXCS(ImageAvx2Lanczos)     // 12 InterpolateImageAvx2Lanc.. functions
		DECLARE_INTERPOLATE_EXCS(ImageAvx2Adaptive)    // 12 InterpolateImageAvx2Adap.. functions
#	endif

#	define DECLARE_PAINTSCANLINE(NAME) \
		static void PaintScanline##NAME(const ScanlineTool & sct, int x, int y, int w, \
		                                int opacityBeg, int opacity, int opacityEnd);

#	define DECLARE_PAINTSCANLINE_CV(NAME) \
		DECLARE_PAINTSCANLINE(NAME) \
		DECLARE_PAINTSCANLINE(NAME##Cv)

#	define DECLARE_PAINTSCANLINE_PSCV(NAME) \
		DECLARE_PAINTSCANLINE_CV(NAME##Ps1) \
		DECLARE_PAINTSCANLINE_CV(NAME##Ps2) \
		DECLARE_PAINTSCANLINE_CV(NAME##Ps4)

#	define DECLARE_PAINTSCANLINE_PFCV(NAME) \
		DECLARE_PAINTSCANLINE_CV(NAME##Pf0) \
		DECLARE_PAINTSCANLINE_CV(NAME##Pf1) \
		DECLARE_PAINTSCANLINE_CV(NAME##Pf2) \
		DECLARE_PAINTSCANLINE_CV(NAME##Pf3)

#	define DECLARE_PAINTSCANLINE_CSPSCV(NAME) \
		DECLARE_PAINTSCANLINE_PSCV(NAME##Cs1) \
		DECLARE_PAINTSCANLINE_PSCV(NAME##Cs2) \
		DECLARE_PAINTSCANLINE_PSCV(NAME##Cs3) \
		DECLARE_PAINTSCANLINE_PSCV(NAME##Cs4)

#	define DECLARE_PAINTSCANLINE_CSPFCV(NAME) \
		DECLARE_PAINTSCANLINE_PFCV(NAME##Cs1) \
		DECLARE_PAINTSCANLINE_PFCV(NAME##Cs2) \
		DECLARE_PAINTSCANLINE_PFCV(NAME##Cs3) \
		DECLARE_PAINTSCANLINE_PFCV(NAME##Cs4)

#	define DECLARE_PAINTSCANLINE_GACSPSCV(NAME) \
		DECLARE_PAINTSCANLINE_CSPSCV(NAME) \
		DECLARE_PAINTSCANLINE_CSPSCV(NAME##A) \
		DECLARE_PAINTSCANLINE_CSPSCV(NAME##G2) \
		DECLARE_PAINTSCANLINE_CSPSCV(NAME##G1) \
		DECLARE_PAINTSCANLINE_CSPSCV(NAME##G1G2)

#	define DECLARE_PAINTSCANLINE_GACSPFCV(NAME) \
		DECLARE_PAINTSCANLINE_CSPFCV(NAME) \
		DECLARE_PAINTSCANLINE_CSPFCV(NAME##A) \
		DECLARE_PAINTSCANLINE_CSPFCV(NAME##G2) \
		DECLARE_PAINTSCANLINE_CSPFCV(NAME##G1) \
		DECLARE_PAINTSCANLINE_CSPFCV(NAME##G1G2)

	DECLARE_PAINTSCANLINE_PSCV(Col)     //   6 PaintScanlineCol.. functions
	DECLARE_PAINTSCANLINE_GACSPSCV(Int) // 120 PaintScanlineInt.. functions

#	if EM_HAVE_X86_INTRINSICS
		DECLARE_PAINTSCANLINE_PFCV(ColAvx2)     //   8 PaintScanlineColAvx2.. functions
		DECLARE_PAINTSCANLINE_GACSPFCV(IntAvx2) // 160 PaintScanlineIntAvx2.. functions
#	endif

#	undef DECLARE_INTERPOLATE
#	undef DECLARE_INTERPOLATE_CS
#	undef DECLARE_INTERPOLATE_EXCS
#	undef DECLARE_PAINTSCANLINE
#	undef DECLARE_PAINTSCANLINE_CV
#	undef DECLARE_PAINTSCANLINE_PSCV
#	undef DECLARE_PAINTSCANLINE_PFCV
#	undef DECLARE_PAINTSCANLINE_CSPSCV
#	undef DECLARE_PAINTSCANLINE_CSPFCV
#	undef DECLARE_PAINTSCANLINE_GACSPSCV
#	undef DECLARE_PAINTSCANLINE_GACSPFCV
};


inline emPainter::ScanlineTool::ScanlineTool(const emPainter & painter)
	: Painter(painter)
{
}


#endif
