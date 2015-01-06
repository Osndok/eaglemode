//------------------------------------------------------------------------------
// emPsFileModel.h
//
// Copyright (C) 2006-2008,2014 Oliver Hamann.
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

#ifndef emPsFileModel_h
#define emPsFileModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emPsDocument_h
#include <emPs/emPsDocument.h>
#endif


class emPsFileModel : public emFileModel {

public:

	static emRef<emPsFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	const emPsDocument & GetDocument() const;

protected:

	emPsFileModel(emContext & context, const emString & name);
	virtual ~emPsFileModel();
	virtual void ResetData();
	virtual void TryStartLoading() throw(emException);
	virtual bool TryContinueLoading() throw(emException);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emException);
	virtual bool TryContinueSaving() throw(emException);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	struct LoadingState {
		FILE * File;
		int FileSize,FilePos;
		emArray<char> Buffer;
	};

	LoadingState * L;

	emPsDocument Document;
};

inline const emPsDocument & emPsFileModel::GetDocument() const
{
	return Document;
}


#endif
