//------------------------------------------------------------------------------
// emCoreConfigPanel.cpp
//
// Copyright (C) 2007-2010,2014-2017 Oliver Hamann.
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

#include <emCore/emCoreConfigPanel.h>


emCoreConfigPanel::emCoreConfigPanel(
	ParentArg parent, const emString & name
)
	: emLinearGroup(parent,name,
			"General Preferences",
			"This panel provides general user settings."
	)
{
	Config=emCoreConfig::Acquire(GetRootContext());
	ResetButton=NULL;
}


emCoreConfigPanel::~emCoreConfigPanel()
{
}


bool emCoreConfigPanel::Cycle()
{
	bool busy;

	busy=emLinearGroup::Cycle();
	if (ResetButton && IsSignaled(ResetButton->GetClickSignal())) {
		Config->SetToDefault();
		Config->Save();
	}
	return busy;
}


void emCoreConfigPanel::AutoExpand()
{
	emRasterLayout * content;
	emLinearLayout * buttons;

	emLinearGroup::AutoExpand();

	SetVertical();
	SetChildWeight(0,12.0);
	SetChildWeight(1,1.0);
	SetSpace(0.01,0.1,0.01,0.1,0.01,0.0);

	content=new emRasterLayout(this,"content");
	content->SetPrefChildTallness(0.5);
	content->SetInnerSpace(0.1,0.2);
	new MouseGroup(content,"mouse",Config);
	new KBGroup(content,"keyboard",Config);
	new KineticGroup(content,"kinetic",Config);
	new PerformanceGroup(content,"performance",Config);

	buttons=new emLinearLayout(this,"buttons");
	buttons->SetChildTallness(0.2);
	buttons->SetAlignment(EM_ALIGN_BOTTOM_RIGHT);
	ResetButton=new emButton(buttons,"reset","Reset To Defaults");
	ResetButton->SetNoEOI();
	AddWakeUpSignal(ResetButton->GetClickSignal());
}


void emCoreConfigPanel::AutoShrink()
{
	emLinearGroup::AutoShrink();
	ResetButton=NULL;
}


emCoreConfigPanel::FactorField::FactorField(
	ParentArg parent, const emString & name,
	const emString & caption, const emString & description,
	const emImage & icon, emCoreConfig * config, emDoubleRec * rec,
	bool minimumMeansDisabled
)
	: emScalarField(
		parent,name,caption,description,icon,
		-200,200,0,true
	),
	emRecListener(rec),
	Config(config)
{
	MinimumMeansDisabled=minimumMeansDisabled;
	ValOut=0;
	SetScaleMarkIntervals(100,10,0);
	SetBorderScaling(1.5);
	SetTextBoxTallness(0.3);
	UpdateValue();
}


emCoreConfigPanel::FactorField::~FactorField()
{
}


void emCoreConfigPanel::FactorField::TextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval
) const
{
	const char * str;

	if (markInterval>=100) {
		switch ((int)value) {
			case -200: str=MinimumMeansDisabled ? "Disabled" : "Minimal"; break;
			case -100: str="Reduced"; break;
			case    0: str="Default"; break;
			case  100: str="Increased"; break;
			case  200: str="Extreme"; break;
			default: str="?";
		}
		snprintf(buf,bufSize,"%s",str);
	}
	else if (markInterval>=10) {
		snprintf(buf,bufSize,"%.2f",Val2Cfg(value));
	}
	else {
		snprintf(buf,bufSize,"%.3f",Val2Cfg(value));
	}
}


void emCoreConfigPanel::FactorField::ValueChanged()
{
	emDoubleRec * rec;
	double d;
	emInt64 val;

	val=GetValue();
	if (ValOut==val) return;
	ValOut=val;
	rec=(emDoubleRec*)GetListenedRec();
	if (!rec) return;
	d=Val2Cfg(val);
	if (rec->Get()!=d) {
		rec->Set(d);
		if (Config) Config->Save();
	}
}


void emCoreConfigPanel::FactorField::OnRecChanged()
{
	UpdateValue();
}


void emCoreConfigPanel::FactorField::UpdateValue()
{
	const emDoubleRec * rec;

	rec=(const emDoubleRec*)GetListenedRec();
	if (!rec) return;
	ValOut=Cfg2Val(rec->Get());
	SetValue(ValOut);
}


double emCoreConfigPanel::FactorField::Val2Cfg(emInt64 value) const
{
	const emDoubleRec * rec;
	double m;

	rec=(const emDoubleRec*)GetListenedRec();
	if (!rec) return 1.0;
	if (value>=0) {
		m=rec->GetMaxValue();
	}
	else {
		m=1.0/rec->GetMinValue();
	}
	return pow(sqrt(m),value/100.0);
}


emInt64 emCoreConfigPanel::FactorField::Cfg2Val(double d) const
{
	const emDoubleRec * rec;
	double m,v;

	rec=(const emDoubleRec*)GetListenedRec();
	if (!rec) return 0;
	if (d>=1.0) {
		m=rec->GetMaxValue();
	}
	else {
		m=1.0/rec->GetMinValue();
	}
	v=log(d)/log(sqrt(m))*100.0;
	if (v>=0.0) v+=0.5; else v-=0.5;
	return (emInt64)v;
}


