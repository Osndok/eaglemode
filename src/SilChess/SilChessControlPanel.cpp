//------------------------------------------------------------------------------
// SilChessControlPanel.cpp
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#include <SilChess/SilChessControlPanel.h>


SilChessControlPanel::SilChessControlPanel(
	ParentArg parent, const emString & name, SilChessModel * model
)
	: emTkGroup(parent,name,"SilChess")
{
	emTkTiling * t1, * t2;

	Mdl=model;
	LastOutputDepth=0;

	GrAbout=new emTkGroup(this,"about","About SilChess");
	LbAbout=new emTkLabel(
		GrAbout,
		"label",
		"SilChess is an easy-to-use chess program for playing human versus computer.\n"
		"It is suitable for beginners and occasional players, but not for chess\n"
		"professionals. The computer opponent is quite weak in compare to other chess\n"
		"programs, and there is no nerving chess clock. In fact, SilChess has been\n"
		"written by a weak chess player (and weak chess programmer), and the name\n"
		"SilChess has been derived from \"silly chess\".\n"
		"\n"
		"How to make a move: Click on the square below the piece you want to move,\n"
		"and then click on the square where you want the piece to move. For castling,\n"
		"move the king accordingly.\n"
		"\n"
		"Messages like \"check!\" or \"MATE!\" are shown in the status field here in\n"
		"the control view.\n"
		"\n"
		"The chess rules are not described here. In SilChess, pawns are always\n"
		"converted to queens when they reach the opposite end."
	);

	t1=new emTkTiling(this,"t");
		t2=new emTkTiling(t1,"t");
			BtNew=new emTkButton(
				t2,
				"new",
				"New",
				"Start a new game.\n"
				"\n"
				"Hotkey: Ctrl+N"
			);
			BtFlip=new emTkButton(
				t2,
				"flip",
				"Flip",
				"Exchange sides.\n"
				"\n"
				"Hotkey: Ctrl+F"
			);
			BtUndo=new emTkButton(
				t2,
				"undo",
				"Undo",
				"Take back your last move.\n"
				"\n"
				"Hotkey: Ctrl+Z"
			);
			BtHint=new emTkButton(
				t2,
				"hint",
				"Hint",
				"Let the computer search a move for you and show it as a hint.\n"
				"\n"
				"Hotkey: Ctrl+H"
			);
			SfDepth=new emTkScalarField(
				t2,
				"depth",
				"Search Depth",
				"How hard the computer is searching for good moves.\n"
				"Zero means to find random moves.\n"
				"\n"
				"Hotkeys: Ctrl+0, Ctrl+1, Ctrl+2, ...",
				emImage(),
				0,
				SilChessMachine::MAX_SEARCH_DEPTH,
				LastOutputDepth,
				true
			);
		TfStatus=new emTkTextField(
			t1,
			"status",
			"Status",
			"This status field shows the last move, the check state, who is on, and\n"
			"more. The number enclosed in < > is the level-one computer evaluation\n"
			"of the chances of the one who is on (it's a debug info, originally)."
		);


	GrAbout->SetBorderScaling(2.0);
	t1->SetPrefChildTallness(0.1);
	t1->SetPrefChildTallness(0.15,-1);
	t2->SetPrefChildTallness(0.4);
	t2->SetPrefChildTallness(0.2,4);
	SetPrefChildTallness(0.6);
	SetPrefChildTallness(0.2,1);

	UpdateControls();

	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(Mdl->GetSearchSignal());
	AddWakeUpSignal(BtNew->GetClickSignal());
	AddWakeUpSignal(BtFlip->GetClickSignal());
	AddWakeUpSignal(BtUndo->GetClickSignal());
	AddWakeUpSignal(BtHint->GetClickSignal());
	AddWakeUpSignal(SfDepth->GetValueSignal());
}


bool SilChessControlPanel::Cycle()
{
	SilChessMachine * machine;
	int depth;

	if (
		IsSignaled(Mdl->GetChangeSignal()) ||
		IsSignaled(Mdl->GetSearchSignal())
	) {
		UpdateControls();
	}

	machine=Mdl->GetMachine();
	if (machine) {
		if (IsSignaled(BtNew->GetClickSignal())) {
			machine->StartNewGame();
			Mdl->SaveAndSignalChanges();
		}
		if (IsSignaled(BtFlip->GetClickSignal())) {
			machine->SetHumanWhite(!machine->IsHumanWhite());
			Mdl->SaveAndSignalChanges();
		}
		if (IsSignaled(BtUndo->GetClickSignal())) {
			machine->UndoMove();
			if (!machine->IsHumanOn()) machine->UndoMove();
			Mdl->SaveAndSignalChanges();
		}
		if (IsSignaled(BtHint->GetClickSignal())) {
			Mdl->RequestHint();
		}
		if (IsSignaled(SfDepth->GetValueSignal())) {
			depth=(int)SfDepth->GetValue();
			if (LastOutputDepth!=depth) {
				LastOutputDepth=depth;
				machine->SetSearchDepth(depth);
				Mdl->SaveAndSignalChanges();
			}
		}
	}

	return emTkGroup::Cycle();
}


void SilChessControlPanel::UpdateControls()
{
	SilChessMachine * machine;
	SilChessMachine::Move move;
	char tmp[512];

	machine=Mdl->GetMachine();
	if (!machine) {
		BtNew->SetEnableSwitch(false);
		BtFlip->SetEnableSwitch(false);
		BtUndo->SetEnableSwitch(false);
		BtHint->SetEnableSwitch(false);
		SfDepth->SetEnableSwitch(false);
		TfStatus->SetEnableSwitch(false);
		return;
	}

	BtNew->SetEnableSwitch(true);
	BtFlip->SetEnableSwitch(true);
	BtUndo->SetEnableSwitch(machine->GetMoveCount()>0);
	BtHint->SetEnableSwitch(machine->IsHumanOn() && !machine->IsSearching());
	SfDepth->SetEnableSwitch(true);
	TfStatus->SetEnableSwitch(true);
	LastOutputDepth=machine->GetSearchDepth();
	SfDepth->SetValue(LastOutputDepth);

	tmp[0]=0;
	if (machine->GetMoveCount()>0) {
		machine->GetMove(machine->GetMoveCount()-1).ToString(tmp+strlen(tmp));
		sprintf(tmp+strlen(tmp)," <%d>  ",machine->GetValue());
	}
	if (machine->IsMate()) strcat(tmp,"MATE!");
	else if (machine->IsDraw()) strcat(tmp,"DRAW!");
	else if (machine->IsEndless()) strcat(tmp,"ENDLESS!");
	else {
		if (machine->IsCheck()) strcat(tmp,"check!");
		switch (Mdl->GetSearchState()) {
		case SilChessModel::SS_NOT_SEARCHING:
			if (machine->IsHumanOn()) {
				if (Mdl->GetResultingHint(&move)) {
					strcat(tmp," hint: ");
					move.ToString(tmp+strlen(tmp));
					strcat(tmp,",");
				}
				strcat(tmp," your move? ");
			}
			break;
		case SilChessModel::SS_SEARCHING:
			sprintf(tmp+strlen(tmp)," searching (%d)...",machine->GetSearchDepth());
			break;
		case SilChessModel::SS_SEARCHING_HINT:
			sprintf(tmp+strlen(tmp)," searching hint (%d)...",machine->GetSearchDepth());
			break;
		}
	}
	TfStatus->SetText(tmp);
}
