//------------------------------------------------------------------------------
// emSvgFilePanel.h
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

#ifndef emSvgFilePanel_h
#define emSvgFilePanel_h

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emSvgFileModel_h
#include <emSvg/emSvgFileModel.h>
#endif


class emSvgFilePanel : public emFilePanel {

public:

	emSvgFilePanel(ParentArg parent, const emString & name,
	               emSvgFileModel * fileModel=NULL,
	               bool updateFileModel=true);

	virtual ~emSvgFilePanel();

	virtual void SetFileModel(emFileModel * fileModel,
	                          bool updateFileModel=true);

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH);

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void GetOutputRect(double * pX, double * pY, double * pW, double * pH);

	void ClearSvgDisplay();
	void UpdateSvgDisplay(bool viewingChanged);

	emRef<emSvgServerModel> ServerModel;

	emSvgServerModel::JobHandle Job;
	emString RenderError;

	emImage Img;
	double SrcX,SrcY,SrcW,SrcH;

	emImage JobImg;
	double JobSrcX,JobSrcY,JobSrcW,JobSrcH;
	bool JobUpToDate;
	emUInt64 JobDelayStartTime;
	emTimer JobDelayTimer;

	emImage RenderIcon;
	emTimer IconTimer;
	bool ShowIcon;
};


#endif
