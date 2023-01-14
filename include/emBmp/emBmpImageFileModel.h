//------------------------------------------------------------------------------
// emBmpImageFileModel.h
//
// Copyright (C) 2004-2008,2010,2014,2018-2019,2022 Oliver Hamann.
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
	virtual void TryStartLoading();
	virtual bool TryContinueLoading();
	virtual void QuitLoading();
	virtual void TryStartSaving();
	virtual bool TryContinueSaving();
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	int Read8();
	int Read16();
	int Read32();

	typedef void * (*PngStartDecodingFunc)(
		FILE * file, int * width, int * height, int * channelCount,
		int * passCount, char * infoBuf, size_t infoBufSize,
		char * errorBuf, size_t errorBufSize
	);
	typedef int (*PngContinueDecodingFunc)(
		void * instance, unsigned char * rowBuf, char * commentBuf,
		size_t commentBufSize, char * errorBuf, size_t errorBufSize
	);
	typedef void (*PngQuitDecodingFunc)(void * instance);

	struct LoadingState {
		int Width,Height,Channels;
		int BitsPerPixel;
		long BitsOffset,ColsOffset;
		int ColSize,ColsUsed,Compress,Y;
		int CMax[3],CPos[3];
		bool IsIcon;
		bool IsPng;
		emLibHandle PngLib;
		PngStartDecodingFunc PngStartDecoding;
		PngContinueDecodingFunc PngContinueDecoding;
		PngQuitDecodingFunc PngQuitDecoding;
		void * PngInst;
		int PassCount,Pass;
		bool ImagePrepared;
		FILE * File;
		unsigned char * Palette;
	};

	LoadingState * L;
};


#endif
