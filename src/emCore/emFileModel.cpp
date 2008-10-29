//------------------------------------------------------------------------------
// emFileModel.cpp
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

#include <emCore/emFileModel.h>


//==============================================================================
//================================ emFileModel =================================
//==============================================================================

const emString & emFileModel::GetFilePath() const
{
	return GetName();
}


void emFileModel::Update()
{
	switch (State) {
		case FS_WAITING:
			if (MemoryNeed>1) {
				MemoryNeed=1;
				Signal(FileStateSignal);
			}
			break;
		case FS_LOADED:
			if (IsOutOfDate()) {
				ResetData();
				State=FS_TOO_COSTLY;
				MemoryNeed=1;
				FileProgress=0.0;
				if (ClientList) {
					State=FS_WAITING;
					StartPSAgent();
				}
				Signal(FileStateSignal);
			}
			break;
		case FS_TOO_COSTLY:
			if (MemoryNeed>1) {
				MemoryNeed=1;
				if (ClientList) {
					State=FS_WAITING;
					StartPSAgent();
				}
				Signal(FileStateSignal);
			}
			break;
		case FS_LOAD_ERROR:
			State=FS_TOO_COSTLY;
			ErrorText.Empty();
			MemoryNeed=1;
			if (ClientList) {
				State=FS_WAITING;
				StartPSAgent();
			}
			Signal(FileStateSignal);
			break;
		default:
			break;
	}
}


emRef<emSigModel> emFileModel::AcquireUpdateSignalModel(
	emRootContext & rootContext
)
{
	return emSigModel::Acquire(rootContext,"emFileModel::UpdateSignal");
}


void emFileModel::SetIgnoreUpdateSignal(bool ignore)
{
	if (ignore) {
		if (UpdateSignalModel) {
			RemoveWakeUpSignal(UpdateSignalModel->Sig);
			UpdateSignalModel=NULL;
		}
	}
	else {
		if (!UpdateSignalModel) {
			UpdateSignalModel=AcquireUpdateSignalModel(GetRootContext());
			AddWakeUpSignal(UpdateSignalModel->Sig);
		}
	}
}


void emFileModel::Load(bool immediately)
{
	bool stateChanged;

	if (State==FS_WAITING || State==FS_LOADING) {
		stateChanged=StepLoading();
		if (immediately) {
			while (State==FS_LOADING) {
				if (StepLoading()) stateChanged=true;
			}
		}
		if (UpdateFileProgress()) stateChanged=true;
		if (stateChanged) Signal(FileStateSignal);
		if (State==FS_LOADING) WakeUp();
	}
}


void emFileModel::Save(bool immediately)
{
	bool stateChanged;

	if (State==FS_SAVING || State==FS_UNSAVED) {
		stateChanged=StepSaving();
		if (immediately) {
			while (State==FS_SAVING) {
				if (StepSaving()) stateChanged=true;
			}
		}
		if (UpdateFileProgress()) stateChanged=true;
		if (stateChanged) Signal(FileStateSignal);
		if (State==FS_SAVING) WakeUp();
	}
}


void emFileModel::ClearSaveError()
{
	if (State==FS_SAVE_ERROR) {
		State=FS_UNSAVED;
		ErrorText.Empty();
		Signal(FileStateSignal);
	}
}


void emFileModel::HardResetFileState()
{
	EndPSAgent();
	switch (State) {
		case FS_LOADING:
			QuitLoading();
			ResetData();
			break;
		case FS_SAVING:
			QuitSaving();
			ResetData();
			break;
		case FS_LOADED:
		case FS_UNSAVED:
		case FS_SAVE_ERROR:
			ResetData();
			break;
		default:
			break;
	}
	State=FS_TOO_COSTLY;
	MemoryNeed=1;
	FileProgress=0.0;
	ErrorText.Empty();
	if (ClientList && MemoryLimit>=MemoryNeed) {
		State=FS_WAITING;
		StartPSAgent();
	}
	Signal(FileStateSignal);
}


emFileModel::emFileModel(
	emContext & context, const emString & name
)
	: emModel(context,name)
{
	State=FS_TOO_COSTLY;
	MemoryNeed=1;
	FileProgress=0.0;
	FileProgressClock=0;
	ClientList=NULL;
	MemoryLimit=0;
	FileTime=0;
	PSAgent=NULL;
	UpdateSignalModel=NULL;
	SetIgnoreUpdateSignal(false);
}


emFileModel::~emFileModel()
{
	EndPSAgent();
}


