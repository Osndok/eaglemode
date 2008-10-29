//------------------------------------------------------------------------------
// emAvFileModel.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emAv/emAvFileModel.h>



emString emAvFileModel::MakeName(
	const emString & filePath, const emString & serverProcPath
)
{
	return emString::Format(
		"%d,%s,%s",
		filePath.GetLen(),
		filePath.Get(),
		serverProcPath.Get()
	);
}


emRef<emAvFileModel> emAvFileModel::Acquire(
	emContext & context, const emString & filePath,
	const emString & serverProcPath, bool common
)
{
	emAvFileModel * m;
	emString name;

	name=MakeName(filePath,serverProcPath);
	if (!common) {
		m=new emAvFileModel(context,name,filePath,serverProcPath);
	}
	else {
		m=(emAvFileModel*)context.Lookup(typeid(emAvFileModel),name);
		if (!m) {
			m=new emAvFileModel(context,name,filePath,serverProcPath);
			m->Register();
		}
	}
	return emRef<emAvFileModel>(m);
}


const emString & emAvFileModel::GetFilePath() const
{
	return FilePath;
}


void emAvFileModel::SetPlayState(PlayStateType playState)
{
	if (GetFileState()!=FS_LOADED) return;
	if (PlayState==playState) return;

	PlayState=playState;
	Signal(PlayStateSignal);

	if (PlayState==PS_STOPPED) {
		RemoveFromActiveList();
		CloseStream();
		PlayPos=0;
		Signal(PlayPosSignal);
		Image.Empty();
		Signal(ImageSignal);
	}
	else {
		AddToActiveList();
		if (GetStreamState()!=STREAM_OPENING && GetStreamState()!=STREAM_OPENED) {
			if (!WarningText.IsEmpty() || !ErrorText.IsEmpty()) {
				WarningText.Empty();
				ErrorText.Empty();
				Signal(InfoSignal);
			}
			OpenStream("auto","emAv",GetFilePath());
#if 0
			//??? This would still be a bad idea because the "none"
			//??? drivers may produce useless initial values.
			SetProperty("pos",emString::Format("%d",PlayPos));
			SetProperty("audio_volume",emString::Format("%d",AudioVolume));
			SetProperty("audio_mute",AudioMute?"on":"off");
			if (AudioVisu>=0 && AudioVisu<AudioVisus.GetCount()) {
				SetProperty("audio_visu",AudioVisus[AudioVisu].Get());
			}
			if (AudioChannel>=0 && AudioChannel<AudioChannels.GetCount()) {
				SetProperty("audio_channel",AudioChannels[AudioChannel].Get());
			}
			if (SpuChannel>=0 && SpuChannel<SpuChannels.GetCount()) {
				SetProperty("spu_channel",SpuChannels[SpuChannel].Get());
			}
#endif
		}
		SetProperty(
			"state",
			PlayState==PS_PAUSED ? "paused" :
			PlayState==PS_SLOW ? "slow" :
			PlayState==PS_FAST ? "fast" :
			"normal"
		);
	}
}


void emAvFileModel::StopAll(emRootContext & rootContext)
{
	emRef<emVarModel<emAvFileModel*> > activeList;

	activeList=emVarModel<emAvFileModel*>::Lookup(
		rootContext,"emAvFileModel::ActiveList"
	);
	if (activeList) {
		while (activeList->Var) activeList->Var->Stop();
	}
}


void emAvFileModel::PlaySolely()
{
	if (GetFileState()!=FS_LOADED) return;
	while (ActiveList->Var && ActiveList->Var!=this) ActiveList->Var->Stop();
	while (ALNext) ALNext->Stop();
	Play();
}


void emAvFileModel::SetPlayPos(int playPos)
{
	if (GetFileState()!=FS_LOADED) return;
	if (playPos<0) playPos=0;
	if (playPos>PlayLength) playPos=PlayLength;
	if (PlayPos!=playPos) {
		if (PlayState==PS_STOPPED) Pause();
		PlayPos=playPos;
		Signal(PlayPosSignal);
		SetProperty("pos",emString::Format("%d",PlayPos));
	}
}


