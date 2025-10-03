//------------------------------------------------------------------------------
// emOwnPtrArray.h
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

#ifndef emOwnPtrArray_h
#define emOwnPtrArray_h

#ifndef emArray_h
#include <emCore/emArray.h>
#endif


//==============================================================================
//=============================== emOwnPtrArray ================================
//==============================================================================

template <class OBJ> class emOwnPtrArray {

public:

	// Template class for a dynamic array of smart pointers which own the
	// referenced objects. The objects are deleted by calling the normal
	// delete operator. NULL pointers are allowed.

	emOwnPtrArray();
	~emOwnPtrArray();

	int GetCount() const;
		// Get number of elements.

	void SetCount(int count, bool compact=false);
		// Set the number of elements in this array. Additional elements
		// are set to NULL.
		// Arguments:
		//   count   - The new number of elements in the array.
		//   compact - Whether to make the capacity equal to the count.

	void Compact();
		// Make the capacity equal to the count.

	const OBJ * const * Get() const;
	OBJ * * Get();
		// Get a pointer to the pointer to the first element in this
		// array, that is, get the array as a normal C array.

	const OBJ * Get(int index) const;
	OBJ * Get(int index);
	const OBJ * operator [] (int index) const;
	OBJ * operator [] (int index);
		// Get a pointer to an element. The index must be within the
		// range of 0 to GetCount()-1.

	void Set(int index, OBJ * obj);
		// Assign an object pointer at the given index and take
		// ownership of the pointed object. Assigning NULL is allowed.
		// The previously owned object is deleted.

	void Reset(int index);
		// Same as Set(index,NULL).

	OBJ * Release(int index);
		// Release ownership of the object at the given index, set NULL
		// there, and return the object pointer.

	void Add(OBJ * obj, bool compact=false);
	emOwnPtrArray & operator += (OBJ * obj);
	void Insert(int index, OBJ * obj, bool compact=false);
		// Take ownership of the given object and add it to the end of
		// the array, or insert it at the given index. NULL is allowed.

	void Remove(int index, int remCount=1, bool compact=false);
		// Remove objects at a particular position.

	void Clear(bool compact=false);
		// Remove all objects.
		// Arguments:
		//   compact - Whether to minimize the internal array capacity.

	bool IsEmpty() const;
		// Ask whether the number of objects is zero.

	bool Sort(
		int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
		void * context=NULL
	);
		// Sort this array. The order of equal elements is preserved.
		// Arguments:
		//   compare - Function for comparing two elements.
		//             If you want the elements to be compared via the
		//             operators '<' and '>', say:
		//               emStdComparer<OBJ>::Compare
		//             with OBJ replaced by the real type of the
		//             elements. The context argument is ignored then.
		//             Arguments:
		//               obj1    - Pointer to first element.
		//               obj2    - Pointer to second element.
		//               context - See below.
		//             Returns: Zero if the elements are equal, a value
		//               greater than zero if the first element is
		//               greater than the second one, and a value less
		//               than zero if the first element is less than the
		//               second one.
		//   context - Any pointer to be forwarded to the compare
		//             function.
		// Returns: Whether there was a change.

	int BinarySearchByKey(
		void * key,
		int(*compareObjKey)(const OBJ * obj, void * key, void * context),
		void * context=NULL
	) const;
		// Binary search for an element that matches the given key. The
		// array must already be sorted accordingly.
		// Arguments:
		//   key           - The key to search for.
		//   compareObjKey - Function for comparing an object with the
		//                   key.
		//   context       - Any pointer to be forwarded to the compare
		//                   function.
		// Returns:
		//   If a matching element could be found, the index of that
		//   element is returned. Otherwise a value less than zero is
		//   returned: the binary inversion of the index for insertion.

	void BinaryInsert(
		OBJ * obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL,
		bool compact=false
	);
		// Insert an element by sorting it into the array, even if there
		// is already an element which equals the given object. The
		// array must already be sorted by the same compare function.
		// Arguments:
		//   obj     - An object to be copied for the insertion.
		//   compare - Please see the Sort method.
		//   context - Please see the Sort method.
		//   compact - Whether to minimize the capacity.

	bool BinaryRemoveByKey(
		void * key,
		int(*compareObjKey)(const OBJ * obj, void * key, void * context),
		void * context=NULL,
		bool compact=false
	);
		// Like BinarySearchByKey, but remove the found element, or
		// return false if no such element can been found.

private:

	struct WrappedCmpContext {
		int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context);
		void * context;
	};

	struct WrappedCmpObjKeyContext {
		int(*compareObjKey)(const OBJ * obj, void * key, void * context);
		void * context;
	};

	void AdaptCapacity(int newCount, bool compact=false);

	emOwnPtrArray(const emOwnPtrArray&);
	emOwnPtrArray & operator = (const emOwnPtrArray &);
		// Copying is not allowed.

	static int WrappedCompare(
		const OBJ * const * obj1, const OBJ * const * obj2, void * context
	);

	static int WrappedCompareObjKey(
		const OBJ * const * obj, void * key, void * context
	);

	int Count, Capacity;
	OBJ * * Array;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class OBJ>
