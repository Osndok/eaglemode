//------------------------------------------------------------------------------
// emHmiDemoTank.cpp
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

#include <emHmiDemo/emHmiDemoTank.h>


emHmiDemoTank::emHmiDemoTank(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, int state,
	emColor color
)
	: emHmiDemoPiece(parent,name,rx,ry,rw,rh,"15",color),
	State(state),
	LbName(NULL),
	FillIndicator(NULL)
{
	SetFocusable(true);
}


emHmiDemoTank::~emHmiDemoTank()
{
}


void emHmiDemoTank::AutoExpand()
{
	emHmiDemoPiece::AutoExpand();

	LbName=new emLabel(this,"name",GetName());
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	LbName->SetLook(look);

	FillIndicator = new emHmiDemoFillIndicator(this,"fill",0.5);
	FillIndicator->SetFill(State/10.0+0.08);
}


void emHmiDemoTank::AutoShrink()
{
	emHmiDemoPiece::AutoShrink();
	LbName=NULL;
	FillIndicator=NULL;
}


void emHmiDemoTank::LayoutChildren()
{
	if (LbName) LbName->Layout(0.1,0.18,0.5,0.14,GetInnerColor());
	if (FillIndicator) FillIndicator->Layout(0.7,0.18,0.16,0.89,GetInnerColor());
}
