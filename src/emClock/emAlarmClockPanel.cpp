//------------------------------------------------------------------------------
// emAlarmClockPanel.cpp
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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

#include <emClock/emAlarmClockPanel.h>


emAlarmClockPanel::emAlarmClockPanel(
	ParentArg parent, const emString & name,
	emClockFileModel * fileModel, emColor fgColor
)
	: emFilePanel(parent,name,fileModel)
{
	FileModel=fileModel;
	AlarmModel=emAlarmClockModel::Acquire(GetView(),FileModel->GetName());
	FgColor=fgColor;

	TimeField=new emTkScalarField(
		this,"time_field",emString(),emString(),emImage(),0,86400,0,true
	);
	TimeField->SetScaleMarkIntervals(6*3600,3600,900,300,60,10,1,0);
	TimeField->SetTextOfValueFunc(TimeFieldTextOfValue,NULL);
	TimeField->SetKeyboardInterval(300);

	OnButton=new emTkRadioButton(
		this,"on_button","On",
		"Enable the alarm."
	);
	OffButton=new emTkRadioButton(
		this,"off_button","Off",
		"Disable the alarm."
	);
	TestButton=new emTkButton(
		this,"test_button","Test Beep",
		"Play a single alarm beep sound for a test. If this\n"
		"does not work, there is probably something wrong with\n"
		"the hardware or with the operating system setup."
	);
	ConfirmButton=new emTkButton(
		this,"confirm_button","Confirm",
		"Confirm the alarm when it is running. Just\n"
		"clicking this means to get the alarm again\n"
		"after 24 hours."
	);

	FileModel->TkLook.Apply(this,true);

	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(AlarmModel->GetChangeSignal());
	AddWakeUpSignal(TimeField->GetValueSignal());
	AddWakeUpSignal(OnButton->GetClickSignal());
	AddWakeUpSignal(OffButton->GetClickSignal());
	AddWakeUpSignal(TestButton->GetClickSignal());
	AddWakeUpSignal(ConfirmButton->GetClickSignal());

	UpdateFieldsAndButtons();
}


emAlarmClockPanel::~emAlarmClockPanel()
{
}


void emAlarmClockPanel::SetFgColor(emColor fgColor)
{
	FgColor=fgColor;
	InvalidatePainting();
}


emString emAlarmClockPanel::GetTitle()
{
	return "Alarm Clock";
}


bool emAlarmClockPanel::Cycle()
{
	bool busy;
	int value;

	busy=emFilePanel::Cycle();

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		IsSignaled(FileModel->GetChangeSignal()) ||
		IsSignaled(AlarmModel->GetChangeSignal())
	) {
		UpdateFieldsAndButtons();
	}

	if (IsSignaled(TimeField->GetValueSignal())) {
		if (IsVFSGood()) {
			value=(int)TimeField->GetValue();
			FileModel->AlarmHour=value/3600;
			FileModel->AlarmMinute=value/60%60;
			FileModel->AlarmSecond=value%60;
			FileModel->Save(true);
			if (
				AlarmModel->IsAlarmEnabled() &&
				AlarmModel->GetAlarmSecOfDay()!=value
			) {
				AlarmModel->EnableAlarm(GetIdentity(),value);
			}
		}
	}

	if (IsSignaled(OnButton->GetClickSignal())) {
		if (IsVFSGood()) {
			value=(int)TimeField->GetValue();
			AlarmModel->EnableAlarm(GetIdentity(),value);
		}
	}

	if (IsSignaled(OffButton->GetClickSignal())) {
		AlarmModel->DisableAlarm();
	}

	if (IsSignaled(TestButton->GetClickSignal())) {
		AlarmModel->Beep();
	}

	if (IsSignaled(ConfirmButton->GetClickSignal())) {
		AlarmModel->ConfirmAlarm();
	}

	return busy;
}


bool emAlarmClockPanel::IsOpaque()
{
	return false;
}


void emAlarmClockPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double h;

	h=GetHeight();

	painter.PaintRoundRectOutline(
		0.005,
		0.005,
		1.0-0.01,
		h-0.01,
		0.07,
		0.07,
		0.01,
		FgColor,
		canvasColor
	);

	painter.PaintTextBoxed(
		0.05,
		h*0.05,
		0.4,
		h*0.1,
		"Alarm Time",
		h*0.1,
		FgColor,
		canvasColor
	);

	painter.PaintTextBoxed(
		0.5,
		h*0.05,
		0.45,
		h*0.11,
		"IMPORTANT: The alarm mechanism works on a per-view basis. This means\n"
		"that the alarm gets disabled as soon as the view or window is closed\n"
		"(but it is okay to navigate away, you will be brought back here when\n"
		"the alarm raises). Another odd consequence is that it is possible to\n"
		"prepare different alarms in different views of the same clock file.",
		h*0.11,
		FgColor,
		canvasColor,
		EM_ALIGN_LEFT
	);
}


void emAlarmClockPanel::LayoutChildren()
{
	double h;

	h=GetHeight();

	TimeField->Layout(
		0.05,
		h*0.18,
		0.9,
		h*0.24,
		GetCanvasColor()
	);
	OnButton->Layout(
		0.05,
		h*0.42,
		0.3,
		h*0.24,
		GetCanvasColor()
	);
	OffButton->Layout(
		0.35,
		h*0.42,
		0.3,
		h*0.24,
		GetCanvasColor()
	);
	TestButton->Layout(
		0.65,
		h*0.42,
		0.3,
		h*0.24,
		GetCanvasColor()
	);
	ConfirmButton->Layout(
		0.05,
		h*0.66,
		0.9,
		h*0.24,
		GetCanvasColor()
	);
}


void emAlarmClockPanel::UpdateFieldsAndButtons()
{
	bool vfsGood, alarming, alarmEnabled;
	int value;

	vfsGood=IsVFSGood();
	alarmEnabled=AlarmModel->IsAlarmEnabled();
	alarming=AlarmModel->IsAlarming();

	if (alarmEnabled) {
		value=AlarmModel->GetAlarmSecOfDay();
	}
	else if (vfsGood) {
		value=
			FileModel->AlarmHour*3600+
			FileModel->AlarmMinute*60+
			FileModel->AlarmSecond
		;
	}
	else {
		value=0;
	}
	TimeField->SetValue(value);
	TimeField->SetEnableSwitch(vfsGood);

	OffButton->SetChecked(!alarmEnabled);

	OnButton->SetChecked(alarmEnabled);
	OnButton->SetEnableSwitch(vfsGood);

	ConfirmButton->SetEnableSwitch(alarming);
}


void emAlarmClockPanel::TimeFieldTextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	int h,m,s;

	h=(int)(value/3600);
	m=(int)((value/60)%60);
	s=(int)(value%60);
	if (markInterval<60) {
		snprintf(buf,bufSize,"%02d:%02d:%02d",h,m,s);
	}
	else {
		snprintf(buf,bufSize,"%02d:%02d",h,m);
	}
	buf[bufSize-1]=0;
}
