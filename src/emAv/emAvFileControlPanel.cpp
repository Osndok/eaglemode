//------------------------------------------------------------------------------
// emAvFileControlPanel.cpp
//
// Copyright (C) 2008,2011 Oliver Hamann.
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

#include <emCore/emRes.h>
#include <emAv/emAvFileControlPanel.h>


emAvFileControlPanel::emAvFileControlPanel(
	ParentArg parent, const emString & name, emAvFileModel * fileModel
)
	: emTkGroup(parent,name,"emAv - Audio & Video Player")
{
	emTkTiling * left, * right, * vol, * inf;
	emTkLook look;

	Mdl=fileModel;

	SetPrefChildTallness(0.2);
	SetPrefChildTallness(0.4,1);

	left=new emTkTiling(this,"left");
	left->SetPrefChildTallness(0.05);
	left->SetPrefChildTallness(0.065,-1);
	left->SetPrefChildTallness(0.08,-2);

		inf=new emTkTiling(left,"inf");
		inf->SetPrefChildTallness(0.4);
		inf->SetPrefChildTallness(0.05,1);
		TfInfo=new emTkTextField(inf,"info","File Info");
		TfInfo->SetMultiLineMode();
		TfWarning=new emTkTextField(inf,"warning","Player Warnings");
		TfWarning->SetMultiLineMode();
		look=TfWarning->GetLook();
		look.SetOutputFgColor(0xFF0000FF);
		TfWarning->SetLook(look);

		SfPlayPos=new emTkScalarField(
			left,"pos",
			"Position",
			"Hotkeys:\n"
			"  D = Decrement\n"
			"  I = Increment\n"
			"  1 = Start\n"
			"  2 = 11%\n"
			"  3 = 22%\n"
			"  ...\n"
			"  9 = 89%"
		);
		SfPlayPos->SetBorderScaling(0.8);
		SfPlayPos->SetScaleMarkIntervals(
			60*60*1000,15*60*1000,5*60*1000,60*1000,
			15*1000,5*1000,1000,500,100,0
		);
		SfPlayPos->SetTextOfValueFunc(TextOfPlayPos,this);
		SfPlayPos->SetTextBoxTallness(0.3);
		SfPlayPos->SetEditable(true);

		RgPlayState=new emTkRadioButton::Group(left,"play_state");
		RgPlayState->SetBorderType(OBT_NONE,IBT_NONE);
		RgPlayState->SetFocusable(false);
		RgPlayState->SetPrefChildTallness(0.3);
		RgPlayState->SetPrefChildTallness(1.0,2,false);
		RgPlayState->SetPrefChildTallness(1.0,4,false);
		RbStop=new emTkRadioButton(
			RgPlayState,"stop",
			emString(),
			"Stop playing.\n"
			"\n"
			"Hotkey: 0",
			emGetInsResImage(GetRootContext(),"emAv","Stop.tga")
		);
		RbPause=new emTkRadioButton(
			RgPlayState,
			"pause",
			emString(),
			"Pause playing.\n"
			"\n"
			"Hotkey: P or Space",
			emGetInsResImage(GetRootContext(),"emAv","Pause.tga")
		);
		RbSlow=new emTkRadioButton(
			RgPlayState,
			"slow",
			emString(),
			"Play slow.\n"
			"\n"
			"Hotkey: S",
			emGetInsResImage(GetRootContext(),"emAv","PlaySlow.tga")
		);
		RbPlay=new emTkRadioButton(
			RgPlayState,
			"play",
			emString(),
			"Play with normal speed.\n"
			"\n"
			"Hotkey: N or Space",
			emGetInsResImage(GetRootContext(),"emAv","Play.tga")
		);
		RbFast=new emTkRadioButton(
			RgPlayState,
			"fast",
			emString(),
			"Play fast.\n"
			"\n"
			"Hotkey: F",
			emGetInsResImage(GetRootContext(),"emAv","PlayFast.tga")
		);

	right=new emTkTiling(this,"right");
	right->SetPrefChildTallness(0.1);

		vol=new emTkTiling(right,"audio_volume");
		vol->SetPrefChildTallness(0.5);
		vol->SetPrefChildTallness(0.12,1);
		CbAudioMute=new emTkCheckButton(
			vol,"mute",
			"Mute",
			"Hotkey: U"
		);
		SfAudioVolume=new emTkScalarField(
			vol,"audio_volume",
			"Audio Volume",
			"Hotkeys:\n"
			"  + = Increase.\n"
			"  - = Decrease."
		);
		SfAudioVolume->SetScaleMarkIntervals(25,5,1,0);
		SfAudioVolume->SetTextOfValueFunc(TextOfAudioVolume,this);
		SfAudioVolume->SetTextBoxTallness(0.43);
		SfAudioVolume->SetEditable(true);
		SfAudioVolume->SetMaxValue(100);

		SfAudioVisu=new emTkScalarField(
			right,"audio_visu",
			"Audio Visualization",
			"Hotkeys:\n"
			"  Ctrl+Z       = Next visualization.\n"
			"  Ctrl+Shift+Z = Previous visualization."
		);
		SfAudioVisu->SetTextOfValueFunc(TextOfAudioVisu,this);
		SfAudioVisu->SetTextBoxTallness(0.3);
		SfAudioVisu->SetEditable(true);

		SfAudioChannel=new emTkScalarField(
			right,"audio_channel",
			"Audio Channel",
			"Hotkeys:\n"
			"  Ctrl+A       = Next channel.\n"
			"  Ctrl+Shift+A = Previous channel."
		);
		SfAudioChannel->SetTextOfValueFunc(TextOfAudioChannel,this);
		SfAudioChannel->SetTextBoxTallness(0.3);
		SfAudioChannel->SetEditable(true);

		SfSpuChannel=new emTkScalarField(
			right,"spu_channel",
			"Subtitles",
			"Hotkeys:\n"
			"  Ctrl+S       = Next channel.\n"
			"  Ctrl+Shift+S = Previous channel."
		);
		SfSpuChannel->SetTextOfValueFunc(TextOfSpuChannel,this);
		SfSpuChannel->SetTextBoxTallness(0.3);
		SfSpuChannel->SetEditable(true);

	AddWakeUpSignal(Mdl->GetInfoSignal());
	AddWakeUpSignal(Mdl->GetPlayStateSignal());
	AddWakeUpSignal(Mdl->GetPlayPosSignal());
	AddWakeUpSignal(Mdl->GetAdjustmentSignal());
	AddWakeUpSignal(SfPlayPos->GetValueSignal());
	AddWakeUpSignal(RgPlayState->GetCheckSignal());
	AddWakeUpSignal(CbAudioMute->GetCheckSignal());
	AddWakeUpSignal(SfAudioVolume->GetValueSignal());
	AddWakeUpSignal(SfAudioVisu->GetValueSignal());
	AddWakeUpSignal(SfAudioChannel->GetValueSignal());
	AddWakeUpSignal(SfSpuChannel->GetValueSignal());

	UpdateControls();
}


