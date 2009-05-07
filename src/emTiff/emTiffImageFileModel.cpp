//------------------------------------------------------------------------------
// emTiffImageFileModel.cpp
//
// Copyright (C) 2004-2009 Oliver Hamann.
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

#include <stddef.h>
#include <tiffio.h>
#include <emTiff/emTiffImageFileModel.h>
#include <emCore/emThread.h>


static emThreadMiniMutex emTiff_ErrorMutex;
static emThreadId emTiff_ErrorThread;
static char emTiff_Error[512];


static void emTiff_ErrorHandler(const char* module, const char* fmt, va_list ap)
{
	emTiff_ErrorMutex.Lock();
	emTiff_ErrorThread=emThread::GetCurrentThreadId();
	emTiff_Error[sizeof(emTiff_Error)-1]=0;
	vsnprintf(
		emTiff_Error,
		sizeof(emTiff_Error)-1,
		fmt,
		ap
	);
	emTiff_ErrorMutex.Unlock();
}


static void emTiff_WarningHandler(const char* module, const char* fmt, va_list ap)
{
}


emRef<emTiffImageFileModel> emTiffImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emTiffImageFileModel,context,name,common)
}


emTiffImageFileModel::emTiffImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emTiffImageFileModel::~emTiffImageFileModel()
{
	emTiffImageFileModel::QuitLoading();
	emTiffImageFileModel::QuitSaving();
}


void emTiffImageFileModel::TryStartLoading() throw(emString)
{
	int samplesPerPixel,bitsPerSample,compression,photometric;
	emString compStr;
	char * imageDesc;
	uint32 u32;
	uint16 u16;
	TIFF * t;

	L=new LoadingState;
	L->Tif=NULL;
	L->Buffer=NULL;
	L->Tiled=false;
	L->ImgW=0;
	L->ImgH=0;
	L->PartW=0;
	L->PartH=0;
	L->Channels=0;
	L->CurrentX=0;
	L->CurrentY=0;
	L->CurrentOp=0;

	emTiff_ErrorMutex.Lock();
	if (emTiff_ErrorThread==emThread::GetCurrentThreadId()) {
		strcpy(emTiff_Error,"unknown TIFF error");
	}
	TIFFSetErrorHandler(emTiff_ErrorHandler);
	TIFFSetWarningHandler(emTiff_WarningHandler);
	emTiff_ErrorMutex.Unlock();

	t=TIFFOpen(GetFilePath(),"r");
	if (!t) ThrowTiffError();
	L->Tif=t;

	TIFFGetFieldDefaulted(t,TIFFTAG_SAMPLESPERPIXEL,&u16);
	samplesPerPixel=u16;
	TIFFGetFieldDefaulted(t,TIFFTAG_COMPRESSION,&u16);
	compression=u16;
	TIFFGetFieldDefaulted(t,TIFFTAG_PHOTOMETRIC,&u16);
	photometric=u16;
	TIFFGetFieldDefaulted(t,TIFFTAG_BITSPERSAMPLE,&u16);
	bitsPerSample=u16;
	TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&u32);
	L->ImgW=(int)u32;
	TIFFGetField(t,TIFFTAG_IMAGELENGTH,&u32);
	L->ImgH=(int)u32;
	L->Tiled=TIFFIsTiled(t)!=0;
	if (L->Tiled) {
		TIFFGetFieldDefaulted(t,TIFFTAG_TILEWIDTH,&u32);
		L->PartW=(int)u32;
		TIFFGetFieldDefaulted(t,TIFFTAG_TILELENGTH,&u32);
		L->PartH=(int)u32;
	}
	else {
		L->PartW=L->ImgW;
		TIFFGetFieldDefaulted(t,TIFFTAG_ROWSPERSTRIP,&u32);
		L->PartH=(int)u32;
	}
	if (L->ImgW<L->PartW || L->ImgH<L->PartH || L->PartW<1 || L->PartH<1) {
		throw emString("Unsupported TIFF file format.");
	}

	if (samplesPerPixel==1) {
		if (photometric==3) {
			L->Channels=3;
		}
		else {
			L->Channels=1;
		}
	}
	else if (samplesPerPixel==2) {
		L->Channels=2;
	}
	else if (samplesPerPixel==3) {
		L->Channels=3;
	}
	else {
		L->Channels=4;
	}

	switch (compression) {
	case 1: compStr="uncompressed"; break;
	case 2: compStr="CCITT RLE compressed"; break;
	case 3: compStr="CCITT Group 3 compressed"; break;
	case 4: compStr="CCITT Group 4 compressed"; break;
	case 5: compStr="LZW compressed"; break;
	case 7: compStr="JPEG compressed"; break;
	case 32773: compStr="PackBits compressed"; break;
	default: compStr=emString::Format("compression=%d",compression); break;
	}
	FileFormatInfo=emString::Format(
		"TIFF %d-bit %s (%d channels extracted)",
		samplesPerPixel*bitsPerSample,
		compStr.Get(),
		L->Channels
	);

	imageDesc=NULL;
	if (TIFFGetField(t,TIFFTAG_IMAGEDESCRIPTION,&imageDesc)==1 && imageDesc) {
		Comment=imageDesc;
	}

	Signal(ChangeSignal);
}


