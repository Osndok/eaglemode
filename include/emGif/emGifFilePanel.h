//------------------------------------------------------------------------------
// emGifFilePanel.h
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

#ifndef emGifFilePanel_h
#define emGifFilePanel_h

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emGifFileModel_h
#include <emGif/emGifFileModel.h>
#endif


class emGifFilePanel : public emFilePanel {

public:

	emGifFilePanel(ParentArg parent, const emString & name,
	               emGifFileModel * fileModel=NULL,
	               bool updateFileModel=true);

	virtual ~emGifFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

	const emSignal & GetPlaySignal() const;
	bool IsPlaying() const;
	void StopPlaying();
	void ContinuePlaying();

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void CalcImageLayout(double * pX, double * pY, double * pW, double * pH) const;
	void InvalidatePerImage(int x, int y, int w, int h);

	emSignal PlaySignal;
	emImage Image, UndoImage;
	int RIndex;
	bool Playing;
	emTimer Timer;
};

inline const emSignal & emGifFilePanel::GetPlaySignal() const
{
	return PlaySignal;
}

inline bool emGifFilePanel::IsPlaying() const
{
	return Playing;
}


#endif
