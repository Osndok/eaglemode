//------------------------------------------------------------------------------
// SilChessPanel.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef SilChessPanel_h
#define SilChessPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef SilChessModel_h
#include <SilChess/SilChessModel.h>
#endif

#ifndef SilChessRayTracer_h
#include <SilChess/SilChessRayTracer.h>
#endif


class SilChessPanel : public emFilePanel {

public:

	SilChessPanel(ParentArg parent, const emString & name,
	              SilChessModel * model);

	virtual ~SilChessPanel();

	virtual emString GetTitle();

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void PrepareRendering(bool viewingChanged);

	void RenderPixel();

	void PanelToBoard(double x, double y, int * bx, int * by) const;
	void BoardToPanel(double x, double y, double * px, double * py) const;

	void PaintSelection(const emPainter & painter);
	void PaintArrow(const emPainter & painter);

	SilChessModel * Mdl;
	bool HaveControlPanel;
	int SelX,SelY;
	SilChessRayTracer RayTracer;
	double ImgX1,ImgY1,ImgX2,ImgY2;
	emImage Image;
	int PixX,PixY,PixStep,InvX1,InvY1,InvX2,InvY2;
	bool ImageGood;
	bool HumanIsWhite;
	double EssenceX,EssenceY,EssenceW,EssenceH;
	double CameraX,CameraY,CameraZ;
	double RayXA,RayXB,RayYA,RayYB,RayZA,RayZB;
};


#endif
