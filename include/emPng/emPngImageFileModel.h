//------------------------------------------------------------------------------
// emPngImageFileModel.h
//
// Copyright (C) 2004-2008,2014,2018,2022 Oliver Hamann.
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

#ifndef emPngImageFileModel_h
#define emPngImageFileModel_h

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif


class emPngImageFileModel : public emImageFileModel {

public:

	static emRef<emPngImageFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emPngImageFileModel(emContext & context, const emString & name);
	virtual ~emPngImageFileModel();
	virtual void TryStartLoading();
	virtual bool TryContinueLoading();
	virtual void QuitLoading();
	virtual void TryStartSaving();
	virtual bool TryContinueSaving();
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:
	struct LoadingState {
		FILE * file;
		void * decodeInstance;
		int width,height,channelCount,passCount;
		bool imagePrepared;
		int y,pass;
	};

	LoadingState * L;
};


#endif
