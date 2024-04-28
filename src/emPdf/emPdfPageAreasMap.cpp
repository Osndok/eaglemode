//------------------------------------------------------------------------------
// emPdfPageAreasMap.cpp
//
// Copyright (C) 2023-2024 Oliver Hamann.
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

#include <emPdf/emPdfPageAreasMap.h>


emPdfPageAreasMap::emPdfPageAreasMap(emScheduler & scheduler)
	: emEngine(scheduler),
	ServerModel(NULL),
	PdfHandle(NULL)
{
}


emPdfPageAreasMap::~emPdfPageAreasMap()
{
	Reset();
}


void emPdfPageAreasMap::Setup(
	emPdfServerModel & serverModel, emPdfServerModel::PdfHandle pdfHandle
)
{
	Reset();
	ServerModel=&serverModel;
	PdfHandle=pdfHandle;
	Entries.SetCount(ServerModel->GetPageCount(pdfHandle));
}


void emPdfPageAreasMap::Reset()
{
	int i;

	if (ServerModel && PdfHandle) {
		for (i=Entries.GetCount()-1; i>=0; i--) {
			if (Entries[i].JobHandle) {
				ServerModel->CloseJob(Entries[i].JobHandle);
			}
		}
	}
	ServerModel=NULL;
	PdfHandle=NULL;
	Entries.Clear();
}


bool emPdfPageAreasMap::RequestPageAreas(int page, double priority)
{
	Entry * e;
	int i;

	if (!ServerModel || !PdfHandle) return false;
	if (page<0 || page>=Entries.GetCount()) return false;

	if (Entries[page].Requested) return true;

	for (i=Entries.GetCount()-1; i>=0; i--) {
		if (!Entries[i].JobHandle) continue;
		e=&Entries.GetWritable(i);
		if (ServerModel->GetJobState(e->JobHandle) == emPdfServerModel::JS_WAITING) {
			ServerModel->CloseJob(e->JobHandle);
			e->Requested=false;
			e->JobHandle=NULL;
		}
	}

	e=&Entries.GetWritable(page);
	if (e->JobHandle) return true;
	e->JobHandle=ServerModel->StartGetAreasJob(
		PdfHandle,page,&e->Areas,priority,this
	);
	e->Requested=true;
	return true;
}


const emPdfServerModel::PageAreas * emPdfPageAreasMap::GetPageAreas(
	int page
) const
{
	const Entry * e;

	if (page>=0 && page<Entries.GetCount()) {
		e=&Entries[page];
		if (e->Requested && !e->JobHandle) {
			return &e->Areas;
		}
	}
	return NULL;
}


const emString * emPdfPageAreasMap::GetError(int page) const
{
	const Entry * e;

	if (page>=0 && page<Entries.GetCount()) {
		e=&Entries[page];
		if (e->Requested && !e->JobHandle && !e->ErrorText.IsEmpty()) {
			return &e->ErrorText;
		}
	}
	return NULL;
}


bool emPdfPageAreasMap::Cycle()
{
	Entry * e;
	int i;

	for (i=Entries.GetCount()-1; i>=0; i--) {
		if (!Entries[i].JobHandle) continue;
		e=&Entries.GetWritable(i);
		switch (ServerModel->GetJobState(e->JobHandle)) {
		case emPdfServerModel::JS_ERROR:
			e->ErrorText=ServerModel->GetJobErrorText(e->JobHandle);
			ServerModel->CloseJob(e->JobHandle);
			e->JobHandle=NULL;
			Signal(PageAreasSignal);
			break;
		case emPdfServerModel::JS_SUCCESS:
			ServerModel->CloseJob(e->JobHandle);
			e->JobHandle=NULL;
			Signal(PageAreasSignal);
			break;
		default:
			break;
		}
	}

	return false;
}


emPdfPageAreasMap::Entry::Entry()
	: Requested(false),
	JobHandle(NULL)
{
}


emPdfPageAreasMap::Entry::Entry(const Entry & entry)
	: Requested(entry.Requested),
	JobHandle(entry.JobHandle),
	Areas(entry.Areas),
	ErrorText(entry.ErrorText)
{
}


emPdfPageAreasMap::Entry::~Entry()
{
}


emPdfPageAreasMap::Entry & emPdfPageAreasMap::Entry::operator = (
	const Entry & entry
)
{
	Requested=entry.Requested;
	JobHandle=entry.JobHandle;
	Areas=entry.Areas;
	ErrorText=entry.ErrorText;
	return *this;
}
