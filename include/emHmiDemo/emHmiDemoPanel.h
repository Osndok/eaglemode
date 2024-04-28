//------------------------------------------------------------------------------
// emHmiDemoPanel.h
//
// Copyright (C) 2012,2016,2024 Oliver Hamann.
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

#ifndef emHmiDemoPanel_h
#define emHmiDemoPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoPanel : public emPanel {
public:
	emHmiDemoPanel(ParentArg parent, const emString & name);
	virtual ~emHmiDemoPanel();
protected:
	virtual bool IsOpaque() const;
	virtual void AutoExpand();
	virtual void LayoutChildren();
};


#endif