emCoreConfigPanel::MouseMiscGroup::MouseMiscGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emRasterGroup(parent,name,"Miscellaneous mouse settings"),
	emRecListener(config),
	Config(config)
{
	StickBox=NULL;
	EmuBox=NULL;
	PanBox=NULL;
	SetBorderScaling(4.0);
	SetPrefChildTallness(0.04);
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

	busy=emRasterGroup::Cycle();

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
	emRasterGroup::AutoExpand();
	StickBox=new emCheckBox(
		this,"stick","Stick mouse when navigating",
		"Whether to keep the mouse pointer at its place while zooming\n"
		"or scrolling with the mouse (except for pan function)."
	);
	EmuBox=new emCheckBox(
		this,"emu","Alt key as middle button",
		"Whether to emulate the middle mouse button by the ALT key."
	);
	PanBox=new emCheckBox(
		this,"pan","Reverse scrolling (pan)",
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
	emRasterGroup::AutoShrink();
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
	: emRasterGroup(parent,name,"Mouse Control"),
	Config(config)
{
	SetPrefChildTallness(0.2);
	SetBorderScaling(3.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::MouseGroup::~MouseGroup()
{
}


void emCoreConfigPanel::MouseGroup::AutoExpand()
{
	emRasterGroup::AutoExpand();
	new FactorField(
		this,"wheelzoom",
		"Speed of zooming by mouse wheel",
		"How fast to zoom by moving the mouse wheel.",
		emImage(),
		Config,&Config->MouseWheelZoomSpeed
	);
	new FactorField(
		this,"wheelaccel",
		"Acceleration of zooming by mouse wheel",
		"Acceleration means: If you move the wheel quickly, the among\n"
		"of zooming is more than when moving the wheel the same\n"
		"distance slowly. Here you can set the strength of that effect.",
		emImage(),
		Config,&Config->MouseWheelZoomAcceleration,
		true
	);
	new FactorField(
		this,"zoom",
		"Speed of zooming by mouse",
		"How fast to zoom with Ctrl Key + Middle Mouse Button + Vertical Mouse Movement.",
		emImage(),
		Config,&Config->MouseZoomSpeed
	);
	new FactorField(
		this,"scroll",
		"Speed of scrolling by mouse",
		"How fast to scroll with Middle Mouse Button + Mouse Movement.",
		emImage(),
		Config,&Config->MouseScrollSpeed
	);
	new MouseMiscGroup(this,"misc",Config);
}


emCoreConfigPanel::KBGroup::KBGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emRasterGroup(parent,name,"Keyboard Control"),
	Config(config)
{
	SetPrefChildTallness(0.2);
	SetBorderScaling(3.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::KBGroup::~KBGroup()
{
}


void emCoreConfigPanel::KBGroup::AutoExpand()
{
	emRasterGroup::AutoExpand();
	new FactorField(
		this,"zoom",
		"Speed of zooming by keyboard",
		"How fast to zoom by pressing Alt + Page-Up/Down.",
		emImage(),
		Config,&Config->KeyboardZoomSpeed
	);
	new FactorField(
		this,"scroll",
		"Speed of scrolling by keyboard",
		"How fast to scroll by pressing Alt + Cursor Key.",
		emImage(),
		Config,&Config->KeyboardScrollSpeed
	);
}


emCoreConfigPanel::KineticGroup::KineticGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emRasterGroup(parent,name,"Kinetic Effects"),
	Config(config)
{
	SetPrefChildTallness(0.2);
	SetBorderScaling(3.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::KineticGroup::~KineticGroup()
{
}


void emCoreConfigPanel::KineticGroup::AutoExpand()
{
	emRasterGroup::AutoExpand();
	new FactorField(
		this,"KineticZoomingAndScrolling",
		"Kinetic zooming and scrolling",
		"This controls the effects of inertia and friction when\n"
		"zooming and scrolling by mouse, keyboard or touch.",
		emImage(),
		Config,&Config->KineticZoomingAndScrolling,
		true
	);
	new FactorField(
		this,"MagnetismRadius",
		"Radius of magnetism",
		"The magnetism zooms and scrolls automatically for showing a\n"
		"content full-sized. It gets active after zooming or scrolling\n"
		"by mouse, but only when a content is not to far from being\n"
		"shown full-sized. That \"to far\" can be set here. The higher\n"
		"the value, the longer the way the magnetism may accept.",
		emImage(),
		Config,&Config->MagnetismRadius,
		true
	);
	new FactorField(
		this,"MagnetismSpeed",
		"Speed of magnetism",
		"This controls the speed of scrolling and zooming by the magnetism.",
		emImage(),
		Config,&Config->MagnetismSpeed
	);
	new FactorField(
		this,"VisitSpeed",
		"Speed of changing location",
		"This controls the speed of scrolling and zooming for logical\n"
		"position changes by keys and bookmarks.",emImage(),
		Config,&Config->VisitSpeed
	);
}


emCoreConfigPanel::MaxMemGroup::MaxMemGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emLinearGroup(parent,name,"Max Megabytes Per View"),
	emRecListener(&config->MaxMegabytesPerView),
	Config(config)
{
	MemField=NULL;
	ValOut=0;
	SetVertical();
	SetChildWeight(0,5.0);
	SetChildWeight(1,1.0);
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

	busy=emLinearGroup::Cycle();
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
	emLinearLayout * linearLayout;

	emLinearGroup::AutoExpand();

	new emLabel(
		this,"label",
		"Here you can set the maximum allowed memory consumption per view (or window) in\n"
		"megabytes. This mainly plays a role when viewing extravagant files like\n"
		"high-resolution image files. The higher the maximum allowed memory consumption,\n"
		"the earlier the files are shown and the more extravagant files are shown at all.\n"
		"\n"
		"IMPORTANT: This is just a guideline for the program. The internal algorithms\n"
		"around this are working with heuristics and they are far from being exact. In\n"
		"very seldom situations, a view may consume much more memory (factor two or so).\n"
		"\n"
		"RECOMMENDATION: The value should not be greater than a quarter of the total\n"
		"system memory (RAM). Examples: 4096MB RAM => 1024MB; 8192MB RAM => 2048MB. This\n"
		"is just a rough recommendation for an average system and user. It depends on the\n"
		"number of windows you open, and on the memory consumption through other running\n"
		"programs.\n"
		"\n"
		"WARNING: If you set a too large value, everything may work fine for a long time,\n"
		"but one day it could happen you zoom into something and the whole system gets\n"
		"extremely slow, or it even hangs, in lack of free memory.\n"
		"\n"
		"NOTE: After changing the value, you may have to restart the program for the\n"
		"change to take effect. Or zoom out from all panels once."
	);
	linearLayout=new emLinearLayout(this,"layout");
	linearLayout->SetOuterSpace(0.02,0.05,0.05,0.0);
	MemField=new emScalarField(
		linearLayout,"field",
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
	emLinearGroup::AutoShrink();
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
	: emTunnel(parent,name,"Max Megabytes Per View"),
	Config(config)
{
	SetBorderScaling(0.75);
	SetChildTallness(0.3);
}


emCoreConfigPanel::MaxMemTunnel::~MaxMemTunnel()
{
}


void emCoreConfigPanel::MaxMemTunnel::AutoExpand()
{
	emTunnel * tunnel;

	emTunnel::AutoExpand();

	tunnel=new emTunnel(
		this,"tunnel",
		"Please read all text herein before making a change!"
	);
	tunnel->SetChildTallness(0.7);
	new MaxMemGroup(tunnel,"group",Config);
}


emCoreConfigPanel::PerformanceGroup::PerformanceGroup(
	ParentArg parent, const emString & name, emCoreConfig * config
)
	: emRasterGroup(parent,name,"Performance"),
	emRecListener(config),
	Config(config)
{
	SetPrefChildTallness(0.2);
	SetBorderScaling(3.0);
	SetSpace(0.05,0.1,0.05,0.1);
}


emCoreConfigPanel::PerformanceGroup::~PerformanceGroup()
{
}


void emCoreConfigPanel::PerformanceGroup::OnRecChanged()
{
	UpdateOutput();
}


bool emCoreConfigPanel::PerformanceGroup::Cycle()
{
	bool busy;

	busy=emRasterGroup::Cycle();

	if (
		MaxRenderThreadsField &&
		IsSignaled(MaxRenderThreadsField->GetValueSignal())
	) {
		int val=(int)MaxRenderThreadsField->GetValue();
		if (Config->MaxRenderThreads.Get() != val) {
			Config->MaxRenderThreads.Set(val);
			Config->Save();
		}
	}

	return busy;
}


void emCoreConfigPanel::PerformanceGroup::AutoExpand()
{
	emRasterGroup::AutoExpand();
	new MaxMemTunnel(this,"maxmem",Config);

	MaxRenderThreadsField=new emScalarField(
		this,"MaxRenderThreads",
		"Max Render Threads",
		"Maximum number of CPU threads used for painting graphics.\n"
		"In any case, no more threads are used than the hardware can\n"
		"run concurrently by multiple CPUs, cores, or hyperthreads.\n"
		"So this setting is just an additional limit, for the case\n"
		"you want this program to use less CPU resources.",
		emImage(),
		1,32,Config->MaxRenderThreads.Get(),true
	);
	MaxRenderThreadsField->SetScaleMarkIntervals(1);
	AddWakeUpSignal(MaxRenderThreadsField->GetValueSignal());
	UpdateOutput();
}


void emCoreConfigPanel::PerformanceGroup::AutoShrink()
{
	emRasterGroup::AutoShrink();
	MaxRenderThreadsField=NULL;
}


void emCoreConfigPanel::PerformanceGroup::UpdateOutput()
{
	if (MaxRenderThreadsField) {
		MaxRenderThreadsField->SetValue(Config->MaxRenderThreads.Get());
	}
}
