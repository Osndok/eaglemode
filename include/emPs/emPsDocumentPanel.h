//------------------------------------------------------------------------------
// emPsDocumentPanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emPsDocumentPanel_h
#define emPsDocumentPanel_h

#ifndef emPsPagePanel_h
#include <emPs/emPsPagePanel.h>
#endif


class emPsDocumentPanel : public emPanel {

public:

	emPsDocumentPanel(ParentArg parent, const emString & name,
	                  const emPsDocument & document);

	virtual ~emPsDocumentPanel();

	const emPsDocument & GetDocument() const;
	void SetDocument(const emPsDocument & document);

	emColor GetBGColor() const;
	void SetBGColor(emColor bgColor);

	emColor GetFGColor() const;
	void SetFGColor(emColor fgColor);

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual void LayoutChildren();

private:

	void CalcLayout();
	void CreatePagePanels();
	void DestroyPagePanels();
	bool ArePagePanelsToBeShown();

	emPsDocument Document;
	emColor BGColor;
	emColor FGColor;
	int Rows,Columns;
	double CellX0,CellY0,CellW,CellH;
	double PgX,PgY,PerPoint;
	double ShadowSize;
	emPsPagePanel * * PagePanels;
};

inline const emPsDocument & emPsDocumentPanel::GetDocument() const
{
	return Document;
}

inline emColor emPsDocumentPanel::GetBGColor() const
{
	return BGColor;
}

inline emColor emPsDocumentPanel::GetFGColor() const
{
	return FGColor;
}


#endif