emAvFileControlPanel::~emAvFileControlPanel()
{
}


bool emAvFileControlPanel::Cycle()
{
	bool busy;

	busy=emTkGroup::Cycle();

	if (
		IsSignaled(Mdl->GetInfoSignal()) ||
		IsSignaled(Mdl->GetPlayStateSignal()) ||
		IsSignaled(Mdl->GetAdjustmentSignal())
	) {
		UpdateControls();
	}
	if (IsSignaled(Mdl->GetPlayPosSignal())) {
		SfPlayPos->SetValue(Mdl->GetPlayPos());
	}

	if (IsSignaled(SfPlayPos->GetValueSignal())) {
		Mdl->SetPlayPos(SfPlayPos->GetValue());
	}
	if (IsSignaled(RgPlayState->GetCheckSignal())) {
		if (RbStop->IsChecked()) Mdl->Stop();
		else if (RbPause->IsChecked()) Mdl->Pause();
		else if (RbPlay->IsChecked()) Mdl->Play();
		else if (RbSlow->IsChecked()) Mdl->PlaySlow();
		else if (RbFast->IsChecked()) Mdl->PlayFast();
	}
	if (IsSignaled(CbAudioMute->GetCheckSignal())) {
		Mdl->SetAudioMute(CbAudioMute->IsChecked());
	}
	if (IsSignaled(SfAudioVolume->GetValueSignal())) {
		Mdl->SetAudioVolume(SfAudioVolume->GetValue());
	}
	if (IsSignaled(SfAudioVisu->GetValueSignal())) {
		Mdl->SetAudioVisu(SfAudioVisu->GetValue());
	}
	if (IsSignaled(SfAudioChannel->GetValueSignal())) {
		Mdl->SetAudioChannel(SfAudioChannel->GetValue());
	}
	if (IsSignaled(SfSpuChannel->GetValueSignal())) {
		Mdl->SetSpuChannel(SfSpuChannel->GetValue());
	}

	return busy;
}


