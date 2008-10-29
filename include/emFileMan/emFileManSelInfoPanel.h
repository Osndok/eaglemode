//------------------------------------------------------------------------------
// emFileManSelInfoPanel.h
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

#ifndef emFileManSelInfoPanel_h
#define emFileManSelInfoPanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emFileManModel_h
#include <emFileMan/emFileManModel.h>
#endif


class emFileManSelInfoPanel : public emPanel {

public:

	emFileManSelInfoPanel(ParentArg parent, const emString & name);

	virtual ~emFileManSelInfoPanel();

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	enum StateType {
		STATE_COSTLY,
		STATE_WAIT,
		STATE_SCANNING,
		STATE_ERROR,
		STATE_SUCCESS
	};

	struct DetailsType {
		StateType State;
		emString ErrorMessage;
		int Entries;
		int HiddenEntries;
		int SymbolicLinks;
		int RegularFiles;
		int Subdirectories;
		int OtherTypes;
		emUInt64 Size;
		emUInt64 DiskUsage;
		bool DiskUsageUnknown;
	};

	void PaintDetails(
		const emPainter & painter, double x, double y, double w,
		double h, const char * caption, const DetailsType & details,
		emColor color, emColor canvasColor
	);

	void PaintSize(
		const emPainter & painter, double x, double y, double w,
		double h, emUInt64 size, emColor color, emColor canvasColor
	);

	void SetRectangles();

	void ResetDetails();

	bool WorkOnDetails();

	void WorkOnDetailEntry(DetailsType * details, emDirEntry dirEntry);

	emRef<emFileManModel> FileMan;

	double TextX;
	double TextY;
	double TextW;
	double TextH;
	double DetailsFrameX;
	double DetailsFrameY;
	double DetailsFrameW;
	double DetailsFrameH;
	double DetailsX;
	double DetailsY;
	double DetailsW;
	double DetailsH;

	bool AllowBusiness;

	DetailsType DirectDetails;
	DetailsType RecursiveDetails;

	emArray<emString> DirStack;
	emArray<emString> InitialDirStack;
	emArray<emString> SelList;
	int SelIndex;
	emString DirPath;
	emDirHandle DirHandle;
};


#endif
