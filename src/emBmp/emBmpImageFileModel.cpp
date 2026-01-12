//------------------------------------------------------------------------------
// emBmpImageFileModel.cpp
//
// Copyright (C) 2004-2010,2014,2018-2019,2022,2025 Oliver Hamann.
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

#include <emBmp/emBmpImageFileModel.h>


emRef<emBmpImageFileModel> emBmpImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emBmpImageFileModel,context,name,common)
}


emBmpImageFileModel::emBmpImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
}


emBmpImageFileModel::~emBmpImageFileModel()
{
}


void emBmpImageFileModel::TryStartLoading()
{
	char infoBuf[1024];
	char errorBuf[256];
	long bestOffset,bihOffset,bihSize,bestSize,size,offset;
	int w,h,pixels,bestPixels,i,iconCnt;

	L=new LoadingState;
	L->File.TryOpen(GetFilePath(),"rb");

	w=L->File.TryReadUInt16LE();
	if (w==0x4D42) {
		// BMP file.
		L->File.TryReadUInt32LE();
		L->File.TryReadUInt32LE();
		L->BitsOffset=L->File.TryReadUInt32LE();
		bihOffset=14;
	}
	else if (w==0) {
		// ICO or CUR file.
		w=L->File.TryReadUInt16LE();
		if (w!=1 && w!=2) goto Err;
		iconCnt=L->File.TryReadUInt16LE();
		if (iconCnt<1) goto Err;
		bestOffset=0;
		bestPixels=0;
		bestSize=0;
		for (i=0; i<iconCnt; i++) {
			w=L->File.TryReadUInt8();
			if (!w) w=256;
			h=L->File.TryReadUInt8();
			if (!h) h=256;
			pixels=w*h;
			L->File.TryReadUInt16LE();
			L->File.TryReadUInt32LE();
			size=L->File.TryReadUInt32LE();
			offset=L->File.TryReadUInt32LE();
			if (bestPixels<pixels || (bestPixels==pixels && bestSize<size)) {
				bestOffset=offset;
				bestPixels=pixels;
				bestSize=size;
			}
		}
		bihOffset=bestOffset;
		L->BitsOffset=0;
		L->IsIcon=true;
	}
	else {
		goto Err;
	}

	L->File.TrySeek(bihOffset);
	bihSize=L->File.TryReadUInt32LE();
	L->ColsOffset=bihOffset+bihSize;

	if (bihSize==40) {
		L->Width=L->File.TryReadUInt32LE();
		L->Height=L->File.TryReadUInt32LE();
		if (L->File.TryReadUInt16LE()!=1) goto Err;
		L->BitsPerPixel=L->File.TryReadUInt16LE();
		L->Compress=L->File.TryReadUInt32LE();
		L->File.TryReadUInt32LE();
		L->File.TryReadUInt32LE();
		L->File.TryReadUInt32LE();
		L->ColsUsed=L->File.TryReadUInt32LE();
		L->ColSize=4;
	}
	else if (bihSize==12) {
		L->Width=L->File.TryReadUInt16LE();
		L->Height=L->File.TryReadUInt16LE();
		if (L->File.TryReadUInt16LE()!=1) goto Err;
		L->BitsPerPixel=L->File.TryReadUInt16LE();
		L->Compress=0;
		L->ColsUsed=0;
		L->ColSize=3;
	}
	else if (bihSize==0x474E5089 && L->IsIcon) {
		L->IsPng=true;
		L->File.TrySeek(bihOffset);
		L->PngLib=emTryOpenLib("emPng",false);
		L->PngStartDecoding=(PngStartDecodingFunc)
			emTryResolveSymbolFromLib(L->PngLib,"emPngStartDecoding")
		;
		L->PngContinueDecoding=(PngContinueDecodingFunc)
			emTryResolveSymbolFromLib(L->PngLib,"emPngContinueDecoding")
		;
		L->PngQuitDecoding=(PngQuitDecodingFunc)
			emTryResolveSymbolFromLib(L->PngLib,"emPngQuitDecoding")
		;
		infoBuf[0]=0;
		errorBuf[0]=0;
		L->PngInst=L->PngStartDecoding(
			L->File.TryGetFile(),&L->Width,&L->Height,&L->Channels,&L->PassCount,
			infoBuf,sizeof(infoBuf),errorBuf,sizeof(errorBuf)
		);
		if (!L->PngInst) throw emException("%s",errorBuf);
		FileFormatInfo="MS Windows icon or cursor file, ";
		FileFormatInfo+=infoBuf;
		Signal(ChangeSignal);
		return;
	}
	else {
		goto Err;
	}

	if (L->IsIcon) {
		L->Channels=4;
		L->Height/=2;
	}
	else {
		if (L->BitsPerPixel==32 && L->Compress==0) L->Channels=4;
		else L->Channels=3;
	}
	if (L->ColsUsed<0) goto Err;
	if (L->BitsPerPixel<=8 && L->ColsUsed>(1<<L->BitsPerPixel)) goto Err;
	if (L->ColsUsed==0 && L->BitsPerPixel<24) L->ColsUsed=1<<L->BitsPerPixel;
	if (!L->BitsOffset) {
		L->BitsOffset=L->ColsOffset;
		if (L->Compress==3) L->BitsOffset+=12;
		else if (L->BitsPerPixel<=8) L->BitsOffset+=L->ColSize*L->ColsUsed;
	}

	if (
		L->Width<1 || L->Width>0x7fffff ||
		L->Height<1 || L->Height>0x7fffff
	) goto Err;
	if (
		L->BitsPerPixel!=1 && L->BitsPerPixel!=4 &&
		L->BitsPerPixel!=8 && L->BitsPerPixel!=16 &&
		L->BitsPerPixel!=24 && L->BitsPerPixel!=32
	) goto Err;
	if (
		L->Compress!=0 &&
		(L->Compress!=1 || L->BitsPerPixel!=8) &&
		(L->Compress!=2 || L->BitsPerPixel!=4) &&
		(L->Compress!=3 || (L->BitsPerPixel!=16 && L->BitsPerPixel!=32))
	) goto Err;

	FileFormatInfo=emString::Format(
		"MS Windows %s file, %d-bit %s",
		L->IsIcon ? "icon or cursor" : "BMP",
		L->BitsPerPixel,
		L->Compress==0 || L->Compress==3 ? "uncompressed" :
		L->Compress==1 || L->Compress==2 ? "RLE-compressed" :
		"compressed"
	);
	Signal(ChangeSignal);

	return;

Err:
	throw emException("BMP format error");
}


