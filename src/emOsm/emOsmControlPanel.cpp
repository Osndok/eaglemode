//------------------------------------------------------------------------------
// emOsmControlPanel.cpp
//
// Copyright (C) 2024 Oliver Hamann.
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

#include <emOsm/emOsmControlPanel.h>


emOsmControlPanel::emOsmControlPanel(
	ParentArg parent, const emString & name, emOsmFileModel & fileModel
) :
	emLinearGroup(parent,name,"Open Street Map"),
	FileModel(&fileModel),
	FileParamChanged(false),
	TfTilesUrl(NULL),
	SfMaxZ(NULL),
	BtApply(NULL),
	TfCacheDirectory(NULL),
	SfMaxCacheMegabytes(NULL),
	SfMaxCacheAgeDays(NULL)
{
	Config=emOsmConfig::Acquire(GetRootContext());

	AddWakeUpSignal(FileModel->GetFileStateSignal());
	AddWakeUpSignal(FileModel->GetChangeSignal());
	AddWakeUpSignal(Config->GetChangeSignal());
}


emOsmControlPanel::~emOsmControlPanel()
{
}


bool emOsmControlPanel::Cycle()
{
	bool busy;

	busy=emLinearGroup::Cycle();

	if (IsAutoExpanded()) {
		if (
			IsSignaled(FileModel->GetFileStateSignal()) ||
			IsSignaled(FileModel->GetChangeSignal()) ||
			IsSignaled(Config->GetChangeSignal())
		) {
			UpdateControls();
		}

		if (
			IsSignaled(TfTilesUrl->GetTextSignal()) ||
			IsSignaled(SfMaxZ->GetValueSignal())
		) {
			UpdateFileParamChanged();
		}

		if (IsSignaled(BtApply->GetClickSignal())) {
			Apply();
		}

		if (IsSignaled(SfMaxCacheMegabytes->GetValueSignal())) {
			Config->MaxCacheMegabytes.Set(
				MegabytesOfScalarFieldValue(SfMaxCacheMegabytes->GetValue())
			);
		}

		if (IsSignaled(SfMaxCacheAgeDays->GetValueSignal())) {
			Config->MaxCacheAgeDays.Set(
				SecondsOfScalarFieldValue(SfMaxCacheAgeDays->GetValue())
			);
		}
	}

	return busy;
}


void emOsmControlPanel::AutoExpand()
{
	emLinearGroup * lg;
	emLinearLayout * ll;

	emLinearGroup::AutoExpand();

	FileParamChanged=false;

	SetChildWeight(0,3.0);
	SetChildWeight(1,1.0);
	SetOrientationThresholdTallness(0.2);

	lg=new emLinearGroup(
		this,"file",
		"Current Map (" + FileModel->GetFilePath() + ")"
	);
	lg->SetVertical();
	lg->SetBorderScaling(2.0);

	ll=new emLinearLayout(lg,"params");
	ll->SetOrientationThresholdTallness(0.2);
	ll->SetChildWeight(0,2.4);
	ll->SetChildWeight(1,1.0);

	TfTilesUrl=new emTextField(
		ll,"TilesUrl",
		"Tiles URL",
		"URL of an OpenStreetMap compatible tile server. This must\n"
		"contain the following placeholders:\n"
		"  {z} - Replaced by the zoom level of tile.\n"
		"  {x} - Replaced by the x coordinate of the tile.\n"
		"  {y} - Replaced by the y coordinate of the tile.\n"
		"The tile file ending must be .png, .jpg, or .jpeg.",
		emImage(),
		"",
		true
	);
	AddWakeUpSignal(TfTilesUrl->GetTextSignal());

	SfMaxZ=new emScalarField(
		ll,"MaxZ",
		"Max Z",
		"Maximum value for the zoom level {z} supported\n"
		"by the tile server (most have 18 or 19).",
		emImage(),
		0,30,1,
		true
	);
	SfMaxZ->SetScaleMarkIntervals(10,5,1,0);
	AddWakeUpSignal(SfMaxZ->GetValueSignal());

	BtApply=new emButton(
		lg,"Apply",
		"Save And Apply Changes"
	);
	AddWakeUpSignal(BtApply->GetClickSignal());

	lg=new emLinearGroup(
		this,"config",
		"General Cache Settings",
		"This is related to all maps."
	);
	lg->SetOrientationThresholdTallness(0.15);
	lg->SetBorderScaling(2.0);

	TfCacheDirectory=new emTextField(
		lg,"CacheDirectory",
		"Cache Directory",
		"This is just informal and cannot be edited."
	);

	SfMaxCacheMegabytes=new emScalarField(
		lg,"MaxCacheSize",
		"Max Cache Size",
		"",
		emImage(),
		16,96,16,
		true
	);
	SfMaxCacheMegabytes->SetScaleMarkIntervals(16,4,1,0);
	SfMaxCacheMegabytes->SetTextOfValueFunc(&ScalarFieldTextOfMegabytesValue);
	AddWakeUpSignal(SfMaxCacheMegabytes->GetValueSignal());

	SfMaxCacheAgeDays=new emScalarField(
		lg,"MaxCacheAge",
		"Max Cache Age",
		"",
		emImage(),
		0,TimeTableSize-1,0,
		true
	);
	SfMaxCacheAgeDays->SetScaleMarkIntervals(5,1,0);
	SfMaxCacheAgeDays->SetTextOfValueFunc(&ScalarFieldTextOfSecondsValue);
	AddWakeUpSignal(SfMaxCacheAgeDays->GetValueSignal());

	UpdateControls();
}


