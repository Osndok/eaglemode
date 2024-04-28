//------------------------------------------------------------------------------
// emHmiDemoControls.cpp
//
// Copyright (C) 2012,2014-2015,2022,2018,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoControls.h>


emHmiDemoControls::emHmiDemoControls(
	ParentArg parent, const emString & name,
	int adCount, int buCount, int fiCount
)
	: emLinearLayout(parent,name)
{
	emLook look;
	char tmp[256];
	int i;

	AdCount=adCount;
	BuCount=buCount;
	FiCount=fiCount;

	SetVertical();

	SetChildTallness(0, 1.0/AdCount);
	SetChildTallness(1, 1.0/AdCount);
	SetChildTallness(
		2,
		3.0/4.0*(
			FiCount<=3 ? 1.0/FiCount :
			FiCount<=4 ? 1.0 :
			FiCount<=6 ? 2.0/3.0 :
			1.0
		)
	);

	Ad=new emHmiDemoAnalogDisplay*[AdCount];
	Bu=new emHmiDemoButton*[BuCount];
	Fi=new emHmiDemoFile*[FiCount];

	AdRL=new emRasterLayout(this,"ad");
	AdRL->SetPrefChildTallness(1.0);

	BuRL=new emRasterLayout(this,"bu");
	BuRL->SetPrefChildTallness(1.0);
	BuRL->SetSpace(0.2,0.4,0.4,0.2);

	FiRL=new emRasterLayout(this,"fi");
	FiRL->SetPrefChildTallness(3.0/4.0);

	look.SetBgColor(0);
	look.SetFgColor(0x00000099);
	look.SetOutputBgColor(0xF1EFECFF);

	if (AdCount>0) {
		Ad[0]=new emHmiDemoAnalogDisplay(AdRL,"ad0","RPM",0,10000,6445);
		Ad[0]->SetLook(look);
		Ad[0]->SetScaleMarkIntervals(1000,200);
		Ad[0]->AddColoredRange(0,7000,0x00CC00FF);
		Ad[0]->AddColoredRange(7000,8000,0xCCCC00FF);
		Ad[0]->AddColoredRange(8000,10000,0xCC0000FF);
		Ad[0]->SetRadix(1);
		Ad[0]->SetAnalogDigitsAfterRadix(0);
		Ad[0]->SetDigitalDigitsAfterRadix(0);
	}
	if (AdCount>1) {
		Ad[1]=new emHmiDemoAnalogDisplay(AdRL,"ad1","kW",0,6000,3473);
		Ad[1]->SetLook(look);
		Ad[1]->SetScaleMarkIntervals(1000,200);
		Ad[1]->AddColoredRange(0,4000,0x00CC00FF);
		Ad[1]->AddColoredRange(4000,5000,0xCCCC00FF);
		Ad[1]->AddColoredRange(5000,6000,0xCC0000FF);
		Ad[1]->SetRadix(1000);
		Ad[1]->SetAnalogDigitsAfterRadix(1);
		Ad[1]->SetDigitalDigitsAfterRadix(2);
	}
	if (AdCount>2) {
		Ad[2]=new emHmiDemoAnalogDisplay(AdRL,"ad2","kPa",0,1000,634);
		Ad[2]->SetLook(look);
		Ad[2]->SetScaleMarkIntervals(100,20);
		Ad[2]->AddColoredRange(0,600,0x00CC00FF);
		Ad[2]->AddColoredRange(600,800,0xCCCC00FF);
		Ad[2]->AddColoredRange(800,1000,0xCC0000FF);
		Ad[2]->SetRadix(1);
		Ad[2]->SetAnalogDigitsAfterRadix(0);
		Ad[2]->SetDigitalDigitsAfterRadix(0);
	}
	if (AdCount>3) {
		emMBState mbState;
		i=emEncodeChar(tmp,0xB0,&mbState);
		tmp[i]=0;
		strcat(tmp,"C");
		Ad[3]=new emHmiDemoAnalogDisplay(AdRL,"ad3",tmp,0,1000,723);
		Ad[3]->SetLook(look);
		Ad[3]->SetScaleMarkIntervals(100,20);
		Ad[3]->AddColoredRange(0,800,0x00CC00FF);
		Ad[3]->AddColoredRange(800,900,0xCCCC00FF);
		Ad[3]->AddColoredRange(900,1000,0xCC0000FF);
		Ad[3]->SetRadix(10);
		Ad[3]->SetAnalogDigitsAfterRadix(0);
		Ad[3]->SetDigitalDigitsAfterRadix(1);
	}
	if (AdCount>4) {
		Ad[4]=new emHmiDemoAnalogDisplay(AdRL,"ad4","pc/m",0,100,57);
		Ad[4]->SetLook(look);
		Ad[4]->SetScaleMarkIntervals(10,2);
		Ad[4]->AddColoredRange(0,70,0x00CC00FF);
		Ad[4]->AddColoredRange(70,80,0xCCCC00FF);
		Ad[4]->AddColoredRange(80,100,0xCC0000FF);
		Ad[4]->SetRadix(1);
		Ad[4]->SetAnalogDigitsAfterRadix(0);
		Ad[4]->SetDigitalDigitsAfterRadix(0);
	}

	if (BuCount>0) {
		look.SetOutputBgColor(0xFF4444FF);
		look.SetOutputFgColor(0x000000FF);
		Bu[0]=new emHmiDemoButton(BuRL,"bu0","Stop");
		Bu[0]->SetLook(look);
	}
	if (BuCount>1) {
		look.SetOutputBgColor(0x00FF00FF);
		look.SetOutputFgColor(0x000000FF);
		Bu[1]=new emHmiDemoButton(BuRL,"bu1","I");
		Bu[1]->SetLook(look);
	}
	if (BuCount>2) {
		look.SetOutputBgColor(0x00FF00FF);
		look.SetOutputFgColor(0x000000FF);
		Bu[2]=new emHmiDemoButton(BuRL,"bu2","II");
		Bu[2]->SetLook(look);
	}
	if (BuCount>3) {
		look.SetOutputBgColor(0x00FF00FF);
		look.SetOutputFgColor(0x000000FF);
		Bu[3]=new emHmiDemoButton(BuRL,"bu3","Auto");
		Bu[3]->SetLook(look);
	}
	if (BuCount>4) {
		look.SetOutputBgColor(0x3344FFFF);
		look.SetOutputFgColor(0x000000FF);
		Bu[4]=new emHmiDemoButton(BuRL,"bu4","Clean");
		Bu[4]->SetLook(look);
	}

	look.SetOutputBgColor(0xF1EFECFF);
	look.SetOutputFgColor(0x000000FF);
	if (FiCount>0) {
		Fi[0]=new emHmiDemoFile(
			FiRL,"fi0",
			EM_IDT_RES,"emHmiDemo","Media","graph1.pdf"
		);
		Fi[0]->SetLook(look);
	}
	if (FiCount>1) {
		Fi[1]=new emHmiDemoFile(
			FiRL,"fi1",
			EM_IDT_RES,"emHmiDemo","Media","graph2.pdf"
		);
		Fi[1]->SetLook(look);
	}
	if (FiCount>2) {
		Fi[2]=new emHmiDemoFile(
			FiRL,"fi2",
			EM_IDT_RES,"emHmiDemo","Media","document.pdf"
		);
		Fi[2]->SetLook(look);
	}
	if (FiCount>3) {
		Fi[3]=new emHmiDemoFile(
			FiRL,"fi3",
			EM_IDT_RES,"emHmiDemo","Media","table.pdf"
		);
		Fi[3]->SetLook(look);
	}
	if (FiCount>4) {
		Fi[4]=new emHmiDemoFile(
			FiRL,"fi4",
			EM_IDT_RES,"emHmiDemo","Media","circuit.pdf"
		);
		Fi[4]->SetLook(look);
	}
	if (FiCount>5) {
		Fi[5]=new emHmiDemoFile(
			FiRL,"fi5",
			EM_IDT_RES,"emHmiDemo","Media","table.pdf"
		);
		Fi[5]->SetLook(look);
	}

	SetState(0);
}


