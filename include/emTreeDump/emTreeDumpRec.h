//------------------------------------------------------------------------------
// emTreeDumpRec.h
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#ifndef emTreeDumpRec_h
#define emTreeDumpRec_h

#ifndef emCrossPtr_h
#include <emCore/emCrossPtr.h>
#endif

#ifndef emRec_h
#include <emCore/emRec.h>
#endif


class emTreeDumpRec : public emStructRec {

public:

	emTreeDumpRec();
	virtual ~emTreeDumpRec();

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);

	virtual const char * GetFormatName() const;

	enum FrameType {
		FRAME_NONE       = 0,
		FRAME_RECTANGLE  = 1,
		FRAME_ROUND_RECT = 2,
		FRAME_ELLIPSE    = 3,
		FRAME_HEXAGON    = 4
	};
	emEnumRec Frame;

	emColorRec BgColor;
	emColorRec FgColor;

	emStringRec Title;

	emStringRec Text;

	class CommandRec : public emStructRec {
	public:
		CommandRec();
		virtual ~CommandRec();
		emStringRec Caption;
		emTArrayRec<emStringRec> Args;
	};
	emTArrayRec<CommandRec> Commands;

	emTArrayRec<emStringRec> Files;

	emTArrayRec<emTreeDumpRec> Children;

private:

	emCrossPtrList CrossPtrList;

};

inline void emTreeDumpRec::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}


#endif
