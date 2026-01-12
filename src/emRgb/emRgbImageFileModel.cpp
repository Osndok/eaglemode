//------------------------------------------------------------------------------
// emRgbImageFileModel.cpp
//
// Copyright (C) 2004-2009,2014,2018-2019,2025 Oliver Hamann.
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


void emRgbImageFileModel::TryStartLoading()
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
	L->OffsetTable=NULL;

	L->File.TryOpen(GetFilePath(),"rb");

	magic=L->File.TryReadUInt16BE();
	L->Storage=L->File.TryReadUInt8();
	L->BytesPerChannel=L->File.TryReadUInt8();
	dimension=L->File.TryReadUInt16BE();
	L->XSize=L->File.TryReadUInt16BE();
	L->YSize=L->File.TryReadUInt16BE();
	L->ZSize=L->File.TryReadUInt16BE();
	L->PixMin=L->File.TryReadUInt32BE();
	L->PixMax=L->File.TryReadUInt32BE();
	L->File.TrySkip(84);
	colorMapId=L->File.TryReadUInt32BE();
	L->File.TrySkip(404);

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
ErrFormat:
	throw emException("SGI image file format error.");
ErrUnsupported:
	throw emException("Unsupported SGI image file format.");
}


bool emRgbImageFileModel::TryContinueLoading()
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
		(L->YSize-L->NextY-1)*(size_t)L->XSize*L->ZUse+L->NextZ
	;

	if (L->Storage==0) {
		for (x=0; x<L->XSize; x++) {
			val=L->File.TryReadUInt8();
			if (L->BytesPerChannel>1) val=(val<<8)|L->File.TryReadUInt8();
			val=((val-L->PixMin)*255+(L->PixMax-L->PixMin)/2)/(L->PixMax-L->PixMin);
			map[x*L->ZUse]=(unsigned char)val;
		}
	}
	else {
		if (L->Storage && !L->OffsetTable) {
			L->OffsetTable=new emUInt32[L->ZUse*L->YSize];
			for (x=0; x<L->ZUse*L->YSize; x++) L->OffsetTable[x]=L->File.TryReadUInt32BE();
			return false;
		}
		L->File.TrySeek(L->OffsetTable[L->YSize*L->NextZ+L->NextY]);
		for (x=0;;) {
			cnt=1;
			rpt=L->File.TryReadUInt8();
			if (L->BytesPerChannel>1) rpt=L->File.TryReadUInt8();
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
				val=L->File.TryReadUInt8();
				if (L->BytesPerChannel>1) val=(val<<8)|L->File.TryReadUInt8();
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

ErrFormat:
	throw emException("SGI image file format error.");
}


void emRgbImageFileModel::QuitLoading()
{
	if (L) {
		if (L->OffsetTable) delete [] L->OffsetTable;
		delete L;
		L=NULL;
	}
}


void emRgbImageFileModel::TryStartSaving()
{
	throw emException("emRgbImageFileModel: Saving not implemented.");
}


bool emRgbImageFileModel::TryContinueSaving()
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
