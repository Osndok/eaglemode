//------------------------------------------------------------------------------
// emRgbImageFileModel.cpp
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

#include <emRgb/emRgbImageFileModel.h>


emRef<emRgbImageFileModel> emRgbImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emRgbImageFileModel,context,name,common)
}


emRgbImageFileModel::emRgbImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emRgbImageFileModel::~emRgbImageFileModel()
{
	emRgbImageFileModel::QuitLoading();
	emRgbImageFileModel::QuitSaving();
}


void emRgbImageFileModel::TryStartLoading() throw(emString)
{
	int magic,dimension,colorMapId;

	L=new LoadingState;
	L->Storage=0;
	L->BytesPerChannel=0;
	L->XSize=0;
	L->YSize=0;
	L->ZSize=0;
	L->PixMin=0;
	L->PixMax=0;
	L->NextY=0;
	L->NextZ=0;
	L->ZUse=0;
	L->ImagePrepared=false;
	L->File=NULL;
	L->OffsetTable=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto ErrFile;

	magic=Read16();
	L->Storage=Read8();
	L->BytesPerChannel=Read8();
	dimension=Read16();
	L->XSize=Read16();
	L->YSize=Read16();
	L->ZSize=Read16();
	L->PixMin=Read32();
	L->PixMax=Read32();
	fseek(L->File,84,SEEK_CUR);
	colorMapId=Read32();
	fseek(L->File,404,SEEK_CUR);

	if (ferror(L->File)) goto ErrFile;
	if (feof(L->File)) goto ErrFormat;

	if (magic!=474) goto ErrFormat;
	if (L->Storage<0 || L->Storage>1) goto ErrFormat;
	if (L->BytesPerChannel<1 || L->BytesPerChannel>2) goto ErrFormat;
	if (dimension==1) { L->YSize=1; L->ZSize=1; }
	else if (dimension==2) { L->ZSize=1; }
	else if (dimension!=3) goto ErrFormat;
	if (L->XSize<1) goto ErrFormat;
	if (L->YSize<1) goto ErrFormat;
	if (L->ZSize<1) goto ErrFormat;
	if (L->PixMin>=L->PixMax) goto ErrFormat;
	if (L->PixMax>=(1<<(L->BytesPerChannel*8))) goto ErrFormat;
	if (colorMapId<0 || colorMapId>3) goto ErrFormat;

	if (colorMapId!=0) goto ErrUnsupported;

	L->ZUse=L->ZSize;
	if (L->ZUse>4) L->ZUse=4;

	return;
ErrFile:
	throw emGetErrorText(errno);
ErrFormat:
	throw emString("SGI image file format error.");
ErrUnsupported:
	throw emString("Unsupported SGI image file format.");
}


bool emRgbImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map;
	int x,val,i,cnt,rpt;

	if (!L->ImagePrepared) {
		FileFormatInfo=emString::Format(
			"SGI Image File (\"RGB\"), %d channels, %s",
			L->ZSize,
			L->Storage ? "RLE-compressed" : "uncompressed"
		);
		Image.Setup(L->XSize,L->YSize,L->ZUse);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	map=
		Image.GetWritableMap()+
		(L->YSize-L->NextY-1)*L->XSize*L->ZUse+L->NextZ
	;

	if (L->Storage==0) {
		for (x=0; x<L->XSize; x++) {
			val=(unsigned char)fgetc(L->File);
			if (L->BytesPerChannel>1) val=(val<<8)|(unsigned char)fgetc(L->File);
			val=((val-L->PixMin)*255+(L->PixMax-L->PixMin)/2)/(L->PixMax-L->PixMin);
			map[x*L->ZUse]=(unsigned char)val;
		}
	}
	else {
		if (L->Storage && !L->OffsetTable) {
			L->OffsetTable=new emUInt32[L->ZUse*L->YSize];
			for (x=0; x<L->ZUse*L->YSize; x++) L->OffsetTable[x]=(emUInt32)Read32();
			if (ferror(L->File)) goto ErrFile;
			if (feof(L->File)) goto ErrFormat;
			return false;
		}
		fseek(L->File,L->OffsetTable[L->YSize*L->NextZ+L->NextY],SEEK_SET);
		if (ferror(L->File)) goto ErrFile;
		if (feof(L->File)) goto ErrFormat;
		for (x=0;;) {
			cnt=1;
			rpt=(unsigned char)fgetc(L->File);
			if (L->BytesPerChannel>1) rpt=(unsigned char)fgetc(L->File);
			if (rpt&0x80) {
				cnt=rpt&0x7f;
				rpt=1;
			}
			if (rpt*cnt==0) {
				if (x<L->XSize) goto ErrFormat;
				break;
			}
			if (x+rpt*cnt>L->XSize) goto ErrFormat;
			do {
				val=(unsigned char)fgetc(L->File);
				if (L->BytesPerChannel>1) val=(val<<8)|(unsigned char)fgetc(L->File);
				val=((val-L->PixMin)*255+(L->PixMax-L->PixMin)/2)/(L->PixMax-L->PixMin);
				for (i=0; i<rpt; i++) map[(x+i)*L->ZUse]=(unsigned char)val;
				x+=rpt;
				cnt--;
			} while (cnt>0);
		}
	}

	Signal(ChangeSignal);

	L->NextY++;
	if (L->NextY>=L->YSize) {
		L->NextY=0;
		L->NextZ++;
		if (L->NextZ>=L->ZUse) return true;
	}
	return false;

ErrFile:
	throw emGetErrorText(errno);
ErrFormat:
	throw emString("SGI image file format error.");
}


void emRgbImageFileModel::QuitLoading()
{
	if (L) {
		if (L->OffsetTable) delete [] L->OffsetTable;
		if (L->File) fclose(L->File);
		delete L;
		L=NULL;
	}
}


void emRgbImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emRgbImageFileModel: Saving not implemented.");
}


bool emRgbImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emRgbImageFileModel::QuitSaving()
{
}


emUInt64 emRgbImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->XSize)*L->YSize*L->ZUse;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emRgbImageFileModel::CalcFileProgress()
{
	if (L && L->YSize>0) {
		return 100.0*L->NextY/L->YSize;
	}
	else {
		return 0.0;
	}
}


int emRgbImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emRgbImageFileModel::Read16()
{
	int i;

	i=Read8()<<8;
	i|=Read8();
	return i;
}


int emRgbImageFileModel::Read32()
{
	int i;

	i=Read16()<<16;
	i|=Read16();
	return i;
}
