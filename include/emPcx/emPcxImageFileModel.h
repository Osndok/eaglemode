//------------------------------------------------------------------------------
// emPcxImageFileModel.h
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#ifndef emPcxImageFileModel_h
#define emPcxImageFileModel_h

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif


class emPcxImageFileModel : public emImageFileModel {

public:

	static emRef<emPcxImageFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emPcxImageFileModel(emContext & context, const emString & name);
	virtual ~emPcxImageFileModel();
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

	struct LoadingState {
		int Width,Height,Channels,PlanePixBits;
		int PlaneCount,BytesPerLine,RowPlaneSize,NextY;
		FILE * File;
		unsigned char * Palette;
		unsigned char * RowBuffer;
	};

	LoadingState * L;
};


#endif
