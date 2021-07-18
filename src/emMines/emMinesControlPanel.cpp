//------------------------------------------------------------------------------
// emMinesControlPanel.cpp
//
// Copyright (C) 2006-2008,2010-2011,2014-2015,2021 Oliver Hamann.
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

#include <emMines/emMinesControlPanel.h>


emMinesControlPanel::emMinesControlPanel(
	ParentArg parent, const emString & name, emMinesFileModel * fileModel
)
	: emLinearLayout(parent,name)
{
	Mdl=fileModel;

	SetMinChildTallness(0.26);
	SetMaxChildTallness(1.5);
	SetAlignment(EM_ALIGN_TOP);

	GrMines=new emPackGroup(this,"","emMines");
	GrMines->SetPrefChildTallness(0, 0.6);
	GrMines->SetPrefChildTallness(1, 0.4);
	GrMines->SetChildWeight(0,0.4);
	GrMines->SetChildWeight(1,0.6);
		GrHelp=new emLinearGroup(GrMines,"help","How to play the game");
		GrHelp->SetBorderScaling(2.0);
			LbHelp=new emLabel(GrHelp,"text",HelpText);
		GrStartGame=new emRasterGroup(GrMines,"start","New Game");
		GrStartGame->SetPrefChildTallness(0.2);
		GrStartGame->SetBorderScaling(2.0);
			SfLevel=new emScalarField(
				GrStartGame,
				"sf",
				"Level of Difficulty",
				"Levels 1 and 2 are good for beginners. Level 3 is good for everyday\n"
				"usage, because it can be solved in a reasonable time and without\n"
				"navigating around. Levels 4 and 5 are just inhuman.",
				emImage(),
				1,5,Mdl->DetectLevel(),
				true
			);
			SfLevel->SetBorderScaling(1.1);
			BtStartGame=new emButton(
				GrStartGame,
				"bt",
				"Start",
				"Start a new game with the given level of difficulty.\n"
				"\n"
				"Hotkeys:\n"
				"\n"
				"  Ctrl+N  Start new game of same level.\n"
				"  Ctrl+1  Start new game of level 1.\n"
				"  Ctrl+2  Start new game of level 2.\n"
				"  ...\n"
				"  Ctrl+5  Start new game of level 5.\n"
			);

	AddWakeUpSignal(BtStartGame->GetClickSignal());
	AddWakeUpSignal(Mdl->GetChangeSignal());
}


emMinesControlPanel::~emMinesControlPanel()
{
}


bool emMinesControlPanel::Cycle()
{
	if (IsSignaled(BtStartGame->GetClickSignal())) {
		if (
			Mdl->GetFileState()==emFileModel::FS_LOADED ||
			Mdl->GetFileState()==emFileModel::FS_UNSAVED
		) {
			Mdl->StartGame((int)SfLevel->GetValue());
		}
	}
	if (IsSignaled(Mdl->GetChangeSignal())) {
		SfLevel->SetValue(Mdl->DetectLevel());
	}
	return emLinearLayout::Cycle();
}


const char * const emMinesControlPanel::HelpText=
	"emMines is a three-dimensional variant of the popular mine sweeper logic game.\n"
	"\n"
	"The game world consists of a three-dimensional grid of cells. Each cell is\n"
	"either a mine or not. Goal of the game is to open all cells which are not\n"
	"mines. When opening a mine, the game is lost.\n"
	"\n"
	"Each cell shows either a cube, a sphere or a number. The meaning is as follows:\n"
	"\n"
	"Cube   - This is a closed cell.\n"
	"\n"
	"Sphere - This is a closed cell where a mark has been set. The mark means that\n"
	"         it is assumed to have a mine here.\n"
	"\n"
	"Number - This is an open cell and it is no mine. It shows the number of mines\n"
	"         in the adjacent cells. All the orthogonal and diagonal neighbours are\n"
	"         counted here. So it can be up to 26, theoretically.\n"
	"\n"
	"The lines between the cells and the colors of the grid layers are just for a\n"
	"better recognition. When the games starts, the cells of the top layer are\n"
	"already solved as a starting aid.\n"
	"\n"
	"To open a cell, click with the left mouse button on it. If a \"0\" is opened,\n"
	"all adjacent cells are automatically opened too.\n"
	"\n"
	"To mark or unmark a cell, click with the right mouse button on it."
;
