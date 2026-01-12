//------------------------------------------------------------------------------
// emTgaImageFileModel.cpp
//
// Copyright (C) 2004-2009,2014,2018,2025 Oliver Hamann.
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

#include <emTga/emTgaImageFileModel.h>


emRef<emTgaImageFileModel> emTgaImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emTgaImageFileModel,context,name,common)
}


emTgaImageFileModel::emTgaImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
}


emTgaImageFileModel::~emTgaImageFileModel()
{
}


void emTgaImageFileModel::TryStartLoading()
{
	int i,c;

	L=new LoadingState;
	L->RunCol.SetGrey(0);
	L->IDLen=0;
	L->CMapType=0;
	L->IMapType=0;
	L->CMapSize=0;
	L->CMapBitsPP=0;
	L->Width=0;
	L->Height=0;
	L->BitsPP=0;
	L->Descriptor=0;
	L->ChannelCount=0;
	L->NextY=0;
	L->RunLen=0;
	L->ImagePrepared=false;

	L->File.TryOpen(GetFilePath(),"rb");

	L->IDLen=L->File.TryReadUInt8();
	L->CMapType=L->File.TryReadUInt8();
	L->IMapType=L->File.TryReadUInt8();
	L->File.TryReadUInt16LE();
	L->CMapSize=L->File.TryReadUInt16LE();
	L->CMapBitsPP=L->File.TryReadUInt8();
	L->File.TryReadUInt16LE();
	L->File.TryReadUInt16LE();
	L->Width=L->File.TryReadUInt16LE();
	L->Height=L->File.TryReadUInt16LE();
	L->BitsPP=L->File.TryReadUInt8();
	L->Descriptor=L->File.TryReadUInt8();
	L->File.TrySkip(L->IDLen);

	if (L->Width<1 || L->Height<1) goto ErrFormat;

	if ((L->IMapType&~8)==1) {
		if (L->CMapType!=1) goto ErrFormat;
		if (L->BitsPP!=8 && L->BitsPP!=16) goto ErrFormat;
		L->Palette=new emColor[L->CMapSize];
		L->ChannelCount=1;
		for (i=0; i<L->CMapSize; i++) {
			if (L->CMapBitsPP==16) {
				c=L->File.TryReadUInt16LE();
				L->Palette[i].SetRed((emByte)((((c>>10)&31)*255)/31));
				L->Palette[i].SetGreen((emByte)((((c>>5)&31)*255)/31));
				L->Palette[i].SetBlue((emByte)(((c&31)*255)/31));
				L->Palette[i].SetAlpha((emByte)((c&0x8000)?255:0));
			}
			else if (L->CMapBitsPP==24) {
				L->Palette[i].SetBlue(L->File.TryReadUInt8());
				L->Palette[i].SetGreen(L->File.TryReadUInt8());
				L->Palette[i].SetRed(L->File.TryReadUInt8());
				L->Palette[i].SetAlpha(255);
			}
			else if (L->CMapBitsPP==32) {
				L->Palette[i].SetBlue(L->File.TryReadUInt8());
				L->Palette[i].SetGreen(L->File.TryReadUInt8());
				L->Palette[i].SetRed(L->File.TryReadUInt8());
				L->Palette[i].SetAlpha(L->File.TryReadUInt8());
			}
			else goto ErrFormat;
			if (L->ChannelCount<3 && !L->Palette[i].IsGrey()) {
				L->ChannelCount+=2;
			}
			if ((L->ChannelCount&1)!=0 && L->Palette[i].GetAlpha()!=255) {
				L->ChannelCount+=1;
			}
		}
	}
	else if ((L->IMapType&~8)==2) {
		if (L->CMapType!=0) goto ErrFormat;
		if (L->BitsPP==16) L->ChannelCount=4;
		else if (L->BitsPP==24) L->ChannelCount=3;
		else if (L->BitsPP==32) L->ChannelCount=4;
		else goto ErrFormat;
	}
	else if ((L->IMapType&~8)==3) {
		if (L->CMapType!=0) goto ErrFormat;
		if (L->BitsPP==8) L->ChannelCount=1;
		else if (L->BitsPP==16) L->ChannelCount=2;
		else goto ErrFormat;
	}
	else goto ErrFormat;

	if ((L->ChannelCount&1)==0 && (L->Descriptor&0x0f)==0) {
		L->ChannelCount--;
	}

	FileFormatInfo=emString::Format("Targa - %d bits/pixel",L->BitsPP);
	switch (L->IMapType) {
	case  1: FileFormatInfo+=" uncompressed color-mapped"; break;
	case  2: FileFormatInfo+=" uncompressed RGB"; break;
	case  3: FileFormatInfo+=" uncompressed grey"; break;
	case  9: FileFormatInfo+=" RLE-compressed color-mapped"; break;
	case 10: FileFormatInfo+=" RLE-compressed RGB"; break;
	case 11: FileFormatInfo+=" RLE-compressed grey"; break;
	}
	if ((L->Descriptor&0x0f)!=0) FileFormatInfo+=" with alpha";
	FileFormatInfo+=emString::Format(" (%d channels)",L->ChannelCount);
	Signal(ChangeSignal);

	return;

ErrFormat:
	throw emException("TGA format error");
}


