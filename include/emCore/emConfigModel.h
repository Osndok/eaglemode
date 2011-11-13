//------------------------------------------------------------------------------
// emConfigModel.h
//
// Copyright (C) 2006-2008,2010-2011 Oliver Hamann.
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

#ifndef emConfigModel_h
#define emConfigModel_h

#ifndef emRec_h
#include <emCore/emRec.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//=============================== emConfigModel ================================
//==============================================================================

class emConfigModel : public emModel {

public:

	// Base class for a configuration model where the data is stored in an
	// emRec. This class is a little bit similar to emRecFileModel, but it
	// is not an emFileModel. The idea here is to have a loaded state
	// always.

	const emString & GetInstallPath() const;
		// Get the installation path/name of the configuration file.

	bool IsUnsaved() const;
		// Ask whether the record data is currently not saved to the
		// file.

	const emSignal & GetChangeSignal() const;
		// Signaled on every modification of the record and even on
		// change of IsUnsaved.

	void TrySave(bool force=false) throw(emString);
	void Save(bool force=false);
		// Save the record to the install path. On error, the first
		// version throws the error message, and the second version
		// calls emFatalError.
		// Arguments:
		//   force - false to save only if unsaved, true to save in
		//           any case.

	int GetAutoSaveDelaySeconds() const;
	void SetAutoSaveDelaySeconds(int seconds);
		// Delay in seconds after which the record is saved
		// automatically when it was modified. The default is -1 which
		// means to disable the auto-save feature. If you make use of
		// this feature, remember to call Save in desctructors of
		// derived classes.

protected:

	emConfigModel(emContext & context, const emString & name);
		// Do not forget to call PostConstruct from an overloaded
		// constructor.

	void PostConstruct(emRec & rec, const emString & installPath);
		// Must be called by the constructor of the derived class after
		// the record has been constructed.
		// Arguments:
		//   rec         - The record making up the configuration.
		//   installPath - Path/name of the configuration file, usually
		//                 a result of calling the global function
		//                 emGetInstallPath.

	virtual ~emConfigModel();
		// Destructor.

	void TryLoad() throw(emString);
	void Load();
		// Load the record from the install path. On error, the first
		// version throws the error message, the second version calls
		// emFatalError. This is typically called by the constructor of
		// the derived class or by a TryAcquire function. Hint:
		// Derivatives of emFilePanel may easily report an error with
		// its SetCustomError method, this is better than calling
		// emFatalError.

	void TryLoadOrInstall(const char * insSrcPath=NULL) throw(emString);
	void LoadOrInstall(const char * insSrcPath=NULL);
		// Like above, but if the file does not yet exist, an
		// installation is performed: If insSrcPath is not NULL, that
		// file is copied to the install path and loaded. Otherwise the
		// record is set to its default state and saved to the install
		// path. Any non existing parent directories of the install path
		// are created automatically.

	emRec & GetRec();
		// Not valid before PostConstruct has been called.

	virtual bool Cycle();
		// Implements the auto-save feature.

private:

	class RecLink : public emRecListener {
	public:
		RecLink(emConfigModel & model);
	protected:
		virtual void OnRecChanged();
	private:
		emConfigModel & Model;
	};
	friend class RecLink;

	emSignal ChangeSignal;
	RecLink Link;
	emString InstallPath;
	bool Unsaved;
	emTimer AutoSaveTimer;
	int AutoSaveDelaySeconds;
};

inline const emString & emConfigModel::GetInstallPath() const
{
	return InstallPath;
}

inline bool emConfigModel::IsUnsaved() const
{
	return Unsaved;
}

inline const emSignal & emConfigModel::GetChangeSignal() const
{
	return ChangeSignal;
}

inline int emConfigModel::GetAutoSaveDelaySeconds() const
{
	return AutoSaveDelaySeconds;
}

inline emRec & emConfigModel::GetRec()
{
	return *Link.GetListenedRec();
}


#endif
