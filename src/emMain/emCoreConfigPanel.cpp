//------------------------------------------------------------------------------
// emCoreConfigPanel.cpp
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#include <emMain/emCoreConfigPanel.h>


emCoreConfigPanel::emCoreConfigPanel(
	ParentArg parent, const emString & name
)
	: emTkGroup(parent,name,
			"Preferences",
			"This panel provides some user settings. Internally, this\n"
			"is also called the emCore Configuration."
	)
{
	Config=emCoreConfig::Acquire(GetRootContext());
	ResetButton=NULL;
	EnableAutoExpansion();
}


emCoreConfigPanel::~emCoreConfigPanel()
{
}


bool emCoreConfigPanel::Cycle()
{
	bool busy;

	busy=emTkGroup::Cycle();
	if (ResetButton && IsSignaled(ResetButton->GetClickSignal())) {
		Config->SetToDefault();
		Config->Save();
	}
	return busy;
}


void emCoreConfigPanel::AutoExpand()
{
	emTkTiling * tl1, * tl2, * tl3;

	emTkGroup::AutoExpand();

	SetFixedRowCount(2);
	SetPrefChildTallness(0.6);
	SetPrefChildTallness(0.05,-1);
	SetSpace(0.01,0.1,0.01,0.1,0.01,0.0);

	tl1=new emTkTiling(this,"tl1");
	tl1->SetPrefChildTallness(0.8);
	tl1->SetInnerSpace(0.06,0.1);

	new MouseGroup(tl1,"mouse",Config);
	tl2=new emTkTiling(tl1,"tl2");
	tl2->SetInnerSpace(0.1,0.2);
	new KBGroup(tl2,"keyboard",Config);
	new MaxMemTunnel(tl2,"maxmem",Config);

	tl3=new emTkTiling(this,"tl3");
	tl3->SetChildTallness(0.2);
	tl3->SetAlignment(EM_ALIGN_BOTTOM_RIGHT);
	ResetButton=new emTkButton(tl3,"reset","Reset To Defaults");
	ResetButton->SetNoEOI();
	AddWakeUpSignal(ResetButton->GetClickSignal());
}


void emCoreConfigPanel::AutoShrink()
{
	emTkGroup::AutoShrink();
	ResetButton=NULL;
}


emCoreConfigPanel::SpeedFacField::SpeedFacField(
	ParentArg parent, const emString & name,
	const emString & caption, const emString & description,
	const emImage & icon, emCoreConfig * config, emDoubleRec * rec
)
	: emTkScalarField(
		parent,name,caption,description,icon,
		-200,200,0,true
	),
	emRecListener(rec),
	Config(config)
{
	ValOut=0;
	SetScaleMarkIntervals(100,10,0);
	SetBorderScaling(1.5);
	UpdateValue();
}


emCoreConfigPanel::SpeedFacField::~SpeedFacField()
{
}


void emCoreConfigPanel::SpeedFacField::TextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval
) const
{
	const char * str;

	if (markInterval>=100) {
		switch ((int)value) {
			case -200: str="1/4"; break;
			case -100: str="1/2"; break;
			case    0: str="1"; break;
			case  100: str="2"; break;
			case  200: str="4"; break;
			default: str="?";
		}
		snprintf(buf,bufSize,"%s",str);
	}
	else if (markInterval>=10) {
		snprintf(buf,bufSize,"%.2f",pow(2.0,value/100.0));
	}
	else {
		snprintf(buf,bufSize,"%.3f",pow(2.0,value/100.0));
	}
}


void emCoreConfigPanel::SpeedFacField::ValueChanged()
{
	emDoubleRec * rec;
	double d;
	emInt64 val;

	val=GetValue();
	if (ValOut==val) return;
	ValOut=val;
	rec=(emDoubleRec*)GetListenedRec();
	if (!rec) return;
	d=pow(2.0,val/100.0);
	if (rec->Get()!=d) {
		rec->Set(d);
		if (Config) Config->Save();
	}
}