bool emTgaImageFileModel::TryContinueLoading()
{
	int i,x,c;

	if (!L->ImagePrepared) {
		Image.Setup(L->Width,L->Height,L->ChannelCount);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	for (x=0; x<L->Width; x++) {
		if ((L->IMapType&8)!=0 && L->RunLen<0) L->RunLen++;
		else {
			if ((L->IMapType&8)!=0 && L->RunLen==0) {
				L->RunLen=L->File.TryReadUInt8();
				if (L->RunLen&0x80) L->RunLen=-(L->RunLen&0x7f)+1;
				else L->RunLen++;
			}
			L->RunLen--;
			for (c=0, i=0; i<L->BitsPP; i+=8) c|=L->File.TryReadUInt8()<<i;
			if ((L->IMapType&~8)==1) {
				if (c<0 || c>=L->CMapSize) throw emException("TGA format error");
				L->RunCol=L->Palette[c];
			}
			else if ((L->IMapType&~8)==2) {
				if (L->BitsPP==16) {
					L->RunCol.SetRed((emByte)((((c>>10)&31)*255)/31));
					L->RunCol.SetGreen((emByte)((((c>>5)&31)*255)/31));
					L->RunCol.SetBlue((emByte)(((c&31)*255)/31));
					L->RunCol.SetAlpha((emByte)((c&0x8000)?255:0));
				}
				else {
					L->RunCol.SetRed((emByte)(c>>16));
					L->RunCol.SetGreen((emByte)(c>>8));
					L->RunCol.SetBlue((emByte)c);
					L->RunCol.SetAlpha((emByte)(L->BitsPP==24 ? 255 : c>>24));
				}
			}
			else {
				L->RunCol.SetRed((emByte)c);
				L->RunCol.SetGreen((emByte)c);
				L->RunCol.SetBlue((emByte)c);
				L->RunCol.SetAlpha((emByte)(L->BitsPP==8 ? 255 : c>>8));
			}
		}
		Image.SetPixel(
			x,
			(L->Descriptor&0x20)?L->NextY:L->Height-L->NextY-1,
			L->RunCol
		);
	}

	Signal(ChangeSignal);

	L->NextY++;
	if (L->NextY>=L->Height) {
		return true;
	}
	return false;
}


void emTgaImageFileModel::QuitLoading()
{
	L.Reset();
}


void emTgaImageFileModel::TryStartSaving()
{
	if (GetImage().GetWidth()>65535 || GetImage().GetHeight()>65535) {
		throw emException(
			"Image to large for a TGA file (maximum possible size is 65535 x 65535)"
		);
	}

	S=new SavingState;
	S->HaveColor=GetImage().HasAnyNonGreyPixel();
	S->HaveAlpha=GetImage().HasAnyTransparentPixel();
	if (S->HaveColor || S->HaveAlpha) {
		S->Pal=GetImage().DetermineAllColorsSorted(256);
	}

	S->PixelSize=
		!S->Pal.IsEmpty() ? 1 : (S->HaveColor ? 3 : 1) + (S->HaveAlpha ? 1 : 0)
	;

	S->NextY=0;
}


bool emTgaImageFileModel::TryContinueSaving()
{
	int i,j,k,y,width,height,palSize,pixelSize;
	const emColor * pal;
	emString str;
	emColor c;
	emInt32 v;

	width=GetImage().GetWidth();
	height=GetImage().GetHeight();

	if (!S->Encoder) {
		S->File.TryOpen(GetFilePath(),"wb");
		S->File.TryWriteUInt8(0);
		S->File.TryWriteUInt8(!S->Pal.IsEmpty() ? 1 : 0);
		S->File.TryWriteUInt8(!S->Pal.IsEmpty() ? 9 : S->HaveColor ? 10 : 11);
		S->File.TryWriteUInt16LE(0);
		S->File.TryWriteUInt16LE(S->Pal.GetCount());
		S->File.TryWriteUInt8(S->Pal.IsEmpty() ? 0 : S->HaveAlpha ? 32 : 24);
		S->File.TryWriteUInt16LE(0);
		S->File.TryWriteUInt16LE(0);
		S->File.TryWriteUInt16LE(width);
		S->File.TryWriteUInt16LE(height);
		S->File.TryWriteUInt8(S->PixelSize*8);
		S->File.TryWriteUInt8(0x20 + (S->HaveAlpha ? 8 : 0));
		for (emColor col: S->Pal) {
			S->File.TryWriteUInt8(col.GetBlue());
			S->File.TryWriteUInt8(col.GetGreen());
			S->File.TryWriteUInt8(col.GetRed());
			if (S->HaveAlpha) S->File.TryWriteUInt8(col.GetAlpha());
		}
		S->Encoder=new RleEncoder(S->File,S->PixelSize);
		return false;
	}

	y=S->NextY++;
	if (y<height) {
		pal=S->Pal.Get();
		palSize=S->Pal.GetCount();
		pixelSize=S->PixelSize;
		for (int x=0; x<width; x++) {
			c=Image.GetPixel(x,y);
			if (palSize>0) {
				for (i=0, j=palSize; i<j;) {
					k=(i+j)/2;
					if (pal[k].Get()<c.Get()) i=k+1; else j=k;
				}
				v=i;
			}
			else {
				if (pixelSize<=2) {
					v=c.GetGreen();
					if (pixelSize==2) v|=c.GetAlpha()<<8;
				}
				else {
					v=c.GetBlue()|(c.GetGreen()<<8)|(c.GetRed()<<16);
					if (pixelSize==4) v|=c.GetAlpha()<<24;
				}
			}
			S->Encoder->TryPut(v);
		}
		return false;
	}

	S->Encoder->TryFlush();
	S->File.TryClose();

	str=emString::Format("Targa - %d bits/pixel",S->PixelSize*8);
	if (!S->HaveColor)          str+=" RLE-compressed grey";
	else if (!S->Pal.IsEmpty()) str+=" RLE-compressed color-mapped";
	else                        str+=" RLE-compressed RGB";
	if (S->HaveAlpha) str+=" with alpha";
	str+=emString::Format(
		" (%d channels)",
		(S->HaveColor ? 3 : 1) + (S->HaveAlpha ? 1 : 0)
	);
	if (FileFormatInfo!=str) {
		FileFormatInfo=str;
		Signal(ChangeSignal);
	}

	return true;
}


void emTgaImageFileModel::QuitSaving()
{
	S.Reset();
}


emUInt64 emTgaImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->Width)*L->Height*L->ChannelCount;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emTgaImageFileModel::CalcFileProgress()
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