bool emFileModel::Cycle()
{
	bool stateChanged;

	if (UpdateSignalModel && IsSignaled(UpdateSignalModel->Sig)) {
		Update();
	}

	switch (State) {
		case FS_WAITING:
			if (!PSAgent) StartPSAgent();
			if (!PSAgent->HasAccess()) return false;
			if (IsTimeSliceAtEnd()) return true;
			// no break
		case FS_LOADING:
			stateChanged=false;
			do {
				if (StepLoading()) stateChanged=true;
			} while (State==FS_LOADING && !IsTimeSliceAtEnd());
			if (UpdateFileProgress()) stateChanged=true;
			if (stateChanged) Signal(FileStateSignal);
			return State==FS_LOADING;
		case FS_SAVING:
			stateChanged=false;
			do {
				if (StepSaving()) stateChanged=true;
			} while (State==FS_SAVING && !IsTimeSliceAtEnd());
			if (UpdateFileProgress()) stateChanged=true;
			if (stateChanged) Signal(FileStateSignal);
			return State==FS_SAVING;
		default:
			return false;
	}
}


void emFileModel::SetUnsavedState()
{
	if (State!=FS_UNSAVED) {
		EndPSAgent();
		switch (State) {
			case FS_LOADING:
				QuitLoading();
				break;
			case FS_SAVING:
				QuitSaving();
				break;
			default:
				break;
		}
		State=FS_UNSAVED;
		FileProgress=0.0;
		ErrorText.Empty();
		Signal(FileStateSignal);
	}
}


bool emFileModel::IsOutOfDate()
{
	try {
		if (FileTime==emTryGetFileTime(GetFilePath())) return false;
	}
	catch (emString) {
		return true;
	}
	return true;
}


void emFileModel::ClientsChanged()
{
	emFileModelClient * c;
	emUInt64 m;
	double pri;

	for (m=0, c=ClientList; c; c=c->NextInList) {
		if (m<c->MemoryLimit) m=c->MemoryLimit;
	}
	MemoryLimit=m;

	if (PSAgent) {
		c=ClientList;
		if (c) {
			pri=c->Priority;
			for (c=c->NextInList; c; c=c->NextInList) {
				if (pri<c->Priority) pri=c->Priority;
			}
			PSAgent->SetAccessPriority(pri);
		}
	}

	switch (State) {
		case FS_WAITING:
			if (!ClientList || MemoryLimit<MemoryNeed) {
				EndPSAgent();
				State=FS_TOO_COSTLY;
				Signal(FileStateSignal);
			}
			break;
		case FS_LOADING:
			if (!ClientList || MemoryLimit<MemoryNeed) {
				EndPSAgent();
				QuitLoading();
				ResetData();
				State=FS_TOO_COSTLY;
				FileProgress=0.0;
				Signal(FileStateSignal);
			}
			break;
		case FS_LOADED:
			if (!ClientList || MemoryLimit<MemoryNeed) {
				ResetData();
				State=FS_TOO_COSTLY;
				FileProgress=0.0;
				Signal(FileStateSignal);
			}
			break;
		case FS_TOO_COSTLY:
			if (ClientList && MemoryLimit>=MemoryNeed) {
				State=FS_WAITING;
				StartPSAgent();
				Signal(FileStateSignal);
			}
			break;
		default:
			break;
	}
}


bool emFileModel::StepLoading()
{
	bool ready, stateChanged;

	if (State==FS_LOADING) {
		try {
			ready=TryContinueLoading();
		}
		catch (emString errorText) {
			EndPSAgent();
			QuitLoading();
			ResetData();
			State=FS_LOAD_ERROR;
			ErrorText=errorText;
			return true;
		}
		stateChanged=false;
	}
	else if (State==FS_WAITING) {
		try {
			FileTime=emTryGetFileTime(GetFilePath());
		}
		catch (emString errorMessage) {
			EndPSAgent();
			State=FS_LOAD_ERROR;
			ErrorText=errorMessage;
			return true;
		}
		ResetData();
		State=FS_LOADING;
		try {
			TryStartLoading();
		}
		catch (emString errorText) {
			EndPSAgent();
			QuitLoading();
			ResetData();
			State=FS_LOAD_ERROR;
			ErrorText=errorText;
			return true;
		}
		ready=false;
		stateChanged=true;
	}
	else {
		return false;
	}

	MemoryNeed=CalcMemoryNeed();
	if (MemoryNeed<1) MemoryNeed=1;
	if (!ClientList || MemoryNeed>MemoryLimit) {
		EndPSAgent();
		QuitLoading();
		ResetData();
		State=FS_TOO_COSTLY;
		return true;
	}
	if (!ready) return stateChanged;
	EndPSAgent();
	QuitLoading();
	State=FS_LOADED;
	return true;
}


