//------------------------------------------------------------------------------
// emAvStates.cpp
//
// Copyright (C) 2011 Oliver Hamann.
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

#include <emAv/emAvStates.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emCoreConfig.h>


emRef<emAvStates> emAvStates::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emAvStates,rootContext,"")
}


emAvStates::FileStateRec::FileStateRec()
	: emStructRec(),
	FilePath(this,"FilePath"),
	PlayLength(this,"PlayLength"),
	PlayPos(this,"PlayPos"),
	AudioChannel(this,"AudioChannel"),
	SpuChannel(this,"SpuChannel")
{
}


emAvStates::FileStateRec::~FileStateRec()
{
}


const char * emAvStates::GetFormatName() const
{
	return "emAvStates";
}


emAvStates::emAvStates(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	AudioVolume(this,"AudioVolume",100,0,100),
	AudioVisu(this,"AudioVisu"),
	MaxAudioStates(this,"MaxAudioStates",100,0,INT_MAX),
	AudioStates(this,"AudioStates"),
	MaxVideoStates(this,"MaxVideoStates",100,0,INT_MAX),
	VideoStates(this,"VideoStates")
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emAv","states.rec")
	);
	SetMinCommonLifetime(20);
	SetAutoSaveDelaySeconds(10);
	LoadOrInstall();
}


emAvStates::~emAvStates()
{
	Save();
}
