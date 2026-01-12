//------------------------------------------------------------------------------
// emPnmImageFileModel.cpp
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
}


emPnmImageFileModel::~emPnmImageFileModel()
{
}


void emPnmImageFileModel::TryStartLoading()
{
	L=new LoadingState;
	L->Format=0;
	L->Width=0;
	L->Height=0;
	L->MaxVal=0;
	L->NextY=0;
	L->ImagePrepared=false;

	L->File.TryOpen(GetFilePath(),"rb");

	if (L->File.TryReadUInt8()!='P') goto Err;
	L->Format=TryReadDecimal();
	if (L->Format<1 || L->Format>6) goto Err;

	L->Width=TryReadDecimal();
	L->Height=TryReadDecimal();
	if (L->Width<1 || L->Height<1) goto Err;
	if (L->Width>0x7fffff || L->Height>0x7fffff) goto Err;

	if (L->Format==2 || L->Format==3 || L->Format==5 || L->Format==6) {
		L->MaxVal=TryReadDecimal();
		if (L->MaxVal<1 || L->MaxVal>65535) goto Err;
	}

	return;

Err:
	throw emException("PNM format error");
}


bool emPnmImageFileModel::TryContinueLoading()
{
	unsigned char * map, * mapEnd;
	int i,n,v;

	if (L->Format==3 || L->Format==6) n=3; else n=1;

	if (!L->ImagePrepared) {
		Image.Setup(L->Width,L->Height,n);
		FileFormatInfo=FormatToString(L->Format);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	if (L->NextY>=L->Height) {
		return true;
	}

	map=Image.GetWritableMap()+L->NextY*(size_t)L->Width*n;
	mapEnd=map+n*L->Width;

	if (L->Format==1) {
		for (; map<mapEnd; map++) {
			v=TryReadDigit(true);
			if (v<0 || v>1) goto Err;
			map[0]=(unsigned char)(v?0:255);
		}
	}
	else if (L->Format==4) {
		for (i=0, v=0; map<mapEnd; map++, i=(i+1)&7) {
			if (i==0) {
				v=L->File.TryReadUInt8();
				if (v<0) goto Err;
			}
			map[0]=(unsigned char)((v&(128>>i))?0:255);
		}
	}
	else if (L->Format==2 || L->Format==5) {
		for (; map<mapEnd; map++) {
			v=TryReadVal();
			if (v<0) goto Err;
			map[0]=(unsigned char)v;
		}
	}
	else if (L->Format==3 || L->Format==6) {
		for (; map<mapEnd; map+=3) {
			v=TryReadVal();
			if (v<0) goto Err;
			map[0]=(unsigned char)v;
			v=TryReadVal();
			if (v<0) goto Err;
			map[1]=(unsigned char)v;
			v=TryReadVal();
			if (v<0) goto Err;
			map[2]=(unsigned char)v;
		}
	}

	Signal(ChangeSignal);

	L->NextY++;
	if (L->NextY>=L->Height) {
		return true;
	}

	return false;

Err:
	throw emException("PNM format error");
}


void emPnmImageFileModel::QuitLoading()
{
	L.Reset();
}


void emPnmImageFileModel::TryStartSaving()
{
	const char * ext;
	emArray<emColor> pal;
	int i;
	emByte r;

	S=new SavingState;
	S->Format=0;
	S->NextY=0;

	ext=emGetExtensionInPath(GetFilePath());
	if (strcasecmp(ext,".pbm")==0) {
		S->Format=4;
	}
	else if (strcasecmp(ext,".pgm")==0) {
		S->Format=5;
	}
	else if (strcasecmp(ext,".ppm")==0) {
		S->Format=6;
	}
	else if (GetImage().HasAnyNonGreyPixel()) {
		S->Format=6;
	}
	else {
		pal=GetImage().DetermineAllColorsSorted(2);
		if (!pal.IsEmpty()) {
			for (i=0; i<pal.GetCount(); i++) {
				r=pal[i].GetRed();
				if (r!=255 && r!=0) break;
			}
			S->Format=(i==pal.GetCount()?4:5);
		}
		else {
			S->Format=5;
		}
	}

	S->File.TryOpen(GetFilePath(),"wb");
	S->File.TryWrite(emString::Format(
		"P%d\n%d %d\n",S->Format,GetImage().GetWidth(),GetImage().GetHeight()
	));
	if (S->Format!=4) {
		S->File.TryWrite("255\n");
	}
}


bool emPnmImageFileModel::TryContinueSaving()
{
	emString str;
	int width,height,x,y,val,shift;
	emColor c;

	width=Image.GetWidth();
	height=Image.GetHeight();

	if (S->NextY<height) {
		y=S->NextY++;
		if (S->Format==4) {
			val=0;
			shift=8;
			for (x=0; x<width; x++) {
				c=Image.GetPixel(x,y);
				shift--;
				if (c.GetGrey()<128) val|=1<<shift;
				if (shift==0 || x+1>=width) {
					S->File.TryWriteUInt8((emUInt8)val);
					val=0;
					shift=8;
				}
			}
		}
		else if (S->Format==5) {
			for (x=0; x<width; x++) {
				c=Image.GetPixel(x,y);
				S->File.TryWriteUInt8(c.GetGrey());
			}
		}
		else {
			for (x=0; x<width; x++) {
				c=Image.GetPixel(x,y);
				S->File.TryWriteUInt8(c.GetRed());
				S->File.TryWriteUInt8(c.GetGreen());
				S->File.TryWriteUInt8(c.GetBlue());
			}
		}
		return false;
	}

	S->File.TryClose();

	str=FormatToString(S->Format);
	if (FileFormatInfo!=str) {
		FileFormatInfo=str;
		Signal(ChangeSignal);
	}

	return true;
}


void emPnmImageFileModel::QuitSaving()
{
	S.Reset();
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
	else if (S && GetImage().GetHeight()>0) {
		return 100.0*S->NextY/GetImage().GetHeight();
	}
	else {
		return 0.0;
	}
}


int emPnmImageFileModel::TryReadDigit(bool allowSpace)
{
	int c;

	for (;;) {
		c=L->File.TryReadCharOrEOF();
		if (c>='0' && c<='9') return c-'0';
		if (c=='#') {
			do {
				c=L->File.TryReadCharOrEOF();
				if (c<0) return -1;
			} while (c!=0x0a);
		}
		if (!allowSpace || c<0 || c>0x20) return -1;
	}
}


int emPnmImageFileModel::TryReadDecimal()
{
	int i,j;

	i=TryReadDigit(true);
	if (i>=0) {
		for (;;) {
			j=TryReadDigit(false);
			if (j<0) break;
			i=i*10+j;
		}
	}
	return i;
}


int emPnmImageFileModel::TryReadVal()
{
	int v;

	if (L->Format<=3) v=TryReadDecimal();
	else if (L->MaxVal<=255) v=L->File.TryReadUInt8();
	else v=L->File.TryReadUInt16BE();
	if (v<0 || v>L->MaxVal) return -1;
	return (v*255+L->MaxVal/2)/L->MaxVal;
}


const char * emPnmImageFileModel::FormatToString(int format)
{
	switch (format) {
		case 1: return "PNM P1 (PBM ASCII)";
		case 2: return "PNM P2 (PGM ASCII)";
		case 3: return "PNM P3 (PPM ASCII)";
		case 4: return "PNM P4 (PBM RAW)";
		case 5: return "PNM P5 (PGM RAW)";
		case 6: return "PNM P6 (PPM RAW)";
		default: return "Unknown PNM format";
	}
}
