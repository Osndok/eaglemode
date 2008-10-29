//------------------------------------------------------------------------------
// SilChessModel.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef SilChessModel_h
#define SilChessModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef SilChessMachine_h
#include <SilChess/SilChessMachine.h>
#endif


class SilChessModel : public emFileModel {

public:

	static emRef<SilChessModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	// (Yes, this interface is not clean)

	// --- Access to the chess machine ---

	SilChessMachine * GetMachine();
		// Returns NULL if not loaded. SaveAndSignalChanges() must be
		// called after each modification of the machine. The search
		// methods of the machine are called automatically by this model
		// (see below) and must not be called by anyone else.

	void SaveAndSignalChanges();
		// Save the machine state to the file and signal the change
		// signal.

	const emSignal & GetChangeSignal() const;
		// Signaled after each change of the machine (except for search
		// state).


	// --- Computer opponent ---

	// This model automatically searches and performs the computer moves.

	enum SearchStateType {
		SS_NOT_SEARCHING, // Human is on or game over or not loaded.
		SS_SEARCHING,     // Computer is on.
		SS_SEARCHING_HINT // Human is on and he requested a hint.
	};

	SearchStateType GetSearchState() const;

	void RequestHint();

	bool GetResultingHint(SilChessMachine::Move * pMove) const;

	const emSignal & GetSearchSignal() const;

protected:

	SilChessModel(emContext & context, const emString & name);
	virtual ~SilChessModel();
	virtual void ResetData();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	void ResetSearching();

	class SearchEngineClass : public emEngine {
	public:
		SearchEngineClass(SilChessModel & model);
		virtual ~SearchEngineClass();
	protected:
		virtual bool Cycle();
	private:
		SilChessModel & Model;
		emUInt64 StartTime;
	};

	friend class SearchEngineClass;

	SilChessMachine * Machine;
	emSignal ChangeSignal;
	SearchStateType SearchState;
	SilChessMachine::Move Hint;
	bool HintRequested,HintValid;
	emSignal SearchSignal;
	SearchEngineClass SearchEngine;
};

inline SilChessMachine * SilChessModel::GetMachine()
{
	return Machine;
}

inline const emSignal & SilChessModel::GetChangeSignal() const
{
	return ChangeSignal;
}

inline SilChessModel::SearchStateType SilChessModel::GetSearchState() const
{
	return SearchState;
}

inline const emSignal & SilChessModel::GetSearchSignal() const
{
	return SearchSignal;
}


#endif
