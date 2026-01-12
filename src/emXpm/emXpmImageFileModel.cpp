//------------------------------------------------------------------------------
// emXpmImageFileModel.cpp
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

#include <emXpm/emXpmImageFileModel.h>


emRef<emXpmImageFileModel> emXpmImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emXpmImageFileModel,context,name,common)
}


emXpmImageFileModel::emXpmImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
}


emXpmImageFileModel::~emXpmImageFileModel()
{
}


void emXpmImageFileModel::TryStartLoading()
{
	emInt64 l;

	L=new LoadingState;
	L->FileSize=0;
	L->BufferFill=0;
	L->File.TryOpen(GetFilePath(),"rb");
	L->File.TrySeekEnd();
	l=L->File.TryTell();
	if (l>INT_MAX-3) throw emException("File too large.");
	L->FileSize=(int)l;
	L->File.TrySeek(0);
}


bool emXpmImageFileModel::TryContinueLoading()
{
	char * p;
	long w,h,c,s;
	int i,pos,len;

	if (!L->Buffer) {
		L->Buffer=new char[L->FileSize];
	}
	else if (L->File.IsOpen()) {
		len=L->FileSize-L->BufferFill;
		if (len>4096) len=4096;
		if (len>0) len=L->File.TryReadAtMost(L->Buffer+L->BufferFill,len);
		if (len>0) L->BufferFill+=len;
		else L->File.Close();
	}
	else if (!L->StringArray) {
		if (L->BufferFill<9 || memcmp(L->Buffer.Get(),"/* XPM */",9)!=0) {
			throw emException("Not an XPM file.");
		}
		for (i=0, pos=0; FindCString(pos,&pos,&len); i++) pos+=len+1;
		L->StringArray=new char*[i+1];
		for (i=0, pos=0; FindCString(pos,&pos,&len); i++) {
			L->StringArray[i]=L->Buffer+pos;
			pos+=len;
			L->Buffer[pos]=0;
			pos++;
		}
		L->StringArray[i]=NULL;
	}
	else {
		p=L->StringArray[0];
		if (!p) throw emException("Illegal XPM file format.");
		w=strtol(p,&p,0);
		h=strtol(p,&p,0);
		c=strtol(p,&p,0);
		s=strtol(p,&p,0);
		if (
			w<1 || w>0x7fffff ||
			h<1 || h>0x7fffff ||
			c<1 || c>0x1000000 ||
			s<1 || s>4 ||
			(w*s+3)*(emInt64)h+c*(s+4)>L->BufferFill
		) {
			throw emException("Unsupported XPM file format.");
		}

		Image.TryParseXpm(L->StringArray);
		FileFormatInfo="XPM";
		Signal(ChangeSignal);
		return true;
	}
	return false;
}


void emXpmImageFileModel::QuitLoading()
{
	L.Reset();
}


void emXpmImageFileModel::TryStartSaving()
{
	S=new SavingState;
	S->MaxPalSize=emMin(
		GetImage().GetWidth()*GetImage().GetHeight(),
		emMin(
			XpmSymCharsCount*XpmSymCharsCount*XpmSymCharsCount*XpmSymCharsCount,
			1000000
		)
	);
	S->PalSize=0;
	S->PixSize=0;
	S->Stage=0;
	S->Index=0;
	S->Pal=new emUInt32[S->MaxPalSize];
}


