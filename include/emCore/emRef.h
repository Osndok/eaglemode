//------------------------------------------------------------------------------
// emRef.h
//
// Copyright (C) 2005-2008,2010,2016,2024 Oliver Hamann.
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

#ifndef emRef_h
#define emRef_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//================================ emRefTarget =================================
//==============================================================================

class emRefTarget {

public:

	// Base class for an object that can be referenced by an emRef.

	emRefTarget();
		// Construct with zero reference count.

	virtual ~emRefTarget();
		// The destructor calls emFatalError when the reference count
		// isn't zero.

	emRefTarget(const emRefTarget& other);
	emRefTarget & operator = (const emRefTarget& other);
		// When copying an emRefTarget, the reference count is not
		// copied.

	void Alloc();
		// Increment the reference count by one. This method is usually
		// only to be called by emRef and special containers.

	void Free();
		// Decrement the reference count by one. If it reaches zero,
		// then this object is deleted. This method is usually only to
		// be called by emRef and special containers.

	int GetRefCount() const;
		// Get the reference count.

private:
	int RefCount;
};


//==============================================================================
//=================================== emRef ====================================
//==============================================================================

template <class CLS> class emRef {

public:

	// Template class for a reference to an emRefTarget, emModel, or to a
	// similar class which also has the methods Alloc and Free. The template
	// parameter CLS must be the referenceable object class. On each
	// construction of a non-NULL reference, Alloc is called on the object.
	// And on each destruction of a non-NULL reference, Free is called on
	// the object. Reference assignment behaves like destructing and
	// constructing. Through Alloc and Free, the object increments and
	// decrements an internal reference counter, and it deletes itself in
	// Free when the reference counter gets zero. Note that a common emModel
	// is also referred by its emContext.

	emRef();
		// Construct a NULL reference.

	emRef(const emRef & ref);
		// Construct a copied reference.

	emRef(CLS * model);
		// Construct from a pointer (NULL is allowed).

	~emRef();
		// Destructor.

	emRef & operator = (const emRef & ref);
	emRef & operator = (CLS * model);
		// Copy from a reference or pointer (NULL-pointer is allowed).

	operator CLS * () const;
		// Cast this reference to a pointer (can be NULL).

	CLS * Get() const;
		// Get the pointer (can be NULL).

	CLS * operator -> () const;
		// This makes the reference a so-called "smart pointer". For
		// example, if r is reference to a model which has a method
		// named Hello(), one could say r->Hello() instead of
		// r.Get()->Hello().

private:

	CLS * Mdl;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline emRefTarget::emRefTarget()
	: RefCount(0)
{
}

inline emRefTarget::emRefTarget(const emRefTarget&)
	: RefCount(0)
{
}

inline emRefTarget & emRefTarget::operator = (const emRefTarget&)
{
	return *this;
}

inline void emRefTarget::Alloc()
{
	RefCount++;
}

inline void emRefTarget::Free()
{
	RefCount--;
	if (RefCount<=0) {
		delete this;
	}
}

inline int emRefTarget::GetRefCount() const
{
	return RefCount;
}

template <class CLS> inline emRef<CLS>::emRef()
{
	Mdl=NULL;
}

template <class CLS> inline emRef<CLS>::emRef(const emRef & ref)
{
	Mdl=ref.Mdl;
	if (Mdl) Mdl->Alloc();
}

template <class CLS> inline emRef<CLS>::emRef(CLS * model)
{
	Mdl=model;
	if (model) model->Alloc();
}

template <class CLS> inline emRef<CLS>::~emRef()
{
	if (Mdl) {
		Mdl->Free();
		Mdl=NULL;
	}
}

template <class CLS> emRef<CLS> & emRef<CLS>::operator = (const emRef & ref)
{
	if (ref.Mdl) ref.Mdl->Alloc();
	if (Mdl) Mdl->Free();
	Mdl=ref.Mdl;
	return *this;
}

template <class CLS> emRef<CLS> & emRef<CLS>::operator = (CLS * model)
{
	if (model) model->Alloc();
	if (Mdl) Mdl->Free();
	Mdl=model;
	return *this;
}

template <class CLS> inline emRef<CLS>::operator CLS * () const
{
	return Mdl;
}

template <class CLS> inline CLS * emRef<CLS>::Get() const
{
	return Mdl;
}

template <class CLS> inline CLS * emRef<CLS>::operator -> () const
{
	return Mdl;
}


#endif
