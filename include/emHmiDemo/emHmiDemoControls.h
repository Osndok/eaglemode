//------------------------------------------------------------------------------
// emHmiDemoControls.h
//
// Copyright (C) 2012,2014-2015,2022,2024 Oliver Hamann.
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

#ifndef emHmiDemoControls_h
#define emHmiDemoControls_h

#ifndef emHmiDemoAnalogDisplay_h
#include <emHmiDemo/emHmiDemoAnalogDisplay.h>
#endif

#ifndef emHmiDemoButton_h
#include <emHmiDemo/emHmiDemoButton.h>
#endif

#ifndef emHmiDemoFile_h
#include <emHmiDemo/emHmiDemoFile.h>
#endif


class emHmiDemoControls : public emLinearLayout {
public:
	emHmiDemoControls(
		ParentArg parent, const emString & name,
		int adCount=4, int buCount=4, int fiCount=4
	);
	virtual ~emHmiDemoControls();
	void SetState(int state);
private:
	int AdCount, BuCount, FiCount;
	emOwnArrayPtr<emHmiDemoAnalogDisplay *> Ad;
	emOwnArrayPtr<emHmiDemoButton *> Bu;
	emOwnArrayPtr<emHmiDemoFile *> Fi;
	emRasterLayout * AdRL;
	emRasterLayout * BuRL;
	emRasterLayout * FiRL;
};


#endif