bool emTiffImageFileModel::TryContinueLoading() throw(emString)
{
	TIFF * t;
	unsigned char * map, * tgt;
	uint32 * src;
	uint32 pix;
	int r,x,y,x2,y2;

	//??? PartW*PartH is often not less than ImgW*ImgH!

	t=(TIFF*)L->Tif;

	if (!L->Buffer) {
		L->Buffer=new uint32[L->PartW*L->PartH];
		Image.Setup(L->ImgW,L->ImgH,L->Channels);
		Signal(ChangeSignal);
		return false;
	}

	if (L->CurrentOp==0) {
		if (L->Tiled) {
			r=TIFFReadRGBATile(t,L->CurrentX,L->CurrentY,(uint32*)L->Buffer);
		}
		else {
			r=TIFFReadRGBAStrip(t,L->CurrentY,(uint32*)L->Buffer);
		}
		if (!r) ThrowTiffError();
		L->CurrentOp=1;
		return false;
	}

	x2=L->CurrentX+L->PartW; if (x2>L->ImgW) x2=L->ImgW;
	y2=L->CurrentY+L->PartH; if (y2>L->ImgH) y2=L->ImgH;
	map=Image.GetWritableMap();
	for (y=L->CurrentY; y<y2; y++) {
		src=((uint32*)L->Buffer)+(y2-1-y)*L->PartW;
		tgt=map+(y*L->ImgW+L->CurrentX)*L->Channels;
		switch (L->Channels) {
		case 1:
			for (x=L->CurrentX; x<x2; x++) {
				pix=src[0];
				tgt[0]=(unsigned char)(
					(((int)TIFFGetR(pix))+TIFFGetG(pix)+TIFFGetB(pix))/3
				);
				src++;
				tgt++;
			}
			break;
		case 2:
			for (x=L->CurrentX; x<x2; x++) {
				pix=src[0];
				tgt[0]=(unsigned char)(
					(((int)TIFFGetR(pix))+TIFFGetG(pix)+TIFFGetB(pix))/3
				);
				tgt[1]=(unsigned char)TIFFGetA(pix);
				src++;
				tgt+=2;
			}
			break;
		case 3:
			for (x=L->CurrentX; x<x2; x++) {
				pix=src[0];
				tgt[0]=(unsigned char)TIFFGetR(pix);
				tgt[1]=(unsigned char)TIFFGetG(pix);
				tgt[2]=(unsigned char)TIFFGetB(pix);
				src++;
				tgt+=3;
			}
			break;
		case 4:
			for (x=L->CurrentX; x<x2; x++) {
				pix=src[0];
				tgt[0]=(unsigned char)TIFFGetR(pix);
				tgt[1]=(unsigned char)TIFFGetG(pix);
				tgt[2]=(unsigned char)TIFFGetB(pix);
				tgt[3]=(unsigned char)TIFFGetA(pix);
				src++;
				tgt+=4;
			}
			break;
		}
	}

	Signal(ChangeSignal);

	L->CurrentOp=0;
	L->CurrentX+=L->PartW;
	if (L->CurrentX>=L->ImgW) {
		L->CurrentX=0;
		L->CurrentY+=L->PartH;
		if (L->CurrentY>=L->ImgH) return true;
	}
	return false;
}


void emTiffImageFileModel::QuitLoading()
{
	if (L) {
		if (L->Buffer) delete [] (uint32*)L->Buffer;
		if (L->Tif) TIFFClose((TIFF*)L->Tif);
		delete L;
		L=NULL;
	}
}


void emTiffImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emTiffImageFileModel: Saving not implemented.");
}


bool emTiffImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emTiffImageFileModel::QuitSaving()
{
}


emUInt64 emTiffImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return
			((emUInt64)L->PartW)*L->PartH*4 +
			((emUInt64)L->ImgW)*L->ImgH*L->Channels
		;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emTiffImageFileModel::CalcFileProgress()
{
	double progress;

	if (L && L->ImgW>0 && L->ImgH>0) {
		progress=L->CurrentY*L->ImgW+L->CurrentX*L->PartH;
		if (L->CurrentOp) progress+=0.5*L->PartW*L->PartH;
		progress*=100.0/L->ImgW/L->ImgH;
		if (progress<0.0) progress=0.0;
		else if (progress>100.0) progress=100.0;
	}
	else {
		progress=0.0;
	}
	return progress;
}


void emTiffImageFileModel::ThrowTiffError() throw(emString)
{
	emString str;

	emTiff_ErrorMutex.Lock();
	if (emTiff_ErrorThread==emThread::GetCurrentThreadId()) str=emTiff_Error;
	else str="unknown TIFF error";
	emTiff_ErrorMutex.Unlock();
	throw str;
}
