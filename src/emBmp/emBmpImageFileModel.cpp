//------------------------------------------------------------------------------
// emBmpImageFileModel.cpp
//
// Copyright (C) 2004-2010 Oliver Hamann.
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
	L=NULL;
}


emBmpImageFileModel::~emBmpImageFileModel()
{
	emBmpImageFileModel::QuitLoading();
	emBmpImageFileModel::QuitSaving();
}


void emBmpImageFileModel::TryStartLoading() throw(emString)
{
	int w,i,iconCnt,bestOffset,bihSize,bestSize,s,o;

	errno=0;

	L=new LoadingState;
	L->Width=0;
	L->Height=0;
	L->Channels=0;
	L->BitsPerPixel=0;
	L->BitsOffset=0;
	L->ColsOffset=0;
	L->ColSize=0;
	L->ColsUsed=0;
	L->Compress=0;
	L->NextY=0;
	for (i=0; i<3; i++) L->CMax[i]=0;
	for (i=0; i<3; i++) L->CPos[i]=0;
	L->IsIcon=false;
	L->ImagePrepared=false;
	L->File=NULL;
	L->Palette=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;

	w=Read16();
	if (w==0x4D42) {
		// BMP file.
		Read32();
		Read32();
		L->BitsOffset=Read32();
		bihSize=Read32();
		L->ColsOffset=bihSize+14;
	}
	else if (w==0) {
		// ICO or CUR file.
		w=Read16();
		if (w!=1 && w!=2) goto Err;
		iconCnt=Read16();
		if (iconCnt<1) goto Err;
		bestOffset=0;
		bestSize=0;
		for (i=0; i<iconCnt; i++) {
			Read32();
			Read32();
			s=Read32();
			o=Read32();
			if (bestSize==0 || bestSize<s) {
				bestOffset=o;
				bestSize=s;
			}
			if (ferror(L->File) || feof(L->File)) goto Err;
		}
		fseek(L->File,bestOffset,SEEK_SET);
		if (ferror(L->File) || feof(L->File)) goto Err;
		L->BitsOffset=0;
		bihSize=Read32();
		L->ColsOffset=bestOffset+bihSize;
		L->IsIcon=true;
	}
	else {
		goto Err;
	}

	if (bihSize==40) {
		L->Width=Read32();
		L->Height=Read32();
		if (Read16()!=1) goto Err;
		L->BitsPerPixel=Read16();
		L->Compress=Read32();
		Read32();
		Read32();
		Read32();
		L->ColsUsed=Read32();
		L->ColSize=4;
	}
	else if (bihSize==12) {
		L->Width=Read16();
		L->Height=Read16();
		if (Read16()!=1) goto Err;
		L->BitsPerPixel=Read16();
		L->Compress=0;
		L->ColsUsed=0;
		L->ColSize=3;
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
	if (L->ColsUsed==0 && L->BitsPerPixel<24) L->ColsUsed=1<<L->BitsPerPixel;
	if (!L->BitsOffset) {
		L->BitsOffset=L->ColsOffset;
		if (L->Compress==3) L->BitsOffset+=12;
		else if (L->BitsPerPixel<=8) L->BitsOffset+=L->ColSize*L->ColsUsed;
	}

	if (ferror(L->File) || feof(L->File)) goto Err;
	if (L->Width<1 || L->Height<1) goto Err;
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
	if (errno) throw emGetErrorText(errno);
	else throw emString("BMP format error");
}


bool emBmpImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map;
	emUInt32 msk;
	int x,n,t,i,j,y;

	errno = 0;

	if (!L->ImagePrepared) {
		Image.Setup(L->Width,L->Height,L->Channels);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		if (L->BitsPerPixel<=8) {
			fseek(L->File,L->ColsOffset,SEEK_SET);
			L->Palette=new unsigned char[4<<L->BitsPerPixel];
			memset(L->Palette,0,4<<L->BitsPerPixel);
			if (L->ColSize==4) {
				if (fread(L->Palette,1,4*L->ColsUsed,L->File)!=(size_t)(4*L->ColsUsed)) goto Err;
			}
			else {
				for (i=0; i<L->ColsUsed; i++) {
					for (j=0; j<L->ColSize; j++) {
						t=Read8();
						if (j<4) L->Palette[i*4+j]=(unsigned char)t;
					}
				}
			}
		}
		else if (L->Compress==3) {
			fseek(L->File,L->ColsOffset,SEEK_SET);
			for (i=0; i<3; i++) {
				msk=Read32();
				for (j=0; msk && (msk&1)==0; j++) msk>>=1;
				L->CMax[i]=msk;
				L->CPos[i]=j;
			}
		}
		fseek(L->File,L->BitsOffset,SEEK_SET);
		if (ferror(L->File) || feof(L->File)) goto Err;
		return false;
	}

	if (L->NextY>=L->Height) return true;

	map=Image.GetWritableMap()+(L->Height-L->NextY-1)*L->Width*L->Channels;

	if (L->Compress==0) {
		if (L->BitsPerPixel==32) {
			for (x=0; x<L->Width; x++) {
				map[2]=(unsigned char)Read8();
				map[1]=(unsigned char)Read8();
				map[0]=(unsigned char)Read8();
				map[3]=(unsigned char)Read8();
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==24) {
			for (x=0; x<L->Width; x++) {
				map[2]=(unsigned char)Read8();
				map[1]=(unsigned char)Read8();
				map[0]=(unsigned char)Read8();
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==16) {
			for (x=0; x<L->Width; x++) {
				n=Read16();
				map[2]=(unsigned char)(((n&0x1f)*255+15)/31);
				map[1]=(unsigned char)((((n>>5)&0x1f)*255+15)/31);
				map[0]=(unsigned char)((((n>>10)&0x1f)*255+15)/31);
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==8) {
			for (x=0; x<L->Width; x++) {
				t=Read8();
				map[0]=L->Palette[t*4+2];
				map[1]=L->Palette[t*4+1];
				map[2]=L->Palette[t*4+0];
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==4) {
			for (n=0,x=0; x<L->Width; x++) {
				if ((x&1)==0) n=Read8(); else n<<=4;
				t=(n>>4)&15;
				map[0]=L->Palette[t*4+2];
				map[1]=L->Palette[t*4+1];
				map[2]=L->Palette[t*4+0];
				map+=L->Channels;
			}
		}
		else if (L->BitsPerPixel==1) {
			for (n=0,x=0; x<L->Width; x++) {
				if ((x&7)==0) n=Read8(); else n<<=1;
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
			n=Read8();
			if (n>0) {
				if (x+n>L->Width) goto Err;
				t=Read8();
				for (i=0; i<n; i++) {
					map[0]=L->Palette[t*4+2];
					map[1]=L->Palette[t*4+1];
					map[2]=L->Palette[t*4+0];
					map+=L->Channels;
				}
				x+=n;
			}
			else {
				n=Read8();
				if (n<=1) {
					if (x!=L->Width) goto Err;
					if (n==1 && L->NextY+1!=L->Height) goto Err;
					break;
				}
				if (n==2) goto Err;
				if (x+n>L->Width) goto Err;
				for (i=0; i<n; i++) {
					t=Read8();
					map[0]=L->Palette[t*4+2];
					map[1]=L->Palette[t*4+1];
					map[2]=L->Palette[t*4+0];
					map+=L->Channels;
				}
				x+=n;
				if (n&1) Read8();
			}
		}
	}
	else if (L->Compress==2 && L->BitsPerPixel==4) {
		for (x=0;;) {
			n=Read8();
			if (n>0) {
				if (x+n>L->Width) goto Err;
				t=Read8();
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
				n=Read8();
				if (n<=1) {
					if (x!=L->Width) goto Err;
					if (n==1 && L->NextY+1!=L->Height) goto Err;
					break;
				}
				if (n==2) goto Err;
				if (x+n>L->Width) goto Err;
				for (i=0, t=0; i<n; i++) {
					if ((i&1)==0) t=Read8();
					t=((t>>4)&0x0f)|((t<<4)&0xf0);
					map[0]=L->Palette[(t&0x0f)*4+2];
					map[1]=L->Palette[(t&0x0f)*4+1];
					map[2]=L->Palette[(t&0x0f)*4+0];
					map+=L->Channels;
				}
				x+=n;
				if (((n+1)/2)&1) Read8();
			}
		}
	}
	else if (L->Compress==3 && (L->BitsPerPixel==16 || L->BitsPerPixel==32)) {
		for (x=0; x<L->Width; x++) {
			if (L->BitsPerPixel==16) n=Read16(); else n=Read32();
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
		fseek(L->File,(0-((L->Width*L->BitsPerPixel+7)>>3))&3,SEEK_CUR);
	}

	if (ferror(L->File)) goto Err;

	L->NextY++;
	if (L->NextY<L->Height) return false;

	if (L->Channels>3 && (L->BitsPerPixel!=32 || L->Compress!=0)) {
		for (y=0; y<L->Height; y++) {
			map=Image.GetWritableMap()+(L->Height-y-1)*L->Width*L->Channels;
			for (n=0, x=0; x<L->Width; x++) {
				if ((x&7)==0) n=Read8(); else n<<=1;
				t=(n>>7)&1;
				map[3]=(unsigned char)(t ? 0 : 255);
				map+=L->Channels;
			}
			fseek(L->File,(0-((L->Width+7)>>3))&3,SEEK_CUR);
			if (ferror(L->File)) goto Err;
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
	if (errno) throw emGetErrorText(errno);
	else throw emString("BMP format error");
}


void emBmpImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Palette) delete [] L->Palette;
		delete L;
		L=NULL;
	}
}


void emBmpImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emBmpImageFileModel: Saving not implemented.");
}


bool emBmpImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emBmpImageFileModel::QuitSaving()
{
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
	if (L && L->Height>0) {
		return 100.0*L->NextY/L->Height;
	}
	else {
		return 0.0;
	}
}


int emBmpImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emBmpImageFileModel::Read16()
{
	int i;

	i=Read8();
	i|=Read8()<<8;
	return i;
}


int emBmpImageFileModel::Read32()
{
	int i;

	i=Read16();
	i|=Read16()<<16;
	return i;
}
