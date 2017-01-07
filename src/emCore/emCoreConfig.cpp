//------------------------------------------------------------------------------
// emCoreConfig.cpp
//
// Copyright (C) 2006-2012,2014,2016 Oliver Hamann.
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
	MouseZoomSpeed(this,"MouseZoomSpeed",1.0,0.25,4.0),
	MouseScrollSpeed(this,"MouseScrollSpeed",1.0,0.25,4.0),
	MouseWheelZoomSpeed(this,"MouseWheelZoomSpeed",1.0,0.25,4.0),
	MouseWheelZoomAcceleration(this,"MouseWheelZoomAcceleration",1.0,0.25,2.0),
	KeyboardZoomSpeed(this,"KeyboardZoomSpeed",1.0,0.25,4.0),
	KeyboardScrollSpeed(this,"KeyboardScrollSpeed",1.0,0.25,4.0),
	KineticZoomingAndScrolling(this,"KineticZoomingAndScrolling",1.0,0.25,2.0),
	MagnetismRadius(this,"MagnetismRadius",1.0,0.25,4.0),
	MagnetismSpeed(this,"MagnetismSpeed",1.0,0.25,4.0),
	VisitSpeed(this,"VisitSpeed",1.0,0.1,10.0),
	MaxMegabytesPerView(
		this,"MaxMegabytesPerView",
#if defined(ANDROID)
		64,
#else
		1024,
#endif
		8,16384
	),
	MaxRenderThreads(this,"MaxRenderThreads",8,1,32)
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emCore","config.rec")
	);

	try {
		TryLoadOrInstall();
	}
	catch (emException &) {
		try {
			TryLoadOldVersion();
			Save(true);
		}
		catch (emException &) {
			LoadOrInstall();
		}
	}
}


emCoreConfig::~emCoreConfig()
{
}


class emCoreConfig_0_85_0 : public emStructRec {
public:
	emBoolRec StickMouseWhenNavigating;
	emBoolRec EmulateMiddleButton;
	emBoolRec PanFunction;
	emDoubleRec MouseZoomSpeedFactor;
	emDoubleRec MouseFineZoomSpeedFactor;
	emDoubleRec MouseScrollSpeedFactor;
	emDoubleRec MouseFineScrollSpeedFactor;
	emDoubleRec WheelZoomSpeedFactor;
	emDoubleRec WheelFineZoomSpeedFactor;
	emDoubleRec KeyboardZoomSpeedFactor;
	emDoubleRec KeyboardFineZoomSpeedFactor;
	emDoubleRec KeyboardScrollSpeedFactor;
	emDoubleRec KeyboardFineScrollSpeedFactor;
	emIntRec MaxMegabytesPerView;

	virtual const char * GetFormatName() const
	{
		return "emCoreConfig";
	}

	emCoreConfig_0_85_0()
		: emStructRec(),
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
		MaxMegabytesPerView(this,"MaxMegabytesPerView",512,8,16384)
	{
	}
};


void emCoreConfig::TryLoadOldVersion() throw(emException)
{
	emCoreConfig_0_85_0 oldCfg;
	oldCfg.TryLoad(GetInstallPath());
	SetToDefault();
	StickMouseWhenNavigating = oldCfg.StickMouseWhenNavigating.Get();
	EmulateMiddleButton = oldCfg.EmulateMiddleButton.Get();
	PanFunction = oldCfg.PanFunction.Get();
	MouseZoomSpeed = oldCfg.MouseZoomSpeedFactor.Get();
	MouseScrollSpeed = oldCfg.MouseScrollSpeedFactor.Get();
	MouseWheelZoomSpeed = oldCfg.WheelZoomSpeedFactor.Get();
	KeyboardZoomSpeed = oldCfg.KeyboardZoomSpeedFactor.Get();
	KeyboardScrollSpeed = oldCfg.KeyboardScrollSpeedFactor.Get();
	MaxMegabytesPerView = oldCfg.MaxMegabytesPerView.Get();
}
