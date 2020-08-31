//------------------------------------------------------------------------------
// emPainter_ScTl.cpp
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

#include "emPainter_ScTl.h"
#include <emCore/emCoreConfig.h>


bool emPainter::ScanlineTool::Init(
	const emTexture & texture, emColor canvasColor
)
{
	typedef	void (*PaintScanlineFunc)(const ScanlineTool & sct, int x, int y, int w,
	                                 int opacityBeg, int opacity, int opacityEnd);
	typedef	void (*InterpolateFunc)(const ScanlineTool & sct, int x, int y, int w);

	enum PSFuncMainIndex {
		PSF_COL     =0<<5,
		PSF_INT     =1<<5,
		PSF_INT_A   =2<<5,
		PSF_INT_G1  =3<<5,
		PSF_INT_G2  =4<<5,
		PSF_INT_G1G2=5<<5
	};
	enum IntImgFuncMainIndex {
		IIF_NEAREST     =0*12,
		IIF_AREA_SAMPLED=1*12,
		IIF_BILINEAR    =2*12,
		IIF_BICUBIC     =3*12,
		IIF_LANCZOS     =4*12,
		IIF_ADAPTIVE    =5*12
	};
#	define INT_IMG_FUNC_MAIN_INDEX_FROM_UQ(UQ) (UQ*12)

#	define EIGHT_TIMES(N) N,N,N,N,N,N,N,N
#	define NAMES_CV(N) N,N##Cv
#	define NAMES_PSCV(N) NAMES_CV(N##Ps1),NAMES_CV(N##Ps2),NULL,NULL,NAMES_CV(N##Ps4)
#	define NAMES_PFCV(N) NAMES_CV(N##Pf0),NAMES_CV(N##Pf1),NAMES_CV(N##Pf2),NAMES_CV(N##Pf3)
#	define NAMES_CSPSCV(N) NAMES_PSCV(N##Cs1),NAMES_PSCV(N##Cs2),NAMES_PSCV(N##Cs3),NAMES_PSCV(N##Cs4)
#	define NAMES_CSPFCV(N) NAMES_PFCV(N##Cs1),NAMES_PFCV(N##Cs2),NAMES_PFCV(N##Cs3),NAMES_PFCV(N##Cs4)
#	define NAMES_CS(N) N##Cs1,N##Cs2,N##Cs3,N##Cs4
#	define NAMES_EXCS(N) NAMES_CS(N##Et),NAMES_CS(N##Ee),NAMES_CS(N##Ez)

	static const PaintScanlineFunc psFuncTable[] = {
		NAMES_PSCV(PaintScanlineCol),EIGHT_TIMES(NULL),EIGHT_TIMES(NULL),EIGHT_TIMES(NULL),
		NAMES_CSPSCV(PaintScanlineInt),
		NAMES_CSPSCV(PaintScanlineIntA),
		NAMES_CSPSCV(PaintScanlineIntG1),
		NAMES_CSPSCV(PaintScanlineIntG2),
		NAMES_CSPSCV(PaintScanlineIntG1G2)
	};

#	if EM_HAVE_X86_INTRINSICS
	static const PaintScanlineFunc psFuncTableAvx2[] = {
		NAMES_PFCV(PaintScanlineColAvx2),EIGHT_TIMES(NULL),EIGHT_TIMES(NULL),EIGHT_TIMES(NULL),
		NAMES_CSPFCV(PaintScanlineIntAvx2),
		NAMES_CSPFCV(PaintScanlineIntAvx2A),
		NAMES_CSPFCV(PaintScanlineIntAvx2G1),
		NAMES_CSPFCV(PaintScanlineIntAvx2G2),
		NAMES_CSPFCV(PaintScanlineIntAvx2G1G2)
	};
#	endif

	static const InterpolateFunc iiFuncTable[] = {
		NAMES_EXCS(InterpolateImageNearest),
		NAMES_EXCS(InterpolateImageAreaSampled),
		NAMES_EXCS(InterpolateImageBilinear),
		NAMES_EXCS(InterpolateImageBicubic),
		NAMES_EXCS(InterpolateImageLanczos),
		NAMES_EXCS(InterpolateImageAdaptive)
	};

#	if EM_HAVE_X86_INTRINSICS
	static const InterpolateFunc iiFuncTableAvx2[] = {
		NAMES_EXCS(InterpolateImageAvx2Nearest),
		NAMES_EXCS(InterpolateImageAreaSampled),
		NAMES_EXCS(InterpolateImageAvx2Bilinear),
		NAMES_EXCS(InterpolateImageAvx2Bicubic),
		NAMES_EXCS(InterpolateImageAvx2Lanczos),
		NAMES_EXCS(InterpolateImageAvx2Adaptive)
	};
#	endif

	const PaintScanlineFunc * psFuncPtr;
#	if EM_HAVE_X86_INTRINSICS
	if (
		Painter.Model->CanCpuDoAvx2 &&
		Painter.Model->CoreConfig->AllowSIMD.Get() &&
		Painter.PixelFormat->OPFIndex >= 0
	) {
		psFuncPtr = psFuncTableAvx2 + (Painter.PixelFormat->OPFIndex<<1);
	}
	else
#	endif
	{
		psFuncPtr = psFuncTable + ((Painter.PixelFormat->BytesPerPixel-1)<<1);
	}

	CanvasColor=canvasColor;
	if (canvasColor.IsOpaque()) psFuncPtr++;

	switch (texture.GetType()) {
	case emTexture::COLOR:
		Color1=texture.Color;
		if (!Color1.GetAlpha()) return false;
		if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
		PaintScanline=psFuncPtr[PSF_COL];
		break;
	case emTexture::IMAGE:
		Alpha=texture.Alpha;
		if (Alpha<=0) return false;
		psFuncPtr+=(texture.Image->GetChannelCount()-1)<<3;
		if (texture.Alpha>=255) {
			PaintScanline=psFuncPtr[PSF_INT];
		}
		else {
			PaintScanline=psFuncPtr[PSF_INT_A];
		}
		goto L_SET_INTERPOLATE_IMAGE;
	case emTexture::IMAGE_COLORED:
		psFuncPtr+=(texture.Image->GetChannelCount()-1)<<3;
		Color1=texture.Color1;
		Color2=texture.Color2;
		if (!Color1.GetAlpha()) {
			if (!Color2.GetAlpha()) return false;
			if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
			PaintScanline=psFuncPtr[PSF_INT_G2];
		}
		else if (!Color2.GetAlpha()) {
			if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
			PaintScanline=psFuncPtr[PSF_INT_G1];
		}
		else {
			if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
			if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
			PaintScanline=psFuncPtr[PSF_INT_G1G2];
		}
		goto L_SET_INTERPOLATE_IMAGE;
	case emTexture::LINEAR_GRADIENT:
		{
			Color1=texture.Color1;
			Color2=texture.Color2;
			if (!Color1.GetAlpha()) {
				if (!Color2.GetAlpha()) return false;
				if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G2];
			}
			else if (!Color2.GetAlpha()) {
				if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G1];
			}
			else {
				if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
				if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G1G2];
			}
			Channels=1;
			double tx1=texture.X1*Painter.ScaleX+Painter.OriginX;
			double ty1=texture.Y1*Painter.ScaleY+Painter.OriginY;
			double tx2=texture.X2*Painter.ScaleX+Painter.OriginX;
			double ty2=texture.Y2*Painter.ScaleY+Painter.OriginY;
			double nx=tx2-tx1;
			double ny=ty2-ty1;
			double nn=nx*nx+ny*ny;
			double f=(nn<1E-3 ? 0.0 : (((emInt64)255)<<24)/nn);
			nx*=f; ny*=f;
			double tx=(tx1-0.5)*nx+(ty1-0.5)*ny;
			TX=((emInt64)tx)-0x7fffff;
			TDX=(emInt64)nx;
			TDY=(emInt64)ny;
			Interpolate=InterpolateLinearGradient;
		}
		break;
	case emTexture::RADIAL_GRADIENT:
		{
			Color1=texture.Color1;
			Color2=texture.Color2;
			if (!Color1.GetAlpha()) {
				if (!Color2.GetAlpha()) return false;
				if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G2];
			}
			else if (!Color2.GetAlpha()) {
				if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G1];
			}
			else {
				if ((Color1.Get()|0xFF) == canvasColor.Get()) return false;
				if ((Color2.Get()|0xFF) == canvasColor.Get()) return false;
				PaintScanline=psFuncPtr[PSF_INT_G1G2];
			}
			Channels=1;
			double rx=texture.W*Painter.ScaleX*0.5;
			double ry=texture.H*Painter.ScaleY*0.5;
			if (rx<1E-3) rx=1E-3;
			if (ry<1E-3) ry=1E-3;
			double nx=(((emInt64)255)<<23)/rx;
			double ny=(((emInt64)255)<<23)/ry;
			double tx=(texture.X*Painter.ScaleX+Painter.OriginX+rx-0.5)*nx;
			double ty=(texture.Y*Painter.ScaleY+Painter.OriginY+ry-0.5)*ny;
			TX=(emInt64)tx;
			TY=(emInt64)ty;
			TDX=(emInt64)nx;
			TDY=(emInt64)ny;
			Interpolate=InterpolateRadialGradient;
		}
		break;
	}

	return true;

