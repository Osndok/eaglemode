//------------------------------------------------------------------------------
// emNetwalkControlPanel.cpp
//
// Copyright (C) 2010-2011 Oliver Hamann.
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

#include <emNetwalk/emNetwalkControlPanel.h>


emNetwalkControlPanel::emNetwalkControlPanel(
	ParentArg parent, const emString & name, emView & contentView,
	emNetwalkModel * fileModel
)
	: emTkGroup(parent,name,"emNetwalk"),
	ContentView(contentView)
{
	emTkTiling * t1, * t2, * t3, * t4;
	Mdl=fileModel;

	SetPrefChildTallness(1.0);
	SetPrefChildTallness(0.3,1);
	SetPrefChildTallness(0.5,2);

	GrAbout=new emTkGroup(this,"about","About emNetwalk");
	LbAbout=new emTkLabel(GrAbout,"text",
		"emNetwalk is a clone of the addictive Netwalk puzzle game where pieces of a\n"
		"computer network have to be rotated in order to connect terminals to a server.\n"
		"In contrast to other implementations, emNetwalk has a very sophisticated random\n"
		"network generator which allows a more free and exact game configuration. In\n"
		"addition, emNetwalk provides a special option for making the game even more fun:\n"
		"The Dig Mode forbids to rotate pieces that are not yet near a server connection.\n"
		"\n"
		"How to play\n"
		"-----------\n"
		"\n"
		"In order to solve the puzzle, you have to connect all the terminals to the\n"
		"server. You can do this by rotating pieces of the network (cable segments,\n"
		"junctions, terminals, and the server). The solution must not contain any loops\n"
		"or islands, so that there is always exactly one path from the server to each\n"
		"terminal. Every game has exactly one such solution.\n"
		"\n"
		"Mouse functions:\n"
		"\n"
		"  Left button: Rotate piece counterclockwise.\n"
		"\n"
		"  Shift + left button: Rotate piece clockwise.\n"
		"\n"
		"  Right button: Mark or unmark a piece. A marked piece cannot be rotated.\n"
		"\n"
		"  Ctrl + left button + movement: Scroll board (only in borderless mode)\n"
		"\n"
		"Here are some tips for beginners:\n"
		"\n"
		"4-way junctions are always correct and must not be rotated.\n"
		"\n"
		"A straight segment besides a border must point along the border.\n"
		"\n"
		"One or more straight segments between two terminals must not connect the\n"
		"terminals.\n"
		"\n"
		"An arc segment between two straight segments requires that the straight\n"
		"segments have different orientations."
	);

	GrStart=new emTkGroup(this,"start","New Game");
	GrStart->SetFixedColumnCount(1);
	GrStart->SetPrefChildTallness(0.2);
	GrStart->SetPrefChildTallness(0.12,-1);
	t1=new emTkTiling(GrStart,"t1");
	t1->SetPrefChildTallness(0.2);
	t2=new emTkTiling(t1,"t2");
	t2->SetPrefChildTallness(0.1);
	t3=new emTkTiling(t1,"t3");
	t3->SetPrefChildTallness(0.1);
	SfSize=new emTkScalarField(
		t2,"size","Size",
		"Here you can set the size of the board as the number\n"
		"of pieces in horizontal and vertical direction."
	);
	SfSize->SetMinMaxValues(5,25);
	SfSize->SetScaleMarkIntervals(5,1,0);
	SfSize->SetEditable();
	SfComplexity=new emTkScalarField(
		t2,"complexity","Complexity",
		"This is the difficulty of the network. The higher the value,\n"
		"the more junctions and terminals are generated per area."
	);
	SfComplexity->SetMinMaxValues(1,5);
	SfComplexity->SetScaleMarkIntervals(1,0);
	SfComplexity->SetEditable();
	CbBorderless=new emTkCheckBox(
		t3,"borderless","Borderless",
		"If this is enabled, connections can wrap from one edge to the\n"
		"opposite edge so that there are no borders. This makes the game\n"
		"more difficult. Remember that you can scroll the whole board\n"
		"with Ctrl + left mouse button + mouse movement."
	);
	CbBorderless->SetNoEOI();
	CbNoFourWayJunctions=new emTkCheckBox(
		t3,"no4wayjunctions","No 4-Way Junctions",
		"If this is enabled, the network will not contain any 4-way\n"
		"junctions. This makes the game more difficult."
	);
	CbNoFourWayJunctions->SetNoEOI();
	CbDigMode=new emTkCheckBox(
		t3,"digmode","Dig Mode",
		"If this is enabled, pieces that are not near a server connection\n"
		"are dug in and cannot be rotated, so that the puzzle has to be\n"
		"solved by \"digging\" from server to terminals. This makes the\n"
		"game more difficult."
	);
	CbDigMode->SetNoEOI();
	BtStart=new emTkButton(
		GrStart,"start","Start New Game",
		"Start a new game with the given settings.\n"
		"\n"
		"Hotkey: Ctrl+N"
	);

	t4=new emTkTiling(this,"t4");
	t4->SetPrefChildTallness(0.07);
	t4->SetPrefChildTallness(0.33,-1);
	GrExtra=new emTkGroup(t4,"extra","Extra");
	GrExtra->SetBorderScaling(2.0);
	CbAutoMark=new emTkCheckBox(
		GrExtra,"automark","Auto Mark",
		"Whether to mark pieces automatically after rotating them."
	);
	CbAutoMark->SetNoEOI();
	BtUnmarkAll=new emTkButton(
		GrExtra,"unmarkall","Unmark All",
		"Unmark all pieces.\n"
		"\n"
		"Hotkey: Ctrl+U"
	);
	TfPenalty=new emTkTextField(
		t4,"penalty","Penalty Points",
		"A penalty point is given whenever you rotate a piece once again\n"
		"after rotating at least one other piece in between. Advanced players\n"
		"should try to solve the puzzle without getting any penalty points."
	);

	UpdateFields();

	AddWakeUpSignal(Mdl->GetChangeSignal());
	AddWakeUpSignal(BtStart->GetClickSignal());
	AddWakeUpSignal(CbAutoMark->GetCheckSignal());
	AddWakeUpSignal(BtUnmarkAll->GetClickSignal());
}


