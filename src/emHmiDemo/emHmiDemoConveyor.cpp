//------------------------------------------------------------------------------
// emHmiDemoConveyor.cpp
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

#include <emHmiDemo/emHmiDemoConveyor.h>


emHmiDemoConveyor::emHmiDemoConveyor(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, int state,
	emColor color
)
	: emHmiDemoPiece(parent,name,rx,ry,rw,rh,"16",color),
	State(state),
	LbName(NULL),
	FlowIndicator(NULL),
	Controls(NULL),
	Monitors(NULL)
{
	SetFocusable(true);
}


emHmiDemoConveyor::~emHmiDemoConveyor()
{
}


void emHmiDemoConveyor::AutoExpand()
{
	emHmiDemoPiece::AutoExpand();

	LbName=new emLabel(this,"name",GetName());
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	LbName->SetLook(look);

	FlowIndicator = new emHmiDemoFlowIndicator(this,"flow",0.0,1);

	Controls = new emHmiDemoControls(this,"controls",4,4,4);

	Monitors = new emHmiDemoMonitors(this,"monitors",3);

	switch (State) {
	case 0:
		FlowIndicator->SetRPM(0.0);
		Controls->SetState(0);
		break;
	case 1:
		FlowIndicator->SetRPM(15.0);
		Controls->SetState(2);
		break;
	default:
		break;
	}
}


void emHmiDemoConveyor::AutoShrink()
{
	emHmiDemoPiece::AutoShrink();
	LbName=NULL;
	FlowIndicator=NULL;
	Controls=NULL;
	Monitors=NULL;
}


void emHmiDemoConveyor::LayoutChildren()
{
	if (LbName) LbName->Layout(0.2,0.05,0.3,0.09,GetInnerColor());
	if (FlowIndicator) FlowIndicator->Layout(0.64,0.07,0.16,0.16,GetInnerColor());
	if (Controls) Controls->Layout(0.45,0.06,0.15,0.18,GetInnerColor());
	if (Monitors) Monitors->Layout(0.2,0.177,0.22,0.0625,GetInnerColor());
}
