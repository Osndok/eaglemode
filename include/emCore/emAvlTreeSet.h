//------------------------------------------------------------------------------
// emAvlTreeSet.h
//
// Copyright (C) 2016 Oliver Hamann.
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

#ifndef emAvlTreeSet_h
#define emAvlTreeSet_h

#ifndef emAvlTree_h
#include <emCore/emAvlTree.h>
#endif


//==============================================================================
//================================ emAvlTreeSet ================================
//==============================================================================

template <class OBJ> class emAvlTreeSet {

public:

	// Template class for an AVL tree which holds a sorted set of unique
	// elements. This class provides copy-on-write behavior and stable
	// iterators. The template parameter OBJ describes the type of the
	// elements. Elements are compared with the normal comparison operators
	// (==, <=, >...).

	emAvlTreeSet();
		// Construct an empty set.

	emAvlTreeSet(const emAvlTreeSet & src);
		// Construct a copied set.

	emAvlTreeSet(const OBJ & obj);
		// Construct a set with one element.

	~emAvlTreeSet();
		// Destructor.

	emAvlTreeSet & operator = (const emAvlTreeSet & set);
		// Make this set a copy of the given set.

	bool Contains(const OBJ & obj) const;
		// Ask whether this set contains an element which equals the
		// given object.

	const OBJ * Get(const OBJ & obj) const;
		// Get a pointer to the element which equals the given object.
		// If there is no such element, NULL is returned. At least
		// because of the copy-on-write feature, the pointer is valid
		// only until calling any non-const method or operator on this
		// set, or giving this set as a non-const argument to any call
		// in the world.

	const OBJ * GetFirst() const;
	const OBJ * GetLast() const;
		// Get the smallest or largest element. If the set is empty,
		// NULL is returned. The rules for the validity of the
		// pointer are the same as with Get(obj).

	const OBJ * GetNearestGreater(const OBJ & obj) const;
	const OBJ * GetNearestGreaterOrEqual(const OBJ & obj) const;
	const OBJ * GetNearestLess(const OBJ & obj) const;
	const OBJ * GetNearestLessOrEqual(const OBJ & obj) const;
		// Get the nearest element which is greater, or greater or
		// equal, or less, or less or equal to a given object. If no
		// such element exists, NULL is returned. The rules for the
		// validity of the pointer are the same as with Get(obj).

	OBJ * GetWritable(const OBJ & obj, bool insertIfNew);
	OBJ * GetWritable(const OBJ * elem);
		// Get a non-const version of a pointer to an element of this
		// set. If insertIfNew is true, the element is created if it is
		// not found. The pointer may be used for modifying the object
		// in a way that the order is not disturbed. The rules for the
		// validity of the pointer are the same as with Get(obj), but:
		// The pointer must not be used for modifying after doing
		// something which could have made a shallow copy of this list.

	void Insert(const OBJ & obj);
	void Insert(const emAvlTreeSet & set);
		// Insert one or more objects. Each objects is only inserted, if
		// there is not already an equal element contained in the set.

	void RemoveFirst();
	void RemoveLast();
	void Remove(const OBJ & obj);
	void Remove(const OBJ * elem);
	void Remove(const emAvlTreeSet & set);
		// Remove (and delete) elements from this set.

	void Intersect(const emAvlTreeSet & set);
		// Remove (and delete) all elements from this set which are not
		// contained in a given set.

	void Clear();
		// Remove (and delete) all elements of this set.

	bool IsEmpty() const;
		// Ask whether this set has no elements.

	int GetCount() const;
		// Compute the number of elements.

	bool operator == (const emAvlTreeSet & set) const;
	bool operator != (const emAvlTreeSet & set) const;
		// Operators for comparing sets.

	emAvlTreeSet & operator += (const OBJ & obj);
	emAvlTreeSet & operator += (const emAvlTreeSet & set);
	emAvlTreeSet operator + (const OBJ & obj) const;
	emAvlTreeSet operator + (const emAvlTreeSet & set) const;
		// Operators for uniting sets.

	emAvlTreeSet & operator |= (const OBJ & obj);
	emAvlTreeSet & operator |= (const emAvlTreeSet & set);
	emAvlTreeSet operator | (const OBJ & obj) const;
	emAvlTreeSet operator | (const emAvlTreeSet & set) const;
		// Operators for uniting sets (same as +).

	emAvlTreeSet & operator -= (const OBJ & obj);
	emAvlTreeSet & operator -= (const emAvlTreeSet & set);
	emAvlTreeSet operator - (const OBJ & obj) const;
	emAvlTreeSet operator - (const emAvlTreeSet & set) const;
		// Operators for subtracting sets.

	emAvlTreeSet & operator &= (const OBJ & obj);
	emAvlTreeSet & operator &= (const emAvlTreeSet & set);
	emAvlTreeSet operator & (const OBJ & obj) const;
	emAvlTreeSet operator & (const emAvlTreeSet & set) const;
		// Operators for intersecting sets.

	//friend emAvlTreeSet operator + (const OBJ & obj, const emAvlTreeSet & set);
	//friend emAvlTreeSet operator | (const OBJ & obj, const emAvlTreeSet & set);
	//friend emAvlTreeSet operator - (const OBJ & obj, const emAvlTreeSet & set);
	//friend emAvlTreeSet operator & (const OBJ & obj, const emAvlTreeSet & set);
		// These ones even exist and can be used.
		// (Having the declaration here would not be portable)

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this set.

	void MakeNonShared();
		// This must be called before handing the set to another thread.
		// This method is not recursive. So if the object class
		// even has such a method, you have to call that manually.

	class Iterator {

	public:

		// Class for a stable pointer to an element of a set.
		// "stable" means:
		// * If the address of an element changes through the
		//   copy-on-write mechanism, iterators pointing to that element
		//   are adapted proper.
		// * If an element is removed from a set, iterators pointing to
		//   that element are set to the next element, or NULL if it was
		//   the last element.
		// * If the assignment operator '=' is called on a set, all
		//   iterators which were pointing to elements of the set are
		//   set to NULL. This is even true if the set is assigned to
		//   itself.
		// This kind of iterator needs little more than 500 bytes of
		// memory, because it manages a stack of tree nodes for the
		// current position (yes, the AVL nodes do not have parent
		// pointers...).
		// Modifying the set while an iterator is active slows down the
		// iterator, because it has to find the tree position again on
		// next increment or decrement.
		// Note the auto-cast operator to a 'const OBJ *'. Wherever
		// there is an argument 'const OBJ *' in the methods of
		// emAvlTreeSet, you can even give an instance of this class as
		// the argument.

		Iterator();
			// Construct a "NULL pointer".

		Iterator(const Iterator & iter);
			// Construct a copied iterator.

		Iterator(const emAvlTreeSet<OBJ> & set, const OBJ * elem);
			// Construct an iterator pointing to a particular
			// element.
			// Arguments:
			//   set  - The set.
			//   elem - Pointer to an element of the set, or NULL.

		~Iterator();
			// Destructor.

		Iterator & operator = (const Iterator & iter);
			// Copy an iterator.

		operator const OBJ * () const;
		const OBJ * operator * () const;
		const OBJ * operator -> () const;
		const OBJ * Get() const;
			// Get the element pointer. It is NULL if this iterator
			// does not point to any element.

		const OBJ * Set(const Iterator & iter);
			// Copy the given iterator and return the element
			// pointer.

		const OBJ * Set(const emAvlTreeSet<OBJ> & set, const OBJ * elem);
			// Set this iterator to the given element of the given
			// set and return the element pointer.

		const OBJ * SetFirst(const emAvlTreeSet<OBJ> & set);
		const OBJ * SetLast(const emAvlTreeSet<OBJ> & set);
			// Set this iterator to the first or last element of the
			// given set and return the element pointer.

		const OBJ * SetNext();
		const OBJ * SetPrev();
		const OBJ * operator ++();
		const OBJ * operator --();
			// Set this iterator to the next or previous element and
			// return the new element pointer. This must be called
			// only if the old element pointer is not NULL.

		const OBJ * operator ++(int);
		const OBJ * operator --(int);
			// Like above, but return the old element pointer.

		bool operator == (const Iterator & iter) const;
		bool operator != (const Iterator & iter) const;
		bool operator == (const OBJ * elem) const;
		bool operator != (const OBJ * elem) const;
			// Ordinary compare operators.

		const emAvlTreeSet<OBJ> * GetSet() const;
			// Get a pointer to the set this iterator is currently
			// attached to. Returns NULL if not attached to any
			// set. (See comments on Detach()).

		void Detach();
			// Detach this iterator from its set and point to NULL.
			// Note: to care about the iterators, each emAvlTreeSet
			// has a single linked list of its iterators. The
			// mechanism is lazy, that means, an iterator may stay
			// in the set even when not pointing to any element,
			// just for quick re-use. On the other hand, such
			// iterators are still costing a tiny number of CPU
			// cycles whenever the set is modified.

	private:
		friend class emAvlTreeSet<OBJ>;
		void SetSet(const emAvlTreeSet<OBJ> * set);
		const OBJ * SetPos(const OBJ & obj);
		const OBJ * Pos;
		emAvlIterator AvlIter;
		bool AvlIterValid;
		emAvlTreeSet<OBJ> * Ats;
		Iterator * NextIter; // Undefined if Ats==NULL
	};

private:
	friend class Iterator;

	struct Element {
		OBJ Obj;
		emAvlNode AvlNode;
		inline Element(const OBJ & obj) : Obj(obj) {}
	};
	struct SharedData {
		emAvlTree AvlTree;
		bool IsStaticEmpty;
		unsigned int RefCount;
	};

	void MakeWritable(const OBJ * * preserve=NULL);
	void DeleteData();
	emAvlNode * CloneTree(emAvlNode * tree, const OBJ * * preserve);
	static void DeleteTree(emAvlNode * tree);

	SharedData * Data;
	Iterator * Iterators;
	static SharedData EmptyData;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class OBJ> inline emAvlTreeSet<OBJ>::emAvlTreeSet()
{
	Iterators=NULL;
	Data=&EmptyData;
}

template <class OBJ> inline emAvlTreeSet<OBJ>::emAvlTreeSet(
	const emAvlTreeSet & src
)
{
	Iterators=NULL;
	Data=src.Data;
	Data->RefCount++;
}

template <class OBJ> emAvlTreeSet<OBJ>::emAvlTreeSet(const OBJ & obj)
{
	Iterators=NULL;
	Data=&EmptyData;
	Insert(obj);
}

template <class OBJ> emAvlTreeSet<OBJ>::~emAvlTreeSet()
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) { i->Pos=NULL; i->Ats=NULL; }
	if (!--Data->RefCount) DeleteData();
}

