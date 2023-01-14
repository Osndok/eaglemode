//------------------------------------------------------------------------------
// emPngImageFileModel.cpp
//
// Copyright (C) 2004-2009,2011,2014,2018-2019,2022 Oliver Hamann.
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
#include <emPng/emPngDecode.h>


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

	L->file=fopen(GetFilePath(),"rb");
	if (!L->file) throw emException("%s",emGetErrorText(errno).Get());

	infoBuf[0]=0;
	errorBuf[0]=0;
	L->decodeInstance=emPngStartDecoding(
		L->file,&L->width,&L->height,&L->channelCount,&L->passCount,
		infoBuf,sizeof(infoBuf),errorBuf,sizeof(errorBuf)
	);
	if (!L->decodeInstance) throw emException("%s",errorBuf);

	FileFormatInfo=infoBuf;
	Signal(ChangeSignal);
}


bool emPngImageFileModel::TryContinueLoading()
{
	char commentBuf[1024];
	char errorBuf[256];
	int r;

	if (!L->imagePrepared) {
		Image.Setup(
			L->width,
			L->height,
			L->channelCount
		);
		Signal(ChangeSignal);
		L->imagePrepared=true;
		return false;
	}

	commentBuf[0]=0;
	errorBuf[0]=0;
	r=emPngContinueDecoding(
		L->decodeInstance,
		Image.GetWritableMap()+L->y*(size_t)Image.GetWidth()*Image.GetChannelCount(),
		commentBuf,sizeof(commentBuf),errorBuf,sizeof(errorBuf)
	);
	if (r<0) throw emException("%s",errorBuf);

	L->y++;
	if (L->y>=L->height) {
		L->pass++;
		L->y=0;
	}

	Comment+=commentBuf;

	Signal(ChangeSignal);

	return r!=0;
}


void emPngImageFileModel::QuitLoading()
{
	if (L) {
		if (L->decodeInstance) emPngQuitDecoding(L->decodeInstance);
		if (L->file) fclose(L->file);
		delete L;
		L=NULL;
	}
}


void emPngImageFileModel::TryStartSaving()
{
	throw emException("emPngImageFileModel: Saving not implemented.");
}


bool emPngImageFileModel::TryContinueSaving()
{
	return true;
}


void emPngImageFileModel::QuitSaving()
{
}


emUInt64 emPngImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return ((emUInt64)L->width)*
		       L->height*
		       L->channelCount;
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emPngImageFileModel::CalcFileProgress()
{
	if (L && L->height>0 && L->passCount>0) {
		return 100.0*(L->pass*L->height+L->y)/(L->passCount*L->height);
	}
	else {
		return 0.0;
	}
}
