//------------------------------------------------------------------------------
// emSvgFileModel.cpp
//
// Copyright (C) 2010 Oliver Hamann.
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


emSvgFileModel::emSvgFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	ServerModel=emSvgServerModel::Acquire(GetRootContext());
	JobHandle=NULL;
	SvgHandle=NULL;
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
	if (SvgHandle) {
		ServerModel->CloseSvg(SvgHandle);
		SvgHandle=NULL;
	}
	FileSize=0;
	Width=0.0;
	Height=0.0;
	Title.Empty();
	Description.Empty();
}


void emSvgFileModel::TryStartLoading() throw(emString)
{
	FileSize=emTryGetFileSize(GetFilePath());
}


bool emSvgFileModel::TryContinueLoading() throw(emString)
{
	if (!JobHandle) {
		JobHandle=ServerModel->StartOpenJob(GetFilePath(),&SvgHandle);
		return false;
	}
	ServerModel->Poll(10);
	switch (ServerModel->GetJobState(JobHandle)) {
	case emSvgServerModel::JS_ERROR:
		throw emString(ServerModel->GetJobErrorText(JobHandle));
	case emSvgServerModel::JS_SUCCESS:
		Width=ServerModel->GetSvgWidth(SvgHandle);
		Height=ServerModel->GetSvgHeight(SvgHandle);
		Title=ServerModel->GetSvgTitle(SvgHandle);
		Description=ServerModel->GetSvgDescription(SvgHandle);
		return true;
	default:
		break;
	}
	return false;
}


void emSvgFileModel::QuitLoading()
{
	if (JobHandle) {
		ServerModel->CloseJob(JobHandle);
		JobHandle=NULL;
	}
}


void emSvgFileModel::TryStartSaving() throw(emString)
{
	throw emString("emSvgFileModel: Saving not supported.");
}


bool emSvgFileModel::TryContinueSaving() throw(emString)
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
	if (
		JobHandle &&
		ServerModel->GetJobState(JobHandle)!=emSvgServerModel::JS_WAITING
	) return 50.0;
	return 0.0;
}
