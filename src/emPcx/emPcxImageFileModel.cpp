//------------------------------------------------------------------------------
// emPcxImageFileModel.cpp
//
// Copyright (C) 2005-2009 Oliver Hamann.
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

#include <emPcx/emPcxImageFileModel.h>


emRef<emPcxImageFileModel> emPcxImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPcxImageFileModel,context,name,common)
}


emPcxImageFileModel::emPcxImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emPcxImageFileModel::~emPcxImageFileModel()
{
	emPcxImageFileModel::QuitLoading();
	emPcxImageFileModel::QuitSaving();
}


void emPcxImageFileModel::TryStartLoading() throw(emString)
{
	int manufactor,version,encoding;

	errno=0;

	L=new LoadingState;
	L->Width=0;
	L->Height=0;
	L->Channels=0;
	L->PlanePixBits=0;
	L->PlaneCount=0;
	L->BytesPerLine=0;
	L->RowPlaneSize=0;
	L->NextY=0;
	L->File=NULL;
	L->Palette=NULL;
	L->RowBuffer=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;

	manufactor=Read8();
	version=Read8();
	encoding=Read8();
	L->PlanePixBits=Read8();
	L->Width=1-Read16();
	L->Height=1-Read16();
	L->Width+=Read16();
	L->Height+=Read16();
	fseek(L->File,65,SEEK_SET);
	L->PlaneCount=Read8();
	L->BytesPerLine=Read16();
	if (ferror(L->File) || feof(L->File)) goto Err;

	if (manufactor!=0x0a) goto Err;
	if (version<1 || version>5) goto Err;
	if (encoding!=1) goto Err;
	if (L->PlanePixBits!=1 && L->PlanePixBits!=2 &&
	    L->PlanePixBits!=4 && L->PlanePixBits!=8) goto Err;
	if (L->Width<1 || L->Height<1) goto Err;

	if (L->BytesPerLine<(L->Width*L->PlanePixBits+7)/8) goto Err;
	if (L->BytesPerLine>(L->Width*L->PlanePixBits+7)/8+1024) goto Err;

	if (L->PlanePixBits*L->PlaneCount>=1 && L->PlanePixBits*L->PlaneCount<=8) {
		L->Channels=3;
	}
	else if (L->PlanePixBits*L->PlaneCount==24) {
		L->Channels=3;
	}
	else if (L->PlanePixBits*L->PlaneCount==32) {
		L->Channels=4;
	}
	else goto Err;

	return;

Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("PCX format error");
}


bool emPcxImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map;
	unsigned int val;
	int i,j,n,d,x;

	errno = 0;

	if (!L->RowBuffer) {
		FileFormatInfo=emString::Format(
			"ZSoft PCX image, %d bits per pixel per plane, %d planes",
			L->PlanePixBits,
			L->PlaneCount
		);
		Image.Setup(L->Width,L->Height,L->Channels);
		Signal(ChangeSignal);
		n=1<<(L->PlanePixBits*L->PlaneCount);
		if (n<=256) {
			L->Palette=new unsigned char[3*n];
			if (n<=16) fseek(L->File,16,SEEK_SET);
			else fseek(L->File,-3*256,SEEK_END);
			if (fread(L->Palette,1,3*n,L->File)!=(size_t)(3*n)) goto Err;
		}
		L->RowBuffer=new unsigned char[L->BytesPerLine*L->PlaneCount];
		fseek(L->File,128,SEEK_SET);
		if (ferror(L->File) || feof(L->File)) goto Err;
		return false;
	}

	i=0;
	n=L->BytesPerLine*L->PlaneCount;
	do {
		d=Read8();
		j=i;
		if (d>=0xc0) {
			j+=d-0xc1;
			if (j>=n) goto Err;
			d=Read8();
		}
		do {
			L->RowBuffer[i]=(unsigned char)d;
			i++;
		} while (i<=j);
	} while (i<n);
	if (ferror(L->File)) goto Err;

	map=Image.GetWritableMap()+L->NextY*L->Width*L->Channels;
	for (x=0; x<L->Width; x++) {
		val=0;
		switch (L->PlanePixBits) {
			case 1:
				for (i=0; i<L->PlaneCount; i++) {
					val|=((L->RowBuffer[i*L->BytesPerLine+(x>>3)]>>(7-(x&7)))&0x01)<<i;
				}
				break;
			case 2:
				for (i=0; i<L->PlaneCount; i++) {
					val|=((L->RowBuffer[i*L->BytesPerLine+(x>>2)]>>(6-2*(x&3)))&0x03)<<i*2;
				}
				break;
			case 4:
				for (i=0; i<L->PlaneCount; i++) {
					val|=((L->RowBuffer[i*L->BytesPerLine+(x>>1)]>>(4-4*(x&1)))&0x0f)<<i*4;
				}
				break;
			case 8:
				for (i=0; i<L->PlaneCount; i++) {
					val|=L->RowBuffer[i*L->BytesPerLine+x]<<i*8;
				}
				break;
		}
		if (L->Palette) {
			val=
				L->Palette[3*val] |
				(L->Palette[3*val+1]<<8) |
				(L->Palette[3*val+2]<<16)
			;
		}
		for (i=0; i<L->Channels; i++) {
			*map++=(unsigned char)val;
			val>>=8;
		}
	}

	Signal(ChangeSignal);

	L->NextY++;
	if (L->NextY<L->Height) return false;

	return true;
Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("PCX format error");
}


void emPcxImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Palette) delete [] L->Palette;
		if (L->RowBuffer) delete [] L->RowBuffer;
		delete L;
		L=NULL;
	}
}


void emPcxImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emPcxImageFileModel: Saving not implemented.");
}


bool emPcxImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emPcxImageFileModel::QuitSaving()
{
}


emUInt64 emPcxImageFileModel::CalcMemoryNeed()
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


double emPcxImageFileModel::CalcFileProgress()
{
	if (L && L->Height>0) {
		return 100.0*L->NextY/L->Height;
	}
	else {
		return 0.0;
	}
}


int emPcxImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emPcxImageFileModel::Read16()
{
	int i;

	i=Read8();
	i|=Read8()<<8;
	return i;
}
