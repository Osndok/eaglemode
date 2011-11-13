//------------------------------------------------------------------------------
// emDirPanel.h
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

#ifndef emDirPanel_h
#define emDirPanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emDirModel_h
#include <emFileMan/emDirModel.h>
#endif

#ifndef emFileManViewConfig_h
#include <emFileMan/emFileManViewConfig.h>
#endif

#ifndef emFileManModel_h
#include <emFileMan/emFileManModel.h>
#endif


class emDirPanel : public emFilePanel {

public:

	emDirPanel(ParentArg parent, const emString & name,
	           const emString & path);

	virtual ~emDirPanel();

	const emString & GetPath();

	bool IsContentComplete() const;

	void SelectAll();
		// Works only if IsContentComplete().

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	virtual void SetFileModel(emFileModel * fileModel, bool updateFileModel=true);

	void UpdateChildren();
	void SortChildren();
	static int CompareChildren(emPanel * c1, emPanel * c2, void * context);
	void ClearKeyWalkState();
	void KeyWalk(emInputEvent & event, const emInputState & state);

	struct KeyWalkStateType {
		emTimer Timer;
		emString String;
		inline KeyWalkStateType(emScheduler & scheduler) : Timer(scheduler) {}
	};

	emString Path;
	emRef<emFileManModel> FileMan;
	emRef<emFileManViewConfig> Config;
	bool ContentComplete;
	KeyWalkStateType * KeyWalkState;
};

inline const emString & emDirPanel::GetPath()
{
	return Path;
}

inline bool emDirPanel::IsContentComplete() const
{
	return ContentComplete;
}


#endif
