//------------------------------------------------------------------------------
// emFileModel.cpp
//
// Copyright (C) 2005-2008,2014,2016,2018-2019,2022 Oliver Hamann.
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
				if (MemoryLimit>=MemoryNeed) {
					State=FS_WAITING;
					StartPSAgent();
				}
				Signal(FileStateSignal);
			}
			break;
		case FS_TOO_COSTLY:
			if (MemoryNeed>1) {
				MemoryNeed=1;
				if (MemoryLimit>=MemoryNeed) {
					State=FS_WAITING;
					StartPSAgent();
				}
				Signal(FileStateSignal);
			}
			break;
		case FS_LOAD_ERROR:
			State=FS_TOO_COSTLY;
			ErrorText.Clear();
			MemoryNeed=1;
			if (MemoryLimit>=MemoryNeed) {
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
		ErrorText.Clear();
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
	ErrorText.Clear();
	if (MemoryLimit>=MemoryNeed) {
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
	MemoryLimitInvalid=0;
	PriorityInvalid=0;
	MemoryNeed=1;
	FileProgress=0.0;
	FileProgressClock=0;
	ClientList=NULL;
	MemoryLimit=0;
	LastMTime=0;
	LastCTime=0;
	LastFSize=0;
	LastINode=0;
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
	bool memoryLimitIncreased,stateChanged;

	memoryLimitIncreased=false;
	if (MemoryLimitInvalid) {
		memoryLimitIncreased=UpdateMemoryLimit();
	}

	if (PriorityInvalid) {
		UpdatePriority();
	}

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
			if (memoryLimitIncreased) {
				// When the limit for this file model has
				// increased, then the limit for other file
				// models may have decreased by the same cause.
				// Give those other file models a chance to free
				// memory before allocating by this file model
				// through calling StepLoading.
				WakeUp();
				return true;
			}
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
		ErrorText.Clear();
		Signal(FileStateSignal);
	}
}


void emFileModel::TryFetchDate()
{
	struct em_stat st;

	if (em_stat(GetFilePath().Get(),&st)!=0) {
		throw emException(
			"Failed to get info about \"%s\": %s",
			GetFilePath().Get(),
			emGetErrorText(errno).Get()
		);
	}
	LastMTime=st.st_mtime;
	LastCTime=st.st_ctime;
	LastFSize=st.st_size;
	LastINode=st.st_ino;
}


bool emFileModel::IsOutOfDate()
{
	emFileModelClient * c;
	struct em_stat st;

	if (em_stat(GetFilePath().Get(),&st)!=0) {
		return true;
	}

	if (
		LastMTime!=st.st_mtime ||
		LastCTime!=st.st_ctime ||
		LastFSize!=(emUInt64)st.st_size ||
		LastINode!=(emUInt64)st.st_ino
	) {
		return true;
	}

	for (c=ClientList; c; c=c->NextInList) {
		if (c->IsReloadAnnoying()) return false;
	}
	return true;
}


