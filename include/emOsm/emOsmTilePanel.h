//------------------------------------------------------------------------------
// emOsmTilePanel.h
//
// Copyright (C) 2011-2012,2016,2022,2024 Oliver Hamann.
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

#ifndef emOsmTilePanel_h
#define emOsmTilePanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emOsmTileCache_h
#include <emOsm/emOsmTileCache.h>
#endif


class emOsmTilePanel : public emPanel {

public:

	emOsmTilePanel(
		ParentArg parent, const emString & name, emOsmTileCache * tileCache,
		const emString & tilesUrl, int maxZ, int tileZ=0, int tileX=0,
		int tileY=0
	);

	virtual ~emOsmTilePanel();

	virtual emString GetTitle() const;
	virtual emString GetIconFileName() const;

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual void AutoExpand();

	virtual void LayoutChildren();

private:

	void UpdateState();
	void ClearAll();
	void SetError(const emString & errorText);

	enum StateEnum {
		S_NOT_VIEWED,
		S_LOADING,
		S_LOADED,
		S_ERROR
	};

	emRef<emOsmTileCache> TileCache;
	emString TilesUrl;
	int MaxZ;
	int TileZ;
	int TileX;
	int TileY;
	bool BusyCleanerToAllow;
	StateEnum State;
	emRef<emOsmTileCache::LoadJob> LoadJob;
	emImage Image;
	emString ErrorText;
	emCrossPtr<emOsmTilePanel> Children[4];
};


#endif
