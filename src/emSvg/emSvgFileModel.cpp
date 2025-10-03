//------------------------------------------------------------------------------
// emSvgFileModel.cpp
//
// Copyright (C) 2010,2014,2018,2024 Oliver Hamann.
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

#include <emSvg/emSvgFileModel.h>


emRef<emSvgFileModel> emSvgFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emSvgFileModel,context,name,common)
}


emSvgServerModel::SvgInstance & emSvgFileModel::GetSvgInstance() const
{
	if (!SvgInstance) emFatalError("SvgInstance is NULL");
	return *SvgInstance;
}


emSvgFileModel::emSvgFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	ServerModel=emSvgServerModel::Acquire(GetRootContext());
	FileSize=0;
	Width=0.0;
	Height=0.0;
}


emSvgFileModel::~emSvgFileModel()
{
	emSvgFileModel::QuitLoading();
	emSvgFileModel::QuitSaving();
	emSvgFileModel::ResetData();
}


void emSvgFileModel::ResetData()
{
	SvgInstance=NULL;
	FileSize=0;
	Width=0.0;
	Height=0.0;
	Title.Clear();
	Description.Clear();
}


void emSvgFileModel::TryStartLoading()
{
	FileSize=emTryGetFileSize(GetFilePath());
}


bool emSvgFileModel::TryContinueLoading()
{
	if (!OpenJob) {
		OpenJob=new emSvgServerModel::OpenJob(GetFilePath(),-1E200);
		ServerModel->EnqueueJob(*OpenJob);
		return false;
	}
	ServerModel->Poll(10);
	switch (OpenJob->GetState()) {
	case emJob::ST_ERROR:
		throw emException("%s",OpenJob->GetErrorText().Get());
	case emJob::ST_ABORTED:
		throw emException("Aborted");
	case emJob::ST_SUCCESS:
		SvgInstance=OpenJob->GetSvgInstance();
		Width=SvgInstance->GetWidth();
		Height=SvgInstance->GetHeight();
		Title=SvgInstance->GetTitle();
		Description=SvgInstance->GetDescription();
		return true;
	default:
		break;
	}
	return false;
}


void emSvgFileModel::QuitLoading()
{
	if (OpenJob) {
		if (
			OpenJob->GetState()==emJob::ST_WAITING ||
			OpenJob->GetState()==emJob::ST_RUNNING
		) ServerModel->AbortJob(*OpenJob);
		OpenJob=NULL;
	}
}


void emSvgFileModel::TryStartSaving()
{
	throw emException("emSvgFileModel: Saving not supported.");
}


bool emSvgFileModel::TryContinueSaving()
{
	return true;
}


void emSvgFileModel::QuitSaving()
{
}


emUInt64 emSvgFileModel::CalcMemoryNeed()
{
	return 3000000+10*FileSize;
}


double emSvgFileModel::CalcFileProgress()
{
	if (SvgInstance) return 99.0;
	if (OpenJob && OpenJob->GetState()!=emJob::ST_WAITING) return 50.0;
	return 0.0;
}
