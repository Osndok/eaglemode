//------------------------------------------------------------------------------
// emMinesControlPanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emMinesControlPanel_h
#define emMinesControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emMinesFileModel_h
#include <emMines/emMinesFileModel.h>
#endif


class emMinesControlPanel : public emTkTiling {

public:

	emMinesControlPanel(
		ParentArg parent, const emString & name,
		emMinesFileModel * fileModel
	);
	virtual ~emMinesControlPanel();

protected:

	virtual bool Cycle();

private:

	emRef<emMinesFileModel> Mdl;

	emTkGroup * GrMines;
		emTkGroup * GrHelp;
			emTkLabel * LbHelp;
		emTkGroup * GrStartGame;
			emTkScalarField * SfLevel;
			emTkButton * BtStartGame;

	static const char * HelpText;
};


#endif
