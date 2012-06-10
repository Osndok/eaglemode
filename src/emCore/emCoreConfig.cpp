//------------------------------------------------------------------------------
// emCoreConfig.cpp
//
// Copyright (C) 2006-2012 Oliver Hamann.
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
#include <emCore/emCoreConfig.h>


emRef<emCoreConfig> emCoreConfig::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emCoreConfig,rootContext,"")
}


const char * emCoreConfig::GetFormatName() const
{
	return "emCoreConfig";
}


emCoreConfig::emCoreConfig(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	StickMouseWhenNavigating(this,"StickMouseWhenNavigating",false),
	EmulateMiddleButton(this,"EmulateMiddleButton",false),
	PanFunction(this,"PanFunction",false),
	MouseZoomSpeedFactor(this,"MouseZoomSpeedFactor",1.0,0.25,4.0),
	MouseFineZoomSpeedFactor(this,"MouseFineZoomSpeedFactor",1.0,0.25,4.0),
	MouseScrollSpeedFactor(this,"MouseScrollSpeedFactor",1.0,0.25,4.0),
	MouseFineScrollSpeedFactor(this,"MouseFineScrollSpeedFactor",1.0,0.25,4.0),
	WheelZoomSpeedFactor(this,"WheelZoomSpeedFactor",1.0,0.25,4.0),
	WheelFineZoomSpeedFactor(this,"WheelFineZoomSpeedFactor",1.0,0.25,4.0),
	KeyboardZoomSpeedFactor(this,"KeyboardZoomSpeedFactor",1.0,0.25,4.0),
	KeyboardFineZoomSpeedFactor(this,"KeyboardFineZoomSpeedFactor",1.0,0.25,4.0),
	KeyboardScrollSpeedFactor(this,"KeyboardScrollSpeedFactor",1.0,0.25,4.0),
	KeyboardFineScrollSpeedFactor(this,"KeyboardFineScrollSpeedFactor",1.0,0.25,4.0),
	MaxMegabytesPerView(
		this,"MaxMegabytesPerView",
#if defined(ANDROID)
		64,
#else
		512,
#endif
		8,16384
	)
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emCore","config.rec")
	);
	LoadOrInstall();
}


emCoreConfig::~emCoreConfig()
{
}
