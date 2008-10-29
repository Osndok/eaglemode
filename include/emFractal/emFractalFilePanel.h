//------------------------------------------------------------------------------
// emFractalFilePanel.h
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

#ifndef emFractalFilePanel_h
#define emFractalFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emFractalFileModel_h
#include <emFractal/emFractalFileModel.h>
#endif


class emFractalFilePanel : public emFilePanel {

public:

	emFractalFilePanel(
		ParentArg parent, const emString & name,
		emFractalFileModel * fileModel
	);

	virtual ~emFractalFilePanel();

	virtual emString GetTitle();

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void Prepare();

	emColor CalcPixel() const;

	void PutPixel(emColor color);

	emFractalFileModel * Mdl;
	emArray<emColor> Colors;
	double ImgX1,ImgY1,ImgX2,ImgY2;
	emImage Image;
	double FrcX,FrcY,FrcSX,FrcSY;
	int PixX,PixY,PixStep,InvX1,InvY1,InvX2,InvY2;
};


#endif
