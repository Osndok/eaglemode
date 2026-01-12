//------------------------------------------------------------------------------
// emTgaImageFileModel.h
//
// Copyright (C) 2004-2008,2014,2018,2025 Oliver Hamann.
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

#ifndef emTgaImageFileModel_h
#define emTgaImageFileModel_h

#ifndef emFileStream_h
#include <emCore/emFileStream.h>
#endif

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif


class emTgaImageFileModel : public emImageFileModel {

public:

	static emRef<emTgaImageFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emTgaImageFileModel(emContext & context, const emString & name);
	virtual ~emTgaImageFileModel();
	virtual void TryStartLoading();
	virtual bool TryContinueLoading();
	virtual void QuitLoading();
	virtual void TryStartSaving();
	virtual bool TryContinueSaving();
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	class RleEncoder {
	public:
		RleEncoder(emFileStream & file, int pixelSize);
		void TryPut(emUInt32 pixel);
		void TryFlush();
	private:
		void TryWriteNext();
		emFileStream & File;
		emUInt32 Buf[256];
		int PixelSize;
		int Pos,Fill;
	};

	struct LoadingState {
		emFileStream File;
		emOwnArrayPtr<emColor> Palette;
		emColor RunCol;
		int IDLen,CMapType,IMapType,CMapSize,CMapBitsPP;
		int Width,Height,BitsPP,Descriptor,ChannelCount;
		int NextY,RunLen;
		bool ImagePrepared;
	};

	struct SavingState {
		emFileStream File;
		emOwnPtr<RleEncoder> Encoder;
		emArray<emColor> Pal;
		bool HaveColor,HaveAlpha;
		int PixelSize;
		int NextY;
	};

	emOwnPtr<LoadingState> L;
	emOwnPtr<SavingState> S;
};


inline void emTgaImageFileModel::RleEncoder::TryPut(emUInt32 pixel)
{
	if (Fill>=256) TryWriteNext();
	Buf[(Pos+Fill)&255]=pixel;
	Fill++;
}


#endif
