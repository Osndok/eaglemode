//------------------------------------------------------------------------------
// emList.h
//
// Copyright (C) 2005-2010 Oliver Hamann.
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

#ifndef emList_h
#define emList_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//=========================== Linked-list functions ============================
//==============================================================================

bool emSortSingleLinkedList(
	void * * pFirst, int nextOffset,
	int(*compare)(void * ptr1, void * ptr2, void * context),
	void * context=NULL
);
	// Sort a single-linked NULL-terminated list. The order of equal
	// elements is preserved. It is a merge-sort algorithm.
	// Arguments:
	//   pFirst     - Pointer to pointer to first element. On return,
	//                *pFirst will be set to the new first element.
	//   nextOffset - Offset where the pointer to the next element is stored
	//                within an element: If e points to an element, then
	//                *((void**)(((char*)e)+nextOffset)) points to the next
	//                element.
	//   compare    - Function for comparing two elements. If you want the
	//                elements to be compared via the operators '<' and '>',
	//                say:
	//                  emStdComparer<OBJ>::Compare
	//                with OBJ replaced by the real type of the elements.
	//                The context argument is ignored then.
	//                Arguments:
	//                  ptr1    - Pointer to first element.
	//                  ptr2    - Pointer to second element.
	//                  context - See below.
	//                Returns: Zero if the elements are equal, a value
	//                  greater than zero if the first element is greater
	//                  than the second one, and a value less than zero if
	//                  the first element is less than the second one.
	//   context    - Any pointer to be forwarded to the compare function.
	// Returns: true if the order has changed, false otherwise.

bool emSortDoubleLinkedList(
	void * * pFirst, void * * pLast, int nextOffset, int prevOffset,
	int(*compare)(void * ptr1, void * ptr2, void * context),
	void * context=NULL
);
	// Sort a double-linked NULL-terminated list. The order of equal
	// elements is preserved. It is a merge-sort algorithm.
	// Arguments:
	//   pFirst     - Pointer to pointer to first element. On return,
	//                *pFirst will be set to the new first element.
	//   pLast      - Pointer to pointer to last element. On return, *pLast
	//                will be set to the new last element.
	//   nextOffset - Offset where the pointer to the next element is stored
	//                within an element: If e points to an element, then
	//                *((void**)(((char*)e)+nextOffset)) points to the next
	//                element.
	//   prevOffset - Offset where the pointer to the previous element is
	//                stored within an element: If e points to an element,
	//                then *((void**)(((char*)e)+prevOffset)) points to
	//                the previous element.
	//   compare    - Function for comparing two elements. If you want the
	//                elements to be compared via the operators '<' and '>',
	//                say:
	//                  emStdComparer<OBJ>::Compare
	//                with OBJ replaced by the real type of the elements.
	//                The context argument is ignored then.
	//                Arguments:
	//                  ptr1    - Pointer to first element.
	//                  ptr2    - Pointer to second element.
	//                  context - See below.
	//                Returns: Zero if the elements are equal, a value
	//                  greater than zero if the first element is greater
	//                  than the second one, and a value less than zero if
	//                  the first element is less than the second one.
	//   context    - Any pointer to be forwarded to the compare function.
	// Returns: true if the order has changed, false otherwise.


//==============================================================================
//=================================== emList ===================================
//==============================================================================

