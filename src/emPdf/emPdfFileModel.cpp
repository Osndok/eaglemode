//------------------------------------------------------------------------------
// emPdfFileModel.cpp
//
// Copyright (C) 2011,2014,2018,2023 Oliver Hamann.
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

#include <emPdf/emPdfFileModel.h>


emRef<emPdfFileModel> emPdfFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emPdfFileModel,context,name,common)
}


emPdfFileModel::emPdfFileModel(emContext & context, const emString & name)
	: emFileModel(context,name),
	PageAreasMap(GetScheduler())
{
	ServerModel=emPdfServerModel::Acquire(GetRootContext());
	JobHandle=NULL;
	PdfHandle=NULL;
	FileSize=0;
	StartTime=0;
	PageCount=0;
}


emPdfFileModel::~emPdfFileModel()
{
	emPdfFileModel::QuitLoading();
	emPdfFileModel::QuitSaving();
	emPdfFileModel::ResetData();
}


void emPdfFileModel::ResetData()
{
	if (PdfHandle) {
		ServerModel->ClosePdf(PdfHandle);
		PdfHandle=NULL;
		Signal(ChangeSignal);
	}
	FileSize=0;
	StartTime=0;
	PageCount=0;
	PageAreasMap.Reset();
}


void emPdfFileModel::TryStartLoading()
{
	FileSize=emTryGetFileSize(GetFilePath());
}


bool emPdfFileModel::TryContinueLoading()
{
	if (!JobHandle) {
		JobHandle=ServerModel->StartOpenJob(GetFilePath(),&PdfHandle);
		StartTime=emGetClockMS();
		return false;
	}
	ServerModel->Poll(10);
	switch (ServerModel->GetJobState(JobHandle)) {
	case emPdfServerModel::JS_ERROR:
		throw emException("%s",ServerModel->GetJobErrorText(JobHandle).Get());
	case emPdfServerModel::JS_SUCCESS:
		PageCount=ServerModel->GetPageCount(PdfHandle);
		PageAreasMap.Setup(*ServerModel,PdfHandle);
		Signal(ChangeSignal);
		return true;
	default:
		break;
	}
	return false;
}


void emPdfFileModel::QuitLoading()
{
	if (JobHandle) {
		ServerModel->CloseJob(JobHandle);
		JobHandle=NULL;
	}
}


void emPdfFileModel::TryStartSaving()
{
	throw emException("emPdfFileModel: Saving not supported.");
}


bool emPdfFileModel::TryContinueSaving()
{
	return true;
}


void emPdfFileModel::QuitSaving()
{
}


emUInt64 emPdfFileModel::CalcMemoryNeed()
{
	return 14000000+3*FileSize;
}


double emPdfFileModel::CalcFileProgress()
{
	emUInt64 t;

	t=emGetClockMS();
	if (JobHandle) {
		switch (ServerModel->GetJobState(JobHandle)) {
		case emPdfServerModel::JS_SUCCESS:
			return 100.0;
		case emPdfServerModel::JS_WAITING:
		case emPdfServerModel::JS_ERROR:
			StartTime=t;
			break;
		default:
			break;
		}
	}
	else {
		StartTime=t;
	}
	return 100.0*(1.0-1.0/(1.0+sqrt((t-StartTime)*5000.0/FileSize)));
}