bool emFileModel::StepSaving()
{
	bool ready;

	if (State==FS_SAVING) {
		try {
			ready=TryContinueSaving();
		}
		catch (emString errorText) {
			EndPSAgent();
			QuitSaving();
			State=FS_SAVE_ERROR;
			ErrorText=errorText;
			return true;
		}
		if (!ready) return false;
	}
	else if (State==FS_UNSAVED) {
		State=FS_SAVING;
		ErrorText.Empty();
		try {
			TryStartSaving();
		}
		catch (emString errorText) {
			EndPSAgent();
			QuitSaving();
			State=FS_SAVE_ERROR;
			ErrorText=errorText;
			return true;
		}
		return true;
	}
	else {
		return false;
	}

	EndPSAgent();
	QuitSaving();
	try {
		FileTime=emTryGetFileTime(GetFilePath());
	}
	catch (emString errorMessage) {
		State=FS_SAVE_ERROR;
		ErrorText=errorMessage;
		return true;
	}
	State=FS_LOADED;
	MemoryNeed=CalcMemoryNeed();
	if (MemoryNeed<1) MemoryNeed=1;
	if (!ClientList || MemoryNeed>MemoryLimit) {
		ResetData();
		State=FS_TOO_COSTLY;
	}
	return true;
}


bool emFileModel::UpdateFileProgress()
{
	double pg;
	emUInt64 clk;

	switch (State) {
		case FS_LOADING:
		case FS_SAVING:
			clk=emGetClockMS();
			if (clk-FileProgressClock>=250) {
				FileProgressClock=clk;
				pg=CalcFileProgress();
			}
			else {
				return false;
			}
			break;
		case FS_LOADED:
		case FS_UNSAVED:
			pg=100.0;
			break;
		default:
			pg=0.0;
			break;
	}
	if (FileProgress>pg-0.01 && FileProgress<pg+0.01) return false;
	FileProgress=pg;
	return true;
}


void emFileModel::StartPSAgent()
{
	emFileModelClient * c;
	double pri;

	if (!PSAgent) PSAgent=new PSAgentClass(*this);
	c=ClientList;
	if (c) {
		pri=c->Priority;
		for (c=c->NextInList; c; c=c->NextInList) {
			if (pri<c->Priority) pri=c->Priority;
		}
		PSAgent->SetAccessPriority(pri);
	}
	PSAgent->RequestAccess();
}


void emFileModel::EndPSAgent()
{
	if (PSAgent) {
		delete PSAgent;
		PSAgent=NULL;
	}
}


emFileModel::PSAgentClass::PSAgentClass(
	emFileModel & fileModel
) :
	emPriSchedAgent(fileModel.GetRootContext(),"cpu"),
	FileModel(fileModel)
{
}


void emFileModel::PSAgentClass::GotAccess()
{
	FileModel.WakeUp();
}


//==============================================================================
//============================= emFileModelClient ==============================
//==============================================================================

emFileModelClient::emFileModelClient(
	emFileModel * model, emUInt64 memoryLimit, double priority
)
{
	MemoryLimit=memoryLimit;
	Priority=priority;
	ThisPtrInList=NULL;
	NextInList=NULL;
	if (model) SetModel(model);
}


emFileModelClient::~emFileModelClient()
{
	SetModel(NULL);
}


void emFileModelClient::SetModel(emFileModel * model)
{
	if (Model==model) return;
	if (Model) {
		*ThisPtrInList=NextInList;
		if (NextInList) NextInList->ThisPtrInList=ThisPtrInList;
		ThisPtrInList=NULL;
		NextInList=NULL;
		Model->ClientsChanged();
		Model=NULL;
	}
	if (model) {
		Model=model;
		NextInList=Model->ClientList;
		if (NextInList) NextInList->ThisPtrInList=&NextInList;
		Model->ClientList=this;
		ThisPtrInList=&Model->ClientList;
		Model->ClientsChanged();
	}
}


void emFileModelClient::SetMemoryLimit(emUInt64 bytes)
{
	if (MemoryLimit!=bytes) {
		MemoryLimit=bytes;
		if (Model) Model->ClientsChanged();
	}
}


void emFileModelClient::SetPriority(double priority)
{
	if (Priority!=priority) {
		Priority=priority;
		if (Model) Model->ClientsChanged();
	}
}
