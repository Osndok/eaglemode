//------------------------------------------------------------------------------
// emNetwalkControlPanel.h
//
// Copyright (C) 2010-2011,2014-2015 Oliver Hamann.
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

#ifndef emNetwalkControlPanel_h
#define emNetwalkControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emNetwalkModel_h
#include <emNetwalk/emNetwalkModel.h>
#endif


class emNetwalkControlPanel : public emLinearLayout {

public:

	emNetwalkControlPanel(ParentArg parent, const emString & name,
	                      emView & contentView, emNetwalkModel * fileModel);

	virtual ~emNetwalkControlPanel();

protected:

	virtual bool Cycle();

private:

	void UpdateFields();

	emView & ContentView;

	emRef<emNetwalkModel> Mdl;

	emLinearGroup * GrAbout;
		emLabel * LbAbout;
	emLinearGroup * GrStart;
		emScalarField * SfSize;
		emScalarField * SfComplexity;
		emCheckBox * CbBorderless;
		emCheckBox * CbNoFourWayJunctions;
		emCheckBox * CbDigMode;
		emButton * BtStart;
	emRasterGroup * GrExtra;
		emCheckBox * CbAutoMark;
		emButton * BtUnmarkAll;
	emTextField * TfPenalty;
};


#endif