void emCoreConfigPanel::SpeedFacField::OnRecChanged()
{
	UpdateValue();
}


void emCoreConfigPanel::SpeedFacField::UpdateValue()
{
	emDoubleRec * rec;
	double d;

	rec=(emDoubleRec*)GetListenedRec();
	if (!rec) return;
	d=log(rec->Get())/log(2.0)*100.0;
	if (d>=0.0) d+=0.5; else d-=0.5;
	ValOut=(emInt64)d;
	SetValue(ValOut);
}


emCoreConfigPanel::SpeedFacGroup::SpeedFacGroup(
	ParentArg parent, const emString & name,
	const emString & caption, emCoreConfig * config,
	emDoubleRec * normalRec, const emString & normalCaption,
	emDoubleRec * fineRec, const emString & fineCaption
)
	: emTkGroup(parent,name,caption),
	Config(config)
{
	NormalRec=normalRec;
	NormalCaption=normalCaption;
	FineRec=fineRec;
	FineCaption=fineCaption;
	EnableAutoExpansion();
	SetFixedColumnCount(1);
	SetBorderScaling(4.0);
}


emCoreConfigPanel::SpeedFacGroup::~SpeedFacGroup()
{
}


void emCoreConfigPanel::SpeedFacGroup::AutoExpand()
{
	emTkGroup::AutoExpand();
	new SpeedFacField(
		this,"normal",NormalCaption,emString(),emImage(),
		Config,NormalRec
	);
	new SpeedFacField(
		this,"fine",FineCaption,emString(),emImage(),
		Config,FineRec
	);
}


emCoreConfigPanel::MouseMiscGroup::MouseMiscGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emTkGroup(parent,name,"Miscellaneous Mouse Settings"),
	emRecListener(config),
	Config(config)
{
	StickBox=NULL;
	EmuBox=NULL;
	PanBox=NULL;
	EnableAutoExpansion();
	SetBorderScaling(4);
	SetPrefChildTallness(0.1);
}


emCoreConfigPanel::MouseMiscGroup::~MouseMiscGroup()
{
}


void emCoreConfigPanel::MouseMiscGroup::OnRecChanged()
{
	UpdateOutput();
}


bool emCoreConfigPanel::MouseMiscGroup::Cycle()
{
	bool busy;

	busy=emTkGroup::Cycle();

	if (StickBox && IsSignaled(StickBox->GetClickSignal())) {
		Config->StickMouseWhenNavigating.Invert();
		Config->Save();
	}

	if (EmuBox && IsSignaled(EmuBox->GetClickSignal())) {
		Config->EmulateMiddleButton.Invert();
		Config->Save();
	}

	if (PanBox && IsSignaled(PanBox->GetClickSignal())) {
		Config->PanFunction.Invert();
		Config->Save();
	}

	return busy;
}


void emCoreConfigPanel::MouseMiscGroup::AutoExpand()
{
	emTkGroup::AutoExpand();
	StickBox=new emTkCheckBox(
		this,"stick","Stick Mouse When Navigating",
		"Whether to keep the mouse pointer at its place while zooming\n"
		"or scrolling with the mouse (except for pan function)."
	);
	EmuBox=new emTkCheckBox(
		this,"emu","Alt Key As Middle Button",
		"Whether to emulate the middle button by the ALT key."
	);
	PanBox=new emTkCheckBox(
		this,"pan","Reverse Scrolling (Pan)",
		"Whether to reverse the direction of scrolling with the\n"
		"mouse. It's the pan function: drag and drop the canvas."
	);
	StickBox->SetNoEOI();
	EmuBox->SetNoEOI();
	PanBox->SetNoEOI();
	AddWakeUpSignal(StickBox->GetClickSignal());
	AddWakeUpSignal(EmuBox->GetClickSignal());
	AddWakeUpSignal(PanBox->GetClickSignal());
	UpdateOutput();
}


