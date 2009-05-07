//------------------------------------------------------------------------------
// emDirEntryAltPanel.h
//
// Copyright (C) 2007-2009 Oliver Hamann.
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

#ifndef emDirEntryAltPanel_h
#define emDirEntryAltPanel_h

#ifndef emVarModel_h
#include <emCore/emVarModel.h>
#endif

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emDirEntry_h
#include <emFileMan/emDirEntry.h>
#endif

#ifndef emFileManModel_h
#include <emFileMan/emFileManModel.h>
#endif


class emDirEntryAltPanel : public emPanel {

public:

	emDirEntryAltPanel(ParentArg parent, const emString & name,
	                   const emDirEntry & dirEntry, int alternative);

	virtual ~emDirEntryAltPanel();

	const emDirEntry & GetDirEntry() const;

	void UpdateDirEntry(const emDirEntry & dirEntry);
		// Must have same path.

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	struct SharedStuff {
		emRef<emFileManModel> FileMan;
		emImage InnerBorderImage;
	};

	static const char * const ContentName;
	static const char * const AltName;
	static const double LayoutLabelX;
	static const double LayoutLabelY;
	static const double LayoutLabelW;
	static const double LayoutLabelH;
	static const double LayoutPathX;
	static const double LayoutPathY;
	static const double LayoutPathW;
	static const double LayoutPathH;
	static const double MinAltVW;
	static const double LayoutAltX;
	static const double LayoutAltY;
	static const double LayoutAltW;
	static const double LayoutAltH;
	static const double MinContentVW;
	static const double LayoutContentFrame;
	static const double LayoutContentX;
	static const double LayoutContentY;
	static const double LayoutContentW;
	static const double LayoutContentH;
	static const emColor ColorBGNormal;
	static const emColor ColorInfoLabel;
	static const emColor ColorPath;

	emRef<emVarModel<SharedStuff> > SharedVar;
	emDirEntry DirEntry;
	int Alternative;
};

inline const emDirEntry & emDirEntryAltPanel::GetDirEntry() const
{
	return DirEntry;
}


#endif
