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
	PdfInstance(NULL)
{
}


emPdfPageAreasMap::~emPdfPageAreasMap()
{
	Reset();
}


void emPdfPageAreasMap::Setup(
	emPdfServerModel & serverModel, emPdfServerModel::PdfInstance & pdfInstance
)
{
	Reset();
	ServerModel=&serverModel;
	PdfInstance=&pdfInstance;
	Entries.SetCount(pdfInstance.GetPageCount());
}


void emPdfPageAreasMap::Reset()
{
	int i;

	if (ServerModel) {
		for (i=Entries.GetCount()-1; i>=0; i--) {
			if (Entries[i].Job) {
				ServerModel->AbortJob(*Entries[i].Job);
			}
		}
	}
	ServerModel=NULL;
	PdfInstance=NULL;
	Entries.Clear();
}


bool emPdfPageAreasMap::RequestPageAreas(int page, double priority)
{
	Entry * e;
	int i;

	if (!ServerModel || !PdfInstance) return false;
	if (page<0 || page>=Entries.GetCount()) return false;

	if (Entries[page].Requested) return true;

	for (i=Entries.GetCount()-1; i>=0; i--) {
		if (!Entries[i].Job) continue;
		e=&Entries.GetWritable(i);
		if (e->Job->GetState() == emJob::ST_WAITING) {
			ServerModel->AbortJob(*e->Job);
			e->Requested=false;
			e->Job=NULL;
		}
	}

	e=&Entries.GetWritable(page);
	if (e->Job) return true;
	e->Job=new emPdfServerModel::GetAreasJob(
		*PdfInstance,page,priority
	);
	ServerModel->EnqueueJob(*e->Job);
	AddWakeUpSignal(e->Job->GetStateSignal());
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
		if (e->Requested && !e->Job) {
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
		if (e->Requested && !e->Job && !e->ErrorText.IsEmpty()) {
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
		if (!Entries[i].Job) continue;
		e=&Entries.GetWritable(i);
		switch (e->Job->GetState()) {
		case emJob::ST_ERROR:
			e->ErrorText=e->Job->GetErrorText();
			e->Job=NULL;
			Signal(PageAreasSignal);
			break;
		case emJob::ST_ABORTED:
			e->ErrorText="Aborted";
			e->Job=NULL;
			Signal(PageAreasSignal);
			break;
		case emJob::ST_SUCCESS:
			e->Areas=e->Job->GetAreas();
			e->Job=NULL;
			Signal(PageAreasSignal);
			break;
		default:
			break;
		}
	}

	return false;
}


emPdfPageAreasMap::Entry::Entry()
	: Requested(false)
{
}


emPdfPageAreasMap::Entry::Entry(const Entry & entry)
	: Requested(entry.Requested),
	Job(entry.Job),
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
	Job=entry.Job;
	Areas=entry.Areas;
	ErrorText=entry.ErrorText;
	return *this;
}