emTgaImageFileModel::RleEncoder::RleEncoder(emFileStream & file, int pixelSize)
	: File(file),
	PixelSize(pixelSize),
	Pos(0),
	Fill(0)
{
}


void emTgaImageFileModel::RleEncoder::TryFlush()
{
	while (Fill) TryWriteNext();
}


void emTgaImageFileModel::RleEncoder::TryWriteNext()
{
	emUInt32 c,c2;
	int i,n,k;

	c=Buf[Pos];
	if (Fill>=2 && c==Buf[(Pos+1)&255]) {
		n=2;
		k=Fill; if (k>128) k=128;
		while (n<k && c==Buf[(Pos+n)&255]) n++;
		File.TryWriteUInt8((emUInt8)(0x7f+n));
		switch (PixelSize) {
			case 4:
				File.TryWriteUInt32LE(c);
				break;
			case 3:
				File.TryWriteUInt8((emUInt8)c);
				File.TryWriteUInt8((emUInt8)(c>>8));
				File.TryWriteUInt8((emUInt8)(c>>16));
				break;
			case 2:
				File.TryWriteUInt16LE((emUInt16)c);
				break;
			default:
				File.TryWriteUInt8((emUInt8)c);
		}
		Pos=(Pos+n)&255;
		Fill-=n;
	}
	else {
		n=Fill;
		if (n>128) n=128;
		k=n+1;
		if (k>Fill) k=Fill;
		for (i=1; i<k; i++) {
			c2=Buf[(Pos+i)&255];
			if (c==c2) {
				if (
					PixelSize>1 ||
					i+1>=k ||
					c==Buf[(Pos+i+1)&255]
				) {
					n=i-1;
					break;
				}
			}
			c=c2;
		}
		File.TryWriteUInt8((emUInt8)(n-1));
		Fill-=n;
		do {
			c=Buf[Pos];
			switch (PixelSize) {
				case 4:
					File.TryWriteUInt32LE(c);
					break;
				case 3:
					File.TryWriteUInt8((emUInt8)c);
					File.TryWriteUInt8((emUInt8)(c>>8));
					File.TryWriteUInt8((emUInt8)(c>>16));
					break;
				case 2:
					File.TryWriteUInt16LE((emUInt16)c);
					break;
				default:
					File.TryWriteUInt8((emUInt8)c);
			}
			n--;
			Pos=(Pos+1)&255;
		} while (n>0);
	}
}