emNetwalkControlPanel::~emNetwalkControlPanel()
{
}


bool emNetwalkControlPanel::Cycle()
{
	if (IsSignaled(Mdl->GetChangeSignal())) {
		UpdateFields();
	}

	if (IsSignaled(BtStart->GetClickSignal())) {
		if (
			Mdl->GetFileState()==emFileModel::FS_LOADED ||
			Mdl->GetFileState()==emFileModel::FS_UNSAVED
		) {
			try {
				Mdl->TrySetup(
					(int)SfSize->GetValue(),
					(int)SfSize->GetValue(),
					CbBorderless->IsChecked(),
					CbNoFourWayJunctions->IsChecked(),
					(int)SfComplexity->GetValue(),
					CbDigMode->IsChecked(),
					CbAutoMark->IsChecked()
				);
			}
			catch (emString errorMessage) {
				emTkDialog::ShowMessage(ContentView,"Error",errorMessage);
			}
		}
	}

	if (IsSignaled(CbAutoMark->GetCheckSignal())) {
		Mdl->SetAutoMark(CbAutoMark->IsChecked());
	}

	if (IsSignaled(BtUnmarkAll->GetClickSignal())) {
		Mdl->UnmarkAll();
	}

	return emTkGroup::Cycle();
}


void emNetwalkControlPanel::UpdateFields()
{
	SfSize->SetValue((Mdl->GetWidth()+Mdl->GetHeight()+1)/2);
	SfComplexity->SetValue(Mdl->GetComplexity());
	CbBorderless->SetChecked(Mdl->IsBorderless());
	CbNoFourWayJunctions->SetChecked(Mdl->IsNoFourWayJunctions());
	CbDigMode->SetChecked(Mdl->IsDigMode());
	CbAutoMark->SetChecked(Mdl->IsAutoMark());
	TfPenalty->SetText(emString::Format("%d",Mdl->GetPenaltyPoints()));
}
