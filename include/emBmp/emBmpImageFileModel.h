//------------------------------------------------------------------------------
// emBmpImageFileModel.h
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

#ifndef emBmpImageFileModel_h
#define emBmpImageFileModel_h

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif


class emBmpImageFileModel : public emImageFileModel {

public:

	// Can load BMP, CUR and ICO.

	static emRef<emBmpImageFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emBmpImageFileModel(emContext & context, const emString & name);
	virtual ~emBmpImageFileModel();
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
	int Read32();

	struct LoadingState {
		int Width,Height,Channels;
		int BitsPerPixel,BitsOffset,ColsOffset,ColSize,ColsUsed,Compress,NextY;
		int CMax[3],CPos[3];
		bool IsIcon;
		bool ImagePrepared;
		FILE * File;
		unsigned char * Palette;
	};

	LoadingState * L;
};


#endif