void emOsmControlPanel::AutoShrink()
{
	FileParamChanged=false;
	TfTilesUrl=NULL;
	SfMaxZ=NULL;
	BtApply=NULL;
	TfCacheDirectory=NULL;
	SfMaxCacheMegabytes=NULL;
	SfMaxCacheAgeDays=NULL;

	emLinearGroup::AutoShrink();
}


emInt64 emOsmControlPanel::ScalarFieldValueOfMegabytes(int megabytes)
{
	return (emInt64)(log((double)megabytes)/log(10.0)*16.0+0.5);
}


int emOsmControlPanel::MegabytesOfScalarFieldValue(emInt64 value)
{
	return (int)(pow(10.0,((double)value)/16.0)+0.5);
}


void emOsmControlPanel::ScalarFieldTextOfMegabytesValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	double d;
	const char * u;
	char * p;
	int l;

	d=pow(10.0,((double)value)/16.0);
	if (d<1000) {
		u="MB";
		snprintf(buf,bufSize,"%.0f",d);
	}
	else if (d<1E6) {
		d/=1000.0;
		u="GB";
		snprintf(buf,bufSize,"%.1f",d);
	}
	else {
		d/=1E6;
		u="TB";
		snprintf(buf,bufSize,"%.1f",d);
	}

	buf[bufSize-1]=0;

	p=strchr(buf,'.');
	if (p && p[1]=='0' && p[2]==0) *p=0;

	l=strlen(buf);
	snprintf(buf+l,bufSize-l," %s",u);
}


emInt64 emOsmControlPanel::ScalarFieldValueOfSeconds(int seconds)
{
	int i1,i2,i;

	i1=0;
	i2=TimeTableSize-1;
	do {
		i=(i1+i2)>>1;
		if (seconds>TimeTable[i].Seconds) i1=i+1; else i2=i;
	} while (i1<i2);
	return i1;
}


int emOsmControlPanel::SecondsOfScalarFieldValue(emInt64 value)
{
	if (value<0) value=0;
	if (value>TimeTableSize-1) value=TimeTableSize-1;
	return TimeTable[value].Seconds;
}


void emOsmControlPanel::ScalarFieldTextOfSecondsValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	if (value<0) value=0;
	if (value>TimeTableSize-1) value=TimeTableSize-1;
	snprintf(buf,bufSize,"%s",TimeTable[value].Text);
}


void emOsmControlPanel::UpdateControls()
{
	emString cacheDir;

	if (!IsAutoExpanded()) return;

	if (
		FileModel->GetFileState() != emFileModel::FS_LOADED &&
		FileModel->GetFileState() != emFileModel::FS_UNSAVED
	) {
		TfTilesUrl->SetEnableSwitch(false);
		TfTilesUrl->SetText(emString());

		SfMaxZ->SetEnableSwitch(false);
		SfMaxZ->SetValue(SfMaxZ->GetMinValue());

		FileParamChanged=false;
		if (BtApply) BtApply->SetLook(GetLook());
	}
	else if (!FileParamChanged) {
		TfTilesUrl->SetEnableSwitch(true);
		TfTilesUrl->SetText(FileModel->TilesUrl);

		SfMaxZ->SetEnableSwitch(true);
		SfMaxZ->SetValue(FileModel->MaxZ);
	}

	BtApply->SetEnableSwitch(FileParamChanged);

	try {
		cacheDir=emOsmConfig::TryGetCacheDirectory();
	}
	catch (const emException & exception) {
		cacheDir=emString::Format("Error: %s",exception.GetText().Get());
	}
	TfCacheDirectory->SetText(cacheDir);

	SfMaxCacheMegabytes->SetValue(
		ScalarFieldValueOfMegabytes(Config->MaxCacheMegabytes)
	);

	SfMaxCacheAgeDays->SetValue(
		ScalarFieldValueOfSeconds(Config->MaxCacheAgeDays)
	);
}


void emOsmControlPanel::UpdateFileParamChanged()
{
	emLook look;

	if (!IsAutoExpanded()) {
		FileParamChanged=false;
		return;
	}

	FileParamChanged=(
		TfTilesUrl->GetText() != FileModel->TilesUrl ||
		SfMaxZ->GetValue() != FileModel->MaxZ
	);

	BtApply->SetEnableSwitch(FileParamChanged);
	if (FileParamChanged) {
		look=BtApply->GetLook();
		look.SetButtonFgColor(0xFF9988FF);
		BtApply->SetLook(look);
	}
	else {
		BtApply->SetLook(GetLook());
	}
}


void emOsmControlPanel::Apply()
{
	if (FileParamChanged) {
		FileModel->TilesUrl.Set(TfTilesUrl->GetText());
		FileModel->MaxZ.Set(SfMaxZ->GetValue());
		FileModel->Save(true);
		FileParamChanged=false;
		if (BtApply) BtApply->SetLook(GetLook());
	}

	UpdateControls();
}


const emOsmControlPanel::TimeEntry emOsmControlPanel::TimeTable[] = {
	{ 1, "1 Day" },
	{ 2, "2 Days" },
	{ 3, "3 Days" },
	{ 4, "4 Days" },
	{ 5, "5 Days" },
	{ 6, "6 Days" },
	{ 7, "7 Days" },
	{ 10, "10 Days" },
	{ 14, "14 Days" },
	{ 20, "20 Days" },
	{ 30, "30 Days" },
	{ 45, "45 Days" },
	{ 60, "60 Days" },
	{ 90, "90 Days" },
	{ 180, "180 Days" },
	{ 365, "365 Days" }
};


const int emOsmControlPanel::TimeTableSize=
	(int)(sizeof(TimeTable)/sizeof(TimeEntry))
;