template <class OBJ> emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator = (
	const emAvlTreeSet & set
)
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	set.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=set.Data;
	return *this;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::Contains(
	const OBJ & obj
) const
{
	return Get(obj)!=NULL;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Get(const OBJ & obj) const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj<element->Obj) EM_AVL_SEARCH_GO_LEFT
		else if (obj>element->Obj) EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return &element->Obj;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetFirst() const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		EM_AVL_SEARCH_GO_LEFT_OR_FOUND
	EM_AVL_SEARCH_END
	return &element->Obj;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetLast() const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		EM_AVL_SEARCH_GO_RIGHT_OR_FOUND
	EM_AVL_SEARCH_END
	return &element->Obj;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetNearestGreater(
	const OBJ & obj
) const
{
	EM_AVL_SEARCH_VARS(Element)
	const OBJ * nearest;

	nearest=NULL;
	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj<element->Obj) { nearest=&element->Obj; EM_AVL_SEARCH_GO_LEFT }
		else EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return nearest;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetNearestGreaterOrEqual(
	const OBJ & obj
) const
{
	EM_AVL_SEARCH_VARS(Element)
	const OBJ * nearest;

	nearest=NULL;
	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj<element->Obj) { nearest=&element->Obj; EM_AVL_SEARCH_GO_LEFT }
		else if (obj>element->Obj) EM_AVL_SEARCH_GO_RIGHT
		else nearest=&element->Obj;
	EM_AVL_SEARCH_END
	return nearest;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetNearestLess(
	const OBJ & obj
) const
{
	EM_AVL_SEARCH_VARS(Element)
	const OBJ * nearest;

	nearest=NULL;
	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj>element->Obj) { nearest=&element->Obj; EM_AVL_SEARCH_GO_RIGHT }
		else EM_AVL_SEARCH_GO_LEFT
	EM_AVL_SEARCH_END
	return nearest;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::GetNearestLessOrEqual(
	const OBJ & obj
) const
{
	EM_AVL_SEARCH_VARS(Element)
	const OBJ * nearest;

	nearest=NULL;
	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj<element->Obj) EM_AVL_SEARCH_GO_LEFT
		else if (obj>element->Obj) { nearest=&element->Obj; EM_AVL_SEARCH_GO_RIGHT }
		else nearest=&element->Obj;
	EM_AVL_SEARCH_END
	return nearest;
}

