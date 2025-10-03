//------------------------------------------------------------------------------
// emPdfFileModel.cpp
//
// Copyright (C) 2011,2014,2018,2023-2024 Oliver Hamann.
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


emPdfServerModel::PdfInstance & emPdfFileModel::GetPdfInstance() const
{
	if (!PdfInstance) emFatalError("PdfInstance is NULL");
	return *PdfInstance;
}


emPdfFileModel::emPdfFileModel(emContext & context, const emString & name)
	: emFileModel(context,name),
	PageAreasMap(GetScheduler())
{
	ServerModel=emPdfServerModel::Acquire(GetRootContext());
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
	PageAreasMap.Reset();
	if (PdfInstance) {
		PdfInstance=NULL;
		Signal(ChangeSignal);
	}
	FileSize=0;
	StartTime=0;
	PageCount=0;
}


void emPdfFileModel::TryStartLoading()
{
	FileSize=emTryGetFileSize(GetFilePath());
}


bool emPdfFileModel::TryContinueLoading()
{
	if (!OpenJob) {
		OpenJob=new emPdfServerModel::OpenJob(GetFilePath());
		ServerModel->EnqueueJob(*OpenJob);
		StartTime=emGetClockMS();
		return false;
	}
	ServerModel->Poll(10);
	switch (OpenJob->GetState()) {
	case emJob::ST_ERROR:
		throw emException("%s",OpenJob->GetErrorText().Get());
	case emJob::ST_ABORTED:
		throw emException("Aborted");
	case emJob::ST_SUCCESS:
		PdfInstance=OpenJob->GetPdfInstance();
		PageCount=PdfInstance->GetPageCount();
		PageAreasMap.Setup(*ServerModel,*PdfInstance);
		Signal(ChangeSignal);
		return true;
	default:
		break;
	}
	return false;
}


void emPdfFileModel::QuitLoading()
{
	if (OpenJob) {
		if (
			OpenJob->GetState()==emJob::ST_WAITING ||
			OpenJob->GetState()==emJob::ST_RUNNING
		) ServerModel->AbortJob(*OpenJob);
		OpenJob=NULL;
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
	if (OpenJob) {
		switch (OpenJob->GetState()) {
		case emJob::ST_SUCCESS:
			return 100.0;
		case emJob::ST_WAITING:
		case emJob::ST_ERROR:
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
