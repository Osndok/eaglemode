//------------------------------------------------------------------------------
// emAvFilePanel.h
//
// Copyright (C) 2005-2010 Oliver Hamann.
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

#ifndef emAvFilePanel_h
#define emAvFilePanel_h

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emAvFileModel_h
#include <emAv/emAvFileModel.h>
#endif


class emAvFilePanel : public emFilePanel {

public:

	emAvFilePanel(ParentArg parent, const emString & name,
	              emAvFileModel * fileModel=NULL,
	              bool updateFileModel=true);

	virtual ~emAvFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

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

	void UpdateEssenceRect();

	void UpdateCursorHiding(bool restart);

	void UpdateScreensaverDisabling();

	emImage BgImage;
	emTimer CursorTimer;
	bool CursorHidden;
	bool ScreensaverDisabled;
	bool HaveControlPanel;
	emString WarningText;
	emUInt64 WarningStartTime;
	emByte WarningAlpha;
	double OldMouseX,OldMouseY;
	double EX,EY,EW,EH;
};


#endif
