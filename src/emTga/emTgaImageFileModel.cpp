//------------------------------------------------------------------------------
// emTgaImageFileModel.cpp
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
	L=NULL;
}


emTgaImageFileModel::~emTgaImageFileModel()
{
	emTgaImageFileModel::QuitLoading();
	emTgaImageFileModel::QuitSaving();
}


void emTgaImageFileModel::TryStartLoading() throw(emString)
{
	int i,c;

	L=new LoadingState;
	L->File=NULL;
	L->Palette=NULL;
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

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto ErrFile;

	L->IDLen=Read8();
	L->CMapType=Read8();
	L->IMapType=Read8();
	Read16();
	L->CMapSize=Read16();
	L->CMapBitsPP=Read8();
	Read16();
	Read16();
	L->Width=Read16();
	L->Height=Read16();
	L->BitsPP=Read8();
	L->Descriptor=Read8();
	for (i=0; i<L->IDLen; i++) Read8();

	if (ferror(L->File)) goto ErrFile;
	if (feof(L->File)) goto ErrFormat;
	if (L->Width<1 || L->Height<1) goto ErrFormat;

	if ((L->IMapType&~8)==1) {
		if (L->CMapType!=1) goto ErrFormat;
		if (L->BitsPP!=8 && L->BitsPP!=16) goto ErrFormat;
		L->Palette=new emColor[L->CMapSize];
		L->ChannelCount=1;
		for (i=0; i<L->CMapSize; i++) {
			if (L->CMapBitsPP==16) {
				c=Read16();
				L->Palette[i].SetRed((emByte)((((c>>10)&31)*255)/31));
				L->Palette[i].SetGreen((emByte)((((c>>5)&31)*255)/31));
				L->Palette[i].SetBlue((emByte)(((c&31)*255)/31));
				L->Palette[i].SetAlpha((emByte)((c&0x8000)?255:0));
			}
			else if (L->CMapBitsPP==24) {
				L->Palette[i].SetBlue((emByte)Read8());
				L->Palette[i].SetGreen((emByte)Read8());
				L->Palette[i].SetRed((emByte)Read8());
				L->Palette[i].SetAlpha(255);
			}
			else if (L->CMapBitsPP==32) {
				L->Palette[i].SetBlue((emByte)Read8());
				L->Palette[i].SetGreen((emByte)Read8());
				L->Palette[i].SetRed((emByte)Read8());
				L->Palette[i].SetAlpha((emByte)Read8());
			}
			else goto ErrFormat;
			if (L->ChannelCount<3 && !L->Palette[i].IsGrey()) {
				L->ChannelCount+=2;
			}
			if ((L->ChannelCount&1)!=0 && L->Palette[i].GetAlpha()!=255) {
				L->ChannelCount+=1;
			}
		}
		if (ferror(L->File)) goto ErrFile;
		if (feof(L->File)) goto ErrFormat;
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

ErrFile:
	throw emGetErrorText(errno);
ErrFormat:
	throw emString("TGA format error");
}


bool emTgaImageFileModel::TryContinueLoading() throw(emString)
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
				L->RunLen=Read8();
				if (L->RunLen&0x80) L->RunLen=-(L->RunLen&0x7f)+1;
				else L->RunLen++;
			}
			L->RunLen--;
			for (c=0, i=0; i<L->BitsPP; i+=8) c|=(Read8()&255)<<i;
			if ((L->IMapType&~8)==1) {
				if (c<0 || c>=L->CMapSize) throw emString("TGA format error");
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

	if (ferror(L->File)) throw emGetErrorText(errno);

	L->NextY++;
	if (L->NextY>=L->Height) {
		return true;
	}
	return false;
}


void emTgaImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Palette) delete [] L->Palette;
		delete L;
		L=NULL;
	}
}


void emTgaImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emTgaImageFileModel: Saving not implemented.");
}


bool emTgaImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emTgaImageFileModel::QuitSaving()
{
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
	else {
		return 0.0;
	}
}


int emTgaImageFileModel::Read8()
{
	return (unsigned char)fgetc(L->File);
}


int emTgaImageFileModel::Read16()
{
	int i;

	i=Read8();
	i|=Read8()<<8;
	return i;
}
