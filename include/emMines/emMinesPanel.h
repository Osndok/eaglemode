//------------------------------------------------------------------------------
// emMinesPanel.h
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

#ifndef emMinesPanel_h
#define emMinesPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emMinesFileModel_h
#include <emMines/emMinesFileModel.h>
#endif


class emMinesPanel : public emFilePanel {

public:

	emMinesPanel(ParentArg parent, const emString & name,
	             emMinesFileModel * fileModel);

	virtual ~emMinesPanel();

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

	void PaintField(const emPainter & painter, int x, int y, int z,
	                emColor color);

	void PaintClosedField(const emPainter & painter, double x, double y,
	                      double z, double r, emColor color);
	void PaintMarkedField(const emPainter & painter, double x, double y,
	                      double z, double r, emColor color);
	void PaintOpenField(const emPainter & painter, double x, double y,
	                    double z, double r, int number, emColor color);
	void PaintExplodingField(const emPainter & painter, double x, double y,
	                         double z, double r);

	void PaintXBeam(const emPainter & painter, double x, double y, double z,
	                double x2, double r, emColor color);
	void PaintYBeam(const emPainter & painter, double x, double y, double z,
	                double y2, double r, emColor color);
	void PaintZBeam(const emPainter & painter, double x, double y, double z,
	                double z2, double r, emColor color);

	bool IsCursorValid() const;

	double TransX(double fieldX, double fieldZ) const;
	double TransY(double fieldY, double fieldZ) const;

	void PrepareTransformation();

	emMinesFileModel * Mdl;
	bool HaveControlPanel;
	int CursorX,CursorY,CursorZ;
	double EssenceX,EssenceY,EssenceW,EssenceH;
	double CameraX,CameraY,CameraZ;
	double TrX0,TrY0,TrScale;
};


#endif
