//------------------------------------------------------------------------------
// emXpmImageFileModel.cpp
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
	L=NULL;
}


emXpmImageFileModel::~emXpmImageFileModel()
{
	emXpmImageFileModel::QuitLoading();
	emXpmImageFileModel::QuitSaving();
}


void emXpmImageFileModel::TryStartLoading() throw(emString)
{
	emInt64 l;

	L=new LoadingState;
	memset(L,0,sizeof(LoadingState));
	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;
	if (fseek(L->File,0,SEEK_END)) goto Err;
	l=ftell(L->File);
	if (l<0) goto Err;
	if (l>INT_MAX) throw emString("File too large.");
	L->FileSize=(int)l;
	if (L->FileSize<0) goto Err;
	if (fseek(L->File,0,SEEK_SET)) goto Err;
	return;
Err:
	throw emGetErrorText(errno);
}


bool emXpmImageFileModel::TryContinueLoading() throw(emString)
{
	int i,pos,len;

	if (!L->Buffer) {
		L->Buffer=new char[L->FileSize];
	}
	else if (L->File) {
		len=L->FileSize-L->BufferFill;
		if (len>4096) len=4096;
		if (len>0) len=fread(L->Buffer+L->BufferFill,1,len,L->File);
		if (len>0) L->BufferFill+=len;
		else {
			fclose(L->File);
			L->File=NULL;
		}
	}
	else if (!L->StringArray) {
		if (L->BufferFill<9 || memcmp(L->Buffer,"/* XPM */",9)!=0) {
			throw emString("Not an XPM file.");
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
		Image.TryParseXpm(L->StringArray);
		FileFormatInfo="XPM";
		Signal(ChangeSignal);
		return true;
	}
	return false;
}


void emXpmImageFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		if (L->Buffer) delete [] L->Buffer;
		if (L->StringArray) delete [] L->StringArray;
		delete L;
		L=NULL;
	}
}


void emXpmImageFileModel::TryStartSaving() throw(emString)
{
	throw emString("emXpmImageFileModel: Saving not implemented.");
}


bool emXpmImageFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emXpmImageFileModel::QuitSaving()
{
}


emUInt64 emXpmImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return L->FileSize*5;
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
	return progress;
}


bool emXpmImageFileModel::FindCString(int startPos, int * pPos, int * pLen)
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
