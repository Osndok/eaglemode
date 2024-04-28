//------------------------------------------------------------------------------
// emHmiDemoPanel.cpp
//
// Copyright (C) 2012,2016,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoPanel.h>
#include <emHmiDemo/emHmiDemoCone.h>
#include <emHmiDemo/emHmiDemoConveyor.h>
#include <emHmiDemo/emHmiDemoMixer.h>
#include <emHmiDemo/emHmiDemoPieceGroup.h>
#include <emHmiDemo/emHmiDemoPump.h>
#include <emHmiDemo/emHmiDemoStation.h>
#include <emHmiDemo/emHmiDemoTank.h>


emHmiDemoPanel::emHmiDemoPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
	SetAutoExpansionThreshold(1200.0);
}


emHmiDemoPanel::~emHmiDemoPanel()
{
}


bool emHmiDemoPanel::IsOpaque() const
{
	return false;
}


void emHmiDemoPanel::AutoExpand()
{
	emHmiDemoPieceGroup * g01=new emHmiDemoPieceGroup(this,"01",3,3,26,32);
	new emHmiDemoTank (g01,"T011",  3,  3, 7, 9,6);
	new emHmiDemoPiece(g01,"02"  ,  6, 12, 1, 6,"01");
	new emHmiDemoPump (g01,"P011",  4, 18, 5, 4,0);
	new emHmiDemoPiece(g01,"04"  ,  6, 22, 1, 5,"01");
	new emHmiDemoPiece(g01,"05"  ,  6, 27, 2, 2,"04");
	new emHmiDemoPiece(g01,"06"  ,  8, 28,19, 1,"00");
	new emHmiDemoPiece(g01,"07"  , 27, 28, 2, 2,"03");
	new emHmiDemoPiece(g01,"08"  , 28, 30, 1, 4,"01");
	new emHmiDemoPiece(g01,"09"  , 28, 34, 1, 1,"14");
	g01->SetColor(0xFFFF50FF);

	emHmiDemoPieceGroup * g02=new emHmiDemoPieceGroup(this,"02",17,3,15,32);
	new emHmiDemoTank (g02,"T021", 17,  3, 7, 9,8);
	new emHmiDemoPiece(g02,"02"  , 20, 12, 1, 6,"01");
	new emHmiDemoPump (g02,"P021", 18, 18, 5, 4,0);
	new emHmiDemoPiece(g02,"04"  , 20, 22, 1, 2,"01");
	new emHmiDemoPiece(g02,"05"  , 20, 24, 2, 2,"04");
	new emHmiDemoPiece(g02,"06"  , 22, 25, 8, 1,"00");
	new emHmiDemoPiece(g02,"07"  , 30, 25, 2, 2,"03");
	new emHmiDemoPiece(g02,"08"  , 31, 27, 1, 7,"01");
	new emHmiDemoPiece(g02,"09"  , 31, 34, 1, 1,"14");
	g02->SetColor(0x50FFFFFF);

	emHmiDemoPieceGroup * g03=new emHmiDemoPieceGroup(this,"03",31,3,7,32);
	new emHmiDemoTank (g03,"T031", 31,  3, 7, 9,7);
	new emHmiDemoPiece(g03,"02"  , 34, 12, 1, 6,"01");
	new emHmiDemoPump (g03,"P031", 32, 18, 5, 4,0);
	new emHmiDemoPiece(g03,"04"  , 34, 22, 1,12,"01");
	new emHmiDemoPiece(g03,"05"  , 34, 34, 1, 1,"14");
	g03->SetColor(0xFF50FFFF);

	emHmiDemoPieceGroup * g04=new emHmiDemoPieceGroup(this,"04",38,3,18,32);
	new emHmiDemoTank (g04,"T041", 45,  3, 7, 9,8);
	new emHmiDemoPiece(g04,"02"  , 48, 12, 1, 2,"01");
	new emHmiDemoPiece(g04,"03"  , 48, 14, 1, 1,"07");
	new emHmiDemoPiece(g04,"04"  , 45, 14, 3, 1,"00");
	new emHmiDemoPiece(g04,"05"  , 43, 14, 2, 2,"02");
	new emHmiDemoPiece(g04,"06"  , 43, 16, 1, 2,"01");
	new emHmiDemoPump (g04,"P041", 41, 18, 5, 4,0);
	new emHmiDemoPiece(g04,"08"  , 43, 22, 1, 2,"01");
	new emHmiDemoPiece(g04,"09"  , 42, 24, 2, 2,"05");
	new emHmiDemoPiece(g04,"10"  , 40, 25, 2, 1,"00");
	new emHmiDemoPiece(g04,"11"  , 38, 25, 2, 2,"02");
	new emHmiDemoPiece(g04,"12"  , 38, 27, 1, 7,"01");
	new emHmiDemoPiece(g04,"13"  , 38, 34, 1, 1,"14");
	new emHmiDemoPiece(g04,"14"  , 49, 14, 3, 1,"00");
	new emHmiDemoPiece(g04,"15"  , 52, 14, 2, 2,"03");
	new emHmiDemoPiece(g04,"16"  , 53, 16, 1, 2,"01");
	new emHmiDemoPump (g04,"P042", 51, 18, 5, 4,2);
	new emHmiDemoPiece(g04,"18"  , 53, 22, 1, 2,"01");
	new emHmiDemoPiece(g04,"19"  , 52, 24, 2, 2,"05");
	new emHmiDemoPiece(g04,"20"  , 49, 25, 3, 1,"00");
	new emHmiDemoPiece(g04,"21"  , 47, 25, 2, 2,"02");
	new emHmiDemoPiece(g04,"22"  , 47, 27, 1, 7,"01");
	new emHmiDemoPiece(g04,"23"  , 47, 34, 1, 1,"14");
	g04->SetColor(0x5050FFFF);

	emHmiDemoPieceGroup * g05=new emHmiDemoPieceGroup(this,"05",50,3,16,32);
	new emHmiDemoTank (g05,"T051", 59,  3, 7, 9,0);
	new emHmiDemoPiece(g05,"02"  , 62, 12, 1, 6,"01");
	new emHmiDemoPump (g05,"P051", 60, 18, 5, 4,2);
	new emHmiDemoPiece(g05,"04"  , 62, 22, 1, 4,"01");
	new emHmiDemoPiece(g05,"05"  , 61, 26, 2, 2,"05");
	new emHmiDemoPiece(g05,"06"  , 52, 27, 9, 1,"00");
	new emHmiDemoPiece(g05,"07"  , 50, 27, 2, 2,"02");
	new emHmiDemoPiece(g05,"08"  , 50, 29, 1, 5,"01");
	new emHmiDemoPiece(g05,"09"  , 50, 34, 1, 1,"14");
	g05->SetColor(0x50FF50FF);

	emHmiDemoPieceGroup * g06=new emHmiDemoPieceGroup(this,"06",53,3,27,32);
	new emHmiDemoTank (g06,"T061", 73,  3, 7, 9,4);
	new emHmiDemoPiece(g06,"02"  , 76, 12, 1, 6,"01");
	new emHmiDemoPump (g06,"P061", 74, 18, 5, 4,2);
	new emHmiDemoPiece(g06,"04"  , 76, 22, 1, 6,"01");
	new emHmiDemoPiece(g06,"05"  , 75, 28, 2, 2,"05");
	new emHmiDemoPiece(g06,"06"  , 55, 29,20, 1,"00");
	new emHmiDemoPiece(g06,"07"  , 53, 29, 2, 2,"02");
	new emHmiDemoPiece(g06,"08"  , 53, 31, 1, 3,"01");
	new emHmiDemoPiece(g06,"09"  , 53, 34, 1, 1,"14");
	g06->SetColor(0xFF5050FF);

	emHmiDemoPieceGroup * g07=new emHmiDemoPieceGroup(this,"07",56,3,38,32);
	new emHmiDemoTank (g07,"T071", 87,  3, 7, 9,9);
	new emHmiDemoPiece(g07,"02"  , 90, 12, 1, 6,"01");
	new emHmiDemoPump (g07,"P071", 88, 18, 5, 4,2);
	new emHmiDemoPiece(g07,"04"  , 90, 22, 1, 8,"01");
	new emHmiDemoPiece(g07,"05"  , 89, 30, 2, 2,"05");
	new emHmiDemoPiece(g07,"06"  , 58, 31,31, 1,"00");
	new emHmiDemoPiece(g07,"07"  , 56, 31, 2, 2,"02");
	new emHmiDemoPiece(g07,"08"  , 56, 33, 1, 1,"01");
	new emHmiDemoPiece(g07,"09"  , 56, 34, 1, 1,"14");
	g07->SetColor(0xFFCC50FF);

	emHmiDemoPieceGroup * g08=new emHmiDemoPieceGroup(this,"08",86,3,22,59);
	new emHmiDemoTank (g08,"T081",101,  3, 7, 9,6);
	new emHmiDemoPiece(g08,"02"  ,104, 12, 1, 6,"01");
	new emHmiDemoPump (g08,"P081",102, 18, 5, 4,1);
	new emHmiDemoPiece(g08,"04"  ,104, 22, 1,17,"01");
	new emHmiDemoPiece(g08,"05"  ,103, 39, 2, 2,"05");
	new emHmiDemoPiece(g08,"06"  , 88, 40,15, 1,"00");
	new emHmiDemoPiece(g08,"07"  , 86, 40, 2, 2,"02");
	new emHmiDemoPiece(g08,"08"  , 86, 42, 1,19,"01");
	new emHmiDemoPiece(g08,"09"  , 86, 61, 1, 1,"14");
	g08->SetColor(0xCC50FFFF);

	emHmiDemoPieceGroup * g09=new emHmiDemoPieceGroup(this,"09",90,3,32,59);
	new emHmiDemoTank (g09,"T091",115,  3, 7, 9,3);
	new emHmiDemoPiece(g09,"02"  ,118, 12, 1, 6,"01");
	new emHmiDemoPump (g09,"P091",116, 18, 5, 4,1);
	new emHmiDemoPiece(g09,"04"  ,118, 22, 1,26,"01");
	new emHmiDemoPiece(g09,"05"  ,117, 48, 2, 2,"05");
	new emHmiDemoPiece(g09,"06"  , 92, 49,25, 1,"00");
	new emHmiDemoPiece(g09,"07"  , 90, 49, 2, 2,"02");
	new emHmiDemoPiece(g09,"08"  , 90, 51, 1,10,"01");
	new emHmiDemoPiece(g09,"09"  , 90, 61, 1, 1,"14");
	g09->SetColor(0xFF50CCFF);

	emHmiDemoPieceGroup * g10=new emHmiDemoPieceGroup(this,"10",3,34,37,28);
	new emHmiDemoMixer(g10,"M101", 27, 35,13,12,0);
	new emHmiDemoPiece(g10,"02"  , 33, 47, 1, 3,"01");
	new emHmiDemoPiece(g10,"03"  , 32, 50, 2, 2,"05");
	new emHmiDemoPiece(g10,"04"  , 23, 51, 9, 1,"00");
	new emHmiDemoPiece(g10,"05"  , 21, 50, 2, 2,"04");
	new emHmiDemoPiece(g10,"06"  , 21, 45, 1, 5,"01");
	new emHmiDemoPump (g10,"P101", 19, 41, 5, 4,4);
	new emHmiDemoPiece(g10,"08"  , 21, 36, 1, 5,"01");
	new emHmiDemoPiece(g10,"09"  , 20, 34, 2, 2,"03");
	new emHmiDemoPiece(g10,"10"  , 11, 34, 9, 1,"00");
	new emHmiDemoPiece(g10,"11"  ,  9, 34, 2, 2,"02");
	new emHmiDemoPiece(g10,"12"  ,  9, 36, 1, 2,"01");
	new emHmiDemoPiece(g10,"13"  ,  9, 38, 1, 1,"14");
	new emHmiDemoCone (g10,"R101",  3, 39,13,11,5);
	new emHmiDemoPiece(g10,"15"  ,  9, 50, 1, 4,"01");
	new emHmiDemoPump (g10,"P102",  7, 54, 5, 4,1);
	new emHmiDemoPiece(g10,"17"  ,  9, 58, 1, 3,"01");
	new emHmiDemoPiece(g10,"18"  ,  9, 61, 1, 1,"14");
	g10->SetColor(0xFFCC50FF);

	emHmiDemoPieceGroup * g11=new emHmiDemoPieceGroup(this,"11",45,34,37,28);
	new emHmiDemoMixer(g11,"M111", 45, 35,13,12,1);
	new emHmiDemoPiece(g11,"02"  , 51, 47, 1, 3,"01");
	new emHmiDemoPiece(g11,"03"  , 51, 50, 2, 2,"04");
	new emHmiDemoPiece(g11,"04"  , 53, 51, 9, 1,"00");
	new emHmiDemoPiece(g11,"05"  , 62, 50, 2, 2,"05");
	new emHmiDemoPiece(g11,"06"  , 63, 45, 1, 5,"01");
	new emHmiDemoPump (g11,"P111", 61, 41, 5, 4,0);
	new emHmiDemoPiece(g11,"08"  , 63, 36, 1, 5,"01");
	new emHmiDemoPiece(g11,"09"  , 63, 34, 2, 2,"02");
	new emHmiDemoPiece(g11,"10"  , 65, 34, 9, 1,"00");
	new emHmiDemoPiece(g11,"11"  , 74, 34, 2, 2,"03");
	new emHmiDemoPiece(g11,"12"  , 75, 36, 1, 2,"01");
	new emHmiDemoPiece(g11,"13"  , 75, 38, 1, 1,"14");
	new emHmiDemoCone (g11,"R111", 69, 39,13,11,8);
	new emHmiDemoPiece(g11,"15"  , 75, 50, 1, 4,"01");
	new emHmiDemoPump (g11,"P112", 73, 54, 5, 4,1);
	new emHmiDemoPiece(g11,"17"  , 75, 58, 1, 3,"01");
	new emHmiDemoPiece(g11,"18"  , 75, 61, 1, 1,"14");
	g11->SetColor(0x50CCFFFF);

	emHmiDemoPieceGroup * g12=new emHmiDemoPieceGroup(this,"12",3,62,123,10);
	new emHmiDemoStation (g12,"S121",  3, 62,13,10,1);
	new emHmiDemoConveyor(g12,"C121", 16, 66, 9, 3,1);
	new emHmiDemoStation (g12,"S122", 25, 62,13,10,1);
	new emHmiDemoConveyor(g12,"C122", 38, 66, 9, 3,1);
	new emHmiDemoStation (g12,"S123", 47, 62,13,10,1);
	new emHmiDemoConveyor(g12,"C123", 60, 66, 9, 3,1);
	new emHmiDemoStation (g12,"S124", 69, 62,13,10,1);
	new emHmiDemoStation (g12,"S125", 82, 62,13,10,1);
	new emHmiDemoConveyor(g12,"C125", 95, 66, 9, 3,1);
	new emHmiDemoStation (g12,"S126",104, 62,13,10,1);
	new emHmiDemoConveyor(g12,"C126",117, 66, 9, 3,1);
	g12->SetColor(0x50FFCCFF);

	emLabel * lbl=new emLabel(
		this,"lbl",
		"emHmiDemo\n"
		"-\n"
		"Demo of a zoomable HMI for something like a pie factory\n"
		"\n"
		"This has no functionality. It's just a picture, a demonstration of\n"
		"what a zoomable human machine interface could look and feel like."
	);
	emLook look;
	look.SetBgColor(0);
	look.SetFgColor(0x000000FF);
	lbl->SetLook(look);
	lbl->SetCaptionAlignment(EM_ALIGN_CENTER);
	lbl->SetLabelAlignment(EM_ALIGN_CENTER);
}