bool emXpmImageFileModel::TryContinueSaving()
{
	int x,y,width,height,palSize,pixSize,i,j,k;
	emUInt32 * pal;
	emUInt32 c;
	emColor col;

	width=GetImage().GetWidth();
	height=GetImage().GetHeight();

	switch (S->Stage) {
	case 0:
		y=S->Index++;
		if (y>=height) {
			S->Stage++;
			S->Index=0;
			return false;
		}
		palSize=S->PalSize;
		pal=S->Pal;
		for (x=0; x<width; x++) {
			col=GetImage().GetPixel(x,y);
			if (col.GetAlpha()<128) c=0;
			else c=0x01000000|(col.GetRed()<<16)|(col.GetGreen()<<8)|col.GetBlue();
			for (i=0, j=palSize; i<j;) {
				k=(i+j)/2;
				if (pal[k]<c) i=k+1; else j=k;
			}
			if (i>=palSize || S->Pal[i]!=c) {
				if (palSize>=S->MaxPalSize) {
					throw emException("Too many colors for XPM");
				}
				if (i<palSize) memmove(pal+i+1,pal+i,sizeof(emUInt32)*(palSize-i));
				pal[i]=c;
				palSize++;
			}
		}
		S->PalSize=palSize;
		return false;

	case 1:
		S->File.TryOpen(GetFilePath(),"wb");

		S->File.TryWrite("/* XPM */\n");

		{
			emString name=emGetNameInPath(GetFilePath());
			const char * e=emGetExtensionInPath(name);
			if (e) name=name.GetSubString(0,e-name);
			S->File.TryWrite(
				emString::Format("static char * %s_xpm[] = {\n",name.Get())
			);
		}

		S->PixSize=1;
		for (i=XpmSymCharsCount; i<S->PalSize; i*=XpmSymCharsCount) S->PixSize++;
		S->File.TryWrite(
			emString::Format("\"%d %d %d %d",width,height,S->PalSize,S->PixSize)
		);
		S->Stage++;
		return false;

	case 2:
		i=S->Index++;
		if (i>=S->PalSize) {
			S->Stage++;
			S->Index=0;
			return false;
		}
		S->File.TryWrite("\",\n\"");
		for (j=0, k=i; j<S->PixSize; j++) {
			S->File.TryWriteUInt8(XpmSymChars[k%XpmSymCharsCount]);
			k/=XpmSymCharsCount;
		}
		c=S->Pal[i];
		if (!c) {
			S->File.TryWrite(" c none");
		}
		else {
			S->File.TryWrite(emString::Format(
				" c #%02X%02X%02X",
				(c>>16)&255,
				(c>>8)&255,
				c&255
			));
		}
		return false;

	default:
		y=S->Index++;
		if (y>=height) {
			S->File.TryWrite("\"\n};\n");
			S->File.TryClose();
			return true;
		}
		S->File.TryWrite("\",\n\"");
		palSize=S->PalSize;
		pixSize=S->PixSize;
		pal=S->Pal;
		for (x=0; x<width; x++) {
			col=GetImage().GetPixel(x,y);
			if (col.GetAlpha()<128) c=0;
			else c=0x01000000|(col.GetRed()<<16)|(col.GetGreen()<<8)|col.GetBlue();
			for (i=0, j=palSize; i<j;) {
				k=(i+j)/2;
				if (pal[k]<c) i=k+1; else j=k;
			}
			for (j=0; j<pixSize; j++) {
				S->File.TryWriteUInt8(XpmSymChars[i%XpmSymCharsCount]);
				i/=XpmSymCharsCount;
			}
		}
		return false;
	}
}


void emXpmImageFileModel::QuitSaving()
{
	S.Reset();
}


emUInt64 emXpmImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return L->FileSize*(emUInt64)5;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emXpmImageFileModel::CalcFileProgress()
{
	double progress;

	progress=0.0;
	if (L) {
		if (L->Buffer) progress+=10.0;
		if (L->FileSize>0) progress+=70.0*L->BufferFill/L->FileSize;
		if (L->StringArray) progress+=10.0;
	}
	else if (S) {
		switch (S->Stage) {
		case 0:
			if (GetImage().GetHeight()>0) {
				progress+=40.0*S->Index/GetImage().GetHeight();
			}
			break;
		case 1:
			progress+=40.0;
			break;
		case 2:
			progress+=45.0;
			if (S->PalSize>0) {
				progress+=5.0*S->Index/S->PalSize;
			}
			break;
		default:
			progress+=50.0;
			if (GetImage().GetHeight()>0) {
				progress+=49.0*S->Index/GetImage().GetHeight();
			}
			break;
		}
	}
	return progress;
}


bool emXpmImageFileModel::FindCString(int startPos, int * pPos, int * pLen) const
{
	int i,pos,len;

	for (i=startPos;;i++) {
		if (i>=L->BufferFill) return false;
		if (L->Buffer[i]=='"') break;
		if (L->Buffer[i]=='/' && i+1<L->BufferFill && L->Buffer[i+1]=='*') {
			for (i+=3;;i++) {
				if (i>=L->BufferFill) return false;
				if (L->Buffer[i-1]=='*' && L->Buffer[i]=='/') break;
			}
		}
	}
	i++;
	pos=i;
	while (i<L->BufferFill && L->Buffer[i]!='"') i++;
	if (i>=L->BufferFill) return false;
	len=i-pos;
	*pPos=pos;
	*pLen=len;
	return true;
}


const char * const emXpmImageFileModel::XpmSymChars=
	// Remember to adapt XpmSymCharsCount when changing this.
	// Impossible: '\', '"' and '?' (last because of trigraphs)
	" 123456789ABCDEF"
	"GHIJKLMNOPQRSTUV"
	"WXYZ0abcdefghijk"
	"lmnopqrstuvwxyz!"
	"#$%&'()*+,-./:;<"
	"=>@[]^_`{|}~"
;
