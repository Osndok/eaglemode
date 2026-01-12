//------------------------------------------------------------------------------
// emGifImageFileModel.cpp
//
// Copyright (C) 2025 Oliver Hamann.
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

#include <emGif/emGifImageFileModel.h>


emRef<emGifImageFileModel> emGifImageFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emGifImageFileModel,context,name,common)
}


emGifImageFileModel::emGifImageFileModel(
	emContext & context, const emString & name
)
	: emImageFileModel(context,name)
{
}


emGifImageFileModel::~emGifImageFileModel()
{
}


void emGifImageFileModel::TryStartLoading()
{
	L=new LoadingState;
	L->GifFileModel=emGifFileModel::Acquire(GetContext(),GetFilePath());
	L->FmClient.SetModel(L->GifFileModel);
	L->GifFileModel->Update();
}


bool emGifImageFileModel::TryContinueLoading()
{
	switch (L->GifFileModel->GetFileState()) {
		case FS_WAITING:
		case FS_LOADING:
			L->GifFileModel->Load(false);
			return false;
		case FS_UNSAVED:
		case FS_LOADED:
		case FS_SAVE_ERROR:
			Image=L->GifFileModel->RenderAll();
			FileFormatInfo="GIF";
			Comment=L->GifFileModel->GetComment();
			Signal(ChangeSignal);
			return true;
		case FS_TOO_COSTLY:
			throw emException("Internal error: too costly");
		default:
			throw emException("%s",L->GifFileModel->GetErrorText().Get());
	}
}


void emGifImageFileModel::QuitLoading()
{
	L.Reset();
}


void emGifImageFileModel::TryStartSaving()
{
	throw emException("emGifImageFileModel: Saving not implemented.");
}


bool emGifImageFileModel::TryContinueSaving()
{
	return true;
}


void emGifImageFileModel::QuitSaving()
{
}


emUInt64 emGifImageFileModel::CalcMemoryNeed()
{
	if (L) {
		return L->GifFileModel->GetMemoryNeed();
	}
	else {
		return ((emUInt64)Image.GetWidth())*
		       Image.GetHeight()*
		       Image.GetChannelCount();
	}
}


double emGifImageFileModel::CalcFileProgress()
{
	if (L) {
		return L->GifFileModel->GetFileProgress();
	}
	else {
		return 0.0;
	}
}
