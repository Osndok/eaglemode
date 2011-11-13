//------------------------------------------------------------------------------
// emStopwatchPanel.cpp
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

#include <emClock/emStopwatchPanel.h>


emStopwatchPanel::emStopwatchPanel(
	ParentArg parent, const emString & name,
	emClockFileModel * fileModel, emColor fgColor
)
	: emFilePanel(parent,name,fileModel)
{
	FileModel=fileModel;
	FgColor=fgColor;
	TimeField=new emTkTextField(this,"time_field");
	StartStopButton=new emTkButton(
		this,"start_stop_button","Start/Stop",
		"Start or stop the stopwatch.\n"
		"Remember that the action is performed\n"
		"at releasing of the mouse button."
	);
	ClearButton=new emTkButton(
		this,"clear_button","Clear",
		"Reset the stopwatch to zero time."
	);

	FileModel->TkLook.Apply(this,true);

	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(StartStopButton->GetClickSignal());
	AddWakeUpSignal(ClearButton->GetClickSignal());
	UpdateTimeFieldAndButtons();
	WakeUp();
}


emStopwatchPanel::~emStopwatchPanel()
{
}


void emStopwatchPanel::SetFgColor(emColor fgColor)
{
	FgColor=fgColor;
	InvalidatePainting();
}


emString emStopwatchPanel::GetTitle()
{
	return "Stopwatch";
}


bool emStopwatchPanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();

	if (IsSignaled(StartStopButton->GetClickSignal()) && IsVFSGood()) {
		if (FileModel->IsStopwatchRunning()) FileModel->StopStopwatch();
		else FileModel->StartStopwatch();
		FileModel->Save(true);
	}

	if (IsSignaled(ClearButton->GetClickSignal()) && IsVFSGood()) {
		FileModel->ClearStopwatch();
		FileModel->Save(true);
	}

	if (
		IsSignaled(GetVirFileStateSignal()) ||
		IsSignaled(FileModel->GetChangeSignal())
	) {
		UpdateTimeFieldAndButtons();
	}

	if (FileModel->IsStopwatchRunning() && IsVFSGood()) {
		UpdateTimeFieldAndButtons();
		busy=true;
	}

	return busy;
}


bool emStopwatchPanel::IsOpaque()
{
	return false;
}


void emStopwatchPanel::Paint(const emPainter & painter, emColor canvasColor)
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
		0.9,
		h*0.1,
		"Stopwatch",
		h*0.1,
		FgColor,
		canvasColor
	);
}


void emStopwatchPanel::LayoutChildren()
{
	double h;

	h=GetHeight();

	TimeField->Layout(
		0.05,
		h*0.18,
		0.9,
		h*0.44,
		GetCanvasColor()
	);
	StartStopButton->Layout(
		0.05,
		h*0.62,
		0.6,
		h*0.28,
		GetCanvasColor()
	);
	ClearButton->Layout(
		0.65,
		h*0.62,
		0.3,
		h*0.28,
		GetCanvasColor()
	);
}


void emStopwatchPanel::UpdateTimeFieldAndButtons()
{
	emString str;
	emInt64 t;

	if (IsVFSGood()) {
			t=FileModel->GetStopwatchTimeMS();
			if (t<0) { str="-"; t=-t; }
			else str="";
			str+=emString::Format(
				"%02d:%02d:%02d.%02d",
				(int)(t/3600000),
				(int)(t/60000%60),
				(int)(t/1000%60),
				(int)(t/10%100)
			);
			TimeField->SetText(str);
			StartStopButton->SetEnableSwitch(true);
			ClearButton->SetEnableSwitch(!FileModel->IsStopwatchRunning());
	}
	else {
		TimeField->SetText("");
		StartStopButton->SetEnableSwitch(false);
		ClearButton->SetEnableSwitch(false);
	}
}
