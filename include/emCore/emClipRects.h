//------------------------------------------------------------------------------
// emClipRects.h
//
// Copyright (C) 2011-2012 Oliver Hamann.
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

#ifndef emClipRects_h
#define emClipRects_h

#ifndef emList_h
#include <emCore/emList.h>
#endif


//==============================================================================
//================================ emClipRects =================================
//==============================================================================

template <class NUM> class emClipRects {

public:

	// Template class for a disjoint set of clip rectangles with
	// copy-on-write behavior. The template parameter NUM is the type of the
	// rectangle coordinates (usually int or double).

	emClipRects();
		// Construct an empty set of clip rectangles.

	emClipRects(const emClipRects & clipRects);
		// Construct a copied set of clip rectangles.
		// Arguments:
		//   clipRects - The set to be copied.

	emClipRects(NUM x1, NUM y1, NUM x2, NUM y2);
		// Construct a set of clip rectangles with a single rectangle.
		// Arguments:
		//   x1,y1,x2,y2 - Coordinates of the rectangle.

	~emClipRects();
		// Destruct a set of clip rectangles.

	emClipRects & operator = (const emClipRects & clipRects);
		// Copy a set of clip rectangles.

	class Rect {
	public:

		// Class for one rectangle of a set of clip rectangles.

		NUM GetX1() const;
		NUM GetY1() const;
		NUM GetX2() const;
		NUM GetY2() const;
		void Get(NUM * pX1, NUM * pY1, NUM * pX2, NUM * pY2) const;
			// Get the coordinates of the rectangle.

		const Rect * GetNext() const;
			// Get the next rectangle in the list of rectangles.

	private:
		friend class emClipRects<NUM>;
		NUM X1,Y1,X2,Y2;
		Rect * Next;
	};

	const Rect * GetFirst() const;
		// Get a pointer to the first rectangle. The rectangles are
		// organized in a single-linked NULL-terminated list. So you can
		// iterate the rectangles with:
		//
		//   for (r=clipRects.GetFirst(); r; r=r->GetNext()) {...}
		//
		// The rectangles have no order, except after one has called
		// Sort().
		//
		// At least because of the copy-on-write feature, the pointers
		// are valid only until calling any non-const method or operator
		// on this list, or giving this list as a non-const argument to
		// any call in the world.

	int GetCount() const;
		// Get the number of rectangles.

	void GetMinMax(NUM * pX1, NUM * pY1, NUM * pX2, NUM * pY2) const;
		// Calculate the min/max rectangle. That is the smallest
		// rectangle which encloses the whole set of clip rectangles. If
		// the set is empty, the rectangle (0,0,0,0) is returned.
		// Arguments:
		//   pX1,pY1,pX2,pY2 - Pointers for returning the coordinates of
		//                     the min/max rectangle.

	void SetToMinMax();
		// Set this set of clip rectangles to its min/max rectangle.

	void SetToMinMaxOf(const emClipRects & clipRects);
		// Set this set of clip rectangles to the min/max rectangle of
		// another set of clip rectangles.

	void Set(NUM x1, NUM y1, NUM x2, NUM y2);
		// Set this set of clip rectangles to a single rectangle.
		// Arguments:
		//   x1,y1,x2,y2 - Coordinates of the rectangle.

	void Set(const emClipRects & clipRects);
		// Set this set of clip rectangles equal to another set of clip
		// rectangles.
		//   clipRects - The set to be copied.

	void Unite(NUM x1, NUM y1, NUM x2, NUM y2);
	void Unite(const emClipRects & clipRects);
	emClipRects & operator += (const emClipRects & clipRects);
	emClipRects operator + (const emClipRects & clipRects) const;
	emClipRects & operator |= (const emClipRects & clipRects);
	emClipRects operator | (const emClipRects & clipRects) const;
		// Perform the OR operation.

	void Intersect(NUM x1, NUM y1, NUM x2, NUM y2);
	void Intersect(const emClipRects & clipRects);
	emClipRects & operator &= (const emClipRects & clipRects);
	emClipRects operator & (const emClipRects & clipRects) const;
		// Perform the AND operation.

	void Subtract(NUM x1, NUM y1, NUM x2, NUM y2);
	void Subtract(const emClipRects & clipRects);
	emClipRects & operator -= (const emClipRects & clipRects);
	emClipRects operator - (const emClipRects & clipRects) const;
		// Perform the AND-NOT operation.

	void Sort();
		// Sort the list of rectangles primarily by Y1 and secondarily
		// by X1.

	void Empty();
		// Empty the set of clip rectangles.

	bool IsEmpty() const;
		// Ask whether the set of clip rectangles is empty.

	bool IsSubsetOf(NUM x1, NUM y1, NUM x2, NUM y2) const;
	bool IsSubsetOf(const emClipRects & clipRects) const;
		// Ask whether the set of clip rectangles is contained in
		// another rectangle or set.

	bool IsSupersetOf(NUM x1, NUM y1, NUM x2, NUM y2) const;
	bool IsSupersetOf(const emClipRects & clipRects) const;
		// Ask whether the set of clip rectangles is containing
		// another rectangle or set.

	bool operator == (const emClipRects & clipRects) const;
	bool operator != (const emClipRects & clipRects) const;
		// Compare sets of clip rectangles.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this set of clip
		// rectangles.

	void MakeNonShared();
		// This must be called before handing the object to another
		// thread.

private:

	void PrivUnite(Rect * * list, NUM x1, NUM y1, NUM x2, NUM y2);
	void DeleteData();
	Rect * AllocRect();
	void FreeRect(Rect * rect);
	void AllocBlock();
	static int CompareRects(void * r1, void * r2, void * context);

	struct MemBlock {
		Rect Rects[16];
		MemBlock * Next;
	};

	struct SharedData {
		Rect * List;
		Rect * FreeList;
		MemBlock * BlockList;
		unsigned int Count;
		unsigned int RefCount;
		bool IsStaticEmpty;
	};

	SharedData * Data;
	static SharedData EmptyData;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//--------------------------- Inline implementations ---------------------------

template <class NUM> inline emClipRects<NUM>::emClipRects()
{
	Data=&EmptyData;
}


template <class NUM> inline
emClipRects<NUM>::emClipRects(const emClipRects & clipRects)
{
	Data=clipRects.Data;
	Data->RefCount++;
}


template <class NUM> inline emClipRects<NUM>::~emClipRects()
{
	if (!--Data->RefCount) DeleteData();
}


template <class NUM> inline
emClipRects<NUM> & emClipRects<NUM>::operator = (const emClipRects & clipRects)
{
	clipRects.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=clipRects.Data;
	return *this;
}


template <class NUM> inline NUM emClipRects<NUM>::Rect::GetX1() const
{
	return X1;
}


template <class NUM> inline NUM emClipRects<NUM>::Rect::GetY1() const
{
	return Y1;
}


template <class NUM> inline NUM emClipRects<NUM>::Rect::GetX2() const
{
	return X2;
}


template <class NUM> inline NUM emClipRects<NUM>::Rect::GetY2() const
{
	return Y2;
}


template <class NUM> inline
void emClipRects<NUM>::Rect::Get(NUM * pX1, NUM * pY1, NUM * pX2, NUM * pY2) const
{
	*pX1=X1;
	*pY1=Y1;
	*pX2=X2;
	*pY2=Y2;
}


template <class NUM> inline
const typename emClipRects<NUM>::Rect * emClipRects<NUM>::Rect::GetNext() const
{
	return Next;
}


template <class NUM> inline
const typename emClipRects<NUM>::Rect * emClipRects<NUM>::GetFirst() const
{
	return Data->List;
}


template <class NUM> inline int emClipRects<NUM>::GetCount() const
{
	return Data->Count;
}


template <class NUM> inline void emClipRects<NUM>::SetToMinMax()
{
	SetToMinMaxOf(*this);
}


template <class NUM> inline
void emClipRects<NUM>::Set(const emClipRects & clipRects)
{
	clipRects.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=clipRects.Data;
}


template <class NUM> inline
emClipRects<NUM> & emClipRects<NUM>::operator += (const emClipRects & clipRects)
{
	Unite(clipRects);
	return *this;
}


template <class NUM> inline
emClipRects<NUM> & emClipRects<NUM>::operator |= (const emClipRects & clipRects)
{
	return *this+=clipRects;
}


template <class NUM> inline
emClipRects<NUM> emClipRects<NUM>::operator | (const emClipRects & clipRects) const
{
	return *this+clipRects;
}


template <class NUM> inline
emClipRects<NUM> & emClipRects<NUM>::operator &= (const emClipRects & clipRects)
{
	Intersect(clipRects);
	return *this;
}


template <class NUM> inline
emClipRects<NUM> & emClipRects<NUM>::operator -= (const emClipRects & clipRects)
{
	Subtract(clipRects);
	return *this;
}


template <class NUM> inline void emClipRects<NUM>::Empty()
{
	if (!--Data->RefCount) DeleteData();
	Data=&EmptyData;
}


template <class NUM> inline bool emClipRects<NUM>::IsEmpty() const
{
	return !Data->Count;
}


template <class NUM> inline
bool emClipRects<NUM>::IsSupersetOf(const emClipRects & clipRects) const
{
	return clipRects.IsSubsetOf(*this);
}


template <class NUM> inline
bool emClipRects<NUM>::operator != (const emClipRects & clipRects) const
{
	return !(*this == clipRects);
}


template <class NUM> inline unsigned int emClipRects<NUM>::GetDataRefCount() const
{
	return Data->IsStaticEmpty ? UINT_MAX/2 : Data->RefCount;
}


template <class NUM> inline
typename emClipRects<NUM>::Rect * emClipRects<NUM>::AllocRect()
{
	Rect * r;

	r=Data->FreeList;
	if (!r) {
		AllocBlock();
		r=Data->FreeList;
	}
	Data->FreeList=r->Next;
	Data->Count++;
	return r;
}


template <class NUM> inline void emClipRects<NUM>::FreeRect(Rect * rect)
{
	Data->Count--;
	rect->Next=Data->FreeList;
	Data->FreeList=rect;
}


//------------------------- Non-inline implementations -------------------------

template <class NUM>
emClipRects<NUM>::emClipRects(NUM x1, NUM y1, NUM x2, NUM y2)
{
	Rect * r;

	Data=new SharedData;
	Data->List=NULL;
	Data->FreeList=NULL;
	Data->BlockList=NULL;
	Data->Count=0;
	Data->RefCount=1;
	Data->IsStaticEmpty=false;
	r=AllocRect();
	r->X1=x1;
	r->Y1=y1;
	r->X2=x2;
	r->Y2=y2;
	r->Next=NULL;
	Data->List=r;
}


template <class NUM>
void emClipRects<NUM>::GetMinMax(NUM * pX1, NUM * pY1, NUM * pX2, NUM * pY2) const
{
	NUM x1,y1,x2,y2;
	Rect * r;

	r=Data->List;
	if (!r) {
		x1=0;
		y1=0;
		x2=0;
		y2=0;
	}
	else {
		x1=r->X1;
		y1=r->Y1;
		x2=r->X2;
		y2=r->Y2;
		for (r=r->Next; r; r=r->Next) {
			if (x1>r->X1) x1=r->X1;
			if (y1>r->Y1) y1=r->Y1;
			if (x2<r->X2) x2=r->X2;
			if (y2<r->Y2) y2=r->Y2;
		}
	}
	*pX1=x1;
	*pY1=y1;
	*pX2=x2;
	*pY2=y2;
}


template <class NUM>
void emClipRects<NUM>::SetToMinMaxOf(const emClipRects & clipRects)
{
	NUM x1,y1,x2,y2;

	if (clipRects.Data->Count<=1) {
		clipRects.Data->RefCount++;
		if (!--Data->RefCount) DeleteData();
		Data=clipRects.Data;
	}
	else {
		GetMinMax(&x1,&y1,&x2,&y2);
		Set(x1,y1,x2,y2);
	}
}


template <class NUM>
void emClipRects<NUM>::Set(NUM x1, NUM y1, NUM x2, NUM y2)
{
	Rect * r;

	if (!--Data->RefCount) DeleteData();
	Data=new SharedData;
	Data->List=NULL;
	Data->FreeList=NULL;
	Data->BlockList=NULL;
	Data->Count=0;
	Data->RefCount=1;
	Data->IsStaticEmpty=false;
	r=AllocRect();
	r->X1=x1;
	r->Y1=y1;
	r->X2=x2;
	r->Y2=y2;
	r->Next=NULL;
	Data->List=r;
}


template <class NUM>
void emClipRects<NUM>::Unite(NUM x1, NUM y1, NUM x2, NUM y2)
{
	if (x1>=x2 || y1>=y2) return;
	MakeNonShared();
	PrivUnite(&Data->List,x1,y1,x2,y2);
}


template <class NUM>
void emClipRects<NUM>::Unite(const emClipRects & clipRects)
{
	const Rect * r;

	if (Data==clipRects.Data || !clipRects.Data) return;
	MakeNonShared();
	for (r=clipRects.Data->List; r; r=r->Next) {
		PrivUnite(&Data->List,r->X1,r->Y1,r->X2,r->Y2);
	}
}


template <class NUM>
emClipRects<NUM> emClipRects<NUM>::operator + (const emClipRects & clipRects) const
{
	emClipRects cr(*this);
	cr.Unite(clipRects);
	return cr;
}


template <class NUM>
void emClipRects<NUM>::Intersect(NUM x1, NUM y1, NUM x2, NUM y2)
{
	Rect * * pr;
	Rect * r;

	if (x1>=x2 || y1>=y2) {
		Empty();
		return;
	}
	MakeNonShared();
	pr=&Data->List;
	for (;;) {
		r=*pr;
		if (!r) break;
		if (r->X1<x1) r->X1=x1;
		if (r->X2>x2) r->X2=x2;
		if (r->X1<r->X2) {
			if (r->Y1<y1) r->Y1=y1;
			if (r->Y2>y2) r->Y2=y2;
			if (r->Y1<r->Y2) {
				pr=&r->Next;
				continue;
			}
		}
		*pr=r->Next;
		FreeRect(r);
	}
}


template <class NUM>
void emClipRects<NUM>::Intersect(const emClipRects & clipRects)
{
	Rect * r1, * r2;

	if (Data!=clipRects.Data) {
		r1=Data->List;
		if (r1) {
			r2=clipRects.Data->List;
			if (!r2) {
				Empty();
			}
			else if (!r2->Next) {
				Intersect(r2->X1,r2->Y1,r2->X2,r2->Y2);
			}
			else if (!r1->Next) {
				emClipRects cr(clipRects);
				cr.Intersect(r1->X1,r1->Y1,r1->X2,r1->Y2);
				Set(cr);
			}
			else {
				emClipRects cr;
				cr.SetToMinMaxOf(*this);
				cr.Subtract(clipRects);
				Subtract(cr);
			}
		}
	}
}


template <class NUM>
emClipRects<NUM> emClipRects<NUM>::operator & (const emClipRects & clipRects) const
{
	emClipRects cr(*this);
	cr.Intersect(clipRects);
	return cr;
}


template <class NUM>
void emClipRects<NUM>::Subtract(NUM x1, NUM y1, NUM x2, NUM y2)
{
	NUM rx1,ry1,rx2,ry2,sy1,sy2;
	Rect * * pr;
	Rect * r;

	if (!Data->List || x1>=x2 || y1>=y2) return;
	MakeNonShared();
	pr=&Data->List;
	for (;;) {
		r=*pr;
		if (!r) break;
		if (
			(rx1=r->X1)<x2 &&
			(rx2=r->X2)>x1 &&
			(ry1=r->Y1)<y2 &&
			(ry2=r->Y2)>y1
		) {
			if (ry2>y2) sy2=y2;
			else sy2=ry2;
			if (ry1<y1) {
				r->Y2=y1;
				sy1=y1;
				pr=&r->Next;
			}
			else {
				sy1=ry1;
				*pr=r->Next;
				FreeRect(r);
			}
			if (rx1<x1) {
				r=AllocRect();
				r->X1=rx1;
				r->Y1=sy1;
				r->X2=x1;
				r->Y2=sy2;
				r->Next=*pr;
				*pr=r;
				pr=&r->Next;
			}
			if (rx2>x2) {
				r=AllocRect();
				r->X1=x2;
				r->Y1=sy1;
				r->X2=rx2;
				r->Y2=sy2;
				r->Next=*pr;
				*pr=r;
				pr=&r->Next;
			}
			if (ry2>y2) {
				r=AllocRect();
				r->X1=rx1;
				r->Y1=y2;
				r->X2=rx2;
				r->Y2=ry2;
				r->Next=*pr;
				*pr=r;
				pr=&r->Next;
			}
		}
		else {
			pr=&r->Next;
		}
	}
}


template <class NUM>
void emClipRects<NUM>::Subtract(const emClipRects & clipRects)
{
	const Rect * r;

	if (Data==clipRects.Data) {
		Empty();
		return;
	}
	for (r=clipRects.Data->List; r; r=r->Next) {
		if (!Data->List) break;
		Subtract(r->X1,r->Y1,r->X2,r->Y2);
	}
}


template <class NUM>
emClipRects<NUM> emClipRects<NUM>::operator - (const emClipRects & clipRects) const
{
	emClipRects cr(*this);
	cr.Subtract(clipRects);
	return cr;
}


template <class NUM> void emClipRects<NUM>::Sort()
{
	if (Data->Count>1) {
		MakeNonShared();
		emSortSingleLinkedList(
			(void**)&Data->List,
			offsetof(Rect,Next),
			CompareRects,
			NULL
		);
	}
}


template <class NUM>
bool emClipRects<NUM>::IsSubsetOf(NUM x1, NUM y1, NUM x2, NUM y2) const
{
	Rect * r;

	for (r=Data->List; r; r=r->Next) {
		if (r->X1<x1 || r->Y1<y1 || r->X2>x2 || r->Y2>y2) return false;
	}
	return true;
}


template <class NUM>
bool emClipRects<NUM>::IsSubsetOf(const emClipRects & clipRects) const
{
	Rect * r;

	if (Data==clipRects.Data) return true;
	if (!Data->List) return true;
	r=clipRects.Data->List;
	if (!r) return false;
	if (!r->Next) return IsSubsetOf(r->X1,r->Y1,r->X2,r->Y2);
	return (*this - clipRects).IsEmpty();
}


template <class NUM>
bool emClipRects<NUM>::IsSupersetOf(NUM x1, NUM y1, NUM x2, NUM y2) const
{
	return emClipRects(x1,y1,x2,y2).IsSubsetOf(*this);
}


template <class NUM>
bool emClipRects<NUM>::operator == (const emClipRects & clipRects) const
{
	if (Data==clipRects.Data) return true;
	return IsSubsetOf(clipRects) && clipRects.IsSubsetOf(*this);
}


template <class NUM> void emClipRects<NUM>::MakeNonShared()
{
	SharedData * d1, * d2;
	Rect * r1, *r2;
	Rect * * pr;

	d1=Data;
	if (d1->RefCount>1 || d1->IsStaticEmpty) {
		d2=new SharedData;
		d2->List=NULL;
		d2->FreeList=NULL;
		d2->BlockList=NULL;
		d2->Count=0;
		d2->RefCount=1;
		d2->IsStaticEmpty=false;
		d1->RefCount--;
		Data=d2;
		r1=d1->List;
		if (r1) {
			pr=&d2->List;
			do {
				r2=AllocRect();
				r2->X1=r1->X1;
				r2->Y1=r1->Y1;
				r2->X2=r1->X2;
				r2->Y2=r1->Y2;
				*pr=r2;
				pr=&r2->Next;
				r1=r1->Next;
			} while (r1);
			*pr=NULL;
		}
	}
}


template <class NUM>
void emClipRects<NUM>::PrivUnite(Rect * * list, NUM x1, NUM y1, NUM x2, NUM y2)
{
	NUM rx1,ry1,rx2,ry2;
	Rect * r;

	for (;;) {
		r=*list;
		if (!r) break;
		if (
			(ry1=r->Y1)>y2 || (ry2=r->Y2)<y1 ||
			(rx1=r->X1)>x2 || (rx2=r->X2)<x1
		) {
			list=&r->Next;
		}
		else if (rx1<=x1 && rx2>=x2 && ry1<=y1 && ry2>=y2) {
			return;
		}
		else if (rx1>=x1 && rx2<=x2 && ry1>=y1 && ry2<=y2) {
			*list=r->Next;
			FreeRect(r);
		}
		else if (rx1==x1 && rx2==x2) {
			if (y1>ry1) y1=ry1;
			if (y2<ry2) y2=ry2;
			*list=r->Next;
			FreeRect(r);
		}
		else if (ry1<y2 && ry2>y1) {
			if (ry2>y2) {
				r->Y1=y2;
				if (ry1<y1) {
					r=AllocRect();
					r->X1=rx1;
					r->Y1=ry1;
					r->X2=rx2;
					r->Y2=y1;
					r->Next=*list;
					*list=r;
				}
			}
			else if (ry1<y1) {
				r->Y2=y1;
			}
			else {
				*list=r->Next;
				FreeRect(r);
			}
			if (y1<ry1) {
				PrivUnite(list,x1,y1,x2,ry1);
				y1=ry1;
			}
			if (y2>ry2) {
				PrivUnite(list,x1,ry2,x2,y2);
				y2=ry2;
			}
			if (x1>rx1) x1=rx1;
			if (x2<rx2) x2=rx2;
		}
		else {
			list=&r->Next;
		}
	}
	r=AllocRect();
	r->X1=x1;
	r->Y1=y1;
	r->X2=x2;
	r->Y2=y2;
	r->Next=NULL;
	*list=r;
}


template <class NUM> void emClipRects<NUM>::DeleteData()
{
	MemBlock * b;

	EmptyData.RefCount=UINT_MAX/2;

	// Never do a
	//  if (Data!=&EmptyData)...
	// instead of
	//  if (!Data->IsStaticEmpty)...
	// because static member variables of template classes could exist
	// multiple times for the same final type (e.g. with Windows DLLs).
	if (!Data->IsStaticEmpty) {
		while ((b=Data->BlockList)!=NULL) {
			Data->BlockList=b->Next;
			delete b;
		}
		delete Data;
	}
}


template <class NUM> void emClipRects<NUM>::AllocBlock()
{
	MemBlock * b;
	Rect * r, * e;
	int n;

	b=new MemBlock;
	b->Next=Data->BlockList;
	Data->BlockList=b;
	n=sizeof(b->Rects)/sizeof(Rect);
	e=b->Rects+n-1;
	for (r=b->Rects; r<e; r++) r->Next=r+1;
	e->Next=Data->FreeList;
	Data->FreeList=b->Rects;
}


template <class NUM>
int emClipRects<NUM>::CompareRects(void * r1, void * r2, void * context)
{
	if (((const Rect*)r1)->Y1<((const Rect*)r2)->Y1) return -1;
	if (((const Rect*)r1)->Y1>((const Rect*)r2)->Y1) return 1;
	if (((const Rect*)r1)->X1<((const Rect*)r2)->X1) return -1;
	if (((const Rect*)r1)->X1>((const Rect*)r2)->X1) return 1;
	return 0;
}


template <class NUM>
typename emClipRects<NUM>::SharedData emClipRects<NUM>::EmptyData=
{
	NULL,NULL,NULL,0,UINT_MAX/2,true
};


#endif
