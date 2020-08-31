//------------------------------------------------------------------------------
// emPainter_ScTlIntImg.cpp
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
// algorithms for emPainter::ScanlineTool::InterpolateImage..(..) with different
// settings. The preprocessor defines for these settings are:
//   EXTENSION:  0, 1, 2       - One of EXTEND_TILED, EXTEND_EDGE, EXTEND_ZERO
//   CHANNELS:   1, 2, 3, or 4 - Number of channels in the input map.
//------------------------------------------------------------------------------

#if !defined(EXTENSION)
//==============================================================================
//===================== Top level include / Set EXTENSION ======================
//==============================================================================

#include "emPainter_ScTl.h"
#define CONCATIMPL(a,b) a##b
#define CONCAT(a,b) CONCATIMPL(a,b)
#define EXTEND_TILED  0
#define EXTEND_EDGE   1
#define EXTEND_ZERO   2
#define METHOD_NAME_EXTENSION_0 Et
#define METHOD_NAME_EXTENSION_1 Ee
#define METHOD_NAME_EXTENSION_2 Ez
#define METHOD_NAME_CHANNELS_1 Cs1
#define METHOD_NAME_CHANNELS_2 Cs2
#define METHOD_NAME_CHANNELS_3 Cs3
#define METHOD_NAME_CHANNELS_4 Cs4

#define EXTENSION EXTEND_TILED
#include "emPainter_ScTlIntImg.cpp"
#undef EXTENSION

#define EXTENSION EXTEND_EDGE
#include "emPainter_ScTlIntImg.cpp"
#undef EXTENSION

#define EXTENSION EXTEND_ZERO
#include "emPainter_ScTlIntImg.cpp"
#undef EXTENSION


#elif !defined(CHANNELS)
//==============================================================================
//================================ Set CHANNELS ================================
//==============================================================================

#define CHANNELS 1
#include "emPainter_ScTlIntImg.cpp"
#undef CHANNELS

#define CHANNELS 2
#include "emPainter_ScTlIntImg.cpp"
#undef CHANNELS

#define CHANNELS 3
#include "emPainter_ScTlIntImg.cpp"
#undef CHANNELS

#define CHANNELS 4
#include "emPainter_ScTlIntImg.cpp"
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


// DEFINE_AND_SET_IMAGE_PIX_PTR(PIX_PTR,ROW_PTR,X)
#ifndef ZERO_PIXEL_DEFINED
#	define ZERO_PIXEL_DEFINED
	static const emInt32 ZeroPixel[1]={0};
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
		if ((size_t)X>=(size_t)ROW_PTR##UsedSX) PIX_PTR=(const emByte*)ZeroPixel;
#endif


// DEFINE_COLOR(C)
#if CHANNELS==1
#	define DEFINE_COLOR(C) emUInt32 C##g;
#elif CHANNELS==2
#	define DEFINE_COLOR(C) emUInt32 C##g, C##a;
#elif CHANNELS==3
#	define DEFINE_COLOR(C) emUInt32 C##r, C##g, C##b;
#else
#	define DEFINE_COLOR(C) emUInt32 C##r, C##g, C##b, C##a;
#endif


// DEFINE_AND_SET_COLOR(C,S)
#if CHANNELS==1
#	define DEFINE_AND_SET_COLOR(C,S) emUInt32 C##g=S;
#elif CHANNELS==2
#	define DEFINE_AND_SET_COLOR(C,S) emUInt32 C##g=S, C##a=S;
#elif CHANNELS==3
#	define DEFINE_AND_SET_COLOR(C,S) emUInt32 C##r=S, C##g=S, C##b=S;
#else
#	define DEFINE_AND_SET_COLOR(C,S) emUInt32 C##r=S, C##g=S, C##b=S, C##a=S;
#endif


// DEFINE_AND_READ_PREMUL_COLOR(C,PTR)
#if CHANNELS==1
#	define DEFINE_AND_READ_PREMUL_COLOR(C,PTR) \
		emUInt32 C##g=PTR[0];
#elif CHANNELS==2
#	define DEFINE_AND_READ_PREMUL_COLOR(C,PTR) \
		emUInt32 C##a=PTR[1], C##g=PTR[0]*C##a;
#elif CHANNELS==3
#	define DEFINE_AND_READ_PREMUL_COLOR(C,PTR) \
		emUInt32 C##r=PTR[0], C##g=PTR[1], C##b=PTR[2];
#else
#	define DEFINE_AND_READ_PREMUL_COLOR(C,PTR) \
		emUInt32 C##a=PTR[3], C##r=PTR[0]*C##a, C##g=PTR[1]*C##a, C##b=PTR[2]*C##a;
#endif


// READ_PREMUL_MUL_COLOR(C,PTR,S)
#if CHANNELS==1
#	define READ_PREMUL_MUL_COLOR(C,PTR,S) { \
		C##g=PTR[0]*S; \
	}
#elif CHANNELS==2
#	define READ_PREMUL_MUL_COLOR(C,PTR,S) { \
		C##a=PTR[1]*S; \
		C##g=PTR[0]*C##a; \
	}
#elif CHANNELS==3
#	define READ_PREMUL_MUL_COLOR(C,PTR,S) { \
		C##r=PTR[0]*S; \
		C##g=PTR[1]*S; \
		C##b=PTR[2]*S; \
	}
#else
#	define READ_PREMUL_MUL_COLOR(C,PTR,S) { \
		C##a=PTR[3]*S; \
		C##r=PTR[0]*C##a; \
		C##g=PTR[1]*C##a; \
		C##b=PTR[2]*C##a; \
	}
#endif


// COPY_COLOR(C1,C2)
#if CHANNELS==1
#	define COPY_COLOR(C1,C2) { \
		C1##g=C2##g; \
	}
#elif CHANNELS==2
#	define COPY_COLOR(C1,C2) { \
		C1##g=C2##g; \
		C1##a=C2##a; \
	}
#elif CHANNELS==3
#	define COPY_COLOR(C1,C2) { \
		C1##r=C2##r; \
		C1##g=C2##g; \
		C1##b=C2##b; \
	}
#else
#	define COPY_COLOR(C1,C2) { \
		C1##r=C2##r; \
		C1##g=C2##g; \
		C1##b=C2##b; \
		C1##a=C2##a; \
	}
#endif


// ADD_COLOR(C1,C2)
#if CHANNELS==1
#	define ADD_COLOR(C1,C2) { \
		C1##g+=C2##g; \
	}
#elif CHANNELS==2
#	define ADD_COLOR(C1,C2) { \
		C1##g+=C2##g; \
		C1##a+=C2##a; \
	}
#elif CHANNELS==3
#	define ADD_COLOR(C1,C2) { \
		C1##r+=C2##r; \
		C1##g+=C2##g; \
		C1##b+=C2##b; \
	}
#else
#	define ADD_COLOR(C1,C2) { \
		C1##r+=C2##r; \
		C1##g+=C2##g; \
		C1##b+=C2##b; \
		C1##a+=C2##a; \
	}
#endif


// SET_MUL_COLOR(C1,C2,S)
#if CHANNELS==1
#	define SET_MUL_COLOR(C1,C2,S) { \
		C1##g=C2##g*S; \
	}
#elif CHANNELS==2
#	define SET_MUL_COLOR(C1,C2,S) { \
		C1##g=C2##g*S; \
		C1##a=C2##a*S; \
	}
#elif CHANNELS==3
#	define SET_MUL_COLOR(C1,C2,S) { \
		C1##r=C2##r*S; \
		C1##g=C2##g*S; \
		C1##b=C2##b*S; \
	}
#else
#	define SET_MUL_COLOR(C1,C2,S) { \
		C1##r=C2##r*S; \
		C1##g=C2##g*S; \
		C1##b=C2##b*S; \
		C1##a=C2##a*S; \
	}
#endif


// ADD_MUL_COLOR(C1,C2,S)
#if CHANNELS==1
#	define ADD_MUL_COLOR(C1,C2,S) { \
		C1##g+=C2##g*S; \
	}
#elif CHANNELS==2
#	define ADD_MUL_COLOR(C1,C2,S) { \
		C1##g+=C2##g*S; \
		C1##a+=C2##a*S; \
	}
