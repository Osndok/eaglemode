//------------------------------------------------------------------------------
// emHmiDemoMixer.cpp
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

#include <emHmiDemo/emHmiDemoMixer.h>


emHmiDemoMixer::emHmiDemoMixer(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, int state,
	emColor color
)
	: emHmiDemoPiece(parent,name,rx,ry,rw,rh,"19",color),
	State(state),
	LbName(NULL),
	FlowIndicator(NULL),
	FillIndicator(NULL),
	Controls(NULL)
{
	SetFocusable(true);
}


emHmiDemoMixer::~emHmiDemoMixer()
{
}


void emHmiDemoMixer::AutoExpand()
{
	emHmiDemoPiece::AutoExpand();

	LbName=new emLabel(this,"name",GetName());
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	LbName->SetLook(look);

	FlowIndicator = new emHmiDemoFlowIndicator(this,"flow",0.0,2);

	FillIndicator = new emHmiDemoFillIndicator(this,"fill",0.5);

	Controls = new emHmiDemoControls(this,"controls",4,4,4);

	switch (State) {
	case 0:
		FlowIndicator->SetRPM(0.0);
		FillIndicator->SetFill(0.3);
		Controls->SetState(0);
		break;
	case 1:
		FlowIndicator->SetRPM(15.0);
		FillIndicator->SetFill(0.7);
		Controls->SetState(2);
		break;
	default:
		break;
	}
}


void emHmiDemoMixer::AutoShrink()
{
	emHmiDemoPiece::AutoShrink();
	LbName=NULL;
	FlowIndicator=NULL;
	FillIndicator=NULL;
	Controls=NULL;
}


void emHmiDemoMixer::LayoutChildren()
{
	if (LbName) LbName->Layout(0.05,0.05,0.4,0.11,GetInnerColor());
	if (FlowIndicator) FlowIndicator->Layout(0.37,0.55,0.26,0.26,GetInnerColor());
	if (FillIndicator) FillIndicator->Layout(0.75,0.07,0.12,0.55,GetInnerColor());
	if (Controls) Controls->Layout(0.05,0.19,0.31,0.39,GetInnerColor());
}
