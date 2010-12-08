//------------------------------------------------------------------------------
// emRecFileModel.h
//
// Copyright (C) 2005-2008,2010 Oliver Hamann.
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

#ifndef emRecFileModel_h
#define emRecFileModel_h

#ifndef emRec_h
#include <emCore/emRec.h>
#endif

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif


//==============================================================================
//=============================== emRecFileModel ===============================
//==============================================================================

class emRecFileModel : public emFileModel {

public:

	// Base class for an emFileModel where the data is an emRec. This class
	// solves loading and saving of the record, and it provides a signal for
	// getting informed about modifications of the record. In addition, the
	// file state is automatically set to FS_UNSAVED on modifications. A
	// derived class has to specify the record through calling PostConstruct
	// from its constructor. Hint: There is a similar class which is not an
	// emFileModel and which could be used for configuration files:
	// emConfigModel.

	const emSignal & GetChangeSignal() const;
		// Signaled on every modification of the record.

protected:

	emRecFileModel(emContext & context, const emString & name);
		// Do not forget to call PostConstruct from an overloaded
		// constructor.

	void PostConstruct(emRec & rec);
		// Must be called by the constructor of the derived class after
		// the record has been constructed.

	virtual ~emRecFileModel();
		// Destructor.

	emRec & GetRec();
		// Not valid before PostConstruct has been called.

	virtual void ResetData();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	class RecLink : public emRecListener {
	public:
		RecLink(emRecFileModel & model);
	protected:
		virtual void OnRecChanged();
	private:
		emRecFileModel & Model;
	};
	friend class RecLink;

	emSignal ChangeSignal;
	RecLink Link;
	emRecFileReader * Reader;
	emRecFileWriter * Writer;
	int ProtectFileState;
	emUInt64 MemoryNeed;
	int MemoryNeedOutOfDate;
	int ReadStep, ReadStepOfMemCalc;
};

inline const emSignal & emRecFileModel::GetChangeSignal() const
{
	return ChangeSignal;
}

inline emRec & emRecFileModel::GetRec()
{
	return *Link.GetListenedRec();
}


#endif