emHmiDemoControls::~emHmiDemoControls()
{
	delete [] Fi;
	delete [] Bu;
	delete [] Ad;
}


void emHmiDemoControls::SetState(int state)
{
	switch (state) {
	case 1:
		if (AdCount>0) {
			Ad[0]->SetValue(1670);
			Ad[0]->SetAnimation(1670,20,1.5);
		}
		if (AdCount>1) {
			Ad[1]->SetValue(1357);
			Ad[1]->SetAnimation(1357,33,0.4);
		}
		if (AdCount>2) {
			Ad[2]->SetValue(273);
			Ad[2]->SetAnimation(273,25,1.5);
		}
		if (AdCount>3) {
			Ad[3]->SetValue(674);
			Ad[3]->SetAnimation(674,0,0);
		}
		if (AdCount>4) {
			Ad[4]->SetValue(24);
			Ad[4]->SetAnimation(24,0,0);
		}
		if (BuCount>0) {
			Bu[0]->SetChecked(false);
			Bu[0]->SetAnimation(0);
		}
		if (BuCount>1) {
			Bu[1]->SetChecked(false);
			Bu[1]->SetAnimation(1);
		}
		if (BuCount>2) {
			Bu[2]->SetChecked(false);
			Bu[2]->SetAnimation(0);
		}
		if (BuCount>3) {
			Bu[3]->SetChecked(true);
			Bu[3]->SetAnimation(0);
		}
		if (BuCount>4) {
			Bu[4]->SetChecked(true);
			Bu[4]->SetAnimation(1);
		}
		break;
	case 2:
		if (AdCount>0) {
			Ad[0]->SetValue(6445);
			Ad[0]->SetAnimation(6445,40,5.0);
		}
		if (AdCount>1) {
			Ad[1]->SetValue(3473);
			Ad[1]->SetAnimation(3473,77,0.4);
		}
		if (AdCount>2) {
			Ad[2]->SetValue(634);
			Ad[2]->SetAnimation(634,40,2.0);
		}
		if (AdCount>3) {
			Ad[3]->SetValue(723);
			Ad[3]->SetAnimation(723,0,0);
		}
		if (AdCount>4) {
			Ad[4]->SetValue(57);
			Ad[4]->SetAnimation(57,0,0);
		}
		if (BuCount>0) {
			Bu[0]->SetChecked(false);
			Bu[0]->SetAnimation(0);
		}
		if (BuCount>1) {
			Bu[1]->SetChecked(false);
			Bu[1]->SetAnimation(0);
		}
		if (BuCount>2) {
			Bu[2]->SetChecked(false);
			Bu[2]->SetAnimation(1);
		}
		if (BuCount>3) {
			Bu[3]->SetChecked(true);
			Bu[3]->SetAnimation(0);
		}
		if (BuCount>4) {
			Bu[4]->SetChecked(true);
			Bu[4]->SetAnimation(1);
		}
		break;
	default:
		if (AdCount>0) {
			Ad[0]->SetValue(0);
			Ad[0]->SetAnimation(0,0,0);
		}
		if (AdCount>1) {
			Ad[1]->SetValue(0);
			Ad[1]->SetAnimation(0,0,0);
		}
		if (AdCount>2) {
			Ad[2]->SetValue(134);
			Ad[2]->SetAnimation(134,0,0);
		}
		if (AdCount>3) {
			Ad[3]->SetValue(465);
			Ad[3]->SetAnimation(465,0,0);
		}
		if (AdCount>4) {
			Ad[4]->SetValue(0);
			Ad[4]->SetAnimation(0,0,0);
		}
		if (BuCount>0) {
			Bu[0]->SetChecked(false);
			Bu[0]->SetAnimation(0);
		}
		if (BuCount>1) {
			Bu[1]->SetChecked(false);
			Bu[1]->SetAnimation(0);
		}
		if (BuCount>2) {
			Bu[2]->SetChecked(false);
			Bu[2]->SetAnimation(0);
		}
		if (BuCount>3) {
			Bu[3]->SetChecked(true);
			Bu[3]->SetAnimation(0);
		}
		if (BuCount>4) {
			Bu[4]->SetChecked(false);
			Bu[4]->SetAnimation(0);
		}
		break;
	}
}