void emAvFileModel::SetAudioVolume(int audioVolume)
{
	if (GetFileState()!=FS_LOADED) return;
	if (audioVolume<0) audioVolume=0;
	if (audioVolume>100) audioVolume=100;
	if (AudioVolume!=audioVolume) {
		AudioVolume=audioVolume;
		Signal(AdjustmentSignal);
		SetProperty("audio_volume",emString::Format("%d",AudioVolume));
	}
}


void emAvFileModel::SetAudioMute(bool audioMute)
{
	if (GetFileState()!=FS_LOADED) return;
	if (AudioMute!=audioMute) {
		AudioMute=audioMute;
		Signal(AdjustmentSignal);
		SetProperty("audio_mute",AudioMute?"on":"off");
	}
}


void emAvFileModel::SetAudioVisu(int audioVisu)
{
	int n;

	if (GetFileState()!=FS_LOADED) return;
	n=AudioVisus.GetCount();
	if (n>0) {
		if (audioVisu<0) audioVisu=0;
		if (audioVisu>=n) audioVisu=n-1;
		if (AudioVisu!=audioVisu) {
			AudioVisu=audioVisu;
			Signal(AdjustmentSignal);
			SetProperty("audio_visu",AudioVisus[audioVisu].Get());
		}
	}
}


void emAvFileModel::SetAudioChannel(int audioChannel)
{
	int n;

	if (GetFileState()!=FS_LOADED) return;
	n=AudioChannels.GetCount();
	if (n>0) {
		if (audioChannel<0) audioChannel=0;
		if (audioChannel>=n) audioChannel=n-1;
		if (AudioChannel!=audioChannel) {
			AudioChannel=audioChannel;
			Signal(AdjustmentSignal);
			SetProperty("audio_channel",AudioChannels[audioChannel].Get());
		}
	}
}


void emAvFileModel::SetSpuChannel(int spuChannel)
{
	int n;

	if (GetFileState()!=FS_LOADED) return;
	n=SpuChannels.GetCount();
	if (n>0) {
		if (spuChannel<0) spuChannel=0;
		if (spuChannel>=n) spuChannel=n-1;
		if (SpuChannel!=spuChannel) {
			SpuChannel=spuChannel;
			Signal(AdjustmentSignal);
			SetProperty("spu_channel",SpuChannels[spuChannel].Get());
		}
	}
}


emAvFileModel::emAvFileModel(
	emContext & context, const emString & name, const emString & filePath,
	const emString & serverProcPath
)
	: emFileModel(context,name),
	emAvClient(emAvServerModel::Acquire(context.GetRootContext(),serverProcPath))
{
	FilePath=filePath;

	ActiveList=emVarModel<emAvFileModel*>::Lookup(
		GetRootContext(),"emAvFileModel::ActiveList"
	);
	if (!ActiveList) {
		ActiveList=emVarModel<emAvFileModel*>::Acquire(
			GetRootContext(),"emAvFileModel::ActiveList"
		);
		ActiveList->Var=NULL;
	}

	ALNext=NULL;
	ALThisPtr=NULL;

	Video=false;
	PlayLength=0;
	PlayState=PS_STOPPED;
	PlayPos=0;
	AudioVolume=0;
	AudioMute=false;
	AudioVisu=0;
	AudioChannel=0;
	SpuChannel=0;
	Tallness=1.0;
}


emAvFileModel::~emAvFileModel()
{
	emAvFileModel::QuitLoading();
	emAvFileModel::ResetData();
}


void emAvFileModel::ResetData()
{
	CloseStream();

	Video=false;
	PlayLength=0;
	InfoText.Empty();
	WarningText.Empty();
	ErrorText.Empty();
	AudioVisus.Empty(true);
	AudioChannels.Empty(true);
	SpuChannels.Empty(true);
	Signal(InfoSignal);

	PlayState=PS_STOPPED;
	RemoveFromActiveList();
	Signal(PlayStateSignal);

	PlayPos=0;
	Signal(PlayPosSignal);

	AudioVolume=0;
	AudioMute=false;
	AudioVisu=0;
	AudioChannel=0;
	SpuChannel=0;
	Signal(AdjustmentSignal);

	Image.Empty();
	Tallness=1.0;
	Signal(ImageSignal);
}