template <class OBJ> class emList {

public:

	// Template class for a double-linked NULL-terminated list with
	// copy-on-write behavior and with support for stable iterators. The
	// template parameter OBJ describes the type of the elements.
	// Internally, emList extends the memory allocation for that type by the
	// next and prev pointers.
	//
	// There are two types of iterators. The very unstable one:
	// const OBJ * (please read the comments on GetFirst()), and the
	// stable one: the class Iterator, which can be found at the end of this
	// public declaration.
	//
	// If you wonder why a NULL in range arguments (first,last) of methods
	// means to cancel the operation, instead of taking the first/last
	// element of the whole list. It is because now one can say for example
	// l.Remove(l.GetNext(e),l.GetLast()) to remove all elements after the
	// element e, even when e is the last element.
	//
	// Here is a crazy example of printing "hello world\n":
	//
	// emList<emString> l;
	// emList<emString>::Iterator i;
	// const emString * j;
	// l="word";
	// l+="helo";
	// for (i.SetFirst(l); i; ++i) l.GetWritable(i)->Insert(3,'l');
	// l.Sort(emStdComparer<emString>::Compare);
	// for (i.SetLast(l); i; --i) l.InsertAfter(i," ");
	// *l.GetLastWritable()="\n";
	// // The following loop does not modify the list, so it is safe
	// // to do it with the unstable iterator j.
	// for (j=l.GetFirst(); j; j=l.GetNext(j)) printf("%s",j->Get());

	emList();
		// Construct an empty list.

	emList(const emList & src);
		// Construct a copied list.

	emList(const OBJ & obj);
		// Construct a list with one element copied from the given
		// object.

	emList(const emList & src1, const emList & src2);
	emList(const emList & src1, const OBJ & src2);
	emList(const OBJ & src1, const emList & src2);
	emList(const emList & src, const OBJ * first, const OBJ * last);
		// These constructors are designed mainly for internal use.

	~emList();
		// Destructor.

	emList & operator = (const emList & list);
	emList & operator = (const OBJ & obj);
		// Make this list a copy of the given list or object.

	const OBJ * GetFirst() const;
	const OBJ * GetLast() const;
	const OBJ * GetNext(const OBJ * elem) const;
	const OBJ * GetPrev(const OBJ * elem) const;
		// Get a pointer to the first or last element of this list, or
		// get a pointer to the next or previous element of an element
		// of this list. At least because of the copy-on-write feature,
		// the pointer is valid only until calling any non-const method
		// or operator on this list, or giving this list as a non-const
		// argument to any call in the world. If you need more stable
		// pointers, please refer to the class Iterator more below.
		// Hint: even methods like Add, Insert and GetSubList may make
		// shallow copies, like the copy operator and copy constructor
		// do.
		// Arguments:
		//   elem - A pointer to an element in this list, must never be
		//          NULL.
		// Returns:
		//   A pointer to the requested element in this list, or NULL if
		//   there is no such element.

	OBJ * GetWritable(const OBJ * elem);
		// Get a non-const version of a pointer to an element of this
		// list. The pointer may be used for modifying the element (but
		// not for deleting). The rules for validity of the pointer are
		// the same as with the GetFirst() method, but: The pointer must
		// not be used for modifying after doing something which could
		// have made a shallow copy of this list.
		// Arguments:
		//   elem - A pointer to an element of this list, or NULL.
		// Returns: Pointer for modifying, or NULL if elem is NULL.

	OBJ * GetFirstWritable();
	OBJ * GetLastWritable();
	OBJ * GetNextWritable(const OBJ * elem);
	OBJ * GetPrevWritable(const OBJ * elem);
		// Like GetWritable(GetFirst()) and so on.

	void Set(const OBJ * pos, const OBJ & obj);
		// Replace an element.
		// Arguments:
		//   pos - A pointer to an element of this list.
		//   obj - An object to be copied to the element.

	void InsertAtBeg(const OBJ & obj);
	void InsertAtBeg(const OBJ * elem);
	void InsertAtBeg(const OBJ * first, const OBJ * last);
	void InsertAtBeg(const emList & src);
	void InsertAtBeg(const emList & src, const OBJ * elem);
	void InsertAtBeg(const emList & src, const OBJ * first,
	                 const OBJ * last);
	void InsertAtEnd(const OBJ & obj);
	void InsertAtEnd(const OBJ * elem);
	void InsertAtEnd(const OBJ * first, const OBJ * last);
	void InsertAtEnd(const emList & src);
	void InsertAtEnd(const emList & src, const OBJ * elem);
	void InsertAtEnd(const emList & src, const OBJ * first,
	                 const OBJ * last);
	void InsertBefore(const OBJ * pos, const OBJ & obj);
	void InsertBefore(const OBJ * pos, const OBJ * elem);
	void InsertBefore(const OBJ * pos, const OBJ * first, const OBJ * last);
	void InsertBefore(const OBJ * pos, const emList & src);
	void InsertBefore(const OBJ * pos, const emList & src,
	                  const OBJ * elem);
	void InsertBefore(const OBJ * pos, const emList & src,
	                  const OBJ * first, const OBJ * last);
	void InsertAfter(const OBJ * pos, const OBJ & obj);
	void InsertAfter(const OBJ * pos, const OBJ * elem);
	void InsertAfter(const OBJ * pos, const OBJ * first, const OBJ * last);
	void InsertAfter(const OBJ * pos, const emList & src);
	void InsertAfter(const OBJ * pos, const emList & src, const OBJ * elem);
	void InsertAfter(const OBJ * pos, const emList & src, const OBJ * first,
	                 const OBJ * last);
		// Insert elements at the beginning or end of this list, or
		// before or after an element of this list. It is even allowed
		// to insert a list into itself.
		// Arguments:
		//   pos   - An element in this list before which or after which
		//           the insertion has to take place. NULL is allowed
		//           here. InsertBefore(NULL,...) means to insert at the
		//           end, and InsertAfter(NULL,...) means to insert at
		//           the beginning.
		//   obj   - An object to be copied for inserting a single
		//           element.
		//   src   - A source list containing the element(s) to be
		//           copied for the insertion. If this argument is not
		//           given, this list itself is the source list.
		//   elem  - An element of the source list. The element is
		//           copied for inserting a single element. If NULL,
		//           nothing is inserted.
		//   first - An element of the source list. It is the first one
		//           of a range of elements to be copied for the
		//           insertion. If NULL, nothing is inserted.
		//   last  - An element of the source list, not before 'first'.
		//           It is the last one of the range of elements to be
		//           copied for the insertion. If NULL, nothing is
		//           inserted.

	void Add(const OBJ & obj);
	void Add(const OBJ * elem);
	void Add(const OBJ * first, const OBJ * last);
	void Add(const emList & src);
	void Add(const emList & src, const OBJ * elem);
	void Add(const emList & src, const OBJ * first, const OBJ * last);
		// Just another name for InsertAtEnd.

	emList & operator += (const emList & list);
	emList & operator += (const OBJ & obj);
	emList operator + (const emList & list) const;
	emList operator + (const OBJ & obj) const;
		// Similar to the Add methods...

	//friend emList operator + (const OBJ & obj, const emList & list);
		// This one even exists and can be used.
		// (Having the declaration here would not be portable)

	void MoveToBeg(const OBJ * elem);
	void MoveToBeg(const OBJ * first, const OBJ * last);
	void MoveToBeg(emList * src);
	void MoveToBeg(emList * src, const OBJ * elem);
	void MoveToBeg(emList * src, const OBJ * first, const OBJ * last);
	void MoveToEnd(const OBJ * elem);
	void MoveToEnd(const OBJ * first, const OBJ * last);
	void MoveToEnd(emList * src);
	void MoveToEnd(emList * src, const OBJ * elem);
	void MoveToEnd(emList * src, const OBJ * first, const OBJ * last);
	void MoveBefore(const OBJ * pos, const OBJ * elem);
	void MoveBefore(const OBJ * pos, const OBJ * first, const OBJ * last);
	void MoveBefore(const OBJ * pos, emList * src);
	void MoveBefore(const OBJ * pos, emList * src, const OBJ * elem);
	void MoveBefore(const OBJ * pos, emList * src, const OBJ * first,
	                const OBJ * last);
	void MoveAfter(const OBJ * pos, const OBJ * elem);
	void MoveAfter(const OBJ * pos, const OBJ * first, const OBJ * last);
	void MoveAfter(const OBJ * pos, emList * src);
	void MoveAfter(const OBJ * pos, emList * src, const OBJ * elem);
	void MoveAfter(const OBJ * pos, emList * src, const OBJ * first,
	               const OBJ * last);
		// Move elements from a source list to the beginning or end of
		// this list, or before or after an element of this list.
		// Arguments:
		//   pos   - An element in this list before which or after which
		//           the elements are to be moved. It must not be a
		//           member of the moved elements! NULL is allowed here.
		//           MoveBefore(NULL,...) means to move to the end, and
		//           MoveAfter(NULL,...) means to move to the beginning.
		//   src   - Pointer to the source list. If NULL or not given,
		//           this list itself is the source list.
		//   elem  - An element of the source list, which shall be
		//           moved. If NULL, nothing is moved.
		//   first - An element of the source list. It is the first one
		//           of a range of elements to be moved. If NULL,
		//           nothing is moved.
		//   last  - An element of the source list, not before 'first'.
		//           It is the last one of the range of elements to be
		//           moved. If NULL, nothing is moved.

	emList GetSubListOfFirst() const;
	emList GetSubListOfLast() const;
	emList GetSubList(const OBJ * elem) const;
	emList GetSubList(const OBJ * first, const OBJ * last) const;
		// Like the Extract methods (see below), but the elements are
		// copied instead of removing them from this list.

	emList ExtractFirst();
	emList ExtractLast();
	emList Extract(const OBJ * elem);
	emList Extract(const OBJ * first, const OBJ * last);
		// Like the Remove methods (see below), but return a list of the
		// removed elements, instead of deleting them.

	void RemoveFirst();
	void RemoveLast();
	void Remove(const OBJ * elem);
	void Remove(const OBJ * first, const OBJ * last);
		// Remove (and delete) the first element, the last element, a
		// given element or a range of elements from this list.
		// Arguments:
		//   elem  - An element of this list, which shall be removed.
		//           If NULL, nothing is removed.
		//   first - An element of this list. It is the first one of a
		//           range of elements to be removed. If NULL, nothing
		//           is removed.
		//   last  - An element of this list, not before 'first'. It is
		//           the last one of the range of elements to be
		//           removed. If NULL, nothing is removed.

	void Empty(bool compact=false);
		// Remove (and delete) all elements of this list.
		// Arguments:
		//   compact - true if you plan to keep this list empty for
		//             a long time. Otherwise a small block of memory
		//             may possibly not be freed for quick re-use.

	bool IsEmpty() const;
		// Ask whether this list has no elements.

	bool Sort(
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL
	);
		// Sort this list. The order of equal elements is preserved.
		// Arguments:
		//   compare - Function for comparing two elements. If you want
		//             the elements to be compared via the operators '<'
		//             and '>', say:
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

	int GetCount() const;
		// Compute the number of elements.

	const OBJ * GetAtIndex(int index) const;
		// Search the element at the given index. Returns NULL if the
		// index is out of range. The rules for the validity of the
		// pointer are the same as with the GetFirst() method.

	int GetIndexOf(const OBJ * elem) const;
		// Search the given element and return its index. Returns -1
		// if it is not an element of this list.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this list.

	void MakeNonShared();
		// This must be called before handing the list to another
		// thread. This method is not recursive. So if the object class
		// even has such a method, you have to call it on every object
		// too.

	class Iterator {

	public:

		// Class for a stable pointer to an element of a list.
		// "stable" means:
		// * If the address of an element changes through the
		//   copy-on-write mechanism, iterators pointing to that element
		//   are adapted proper.
		// * If an element is moved to another list, iterators pointing
		//   to that element keep pointing to that element (and the list
		//   pointers of the iterators are adapted).
		// * If an element is removed or extracted from a list,
		//   iterators pointing to that element are set to the next
		//   element, or NULL if it was the last element.
		// * If the assignment operator '=' is called on a list, all
		//   iterators which were pointing to elements of the list are
		//   set to NULL. This is even true if the list is assigned to
		//   itself.
		// Note the auto-cast operator to a 'const OBJ *'. Wherever
		// there is an argument 'const OBJ *' in the methods of emList,
		// you can even give an instance of this class as the argument.

		Iterator();
			// Construct a "NULL pointer".

		Iterator(const Iterator & iter);
			// Construct a copied iterator.

		Iterator(const emList<OBJ> & list, const OBJ * elem);
			// Construct an iterator pointing to a particular
			// element.
			// Arguments:
			//   list - The list.
			//   elem - Pointer to an element of the list, or NULL.

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

		const OBJ * Set(const emList<OBJ> & list, const OBJ * elem);
			// Set this iterator to the given element of the given
			// list and return the element pointer.

		const OBJ * SetFirst(const emList<OBJ> & list);
		const OBJ * SetLast(const emList<OBJ> & list);
			// Set this iterator to the first or last element of the
			// given list and return the element pointer.

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

		const emList<OBJ> * GetList() const;
			// Get a pointer to the list this iterator is currently
			// attached to. Returns NULL if not attached to any
			// list. (See comments on Detach()).

		void Detach();
			// Detach this iterator from its list and point to NULL.
			// Note: to care about the iterators, each emList has a
			// single linked list of its iterators. The mechanism is
			// lazy, that means, an iterator may stay in the list
			// even when not pointing to any element, just for quick
			// re-use. On the other hand, such iterators are still
			// costing a tiny number of CPU cycles whenever the list
			// of elements is modified.

	private:
		friend class emList<OBJ>;
		const OBJ * Pos;
		emList<OBJ> * List;
		Iterator * NextIter; // Undefined if List==NULL
	};

private:
	friend class Iterator;

	struct Element {
		OBJ Obj;
		OBJ * Next;
		OBJ * Prev;
		inline Element(const OBJ & obj) : Obj(obj) {}
	};
	struct SharedData {
		OBJ * First;
		OBJ * Last;
		bool IsStaticEmpty;
		unsigned int RefCount;
	};

	SharedData * Data;
	Iterator * Iterators;
	static SharedData EmptyData;

	inline emList(SharedData * d) { Data=d; Iterators=NULL; }
	void MakeWritable();
	void MakeWritable(const OBJ * * preserve);
	void MakeWritable(const OBJ * * preserve1, const OBJ * * preserve2);
	void MakeWritable(const OBJ * * preserve1, const OBJ * * preserve2,
	                  const OBJ * * preserve3);
	void DeleteData();
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

#define EM_LSTIMP_ELEM(objPtr) \
	((Element*)(((char*)(objPtr))-offsetof(Element,Obj)))
#define EM_LSTIMP_PREV(objPtr) EM_LSTIMP_ELEM(objPtr)->Prev
#define EM_LSTIMP_NEXT(objPtr) EM_LSTIMP_ELEM(objPtr)->Next

template <class OBJ> inline emList<OBJ>::emList()
{
	Iterators=NULL;
	Data=&EmptyData;
}

template <class OBJ> inline emList<OBJ>::emList(const emList & src)
{
	Iterators=NULL;
	Data=src.Data;
	Data->RefCount++;
}

template <class OBJ> emList<OBJ>::emList(const OBJ & obj)
{
	Iterators=NULL;
	Data=&EmptyData;
	InsertBefore(NULL,obj);
}

template <class OBJ> emList<OBJ>::emList(
	const emList & src1, const emList & src2
)
{
	Iterators=NULL;
	Data=src1.Data;
	Data->RefCount++;
	InsertBefore(NULL,src2,src2.Data->First,src2.Data->Last);
}

template <class OBJ> emList<OBJ>::emList(
	const emList & src1, const OBJ & src2
)
{
	Iterators=NULL;
	Data=src1.Data;
	Data->RefCount++;
	InsertBefore(NULL,src2);
}

template <class OBJ> emList<OBJ>::emList(
	const OBJ & src1, const emList & src2
)
{
	Iterators=NULL;
	Data=src2.Data;
	Data->RefCount++;
	InsertAfter(NULL,src1);
}

template <class OBJ> emList<OBJ>::emList(
	const emList & src, const OBJ * first, const OBJ * last
)
{
	Iterators=NULL;
	Data=&EmptyData;
	InsertBefore(NULL,src,first,last);
}

template <class OBJ> emList<OBJ>::~emList()
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) { i->Pos=NULL; i->List=NULL; }
	if (!--Data->RefCount) DeleteData();
}

