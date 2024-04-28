//------------------------------------------------------------------------------
// emHmiDemoStation.cpp
//
// Copyright (C) 2012,2014,2021,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoStation.h>


emHmiDemoStation::emHmiDemoStation(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, int state,
	emColor color
)
	: emHmiDemoPiece(parent,name,rx,ry,rw,rh,"20",color),
	State(state),
	LbName(NULL),
	Controls(NULL)
{
	SetFocusable(true);
}


emHmiDemoStation::~emHmiDemoStation()
{
}


void emHmiDemoStation::AutoExpand()
{
	emHmiDemoPiece::AutoExpand();

	LbName=new emLabel(this,"name",GetName());
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	LbName->SetLook(look);

	Controls = new emHmiDemoControls(this,"controls",5,5,6);

	switch (State) {
	case 0:
		Controls->SetState(0);
		break;
	case 1:
		Controls->SetState(2);
		break;
	default:
		break;
	}
}


void emHmiDemoStation::AutoShrink()
{
	emHmiDemoPiece::AutoShrink();
	LbName=NULL;
	Controls=NULL;
}


void emHmiDemoStation::LayoutChildren()
{
	if (LbName) LbName->Layout(0.05,0.05,0.41,0.11,GetInnerColor());
	if (Controls) Controls->Layout(0.05,0.19,0.44,0.41,GetInnerColor());
}
