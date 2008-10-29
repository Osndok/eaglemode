//------------------------------------------------------------------------------
// emRes.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>


//==============================================================================
//============================== Resource Images ===============================
//==============================================================================

emImage emGetResImage(
	emRootContext & rootContext, const emString & filePath, int channelCount
)
{
	try {
		return emTryGetResImage(rootContext,filePath,channelCount);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
		return emImage();
	}
}


emImage emTryGetResImage(
	emRootContext & rootContext, const emString & filePath, int channelCount
) throw(emString)
{
	emRef<emResModel<emImage> > m;
	emArray<char> buf;
	emString absPath;
	emImage img;

	absPath=emGetAbsolutePath(filePath);

	m=emResModel<emImage>::Lookup(rootContext,absPath);
	if (m) {
		img=m->Get();
	}
	else {
		emDLog("emRes: Loading %s",absPath.Get());
		buf=emTryLoadFile(absPath);
		try {
			img.TryParseTga(
				(unsigned char*)buf.Get(),
				buf.GetCount()
			);
		}
		catch (emString errorMessage) {
			throw emString::Format(
				"Could not read image file \"%s\": %s",
				absPath.Get(),
				errorMessage.Get()
			);
		}
		buf.Empty();
		m=emResModel<emImage>::Acquire(rootContext,absPath);
		m->Set(img);
	}

	if (channelCount>=0 && img.GetChannelCount()!=channelCount) {
		throw emString::Format(
			"Image file \"%s\" does not have %d channels",
			absPath.Get(),
			channelCount
		);
	}

	return img;
}


emImage emGetInsResImage(
	emRootContext & rootContext, const char * prj, const char * subPath,
	int channelCount
)
{
	return emGetResImage(
		rootContext,
		emGetInstallPath(EM_IDT_RES,prj,subPath),
		channelCount
	);
}


emImage emTryGetInsResImage(
	emRootContext & rootContext, const char * prj, const char * subPath,
	int channelCount
) throw(emString)
{
	return emTryGetResImage(
		rootContext,
		emGetInstallPath(EM_IDT_RES,prj,subPath),
		channelCount
	);
}


//==============================================================================
//=============================== emResModelBase ===============================
//==============================================================================

emResModelBase::emResModelBase(emContext & context, const emString & name)
	: emModel(context,name)
{
	AddWakeUpSignal(
		PollTimer::Acquire(GetRootContext())->Timer.GetSignal()
	);
}


bool emResModelBase::Cycle()
{
	if (GetDataRefCount()<=1 && (int)GetMinCommonLifetime()<0) {
		SetMinCommonLifetime(10);
			//??? Should be configurable somehow. But remember that
			//??? the garbage collector of the context may have a
			//??? quite large time resolution (=> saying 1 instead
			//??? of 10 here may not be very effective). And there's
			//??? even the time resolution of our PollTimer (see
			//??? below).
	}
	return false;
}


emRef<emResModelBase::PollTimer> emResModelBase::PollTimer::Acquire(
	emRootContext & rootContext
)
{
	EM_IMPL_ACQUIRE_COMMON(emResModelBase::PollTimer,rootContext,"")
}


emResModelBase::PollTimer::PollTimer(
	emContext & context, const emString & name
)
	: emModel(context,name), Timer(GetScheduler())
{
	SetMinCommonLifetime(UINT_MAX);
	Timer.Start(
		4000, //??? Should be configurable somehow...
		true
	);
}


emResModelBase::PollTimer::~PollTimer()
{
}