template <class OBJ> emList<OBJ> & emList<OBJ>::operator = (
	const emList & list
)
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	list.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=list.Data;
	return *this;
}

template <class OBJ> emList<OBJ> & emList<OBJ>::operator = (const OBJ & obj)
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	if (Data->RefCount>1) {
		Data->RefCount--;
		Data=&EmptyData;
	}
	InsertBefore(NULL,obj);
	if (EM_LSTIMP_PREV(Data->Last)) {
		Remove(Data->First,EM_LSTIMP_PREV(Data->Last));
	}
	return *this;
}

template <class OBJ> inline const OBJ * emList<OBJ>::GetFirst() const
{
	return Data->First;
}

template <class OBJ> inline const OBJ * emList<OBJ>::GetLast() const
{
	return Data->Last;
}

template <class OBJ> inline const OBJ * emList<OBJ>::GetNext(
	const OBJ * elem
) const
{
	return EM_LSTIMP_NEXT(elem);
}

template <class OBJ> inline const OBJ * emList<OBJ>::GetPrev(
	const OBJ * elem
) const
{
	return EM_LSTIMP_PREV(elem);
}

template <class OBJ> inline OBJ * emList<OBJ>::GetWritable(const OBJ * elem)
{
	if (Data->RefCount>1) MakeWritable(&elem);
	return (OBJ*)elem;
}

