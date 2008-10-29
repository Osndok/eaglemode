//------------------------------------------------------------------------------
// emList.cpp
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

#include <emCore/emList.h>


bool emSortSingleLinkedList(
	void * * pFirst, int nextOffset,
	int(*compare)(void * ptr1, void * ptr2, void * context),
	void * context
)
{
	void * arr[64];
	void * first, * l, * e1, * e2;
	void * * pe, * * pi, * * pn;
	bool changed;

#	define EM_SSLL_PNEXT(ELEM) ((void**)(((char*)ELEM)+nextOffset))

	first=*pFirst;
	if (!first || !*EM_SSLL_PNEXT(first)) return false;
	changed=false;
	arr[0]=NULL;
	arr[1]=NULL;
	pn=arr+1;
	do {
		l=first;
		e1=*EM_SSLL_PNEXT(l);
		if (!e1) {
			first=NULL;
		}
		else {
			first=*EM_SSLL_PNEXT(e1);
			if (compare(l,e1,context)<=0) {
				*EM_SSLL_PNEXT(e1)=NULL;
			}
			else {
				changed=true;
				*EM_SSLL_PNEXT(e1)=l;
				*EM_SSLL_PNEXT(l)=NULL;
				l=e1;
			}
		}
		pi=arr;
		e1=*pi;
		if (e1) {
			do {
				e2=l;
				pe=&l;
				for (;;) {
					if (compare(e1,e2,context)<=0) {
						*pe=e1;
						pe=EM_SSLL_PNEXT(e1);
						e1=*pe;
						if (e1) continue;
						*pe=e2;
					}
					else {
						changed=true;
						*pe=e2;
						pe=EM_SSLL_PNEXT(e2);
						e2=*pe;
						if (e2) continue;
						*pe=e1;
					}
					break;
				}
				*pi=NULL;
				pi++;
				e1=*pi;
			} while(e1);
			if (pi==pn) {
				pn++;
				*pn=NULL;
			}
		}
		*pi=l;
	} while (first);
	pi=arr;
	do {
		first=*pi++;
	} while (!first);
	while (pi<pn) {
		e1=*pi;
		if (e1) {
			e2=first;
			pe=&first;
			for (;;) {
				if (compare(e1,e2,context)<=0) {
					*pe=e1;
					pe=EM_SSLL_PNEXT(e1);
					e1=*pe;
					if (e1) continue;
					*pe=e2;
				}
				else {
					changed=true;
					*pe=e2;
					pe=EM_SSLL_PNEXT(e2);
					e2=*pe;
					if (e2) continue;
					*pe=e1;
				}
				break;
			}
		}
		pi++;
	}
	*pFirst=first;
	return changed;
}


bool emSortDoubleLinkedList(
	void * * pFirst, void * * pLast, int nextOffset, int prevOffset,
	int(*compare)(void * ptr1, void * ptr2, void * context),
	void * context
)
{
	void * e1, * e2;

#	define EM_SDLL_PELEM(ELEM,OFFSET) ((void**)(((char*)ELEM)+OFFSET))

	if (!emSortSingleLinkedList(pFirst,nextOffset,compare,context)) {
		return false;
	}
	e1=NULL;
	e2=*pFirst;
	do {
		*EM_SDLL_PELEM(e2,prevOffset)=e1;
		e1=e2;
		e2=*EM_SDLL_PELEM(e2,nextOffset);
	} while (e2);
	*pLast=e1;
	return true;
}
