//------------------------------------------------------------------------------
// emAvFileModel.h
//
// Copyright (C) 2005-2008,2011 Oliver Hamann.
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

#ifndef emAvFileModel_h
#define emAvFileModel_h

#ifndef emVarModel_h
#include <emCore/emVarModel.h>
#endif

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emAvStates_h
#include <emAv/emAvStates.h>
#endif

#ifndef emAvClient_h
#include <emAv/emAvClient.h>
#endif


class emAvFileModel : public emFileModel, private emAvClient {

public:

	static emString MakeName(
		const emString & filePath, const emString & serverProcPath
	);

	static emRef<emAvFileModel> Acquire(
		emContext & context, const emString & filePath,
		const emString & serverProcPath, bool common=true
	);

	virtual const emString & GetFilePath() const;


	// - - - Infos - - -

	const emSignal & GetInfoSignal() const;

	bool IsVideo() const;

	int GetPlayLength() const;

	const emString & GetInfoText() const;

	const emString & GetWarningText() const;

	const emArray<emString> & GetAudioVisus() const;

	const emArray<emString> & GetAudioChannels() const;

	const emArray<emString> & GetSpuChannels() const;


	// - - - Play State - - -

	const emSignal & GetPlayStateSignal() const;

	enum PlayStateType {
		PS_STOPPED,
		PS_PAUSED,
		PS_NORMAL,
		PS_FAST,
		PS_SLOW
	};

	PlayStateType GetPlayState() const;
	void SetPlayState(PlayStateType playState);

	void Stop();
	void Pause();
	void Play();
	void PlaySlow();
	void PlayFast();

	static void StopAll(emRootContext & rootContext);
	void PlaySolely();


	// - - - Play Pos - - -

	const emSignal & GetPlayPosSignal() const;

	int GetPlayPos() const;
	void SetPlayPos(int playPos);


	// - - - Adjustment - - -

	const emSignal & GetAdjustmentSignal() const;

	int GetAudioVolume() const;
	void SetAudioVolume(int audioVolume);

	bool GetAudioMute() const;
	void SetAudioMute(bool audioMute);

	int GetAudioVisu() const;
	void SetAudioVisu(int audioVisu);

	int GetAudioChannel() const;
	void SetAudioChannel(int audioChannel);

	int GetSpuChannel() const;
	void SetSpuChannel(int spuChannel);


	// - - - Image - - -

	const emSignal & GetImageSignal() const;

	const emImage & GetImage() const;

	double GetTallness() const;


	// - - - - - - - - -

protected:

	emAvFileModel(
		emContext & context, const emString & name,
		const emString & filePath, const emString & serverProcPath
	);

	virtual ~emAvFileModel();

	virtual void ResetData();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

	virtual void StreamStateChanged(StreamStateType streamState);
	virtual void PropertyChanged(const emString & name, const emString & value);
	virtual void ShowFrame(const emImage & image, double tallness);

private:

	void AddToActiveList();
	void RemoveFromActiveList();

	void SaveFileState();
	void LoadFileState();
	void SaveAudioVolume();
	void LoadAudioVolume();
	void SaveAudioVisu();
	void LoadAudioVisu();

	static bool UpdateStringArray(emArray<emString> & arr, const emString & val);

	emString FilePath;

	emRef<emAvStates> States;

	emRef<emVarModel<emAvFileModel*> > ActiveList;
	emAvFileModel * ALNext;
	emAvFileModel * * ALThisPtr;

	emSignal InfoSignal;
	bool Video;
	int PlayLength;
	emString InfoText;
	emString WarningText;
	emString ErrorText;
	emArray<emString> AudioVisus;
	emArray<emString> AudioChannels;
	emArray<emString> SpuChannels;

	emSignal PlayStateSignal;
	PlayStateType PlayState;

	emSignal PlayPosSignal;
	int PlayPos;

	emSignal AdjustmentSignal;
	int AudioVolume;
	bool AudioMute;
	int AudioVisu;
	int AudioChannel;
	int SpuChannel;

	emSignal ImageSignal;
	emImage Image;
	double Tallness;
};

inline const emSignal & emAvFileModel::GetInfoSignal() const
{
	return InfoSignal;
}

inline bool emAvFileModel::IsVideo() const
{
	return Video;
}

inline int emAvFileModel::GetPlayLength() const
{
	return PlayLength;
}

inline const emString & emAvFileModel::GetInfoText() const
{
	return InfoText;
}

inline const emString & emAvFileModel::GetWarningText() const
{
	return ErrorText.IsEmpty() ? WarningText : ErrorText;
}

inline const emArray<emString> & emAvFileModel::GetAudioVisus() const
{
	return AudioVisus;
}

inline const emArray<emString> & emAvFileModel::GetAudioChannels() const
{
	return AudioChannels;
}

inline const emArray<emString> & emAvFileModel::GetSpuChannels() const
{
	return SpuChannels;
}

inline const emSignal & emAvFileModel::GetPlayStateSignal() const
{
	return PlayStateSignal;
}

inline emAvFileModel::PlayStateType emAvFileModel::GetPlayState() const
{
	return PlayState;
}

inline void emAvFileModel::Stop()
{
	SetPlayState(PS_STOPPED);
}

inline void emAvFileModel::Pause()
{
	SetPlayState(PS_PAUSED);
}

inline void emAvFileModel::Play()
{
	SetPlayState(PS_NORMAL);
}

inline void emAvFileModel::PlaySlow()
{
	SetPlayState(PS_SLOW);
}

inline void emAvFileModel::PlayFast()
{
	SetPlayState(PS_FAST);
}

inline const emSignal & emAvFileModel::GetPlayPosSignal() const
{
	return PlayPosSignal;
}

inline int emAvFileModel::GetPlayPos() const
{
	return PlayPos;
}

inline const emSignal & emAvFileModel::GetAdjustmentSignal() const
{
	return AdjustmentSignal;
}

inline int emAvFileModel::GetAudioVolume() const
{
	return AudioVolume;
}

inline bool emAvFileModel::GetAudioMute() const
{
	return AudioMute;
}

inline int emAvFileModel::GetAudioVisu() const
{
	return AudioVisu;
}

inline int emAvFileModel::GetAudioChannel() const
{
	return AudioChannel;
}

inline int emAvFileModel::GetSpuChannel() const
{
	return SpuChannel;
}

inline const emSignal & emAvFileModel::GetImageSignal() const
{
	return ImageSignal;
}

inline const emImage & emAvFileModel::GetImage() const
{
	return Image;
}

inline double emAvFileModel::GetTallness() const
{
	return Tallness;
}


#endif
