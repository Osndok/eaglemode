//------------------------------------------------------------------------------
// emHmiDemoMonitors.cpp
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

#include <emHmiDemo/emHmiDemoMonitors.h>


emHmiDemoMonitors::emHmiDemoMonitors(
	ParentArg parent, const emString & name,
	int caCount
)
	: emRasterLayout(parent,name)
{
	emLook look;
	emRef<emFpPluginList> FpPluginList;

	FpPluginList=emFpPluginList::Acquire(GetRootContext());

	SetPrefChildTallness(3.0/4.0);

	Ca=new emHmiDemoFile*[caCount];

	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	look.SetOutputBgColor(0x444444FF);

	if (caCount>0) {
		Ca[0]=new emHmiDemoFile(
			this,"ca0",
			EM_IDT_RES,"emHmiDemo","Monitor","pies.gif"
		);
		Ca[0]->SetLook(look);
		Ca[0]->SetCaption("Camera 1");
		Ca[0]->SetBorderScaling(0.5);
	}
	if (caCount>1) {
		Ca[1]=new emHmiDemoFile(
			this,"ca1",
			EM_IDT_RES,"emHmiDemo","Monitor","off.gif"
		);
		Ca[1]->SetLook(look);
		Ca[1]->SetCaption("Camera 2");
		Ca[1]->SetBorderScaling(0.5);
	}
	if (caCount>2) {
		Ca[2]=new emHmiDemoFile(
			this,"ca2",
			EM_IDT_RES,"emHmiDemo","Monitor","noise.gif"
		);
		Ca[2]->SetLook(look);
		Ca[2]->SetCaption("Camera 3");
		Ca[2]->SetBorderScaling(0.5);
	}
}


emHmiDemoMonitors::~emHmiDemoMonitors()
{
}