#elif CHANNELS==3
#	define ADD_MUL_COLOR(C1,C2,S) { \
		C1##r+=C2##r*S; \
		C1##g+=C2##g*S; \
		C1##b+=C2##b*S; \
	}
#else
#	define ADD_MUL_COLOR(C1,C2,S) { \
		C1##r+=C2##r*S; \
		C1##g+=C2##g*S; \
		C1##b+=C2##b*S; \
		C1##a+=C2##a*S; \
	}
#endif


// ADD_READ_PREMUL_COLOR(C,PTR)
#define ADD_READ_PREMUL_COLOR(C,PTR) { \
	DEFINE_AND_READ_PREMUL_COLOR(cTmp,PTR) \
	ADD_COLOR(C,cTmp) \
}


// ADD_READ_PREMUL_MUL_COLOR(C,PTR,S)
#define ADD_READ_PREMUL_MUL_COLOR(C,PTR,S) { \
	DEFINE_COLOR(cTmp) \
	READ_PREMUL_MUL_COLOR(cTmp,PTR,S) \
	ADD_COLOR(C,cTmp) \
}


// FINPREMUL_SIGNED_COLOR(C)
#if CHANNELS==1
#	define FINPREMUL_SIGNED_COLOR(C)
#elif CHANNELS==2
#	define FINPREMUL_SIGNED_COLOR(C) { \
		C##g=((emInt32)C##g+0x7f)/0xff; \
	}
#elif CHANNELS==3
#	define FINPREMUL_SIGNED_COLOR(C)
#else
#	define FINPREMUL_SIGNED_COLOR(C) { \
		C##r=((emInt32)C##r+0x7f)/0xff; \
		C##g=((emInt32)C##g+0x7f)/0xff; \
		C##b=((emInt32)C##b+0x7f)/0xff; \
	}
#endif


// FINPREMUL_SHR_COLOR(C,S)
#if CHANNELS==1
#	define FINPREMUL_SHR_COLOR(C,S) { \
		C##g=(C##g+(((1<<S)>>1)-1))>>S; \
	}
#elif CHANNELS==2
#	define FINPREMUL_SHR_COLOR(C,S) { \
		C##g=(C##g+(((0xff<<S)>>1)-1))/(0xff<<S); \
		C##a=(C##a+(((1<<S)>>1)-1))>>S; \
	}
#elif CHANNELS==3
#	define FINPREMUL_SHR_COLOR(C,S) { \
		C##r=(C##r+(((1<<S)>>1)-1))>>S; \
		C##g=(C##g+(((1<<S)>>1)-1))>>S; \
		C##b=(C##b+(((1<<S)>>1)-1))>>S; \
	}
#else
#	define FINPREMUL_SHR_COLOR(C,S) { \
		C##r=(C##r+(((0xff<<S)>>1)-1))/(0xff<<S); \
		C##g=(C##g+(((0xff<<S)>>1)-1))/(0xff<<S); \
		C##b=(C##b+(((0xff<<S)>>1)-1))/(0xff<<S); \
		C##a=(C##a+(((1<<S)>>1)-1))>>S; \
	}
#endif


// WRITE_COLOR(PTR,C)
#if CHANNELS==1
#	define WRITE_COLOR(PTR,C) { \
		PTR[0]=(emByte)C##g; \
	}
#elif CHANNELS==2
#	define WRITE_COLOR(PTR,C) { \
		PTR[0]=(emByte)C##g; \
		PTR[1]=(emByte)C##a; \
	}
#elif CHANNELS==3
#	define WRITE_COLOR(PTR,C) { \
		PTR[0]=(emByte)C##r; \
		PTR[1]=(emByte)C##g; \
		PTR[2]=(emByte)C##b; \
	}
#else
#	define WRITE_COLOR(PTR,C) { \
		PTR[0]=(emByte)C##r; \
		PTR[1]=(emByte)C##g; \
		PTR[2]=(emByte)C##b; \
		PTR[3]=(emByte)C##a; \
	}
#endif


// WRITE_SHR_CLIP_SIGNED_COLOR(PTR,C,S)
#if CHANNELS==1
#	define WRITE_SHR_CLIP_SIGNED_COLOR(PTR,C,S) { \
		emInt32 g=((emInt32)C##g+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)g>255) g=g<0?0:255; \
		PTR[0]=(emByte)g; \
	}