template <class OBJ> inline OBJ * emList<OBJ>::GetFirstWritable()
{
	if (Data->RefCount>1) MakeWritable();
	return Data->First;
}

template <class OBJ> inline OBJ * emList<OBJ>::GetLastWritable()
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Last;
}

template <class OBJ> inline OBJ * emList<OBJ>::GetNextWritable(
	const OBJ * elem
)
{
	if (Data->RefCount>1) MakeWritable(&elem);
	return EM_LSTIMP_NEXT(elem);
}

template <class OBJ> inline OBJ * emList<OBJ>::GetPrevWritable(
	const OBJ * elem
)
{
	if (Data->RefCount>1) MakeWritable(&elem);
	return EM_LSTIMP_NEXT(elem);
}

template <class OBJ> inline void emList<OBJ>::Set(
	const OBJ * pos, const OBJ & obj
)
{
	if (Data->RefCount>1) MakeWritable(&pos);
	*((OBJ*)pos)=obj;
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(const OBJ & obj)
{
	InsertAfter(NULL,obj);
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(const OBJ * elem)
{
	InsertAfter(NULL,*this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(
	const OBJ * first, const OBJ * last
)
{
	InsertAfter(NULL,*this,first,last);
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(
	const emList & src
)
{
	InsertAfter(NULL,src,src.Data->First,src.Data->Last);
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(
	const emList & src, const OBJ * elem
)
{
	InsertAfter(NULL,src,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertAtBeg(
	const emList & src, const OBJ * first, const OBJ * last
)
{
	InsertAfter(NULL,src,first,last);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(const OBJ & obj)
{
	InsertBefore(NULL,obj);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(const OBJ * elem)
{
	InsertBefore(NULL,*this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(
	const OBJ * first, const OBJ * last
)
{
	InsertBefore(NULL,*this,first,last);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(const emList & src)
{
	InsertBefore(NULL,src,src.Data->First,src.Data->Last);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(
	const emList & src, const OBJ * elem
)
{
	InsertBefore(NULL,src,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertAtEnd(
	const emList & src, const OBJ * first, const OBJ * last
)
{
	InsertBefore(NULL,src,first,last);
}

template <class OBJ> void emList<OBJ>::InsertBefore(
	const OBJ * pos, const OBJ & obj
)
{
	OBJ * e;

	if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos);
	e=&(new Element(obj))->Obj;
	if ((EM_LSTIMP_NEXT(e)=(OBJ*)pos)==NULL) {
		if ((EM_LSTIMP_PREV(e)=Data->Last)==NULL) Data->First=e;
		else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e))=e;
		Data->Last=e;
	}
	else {
		if ((EM_LSTIMP_PREV(e)=EM_LSTIMP_PREV(pos))==NULL) Data->First=e;
		else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e))=e;
		EM_LSTIMP_PREV(pos)=e;
	}
}

template <class OBJ> inline void emList<OBJ>::InsertBefore(
	const OBJ * pos, const OBJ * elem
)
{
	InsertBefore(pos,*this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertBefore(
	const OBJ * pos, const OBJ * first, const OBJ * last
)
{
	InsertBefore(pos,*this,first,last);
}

template <class OBJ> inline void emList<OBJ>::InsertBefore(
	const OBJ * pos, const emList & src
)
{
	InsertBefore(pos,src,src.Data->First,src.Data->Last);
}

template <class OBJ> inline void emList<OBJ>::InsertBefore(
	const OBJ * pos, const emList & src, const OBJ * elem
)
{
	InsertBefore(pos,src,elem,elem);
}

template <class OBJ> void emList<OBJ>::InsertBefore(
	const OBJ * pos, const emList & src, const OBJ * first,
	const OBJ * last
)
{
	OBJ * p, * e;

	if (!first || !last) return;
	if (!Data->First && first==src.Data->First && last==src.Data->Last) {
		src.Data->RefCount++;
		if (!--Data->RefCount) DeleteData();
		Data=src.Data;
		return;
	}
	if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos,&first,&last);
	p=(OBJ*)pos;
	for (;;) {
		e=&(new Element(*last))->Obj;
		if ((EM_LSTIMP_NEXT(e)=p)==NULL) {
			if ((EM_LSTIMP_PREV(e)=Data->Last)==NULL) Data->First=e;
			else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e))=e;
			Data->Last=e;
		}
		else {
			if ((EM_LSTIMP_PREV(e)=EM_LSTIMP_PREV(p))==NULL) Data->First=e;
			else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e))=e;
			EM_LSTIMP_PREV(p)=e;
		}
		if (last==first) break;
		p=e;
		if (last==pos) last=p;
		last=EM_LSTIMP_PREV(last);
	}
}

template <class OBJ> void emList<OBJ>::InsertAfter(
	const OBJ * pos, const OBJ & obj
)
{
	OBJ * e;

	if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos);
	e=&(new Element(obj))->Obj;
	if ((EM_LSTIMP_PREV(e)=(OBJ*)pos)==NULL) {
		if ((EM_LSTIMP_NEXT(e)=Data->First)==NULL) Data->Last=e;
		else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(e))=e;
		Data->First=e;
	}
	else {
		if ((EM_LSTIMP_NEXT(e)=EM_LSTIMP_NEXT(pos))==NULL) Data->Last=e;
		else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(e))=e;
		EM_LSTIMP_NEXT(pos)=e;
	}
}

template <class OBJ> inline void emList<OBJ>::InsertAfter(
	const OBJ * pos, const OBJ * elem
)
{
	InsertAfter(pos,*this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::InsertAfter(
	const OBJ * pos, const OBJ * first, const OBJ * last
)
{
	InsertAfter(pos,*this,first,last);
}

template <class OBJ> inline void emList<OBJ>::InsertAfter(
	const OBJ * pos, const emList & src
)
{
	InsertAfter(pos,src,src.Data->First,src.Data->Last);
}

template <class OBJ> inline void emList<OBJ>::InsertAfter(
	const OBJ * pos, const emList & src, const OBJ * elem
)
{
	InsertAfter(pos,src,elem,elem);
}

template <class OBJ> void emList<OBJ>::InsertAfter(
	const OBJ * pos, const emList & src, const OBJ * first,
	const OBJ * last
)
{
	OBJ * p, * e;

	if (!first || !last) return;
	if (!Data->First && first==src.Data->First && last==src.Data->Last) {
		src.Data->RefCount++;
		if (!--Data->RefCount) DeleteData();
		Data=src.Data;
		return;
	}
	if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos,&first,&last);
	p=(OBJ*)pos;
	for (;;) {
		e=&(new Element(*first))->Obj;
		if ((EM_LSTIMP_PREV(e)=p)==NULL) {
			if ((EM_LSTIMP_NEXT(e)=Data->First)==NULL) Data->Last=e;
			else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(e))=e;
			Data->First=e;
		}
		else {
			if ((EM_LSTIMP_NEXT(e)=EM_LSTIMP_NEXT(p))==NULL) Data->Last=e;
			else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(e))=e;
			EM_LSTIMP_NEXT(p)=e;
		}
		if (first==last) break;
		p=e;
		if (first==pos) first=p;
		first=EM_LSTIMP_NEXT(first);
	}
}

template <class OBJ> inline void emList<OBJ>::Add(const OBJ & obj)
{
	InsertBefore(NULL,obj);
}

template <class OBJ> inline void emList<OBJ>::Add(const OBJ * elem)
{
	InsertBefore(NULL,*this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::Add(
	const OBJ * first, const OBJ * last
)
{
	InsertBefore(NULL,*this,first,last);
}

template <class OBJ> inline void emList<OBJ>::Add(const emList & src)
{
	InsertBefore(NULL,src,src.Data->First,src.Data->Last);
}

template <class OBJ> inline void emList<OBJ>::Add(
	const emList & src, const OBJ * elem
)
{
	InsertBefore(NULL,src,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::Add(
	const emList & src, const OBJ * first, const OBJ * last
)
{
	InsertBefore(NULL,src,first,last);
}

template <class OBJ> inline emList<OBJ> & emList<OBJ>::operator += (
	const emList & list
)
{
	InsertBefore(NULL,list,list.Data->First,list.Data->Last);
	return *this;
}

template <class OBJ> inline emList<OBJ> & emList<OBJ>::operator += (
	const OBJ & obj
)
{
	InsertBefore(NULL,obj);
	return *this;
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::operator + (
	const emList & list
) const
{
	return emList<OBJ>(*this,list);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::operator + (
	const OBJ & obj
) const
{
	return emList<OBJ>(*this,obj);
}

template <class OBJ> inline emList<OBJ> operator + (
	const OBJ & obj, const emList<OBJ> & list
)
{
	return emList<OBJ>(obj,list);
}

template <class OBJ> inline void emList<OBJ>::MoveToBeg(const OBJ * elem)
{
	MoveAfter(NULL,this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveToBeg(
	const OBJ * first, const OBJ * last
)
{
	MoveAfter(NULL,this,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveToBeg(emList * src)
{
	if (src) MoveAfter(NULL,src,src->Data->First,src->Data->Last);
}

template <class OBJ> inline void emList<OBJ>::MoveToBeg(
	emList * src, const OBJ * elem
)
{
	MoveAfter(NULL,src,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveToBeg(
	emList * src, const OBJ * first, const OBJ * last
)
{
	MoveAfter(NULL,src,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveToEnd(const OBJ * elem)
{
	MoveBefore(NULL,this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveToEnd(
	const OBJ * first, const OBJ * last
)
{
	MoveBefore(NULL,this,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveToEnd(emList * src)
{
	if (src) MoveBefore(NULL,src,src->Data->First,src->Data->Last);
}

template <class OBJ> inline void emList<OBJ>::MoveToEnd(
	emList * src, const OBJ * elem
)
{
	MoveBefore(NULL,src,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveToEnd(
	emList * src, const OBJ * first, const OBJ * last
)
{
	MoveBefore(NULL,src,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveBefore(
	const OBJ * pos, const OBJ * elem
)
{
	MoveBefore(pos,this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveBefore(
	const OBJ * pos, const OBJ * first, const OBJ * last
)
{
	MoveBefore(pos,this,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveBefore(
	const OBJ * pos, emList * src
)
{
	if (src) MoveBefore(pos,src,src->Data->First,src->Data->Last);
}

template <class OBJ> inline void emList<OBJ>::MoveBefore(
	const OBJ * pos, emList * src, const OBJ * elem
)
{
	MoveBefore(pos,src,elem,elem);
}

template <class OBJ> void emList<OBJ>::MoveBefore(
	const OBJ * pos, emList * src, const OBJ * first, const OBJ * last
)
{
	Iterator * * pi;
	Iterator * i;
	const OBJ * e;

	if (!first || !last) return;
	if (!src) src=this;
	if (src!=this) {
		if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos);
		if (src->Data->RefCount>1) src->MakeWritable(&first,&last);
		for (pi=&src->Iterators, i=*pi; i; i=*pi) {
			if (i->Pos!=last) {
				if (!i->Pos) { pi=&i->NextIter; continue; }
				for (e=first; i->Pos!=e && e!=last; e=EM_LSTIMP_NEXT(e));
				if (e==last) { pi=&i->NextIter; continue; }
			}
			*pi=i->NextIter;
			i->List=this;
			i->NextIter=Iterators;
			Iterators=i;
		}
	}
	else if (Data->RefCount>1) {
		if (EM_LSTIMP_NEXT(last)==pos) return;
		MakeWritable(&pos,&first,&last);
	}
	if (!EM_LSTIMP_PREV(first)) src->Data->First=EM_LSTIMP_NEXT(last);
	else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=EM_LSTIMP_NEXT(last);
	if (!EM_LSTIMP_NEXT(last)) src->Data->Last=EM_LSTIMP_PREV(first);
	else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=EM_LSTIMP_PREV(first);
	if ((EM_LSTIMP_NEXT(last)=(OBJ*)pos)==NULL) {
		if ((EM_LSTIMP_PREV(first)=Data->Last)==NULL) Data->First=(OBJ*)first;
		else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=(OBJ*)first;
		Data->Last=(OBJ*)last;
	}
	else {
		if ((EM_LSTIMP_PREV(first)=EM_LSTIMP_PREV(pos))==NULL) {
			Data->First=(OBJ*)first;
		}
		else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=(OBJ*)first;
		EM_LSTIMP_PREV(pos)=(OBJ*)last;
	}
}

template <class OBJ> inline void emList<OBJ>::MoveAfter(
	const OBJ * pos, const OBJ * elem
)
{
	MoveAfter(pos,this,elem,elem);
}

template <class OBJ> inline void emList<OBJ>::MoveAfter(
	const OBJ * pos, const OBJ * first, const OBJ * last
)
{
	MoveAfter(pos,this,first,last);
}

template <class OBJ> inline void emList<OBJ>::MoveAfter(
	const OBJ * pos, emList * src
)
{
	if (src) MoveAfter(pos,src,src->Data->First,src->Data->Last);
}

template <class OBJ> inline void emList<OBJ>::MoveAfter(
	const OBJ * pos, emList * src, const OBJ * elem
)
{
	MoveAfter(pos,src,elem,elem);
}

template <class OBJ> void emList<OBJ>::MoveAfter(
	const OBJ * pos, emList * src, const OBJ * first, const OBJ * last
)
{
	Iterator * * pi;
	Iterator * i;
	const OBJ * e;

	if (!first || !last) return;
	if (!src) src=this;
	if (src!=this) {
		if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable(&pos);
		if (src->Data->RefCount>1) src->MakeWritable(&first,&last);
		for (pi=&src->Iterators, i=*pi; i; i=*pi) {
			if (i->Pos!=last) {
				if (!i->Pos) { pi=&i->NextIter; continue; }
				for (e=first; i->Pos!=e && e!=last; e=EM_LSTIMP_NEXT(e));
				if (e==last) { pi=&i->NextIter; continue; }
			}
			*pi=i->NextIter;
			i->List=this;
			i->NextIter=Iterators;
			Iterators=i;
		}
	}
	else if (Data->RefCount>1) {
		if (EM_LSTIMP_PREV(first)==pos) return;
		MakeWritable(&pos,&first,&last);
	}
	if (!EM_LSTIMP_PREV(first)) src->Data->First=EM_LSTIMP_NEXT(last);
	else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=EM_LSTIMP_NEXT(last);
	if (!EM_LSTIMP_NEXT(last)) src->Data->Last=EM_LSTIMP_PREV(first);
	else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=EM_LSTIMP_PREV(first);
	if ((EM_LSTIMP_PREV(first)=(OBJ*)pos)==NULL) {
		if ((EM_LSTIMP_NEXT(last)=Data->First)==NULL) Data->Last=(OBJ*)last;
		else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=(OBJ*)last;
		Data->First=(OBJ*)first;
	}
	else {
		if ((EM_LSTIMP_NEXT(last)=EM_LSTIMP_NEXT(pos))==NULL) {
			Data->Last=(OBJ*)last;
		}
		else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=(OBJ*)last;
		EM_LSTIMP_NEXT(pos)=(OBJ*)first;
	}
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::GetSubListOfFirst() const
{
	return emList<OBJ>(*this,Data->First,Data->First);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::GetSubListOfLast() const
{
	return emList<OBJ>(*this,Data->Last,Data->Last);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::GetSubList(
	const OBJ * elem
) const
{
	return emList<OBJ>(*this,elem,elem);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::GetSubList(
	const OBJ * first, const OBJ * last
) const
{
	return emList<OBJ>(*this,first,last);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::ExtractFirst()
{
	return Extract(Data->First,Data->First);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::ExtractLast()
{
	return Extract(Data->Last,Data->Last);
}

template <class OBJ> inline emList<OBJ> emList<OBJ>::Extract(const OBJ * elem)
{
	return Extract(elem,elem);
}

template <class OBJ> emList<OBJ> emList<OBJ>::Extract(
	const OBJ * first, const OBJ * last
)
{
	SharedData * d;
	const OBJ * e;
	Iterator * i;

	if (!first || !last) {
		d=&EmptyData;
	}
	else if (first==Data->First && last==Data->Last) {
		for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
		d=Data;
		Data=&EmptyData;
	}
	else {
		if (Data->RefCount>1) MakeWritable(&first,&last);
		for (i=Iterators; i; i=i->NextIter) {
			if (i->Pos!=last) {
				if (!i->Pos) continue;
				for (e=first; i->Pos!=e && e!=last; e=EM_LSTIMP_NEXT(e));
				if (e==last) continue;
			}
			i->Pos=EM_LSTIMP_NEXT(last);
		}
		if (!EM_LSTIMP_PREV(first)) {
			Data->First=EM_LSTIMP_NEXT(last);
		}
		else {
			EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=EM_LSTIMP_NEXT(last);
			EM_LSTIMP_PREV(first)=NULL;
		}
		if (!EM_LSTIMP_NEXT(last)) {
			Data->Last=EM_LSTIMP_PREV(first);
		}
		else {
			EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=EM_LSTIMP_PREV(first);
			EM_LSTIMP_NEXT(last)=NULL;
		}
		d=new SharedData;
		d->First=(OBJ*)first;
		d->Last=(OBJ*)last;
		d->IsStaticEmpty=false;
		d->RefCount=1;
	}
	return emList<OBJ>(d);
}

template <class OBJ> inline void emList<OBJ>::RemoveFirst()
{
	Remove(Data->First,Data->First);
}

template <class OBJ> inline void emList<OBJ>::RemoveLast()
{
	Remove(Data->Last,Data->Last);
}

template <class OBJ> inline void emList<OBJ>::Remove(const OBJ * elem)
{
	Remove(elem,elem);
}

template <class OBJ> void emList<OBJ>::Remove(
	const OBJ * first, const OBJ * last
)
{
	const OBJ * e;
	OBJ * e2;
	Iterator * i;
	SharedData * d;

	if (!first || !last) return;
	if (first==Data->First && last==Data->Last) {
		for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
		if (Data->RefCount>1) {
			Data->RefCount--;
			Data=&EmptyData;
			return;
		}
	}
	else {
		for (i=Iterators; i; i=i->NextIter) {
			if (i->Pos!=last) {
				if (!i->Pos) continue;
				for (e=first; i->Pos!=e && e!=last; e=EM_LSTIMP_NEXT(e));
				if (e==last) continue;
			}
			i->Pos=EM_LSTIMP_NEXT(last);
		}
	}
	if (Data->RefCount==1) {
		if (!EM_LSTIMP_PREV(first)) Data->First=EM_LSTIMP_NEXT(last);
		else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(first))=EM_LSTIMP_NEXT(last);
		if (!EM_LSTIMP_NEXT(last)) Data->Last=EM_LSTIMP_PREV(first);
		else EM_LSTIMP_PREV(EM_LSTIMP_NEXT(last))=EM_LSTIMP_PREV(first);
		do {
			e=first;
			first=EM_LSTIMP_NEXT(first);
			delete EM_LSTIMP_ELEM(e);
		} while (e!=last);
	}
	else {
		d=new SharedData;
		d->First=NULL;
		d->Last=NULL;
		d->IsStaticEmpty=false;
		d->RefCount=1;
		for (e=Data->First; e; e=EM_LSTIMP_NEXT(e)) {
			if (e==first) {
				e=EM_LSTIMP_NEXT(last);
				if (!e) break;
			}
			e2=&(new Element(*e))->Obj;
			EM_LSTIMP_NEXT(e2)=NULL;
			if ((EM_LSTIMP_PREV(e2)=d->Last)==NULL) d->First=e2;
			else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e2))=e2;
			d->Last=e2;
			for (i=Iterators; i; i=i->NextIter) {
				if (i->Pos==e) i->Pos=e2;
			}
		}
		Data->RefCount--;
		Data=d;
	}
}

template <class OBJ> void emList<OBJ>::Empty(bool compact)
{
	OBJ * e1, * e2;
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	if (Data->RefCount>1 || compact) {
		if (!--Data->RefCount) DeleteData();
		Data=&EmptyData;
	}
	else {
		for (e1=Data->First; e1; e1=e2) {
			e2=EM_LSTIMP_NEXT(e1);
			delete EM_LSTIMP_ELEM(e1);
		}
		Data->First=NULL;
		Data->Last=NULL;
	}
}

template <class OBJ> inline bool emList<OBJ>::IsEmpty() const
{
	return !Data->First;
}

template <class OBJ> bool emList<OBJ>::Sort(
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
)
{
	if (Data->First==Data->Last) return false;
	if (Data->RefCount>1) MakeWritable();
	return emSortDoubleLinkedList(
		(void**)(void*)&Data->First,
		(void**)(void*)&Data->Last,
		offsetof(Element,Next)-offsetof(Element,Obj),
		offsetof(Element,Prev)-offsetof(Element,Obj),
		(int(*)(void*,void*,void*))compare,
		context
	);
}

template <class OBJ> int emList<OBJ>::GetCount() const
{
	const OBJ * e;
	int cnt;

	for (cnt=0, e=Data->First; e; cnt++, e=EM_LSTIMP_NEXT(e));
	return cnt;
}

template <class OBJ> const OBJ * emList<OBJ>::GetAtIndex(int index) const
{
	const OBJ * e;

	if (index<0) e=NULL;
	else for (e=Data->First; e && --index>=0; e=EM_LSTIMP_NEXT(e));
	return e;
}

template <class OBJ> int emList<OBJ>::GetIndexOf(const OBJ * elem) const
{
	const OBJ * e;
	int i;

	for (i=0, e=Data->First; e; i++, e=EM_LSTIMP_NEXT(e)) {
		if (e==elem) return i;
	}
	return -1;
}

template <class OBJ> unsigned int emList<OBJ>::GetDataRefCount() const
{
	return Data->IsStaticEmpty ? UINT_MAX/2 : Data->RefCount;
}

template <class OBJ> inline void emList<OBJ>::MakeNonShared()
{
	MakeWritable();
}

template <class OBJ> inline emList<OBJ>::Iterator::Iterator()
{
	Pos=NULL;
	List=NULL;
}

template <class OBJ> emList<OBJ>::Iterator::Iterator(
#	if defined(__WATCOMC__)
		const emList<OBJ>::Iterator & iter
#	else
		const Iterator & iter
#	endif
)
{
	Pos=iter.Pos;
	if ((List=iter.List)!=NULL) {
		NextIter=List->Iterators;
		List->Iterators=this;
	}
}

template <class OBJ> emList<OBJ>::Iterator::Iterator(
	const emList<OBJ> & list, const OBJ * elem
)
{
	Pos=elem;
	if ((List=(emList<OBJ>*)&list)!=NULL) {
		NextIter=List->Iterators;
		List->Iterators=this;
	}
}

template <class OBJ> emList<OBJ>::Iterator::~Iterator()
{
	Iterator * * pi;

	if (List) {
		for (pi=&List->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
	}
}

template <class OBJ> inline typename emList<OBJ>::Iterator &
	emList<OBJ>::Iterator::operator = (const Iterator & iter)
{
	Set(iter);
	return *this;
}

template <class OBJ> inline
	emList<OBJ>::Iterator::operator const OBJ * () const
{
	return Pos;
}

template <class OBJ> inline
	const OBJ * emList<OBJ>::Iterator::operator * () const
{
	return Pos;
}

template <class OBJ> inline
	const OBJ * emList<OBJ>::Iterator::operator -> () const
{
	return Pos;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::Get() const
{
	return Pos;
}

template <class OBJ> const OBJ * emList<OBJ>::Iterator::Set(
	const Iterator & iter
)
{
	Iterator * * pi;

	if (List!=iter.List) {
		if (List) {
			for (pi=&List->Iterators; *pi!=this; pi=&(*pi)->NextIter);
			*pi=NextIter;
		}
		if ((List=iter.List)!=NULL) {
			NextIter=List->Iterators;
			List->Iterators=this;
		}
	}
	Pos=iter.Pos;
	return Pos;
}

template <class OBJ> const OBJ * emList<OBJ>::Iterator::Set(
	const emList<OBJ> & list, const OBJ * elem
)
{
	Iterator * * pi;

	if (List!=&list) {
		if (List) {
			for (pi=&List->Iterators; *pi!=this; pi=&(*pi)->NextIter);
			*pi=NextIter;
		}
		List=(emList<OBJ>*)&list;
		NextIter=List->Iterators;
		List->Iterators=this;
	}
	Pos=elem;
	return elem;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::SetFirst(
	const emList<OBJ> & list
)
{
	return Set(list,list.Data->First);
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::SetLast(
	const emList<OBJ> & list
)
{
	return Set(list,list.Data->Last);
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::SetNext()
{
	Pos=EM_LSTIMP_NEXT(Pos);
	return Pos;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::SetPrev()
{
	Pos=EM_LSTIMP_PREV(Pos);
	return Pos;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::operator ++()
{
	Pos=EM_LSTIMP_NEXT(Pos);
	return Pos;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::operator --()
{
	Pos=EM_LSTIMP_PREV(Pos);
	return Pos;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::operator ++(int)
{
	const OBJ * res=Pos;
	Pos=EM_LSTIMP_NEXT(Pos);
	return res;
}

template <class OBJ> inline const OBJ * emList<OBJ>::Iterator::operator --(int)
{
	const OBJ * res=Pos;
	Pos=EM_LSTIMP_PREV(Pos);
	return res;
}

template <class OBJ> inline bool emList<OBJ>::Iterator::operator == (
	const Iterator & iter
) const
{
	return Pos==iter.Pos;
}

template <class OBJ> inline bool emList<OBJ>::Iterator::operator != (
	const Iterator & iter
) const
{
	return Pos!=iter.Pos;
}

template <class OBJ> inline bool emList<OBJ>::Iterator::operator == (
	const OBJ * elem
) const
{
	return Pos==elem;
}

template <class OBJ> inline bool emList<OBJ>::Iterator::operator != (
	const OBJ * elem
) const
{
	return Pos!=elem;
}

template <class OBJ> inline
	const emList<OBJ> * emList<OBJ>::Iterator::GetList() const
{
	return List;
}

template <class OBJ> void emList<OBJ>::Iterator::Detach()
{
	Iterator * * pi;

	if (List) {
		for (pi=&List->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
		List=NULL;
		Pos=NULL;
	}
}

template <class OBJ> typename emList<OBJ>::SharedData emList<OBJ>::EmptyData=
{
	NULL,NULL,true,UINT_MAX/2
};

template <class OBJ> void emList<OBJ>::MakeWritable()
{
	const OBJ * p1, * p2, * p3;

	p1=NULL; p2=NULL; p3=NULL;
	MakeWritable(&p1,&p2,&p3);
}

template <class OBJ> void emList<OBJ>::MakeWritable(const OBJ * * preserve)
{
	const OBJ * p2, * p3;

	p2=NULL; p3=NULL;
	MakeWritable(preserve,&p2,&p3);
}

template <class OBJ> void emList<OBJ>::MakeWritable(
	const OBJ * * preserve1, const OBJ * * preserve2
)
{
	const OBJ * p3;

	p3=NULL;
	MakeWritable(preserve1,preserve2,&p3);
}

template <class OBJ> void emList<OBJ>::MakeWritable(
	const OBJ * * preserve1, const OBJ * * preserve2,
	const OBJ * * preserve3
)
{
	SharedData * d1, * d2;
	OBJ * e1, * e2;
	Iterator * i;

	d1=Data;
	if (d1->RefCount>1 || Data->IsStaticEmpty) {
		d2=new SharedData;
		d2->First=NULL;
		d2->Last=NULL;
		d2->IsStaticEmpty=false;
		d2->RefCount=1;
		d1->RefCount--;
		Data=d2;
		for (e1=d1->First; e1; e1=EM_LSTIMP_NEXT(e1)) {
			e2=&(new Element(*e1))->Obj;
			EM_LSTIMP_NEXT(e2)=NULL;
			if ((EM_LSTIMP_PREV(e2)=d2->Last)==NULL) d2->First=e2;
			else EM_LSTIMP_NEXT(EM_LSTIMP_PREV(e2))=e2;
			d2->Last=e2;
			for (i=Iterators; i; i=i->NextIter) {
				if (i->Pos==e1) i->Pos=e2;
			}
			if (*preserve1==e1) *preserve1=e2;
			if (*preserve2==e1) *preserve2=e2;
			if (*preserve3==e1) *preserve3=e2;
		}
	}
}

template <class OBJ> void emList<OBJ>::DeleteData()
{
	OBJ * e, * n;

	EmptyData.RefCount=UINT_MAX/2;

	// Never do a
	//  if (Data!=&EmptyData)...
	// instead of
	//  if (!Data->IsStaticEmpty)...
	// because static member variables of template classes could exist
	// multiple times for the same final type (e.g. with Windows DLLs).
	if (!Data->IsStaticEmpty) {
		for (e=Data->First; e; e=n) {
			n=EM_LSTIMP_NEXT(e);
			delete EM_LSTIMP_ELEM(e);
		}
		delete Data;
	}
}


#endif
