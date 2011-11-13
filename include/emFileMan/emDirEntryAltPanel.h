//------------------------------------------------------------------------------
// emDirEntryAltPanel.h
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#ifndef emFileManViewConfig_h
#include <emFileMan/emFileManViewConfig.h>
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

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void UpdateContentPanel(bool forceRecreation=false, bool forceRelayout=false);
	void UpdateAltPanel(bool forceRecreation=false, bool forceRelayout=false);

	static const char * const ContentName;
	static const char * const AltName;

	emRef<emFileManModel> FileMan;
	emRef<emFileManViewConfig> Config;
	emDirEntry DirEntry;
	int Alternative;
};

inline const emDirEntry & emDirEntryAltPanel::GetDirEntry() const
{
	return DirEntry;
}


#endif