#elif CHANNELS==2
#	define WRITE_SHR_CLIP_SIGNED_COLOR(PTR,C,S) { \
		emInt32 a=((emInt32)C##a+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)a>255) a=a<0?0:255; \
		PTR[1]=(emByte)a; \
		emInt32 g=((emInt32)C##g+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)g>(emUInt32)a) g=g<0?0:a; \
		PTR[0]=(emByte)g; \
	}
#elif CHANNELS==3
#	define WRITE_SHR_CLIP_SIGNED_COLOR(PTR,C,S) { \
		emInt32 r=((emInt32)C##r+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)r>255) r=r<0?0:255; \
		PTR[0]=(emByte)r; \
		emInt32 g=((emInt32)C##g+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)g>255) g=g<0?0:255; \
		PTR[1]=(emByte)g; \
		emInt32 b=((emInt32)C##b+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)b>255) b=b<0?0:255; \
		PTR[2]=(emByte)b; \
	}
#else
#	define WRITE_SHR_CLIP_SIGNED_COLOR(PTR,C,S) { \
		emInt32 a=((emInt32)C##a+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)a>255) a=a<0?0:255; \
		PTR[3]=(emByte)a; \
		emInt32 r=((emInt32)C##r+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)r>(emUInt32)a) r=r<0?0:a; \
		PTR[0]=(emByte)r; \
		emInt32 g=((emInt32)C##g+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)g>(emUInt32)a) g=g<0?0:a; \
		PTR[1]=(emByte)g; \
		emInt32 b=((emInt32)C##b+(((1<<S)>>1)-1))>>S; \
		if ((emUInt32)b>(emUInt32)a) b=b<0?0:a; \
		PTR[2]=(emByte)b; \
	}
#endif


// WRITE_NO_ROUND_SHR_COLOR(PTR,C,S)
#if CHANNELS==1
#	define WRITE_NO_ROUND_SHR_COLOR(PTR,C,S) { \
		PTR[0]=(emByte)(C##g>>S); \
	}
#elif CHANNELS==2
#	define WRITE_NO_ROUND_SHR_COLOR(PTR,C,S) { \
		PTR[0]=(emByte)(C##g>>S); \
		PTR[1]=(emByte)(C##a>>S); \
	}
#elif CHANNELS==3
#	define WRITE_NO_ROUND_SHR_COLOR(PTR,C,S) { \
		PTR[0]=(emByte)(C##r>>S); \
		PTR[1]=(emByte)(C##g>>S); \
		PTR[2]=(emByte)(C##b>>S); \
	}
#else
#	define WRITE_NO_ROUND_SHR_COLOR(PTR,C,S) { \
		PTR[0]=(emByte)(C##r>>S); \
		PTR[1]=(emByte)(C##g>>S); \
		PTR[2]=(emByte)(C##b>>S); \
		PTR[3]=(emByte)(C##a>>S); \
	}
#endif


// WRITE_ZERO_COLOR(PTR)
#if CHANNELS==1
#	define WRITE_ZERO_COLOR(PTR) { \
		PTR[0]=0; \
	}
#elif CHANNELS==2
#	define WRITE_ZERO_COLOR(PTR) { \
		PTR[0]=0; \
		PTR[1]=0; \
	}
#elif CHANNELS==3
#	define WRITE_ZERO_COLOR(PTR) { \
		PTR[0]=0; \
		PTR[1]=0; \
		PTR[2]=0; \
	}
#else
#	define WRITE_ZERO_COLOR(PTR) { \
		PTR[0]=0; \
		PTR[1]=0; \
		PTR[2]=0; \
		PTR[3]=0; \
	}
#endif


//==============================================================================
//============ emPainter::ScanlineTool::InterpolateImageNearest... =============
//==============================================================================

void emPainter::ScanlineTool::CONCAT(InterpolateImageNearest,CONCAT(
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

	do {
		DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
		DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)

		DEFINE_AND_READ_PREMUL_COLOR(c,p)
		FINPREMUL_SIGNED_COLOR(c)
		WRITE_COLOR(buf,c)

		buf+=CHANNELS;
		tx+=tdx;
	} while (buf<bufEnd);
}


//==============================================================================
//========== emPainter::ScanlineTool::InterpolateImageAreaSampled... ===========
//==============================================================================

void emPainter::ScanlineTool::CONCAT(InterpolateImageAreaSampled,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
#	if EXTENSION==EXTEND_TILED
		emInt64 tx=x*sct.TDX-sct.TX;
		emUInt32 odx=sct.ODX;
		emUInt32 ox1=((0x1000000-(tx&0xffffff))*(emInt64)odx+0xffffff)>>24;
		if (odx==0x7fffffff) ox1=0x7fffffff;
		ssize_t imgDX=sct.ImgDX;
		ssize_t imgSX=sct.ImgSX;
		DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,imgDX,imgSX)

		emInt64 ty=y*sct.TDY-sct.TY;
		emUInt32 ody=sct.ODY;
		emUInt32 oy1=((0x1000000-(ty&0xffffff))*(emInt64)ody+0xffffff)>>24;
		if (oy1>=0x10000 || ody==0x7fffffff) oy1=0x10000;
		emUInt32 oy1n=0x10000-oy1;
		ssize_t imgDY=sct.ImgDY;
		ssize_t imgSY=sct.ImgSY;
		DEFINE_AND_SET_IMAGE_Y(imgY1,ty>>24,imgDY,imgSY)

		emByte * buf=(emByte*)sct.InterpolationBuffer;
		emByte * bufEnd=buf+w*CHANNELS;
		const emByte * imgMap=sct.ImgMap;

		DEFINE_AND_SET_COLOR(cy,0)

		emUInt32 ox=0;
		do {
			DEFINE_AND_SET_COLOR(cyx,0x7fffff)

			emUInt32 oxs=0x10000;
			while (ox<oxs) {
				ADD_MUL_COLOR(cyx,cy,ox)
				oxs-=ox;

				DEFINE_AND_COPY_IMAGE_Y(imgY,imgY1)

				{
					DEFINE_AND_SET_IMAGE_ROW_PTR(row,imgY,imgSX,imgSY,imgMap)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
					READ_PREMUL_MUL_COLOR(cy,p,oy1)
				}

				emUInt32 oys=oy1n;
				if (oys>0) {
					INCREMENT_IMAGE_Y(imgY,imgDY,imgSY)
					if (oys>ody) {
						DEFINE_AND_SET_COLOR(ctmp,0)
						do {
							DEFINE_AND_SET_IMAGE_ROW_PTR(row,imgY,imgSX,imgSY,imgMap)
							DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
							ADD_READ_PREMUL_COLOR(ctmp,p)
							INCREMENT_IMAGE_Y(imgY,imgDY,imgSY)
							oys-=ody;
						} while (oys>ody);
						ADD_MUL_COLOR(cy,ctmp,ody)
					}
					DEFINE_AND_SET_IMAGE_ROW_PTR(row,imgY,imgSX,imgSY,imgMap)
					DEFINE_AND_SET_IMAGE_PIX_PTR(p,row,imgX)
					ADD_READ_PREMUL_MUL_COLOR(cy,p,oys)
				}

				FINPREMUL_SHR_COLOR(cy,8)
				INCREMENT_IMAGE_X(imgX,imgDX,imgSX)

				ox=ox1;
				ox1=odx;
			}
			ADD_MUL_COLOR(cyx,cy,oxs)
			WRITE_NO_ROUND_SHR_COLOR(buf,cyx,24)
			buf+=CHANNELS;
			ox-=oxs;
		} while (buf<bufEnd);
#	else
		// Note: Here, for the non-tiled case, the calculations at the
		// edges are made for perfect quality when painting with
		// PaintImage(..), i.e. when the output rectangle (or polygon
		// etc.) doesn't show anything "outside" the image.

		emByte * buf=(emByte*)sct.InterpolationBuffer;
		emByte * bufEnd=buf+w*CHANNELS;

		emInt64 ty1=y*sct.TDY-sct.TY;
		emInt64 ty2=ty1+sct.TDY;
		emInt64 tyEnd=((emInt64)sct.ImgH)<<24;
		emUInt32 ody=sct.ODY;
		if (ty1<0) {
			if (ty2<=0) {
#				if EXTENSION==EXTEND_EDGE
					ty2=1<<24;
#				else
					do {
						WRITE_ZERO_COLOR(buf)
						buf+=CHANNELS;
					} while (buf<bufEnd);
					return;
#				endif
			}
			else if (ty2>tyEnd) ty2=tyEnd;
			ty1=0;
			ody = ty2<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/ty2+1;
		}
		else if (ty2>tyEnd) {
			if (ty1>=tyEnd) {
#				if EXTENSION==EXTEND_EDGE
					ty1=tyEnd-(1<<24);
#				else
					do {
						WRITE_ZERO_COLOR(buf)
						buf+=CHANNELS;
					} while (buf<bufEnd);
					return;
#				endif
			}
			emInt64 t=tyEnd-ty1;
			ody = t<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/t+1;
		}
		emUInt32 oy1=((0x1000000-(ty1&0xffffff))*(emInt64)ody+0xffffff)>>24;
		if (oy1>=0x10000 || ody==0x7fffffff) oy1=0x10000;
		emUInt32 oy1n=0x10000-oy1;
		ssize_t imgDY=sct.ImgDY;
		const emByte * row0=sct.ImgMap+(ty1>>24)*imgDY;

		emInt64 tdx=sct.TDX;
		emInt64 tx=x*tdx-sct.TX;
		emInt64 txEnd=((emInt64)sct.ImgW)<<24;
		DEFINE_AND_SET_COLOR(cy,0)
		const emByte * pCy=NULL;
		ssize_t imgDX=sct.ImgDX;
		emUInt32 odx0=sct.ODX;

		do {
			emInt64 tx1=tx;
			emInt64 tx2=tx+tdx;
			emUInt32 odx;
			emInt64 txStop;
			if (tx1<0) {
				tx1=0;
				if (tx2<=0) {
#					if EXTENSION==EXTEND_EDGE
						tx2=1<<24;
#					else
						WRITE_ZERO_COLOR(buf)
						buf+=CHANNELS;
						tx+=tdx;
						continue;
#					endif
				}
				else if (tx2>txEnd) tx2=txEnd;
				odx = tx2<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/tx2+1;
				txStop=tx;
			}
			else if (tx2>txEnd) {
				if (tx1>=txEnd) {
#					if EXTENSION==EXTEND_EDGE
						tx1=txEnd-(1<<24);
#					else
						WRITE_ZERO_COLOR(buf)
						buf+=CHANNELS;
						tx+=tdx;
						continue;
#					endif
				}
				emInt64 t=txEnd-tx1;
				odx = t<=0x200 ? 0x7fffffff : (((emInt64)1<<40)-1)/t+1;
				txStop=tx;
			}
			else {
				odx=odx0;
				txStop=txEnd-tdx+1;
				emInt64 t=tx+((bufEnd-buf)/CHANNELS)*tdx;
				if (txStop>t) txStop=t;
			}
			emUInt32 ox=((0x1000000-(tx1&0xffffff))*(emInt64)odx+0xffffff)>>24;
			if (odx==0x7fffffff) ox=0x7fffffff;
			const emByte * p0=row0+(tx1>>24)*imgDX;
			emUInt32 ox1;
			if (pCy!=p0) {
				ox1=ox;
				ox=0;
			}
			else {
				ox1=odx;
				p0+=imgDX;
			}

			do {
				DEFINE_AND_SET_COLOR(cyx,0x7fffff)
				emUInt32 oxs=0x10000;
				while (ox<oxs) {
					ADD_MUL_COLOR(cyx,cy,ox)
					oxs-=ox;
					pCy=p0;
					const emByte * p = p0;
					READ_PREMUL_MUL_COLOR(cy,p,oy1)
					emUInt32 oys=oy1n;
					if (oys>0) {
						p += imgDY;
						if (oys>ody) {
							DEFINE_AND_READ_PREMUL_COLOR(ctmp,p)
							p += imgDY;
							oys-=ody;
							while (oys>ody) {
								ADD_READ_PREMUL_COLOR(ctmp,p)
								p += imgDY;
								oys-=ody;
							}
							ADD_MUL_COLOR(cy,ctmp,ody)
						}
						ADD_READ_PREMUL_MUL_COLOR(cy,p,oys)
					}
					FINPREMUL_SHR_COLOR(cy,8)
					p0+=imgDX;
					ox=ox1;
					ox1=odx;
				}
				ADD_MUL_COLOR(cyx,cy, oxs)
				WRITE_NO_ROUND_SHR_COLOR(buf,cyx,24)
				buf+=CHANNELS;
				ox-=oxs;
				tx+=tdx;
			} while (tx<txStop);
		} while (buf<bufEnd);
#	endif
}


//==============================================================================
//============ emPainter::ScanlineTool::InterpolateImageBilinear... ============
//==============================================================================

void emPainter::ScanlineTool::CONCAT(InterpolateImageBilinear,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x800000;
	emUInt32 oy1=((ty&0xffffff)+0x7fff)>>16;
	emUInt32 oy0=256-oy1;

	DEFINE_AND_SET_IMAGE_Y(imgY,ty>>24,sct.ImgDY,sct.ImgSY)
	ssize_t imgSX=sct.ImgSX;
	DEFINE_AND_SET_IMAGE_ROW_PTR(row0,imgY,imgSX,sct.ImgSY,sct.ImgMap)
	INCREMENT_IMAGE_Y(imgY,sct.ImgDY,sct.ImgSY)
	DEFINE_AND_SET_IMAGE_ROW_PTR(row1,imgY,imgSX,sct.ImgSY,sct.ImgMap)

	emInt64 tdx=sct.TDX;
	emInt64 tx=x*tdx-sct.TX-0x1800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
	DEFINE_AND_SET_COLOR(c0,0)
	DEFINE_AND_SET_COLOR(c1,0)

	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;
	tx=(tx&0xffffff)+0x1000000;

	do {
		while (tx>=0) {
			tx-=0x1000000;
			INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)
			COPY_COLOR(c0,c1)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			READ_PREMUL_MUL_COLOR    (c1,p0,oy0)
			ADD_READ_PREMUL_MUL_COLOR(c1,p1,oy1)
		}

		emUInt32 ox1=(tx+0x1007fff)>>16;
		emUInt32 ox0=256-ox1;
		DEFINE_COLOR(c)
		SET_MUL_COLOR(c,c0,ox0)
		ADD_MUL_COLOR(c,c1,ox1)

		FINPREMUL_SHR_COLOR(c,16)
		WRITE_COLOR(buf,c)

		buf+=CHANNELS;
		tx+=tdx;
	} while (buf<bufEnd);
}


//==============================================================================
//============ emPainter::ScanlineTool::InterpolateImageBicubic... =============
//==============================================================================

#ifndef BICUBIC_FACTORS_TABLE_DEFINED
#	define BICUBIC_FACTORS_TABLE_DEFINED
	struct BicubicFactors {
		emInt16 f1;
		emInt16 f2;
		emInt8 f0;
		emInt8 f3;
	};
	static const BicubicFactors BicubicFactorsTable[257] = {
		// #include <stdio.h>
		// #include <math.h>
		// int main(int argc, char * argv[])
		// {
		//   for (int i=0; i<=256; i++) {
		//     double o=i/256.0;
		//     double s=1.0-o;
		//     double f=1024;
		//     int f0=(int)round((-0.5*s*o)*s*f);
		//     int f1=(int)round(((1 - 1.5*o)*o + 1)*s*f);
		//     int f2=(int)round(((1 - 1.5*s)*s + 1)*o*f);
		//     int f3=(int)round((-0.5*s*o)*o*f);
		//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",f1,f2,f0,f3);
		//   }
		//   return 0;
		// }
		{1024,0,0,0},{1024,2,-2,0},{1024,4,-4,0},{1024,6,-6,0},
		{1023,8,-8,0},{1023,11,-10,0},{1023,13,-11,0},{1022,15,-13,0},
		{1022,18,-15,0},{1021,20,-17,-1},{1020,23,-18,-1},{1019,26,-20,-1},
		{1019,28,-22,-1},{1018,31,-23,-1},{1017,34,-25,-1},{1016,37,-27,-2},
		{1014,40,-28,-2},{1013,43,-30,-2},{1012,46,-31,-2},{1011,49,-33,-3},
		{1009,52,-34,-3},{1008,55,-35,-3},{1006,58,-37,-3},{1004,61,-38,-4},
		{1003,65,-39,-4},{1001,68,-41,-4},{999,72,-42,-5},{997,75,-43,-5},
		{995,78,-44,-5},{993,82,-46,-6},{991,86,-47,-6},{989,89,-48,-7},
		{987,93,-49,-7},{985,97,-50,-7},{982,101,-51,-8},{980,104,-52,-8},
		{978,108,-53,-9},{975,112,-54,-9},{973,116,-55,-10},{970,120,-56,-10},
		{967,124,-57,-11},{965,128,-58,-11},{962,132,-59,-12},{959,137,-60,-12},
		{956,141,-60,-13},{953,145,-61,-13},{950,149,-62,-14},{947,154,-63,-14},
		{944,158,-63,-15},{941,162,-64,-15},{938,167,-65,-16},{935,171,-65,-16},
		{931,176,-66,-17},{928,180,-67,-17},{925,185,-67,-18},{921,189,-68,-19},
		{918,194,-68,-19},{914,199,-69,-20},{910,203,-69,-20},{907,208,-70,-21},
		{903,213,-70,-22},{899,218,-71,-22},{896,222,-71,-23},{892,227,-72,-23},
		{888,232,-72,-24},{884,237,-72,-25},{880,242,-73,-25},{876,247,-73,-26},
		{872,252,-73,-27},{868,257,-74,-27},{864,262,-74,-28},{860,267,-74,-28},
		{856,272,-74,-29},{851,277,-75,-30},{847,282,-75,-30},{843,287,-75,-31},
		{839,292,-75,-32},{834,297,-75,-32},{830,303,-75,-33},{825,308,-76,-34},
		{821,313,-76,-34},{816,318,-76,-35},{812,324,-76,-36},{807,329,-76,-36},
		{803,334,-76,-37},{798,340,-76,-38},{793,345,-76,-38},{789,350,-76,-39},
		{784,356,-76,-40},{779,361,-76,-40},{774,366,-76,-41},{770,372,-76,-42},
		{765,377,-76,-42},{760,383,-75,-43},{755,388,-75,-44},{750,394,-75,-44},
		{745,399,-75,-45},{740,404,-75,-46},{735,410,-75,-46},{730,415,-74,-47},
		{725,421,-74,-48},{720,426,-74,-48},{715,432,-74,-49},{710,437,-74,-50},
		{704,443,-73,-50},{699,449,-73,-51},{694,454,-73,-51},{689,460,-72,-52},
		{684,465,-72,-53},{678,471,-72,-53},{673,476,-72,-54},{668,482,-71,-55},
		{663,487,-71,-55},{657,493,-71,-56},{652,498,-70,-56},{647,504,-70,-57},
		{641,510,-69,-57},{636,515,-69,-58},{631,521,-69,-59},{625,526,-68,-59},
		{620,532,-68,-60},{614,537,-67,-60},{609,543,-67,-61},{603,548,-66,-61},
		{598,554,-66,-62},{592,559,-65,-62},{587,565,-65,-63},{581,570,-64,-63},
		{576,576,-64,-64},{570,581,-63,-64},{565,587,-63,-65},{559,592,-62,-65},
		{554,598,-62,-66},{548,603,-61,-66},{543,609,-61,-67},{537,614,-60,-67},
		{532,620,-60,-68},{526,625,-59,-68},{521,631,-59,-69},{515,636,-58,-69},
		{510,641,-57,-69},{504,647,-57,-70},{498,652,-56,-70},{493,657,-56,-71},
		{487,663,-55,-71},{482,668,-55,-71},{476,673,-54,-72},{471,678,-53,-72},
		{465,684,-53,-72},{460,689,-52,-72},{454,694,-51,-73},{449,699,-51,-73},
		{443,704,-50,-73},{437,710,-50,-74},{432,715,-49,-74},{426,720,-48,-74},
		{421,725,-48,-74},{415,730,-47,-74},{410,735,-46,-75},{404,740,-46,-75},
		{399,745,-45,-75},{394,750,-44,-75},{388,755,-44,-75},{383,760,-43,-75},
		{377,765,-42,-76},{372,770,-42,-76},{366,774,-41,-76},{361,779,-40,-76},
		{356,784,-40,-76},{350,789,-39,-76},{345,793,-38,-76},{340,798,-38,-76},
		{334,803,-37,-76},{329,807,-36,-76},{324,812,-36,-76},{318,816,-35,-76},
		{313,821,-34,-76},{308,825,-34,-76},{303,830,-33,-75},{297,834,-32,-75},
		{292,839,-32,-75},{287,843,-31,-75},{282,847,-30,-75},{277,851,-30,-75},
		{272,856,-29,-74},{267,860,-28,-74},{262,864,-28,-74},{257,868,-27,-74},
		{252,872,-27,-73},{247,876,-26,-73},{242,880,-25,-73},{237,884,-25,-72},
		{232,888,-24,-72},{227,892,-23,-72},{222,896,-23,-71},{218,899,-22,-71},
		{213,903,-22,-70},{208,907,-21,-70},{203,910,-20,-69},{199,914,-20,-69},
		{194,918,-19,-68},{189,921,-19,-68},{185,925,-18,-67},{180,928,-17,-67},
		{176,931,-17,-66},{171,935,-16,-65},{167,938,-16,-65},{162,941,-15,-64},
		{158,944,-15,-63},{154,947,-14,-63},{149,950,-14,-62},{145,953,-13,-61},
		{141,956,-13,-60},{137,959,-12,-60},{132,962,-12,-59},{128,965,-11,-58},
		{124,967,-11,-57},{120,970,-10,-56},{116,973,-10,-55},{112,975,-9,-54},
		{108,978,-9,-53},{104,980,-8,-52},{101,982,-8,-51},{97,985,-7,-50},
		{93,987,-7,-49},{89,989,-7,-48},{86,991,-6,-47},{82,993,-6,-46},
		{78,995,-5,-44},{75,997,-5,-43},{72,999,-5,-42},{68,1001,-4,-41},
		{65,1003,-4,-39},{61,1004,-4,-38},{58,1006,-3,-37},{55,1008,-3,-35},
		{52,1009,-3,-34},{49,1011,-3,-33},{46,1012,-2,-31},{43,1013,-2,-30},
		{40,1014,-2,-28},{37,1016,-2,-27},{34,1017,-1,-25},{31,1018,-1,-23},
		{28,1019,-1,-22},{26,1019,-1,-20},{23,1020,-1,-18},{20,1021,-1,-17},
		{18,1022,0,-15},{15,1022,0,-13},{13,1023,0,-11},{11,1023,0,-10},
		{8,1023,0,-8},{6,1024,0,-6},{4,1024,0,-4},{2,1024,0,-2},
		{0,1024,0,0}
	};
#endif


void emPainter::ScanlineTool::CONCAT(InterpolateImageBicubic,CONCAT(
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
	emInt64 tx=x*tdx-sct.TX-0x2800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
	DEFINE_AND_SET_COLOR(c0,0)
	DEFINE_AND_SET_COLOR(c1,0)
	DEFINE_AND_SET_COLOR(c2,0)
	DEFINE_AND_SET_COLOR(c3,0)

	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;
	tx=(tx&0xffffff)+0x3000000;

	do {
		while (tx>=0) {
			tx-=0x1000000;
			INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)

			COPY_COLOR(c0,c1)
			COPY_COLOR(c1,c2)
			COPY_COLOR(c2,c3)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			READ_PREMUL_MUL_COLOR(c3,p0,fy.f0)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p1,fy.f1)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p2,fy.f2)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p3,fy.f3)

			FINPREMUL_SIGNED_COLOR(c3)
		}

		emUInt32 ox=(tx+0x1007fff)>>16;
		const BicubicFactors & fx=BicubicFactorsTable[ox];
		DEFINE_COLOR(c)
		SET_MUL_COLOR(c,c0,fx.f0)
		ADD_MUL_COLOR(c,c1,fx.f1)
		ADD_MUL_COLOR(c,c2,fx.f2)
		ADD_MUL_COLOR(c,c3,fx.f3)

		WRITE_SHR_CLIP_SIGNED_COLOR(buf,c,20)

		buf+=CHANNELS;
		tx+=tdx;
	} while (buf<bufEnd);
}


//==============================================================================
//============ emPainter::ScanlineTool::InterpolateImageLanczos... =============
//==============================================================================

#ifndef LANCZOS_FACTORS_TABLE_DEFINED
#	define LANCZOS_FACTORS_TABLE_DEFINED
	struct LanczosFactors {
		emInt16 f1;
		emInt16 f2;
		emInt16 f0;
		emInt16 f3;
	};
	static const LanczosFactors LanczosFactorsTable[257] = {
		// #include <stdio.h>
		// #include <math.h>
		// int main(int argc, char * argv[])
		// {
		//   for (int i=0; i<=256; i++) {
		//     double f=1024;
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
		//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",f1,f2,f0,f3);
		//   }
		//   return 0;
		// }
		{1024,0,0,0},{1024,3,-3,0},{1025,6,-6,-1},{1025,9,-9,-1},
		{1025,12,-12,-2},{1026,16,-15,-2},{1026,19,-18,-3},{1026,22,-20,-4},
		{1026,26,-23,-4},{1026,29,-26,-5},{1025,32,-29,-5},{1025,36,-31,-6},
		{1025,39,-34,-6},{1025,43,-36,-7},{1024,46,-39,-8},{1024,50,-41,-8},
		{1023,54,-44,-9},{1023,57,-46,-9},{1022,61,-49,-10},{1021,65,-51,-11},
		{1020,68,-53,-11},{1019,72,-56,-12},{1019,76,-58,-13},{1018,80,-60,-13},
		{1016,84,-62,-14},{1015,88,-64,-15},{1014,92,-67,-16},{1013,96,-69,-16},
		{1012,100,-71,-17},{1010,104,-73,-18},{1009,108,-75,-19},{1007,113,-77,-19},
		{1006,117,-78,-20},{1004,121,-80,-21},{1002,125,-82,-22},{1001,130,-84,-23},
		{999,134,-86,-23},{997,139,-87,-24},{995,143,-89,-25},{993,147,-91,-26},
		{991,152,-92,-27},{989,157,-94,-28},{987,161,-95,-29},{984,166,-97,-29},
		{982,170,-98,-30},{980,175,-100,-31},{977,180,-101,-32},{975,184,-102,-33},
		{972,189,-104,-34},{970,194,-105,-35},{967,199,-106,-36},{964,204,-107,-37},
		{962,208,-108,-38},{959,213,-109,-39},{956,218,-111,-40},{953,223,-112,-40},
		{950,228,-113,-41},{947,233,-114,-42},{944,238,-115,-43},{941,243,-116,-44},
		{937,248,-116,-45},{934,253,-117,-46},{931,259,-118,-47},{928,264,-119,-48},
		{924,269,-120,-49},{921,274,-120,-50},{917,279,-121,-51},{914,284,-122,-52},
		{910,290,-122,-53},{906,295,-123,-54},{903,300,-123,-56},{899,306,-124,-57},
		{895,311,-125,-58},{891,316,-125,-59},{887,322,-125,-60},{883,327,-126,-61},
		{879,333,-126,-62},{875,338,-127,-63},{871,343,-127,-64},{867,349,-127,-65},
		{863,354,-127,-66},{859,360,-128,-67},{855,365,-128,-68},{850,371,-128,-69},
		{846,376,-128,-70},{842,382,-128,-71},{837,387,-128,-72},{833,393,-129,-73},
		{828,399,-129,-74},{824,404,-129,-75},{819,410,-129,-76},{815,415,-129,-77},
		{810,421,-128,-79},{805,427,-128,-80},{801,432,-128,-81},{796,438,-128,-82},
		{791,444,-128,-83},{786,449,-128,-84},{781,455,-128,-85},{777,461,-127,-86},
		{772,466,-127,-87},{767,472,-127,-88},{762,478,-127,-89},{757,483,-126,-90},
		{752,489,-126,-91},{747,495,-126,-92},{742,500,-125,-93},{737,506,-125,-94},
		{731,512,-124,-95},{726,517,-124,-96},{721,523,-123,-97},{716,529,-123,-98},
		{711,534,-123,-99},{705,540,-122,-99},{700,546,-121,-100},{695,551,-121,-101},
		{690,557,-120,-102},{684,563,-120,-103},{679,568,-119,-104},{673,574,-119,-105},
		{668,580,-118,-106},{663,585,-117,-107},{657,591,-117,-107},{652,596,-116,-108},
		{646,602,-115,-109},{641,608,-114,-110},{635,613,-114,-111},{630,619,-113,-111},
		{624,624,-112,-112},{619,630,-111,-113},{613,635,-111,-114},{608,641,-110,-114},
		{602,646,-109,-115},{596,652,-108,-116},{591,657,-107,-117},{585,663,-107,-117},
		{580,668,-106,-118},{574,673,-105,-119},{568,679,-104,-119},{563,684,-103,-120},
		{557,690,-102,-120},{551,695,-101,-121},{546,700,-100,-121},{540,705,-99,-122},
		{534,711,-99,-123},{529,716,-98,-123},{523,721,-97,-123},{517,726,-96,-124},
		{512,731,-95,-124},{506,737,-94,-125},{500,742,-93,-125},{495,747,-92,-126},
		{489,752,-91,-126},{483,757,-90,-126},{478,762,-89,-127},{472,767,-88,-127},
		{466,772,-87,-127},{461,777,-86,-127},{455,781,-85,-128},{449,786,-84,-128},
		{444,791,-83,-128},{438,796,-82,-128},{432,801,-81,-128},{427,805,-80,-128},
		{421,810,-79,-128},{415,815,-77,-129},{410,819,-76,-129},{404,824,-75,-129},
		{399,828,-74,-129},{393,833,-73,-129},{387,837,-72,-128},{382,842,-71,-128},
		{376,846,-70,-128},{371,850,-69,-128},{365,855,-68,-128},{360,859,-67,-128},
		{354,863,-66,-127},{349,867,-65,-127},{343,871,-64,-127},{338,875,-63,-127},
		{333,879,-62,-126},{327,883,-61,-126},{322,887,-60,-125},{316,891,-59,-125},
		{311,895,-58,-125},{306,899,-57,-124},{300,903,-56,-123},{295,906,-54,-123},
		{290,910,-53,-122},{284,914,-52,-122},{279,917,-51,-121},{274,921,-50,-120},
		{269,924,-49,-120},{264,928,-48,-119},{259,931,-47,-118},{253,934,-46,-117},
		{248,937,-45,-116},{243,941,-44,-116},{238,944,-43,-115},{233,947,-42,-114},
		{228,950,-41,-113},{223,953,-40,-112},{218,956,-40,-111},{213,959,-39,-109},
		{208,962,-38,-108},{204,964,-37,-107},{199,967,-36,-106},{194,970,-35,-105},
		{189,972,-34,-104},{184,975,-33,-102},{180,977,-32,-101},{175,980,-31,-100},
		{170,982,-30,-98},{166,984,-29,-97},{161,987,-29,-95},{157,989,-28,-94},
		{152,991,-27,-92},{147,993,-26,-91},{143,995,-25,-89},{139,997,-24,-87},
		{134,999,-23,-86},{130,1001,-23,-84},{125,1002,-22,-82},{121,1004,-21,-80},
		{117,1006,-20,-78},{113,1007,-19,-77},{108,1009,-19,-75},{104,1010,-18,-73},
		{100,1012,-17,-71},{96,1013,-16,-69},{92,1014,-16,-67},{88,1015,-15,-64},
		{84,1016,-14,-62},{80,1018,-13,-60},{76,1019,-13,-58},{72,1019,-12,-56},
		{68,1020,-11,-53},{65,1021,-11,-51},{61,1022,-10,-49},{57,1023,-9,-46},
		{54,1023,-9,-44},{50,1024,-8,-41},{46,1024,-8,-39},{43,1025,-7,-36},
		{39,1025,-6,-34},{36,1025,-6,-31},{32,1025,-5,-29},{29,1026,-5,-26},
		{26,1026,-4,-23},{22,1026,-4,-20},{19,1026,-3,-18},{16,1026,-2,-15},
		{12,1025,-2,-12},{9,1025,-1,-9},{6,1025,-1,-6},{3,1024,0,-3},
		{0,1024,0,0}
	};
#endif


void emPainter::ScanlineTool::CONCAT(InterpolateImageLanczos,CONCAT(
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
	emInt64 tx=x*tdx-sct.TX-0x2800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
	DEFINE_AND_SET_COLOR(c0,0)
	DEFINE_AND_SET_COLOR(c1,0)
	DEFINE_AND_SET_COLOR(c2,0)
	DEFINE_AND_SET_COLOR(c3,0)

	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;
	tx=(tx&0xffffff)+0x3000000;

	do {
		while (tx>=0) {
			tx-=0x1000000;
			INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)

			COPY_COLOR(c0,c1)
			COPY_COLOR(c1,c2)
			COPY_COLOR(c2,c3)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			READ_PREMUL_MUL_COLOR(c3,p0,fy.f0)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p1,fy.f1)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p2,fy.f2)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			ADD_READ_PREMUL_MUL_COLOR(c3,p3,fy.f3)

			FINPREMUL_SIGNED_COLOR(c3)
		}

		emUInt32 ox=(tx+0x1007fff)>>16;
		const LanczosFactors & fx=LanczosFactorsTable[ox];
		DEFINE_COLOR(c)
		SET_MUL_COLOR(c,c0,fx.f0)
		ADD_MUL_COLOR(c,c1,fx.f1)
		ADD_MUL_COLOR(c,c2,fx.f2)
		ADD_MUL_COLOR(c,c3,fx.f3)

		WRITE_SHR_CLIP_SIGNED_COLOR(buf,c,20)

		buf+=CHANNELS;
		tx+=tdx;
	} while (buf<bufEnd);
}


//==============================================================================
//============ emPainter::ScanlineTool::InterpolateImageAdaptive... ============
//==============================================================================

//----------------- Subroutine: InterpolateFourValuesAdaptive ------------------

#ifndef INTERPOLATE_FOUR_VALUES_ADAPTIVE_DEFINED
#	define INTERPOLATE_FOUR_VALUES_ADAPTIVE_DEFINED
	static int InterpolateFourValuesAdaptive(
		int v0, int v1, int v2, int v3, emUInt32 o
	)
	{

#		if 0 // Original ("reference") implementation.

			struct Hlp {

				static double AdaptPeak(double s0, double s1)
				{
					if (s1<0.0) {
						if (s0>0.0) {
							if (s0 >= -s1) {
								return emMin(-s1,(s0-s1)/16);
							}
							else {
								return emMin(s0,(s0-s1)/16);
							}
						}
					}
					else if (s1>0.0) {
						if (s0<0.0) {
							if (s0 >= -s1) {
								return -emMin(-s0,(s1-s0)/16);
							}
							else {
								return -emMin(s1,(s1-s0)/16);
							}
						}
					}
					return 0.0;
				}

				static double AdaptSlope(double s0, double s1, double s2)
				{
					double n=1.0;
					if (s0<0.0) { n=-1.0; s0=-s0; s1=-s1; s2=-s2; }
					if (s1<=0.0) return 0.0;
					double s=emMax(emMin(s0,s1+s1), emMin(s0+s0,s1));
					double q=s-s2-s2;
					if (q>0) s+=emMin(s,q);
					return s*n;
				}
			};

			double s0 = v1-v0;
			double s1 = v2-v1;
			double s2 = v3-v2;
			double av1 = v1 + Hlp::AdaptPeak(s0,s1);
			double av2 = v2 + Hlp::AdaptPeak(-s2,-s1);

			// No matter whether doing this here or not:
			// s0 = av1-v0; s1 = av2-av1; s2 = v3-av2;

			double as1 = Hlp::AdaptSlope(s0,s1,s2);
			double as2 = -Hlp::AdaptSlope(-s2,-s1,-s0);

			// y = a*x*x + b*x + c
			double a1=av2-av1-as1;
			double b1=as1;
			double c1=av1;
			double a2=av1-av2+as2;
			double b2=-as2;
			double c2=av2;
			double rx=o/256.0;
			double sx=1.0-rx;
			double y1 = (a1*rx+b1)*rx+c1;
			double y2 = (a2*sx+b2)*sx+c2;
			double v=y1*sx+y2*rx;
			return (int)(v*1024.0+0.5);

#		else // Optimized implementation.

			int s01 = v1-v0;
			int s12 = v2-v1;
			int s32 = v2-v3;

			int s1=0;
			int s2=0;
			if (s12<0) {
				if (s01<0) {
					s1=s01<<1;
					if (s1<s12) s1=s12;
					int t=s12<<1;
					if (t<s01) t=s01;
					if (s1>t) s1=t;
					int q=s1+(s32<<1);
					if (q<0) s1+=q>s1?q:s1;
				}
				else if (s01>0) {
					int s21 = -s12;
					int t=(s01+s21+7)>>4;
					int s=s21<s01?s21:s01;
					v1+=s<t?s:t;
				}
				if (s32>0) {
					int s23=-s32;
					s2=s23<<1;
					if (s2<s12) s2=s12;
					int t=s12<<1;
					if (t<s23) t=s23;
					if (s2>t) s2=t;
					int q=s2-(s01<<1);
					if (q<0) s2+=q>s2?q:s2;
				}
				else if (s32<0) {
					int t=(s12+s32+7)>>4;
					int s=s12>s32?s12:s32;
					v2+=s>t?s:t;
				}
			}
			else if (s12>0) {
				if (s01>0) {
					s1=s01<<1;
					if (s1>s12) s1=s12;
					int t=s12<<1;
					if (t>s01) t=s01;
					if (s1<t) s1=t;
					int q=s1+(s32<<1);
					if (q>0) s1+=q<s1?q:s1;
				}
				else if (s01<0) {
					int s21 = -s12;
					int t=(s21+s01+7)>>4;
					int s=s21>s01?s21:s01;
					v1+=s>t?s:t;
				}
				if (s32<0) {
					int s23=-s32;
					s2=s23<<1;
					if (s2>s12) s2=s12;
					int t=s12<<1;
					if (t>s23) t=s23;
					if (s2<t) s2=t;
					int q=s2-(s01<<1);
					if (q>0) s2+=q<s2?q:s2;
				}
				else if (s32>0) {
					int t=(s32+s12+7)>>4;
					int s=s12<s32?s12:s32;
					v2+=s<t?s:t;
				}
			}

			struct Factors {
				emInt16 fv1;
				emInt16 fv2;
				emInt16 fs1;
				emInt16 fs2;
			};
			static const Factors FactorsTable[257] = {
				// #include <stdio.h>
				// #include <math.h>
				// int main(int argc, char * argv[])
				// {
				//   for (int i=0; i<=256; i++) {
				//     double f=1024.0;
				//     double o=i/256.0;
				//     int fv1=(int)round((2*o*o*o-3*o*o+1)*f);
				//     int fv2=(int)round((-2*o*o*o+3*o*o)*f);
				//     int fs1=(int)round((o*o*o-2*o*o+o)*f);
				//     int fs2=(int)round((o*o*o-o*o)*f);
				//     printf("%s{%d,%d,%d,%d},",i%4?"":"\n",fv1,fv2,fs1,fs2);
				//   }
				//   return 0;
				// }
				{1024,0,0,0},{1024,0,4,0},{1024,0,8,0},{1024,0,12,0},
				{1023,1,16,0},{1023,1,19,0},{1022,2,23,-1},{1022,2,26,-1},
				{1021,3,30,-1},{1020,4,34,-1},{1019,5,37,-2},{1018,6,40,-2},
				{1017,7,44,-2},{1016,8,47,-3},{1015,9,50,-3},{1014,10,53,-3},
				{1013,12,56,-4},{1011,13,59,-4},{1010,14,62,-5},{1008,16,65,-5},
				{1006,18,68,-6},{1004,20,71,-6},{1003,21,74,-7},{1001,23,76,-8},
				{999,25,79,-8},{997,27,81,-9},{994,30,84,-9},{992,32,86,-10},
				{990,34,89,-11},{988,36,91,-12},{985,39,94,-12},{983,41,96,-13},
				{980,44,98,-14},{977,47,100,-15},{975,49,102,-16},{972,52,104,-17},
				{969,55,106,-17},{966,58,108,-18},{963,61,110,-19},{960,64,112,-20},
				{957,67,114,-21},{954,70,116,-22},{950,74,117,-23},{947,77,119,-24},
				{944,80,121,-25},{940,84,122,-26},{937,87,124,-27},{933,91,125,-28},
				{930,95,127,-29},{926,98,128,-30},{922,102,130,-31},{918,106,131,-33},
				{914,110,132,-34},{911,113,133,-35},{907,117,134,-36},{903,121,136,-37},
				{898,126,137,-38},{894,130,138,-39},{890,134,139,-41},{886,138,140,-42},
				{882,142,141,-43},{877,147,142,-44},{873,151,142,-46},{868,156,143,-47},
				{864,160,144,-48},{859,165,145,-49},{855,169,145,-51},{850,174,146,-52},
				{846,178,147,-53},{841,183,147,-54},{836,188,148,-56},{831,193,148,-57},
				{827,197,149,-58},{822,202,149,-60},{817,207,150,-61},{812,212,150,-62},
				{807,217,150,-63},{802,222,151,-65},{797,227,151,-66},{792,232,151,-67},
				{787,238,151,-69},{781,243,151,-70},{776,248,152,-71},{771,253,152,-73},
				{766,258,152,-74},{760,264,152,-75},{755,269,152,-77},{750,274,152,-78},
				{744,280,152,-79},{739,285,151,-81},{733,291,151,-82},{728,296,151,-83},
				{722,302,151,-85},{717,307,151,-86},{711,313,151,-87},{706,318,150,-89},
				{700,324,150,-90},{694,330,150,-91},{689,335,149,-93},{683,341,149,-94},
				{677,347,149,-95},{672,352,148,-97},{666,358,148,-98},{660,364,147,-99},
				{654,370,147,-100},{649,375,146,-102},{643,381,146,-103},{637,387,145,-104},
				{631,393,144,-105},{625,399,144,-107},{619,405,143,-108},{613,411,142,-109},
				{608,417,142,-110},{602,422,141,-111},{596,428,140,-113},{590,434,140,-114},
				{584,440,139,-115},{578,446,138,-116},{572,452,137,-117},{566,458,136,-118},
				{560,464,135,-120},{554,470,135,-121},{548,476,134,-122},{542,482,133,-123},
				{536,488,132,-124},{530,494,131,-125},{524,500,130,-126},{518,506,129,-127},
				{512,512,128,-128},{506,518,127,-129},{500,524,126,-130},{494,530,125,-131},
				{488,536,124,-132},{482,542,123,-133},{476,548,122,-134},{470,554,121,-135},
				{464,560,120,-135},{458,566,118,-136},{452,572,117,-137},{446,578,116,-138},
				{440,584,115,-139},{434,590,114,-140},{428,596,113,-140},{422,602,111,-141},
				{417,608,110,-142},{411,613,109,-142},{405,619,108,-143},{399,625,107,-144},
				{393,631,105,-144},{387,637,104,-145},{381,643,103,-146},{375,649,102,-146},
				{370,654,100,-147},{364,660,99,-147},{358,666,98,-148},{352,672,97,-148},
				{347,677,95,-149},{341,683,94,-149},{335,689,93,-149},{330,694,91,-150},
				{324,700,90,-150},{318,706,89,-150},{313,711,87,-151},{307,717,86,-151},
				{302,722,85,-151},{296,728,83,-151},{291,733,82,-151},{285,739,81,-151},
				{280,744,79,-152},{274,750,78,-152},{269,755,77,-152},{264,760,75,-152},
				{258,766,74,-152},{253,771,73,-152},{248,776,71,-152},{243,781,70,-151},
				{238,787,69,-151},{232,792,67,-151},{227,797,66,-151},{222,802,65,-151},
				{217,807,63,-150},{212,812,62,-150},{207,817,61,-150},{202,822,60,-149},
				{197,827,58,-149},{193,831,57,-148},{188,836,56,-148},{183,841,54,-147},
				{178,846,53,-147},{174,850,52,-146},{169,855,51,-145},{165,859,49,-145},
				{160,864,48,-144},{156,868,47,-143},{151,873,46,-142},{147,877,44,-142},
				{142,882,43,-141},{138,886,42,-140},{134,890,41,-139},{130,894,39,-138},
				{126,898,38,-137},{121,903,37,-136},{117,907,36,-134},{113,911,35,-133},
				{110,914,34,-132},{106,918,33,-131},{102,922,31,-130},{98,926,30,-128},
				{95,930,29,-127},{91,933,28,-125},{87,937,27,-124},{84,940,26,-122},
				{80,944,25,-121},{77,947,24,-119},{74,950,23,-117},{70,954,22,-116},
				{67,957,21,-114},{64,960,20,-112},{61,963,19,-110},{58,966,18,-108},
				{55,969,17,-106},{52,972,17,-104},{49,975,16,-102},{47,977,15,-100},
				{44,980,14,-98},{41,983,13,-96},{39,985,12,-94},{36,988,12,-91},
				{34,990,11,-89},{32,992,10,-86},{30,994,9,-84},{27,997,9,-81},
				{25,999,8,-79},{23,1001,8,-76},{21,1003,7,-74},{20,1004,6,-71},
				{18,1006,6,-68},{16,1008,5,-65},{14,1010,5,-62},{13,1011,4,-59},
				{12,1013,4,-56},{10,1014,3,-53},{9,1015,3,-50},{8,1016,3,-47},
				{7,1017,2,-44},{6,1018,2,-40},{5,1019,2,-37},{4,1020,1,-34},
				{3,1021,1,-30},{2,1022,1,-26},{2,1022,1,-23},{1,1023,0,-19},
				{1,1023,0,-16},{0,1024,0,-12},{0,1024,0,-8},{0,1024,0,-4},
				{0,1024,0,0}
			};
			const Factors & f = FactorsTable[o];
			return v1*f.fv1 + v2*f.fv2 + s1*f.fs1 + s2*f.fs2;

#		endif
	}
#endif


//--------------- Helper macro: INTERPOLATE_FOUR_PIXELS_ADAPTIVE ---------------

#if CHANNELS==1
#	define INTERPOLATE_FOUR_PIXELS_ADAPTIVE(C,C0,C1,C2,C3,O) { \
		C##g = InterpolateFourValuesAdaptive(C0##g,C1##g,C2##g,C3##g,O); \
	}
