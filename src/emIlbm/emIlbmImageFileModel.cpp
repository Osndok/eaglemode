//------------------------------------------------------------------------------
// emIlbmImageFileModel.cpp
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

#include <emIlbm/emIlbmImageFileModel.h>


emRef<emIlbmImageFileModel> emIlbmImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emIlbmImageFileModel,context,name,common)
}


emIlbmImageFileModel::emIlbmImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emIlbmImageFileModel::~emIlbmImageFileModel()
{
	emIlbmImageFileModel::QuitLoading();
	emIlbmImageFileModel::QuitSaving();
}


void emIlbmImageFileModel::TryStartLoading() throw(emString)
{
	int name;

	L=new LoadingState;
	L->HeaderFound=false;
	L->Width=0;
	L->Height=0;
	L->Depth=0;
	L->Compress=0;
	L->File=NULL;
	L->Palette=NULL;
	L->Body=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) throw emGetErrorText(errno);

	Read32();
	Read32();
	name=Read32();

	if (ferror(L->File)) throw emGetErrorText(errno);

	if (feof(L->File) || name!=0x494C424D/*ILBM*/) {
		throw emString("ILBM format error");
	}
}


bool emIlbmImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map, * row, * bm, * p;
	int name,size;
	int x,y,bb,b,d,pbw,c,cb;

	if (!L->HeaderFound || !L->Palette || !L->Body) {
		name=Read32();
		size=Read32();
		if (ferror(L->File)) goto ErrFile;
		if (feof(L->File)) goto ErrFormat;
		if (name==0x424D4844/*BMHD*/) {
			L->Width=Read16();
			L->Height=Read16();
			Read32();
			L->Depth=Read8();
			Read8();
			L->Compress=Read8();
			fseek(L->File,((size+1)&~1)-11,SEEK_CUR);
			if (ferror(L->File)) goto ErrFile;
			if (L->Depth>8 || L->Compress>1 ||
			    L->Width<=0 || L->Height<=0) goto ErrFormat;
			L->HeaderFound=true;
		}
		else if (name==0x434D4150/*CMAP*/) {
			if (!L->HeaderFound || L->Palette) goto ErrFormat;
			L->Palette=new unsigned char[3<<L->Depth];
			if (fread(L->Palette,1,3<<L->Depth,L->File)!=(size_t)(3<<L->Depth)) goto ErrFormat;
			fseek(L->File,((size+1)&~1)-(3<<L->Depth),SEEK_CUR);
			if (ferror(L->File)) goto ErrFile;
		}
		else if (name==0x424F4459/*BODY*/) {
			if (!L->HeaderFound || L->Body) goto ErrFormat;
			L->Body=new unsigned char[size];
			if (fread(L->Body,1,size,L->File)!=(size_t)size) goto ErrFormat;
			fseek(L->File,((size+1)&~1)-size,SEEK_CUR);
			if (ferror(L->File)) goto ErrFile;
		}
		else {
			fseek(L->File,(size+1)&~1,SEEK_CUR);
			if (ferror(L->File)) goto ErrFile;
		}
		return false;
	}

	Image.Setup(L->Width,L->Height,3);
	map=Image.GetWritableMap();
	pbw=((L->Width+15)&0xfff0)>>3;
	p=L->Body;
	row=new unsigned char[L->Width+15];
	for(y=0; y<L->Height; y++) {
		memset(row,0,L->Width);
		cb=1;
		for(d=0; d<L->Depth; d++) {
			bm=row;
			x=0;
			if (L->Compress==0) {
				while (x<pbw) {
					b=*(p++);
					for(bb=128; bb!=0; bb>>=1) {
						if ( (b & bb) !=0) *bm|=(unsigned char)cb;
						bm++;
					}
					x++;
				}
			}
			else { // Compress==1
				while(x<pbw) {
					c=*(p++);
					if(c<128) {
						c=c+x; if (c>=pbw) c=pbw-1;
						while (x<=c) {
							b=*(p++);
							for(bb=128; bb!=0; bb>>=1) {
								if ( (b & bb) !=0) *bm|=(unsigned char)cb;
								bm++;
							}
							x++;
						}
					}
					else if (c!=128) {
						c=256-c+x; if (c>=pbw) c=pbw-1;
						b=*(p++);
						while (x<=c) {
							for(bb=128; bb!=0; bb>>=1) {
								if ( (b & bb) !=0) *bm|=(unsigned char)cb;
								bm++;
							}
							x++;
						}
					}
				}
			}
			cb<<=1;
		}
		for (x=0; x<L->Width; x++) {
			map[0]=L->Palette[row[x]*3+0];
			map[1]=L->Palette[row[x]*3+1];
			map[2]=L->Palette[row[x]*3+2];
			map+=3;
		}
	}
	delete [] row;

	FileFormatInfo=emString::Format(
		"ILBM %d-bit %s",
		L->Depth,
		L->Compress ? "RLE-compressed" : "uncompressed"
	);

	Signal(ChangeSignal);

	return true;

ErrFile:
	throw emGetErrorText(errno);
ErrFormat:
	throw emString("ILBM format error");
}


void emIlbmImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Palette) delete [] L->Palette;
		if (L->Body) delete [] L->Body;
		delete L;
		L=NULL;
	}
}


void emIlbmImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emIlbmImageFileModel: Saving not implemented.");
}


bool emIlbmImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emIlbmImageFileModel::QuitSaving()
{
}


emUInt64 emIlbmImageFileModel::CalcMemoryNeed()
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


double emIlbmImageFileModel::CalcFileProgress()
{
	double progress;

	progress=0.0;
	if (L) {
		if (L->HeaderFound) progress+=10.0;
		if (L->Palette) progress+=30.0;
		if (L->Body) progress+=59.0;
	}
	return progress;
}


int emIlbmImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emIlbmImageFileModel::Read16()
{
	int i;

	i=Read8()<<8;
	i|=Read8();
	return i;
}


int emIlbmImageFileModel::Read32()
{
	int i;

	i=Read16()<<16;
	i|=Read16();
	return i;
}
