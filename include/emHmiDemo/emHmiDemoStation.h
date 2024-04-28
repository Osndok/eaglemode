//------------------------------------------------------------------------------
// emHmiDemoStation.h
//
// Copyright (C) 2012,2014,2024 Oliver Hamann.
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

#ifndef emHmiDemoStation_h
#define emHmiDemoStation_h

#ifndef emHmiDemoControls_h
#include <emHmiDemo/emHmiDemoControls.h>
#endif

#ifndef emHmiDemoPiece_h
#include <emHmiDemo/emHmiDemoPiece.h>
#endif


class emHmiDemoStation : public emHmiDemoPiece {
public:
	emHmiDemoStation(
		ParentArg parent, const emString & name,
		int rx, int ry, int rw=13, int rh=10, int state=0,
		emColor color=0x808080FF
	);
	virtual ~emHmiDemoStation();
protected:
	virtual void AutoExpand();
	virtual void AutoShrink();
	virtual void LayoutChildren();
private:
	int State;
	emLabel * LbName;
	emHmiDemoControls * Controls;
};


#endif