void emAvFileModel::TryStartLoading() throw(emString)
{
}


bool emAvFileModel::TryContinueLoading() throw(emString)
{
	switch (GetStreamState()) {
	case STREAM_CLOSED:
		OpenStream("none","none",GetFilePath());
		return false;
	case STREAM_OPENED:
		CloseStream();
		return true;
	case STREAM_ERRORED:
		throw emString(GetStreamErrorText());
	default:
		emSleepMS(10);
		return false;
	}
}


void emAvFileModel::QuitLoading()
{
}


void emAvFileModel::TryStartSaving() throw(emString)
{
	throw emString("emAvFileModel: Saving not possible.");
}


bool emAvFileModel::TryContinueSaving() throw(emString)
{
	return false;
}


void emAvFileModel::QuitSaving()
{
}


emUInt64 emAvFileModel::CalcMemoryNeed()
{
	return 30000000;
}


double emAvFileModel::CalcFileProgress()
{
	return 50.0;
}


void emAvFileModel::StreamStateChanged(StreamStateType streamState)
{
	emString str;

	if (streamState==STREAM_ERRORED && GetFileState()==FS_LOADED) {
		str=GetStreamErrorText();
		if (ErrorText!=str) {
			ErrorText=str;
			Signal(InfoSignal);
		}
		if (PlayState!=PS_STOPPED) {
			RemoveFromActiveList();
			PlayState=PS_STOPPED;
			Signal(PlayStateSignal);
		}
		if (PlayPos!=0) {
			PlayPos=0;
			Signal(PlayPosSignal);
		}
		if (!Image.IsEmpty()) {
			Image.Empty();
			Signal(ImageSignal);
		}
	}
}