L_SET_INTERPOLATE_IMAGE:

	int iw=texture.Image->GetWidth();
	int sx=texture.SrcX;
	int sx2=sx+texture.SrcW;
	if(sx<0) sx=0;
	if(sx2>iw) sx2=iw;
	if (sx>=sx2) return false;
	int ih=texture.Image->GetHeight();
	int sy=texture.SrcY;
	int sy2=sy+texture.SrcH;
	if(sy<0) sy=0;
	if(sy2>ih) sy2=ih;
	if (sy>=sy2) return false;
	int channels=texture.Image->GetChannelCount();
	Channels=channels;
	ImgMap=texture.Image->GetMap()+(sy*(size_t)iw+sx)*channels;
	ImgW=sx2-sx;
	ImgH=sy2-sy;
	ImgDX=channels;
	ImgDY=((ssize_t)iw)*channels;
	ImgSX=ImgW*ImgDX;
	ImgSY=ImgH*ImgDY;

	const InterpolateFunc * iiFuncPtr;
#	if EM_HAVE_X86_INTRINSICS
	if (
		Painter.Model->CanCpuDoAvx2 &&
		Painter.Model->CoreConfig->AllowSIMD.Get()
	) {
		iiFuncPtr = iiFuncTableAvx2;
	}
	else
