//------------------------------------------------------------------------------
// emPsFileModel.cpp
//
// Copyright (C) 2006-2009 Oliver Hamann.
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

#include <emPs/emPsFileModel.h>


emRef<emPsFileModel> emPsFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPsFileModel,context,name,common)
}


emPsFileModel::emPsFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	L=NULL;
}


emPsFileModel::~emPsFileModel()
{
	emPsFileModel::QuitLoading();
	emPsFileModel::QuitSaving();
	emPsFileModel::ResetData();
}


void emPsFileModel::ResetData()
{
	Document.Empty();
}


void emPsFileModel::TryStartLoading() throw(emString)
{
	emInt64 l;

	L=new LoadingState;
	L->File=NULL;
	L->FileSize=0;
	L->FilePos=0;
	L->Buffer.SetTuningLevel(4);

	if (
		(L->File=fopen(GetFilePath(),"rb"))==NULL ||
		fseek(L->File,0,SEEK_END)!=0 ||
		(l=ftell(L->File))<0 ||
		fseek(L->File,0,SEEK_SET)!=0
	) throw emGetErrorText(errno);

	if (l>INT_MAX) throw emString("File too large.");

	L->FileSize=(int)l;
}


bool emPsFileModel::TryContinueLoading() throw(emString)
{
	int len;

	if (L->FilePos==0) L->Buffer.SetCount(L->FileSize,true);
	len=L->FileSize-L->FilePos;
	if (len>4096) len=4096;
	if (len>0) {
		len=fread(L->Buffer.GetWritable()+L->FilePos,1,len,L->File);
		if (ferror(L->File)) throw emGetErrorText(errno);
		if (len>0) L->FilePos+=len;
		if (!feof(L->File)) return false;
	}
	L->FileSize=L->FilePos;
	L->Buffer.SetCount(L->FileSize,true);
	Document.TrySetScript(L->Buffer);
	return true;
}


void emPsFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		delete L;
		L=NULL;
	}
}


void emPsFileModel::TryStartSaving() throw(emString)
{
	throw emString("emPsFileModel: Saving not implemented.");
}


bool emPsFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emPsFileModel::QuitSaving()
{
}


emUInt64 emPsFileModel::CalcMemoryNeed()
{
	emUInt64 m;

	if (L) {
		m=L->FileSize;
	}
	else {
		m=Document.CalcMemoryNeed();
	}

	m*=2; // For the memory effort of emPsPagePanel
	m+=10000000; // For the memory effort of emPsRenderer

	return m;
}


double emPsFileModel::CalcFileProgress()
{
	if (L && L->FileSize>0) return ((double)L->FilePos)*100.0/L->FileSize;
	else return 0.0;
}