inline emOwnPtrArray<OBJ>::emOwnPtrArray()
	: Count(0),
	Capacity(0),
	Array(NULL)
{
}

template <class OBJ>
inline emOwnPtrArray<OBJ>::~emOwnPtrArray()
{
	SetCount(0,true);
}

template <class OBJ>
inline int emOwnPtrArray<OBJ>::GetCount() const
{
	return Count;
}

template <class OBJ>
void emOwnPtrArray<OBJ>::SetCount(int count, bool compact)
{
	if (Count<count) {
		AdaptCapacity(count, compact);
		memset(Array+Count,0,sizeof(OBJ*)*(count-Count));
		Count=count;
	}
	else {
		if (count<0) count=0;
		while (Count>count) {
			Count--;
			if (Array[Count]) delete Array[Count];
		}
		AdaptCapacity(Count, compact);
	}
}

template <class OBJ>
inline void emOwnPtrArray<OBJ>::Compact()
{
	AdaptCapacity(Count,true);
}

template <class OBJ>
inline const OBJ * const * emOwnPtrArray<OBJ>::Get() const
{
	return Array;
}

template <class OBJ>
inline OBJ * * emOwnPtrArray<OBJ>::Get()
{
	return Array;
}

template <class OBJ>
inline const OBJ * emOwnPtrArray<OBJ>::Get(int index) const
{
	return Array[index];
}

template <class OBJ>
inline OBJ * emOwnPtrArray<OBJ>::Get(int index)
{
	return Array[index];
}

template <class OBJ>
inline const OBJ * emOwnPtrArray<OBJ>::operator [](int index) const
{
	return Array[index];
}

template <class OBJ>
inline OBJ * emOwnPtrArray<OBJ>::operator [](int index)
{
	return Array[index];
}

template <class OBJ>
inline void emOwnPtrArray<OBJ>::Set(int index, OBJ * obj)
{
	if (Array[index]) delete Array[index];
	Array[index] = obj;
}

template <class OBJ>
inline void emOwnPtrArray<OBJ>::Reset(int index)
{
	Set(index,NULL);
}

template <class OBJ>
inline OBJ *  emOwnPtrArray<OBJ>::Release(int index)
{
	OBJ * e;

	e=Array[index];
	Array[index]=NULL;
	return e;
}

template <class OBJ>
inline void emOwnPtrArray<OBJ>::Add(OBJ * obj, bool compact)
{
	Insert(Count,obj,compact);
}

template <class OBJ>
inline emOwnPtrArray<OBJ> & emOwnPtrArray<OBJ>::operator += (OBJ * obj)
{
	Add(obj);
	return *this;
}

template <class OBJ>
void emOwnPtrArray<OBJ>::Insert(int index, OBJ * obj, bool compact)
{
	int n;

	if (index<0) index=0;
	if (index>Count) index=Count;
	Count++;
	AdaptCapacity(Count,compact);
	n=Count-index-1;
	if (n>0) memmove(Array+index+1,Array+index,sizeof(OBJ*)*n);
	Array[index]=obj;
}

