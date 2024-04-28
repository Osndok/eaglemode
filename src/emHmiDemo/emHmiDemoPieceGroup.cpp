//------------------------------------------------------------------------------
// emHmiDemoPieceGroup.cpp
//
// Copyright (C) 2012,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoPieceGroup.h>
#include <emHmiDemo/emHmiDemoPiece.h>


emHmiDemoPieceGroup::emHmiDemoPieceGroup(
	ParentArg parent, const emString & name,
	int rx, int ry, int rw, int rh, emColor color
)
	: emPanel(parent,name)
{
	RX=rx;
	RY=ry;
	RW=rw;
	RH=rh;
	Color=color;
	SetFocusable(false);
}


emHmiDemoPieceGroup::~emHmiDemoPieceGroup()
{
}


void emHmiDemoPieceGroup::SetColor(emColor color)
{
	emPanel * p;
	emHmiDemoPiece * ip;

	Color=color;
	for (p=GetFirstChild(); p; p=p->GetNext()) {
		ip=dynamic_cast<emHmiDemoPiece*>(p);
		if (ip) ip->SetColor(Color);
	}
}


void emHmiDemoPieceGroup::LayoutChildren()
{
	double fx,fy;
	emPanel * p;
	emHmiDemoPiece * dp;

	fx=1.0/RW;
	fy=GetHeight()/RH;
	for (p=GetFirstChild(); p; p=p->GetNext()) {
		dp=dynamic_cast<emHmiDemoPiece*>(p);
		if (dp) dp->Layout(
			(dp->GetRX()-RX)*fx,
			(dp->GetRY()-RY)*fy,
			dp->GetRW()*fx,
			dp->GetRH()*fy,
			GetParent()->GetCanvasColor()
		);
	}
}
