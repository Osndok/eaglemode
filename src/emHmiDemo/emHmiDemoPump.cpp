//------------------------------------------------------------------------------
// emHmiDemoPump.cpp
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

#include <emHmiDemo/emHmiDemoPump.h>


emHmiDemoPump::emHmiDemoPump(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, int state,
	emColor color
)
	: emHmiDemoPiece(parent,name,rx,ry,rw,rh,"17",color),
	State(state),
	LbName(NULL),
	FlowIndicator(NULL),
	Controls(NULL)
{
	SetFocusable(true);
}


emHmiDemoPump::~emHmiDemoPump()
{
}


void emHmiDemoPump::AutoExpand()
{
	emHmiDemoPiece::AutoExpand();

	LbName=new emLabel(this,"name",GetName());
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	LbName->SetLook(look);

	FlowIndicator = new emHmiDemoFlowIndicator(this,"flow",0.0,0);

	Controls = new emHmiDemoControls(this,"controls",3,4,4);

	switch (State) {
	case 0:
		FlowIndicator->SetRPM(0.0);
		Controls->SetState(0);
		break;
	case 1:
		FlowIndicator->SetRPM(10.0);
		Controls->SetState(1);
		break;
	case 2:
		FlowIndicator->SetRPM(50.0);
		Controls->SetState(2);
		break;
	case 3:
		FlowIndicator->SetRPM(-10.0);
		Controls->SetState(1);
		break;
	case 4:
		FlowIndicator->SetRPM(-50.0);
		Controls->SetState(2);
		break;
	default:
		break;
	}
}


void emHmiDemoPump::AutoShrink()
{
	emHmiDemoPiece::AutoShrink();
	LbName=NULL;
	FlowIndicator=NULL;
	Controls=NULL;
}


void emHmiDemoPump::LayoutChildren()
{
	if (LbName) LbName->Layout(0.12,0.1,0.4,0.11,GetInnerColor());
	if (FlowIndicator) FlowIndicator->Layout(0.12,0.22,0.33,0.33,GetInnerColor());
	if (Controls) Controls->Layout(0.5,0.12,0.36,0.5,GetInnerColor());
}