template <class OBJ> OBJ * emAvlTreeSet<OBJ>::GetWritable(
	const OBJ & obj, bool insertIfNew
)
{
	if (insertIfNew) {
		if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable();
		EM_AVL_INSERT_VARS(Element)
		EM_AVL_INSERT_BEGIN_SEARCH(Element,AvlNode,Data->AvlTree)
			if (obj<element->Obj) EM_AVL_INSERT_GO_LEFT
			else if (obj>element->Obj) EM_AVL_INSERT_GO_RIGHT
			else return &element->Obj;
		EM_AVL_INSERT_END_SEARCH
		element=new Element(obj);
		Iterator * i;
		for (i=Iterators; i; i=i->NextIter) {
			i->AvlIterValid=false;
		}
		EM_AVL_INSERT_NOW(AvlNode)
		return &element->Obj;
	}
	else {
		const OBJ * elem = Get(obj);
		if (!elem) return NULL;
		if (Data->RefCount>1) MakeWritable(&elem);
		return (OBJ*)elem;
	}
}

template <class OBJ> OBJ * emAvlTreeSet<OBJ>::GetWritable(
	const OBJ * elem
)
{
	if (!elem) return NULL;
	if (Data->RefCount>1) MakeWritable(&elem);
	return (OBJ*)elem;
}