void emHmiDemoPanel::LayoutChildren()
{
	double w,h,rw,rh,px,py,fx,fy;
	emPanel * p;
	emHmiDemoPieceGroup * dpg;
	emHmiDemoPiece * dp;
	emLabel * lbl;

	rw=128;
	rh=75;
	w=1.0;
	h=GetHeight();
	fx=emMin(w/rw,h/rh);
	fy=fx;
	px=(w-rw*fx)*0.5;
	py=(h-rh*fy)*0.5;
	for (p=GetFirstChild(); p; p=p->GetNext()) {
		dpg=dynamic_cast<emHmiDemoPieceGroup*>(p);
		if (dpg) {
			dpg->Layout(
				px+dpg->GetRX()*fx,
				py+dpg->GetRY()*fy,
				dpg->GetRW()*fx,
				dpg->GetRH()*fy
			);
			continue;
		}
		dp=dynamic_cast<emHmiDemoPiece*>(p);
		if (dp) {
			dp->Layout(
				px+dp->GetRX()*fx,
				py+dp->GetRY()*fy,
				dp->GetRW()*fx,
				dp->GetRH()*fy,
				GetCanvasColor()
			);
			continue;
		}
		lbl=dynamic_cast<emLabel*>(p);
		if (lbl) {
			lbl->Layout(
				px+32*fx,
				py+55*fy,
				21*fx,
				2*fy,
				GetCanvasColor()
			);
			continue;
		}
	}
}