#elif CHANNELS==2
#	define INTERPOLATE_FOUR_PIXELS_ADAPTIVE(C,C0,C1,C2,C3,O) { \
		C##g = InterpolateFourValuesAdaptive(C0##g,C1##g,C2##g,C3##g,O); \
		C##a = InterpolateFourValuesAdaptive(C0##a,C1##a,C2##a,C3##a,O); \
	}
#elif CHANNELS==3
#	define INTERPOLATE_FOUR_PIXELS_ADAPTIVE(C,C0,C1,C2,C3,O) { \
		C##r = InterpolateFourValuesAdaptive(C0##r,C1##r,C2##r,C3##r,O); \
		C##g = InterpolateFourValuesAdaptive(C0##g,C1##g,C2##g,C3##g,O); \
		C##b = InterpolateFourValuesAdaptive(C0##b,C1##b,C2##b,C3##b,O); \
	}
#else
#	define INTERPOLATE_FOUR_PIXELS_ADAPTIVE(C,C0,C1,C2,C3,O) { \
		C##r = InterpolateFourValuesAdaptive(C0##r,C1##r,C2##r,C3##r,O); \
		C##g = InterpolateFourValuesAdaptive(C0##g,C1##g,C2##g,C3##g,O); \
		C##b = InterpolateFourValuesAdaptive(C0##b,C1##b,C2##b,C3##b,O); \
		C##a = InterpolateFourValuesAdaptive(C0##a,C1##a,C2##a,C3##a,O); \
	}