#	endif
	{
		iiFuncPtr = iiFuncTable;
	}

	iiFuncPtr+=channels-1;

	emTexture::ExtensionType ext=texture.GetExtension();
	if (ext==emTexture::EXTEND_EDGE_OR_ZERO) {
		if (
			(channels&1)==0 || (
				texture.GetType()==emTexture::IMAGE_COLORED && (
					!Color1.GetAlpha() || !Color2.GetAlpha()
				)
			)
		) {
			ext=emTexture::EXTEND_ZERO;
		}
		else {
			ext=emTexture::EXTEND_EDGE;
		}
	}
	iiFuncPtr+=ext<<2;

	double tw=texture.W*Painter.ScaleX;
	double th=texture.H*Painter.ScaleY;
	double tdx=(((emInt64)ImgW)<<24)/tw;
	double tdy=(((emInt64)ImgH)<<24)/th;
	if (tdx<0.0 || tdx>2.8E14) return false;
	if (tdy<0.0 || tdy>2.8E14) return false;
	double tx=texture.X*Painter.ScaleX+Painter.OriginX;
	double ty=texture.Y*Painter.ScaleY+Painter.OriginY;
	TDX=(emInt64)tdx;
	TDY=(emInt64)tdy;

	if (TDX>0xFFFF00 || TDY>0xFFFF00) {
		int downscaleQuality=texture.GetDownscaleQuality();
		if (downscaleQuality==emTexture::DQ_BY_CONFIG) {
			downscaleQuality=Painter.Model->CoreConfig->DownscaleQuality.Get();
		}
		if (
			downscaleQuality==emTexture::DQ_NEAREST_PIXEL || (
				TDX<0x10000FF && TDY<0x10000FF &&
				TDX>0x0FFFF00 && TDY>0x0FFFF00 &&
				(((emInt64)(tx*tdx)+0x800)&0xFFF000)==0 &&
				(((emInt64)(ty*tdy)+0x800)&0xFFF000)==0
			)
		) {
			TX=(emInt64)((tx-0.5)*tdx);
			TY=(emInt64)((ty-0.5)*tdy);
			Interpolate=iiFuncPtr[IIF_NEAREST];
		}
		else {
			int n = (TDX/downscaleQuality+0xFFFFFF)>>24;
			if (n>1) {
				int t=ImgW;
				if (n>t) return false;
				ImgW=(t+n-1)/n;
				t-=(ImgW-1)*n+1;
				ImgMap+=ImgDX*(t>>1);
				ImgDX*=n;
				ImgSX=ImgW*ImgDX;
				tdx=ImgW*((emInt64)1<<24)/tw;
				TDX=(emInt64)tdx;
			}
			n = (TDY/downscaleQuality+0xFFFFFF)>>24;
			if (n>1) {
				int t=ImgH;
				if (n>t) return false;
				ImgH=(t+n-1)/n;
				t-=(ImgH-1)*n+1;
				ImgMap+=ImgDY*(t>>1);
				ImgDY*=n;
				ImgSY=ImgH*ImgDY;
				tdy=ImgH*((emInt64)1<<24)/th;
				TDY=(emInt64)tdy;
			}
			TX=(emInt64)(tx*tdx);
			TY=(emInt64)(ty*tdy);
			ODX = TDX<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/TDX+1;
			ODY = TDY<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/TDY+1;
			Interpolate=iiFuncPtr[IIF_AREA_SAMPLED];
		}
	}
	else {
		int upscaleQuality=texture.GetUpscaleQuality();
		if (upscaleQuality==emTexture::UQ_BY_CONFIG) {
			upscaleQuality=Painter.Model->CoreConfig->UpscaleQuality.Get();
		}
		else if (upscaleQuality==emTexture::UQ_BY_CONFIG_FOR_VIDEO) {
			upscaleQuality=Painter.Model->CoreConfig->UpscaleQuality.Get();
#			if EM_HAVE_X86_INTRINSICS
			if (
				!Painter.Model->CanCpuDoAvx2 ||
				!Painter.Model->CoreConfig->AllowSIMD.Get()
			)
#			endif
			{
				if (upscaleQuality > emTexture::UQ_BILINEAR) {
					upscaleQuality = emTexture::UQ_BILINEAR;
				}
			}
		}
		if (upscaleQuality==emTexture::UQ_AREA_SAMPLING) {
			TX=(emInt64)(tx*tdx);
			TY=(emInt64)(ty*tdy);
			ODX = TDX<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/TDX+1;
			ODY = TDY<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/TDY+1;
			Interpolate=iiFuncPtr[IIF_AREA_SAMPLED];
		}
		else {
			TX=(emInt64)((tx-0.5)*tdx);
			TY=(emInt64)((ty-0.5)*tdy);
			// Never call Bilinear or higher with TDX or
			// TDY greater than 0x1000000!
			Interpolate=iiFuncPtr[INT_IMG_FUNC_MAIN_INDEX_FROM_UQ(upscaleQuality)];
		}
	}

	return true;
}


void emPainter::ScanlineTool::PaintLargeScanlineInt(
	const ScanlineTool & sct, int x, int y, int w,
	int opacityBeg, int opacity, int opacityEnd
)
{
	int n=MaxInterpolationBytesAtOnce/sct.Channels;

	if (w>n) {
		static const int align=32; // Good for the SIMD algorithms.
		const char * pRow=
			(const char*)sct.Painter.Map +
			y*(size_t)sct.Painter.BytesPerRow
		;
		int bytesPerPixel=sct.Painter.PixelFormat->BytesPerPixel;
		do {
			int r=(pRow+(x+n)*bytesPerPixel-(char*)NULL)&(align-1);
			int m=n-r/bytesPerPixel;
			sct.PaintScanline(sct,x,y,m,opacityBeg,opacity,opacity);
			opacityBeg=opacity;
			x+=m;
			w-=m;
		} while (w>n);

		if (w==1) opacityBeg=opacityEnd;
	}

	sct.PaintScanline(sct,x,y,w,opacityBeg,opacity,opacityEnd);
}