void emAvFileControlPanel::UpdateControls()
{
	emTkRadioButton * rb;
	bool adjustingEnabled;
	int n;

	adjustingEnabled=(Mdl->GetPlayState()!=emAvFileModel::PS_STOPPED);
	// When changing the enable stuff here, also remember to adapt the
	// enabling of hotkeys in emAvFilePanel::Input.

	TfInfo->SetText(Mdl->GetInfoText());
	TfWarning->SetText(Mdl->GetWarningText());

	n=Mdl->GetPlayLength();
	SfPlayPos->SetEnableSwitch(n>0);
	SfPlayPos->SetMaxValue(n);
	SfPlayPos->SetValue(Mdl->GetPlayPos());

	switch (Mdl->GetPlayState()) {
	case emAvFileModel::PS_STOPPED: rb=RbStop;  break;
	case emAvFileModel::PS_PAUSED : rb=RbPause; break;
	case emAvFileModel::PS_NORMAL : rb=RbPlay;  break;
	case emAvFileModel::PS_SLOW   : rb=RbSlow;  break;
	case emAvFileModel::PS_FAST   : rb=RbFast;  break;
	default                       : rb=NULL;
	}
	RgPlayState->SetChecked(rb);

	CbAudioMute->SetEnableSwitch(adjustingEnabled);
	CbAudioMute->SetChecked(Mdl->GetAudioMute());

	SfAudioVolume->SetEnableSwitch(adjustingEnabled);
	SfAudioVolume->SetValue(Mdl->GetAudioVolume());

	n=Mdl->GetAudioVisus().GetCount();
	SfAudioVisu->SetEnableSwitch(n>1 && adjustingEnabled);
	SfAudioVisu->SetMaxValue(emMax(n-1,0));
	SfAudioVisu->SetValue(Mdl->GetAudioVisu());

	n=Mdl->GetAudioChannels().GetCount();
	SfAudioChannel->SetEnableSwitch(n>1 && adjustingEnabled);
	SfAudioChannel->SetMaxValue(emMax(n-1,0));
	SfAudioChannel->SetValue(Mdl->GetAudioChannel());

	n=Mdl->GetSpuChannels().GetCount();
	SfSpuChannel->SetEnableSwitch(n>1 && adjustingEnabled);
	SfSpuChannel->SetMaxValue(emMax(n-1,0));
	SfSpuChannel->SetValue(Mdl->GetSpuChannel());
}


void emAvFileControlPanel::TextOfPlayPos(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	int h,m,s,ms;

	h=(int)(value/3600000);
	m=(int)((value/60000)%60);
	s=(int)((value/1000)%60);
	ms=(int)(value%1000);
	if (markInterval<10) {
		snprintf(buf,bufSize,"%02d:%02d:%02d.%03d",h,m,s,ms);
	}
	else if (markInterval<100) {
		snprintf(buf,bufSize,"%02d:%02d:%02d.%02d",h,m,s,ms/10);
	}
	else if (markInterval<1000) {
		snprintf(buf,bufSize,"%02d:%02d:%02d.%01d",h,m,s,ms/100);
	}
	else if (markInterval<60000) {
		snprintf(buf,bufSize,"%02d:%02d:%02d",h,m,s);
	}
	else {
		snprintf(buf,bufSize,"%02d:%02d",h,m);
	}
	buf[bufSize-1]=0;
}


void emAvFileControlPanel::TextOfAudioVolume(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	snprintf(buf,bufSize,"%d%%",(int)value);
	buf[bufSize-1]=0;
}


void emAvFileControlPanel::TextOfAudioVisu(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	const emArray<emString> * arr;
	const char * p;
	int i;

	arr=&((emAvFileControlPanel*)context)->Mdl->GetAudioVisus();
	i=(int)value;
	if (i>=0 && i<arr->GetCount()) p=arr->Get(i).Get(); else p="";
	snprintf(buf,bufSize,"%s",p);
	buf[bufSize-1]=0;
}


void emAvFileControlPanel::TextOfAudioChannel(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	const emArray<emString> * arr;
	const char * p;
	int i;

	arr=&((emAvFileControlPanel*)context)->Mdl->GetAudioChannels();
	i=(int)value;
	if (i>=0 && i<arr->GetCount()) p=arr->Get(i).Get(); else p="";
	snprintf(buf,bufSize,"%s",p);
	buf[bufSize-1]=0;
}


void emAvFileControlPanel::TextOfSpuChannel(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	const emArray<emString> * arr;
	const char * p;
	int i;

	arr=&((emAvFileControlPanel*)context)->Mdl->GetSpuChannels();
	i=(int)value;
	if (i>=0 && i<arr->GetCount()) p=arr->Get(i).Get(); else p="";
	snprintf(buf,bufSize,"%s",p);
	buf[bufSize-1]=0;
}
