//------------------------------------------------------------------------------
// emXbmImageFileModel.cpp
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

#include <emXbm/emXbmImageFileModel.h>


emRef<emXbmImageFileModel> emXbmImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emXbmImageFileModel,context,name,common)
}


emXbmImageFileModel::emXbmImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
	L=NULL;
}


emXbmImageFileModel::~emXbmImageFileModel()
{
	emXbmImageFileModel::QuitLoading();
	emXbmImageFileModel::QuitSaving();
}


void emXbmImageFileModel::TryStartLoading() throw(emString)
{
	emInt64 l;

	L=new LoadingState;
	L->File=NULL;
	L->Str=NULL;
	L->StrMaxLen=0;
	L->StrLen=0;
	L->Width=0;
	L->Height=0;
	L->MapPtr=NULL;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;
	if (fseek(L->File,0,SEEK_END)) goto Err;
	l=ftell(L->File);
	if (l<0) goto Err;
	if (l>INT_MAX) throw emString("File too large.");
	L->StrMaxLen=(int)l;
	if (fseek(L->File,0,SEEK_SET)) goto Err;

	return;

Err:
	throw emGetErrorText(errno);
}


bool emXbmImageFileModel::TryContinueLoading() throw(emString)
{
	char * p, * p2;
	unsigned char * map;
	int i,x,y,m,c,len;

	if (L->File) {
		if (!L->Str) L->Str=(char*)malloc(L->StrMaxLen+1);
		len=L->StrMaxLen-L->StrLen;
		if (len>4096) len=4096;
		if (len>0) len=fread(L->Str+L->StrLen,1,len,L->File);
		if (len>0) L->StrLen+=len;
		else {
			fclose(L->File);
			L->File=NULL;
		}
		return false;
	}
	else if (!L->Width) {
		L->Str[L->StrLen]=0;
		for (p=L->Str;;) {
			p=strstr(p,"/*");
			if (!p) break;
			p2=strstr(p+2,"*/");
			len=(p2 ? p2+2 : L->Str+L->StrLen)-p;
			memset(p,0x20,len);
			p+=len;
		}
		p=strstr(L->Str,"width");
		if (!p) goto ErrFormat;
		L->Width=(int)strtol(p+5,&p,0);
		if (L->Width<1 || L->Width>65535) goto ErrFormat;
		p=strstr(L->Str,"height");
		if (!p) goto ErrFormat;
		L->Height=(int)strtol(p+6,&p,0);
		if (L->Height<1 || L->Height>65535) goto ErrFormat;
		p=strstr(L->Str,"static");
		if (!p) goto ErrFormat;
		p=strstr(p,"char");
		if (!p) goto ErrFormat;
		p=strstr(p,"{");
		if (!p) goto ErrFormat;
		L->MapPtr=p;
		return false;
	}
	else {
		p=L->MapPtr;
		FileFormatInfo="XBM";
		Image.Setup(L->Width,L->Height,1);
		map=Image.GetWritableMap();
		for (y=0; y<L->Height; y++) {
			for (x=0, m=0; x<L->Width; x++) {
				if ((x&7)==0) {
					p=strstr(p,"0x");
					if (!p) goto ErrFormat;
					p+=2;
					m=0;
					for (i=0;;i++) {
						c=*p++;
						if (c>='0' && c<='9') m=(m<<4)|(c-'0');
						else if (c>='a' && c<='f') m=(m<<4)|(c-'a'+10);
						else if (c>='A' && c<='F') m=(m<<4)|(c-'A'+10);
						else if (i==0) goto ErrFormat;
						else break;
					}
				}
				if (m&1) map[0]=0;
				else map[0]=255;
				map++;
				m>>=1;
			}
		}
		Signal(ChangeSignal);
		return true;
	}

ErrFormat:
	throw emString("XBM format error");
}


void emXbmImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Str) free(L->Str);
		delete L;
		L=NULL;
	}
}


void emXbmImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emXbmImageFileModel: Saving not implemented.");
}


bool emXbmImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emXbmImageFileModel::QuitSaving()
{
}


emUInt64 emXbmImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return L->StrMaxLen+((emUInt64)L->Width)*L->Height;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emXbmImageFileModel::CalcFileProgress()
{
	if (L && L->StrMaxLen>0) {
		return 100.0*L->StrLen/L->StrMaxLen;
	}
	else {
		return 0.0;
	}
}
