//------------------------------------------------------------------------------
// emMainConfig.cpp
//
// Copyright (C) 2010 Oliver Hamann.
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
#include <emMain/emMainConfig.h>


emRef<emMainConfig> emMainConfig::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emMainConfig,rootContext,"")
}


const char * emMainConfig::GetFormatName() const
{
	return "emMainConfig";
}


emMainConfig::emMainConfig(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	AutoHideControlView(this,"AutoHideControlView",false),
	AutoHideSlider(this,"AutoHideSlider",false),
	ControlViewSize(this,"ControlViewSize",0.7,0.0,1.0)
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","config.rec")
	);
	LoadOrInstall();
}


emMainConfig::~emMainConfig()
{
}
