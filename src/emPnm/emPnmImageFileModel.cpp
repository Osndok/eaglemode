//------------------------------------------------------------------------------
// emPnmImageFileModel.cpp
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

#include <emPnm/emPnmImageFileModel.h>


emRef<emPnmImageFileModel> emPnmImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPnmImageFileModel,context,name,common)
}


emPnmImageFileModel::emPnmImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emPnmImageFileModel::~emPnmImageFileModel()
{
	emPnmImageFileModel::QuitLoading();
	emPnmImageFileModel::QuitSaving();
}


void emPnmImageFileModel::TryStartLoading() throw(emString)
{
	errno=0;

	L=new LoadingState;
	memset(L,0,sizeof(LoadingState));

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;

	if (Read8()!='P') goto Err;
	L->Format=ReadDecimal();
	if (L->Format<1 || L->Format>6) goto Err;

	L->Width=ReadDecimal();
	L->Height=ReadDecimal();
	if (L->Width<1 || L->Height<1) goto Err;
	if (L->Width>65535 || L->Height>65535) goto Err;

	if (L->Format==2 || L->Format==3 || L->Format==5 || L->Format==6) {
		L->MaxVal=ReadDecimal();
		if (L->MaxVal<1 || L->MaxVal>65535) goto Err;
	}

	return;

Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("PNM format error");
}


bool emPnmImageFileModel::TryContinueLoading() throw(emString)
{
	unsigned char * map, * mapEnd;
	int i,n,v;

	errno=0;

	if (L->Format==3 || L->Format==6) n=3; else n=1;

	if (!L->ImagePrepared) {
		Image.Setup(L->Width,L->Height,n);
		switch (L->Format) {
		case 1: FileFormatInfo="PNM P1 (PBM ASCII)"; break;
		case 2: FileFormatInfo="PNM P2 (PGM ASCII)"; break;
		case 3: FileFormatInfo="PNM P3 (PPM ASCII)"; break;
		case 4: FileFormatInfo="PNM P4 (PBM RAW)"; break;
		case 5: FileFormatInfo="PNM P5 (PGM RAW)"; break;
		case 6: FileFormatInfo="PNM P6 (PPM RAW)"; break;
		}
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	if (L->NextY>=L->Height) {
		return true;
	}

	map=Image.GetWritableMap()+L->NextY*L->Width*n;
	mapEnd=map+n*L->Width;

	if (L->Format==1) {
		for (; map<mapEnd; map++) {
			v=ReadDigit(true);
			if (v<0 || v>1) goto Err;
			map[0]=(unsigned char)(v?0:255);
		}
	}
	else if (L->Format==4) {
		for (i=0, v=0; map<mapEnd; map++, i=(i+1)&7) {
			if (i==0) {
				v=Read8();
				if (v<0) goto Err;
			}
			map[0]=(unsigned char)((v&(128>>i))?0:255);
		}
	}
	else if (L->Format==2 || L->Format==5) {
		for (; map<mapEnd; map++) {
			v=ReadVal();
			if (v<0) goto Err;
			map[0]=(unsigned char)v;
		}
	}
	else if (L->Format==3 || L->Format==6) {
		for (; map<mapEnd; map+=3) {
			v=ReadVal();
			if (v<0) goto Err;
			map[0]=(unsigned char)v;
			v=ReadVal();
			if (v<0) goto Err;
			map[1]=(unsigned char)v;
			v=ReadVal();
			if (v<0) goto Err;
			map[2]=(unsigned char)v;
		}
	}

	Signal(ChangeSignal);

	if (ferror(L->File)) goto Err;

	L->NextY++;
	if (L->NextY>=L->Height) {
		return true;
	}

	return false;

Err:
	if (errno) throw emGetErrorText(errno);
	else throw emString("PNM format error");
}


void emPnmImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		delete L;
		L=NULL;
	}
}


void emPnmImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emPnmImageFileModel: Saving not implemented.");
}


bool emPnmImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emPnmImageFileModel::QuitSaving()
{
}


emUInt64 emPnmImageFileModel::CalcMemoryNeed()
{
	emUInt64 m;

	if (L) {
		m=((emUInt64)L->Width)*L->Height;
		if (L->Format==3 || L->Format==6) m*=3;
		return m;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emPnmImageFileModel::CalcFileProgress()
{
	if (L && L->Height>0) {
		return 100.0*L->NextY/L->Height;
	}
	else {
		return 0.0;
	}
}


int emPnmImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emPnmImageFileModel::Read16()
{
	int i;

	i=Read8()<<8;
	i|=Read8();
	return i;
}


int emPnmImageFileModel::ReadDigit(bool allowSpace)
{
	int c;

	for (;;) {
		c=fgetc(L->File);
		if (c>='0' && c<='9') return c-'0';
		if (c=='#') {
			do {
				c=fgetc(L->File);
				if (c<0) return -1;
			} while (c!=0x0a);
		}
		if (!allowSpace || c<0 || c>0x20) return -1;
	}
}


int emPnmImageFileModel::ReadDecimal()
{
	int i,j;

	i=ReadDigit(true);
	if (i>=0) {
		for (;;) {
			j=ReadDigit(false);
			if (j<0) break;
			i=i*10+j;
		}
	}
	return i;
}


int emPnmImageFileModel::ReadVal()
{
	int v;

	if (L->Format<=3) v=ReadDecimal();
	else if (L->MaxVal<=255) v=Read8();
	else v=Read16();
	if (v<0 || v>L->MaxVal) return -1;
	return (v*255+L->MaxVal/2)/L->MaxVal;
}
