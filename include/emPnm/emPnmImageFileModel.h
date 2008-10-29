//------------------------------------------------------------------------------
// emPnmImageFileModel.h
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#ifndef emPnmImageFileModel_h
#define emPnmImageFileModel_h

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif


class emPnmImageFileModel : public emImageFileModel {

public:

	// Can load PNM, PBM, PGM and PPM file formats.

	static emRef<emPnmImageFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emPnmImageFileModel(emContext & context, const emString & name);
	virtual ~emPnmImageFileModel();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	int Read8();
	int Read16();
	int ReadDigit(bool allowSpace);
	int ReadDecimal();
	int ReadVal();

	struct LoadingState {
		int Format,Width,Height,MaxVal,NextY;
		bool ImagePrepared;
		FILE * File;
	};

	LoadingState * L;
};


#endif
