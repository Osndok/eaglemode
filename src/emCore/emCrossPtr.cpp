//------------------------------------------------------------------------------
// emCrossPtr.cpp
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

#include <emCore/emCrossPtr.h>


void emCrossPtrPrivate::Unlink()
{
	*ThisPtr=Next;
	if (Next) Next->ThisPtr=ThisPtr;
}


void emCrossPtrList::BreakCrossPtrs()
{
	emCrossPtrPrivate * p;

	p=First;
	if (p) {
		do {
			p->Obj=NULL;
			p=p->Next;
		} while (p);
		First=NULL;
	}
}


void emCrossPtrList::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	crossPtr.ThisPtr=&First;
	crossPtr.Next=First;
	if (First) First->ThisPtr=&crossPtr.Next;
	First=&crossPtr;
}