template <class OBJ> inline void emAvlTreeSet<OBJ>::Insert(const OBJ & obj)
{
	GetWritable(obj,true);
}

template <class OBJ> void emAvlTreeSet<OBJ>::Insert(const emAvlTreeSet & set)
{
	emAvlTreeSet::Iterator i;

	for (i.SetFirst(set); i; ++i) {
		Insert(*i.Get());
	}
}

template <class OBJ> inline void emAvlTreeSet<OBJ>::RemoveFirst()
{
	Remove(GetFirst());
}

template <class OBJ> inline void emAvlTreeSet<OBJ>::RemoveLast()
{
	Remove(GetLast());
}

template <class OBJ> void emAvlTreeSet<OBJ>::Remove(const OBJ & obj)
{
	EM_AVL_REMOVE_VARS(Element)
	Iterator * i;

	if (Data->RefCount>1 && !Data->IsStaticEmpty) MakeWritable();
	EM_AVL_REMOVE_BEGIN(Element,AvlNode,Data->AvlTree)
		if (obj<element->Obj) {
			EM_AVL_REMOVE_GO_LEFT
		}
		else if (obj>element->Obj) {
			EM_AVL_REMOVE_GO_RIGHT
		}
		else {
			for (i=Iterators; i; i=i->NextIter) {
				if (i->Pos==&element->Obj) i->SetNext();
				i->AvlIterValid=false;
			}
			EM_AVL_REMOVE_NOW
			delete element;
		}
	EM_AVL_REMOVE_END
}

template <class OBJ> void emAvlTreeSet<OBJ>::Remove(const OBJ * elem)
{
	if (elem) Remove(*elem);
}

template <class OBJ> void emAvlTreeSet<OBJ>::Remove(const emAvlTreeSet & set)
{
	emAvlTreeSet::Iterator i;

	if (Data != set.Data) {
		for (i.SetFirst(set); i; ++i) {
			Remove(*i.Get());
		}
	}
	else {
		Clear();
	}
}

template <class OBJ> void emAvlTreeSet<OBJ>::Intersect(const emAvlTreeSet & set)
{
	emAvlTreeSet::Iterator i;

	for (i.SetFirst(*this); i;) {
		if (!set.Contains(*i.Get())) Remove(*i);
		else ++i;
	}
}

template <class OBJ> void emAvlTreeSet<OBJ>::Clear()
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	if (!--Data->RefCount) DeleteData();
	Data=&EmptyData;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::IsEmpty() const
{
	return !Data->AvlTree;
}