void emCoreConfigPanel::MouseMiscGroup::AutoShrink()
{
	emTkGroup::AutoShrink();
	StickBox=NULL;
	EmuBox=NULL;
	PanBox=NULL;
}


void emCoreConfigPanel::MouseMiscGroup::UpdateOutput()
{
	if (StickBox) StickBox->SetChecked(Config->StickMouseWhenNavigating);
	if (EmuBox) EmuBox->SetChecked(Config->EmulateMiddleButton);
	if (PanBox) PanBox->SetChecked(Config->PanFunction);
}


emCoreConfigPanel::MouseGroup::MouseGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emTkGroup(parent,name,"Mouse Navigation"),
	Config(config)
{
	EnableAutoExpansion();
	SetPrefChildTallness(0.4);
	SetBorderScaling(2.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::MouseGroup::~MouseGroup()
{
}


void emCoreConfigPanel::MouseGroup::AutoExpand()
{
	emTkGroup::AutoExpand();
	new SpeedFacGroup(
		this,"wheelzoom","Zooming By Mouse Wheel",
		Config,
		&Config->WheelZoomSpeedFactor,
		"Speed factor for normal zooming by mouse wheel",
		&Config->WheelFineZoomSpeedFactor,
		"Speed factor for fine zooming by mouse wheel"
	);
	new SpeedFacGroup(
		this,"zoom","Zooming By Mouse",
		Config,
		&Config->MouseZoomSpeedFactor,
		"Speed factor for normal zooming by mouse",
		&Config->MouseFineZoomSpeedFactor,
		"Speed factor for fine zooming by mouse"
	);
	new SpeedFacGroup(
		this,"scroll","Scrolling By Mouse",
		Config,
		&Config->MouseScrollSpeedFactor,
		"Speed factor for normal scrolling by mouse",
		&Config->MouseFineScrollSpeedFactor,
		"Speed factor for fine scrolling by mouse"
	);
	new MouseMiscGroup(this,"misc",Config);
}


emCoreConfigPanel::KBGroup::KBGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emTkGroup(parent,name,"Keyboard Navigation"),
	Config(config)
{
	EnableAutoExpansion();
	SetPrefChildTallness(0.4);
	SetBorderScaling(4.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::KBGroup::~KBGroup()
{
}


void emCoreConfigPanel::KBGroup::AutoExpand()
{
	emTkGroup::AutoExpand();
	new SpeedFacGroup(
		this,"zoom","Zooming By Keyboard",
		Config,
		&Config->KeyboardZoomSpeedFactor,
		"Speed factor for normal zooming by keyboard",
		&Config->KeyboardFineZoomSpeedFactor,
		"Speed factor for fine zooming by keyboard"
	);
	new SpeedFacGroup(
		this,"scroll","Scrolling By Keyboard",
		Config,
		&Config->KeyboardScrollSpeedFactor,
		"Speed factor for normal scrolling by keyboard",
		&Config->KeyboardFineScrollSpeedFactor,
		"Speed factor for fine scrolling by keyboard"
	);
}


emCoreConfigPanel::MaxMemGroup::MaxMemGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emTkGroup(parent,name,"Max Megabytes Per View"),
	emRecListener(&config->MaxMegabytesPerView),
	Config(config)
{
	MemField=NULL;
	ValOut=0;
	EnableAutoExpansion();
	SetFixedColumnCount(1);
	SetPrefChildTallness(0.5);
	SetPrefChildTallness(0.1,-1);
}


emCoreConfigPanel::MaxMemGroup::~MaxMemGroup()
{
}


void emCoreConfigPanel::MaxMemGroup::OnRecChanged()
{
	UpdateOutput();
}


bool emCoreConfigPanel::MaxMemGroup::Cycle()
{
	emInt64 v;
	double d;
	bool busy;

	busy=emTkGroup::Cycle();
	if (MemField && IsSignaled(MemField->GetValueSignal())) {
		v=MemField->GetValue();
		if (ValOut!=v) {
			d=pow(2.0,((double)v)/100.0);
			Config->MaxMegabytesPerView.Set((int)(d+0.5));
			Config->Save();
		}
	}
	return busy;
}


void emCoreConfigPanel::MaxMemGroup::AutoExpand()
{
	emTkTiling * tiling;

	emTkGroup::AutoExpand();

	new emTkLabel(
		this,"label",
		"Here you can set the maximum allowed memory consumption per view (or window) in\n"
		"megabytes. This mainly plays a role when viewing extravagant files like\n"
		"high-resolution images files. The higher the maximum allowed memory consumption,\n"
		"the earlier the files are shown and the more extravagant files are shown at all.\n"
		"\n"
		"IMPORTANT: This is just a guideline for the program. The internal algorithms\n"
		"around this are working with heuristics and they are far from being exact. In\n"
		"seldom situations, a view may consume much more memory (factor two or so).\n"
		"\n"
		"RECOMMENDATION: The value should not be larger than a quarter of the total\n"
		"memory including swap space, and it should not be larger than the half of the\n"
		"system RAM. Examples: 2048MB RAM, 0MB Swap => 512MB; 2048MB RAM, 2048MB Swap =>\n"
		"1024MB; 1024MB RAM, 8192MB Swap => 512MB. This is just a rough recommendation\n"
		"for an average system and user. It depends on the number of windows you open,\n"
		"and on the memory consumption through other running programs.\n"
		"\n"
		"WARNING: If you set a too large value, everything may work fine for a long time,\n"
		"but one day it could happen you zoom into something and the whole system gets\n"
		"extremely slow, or it even hangs, in lack of free memory.\n"
		"\n"
		"NOTE: After changing the value, you may have to restart the program for the\n"
		"change to take effect. Or zoom out from all panels once."
	);
	tiling=new emTkTiling(this,"tiling");
	tiling->SetOuterSpace(0.02,0.05,0.05,0.0);
	MemField=new emTkScalarField(
		tiling,"field",
		emString(),
		emString(),
		emImage(),
		300,1400,ValOut,true
	);
	MemField->SetScaleMarkIntervals(100,10,0);
	MemField->SetTextOfValueFunc(&TextOfMemValue);
	AddWakeUpSignal(MemField->GetValueSignal());
	UpdateOutput();
}


void emCoreConfigPanel::MaxMemGroup::AutoShrink()
{
	emTkGroup::AutoShrink();
	MemField=NULL;
}


void emCoreConfigPanel::MaxMemGroup::UpdateOutput()
{
	double d;

	if (MemField) {
		d=(double)Config->MaxMegabytesPerView.Get();
		d=log(d)/log(2.0)*100.0;
		ValOut=(emInt64)(d+0.5);
		MemField->SetValue(ValOut);
	}
}


void emCoreConfigPanel::MaxMemGroup::TextOfMemValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	double d;

	d=pow(2.0,((double)value)/100.0);
	if (markInterval<100 && d<64.0) {
		snprintf(buf,bufSize,"%.1f",d);
	}
	else {
		snprintf(buf,bufSize,"%d",(int)(d+0.5));
	}
}


emCoreConfigPanel::MaxMemTunnel::MaxMemTunnel(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emTkTunnel(parent,name,"Max Megabytes Per View"),
	Config(config)
{
	SetChildTallness(0.3);
	EnableAutoExpansion();
}


emCoreConfigPanel::MaxMemTunnel::~MaxMemTunnel()
{
}


void emCoreConfigPanel::MaxMemTunnel::AutoExpand()
{
	emTkTunnel * tunnel;

	emTkTunnel::AutoExpand();

	tunnel=new emTkTunnel(
		this,"tunnel",
		"Please read all text herein before making a change!"
	);
	tunnel->SetChildTallness(0.7);
	new MaxMemGroup(tunnel,"group",Config);
}