#endif


//------------ emPainter::ScanlineTool::InterpolateImageAdaptive... ------------

void emPainter::ScanlineTool::CONCAT(InterpolateImageAdaptive,CONCAT(
	CONCAT(METHOD_NAME_EXTENSION_,EXTENSION),
	CONCAT(METHOD_NAME_CHANNELS_,CHANNELS)
)) (const ScanlineTool & sct, int x, int y, int w)
{
	emInt64 ty=y*sct.TDY-sct.TY-0x1800000;
	emUInt32 oy=((ty&0xffffff)+0x7fff)>>16;

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
	emInt64 tx=x*tdx-sct.TX-0x2800000;

	DEFINE_AND_SET_IMAGE_X(imgX,tx>>24,CHANNELS,imgSX)
	DEFINE_AND_SET_COLOR(c0,0)
	DEFINE_AND_SET_COLOR(c1,0)
	DEFINE_AND_SET_COLOR(c2,0)
	DEFINE_AND_SET_COLOR(c3,0)

	emByte * buf=(emByte*)sct.InterpolationBuffer;
	emByte * bufEnd=buf+w*CHANNELS;
	tx=(tx&0xffffff)+0x3000000;

	do {
		while (tx>=0) {
			tx-=0x1000000;
			INCREMENT_IMAGE_X(imgX,CHANNELS,imgSX)

			COPY_COLOR(c0,c1)
			COPY_COLOR(c1,c2)
			COPY_COLOR(c2,c3)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p0,row0,imgX)
			DEFINE_AND_READ_PREMUL_COLOR(c30,p0)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p1,row1,imgX)
			DEFINE_AND_READ_PREMUL_COLOR(c31,p1)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p2,row2,imgX)
			DEFINE_AND_READ_PREMUL_COLOR(c32,p2)

			DEFINE_AND_SET_IMAGE_PIX_PTR(p3,row3,imgX)
			DEFINE_AND_READ_PREMUL_COLOR(c33,p3)

			INTERPOLATE_FOUR_PIXELS_ADAPTIVE(c3,c30,c31,c32,c33,oy)
			FINPREMUL_SIGNED_COLOR(c3)
		}

		emUInt32 ox=(tx+0x1007fff)>>16;
		DEFINE_COLOR(c)
		INTERPOLATE_FOUR_PIXELS_ADAPTIVE(c,c0,c1,c2,c3,ox)

		WRITE_SHR_CLIP_SIGNED_COLOR(buf,c,20)

		buf+=CHANNELS;
		tx+=tdx;
	} while (buf<bufEnd);
}


