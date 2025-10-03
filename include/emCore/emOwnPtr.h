//------------------------------------------------------------------------------
// emOwnPtr.h
//
// Copyright (C) 2024 Oliver Hamann.
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

#ifndef emOwnPtr_h
#define emOwnPtr_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//================================ emOwnPtrBase ================================
//==============================================================================

template <class OBJECT, class DELETER> class emOwnPtrBase {

public:

	// Template class for a smart pointer which owns the referenced object.
	// The object is deleted by calling the given DELETER when the pointer
	// is destructed.

	emOwnPtrBase(OBJECT * ptr=NULL);
		// Construct from an object pointer. Ownership of that object
		// goes to this emOwnPtrBase. NULL means to own no object.

	~emOwnPtrBase();
		// Destructor. This deletes the owned object.

	void Set(OBJECT * ptr = NULL);
	emOwnPtrBase & operator = (OBJECT * ptr);
		// Assign an object pointer and take ownership of the pointed
		// object. Assigning NULL is allowed. The previously owned
		// object is deleted.

	void Reset();
		// Same as Set(NULL).

	OBJECT * Release();
		// Release ownership of the object, make this emOwnPtrBase a
		// NULL pointer, and return the object pointer.

	operator const OBJECT * () const;
	operator OBJECT * ();
		// Cast this to a normal pointer (can be NULL).

	const OBJECT * Get() const;
	OBJECT * Get();
		// Get the normal pointer (can be NULL).

	const OBJECT * operator -> () const;
	OBJECT * operator -> ();
		// This makes this class a so-called "smart pointer". For
		// example, if p is an emOwnPtrBase to an object which has a
		// method named Hello(), one could say p->Hello() instead of
		// p.Get()->Hello().

private:

	emOwnPtrBase(const emOwnPtrBase&);
	emOwnPtrBase & operator = (const emOwnPtrBase&);
		// Copying is not allowed.

	OBJECT * Ptr;
};


template <class OBJECT> struct emOwnPtrDeleter {
	void operator () (OBJECT * ptr) const;
};

template <class OBJECT> struct emOwnArrayPtrDeleter {
	void operator () (OBJECT * ptr) const;
};


//==============================================================================
//================================== emOwnPtr ==================================
//==============================================================================

template <class OBJECT> class emOwnPtr :
	public emOwnPtrBase<OBJECT, emOwnPtrDeleter<OBJECT> > {

public:

	// Like emOwnPtrBase, but the object is deleted by calling the normal
	// delete operator.

	emOwnPtr(OBJECT * ptr=NULL);
	emOwnPtr & operator = (OBJECT * ptr);
};


//==============================================================================
//=============================== emOwnArrayPtr ================================
//==============================================================================

template <class OBJECT> class emOwnArrayPtr :
	public emOwnPtrBase<OBJECT, emOwnArrayPtrDeleter<OBJECT> > {

public:

	// Like emOwnPtrBase, but the object is deleted by calling the array
	// delete operator (delete []).

	emOwnArrayPtr(OBJECT * ptr=NULL);
	emOwnArrayPtr & operator = (OBJECT * ptr);
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER>::emOwnPtrBase(OBJECT * ptr)
	: Ptr(ptr)
{
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER>::~emOwnPtrBase()
{
	if (Ptr) {
		DELETER()(Ptr);
		Ptr=NULL;
	}
}

template <class OBJECT, class DELETER>
inline void emOwnPtrBase<OBJECT, DELETER>::Set(OBJECT * ptr)
{
	if (Ptr!=ptr) {
		if (Ptr) DELETER()(Ptr);
		Ptr=ptr;
	}
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER> & emOwnPtrBase<OBJECT, DELETER>::operator =
	(OBJECT * ptr)
{
	Set(ptr);
	return *this;
}

template <class OBJECT, class DELETER>
inline void emOwnPtrBase<OBJECT, DELETER>::Reset()
{
	Set(NULL);
}

template <class OBJECT, class DELETER>
inline OBJECT * emOwnPtrBase<OBJECT, DELETER>::Release()
{
	OBJECT * ptr = Ptr;
	Ptr=NULL;
	return ptr;
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER>::operator const OBJECT * () const
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER>::operator OBJECT * ()
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline const OBJECT * emOwnPtrBase<OBJECT, DELETER>::Get() const
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline OBJECT * emOwnPtrBase<OBJECT, DELETER>::Get()
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline const OBJECT * emOwnPtrBase<OBJECT, DELETER>::operator -> () const
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline OBJECT * emOwnPtrBase<OBJECT, DELETER>::operator -> ()
{
	return Ptr;
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER>::emOwnPtrBase(const emOwnPtrBase&)
{
}

template <class OBJECT, class DELETER>
inline emOwnPtrBase<OBJECT, DELETER> & emOwnPtrBase<OBJECT, DELETER>::operator =
	(const emOwnPtrBase&)
{
	return *this;
}

template <class OBJECT>
inline void emOwnPtrDeleter<OBJECT>::operator () (OBJECT * ptr) const {
	delete ptr;
}

template <class OBJECT>
inline void emOwnArrayPtrDeleter<OBJECT>::operator () (OBJECT * ptr) const {
	delete [] ptr;
}

template <class OBJECT>
inline emOwnPtr<OBJECT>::emOwnPtr(OBJECT * ptr)
	: emOwnPtrBase<OBJECT, emOwnPtrDeleter<OBJECT> >(ptr)
{
}

template <class OBJECT>
inline emOwnPtr<OBJECT> & emOwnPtr<OBJECT>::operator = (OBJECT * ptr)
{
	this->Set(ptr);
	return *this;
}

template <class OBJECT>
inline emOwnArrayPtr<OBJECT>::emOwnArrayPtr(OBJECT * ptr)
	: emOwnPtrBase<OBJECT, emOwnArrayPtrDeleter<OBJECT> >(ptr)
{
}

template <class OBJECT>
inline emOwnArrayPtr<OBJECT> & emOwnArrayPtr<OBJECT>::operator = (OBJECT * ptr)
{
	this->Set(ptr);
	return *this;
}


#endif