void emAvFileModel::PropertyChanged(const emString & name, const emString & value)
{
	PlayStateType ps;
	double d;
	bool b;
	int i;

	if (name=="type") {
		b=(value=="video");
		if (Video!=b) {
			Video=b;
			Signal(InfoSignal);
		}
	}
	else if (name=="length") {
		i=atoi(value);
		if (i<0) i=0;
		if (PlayLength!=i) {
			PlayLength=i;
			Signal(InfoSignal);
			if (PlayPos>PlayLength) {
				PlayPos=PlayLength;
				Signal(PlayPosSignal);
			}
		}
	}
	else if (name=="aspect") {
		i=atoi(value);
		if (i>0) {
			d=65536.0/i;
			if (Tallness!=d) {
				Tallness=d;
				Signal(ImageSignal);
			}
		}
	}
	else if (name=="info") {
		if (InfoText!=value) {
			InfoText=value;
			Signal(InfoSignal);
		}
	}
	else if (name=="warning") {
		if (WarningText!=value) {
			WarningText=value;
			Signal(InfoSignal);
		}
	}
	else if (name=="audio_visus") {
		if (UpdateStringArray(AudioVisus,value)) {
			Signal(InfoSignal);
			if (AudioVisu>=AudioVisus.GetCount()) {
				AudioVisu=0;
				Signal(AdjustmentSignal);
			}
		}
	}
	else if (name=="audio_channels") {
		if (UpdateStringArray(AudioChannels,value)) {
			Signal(InfoSignal);
			if (AudioChannel>=AudioChannels.GetCount()) {
				AudioChannel=0;
				Signal(AdjustmentSignal);
			}
		}
	}
	else if (name=="spu_channels") {
		if (UpdateStringArray(SpuChannels,value)) {
			Signal(InfoSignal);
			if (SpuChannel>=SpuChannels.GetCount()) {
				SpuChannel=0;
				Signal(AdjustmentSignal);
			}
		}
	}
	else if (name=="state") {
		if (value=="paused") ps=PS_PAUSED;
		else if (value=="normal") ps=PS_NORMAL;
		else if (value=="fast") ps=PS_FAST;
		else if (value=="slow") ps=PS_SLOW;
		else ps=PS_STOPPED;
		if (PlayState!=ps) {
			PlayState=ps;
			Signal(PlayStateSignal);
			if (ps==PS_STOPPED) RemoveFromActiveList();
			else AddToActiveList();
			if (PlayState==PS_STOPPED && GetFileState()==FS_LOADED) {
				CloseStream();
				PlayPos=0;
				Signal(PlayPosSignal);
				Image.Empty();
				Signal(ImageSignal);
			}
		}
	}
	else if (name=="pos") {
		i=atoi(value);
		if (i<0) i=0;
		if (i>PlayLength) i=PlayLength;
		if (PlayPos!=i) {
			PlayPos=i;
			Signal(PlayPosSignal);
		}
	}
	else if (name=="audio_volume") {
		i=atoi(value);
		if (i<0) i=0;
		if (i>100) i=100;
		if (AudioVolume!=i) {
			AudioVolume=i;
			Signal(AdjustmentSignal);
		}
	}
	else if (name=="audio_mute") {
		b=(value=="on");
		if (AudioMute!=b) {
			AudioMute=b;
			Signal(AdjustmentSignal);
		}
	}
	else if (name=="audio_visu") {
		for (i=AudioVisus.GetCount()-1; i>=0; i--) {
			if (AudioVisus[i]==value) break;
		}
		if (i>=0 && AudioVisu!=i) {
			AudioVisu=i;
			Signal(AdjustmentSignal);
		}
	}
	else if (name=="audio_channel") {
		for (i=AudioChannels.GetCount()-1; i>=0; i--) {
			if (AudioChannels[i]==value) break;
		}
		if (i>=0 && AudioChannel!=i) {
			AudioChannel=i;
			Signal(AdjustmentSignal);
		}
	}
	else if (name=="spu_channel") {
		for (i=SpuChannels.GetCount()-1; i>=0; i--) {
			if (SpuChannels[i]==value) break;
		}
		if (i>=0 && SpuChannel!=i) {
			SpuChannel=i;
			Signal(AdjustmentSignal);
		}
	}
	else {
		emDLog(
			"emAvFileModel::PropertyChanged: Unsupported property name \"%s\".",
			name.Get()
		);
	}
}


void emAvFileModel::ShowFrame(const emImage & image, double tallness)
{
	Image=image;
	Tallness=tallness;
	Signal(ImageSignal);
}


void emAvFileModel::AddToActiveList()
{
	if (!ALThisPtr) {
		ALNext=ActiveList->Var;
		if (ALNext) ALNext->ALThisPtr=&ALNext;
		ALThisPtr=&ActiveList->Var;
		ActiveList->Var=this;
	}
}


void emAvFileModel::RemoveFromActiveList()
{
	if (ALThisPtr) {
		*ALThisPtr=ALNext;
		if (ALNext) {
			ALNext->ALThisPtr=ALThisPtr;
			ALNext=NULL;
		}
		ALThisPtr=NULL;
	}
}


bool emAvFileModel::UpdateStringArray(
	emArray<emString> & arr, const emString & val
)
{
	const char * p1, * p2;
	emString elem;
	bool changed;
	int i;

	changed=false;
	i=0;
	p1=val.Get();
	do {
		p2=strchr(p1,':');
		if (!p2) {
			elem=emString(p1);
			p1=NULL;
		}
		else {
			elem=emString(p1,p2-p1);
			p1=p2+1;
		}
		if (arr.GetCount()<=i) {
			arr.Add(elem);
			changed=true;
		}
		else if (arr[i]!=elem) {
			arr.Set(i,elem);
			changed=true;
		}
		i++;
	} while (p1);
	if (arr.GetCount()>i) {
		arr.Remove(i,arr.GetCount()-i);
		changed=true;
	}
	return changed;
}
