//------------------------------------------------------------------------------
// emConfigModel.cpp
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

#include <emCore/emConfigModel.h>


void emConfigModel::TrySave() throw(emString)
{
	GetRec().TrySave(InstallPath);
	if (Unsaved) {
		Unsaved=false;
		Signal(ChangeSignal);
	}
}


void emConfigModel::Save()
{
	try {
		TrySave();
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
}


emConfigModel::emConfigModel(emContext & context, const emString & name)
	: emModel(context,name), Link(*this)
{
	Unsaved=false;
}


void emConfigModel::PostConstruct(emRec & rec, const emString & installPath)
{
	Link.SetListenedRec(&rec);
	InstallPath=installPath;
}


emConfigModel::~emConfigModel()
{
}


void emConfigModel::TryLoad() throw(emString)
{
	GetRec().TryLoad(InstallPath);
	if (Unsaved) {
		Unsaved=false;
		Signal(ChangeSignal);
	}
}


void emConfigModel::Load()
{
	try {
		TryLoad();
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
}


void emConfigModel::TryLoadOrInstall(const char * insSrcPath) throw(emString)
{
	if (emIsExistingPath(InstallPath)) {
		TryLoad();
	}
	else {
		emTryMakeDirectories(emGetParentPath(InstallPath));
		if (insSrcPath) {
			emTryCopyFileOrTree(InstallPath,insSrcPath);
			TryLoad();
		}
		else {
			GetRec().SetToDefault();
			TrySave();
		}
	}
}


void emConfigModel::LoadOrInstall(const char * insSrcPath)
{
	try {
		TryLoadOrInstall(insSrcPath);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
}


emConfigModel::RecLink::RecLink(emConfigModel & model)
	: Model(model)
{
}


void emConfigModel::RecLink::OnRecChanged()
{
	Model.Unsaved=true;
	Model.Signal(Model.ChangeSignal);
}
