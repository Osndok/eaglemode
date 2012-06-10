//------------------------------------------------------------------------------
// emPdfFilePanel.h
//
// Copyright (C) 2011-2012 Oliver Hamann.
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

#ifndef emPdfFilePanel_h
#define emPdfFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emPdfFileModel_h
#include <emPdf/emPdfFileModel.h>
#endif

#ifndef emPdfPagePanel_h
#include <emPdf/emPdfPagePanel.h>
#endif


class emPdfFilePanel : public emFilePanel {

public:

	emPdfFilePanel(ParentArg parent, const emString & name,
	               emPdfFileModel * fileModel=NULL,
	               bool updateFileModel=true);

	virtual ~emPdfFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

	emColor GetBGColor() const;
	void SetBGColor(emColor bgColor);

	emColor GetFGColor() const;
	void SetFGColor(emColor fgColor);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void CalcLayout();

	void CreatePagePanels();

	void DestroyPagePanels();

	bool ArePagePanelsToBeShown();

	emColor BGColor;
	emColor FGColor;
	int Rows,Columns;
	double CellX0,CellY0,CellW,CellH;
	double PgX,PgY,PerPoint;
	double ShadowSize;
	emArray<emPdfPagePanel*> PagePanels;
};

inline emColor emPdfFilePanel::GetBGColor() const
{
	return BGColor;
}

inline emColor emPdfFilePanel::GetFGColor() const
{
	return FGColor;
}


#endif
