//------------------------------------------------------------------------------
// emPdfPagePanel.h
//
// Copyright (C) 2011 Oliver Hamann.
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

#ifndef emPdfPagePanel_h
#define emPdfPagePanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emPdfFileModel_h
#include <emPdf/emPdfFileModel.h>
#endif


class emPdfPagePanel : public emPanel {

public:

	emPdfPagePanel(ParentArg parent, const emString & name,
	               emPdfFileModel * fileModel, int pageIndex);

	virtual ~emPdfPagePanel();

	emPdfFileModel * GetFileModel();
	int GetPageIndex() const;

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool Cycle();
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void UpdatePageDisplay(bool viewingChanged);

	emRef<emPdfServerModel> Server;
	emRef<emPdfFileModel> FileModel;
	int PageIndex;

	emPdfServerModel::JobHandle Job;
	emString JobErrorText;

	emImage PreImg;

	emImage Img;
	double SrcX,SrcY,SrcW,SrcH;

	emImage JobImg;
	double JobSrcX,JobSrcY,JobSrcW,JobSrcH;
	bool JobUpToDate;
	emUInt64 JobDelayStartTime;
	emTimer JobDelayTimer;

	emImage WaitIcon,RenderIcon;
	emTimer IconTimer;
	bool ShowIcon;
};

inline emPdfFileModel * emPdfPagePanel::GetFileModel()
{
	return FileModel;
}

inline int emPdfPagePanel::GetPageIndex() const
{
	return PageIndex;
}


#endif
