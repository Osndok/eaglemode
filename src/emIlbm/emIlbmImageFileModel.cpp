//------------------------------------------------------------------------------
// emIlbmImageFileModel.cpp
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


void emIlbmImageFileModel::TryStartLoading()
{
	emUInt32 name;

	L=new LoadingState;
	L->HeaderFound=false;
	L->Width=0;
	L->Height=0;
	L->Depth=0;
	L->Compress=0;
	L->NextY=0;
	L->Palette=NULL;
	L->RowBuf=NULL;
	L->BodyPos=0;

	L->File.TryOpen(GetFilePath(),"rb");

	L->File.TryReadUInt32BE();
	L->File.TryReadUInt32BE();
	name=L->File.TryReadUInt32BE();

	if (name!=0x494C424D/*ILBM*/) {
		throw emException("ILBM format error");
	}
}


bool emIlbmImageFileModel::TryContinueLoading()
{
	unsigned char * map, * row, * bm;
	long size;
	emUInt32 name;
	int x,y,bb,b,d,pbw,c,cb;

	if (!L->BodyPos || !L->Palette || !L->HeaderFound) {
		name=L->File.TryReadUInt32BE();
		size=L->File.TryReadUInt32BE();
		if (name==0x424D4844/*BMHD*/) {
			if (size<11) goto ErrFormat;
			L->Width=L->File.TryReadUInt16BE();
			L->Height=L->File.TryReadUInt16BE();
			L->File.TryReadUInt32BE();
			L->Depth=L->File.TryReadUInt8();
			L->File.TryReadUInt8();
			L->Compress=L->File.TryReadUInt8();
			L->File.TrySkip(((size+1)&~1)-11);
			if (L->Depth>8 || L->Compress>1 ||
			    L->Width<=0 || L->Height<=0) goto ErrFormat;
			L->HeaderFound=true;
		}
		else if (name==0x434D4150/*CMAP*/) {
			if (!L->HeaderFound || L->Palette || size<(3<<L->Depth)) goto ErrFormat;
			L->Palette=new unsigned char[3<<L->Depth];
			L->File.TryRead(L->Palette,3<<L->Depth);
			L->File.TrySkip(((size+1)&~1)-(3<<L->Depth));
		}
		else if (name==0x424F4459/*BODY*/) {
			if (!L->HeaderFound || L->BodyPos) goto ErrFormat;
			L->BodyPos=L->File.TryTell();
			L->File.TrySkip((size+1)&~1);
		}
		else {
			L->File.TrySkip((size+1)&~1);
		}
		return false;
	}

	if (!L->RowBuf) {
		L->RowBuf=new unsigned char[L->Width+15];
		L->File.TrySeek(L->BodyPos);
		Image.Setup(L->Width,L->Height,3);
		return false;
	}

	if (L->NextY<L->Height) {
		y=L->NextY++;
		map=Image.GetWritableMap()+y*(size_t)Image.GetWidth()*Image.GetChannelCount();
		pbw=((L->Width+15)&0xfff0)>>3;
		row=L->RowBuf;
		memset(row,0,L->Width);
		cb=1;
		for(d=0; d<L->Depth; d++) {
			bm=row;
			x=0;
			if (L->Compress==0) {
				while (x<pbw) {
					b=L->File.TryReadUInt8();
					for(bb=128; bb!=0; bb>>=1) {
						if ( (b & bb) !=0) *bm|=(unsigned char)cb;
						bm++;
					}
					x++;
				}
			}
			else { // Compress==1
				while(x<pbw) {
					c=L->File.TryReadUInt8();
					if(c<128) {
						c=c+x; if (c>=pbw) c=pbw-1;
						while (x<=c) {
							b=L->File.TryReadUInt8();
							for(bb=128; bb!=0; bb>>=1) {
								if ( (b & bb) !=0) *bm|=(unsigned char)cb;
								bm++;
							}
							x++;
						}
					}
					else if (c!=128) {
						c=256-c+x; if (c>=pbw) c=pbw-1;
						b=L->File.TryReadUInt8();
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
		return false;
	}

	FileFormatInfo=emString::Format(
		"ILBM %d-bit %s",
		L->Depth,
		L->Compress ? "RLE-compressed" : "uncompressed"
	);

	Signal(ChangeSignal);

	return true;

ErrFormat:
	throw emException("ILBM format error");
}


void emIlbmImageFileModel::QuitLoading()
{
	if (L) {
		if (L->Palette) delete [] L->Palette;
		if (L->RowBuf) delete [] L->RowBuf;
		delete L;
		L=NULL;
	}
}


void emIlbmImageFileModel::TryStartSaving()
{
	throw emException("emIlbmImageFileModel: Saving not implemented.");
}


bool emIlbmImageFileModel::TryContinueSaving()
{
	return true;
}


void emIlbmImageFileModel::QuitSaving()
{
}


emUInt64 emIlbmImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->Width)*(L->Height*3+1);
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
		if (L->HeaderFound) progress+=5.0;
		if (L->Palette) progress+=10.0;
		if (L->Height>0) {
			progress+=85.0*L->NextY/L->Height;
		}
	}
	return progress;
}
