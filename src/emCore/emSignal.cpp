//------------------------------------------------------------------------------
// emSignal.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emCore/emEngine.h>


emSignal::emSignal()
{
	RNode.Next=NULL;
	ELFirst=NULL;
	Clock=0;
}


emSignal::~emSignal()
{
	Abort();
	while (ELFirst) emEngine::RemoveLink(ELFirst);
}


void emSignal::Abort()
{
	emScheduler::SignalRingNode * r, * q;

	r=RNode.Next;
	if (r) {
		for (;;) {
			if ((q=r->Next)!=&RNode) {
				if ((r=q->Next)!=&RNode) continue;
				r=q;
			}
			break;
		}
		r->Next=RNode.Next;
		RNode.Next=NULL;
	}
	Clock=0;
}