bool emFileModel::StepLoading()
{
	bool ready, stateChanged;

	if (State==FS_LOADING) {
		try {
			ready=TryContinueLoading();
		}
		catch (const emException & exception) {
			EndPSAgent();
			QuitLoading();
			ResetData();
			State=FS_LOAD_ERROR;
			ErrorText=exception.GetText();
			return true;
		}
		stateChanged=false;
	}
	else if (State==FS_WAITING) {
		try {
			TryFetchDate();
		}
		catch (const emException & e) {
			EndPSAgent();
			State=FS_LOAD_ERROR;
			ErrorText=e.GetText();
			return true;
		}
		ResetData();
		State=FS_LOADING;
		try {
			TryStartLoading();
		}
		catch (const emException & exception) {
			EndPSAgent();
			QuitLoading();
			ResetData();
			State=FS_LOAD_ERROR;
			ErrorText=exception.GetText();
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
	if (MemoryNeed>MemoryLimit) {
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
		catch (const emException & exception) {
			EndPSAgent();
			QuitSaving();
			State=FS_SAVE_ERROR;
			ErrorText=exception.GetText();
			return true;
		}
		if (!ready) return false;
	}
	else if (State==FS_UNSAVED) {
		State=FS_SAVING;
		ErrorText.Clear();
		try {
			TryStartSaving();
		}
		catch (const emException & exception) {
			EndPSAgent();
			QuitSaving();
			State=FS_SAVE_ERROR;
			ErrorText=exception.GetText();
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
		TryFetchDate();
	}
	catch (const emException & e) {
		State=FS_SAVE_ERROR;
		ErrorText=e.GetText();
		return true;
	}
	State=FS_LOADED;
	MemoryNeed=CalcMemoryNeed();
	if (MemoryNeed<1) MemoryNeed=1;
	if (MemoryNeed>MemoryLimit) {
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


bool emFileModel::UpdateMemoryLimit()
{
	emFileModelClient * c;
	emUInt64 maxLim,lim;
	bool limitIncreased;

	maxLim=0;
	for (c=ClientList; c; c=c->NextInList) {
		lim=c->GetMemoryLimit();
		if (maxLim<lim) maxLim=lim;
	}
	MemoryLimitInvalid=0;
	if (MemoryLimit==maxLim) return false;
	limitIncreased=(MemoryLimit<maxLim);
	MemoryLimit=maxLim;

	switch (State) {
		case FS_WAITING:
			if (MemoryLimit<MemoryNeed) {
				EndPSAgent();
				State=FS_TOO_COSTLY;
				Signal(FileStateSignal);
			}
			break;
		case FS_LOADING:
			if (MemoryLimit<MemoryNeed) {
				EndPSAgent();
				QuitLoading();
				ResetData();
				State=FS_TOO_COSTLY;
				FileProgress=0.0;
				Signal(FileStateSignal);
			}
			break;
		case FS_LOADED:
			if (MemoryLimit<MemoryNeed) {
				ResetData();
				State=FS_TOO_COSTLY;
				FileProgress=0.0;
				Signal(FileStateSignal);
			}
			break;
		case FS_TOO_COSTLY:
			if (MemoryLimit>=MemoryNeed) {
				State=FS_WAITING;
				StartPSAgent();
				Signal(FileStateSignal);
			}
			break;
		default:
			break;
	}

	return limitIncreased;
}


void emFileModel::UpdatePriority()
{
	emFileModelClient * c;
	double maxPri,pri;

	if (PSAgent) {
		c=ClientList;
		if (c) {
			maxPri=c->GetPriority();
			for (c=c->NextInList; c; c=c->NextInList) {
				pri=c->GetPriority();
				if (maxPri<pri) maxPri=pri;
			}
			PSAgent->SetAccessPriority(maxPri);
		}
	}
	PriorityInvalid=0;
}


void emFileModel::StartPSAgent()
{
	if (!PSAgent) PSAgent=new PSAgentClass(*this);
	UpdatePriority();
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

emFileModelClient::emFileModelClient(emFileModel * model)
{
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
		Model->MemoryLimitInvalid=1;
		Model->PriorityInvalid=1;
		Model->WakeUp();
		Model=NULL;
	}
	if (model) {
		Model=model;
		NextInList=Model->ClientList;
		if (NextInList) NextInList->ThisPtrInList=&NextInList;
		Model->ClientList=this;
		ThisPtrInList=&Model->ClientList;
		Model->MemoryLimitInvalid=1;
		Model->PriorityInvalid=1;
		Model->WakeUp();
	}
}


void emFileModelClient::InvalidateMemoryLimit()
{
	if (Model) {
		Model->MemoryLimitInvalid=1;
		Model->WakeUp();
	}
}


void emFileModelClient::InvalidatePriority()
{
	if (Model) {
		Model->PriorityInvalid=1;
		Model->WakeUp();
	}
}


bool emFileModelClient::IsTheOnlyClient() const
{
	if (!Model) return false;
	return Model->ClientList->NextInList==NULL;
}


//==============================================================================
//========================= emAbsoluteFileModelClient ==========================
//==============================================================================

emAbsoluteFileModelClient::emAbsoluteFileModelClient(emFileModel * model)
	: emFileModelClient(model)
{
}


emUInt64 emAbsoluteFileModelClient::GetMemoryLimit() const
{
	return EM_UINT64_MAX;
}


double emAbsoluteFileModelClient::GetPriority() const
{
	return 1.0;
}


bool emAbsoluteFileModelClient::IsReloadAnnoying() const
{
	return true;
}
