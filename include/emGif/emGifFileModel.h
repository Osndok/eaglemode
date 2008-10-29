//------------------------------------------------------------------------------
// emGifFileModel.h
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

#ifndef emGifFileModel_h
#define emGifFileModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emImage_h
#include <emCore/emImage.h>
#endif


class emGifFileModel : public emFileModel {

public:

	static emRef<emGifFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	int GetWidth() const;
	int GetHeight() const;
	double GetPixelTallness() const;
	double GetTallness() const;
	emColor GetBGColor() const;

	const emString & GetComment() const;

	bool IsAnimated() const;

	int GetRenderCount() const;
	int GetRenderX(int index) const;
	int GetRenderY(int index) const;
	int GetRenderWidth(int index) const;
	int GetRenderHeight(int index) const;
	bool IsRenderTransparent(int index) const;
	int GetRenderDisposal(int index) const;
	int GetRenderDelay(int index) const;
	bool GetRenderInput(int index) const;

	int GetChannelCount() const;

	void RenderImage(int index, emImage * image) const;
		// The image must not be smaller than GetWidth() x GetHeight().

	emImage RenderAll() const;

protected:

	emGifFileModel(emContext & context, const emString & name);
	virtual ~emGifFileModel();
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

	bool PostProcess();
	int Read8();
	int Read16();

	struct Render {
		Render * Next;
		int Disposal;
		int Delay;
		int Transparent;
		bool UserInput;
		bool Interlaced;
		int X,Y,Width,Height;
		int MinCodeSize;
		int ColorCount;
		int DataSize,DataFill;
		emColor * Colors;
		char * Data;
	};

	int Width,Height;
	int ChannelCount,ColorCount;
	int RenderCount,RenderArraySize;
	bool Animated;
	emColor BGColor;
	emUInt64 FileSize;
	double PixelTallness;
	emString Comment;
	emColor * Colors;
	Render * * RenderArray;
	FILE * File;
	bool InLoadingRenderData;
	int NextDisposal;
	bool NextUserInput;
	int NextDelay;
	int NextTransparent;
};

inline int emGifFileModel::GetWidth() const
{
	return Width;
}

inline int emGifFileModel::GetHeight() const
{
	return Height;
}

inline double emGifFileModel::GetPixelTallness() const
{
	return PixelTallness;
}

inline emColor emGifFileModel::GetBGColor() const
{
	return BGColor;
}

inline const emString & emGifFileModel::GetComment() const
{
	return Comment;
}

inline bool emGifFileModel::IsAnimated() const
{
	return Animated;
}

inline int emGifFileModel::GetRenderCount() const
{
	return RenderCount;
}

inline int emGifFileModel::GetRenderX(int index) const
{
	return RenderArray[index]->X;
}

inline int emGifFileModel::GetRenderY(int index) const
{
	return RenderArray[index]->Y;
}

inline int emGifFileModel::GetRenderWidth(int index) const
{
	return RenderArray[index]->Width;
}

inline int emGifFileModel::GetRenderHeight(int index) const
{
	return RenderArray[index]->Height;
}

inline bool emGifFileModel::IsRenderTransparent(int index) const
{
	return RenderArray[index]->Transparent>=0;
}

inline int emGifFileModel::GetRenderDisposal(int index) const
{
	return RenderArray[index]->Disposal;
}

inline int emGifFileModel::GetRenderDelay(int index) const
{
	return RenderArray[index]->Delay;
}

inline bool emGifFileModel::GetRenderInput(int index) const
{
	return RenderArray[index]->UserInput;
}

inline int emGifFileModel::GetChannelCount() const
{
	return ChannelCount;
}


#endif