template <class OBJ>
void emOwnPtrArray<OBJ>::Remove(int index, int remCount, bool compact)
{
	int i,n;

	if (index<0) { remCount+=index; index=0; }
	if (remCount>Count-index) remCount=Count-index;
	if (remCount<=0) return;
	for (i=index; i<index+remCount; i++) {
		if (Array[i]) delete Array[i];
	}
	n=Count-index-remCount;
	if (n>0) memmove(Array+index,Array+index+remCount,sizeof(OBJ*)*n);
	Count-=remCount;
	AdaptCapacity(Count,compact);
}

template <class OBJ>
inline void emOwnPtrArray<OBJ>::Clear(bool compact)
{
	SetCount(0,compact);
}

template <class OBJ>
inline bool emOwnPtrArray<OBJ>::IsEmpty() const
{
	return Count==0;
}

template <class OBJ>
bool emOwnPtrArray<OBJ>::Sort(
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
)
{
	WrappedCmpContext wrapped;

	wrapped.compare=compare;
	wrapped.context=context;
	return emSortArray((const OBJ**)Array,Count,WrappedCompare,&wrapped);
}

template <class OBJ>
int emOwnPtrArray<OBJ>::BinarySearchByKey(
	void * key,
	int(*compareObjKey)(const OBJ * obj, void * key, void * context),
	void * context
) const
{
	WrappedCmpObjKeyContext wrapped;

	wrapped.compareObjKey=compareObjKey;
	wrapped.context=context;
	return emBinarySearch((const OBJ**)Array,Count,key,WrappedCompareObjKey,&wrapped);
}

template <class OBJ> void emOwnPtrArray<OBJ>::BinaryInsert(
	OBJ * obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	WrappedCmpContext wrapped;
	int i;

	wrapped.compare=compare;
	wrapped.context=context;
	i=emBinarySearch(
		(const OBJ * *)Array,Count,
		(const OBJ * const *)&obj,
		WrappedCompare,&wrapped
	);
	if (i<0) i=~i;
	Insert(i,obj,compact);
}

template <class OBJ>
bool emOwnPtrArray<OBJ>::BinaryRemoveByKey(
	void * key,
	int(*compareObjKey)(const OBJ * obj, void * key, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearchByKey(key,compareObjKey,context);
	if (i>=0) {
		Remove(i,1,compact);
		return true;
	}
	else {
		if (compact) Compact();
		return false;
	}
}

template <class OBJ>
void emOwnPtrArray<OBJ>::AdaptCapacity(int newCount, bool compact)
{
	int capacity;

	if (compact) capacity=newCount;
	else if (Capacity<newCount || Capacity>=newCount*3) capacity=newCount*2;
	else return;

	if (Capacity==capacity) return;

	if (capacity==0) {
		free(Array);
		Array=NULL;
	}
	else {
		Array=(OBJ**)realloc((void*)Array,sizeof(OBJ*)*capacity);
	}
	Capacity=capacity;
}

template <class OBJ>
inline emOwnPtrArray<OBJ>::emOwnPtrArray(const emOwnPtrArray&)
{
}

template <class OBJ>
inline emOwnPtrArray<OBJ> & emOwnPtrArray<OBJ>::operator =
	(const emOwnPtrArray&)
{
	return *this;
}

template <class OBJ>
int emOwnPtrArray<OBJ>::WrappedCompare(
	const OBJ * const * obj1, const OBJ * const * obj2, void * context
)
{
	const WrappedCmpContext * wrapped;

	wrapped=(WrappedCmpContext*)context;
	return wrapped->compare(*obj1,*obj2,wrapped->context);
}

template <class OBJ>
int emOwnPtrArray<OBJ>::WrappedCompareObjKey(
	const OBJ * const * obj, void * key, void * context
)
{
	const WrappedCmpObjKeyContext * wrapped;

	wrapped=(WrappedCmpObjKeyContext*)context;
	return wrapped->compareObjKey(*obj,key,wrapped->context);
}


#endif
