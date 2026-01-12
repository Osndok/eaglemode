//------------------------------------------------------------------------------
// emPngImageFileModel.cpp
//
// Copyright (C) 2004-2009,2011,2014,2018-2019,2022,2025 Oliver Hamann.
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

#include <emPng/emPngImageFileModel.h>
#include <emPng/emPngFunctions.h>


emRef<emPngImageFileModel> emPngImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPngImageFileModel,context,name,common)
}


emPngImageFileModel::emPngImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
	S=NULL;
}


emPngImageFileModel::~emPngImageFileModel()
{
	emPngImageFileModel::QuitLoading();
	emPngImageFileModel::QuitSaving();
}


void emPngImageFileModel::TryStartLoading()
{
	char infoBuf[1024];
	char errorBuf[256];

	L=new LoadingState;
	memset(L,0,sizeof(LoadingState));

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) throw emException("%s",emGetErrorText(errno).Get());

	infoBuf[0]=0;
	errorBuf[0]=0;
	L->DecodeInstance=emPngStartDecoding(
		L->File,&L->Width,&L->Height,&L->ChannelCount,&L->PassCount,
		infoBuf,sizeof(infoBuf),errorBuf,sizeof(errorBuf)
	);
	if (!L->DecodeInstance) throw emException("%s",errorBuf);

	FileFormatInfo=infoBuf;
	Signal(ChangeSignal);
}


bool emPngImageFileModel::TryContinueLoading()
{
	char commentBuf[1024];
	char errorBuf[256];
	int r;

	if (!L->ImagePrepared) {
		Image.Setup(
			L->Width,
			L->Height,
			L->ChannelCount
		);
		Signal(ChangeSignal);
		L->ImagePrepared=true;
		return false;
	}

	commentBuf[0]=0;
	errorBuf[0]=0;
	r=emPngContinueDecoding(
		L->DecodeInstance,
		Image.GetWritableMap()+L->Y*(size_t)Image.GetWidth()*Image.GetChannelCount(),
		commentBuf,sizeof(commentBuf),errorBuf,sizeof(errorBuf)
	);
	if (r<0) throw emException("%s",errorBuf);

	L->Y++;
	if (L->Y>=L->Height) {
		L->Pass++;
		L->Y=0;
	}

	Comment+=commentBuf;

	Signal(ChangeSignal);

	return r!=0;
}


void emPngImageFileModel::QuitLoading()
{
	if (L) {
		if (L->DecodeInstance) emPngQuitDecoding(L->DecodeInstance);
		if (L->File) fclose(L->File);
		delete L;
		L=NULL;
	}
}


void emPngImageFileModel::TryStartSaving()
{
	char errorBuf[256];
	emOwnArrayPtr<emByte> palette;
	bool anyNonGrey,anyAlpha;
	int i,width,height;
	emColor c;

	S=new SavingState;
	S->File=NULL;
	S->EncodeInstance=NULL;
	S->BitDepth=0;
	S->PixelBits=0;
	S->RowSize=0;
	S->Y=0;

	width=GetImage().GetWidth();
	height=GetImage().GetHeight();

	anyNonGrey=GetImage().HasAnyNonGreyPixel();
	anyAlpha=GetImage().HasAnyTransparentPixel();
	S->Pal=GetImage().DetermineAllColorsSorted(256);

	if (!S->Pal.IsEmpty() && (S->Pal.GetCount()<=16 || anyNonGrey || anyAlpha)) {
		if (S->Pal.GetCount()<=2) S->BitDepth=1;
		else if (S->Pal.GetCount()<=4) S->BitDepth=2;
		else if (S->Pal.GetCount()<=16) S->BitDepth=4;
		else S->BitDepth=8;
		S->PixelBits=S->BitDepth;
		palette=new emByte[4*S->Pal.GetCount()];
		for (i=0; i<S->Pal.GetCount(); i++) {
			c=S->Pal[i];
			palette[i*4  ]=c.GetRed();
			palette[i*4+1]=c.GetGreen();
			palette[i*4+2]=c.GetBlue();
			palette[i*4+3]=c.GetAlpha();
		}
	}
	else {
		S->Pal.Clear();
		S->BitDepth=8;
		S->PixelBits=8;
		if (anyNonGrey) S->PixelBits+=16;
		if (anyAlpha) S->PixelBits+=8;
	}

	S->File=fopen(GetFilePath(),"wb");
	if (!S->File) throw emException("%s",emGetErrorText(errno).Get());

	errorBuf[0]=0;
	S->EncodeInstance=emPngStartEncoding(
		S->File,width,height,S->BitDepth,S->PixelBits,palette.Get(),
		S->Pal.GetCount(),Comment.Get(),errorBuf,sizeof(errorBuf)
	);
	if (!S->EncodeInstance) throw emException("%s",errorBuf);

	S->RowSize=(width*S->PixelBits+31)/32*4;
	S->RowBuf=new emByte[S->RowSize];
}


bool emPngImageFileModel::TryContinueSaving()
{
	char errorBuf[256];
	emByte * row;
	const emColor * pal;
	int r,x,y,i,j,k,palSize,bitDepth,pixelBits,width,height;
	emColor c;

	width=GetImage().GetWidth();
	height=GetImage().GetHeight();
	y=S->Y++;
	if (y<height) {
		row=S->RowBuf.Get();
		memset(row,0,S->RowSize);
		pal=S->Pal.Get();
		palSize=S->Pal.GetCount();
		bitDepth=S->BitDepth;
		pixelBits=S->PixelBits;
		if (palSize>0) {
			for (x=0; x<width; x++) {
				c=GetImage().GetPixel(x,y);
				for (i=0, j=palSize; i<j;) {
					k=(i+j)/2;
					if (pal[k].Get()<c.Get()) i=k+1; else j=k;
				}
				switch (bitDepth) {
				case 1:
					row[x>>3]|=(emByte)(i<<(7-(x&7)));
					break;
				case 2:
					row[x>>2]|=(emByte)(i<<(6-(x&3)*2));
					break;
				case 4:
					row[x>>1]|=(emByte)(i<<(4-(x&1)*4));
					break;
				default:
					row[x]=(emByte)i;
				}
			}
		}
		else {
			for (x=0; x<width; x++) {
				c=GetImage().GetPixel(x,y);
				*row++=c.GetRed();
				if (pixelBits>=24) {
					*row++=c.GetGreen();
					*row++=c.GetBlue();
				}
				if ((pixelBits&8)==0) {
					*row++=c.GetAlpha();
				}
			}
		}
	}

	errorBuf[0]=0;
	r=emPngContinueEncoding(
		S->EncodeInstance,S->RowBuf.Get(),errorBuf,sizeof(errorBuf)
	);
	if (r<0) throw emException("%s",errorBuf);

	if (fflush(S->File)!=0) {
		throw emException("%s",emGetErrorText(errno).Get());
	}

	return r!=0;
}


void emPngImageFileModel::QuitSaving()
{
	if (S) {
		if (S->EncodeInstance) emPngQuitEncoding(S->EncodeInstance);
		if (S->File) fclose(S->File);
		delete S;
		S=NULL;
	}
}


emUInt64 emPngImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->Width)*
		       L->Height*
		       L->ChannelCount;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emPngImageFileModel::CalcFileProgress()
{
	if (L && L->Height>0 && L->PassCount>0) {
		return 100.0*(L->Pass*L->Height+L->Y)/(L->PassCount*L->Height);
	}
	else if (S && GetImage().GetHeight()>0) {
		return 100.0*S->Y/GetImage().GetHeight();
	}
	else {
		return 0.0;
	}
}
