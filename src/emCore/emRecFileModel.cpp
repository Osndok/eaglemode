//------------------------------------------------------------------------------
// emRecFileModel.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emCore/emRecFileModel.h>


emRecFileModel::emRecFileModel(emContext & context, const emString & name)
	: emFileModel(context,name), Link(*this)
{
	Reader=NULL;
	Writer=NULL;
	ProtectFileState=0;
	MemoryNeed=0;
	MemoryNeedOutOfDate=true;
	ReadStep=0;
	ReadStepOfMemCalc=0;
}


void emRecFileModel::PostConstruct(emRec & rec)
{
	Link.SetListenedRec(&rec);
}


emRecFileModel::~emRecFileModel()
{
	if (Reader) {
		delete Reader;
		Reader=NULL;
	}
	if (Writer) {
		delete Writer;
		Writer=NULL;
	}
}


void emRecFileModel::ResetData()
{
	ProtectFileState++;
	GetRec().SetToDefault();
	ProtectFileState--;
}


void emRecFileModel::TryStartLoading() throw(emString)
{
	ProtectFileState++;
	Reader=new emRecFileReader;
	try {
		Reader->TryStartReading(GetRec(),GetFilePath());
	}
	catch (emString errorMessage) {
		ProtectFileState--;
		throw errorMessage;
	}
	ProtectFileState--;
	ReadStep=1;
	ReadStepOfMemCalc=1;
}


bool emRecFileModel::TryContinueLoading() throw(emString)
{
	bool b;

	ProtectFileState++;
	try {
		b=Reader->TryContinueReading();
	}
	catch (emString errorMessage) {
		ProtectFileState--;
		throw errorMessage;
	}
	ProtectFileState--;

	if (b) {
		ReadStep=0;
		ReadStepOfMemCalc=0;
	}
	else {
		ReadStep++;
	}

	return b;
}


void emRecFileModel::QuitLoading()
{
	if (Reader) {
		ProtectFileState++;
		Reader->QuitReading();
		delete Reader;
		Reader=NULL;
		ProtectFileState--;
		ReadStep=0;
		ReadStepOfMemCalc=0;
	}
}


void emRecFileModel::TryStartSaving() throw(emString)
{
	ProtectFileState++;
	Writer=new emRecFileWriter;
	try {
		Writer->TryStartWriting(GetRec(),GetFilePath());
	}
	catch (emString errorMessage) {
		ProtectFileState--;
		throw errorMessage;
	}
	ProtectFileState--;
}


bool emRecFileModel::TryContinueSaving() throw(emString)
{
	bool b;

	ProtectFileState++;
	try {
		b=Writer->TryContinueWriting();
	}
	catch (emString errorMessage) {
		ProtectFileState--;
		throw errorMessage;
	}
	ProtectFileState--;
	return b;
}


void emRecFileModel::QuitSaving()
{
	if (Writer) {
		ProtectFileState++;
		Writer->QuitWriting();
		delete Writer;
		Writer=NULL;
		ProtectFileState--;
	}
}


emUInt64 emRecFileModel::CalcMemoryNeed()
{
	emUInt64 fileSize;

	// Problem: GetRec().CalcRecMemNeed() is too costly for calling
	// it after each step of reading. Therefore, when reading, we
	// call it on certain read steps only.
	if (Reader && ReadStep) {
		if (ReadStepOfMemCalc<=ReadStep) {
			MemoryNeed=GetRec().CalcRecMemNeed();
			fileSize=Reader->GetFileSize();
			if (fileSize>MemoryNeed) {
				MemoryNeed=fileSize;
				MemoryNeedOutOfDate=true; // Because it's an estimation.
			}
			else {
				MemoryNeedOutOfDate=false;
			}
			ReadStepOfMemCalc=ReadStep+(ReadStep+3)/4;
		}
	}
	else if (MemoryNeedOutOfDate) {
		MemoryNeed=GetRec().CalcRecMemNeed();
		MemoryNeedOutOfDate=false;
	}
	return MemoryNeed;
}


double emRecFileModel::CalcFileProgress()
{
	if (Reader) return Reader->GetProgress();
	else if (Writer) return 50.0;
	else return 100.0;
}


emRecFileModel::RecLink::RecLink(emRecFileModel & model)
	: Model(model)
{
}


void emRecFileModel::RecLink::OnRecChanged()
{
	Model.MemoryNeedOutOfDate=true;
	if (!Model.ProtectFileState) Model.SetUnsavedState();
	Model.Signal(Model.ChangeSignal);
}
