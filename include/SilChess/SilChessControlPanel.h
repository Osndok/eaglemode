//------------------------------------------------------------------------------
// SilChessControlPanel.h
//
// Copyright (C) 2007-2008,2014-2015 Oliver Hamann.
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

#ifndef SilChessControlPanel_h
#define SilChessControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef SilChessModel_h
#include <SilChess/SilChessModel.h>
#endif


class SilChessControlPanel : public emLinearLayout {

public:

	SilChessControlPanel(ParentArg parent, const emString & name,
	                     SilChessModel * model);

protected:

	virtual bool Cycle();

private:

	void UpdateControls();

	emRef<SilChessModel> Mdl;

	emLinearGroup * GrAbout;
	emLabel * LbAbout;
	emButton * BtNew;
	emButton * BtFlip;
	emButton * BtUndo;
	emButton * BtHint;
	emScalarField * SfDepth;
	emTextField * TfStatus;

	int LastOutputDepth;
};


#endif