bool emBmpImageFileModel::TryContinueLoading()
{
	char commentBuf[1024];
	char errorBuf[256];
	unsigned char * map;
	emUInt32 msk;
	int x,n,t,i,j,y,r;

	if (!L->ImagePrepared) {
		Image.Setup(L->Width,L->Height,L->Channels);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		if (L->IsPng) return false;
		if (L->BitsPerPixel<=8) {
			L->File.TrySeek(L->ColsOffset);
			L->Palette=new unsigned char[4<<L->BitsPerPixel];
			memset(L->Palette.Get(),0,4<<L->BitsPerPixel);
			if (L->ColSize==4) {
				L->File.TryRead(L->Palette.Get(),4*L->ColsUsed);
			}
			else {
				for (i=0; i<L->ColsUsed; i++) {
					for (j=0; j<L->ColSize; j++) {
						t=L->File.TryReadUInt8();
						if (j<4) L->Palette[i*4+j]=(unsigned char)t;
					}
				}
			}
		}
		else if (L->Compress==3) {
			L->File.TrySeek(L->ColsOffset);
			for (i=0; i<3; i++) {
				msk=L->File.TryReadUInt32LE();
				for (j=0; msk && (msk&1)==0; j++) msk>>=1;
				L->CMax[i]=msk;
				L->CPos[i]=j;
			}
		}
		L->File.TrySeek(L->BitsOffset);
		return false;
	}

	if (L->IsPng) {
		commentBuf[0]=0;
		errorBuf[0]=0;
		r=L->PngContinueDecoding(
			L->PngInst,
			Image.GetWritableMap()+L->Y*(size_t)Image.GetWidth()*Image.GetChannelCount(),
			commentBuf,sizeof(commentBuf),errorBuf,sizeof(errorBuf)
		);
		if (r<0) throw emException("%s",errorBuf);
		L->Y++;
		if (L->Y>=L->Height) {
			L->Pass++;
			L->Y=0;
		}
		Comment+=commentBuf;
		Signal(ChangeSignal);
		return r!=0;
	}

	if (L->Y>=L->Height) return true;

	map=Image.GetWritableMap()+(L->Height-L->Y-1)*(size_t)L->Width*L->Channels;

	if (L->Compress==0) {
		if (L->BitsPerPixel==32) {
			for (x=0; x<L->Width; x++) {
				map[2]=L->File.TryReadUInt8();
				map[1]=L->File.TryReadUInt8();
				map[0]=L->File.TryReadUInt8();
				map[3]=L->File.TryReadUInt8();
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==24) {
			for (x=0; x<L->Width; x++) {
				map[2]=L->File.TryReadUInt8();
				map[1]=L->File.TryReadUInt8();
				map[0]=L->File.TryReadUInt8();
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==16) {
			for (x=0; x<L->Width; x++) {
				n=L->File.TryReadUInt16LE();
				map[2]=(unsigned char)(((n&0x1f)*255+15)/31);
				map[1]=(unsigned char)((((n>>5)&0x1f)*255+15)/31);
				map[0]=(unsigned char)((((n>>10)&0x1f)*255+15)/31);
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==8) {
			for (x=0; x<L->Width; x++) {
				t=L->File.TryReadUInt8();
				map[0]=L->Palette[t*4+2];
				map[1]=L->Palette[t*4+1];
				map[2]=L->Palette[t*4+0];
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==4) {
			for (n=0,x=0; x<L->Width; x++) {
				if ((x&1)==0) n=L->File.TryReadUInt8(); else n<<=4;
				t=(n>>4)&15;
				map[0]=L->Palette[t*4+2];
				map[1]=L->Palette[t*4+1];
				map[2]=L->Palette[t*4+0];
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==1) {
			for (n=0,x=0; x<L->Width; x++) {
				if ((x&7)==0) n=L->File.TryReadUInt8(); else n<<=1;
				t=(n>>7)&1;
				map[0]=L->Palette[t*4+2];
				map[1]=L->Palette[t*4+1];
				map[2]=L->Palette[t*4+0];
				map+=L->Channels;
			}
		}
		else goto Err;
	}
	else if (L->Compress==1 && L->BitsPerPixel==8) {
		for (x=0;;) {
			n=L->File.TryReadUInt8();
			if (n>0) {
				if (x+n>L->Width) goto Err;
				t=L->File.TryReadUInt8();
				for (i=0; i<n; i++) {
					map[0]=L->Palette[t*4+2];
					map[1]=L->Palette[t*4+1];
					map[2]=L->Palette[t*4+0];
					map+=L->Channels;
				}
				x+=n;
			}
			else {
				n=L->File.TryReadUInt8();
				if (n<=1) {
					if (x!=L->Width) goto Err;
					if (n==1 && L->Y+1!=L->Height) goto Err;
					break;
				}
				if (n==2) goto Err;
				if (x+n>L->Width) goto Err;
				for (i=0; i<n; i++) {
					t=L->File.TryReadUInt8();
					map[0]=L->Palette[t*4+2];
					map[1]=L->Palette[t*4+1];
					map[2]=L->Palette[t*4+0];
					map+=L->Channels;
				}
				x+=n;
				if (n&1) L->File.TryReadUInt8();
			}
		}
	}
	else if (L->Compress==2 && L->BitsPerPixel==4) {
		for (x=0;;) {
			n=L->File.TryReadUInt8();
			if (n>0) {
				if (x+n>L->Width) goto Err;
				t=L->File.TryReadUInt8();
				for (i=0; i<n; i++) {
					t=((t>>4)&0x0f)|((t<<4)&0xf0);
					map[0]=L->Palette[(t&0x0f)*4+2];
					map[1]=L->Palette[(t&0x0f)*4+1];
					map[2]=L->Palette[(t&0x0f)*4+0];
					map+=L->Channels;
				}
				x+=n;
			}
			else {
				n=L->File.TryReadUInt8();
				if (n<=1) {
					if (x!=L->Width) goto Err;
					if (n==1 && L->Y+1!=L->Height) goto Err;
					break;
				}
				if (n==2) goto Err;
				if (x+n>L->Width) goto Err;
				for (i=0, t=0; i<n; i++) {
					if ((i&1)==0) t=L->File.TryReadUInt8();
					t=((t>>4)&0x0f)|((t<<4)&0xf0);
					map[0]=L->Palette[(t&0x0f)*4+2];
					map[1]=L->Palette[(t&0x0f)*4+1];
					map[2]=L->Palette[(t&0x0f)*4+0];
					map+=L->Channels;
				}
				x+=n;
				if (((n+1)/2)&1) L->File.TryReadUInt8();
			}
		}
	}
	else if (L->Compress==3 && (L->BitsPerPixel==16 || L->BitsPerPixel==32)) {
		for (x=0; x<L->Width; x++) {
			if (L->BitsPerPixel==16) n=L->File.TryReadUInt16LE();
			else n=L->File.TryReadUInt32LE();
			for (i=0; i<3; i++) {
				t=L->CMax[i];
				if (t) t=(((n>>L->CPos[i])&t)*255+(t>>1))/t;
				map[i]=(unsigned char)t;
			}
			map+=L->Channels;
		}
	}
	else goto Err;

	Signal(ChangeSignal);

	if (L->Compress==0 || L->Compress==3) {
		L->File.TrySkip((0-((L->Width*L->BitsPerPixel+7)>>3))&3);
	}

	L->Y++;
	if (L->Y<L->Height) return false;

	if (L->Channels>3 && (L->BitsPerPixel!=32 || L->Compress!=0)) {
		for (y=0; y<L->Height; y++) {
			map=Image.GetWritableMap()+(L->Height-y-1)*(size_t)L->Width*L->Channels;
			for (n=0, x=0; x<L->Width; x++) {
				if ((x&7)==0) n=L->File.TryReadUInt8(); else n<<=1;
				t=(n>>7)&1;
				map[3]=(unsigned char)(t ? 0 : 255);
				map+=L->Channels;
			}
			L->File.TrySkip((0-((L->Width+7)>>3))&3);
		}
	}

	if (!L->IsIcon && L->BitsPerPixel==32 && L->Compress==0 && L->Channels==4) {
		n=0;
		map=Image.GetWritableMap();
		for (y=0; y<L->Height; y++) {
			for (x=0; x<L->Width; x++) {
				n|=map[3];
				map+=4;
			}
			if (n) break;
		}
		if (!n) Image.FillChannel(3,0xff);
	}

	return true;

Err:
	throw emException("BMP format error");
}


void emBmpImageFileModel::QuitLoading()
{
	L.Reset();
}


void emBmpImageFileModel::TryStartSaving()
{
	const char * ext;

	ext=emGetExtensionInPath(GetFilePath());
	if (strcasecmp(ext,".ico")==0) {
		throw emException("Saving ICO format is not supported.");
	}
	if (strcasecmp(ext,".cur")==0) {
		throw emException("Saving CUR format is not supported.");
	}
	if (strcasecmp(ext,".dib")==0) {
		throw emException("Saving DIB format is not supported.");
	}

	S=new SavingState;

	if (Image.HasAnyTransparentPixel()) {
		S->BitsPerPixel=32;
	}
	else {
		S->Palette=Image.DetermineAllColorsSorted(256);
		if (S->Palette.IsEmpty()) {
			S->BitsPerPixel=24;
		}
		else if (S->Palette.GetCount()<=2) {
			S->BitsPerPixel=1;
		}
		else if (S->Palette.GetCount()<=16) {
			S->BitsPerPixel=4;
		}
		else {
			S->BitsPerPixel=8;
		}
	}
}


bool emBmpImageFileModel::TryContinueSaving()
{
	int bitsPerPixel,width,height,rowBytes,alignBytes,palSize;
	int x,y,i,j,k,val,shift;
	emColor c;
	emString str;

	bitsPerPixel=S->BitsPerPixel;
	width=Image.GetWidth();
	height=Image.GetHeight();
	rowBytes=(width*bitsPerPixel+7)/8;
	alignBytes=((rowBytes+3)&(~3))-rowBytes;

	if (!S->File.IsOpen()) {
		palSize=bitsPerPixel<=8 ? (1<<bitsPerPixel) : 0;
		S->File.TryOpen(GetFilePath(),"wb");
		S->File.TryWriteInt16LE(0x4D42);
		S->File.TryWriteInt32LE(54+palSize*4+(rowBytes+alignBytes)*height);
		S->File.TryWriteInt32LE(0);
		S->File.TryWriteInt32LE(54+palSize*4);
		S->File.TryWriteInt32LE(40);
		S->File.TryWriteInt32LE(width);
		S->File.TryWriteInt32LE(height);
		S->File.TryWriteInt16LE(1);
		S->File.TryWriteInt16LE(bitsPerPixel);
		S->File.TryWriteInt32LE(0);
		S->File.TryWriteInt32LE((rowBytes+alignBytes)*height);
		S->File.TryWriteInt32LE(0);
		S->File.TryWriteInt32LE(0);
		S->File.TryWriteInt32LE(0);
		S->File.TryWriteInt32LE(0);
		for (i=0; i<palSize; i++) {
			if (i<S->Palette.GetCount()) c=S->Palette[i]; else c=0;
			S->File.TryWriteUInt8(c.GetBlue());
			S->File.TryWriteUInt8(c.GetGreen());
			S->File.TryWriteUInt8(c.GetRed());
			S->File.TryWriteUInt8(0);
		}
		return false;
	}

	if (S->NextY<height) {
		y=height-1-S->NextY;
		S->NextY++;
		val=0;
		shift=8;
		for (x=0; x<width; x++) {
			c=Image.GetPixel(x,y);
			if (bitsPerPixel<=8) {
				for (i=0, j=S->Palette.GetCount(); i<j;) {
					k=(i+j)/2;
					if (S->Palette[k].Get()<c.Get()) i=k+1; else j=k;
				}
				shift-=bitsPerPixel;
				val|=i<<shift;
				if (shift==0 || x+1>=width) {
					S->File.TryWriteUInt8((emUInt8)val);
					val=0;
					shift=8;
				}
			}
			else {
				S->File.TryWriteUInt8(c.GetBlue());
				S->File.TryWriteUInt8(c.GetGreen());
				S->File.TryWriteUInt8(c.GetRed());
				if (bitsPerPixel==32) S->File.TryWriteUInt8(c.GetAlpha());
			}
		}
		for (i=0; i<alignBytes; i++) S->File.TryWriteUInt8(0);
		return false;
	}

	S->File.TryClose();

	str=emString::Format(
		"MS Windows BMP file, %d-bit uncompressed",
		S->BitsPerPixel
	);
	if (FileFormatInfo!=str) {
		FileFormatInfo=str;
		Signal(ChangeSignal);
	}

	return true;
}


void emBmpImageFileModel::QuitSaving()
{
	S.Reset();
}


emUInt64 emBmpImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->Width)*L->Height*L->Channels;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emBmpImageFileModel::CalcFileProgress()
{
	if (L && L->IsPng && L->Height>0 && L->PassCount>0) {
		return 100.0*(L->Pass*L->Height+L->Y)/(L->PassCount*L->Height);
	}
	else if (L && L->Height>0) {
		return 100.0*L->Y/L->Height;
	}
	else if (S && GetImage().GetHeight()>0) {
		return 100.0*S->NextY/GetImage().GetHeight();
	}
	else {
		return 0.0;
	}
}


emBmpImageFileModel::LoadingState::LoadingState()
	: Width(0),
	Height(0),
	Channels(0),
	BitsPerPixel(0),
	BitsOffset(0),
	ColsOffset(0),
	ColSize(0),
	ColsUsed(0),
	Compress(0),
	Y(0),
	IsIcon(false),
	IsPng(false),
	PngLib(NULL),
	PngStartDecoding(NULL),
	PngContinueDecoding(NULL),
	PngQuitDecoding(NULL),
	PngInst(NULL),
	PassCount(0),
	Pass(0),
	ImagePrepared(false)
{
	int i;

	for (i=0; i<3; i++) CMax[i]=0;
	for (i=0; i<3; i++) CPos[i]=0;
}


emBmpImageFileModel::LoadingState::~LoadingState()
{
	if (PngInst) PngQuitDecoding(PngInst);
	if (PngLib) emCloseLib(PngLib);
}


emBmpImageFileModel::SavingState::SavingState()
	: BitsPerPixel(0),
	NextY(0)
{
}


emBmpImageFileModel::SavingState::~SavingState()
{
}
