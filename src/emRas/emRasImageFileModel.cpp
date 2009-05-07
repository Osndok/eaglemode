//------------------------------------------------------------------------------
// emRasImageFileModel.cpp
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

#include <emRas/emRasImageFileModel.h>


emRef<emRasImageFileModel> emRasImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emRasImageFileModel,context,name,common)
}


emRasImageFileModel::emRasImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emRasImageFileModel::~emRasImageFileModel()
{
	emRasImageFileModel::QuitLoading();
	emRasImageFileModel::QuitSaving();
}


void emRasImageFileModel::TryStartLoading() throw(emString)
{
	errno=0;

	L=new LoadingState;
	L->Width=0;
	L->Height=0;
	L->Depth=0;
	L->PixMapType=0;
	L->ColMapType=0;
	L->ColMapSize=0;
	L->NextY=0;
	L->RowSize=0;
	L->BufFill=0;
	L->File=NULL;
	L->ColMap=NULL;
	L->PixBuf=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;

	if (Read32()!=0x59a66a95) goto Err;
	L->Width=Read32();
	L->Height=Read32();
	L->Depth=Read32();
	Read32();
	L->PixMapType=Read32();
	L->ColMapType=Read32();
	L->ColMapSize=Read32();
	if (ferror(L->File) || feof(L->File)) goto Err;
	if (L->Width<1 || L->Height<1) goto Err;
	if (L->Width>65535 || L->Height>65535) goto Err;
	if (L->Depth!=1 && L->Depth!=8 && L->Depth!=24) goto Err;
	if (L->PixMapType<0 || L->PixMapType>3) goto Err;
	if (L->ColMapType<0 || L->ColMapType>1) goto Err;
	if (L->Depth<=8 && L->ColMapType==0) goto Err;
	if (L->Depth>8 && L->ColMapType!=0) goto Err;
	if (L->ColMapType==0 && L->ColMapSize!=0) goto Err;
	if (
		L->ColMapType!=0 &&
		(L->ColMapSize<=0 || L->ColMapSize>(3<<L->Depth))
	) goto Err;
	L->RowSize=((L->Width*L->Depth+7)/8+1)&~1;

	return;
Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("RAS format error");
}


bool emRasImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map;
	int x,n;

	errno = 0;

	if (!L->PixBuf) {
		FileFormatInfo=emString::Format(
			"Sun Rasterfile %d-bit %s",
			L->Depth,
			L->PixMapType==2 ? "RLE-compressed" : "uncompressed"
		);
		Image.Setup(L->Width,L->Height,3);
		Signal(ChangeSignal);
		if (L->Depth<24) {
			L->ColMap=new unsigned char[3<<L->Depth];
			memset(L->ColMap,0,3<<L->Depth);
			if (fread(L->ColMap,1,L->ColMapSize,L->File)!=(size_t)L->ColMapSize) goto Err;
		}
		L->PixBuf=new unsigned char[L->RowSize+256];
		return false;
	}

	map=Image.GetWritableMap()+(L->NextY*L->Width)*3;

	if (L->PixMapType==2) {
		while (L->BufFill<L->RowSize) {
			n=Read8();
			if (n!=0x80) {
				L->PixBuf[L->BufFill++]=(unsigned char)n;
			}
			else if ((n=Read8())>0) {
				n++;
				memset(L->PixBuf+L->BufFill,Read8(),n);
				L->BufFill+=n;
			}
			else {
				L->PixBuf[L->BufFill++]=0x80;
			}
		}
	}
	else {
		if (fread(L->PixBuf,1,L->RowSize,L->File)!=(size_t)L->RowSize) goto Err;
		L->BufFill=L->RowSize;
	}

	if (L->Depth==24) {
		if (L->PixMapType==3) {
			for (x=0; x<L->Width; x++) {
				map[0]=L->PixBuf[x*3];
				map[1]=L->PixBuf[x*3+1];
				map[2]=L->PixBuf[x*3+2];
				map+=3;
			}
		}
		else {
			for (x=0; x<L->Width; x++) {
				map[2]=L->PixBuf[x*3];
				map[1]=L->PixBuf[x*3+1];
				map[0]=L->PixBuf[x*3+2];
				map+=3;
			}
		}
	}
	else if (L->Depth==8) {
		for (x=0; x<L->Width; x++) {
			n=L->PixBuf[x];
			map[0]=L->ColMap[n];
			map[1]=L->ColMap[L->ColMapSize/3+n];
			map[2]=L->ColMap[L->ColMapSize/3*2+n];
			map+=3;
		}
	}
	else {
		for (x=0; x<L->Width; x++) {
			n=(L->PixBuf[x>>3]>>(7-(x&7)))&1;
			map[0]=L->ColMap[n];
			map[1]=L->ColMap[L->ColMapSize/3+n];
			map[2]=L->ColMap[L->ColMapSize/3*2+n];
			map+=3;
		}
	}

	L->BufFill-=L->RowSize;
	if (L->BufFill>0) memmove(L->PixBuf,L->PixBuf+L->RowSize,L->BufFill);

	Signal(ChangeSignal);

	if (ferror(L->File)) goto Err;

	L->NextY++;
	if (L->NextY>=L->Height) return true;

	return false;

Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("RAS format error");
}


void emRasImageFileModel::QuitLoading()
{
	if (L) {
		if (L->PixBuf) delete [] L->PixBuf;
		if (L->File) fclose(L->File);
		if (L->ColMap) delete [] L->ColMap;
		delete L;
		L=NULL;
	}
}


void emRasImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emRasImageFileModel: Saving not implemented.");
}


bool emRasImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emRasImageFileModel::QuitSaving()
{
}


emUInt64 emRasImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->Width)*L->Height*3;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emRasImageFileModel::CalcFileProgress()
{
	if (L && L->Height>0) {
		return 100.0*L->NextY/L->Height;
	}
	else {
		return 0.0;
	}
}


int emRasImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emRasImageFileModel::Read16()
{
	int i;

	i=Read8()<<8;
	i|=Read8();
	return i;
}


int emRasImageFileModel::Read32()
{
	int i;

	i=Read16()<<16;
	i|=Read16();
	return i;
}