template <class OBJ> int emAvlTreeSet<OBJ>::GetCount() const
{
	EM_AVL_LOOP_VARS(Element)
	int count;

	count=0;
	EM_AVL_LOOP_START(Element,AvlNode,Data->AvlTree)
		count++;
	EM_AVL_LOOP_END
	return count;
}

template <class OBJ> bool emAvlTreeSet<OBJ>::operator == (
	const emAvlTreeSet & set
) const
{
	emAvlTreeSet::Iterator i,j;

	if (Data != set.Data) {
		for (i.SetFirst(*this), j.SetFirst(set); ; ++i, ++j) {
			if (!i) {
				if (j) return false;
				break;
			}
			if (!j) {
				if (i) return false;
				break;
			}
			if (*i.Get() != *j.Get()) return false;
		}
	}
	return true;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::operator != (
	const emAvlTreeSet & set
) const
{
	return !(*this == set);
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator += (
	const OBJ & obj
)
{
	Insert(obj);
	return *this;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator += (
	const emAvlTreeSet & set
)
{
	Insert(set);
	return *this;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator + (
	const OBJ & obj
) const
{
	emAvlTreeSet result(*this);
	result += obj;
	return result;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator + (
	const emAvlTreeSet & set
) const
{
	emAvlTreeSet result(*this);
	result += set;
	return result;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator |= (
	const OBJ & obj
)
{
	return *this += obj;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator |= (
	const emAvlTreeSet & set
)
{
	return *this += set;
}

template <class OBJ> inline emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator | (
	const OBJ & obj
) const
{
	return *this + obj;
}

template <class OBJ> inline emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator | (
	const emAvlTreeSet & set
) const
{
	return *this + set;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator -= (
	const OBJ & obj
)
{
	Remove(obj);
	return *this;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator -= (
	const emAvlTreeSet & set
)
{
	Remove(set);
	return *this;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator - (
	const OBJ & obj
) const
{
	emAvlTreeSet result(*this);
	result -= obj;
	return result;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator - (
	const emAvlTreeSet & set
) const
{
	emAvlTreeSet result(*this);
	result -= set;
	return result;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator &= (
	const OBJ & obj
)
{
	Intersect(emAvlTreeSet(obj));
	return *this;
}

template <class OBJ> inline emAvlTreeSet<OBJ> & emAvlTreeSet<OBJ>::operator &= (
	const emAvlTreeSet & set
)
{
	Intersect(set);
	return *this;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator & (
	const OBJ & obj
) const
{
	emAvlTreeSet result(*this);
	result &= obj;
	return result;
}

template <class OBJ> emAvlTreeSet<OBJ> emAvlTreeSet<OBJ>::operator & (
	const emAvlTreeSet & set
) const
{
	emAvlTreeSet result(*this);
	result &= set;
	return result;
}

template <class OBJ> inline emAvlTreeSet<OBJ> operator + (
	const OBJ & obj, const emAvlTreeSet<OBJ> & set
)
{
	return set + obj;
}

template <class OBJ> inline emAvlTreeSet<OBJ> operator | (
	const OBJ & obj, const emAvlTreeSet<OBJ> & set
)
{
	return set | obj;
}

template <class OBJ> emAvlTreeSet<OBJ> operator - (
	const OBJ & obj, const emAvlTreeSet<OBJ> & set
)
{
	if (!set.Contains(obj)) return emAvlTreeSet<OBJ>(obj);
	else return emAvlTreeSet<OBJ>();
}

template <class OBJ> inline emAvlTreeSet<OBJ> operator & (
	const OBJ & obj, const emAvlTreeSet<OBJ> & set
)
{
	return set & obj;
}

template <class OBJ> unsigned int emAvlTreeSet<OBJ>::GetDataRefCount() const
{
	return Data->IsStaticEmpty ? UINT_MAX/2 : Data->RefCount;
}

template <class OBJ> inline void emAvlTreeSet<OBJ>::MakeNonShared()
{
	MakeWritable();
}

template <class OBJ> inline emAvlTreeSet<OBJ>::Iterator::Iterator()
{
	Pos=NULL;
	Ats=NULL;
}

template <class OBJ> emAvlTreeSet<OBJ>::Iterator::Iterator(
	const typename emAvlTreeSet<OBJ>::Iterator & iter
)
{
	Pos=NULL;
	Ats=NULL;
	Set(iter);
}

template <class OBJ> emAvlTreeSet<OBJ>::Iterator::Iterator(
	const emAvlTreeSet<OBJ> & set,
	const OBJ * elem
)
{
	Pos=NULL;
	Ats=NULL;
	Set(set,elem);
}

template <class OBJ> emAvlTreeSet<OBJ>::Iterator::~Iterator()
{
	Iterator * * pi;

	if (Ats) {
		for (pi=&Ats->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
	}
}

template <class OBJ> inline
typename emAvlTreeSet<OBJ>::Iterator & emAvlTreeSet<OBJ>::Iterator::operator = (
	const Iterator & iter
)
{
	Set(iter);
	return *this;
}

template <class OBJ> inline
emAvlTreeSet<OBJ>::Iterator::operator const OBJ * () const
{
	return Pos;
}

template <class OBJ> inline
const OBJ * emAvlTreeSet<OBJ>::Iterator::operator * () const
{
	return Pos;
}

template <class OBJ> inline
const OBJ * emAvlTreeSet<OBJ>::Iterator::operator -> () const
{
	return Pos;
}

template <class OBJ> inline const OBJ * emAvlTreeSet<OBJ>::Iterator::Get() const
{
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::Set(
	const Iterator & iter
)
{
	SetSet(iter.Ats);
	if (Pos!=iter.Pos) {
		AvlIterValid=false;
		Pos=iter.Pos;
	}
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::Set(
	const emAvlTreeSet<OBJ> & set, const OBJ * elem
)
{
	SetSet(&set);
	if (Pos!=elem) {
		AvlIterValid=false;
		Pos=elem;
	}
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::SetFirst(
	const emAvlTreeSet<OBJ> & set
)
{
	EM_AVL_ITER_VARS(Element)

	SetSet(&set);
	EM_AVL_ITER_FIRST(Element,AvlNode,Ats->Data->AvlTree,AvlIter)
	AvlIterValid=true;
	Pos=(element ? &element->Obj : NULL);
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::SetLast(
	const emAvlTreeSet<OBJ> & set
)
{
	EM_AVL_ITER_VARS(Element)

	SetSet(&set);
	EM_AVL_ITER_LAST(Element,AvlNode,Ats->Data->AvlTree,AvlIter)
	AvlIterValid=true;
	Pos=(element ? &element->Obj : NULL);
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::SetNext()
{
	EM_AVL_ITER_VARS(Element)

	if (Pos) {
		if (!AvlIterValid) SetPos(*Pos);
		EM_AVL_ITER_NEXT(Element,AvlNode,AvlIter)
		Pos=(element ? &element->Obj : NULL);
	}
	return Pos;
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::SetPrev()
{
	EM_AVL_ITER_VARS(Element)

	if (Pos) {
		if (!AvlIterValid) SetPos(*Pos);
		EM_AVL_ITER_PREV(Element,AvlNode,AvlIter)
		Pos=(element ? &element->Obj : NULL);
	}
	return Pos;
}

template <class OBJ> inline const OBJ * emAvlTreeSet<OBJ>::Iterator::operator ++()
{
	return SetNext();
}

template <class OBJ> inline const OBJ * emAvlTreeSet<OBJ>::Iterator::operator --()
{
	return SetPrev();
}

template <class OBJ> inline const OBJ * emAvlTreeSet<OBJ>::Iterator::operator ++(int)
{
	const OBJ * res=Pos;
	SetNext();
	return res;
}

template <class OBJ> inline const OBJ * emAvlTreeSet<OBJ>::Iterator::operator --(int)
{
	const OBJ * res=Pos;
	SetPrev();
	return res;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::Iterator::operator == (
	const Iterator & iter
) const
{
	return Pos==iter.Pos;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::Iterator::operator != (
	const Iterator & iter
) const
{
	return Pos!=iter.Pos;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::Iterator::operator == (
	const OBJ * elem
) const
{
	return Pos==elem;
}

template <class OBJ> inline bool emAvlTreeSet<OBJ>::Iterator::operator != (
	const OBJ * elem
) const
{
	return Pos!=elem;
}

template <class OBJ> inline
const emAvlTreeSet<OBJ> * emAvlTreeSet<OBJ>::Iterator::GetSet() const
{
	return Ats;
}

template <class OBJ> void emAvlTreeSet<OBJ>::Iterator::Detach()
{
	Iterator * * pi;

	if (Ats) {
		for (pi=&Ats->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
		Ats=NULL;
		Pos=NULL;
	}
}

template <class OBJ> void emAvlTreeSet<OBJ>::Iterator::SetSet(
	const emAvlTreeSet<OBJ> * set
)
{
	Iterator * * pi;

	if (Ats!=set) {
		if (Ats) {
			for (pi=&Ats->Iterators; *pi!=this; pi=&(*pi)->NextIter);
			*pi=NextIter;
		}
		Ats=(emAvlTreeSet<OBJ>*)set;
		if (Ats) {
			NextIter=Ats->Iterators;
			Ats->Iterators=this;
		}
	}
}

template <class OBJ> const OBJ * emAvlTreeSet<OBJ>::Iterator::SetPos(
	const OBJ & obj
)
{
	EM_AVL_ITER_VARS(Element)

	EM_AVL_ITER_START_ANY_BEGIN(Element,AvlNode,Ats->Data->AvlTree,AvlIter)
		if (obj<element->Obj) EM_AVL_ITER_START_ANY_GO_LEFT(AvlIter)
		else if (obj>element->Obj) EM_AVL_ITER_START_ANY_GO_RIGHT(AvlIter)
	EM_AVL_ITER_START_ANY_END
	AvlIterValid=true;
	Pos=(element ? &element->Obj : NULL);
	return Pos;
}

template <class OBJ> void emAvlTreeSet<OBJ>::MakeWritable(
	const OBJ * * preserve
)
{
	SharedData * d1, * d2;

	d1=Data;
	if (d1->RefCount>1 || Data->IsStaticEmpty) {
		d2=new SharedData;
		d2->AvlTree=NULL;
		d2->IsStaticEmpty=false;
		d2->RefCount=1;
		d1->RefCount--;
		Data=d2;
		if (d1->AvlTree) {
			d2->AvlTree=CloneTree(d1->AvlTree,preserve);
		}
	}
}

template <class OBJ> void emAvlTreeSet<OBJ>::DeleteData()
{
	EmptyData.RefCount=UINT_MAX/2;

	// Never do a
	//  if (Data!=&EmptyData)...
	// instead of
	//  if (!Data->IsStaticEmpty)...
	// because static member variables of template classes could exist
	// multiple times for the same final type (e.g. with Windows DLLs).
	if (!Data->IsStaticEmpty) {
		if (Data->AvlTree) {
			DeleteTree(Data->AvlTree);
		}
		delete Data;
	}
}

template <class OBJ> emAvlNode * emAvlTreeSet<OBJ>::CloneTree(
	emAvlNode * tree, const OBJ * * preserve
)
{
	Element * e1, * e2;
	Iterator * i;

	e1=EM_AVL_ELEMENT(Element,AvlNode,tree);
	e2=new Element(e1->Obj);
	e2->AvlNode=e1->AvlNode;
	if (preserve && *preserve==&e1->Obj) *preserve=&e2->Obj;
	for (i=Iterators; i; i=i->NextIter) {
		if (i->Pos==&e1->Obj) {
			i->Pos=&e2->Obj;
			i->AvlIterValid=false;
		}
	}
	if (e1->AvlNode.Left) {
		e2->AvlNode.Left=CloneTree(e1->AvlNode.Left,preserve);
	}
	if (e1->AvlNode.Right) {
		e2->AvlNode.Right=CloneTree(e1->AvlNode.Right,preserve);
	}
	return &e2->AvlNode;
}

template <class OBJ> void emAvlTreeSet<OBJ>::DeleteTree(emAvlNode * tree)
{
	Element * element=EM_AVL_ELEMENT(Element,AvlNode,tree);
	if (element->AvlNode.Left ) DeleteTree(element->AvlNode.Left );
	if (element->AvlNode.Right) DeleteTree(element->AvlNode.Right);
	delete element;
}

template <class OBJ>
typename emAvlTreeSet<OBJ>::SharedData emAvlTreeSet<OBJ>::EmptyData=
{
	NULL,true,UINT_MAX/2
};


#endif
