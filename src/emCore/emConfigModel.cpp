//------------------------------------------------------------------------------
// emConfigModel.cpp
//
// Copyright (C) 2006-2008,2011 Oliver Hamann.
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


void emConfigModel::TrySave(bool force) throw(emString)
{
	if (Unsaved || force) {
		GetRec().TrySave(InstallPath);
		if (Unsaved) {
			Unsaved=false;
			Signal(ChangeSignal);
		}
	}
}


void emConfigModel::Save(bool force)
{
	try {
		TrySave(force);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
}


void emConfigModel::SetAutoSaveDelaySeconds(int seconds)
{
	AutoSaveDelaySeconds=seconds;
	if (Unsaved && AutoSaveDelaySeconds>=0) {
		AutoSaveTimer.Start(((emUInt64)AutoSaveDelaySeconds)*1000);
	}
}


emConfigModel::emConfigModel(emContext & context, const emString & name)
	: emModel(context,name), Link(*this), AutoSaveTimer(GetScheduler())
{
	Unsaved=false;
	AutoSaveDelaySeconds=-1;
	AddWakeUpSignal(AutoSaveTimer.GetSignal());
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
			TrySave(true);
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


bool emConfigModel::Cycle()
{
	bool busy;

	busy=emModel::Cycle();
	if (
		IsSignaled(AutoSaveTimer.GetSignal()) &&
		AutoSaveDelaySeconds>=0
	) {
		Save();
	}
	return busy;
}


emConfigModel::RecLink::RecLink(emConfigModel & model)
	: Model(model)
{
}


void emConfigModel::RecLink::OnRecChanged()
{
	if (!Model.Unsaved) {
		Model.Unsaved=true;
		if (Model.AutoSaveDelaySeconds>=0) {
			Model.AutoSaveTimer.Start(((emUInt64)Model.AutoSaveDelaySeconds)*1000);
		}
	}
	Model.Signal(Model.ChangeSignal);
}
