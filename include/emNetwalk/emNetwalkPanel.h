//------------------------------------------------------------------------------
// emNetwalkPanel.h
//
// Copyright (C) 2010 Oliver Hamann.
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

#ifndef emNetwalkPanel_h
#define emNetwalkPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emNetwalkModel_h
#include <emNetwalk/emNetwalkModel.h>
#endif


class emNetwalkPanel : public emFilePanel {

public:

	emNetwalkPanel(ParentArg parent, const emString & name,
	               emNetwalkModel * fileModel);

	virtual ~emNetwalkPanel();

	virtual emString GetTitle();

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual emCursor GetCursor();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void PaintPieceBackground(
		const emPainter & painter, double x, double y, double w, double h,
		int px, int py, emColor canvasColor
	);

	void PaintPiecePipe(
		const emPainter & painter, double x, double y, double w, double h,
		int px, int py
	);

	static void PaintShapeWithRoundedEdges(
		const emPainter & painter, double x, double y, double w, double h,
		const emImage & img, double srcX, double srcY, double srcW, double srcH,
		int channel=0, emColor color=emColor::WHITE, emColor canvasColor=0
	);

	static void PaintImageWithRoundedEdges(
		const emPainter & painter, double x, double y, double w, double h,
		const emImage & img, double srcX, double srcY, double srcW, double srcH,
		int alpha=255, emColor canvasColor=0
	);

	void PrepareTransformation();

	enum {
		PF_EAST   =emNetwalkModel::PF_EAST   ,
		PF_SOUTH  =emNetwalkModel::PF_SOUTH  ,
		PF_WEST   =emNetwalkModel::PF_WEST   ,
		PF_NORTH  =emNetwalkModel::PF_NORTH  ,
		PF_SOURCE =emNetwalkModel::PF_SOURCE ,
		PF_TARGET =emNetwalkModel::PF_TARGET ,
		PF_FILLED =emNetwalkModel::PF_FILLED ,
		PF_TOUCHED=emNetwalkModel::PF_TOUCHED,
		PF_MARKED =emNetwalkModel::PF_MARKED ,
		PF_BLOCKED=emNetwalkModel::PF_BLOCKED,
		PF_CONMASK=emNetwalkModel::PF_CONMASK
	};

	emNetwalkModel * Mdl;
	bool HaveControlPanel;
	bool Scrolling;
	double ScrollX0,ScrollY0;
	double EssenceX,EssenceY,EssenceW,EssenceH;
	double X0,Y0,DX,DY;

	emImage ImgBackground;
	emImage ImgBorder;
	emImage ImgLights;
	emImage ImgMarks;
	emImage ImgNoBorder;
	emImage ImgPipes;
	emImage ImgSymbols;

	static const double BorderSize;
	static const emColor BgColor;
	static const emColor LightColor;
	static const emColor MarkColor;
};


#endif
