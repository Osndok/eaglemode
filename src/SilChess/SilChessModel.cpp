//------------------------------------------------------------------------------
// SilChessModel.cpp
//
// Copyright (C) 2007-2009 Oliver Hamann.
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

#include <SilChess/SilChessModel.h>


emRef<SilChessModel> SilChessModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(SilChessModel,context,name,common)
}


void SilChessModel::SaveAndSignalChanges()
{
	if (Machine) {
		SetUnsavedState();
		Save(true);
		Signal(ChangeSignal);
		ResetSearching();
	}
}


void SilChessModel::RequestHint()
{
	HintRequested=true;
	SearchEngine.WakeUp();
}


bool SilChessModel::GetResultingHint(SilChessMachine::Move * pMove) const
{
	if (HintValid && pMove) *pMove=Hint;
	return HintValid;
}


SilChessModel::SilChessModel(emContext & context, const emString & name)
	: emFileModel(context,name), SearchEngine(*this)
{
	Machine=NULL;
	SearchState=SS_NOT_SEARCHING;
	HintRequested=false;
	HintValid=false;
}


SilChessModel::~SilChessModel()
{
	SilChessModel::ResetData();
}


void SilChessModel::ResetData()
{
	if (Machine) {
		delete Machine;
		Machine=NULL;
		Signal(ChangeSignal);
		ResetSearching();
	}
}


void SilChessModel::TryStartLoading() throw(emString)
{
	Machine=new SilChessMachine();
	errno=0;
	if (!Machine->Load(GetFilePath())) {
		if (errno) throw emGetErrorText(errno);
		else throw emString("file format error");
	}
	Signal(ChangeSignal);
	ResetSearching();
}


bool SilChessModel::TryContinueLoading() throw(emString)
{
	return true;
}


void SilChessModel::QuitLoading()
{
}


void SilChessModel::TryStartSaving() throw(emString)
{
	errno=0;
	if (!Machine->Save(GetFilePath())) {
		if (errno) throw emGetErrorText(errno);
		else throw emString("unknown error");
	}
}


bool SilChessModel::TryContinueSaving() throw(emString)
{
	return true;
}


void SilChessModel::QuitSaving()
{
}


emUInt64 SilChessModel::CalcMemoryNeed()
{
	return 2*sizeof(SilChessMachine);
}


double SilChessModel::CalcFileProgress()
{
	return 0.0;
}


void SilChessModel::ResetSearching()
{
	if (SearchState!=SS_NOT_SEARCHING) {
		SearchState=SS_NOT_SEARCHING;
		Signal(SearchSignal);
	}
	if (HintValid) {
		HintValid=false;
		Signal(SearchSignal);
	}
	HintRequested=false;
	if (Machine) {
		Machine->EndSearching();
		SearchEngine.WakeUp();
	}
}


SilChessModel::SearchEngineClass::SearchEngineClass(SilChessModel & model)
	: emEngine(model.GetScheduler()), Model(model)
{
	StartTime=0;
	SetEnginePriority(LOW_PRIORITY);
}


SilChessModel::SearchEngineClass::~SearchEngineClass()
{
}


bool SilChessModel::SearchEngineClass::Cycle()
{
	SilChessMachine * machine;
	SilChessMachine::Move move;

	machine=Model.Machine;
	if (!machine) return false;

	if (machine->IsSearching()) {
		if (emGetClockMS()-StartTime<5000 && IsTimeSliceAtEnd()) return true;
		if (!machine->ContinueSearching()) {
			if (!IsTimeSliceAtEnd()) WakeUp();
			return true;
		}
		Model.SearchState=SS_NOT_SEARCHING;
		Signal(Model.SearchSignal);
		if (machine->EndSearching(&move)) {
			if (machine->IsHumanOn()) {
				Model.HintRequested=false;
				Model.Hint=move;
				Model.HintValid=true;
			}
			else {
				machine->DoMove(move);
				Model.SaveAndSignalChanges();
			}
		}
		return true;
	}

	if (
		machine->IsMate() ||
		machine->IsDraw() ||
		machine->IsEndless() ||
		(machine->IsHumanOn() && !Model.HintRequested)
	) return false;

	machine->StartSearching();
	if (machine->IsHumanOn()) {
		Model.SearchState=SS_SEARCHING_HINT;
	}
	else {
		Model.SearchState=SS_SEARCHING;
	}
	Signal(Model.SearchSignal);
	StartTime=emGetClockMS();
	return true;
}