//--------------------------- Undefine helper macro ----------------------------

#undef INTERPOLATE_FOUR_PIXELS_ADAPTIVE


//==============================================================================
//======================= Undefine General Helper Macros =======================
//==============================================================================

#undef DEFINE_AND_SET_IMAGE_Y
#undef DEFINE_AND_COPY_IMAGE_Y
#undef INCREMENT_IMAGE_Y
#undef DEFINE_AND_SET_IMAGE_ROW_PTR
#undef DEFINE_AND_SET_IMAGE_X
#undef INCREMENT_IMAGE_X
#undef DEFINE_AND_SET_IMAGE_PIX_PTR
#undef DEFINE_COLOR
#undef DEFINE_AND_SET_COLOR
#undef DEFINE_AND_READ_PREMUL_COLOR
#undef READ_PREMUL_MUL_COLOR
#undef COPY_COLOR
#undef ADD_COLOR
#undef SET_MUL_COLOR
#undef ADD_MUL_COLOR
#undef ADD_READ_PREMUL_COLOR
#undef ADD_READ_PREMUL_MUL_COLOR
#undef FINPREMUL_SIGNED_COLOR
#undef FINPREMUL_SHR_COLOR
#undef WRITE_COLOR
#undef WRITE_SHR_CLIP_SIGNED_COLOR
#undef WRITE_NO_ROUND_SHR_COLOR
#undef WRITE_ZERO_COLOR


#endif
