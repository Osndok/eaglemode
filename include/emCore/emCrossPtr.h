//------------------------------------------------------------------------------
// emCrossPtr.h
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

#ifndef emCrossPtr_h
#define emCrossPtr_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif

class emCrossPtrPrivate {
protected:
	friend class emCrossPtrList;
	void Unlink();
	void * Obj;
	emCrossPtrPrivate * * ThisPtr;
	emCrossPtrPrivate * Next;
};


//==============================================================================
//================================= emCrossPtr =================================
//==============================================================================

template <class CLS> class emCrossPtr : private emCrossPtrPrivate {

public:

	// Template class for an object pointer which is automatically set to
	// NULL when the object is destructed. It is something like a so-called
	// weak reference (in compare to emRef). The name "cross pointer" comes
	// from the fact that these pointers can be safely used to cross the
	// ownership hierarchy.
	//
	// The template parameter CLS is the class of the object to be pointed.
	// It must be an emCrossPtrList or a class that has the method
	// LinkCrossPtr(emCrossPtrPrivate) which forwards the call to a member
	// emCrossPtrList. Typical candidates are emContext, emModel, emPanel
	// and their derivatives.

	emCrossPtr();
		// Construct a NULL pointer.

	emCrossPtr(const emCrossPtr & crossPtr);
		// Construct a copied pointer.

	emCrossPtr(CLS * obj);
		// Construct from a normal pointer (NULL is allowed).

	~emCrossPtr();
		// Destructor.

	emCrossPtr & operator = (const emCrossPtr & crossPtr);
	emCrossPtr & operator = (CLS * obj);
		// Copy operators (NULL-pointer is allowed).

	operator CLS * () const;
		// Cast this to a normal pointer (can be NULL).

	CLS * Get() const;
		// Get the normal pointer (can be NULL).

	CLS * operator -> () const;
		// This makes this class a so-called "smart pointer". For
		// example, if p is an emCrossPtr to an object which has a
		// method named Hello(), one could say p->Hello() instead of
		// p.Get()->Hello().
};


//==============================================================================
//=============================== emCrossPtrList ===============================
//==============================================================================

class emCrossPtrList : public emUncopyable {

public:

	// Class for a list of cross pointers pointing to an object.

	emCrossPtrList();
		// Start with an empty list.

	~emCrossPtrList();
		// Like BreakCrossPtrs().

	void BreakCrossPtrs();
		// Remove all cross pointers and set them to NULL. This could be
		// called at the beginning of a destructor of an object for
		// getting rid of the pointers a little bit earlier than through
		// the destructor of this list.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// Insert a cross pointer to this list (the field crossPtr.Obj
		// is not touched by this). This method is only to be called by
		// emCrossPtr or by forwarding from an object class instance to
		// a member instance of this class.

private:
	emCrossPtrPrivate * First;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class CLS> inline emCrossPtr<CLS>::emCrossPtr()
{
	Obj=NULL;
}

template <class CLS> inline emCrossPtr<CLS>::emCrossPtr(
	const emCrossPtr & crossPtr
)
{
	Obj=crossPtr.Obj;
	if (Obj) ((CLS*)Obj)->LinkCrossPtr(*this);
}

template <class CLS> inline emCrossPtr<CLS>::emCrossPtr(CLS * obj)
{
	Obj=obj;
	if (obj) obj->LinkCrossPtr(*this);
}

template <class CLS> inline emCrossPtr<CLS>::~emCrossPtr()
{
	if (Obj) Unlink();
}

template <class CLS> emCrossPtr<CLS> & emCrossPtr<CLS>::operator = (
	const emCrossPtr & crossPtr
)
{
	if (Obj) Unlink();
	Obj=crossPtr.Obj;
	if (Obj) ((CLS*)Obj)->LinkCrossPtr(*this);
	return *this;
}

template <class CLS> emCrossPtr<CLS> & emCrossPtr<CLS>::operator = (CLS * obj)
{
	if (Obj) Unlink();
	Obj=obj;
	if (obj) obj->LinkCrossPtr(*this);
	return *this;
}

template <class CLS> inline emCrossPtr<CLS>::operator CLS * () const
{
	return (CLS*)Obj;
}

template <class CLS> inline CLS * emCrossPtr<CLS>::Get() const
{
	return (CLS*)Obj;
}

template <class CLS> inline CLS * emCrossPtr<CLS>::operator -> () const
{
	return (CLS*)Obj;
}

inline emCrossPtrList::emCrossPtrList()
{
	First=NULL;
}

inline emCrossPtrList::~emCrossPtrList()
{
	BreakCrossPtrs();
}


#endif
