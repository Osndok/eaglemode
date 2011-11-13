//------------------------------------------------------------------------------
// emTextFilePanel.h
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

#ifndef emTextFilePanel_h
#define emTextFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emTextFileModel_h
#include <emText/emTextFileModel.h>
#endif


class emTextFilePanel : public emFilePanel {

public:

	emTextFilePanel(ParentArg parent, const emString & name,
	                emTextFileModel * fileModel=NULL,
	                bool updateFileModel=true, bool alternativeView=false);

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

protected:

	virtual bool Cycle();

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void PaintAsText(const emPainter & painter, emColor canvasColor);

	int PaintTextUtf8ToUtf8(
		const emPainter & painter, double x, double y, double charWidth,
		double charHeight, const char * text, int textLen,
		emColor color, emColor canvasColor
	) const;

	int PaintText8BitToUtf8(
		const emPainter & painter, double x, double y, double charWidth,
		double charHeight, const char * text, int textLen,
		emColor color, emColor canvasColor
	) const;

	int PaintTextUtf8To8Bit(
		const emPainter & painter, double x, double y, double charWidth,
		double charHeight, const char * text, int textLen,
		emColor color, emColor canvasColor
	) const;

	int PaintTextUtf16(
		const emPainter & painter, double x, double y, double charWidth,
		double charHeight, const char * text, int textLen,
		emColor color, emColor canvasColor
	) const;

	void PaintAsHex(const emPainter & painter, emColor canvasColor);

	bool AlternativeView;
	emTextFileModel * Model;
};


#endif
