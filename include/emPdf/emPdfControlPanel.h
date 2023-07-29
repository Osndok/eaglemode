//------------------------------------------------------------------------------
// emPdfControlPanel.h
//
// Copyright (C) 2023 Oliver Hamann.
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

#ifndef emPdfControlPanel_h
#define emPdfControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emPdfSelection_h
#include <emPdf/emPdfSelection.h>
#endif


class emPdfControlPanel : public emLinearGroup {

public:

	emPdfControlPanel(
		ParentArg parent, const emString & name,
		emPdfFileModel * fileModel, emPdfSelection & selection
	);

	~emPdfControlPanel();

protected:

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void UpdateControls();

	emString CalculatePageSizes() const;
	static emString PageSizeToString(int w, int h);

	emRef<emPdfFileModel> FileModel;
	emCrossPtr<emPdfSelection> Selection;

	emTextField * Title;
	emTextField * Author;
	emTextField * Subject;
	emTextField * Keywords;
	emTextField * Creator;
	emTextField * Producer;
	emTextField * CreationDate;
	emTextField * ModificationDate;
	emTextField * Version;
	emTextField * PageCount;
	emTextField * PageSize;

	emButton * Copy;
	emButton * SelectAll;
	emButton * ClearSelection;
};


#endif
