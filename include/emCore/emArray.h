//------------------------------------------------------------------------------
// emArray.h
//
// Copyright (C) 2005-2009 Oliver Hamann.
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

#ifndef emArray_h
#define emArray_h

#include <new>

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//============================== Array functions ===============================
//==============================================================================

template <class OBJ> bool emSortArray(
	OBJ * array, int count,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
);
	// Sort an array where the elements are of the type OBJ. The order of
	// equal elements is preserved. It is a merge-sort algorithm.
	// Arguments:
	//   array   - The array to be sorted.
	//   count   - Number of elements in the array.
	//   compare - Function for comparing two elements.
	//             If you want the elements to be compared via the operators
	//             '<' and '>', say:
	//               emStdComparer<OBJ>::Compare
	//             with OBJ replaced by the real type of the elements. The
	//             context argument is ignored then.
	//             Arguments:
	//               obj1    - Pointer to first element.
	//               obj2    - Pointer to second element.
	//               context - See below.
	//             Returns: Zero if the elements are equal, a value greater
	//               than zero if the first element is greater than the
	//               second one, and a value less than zero if the first
	//               element is less than the second one.
	//   context - Any pointer to be forwarded to the compare function.
	// Returns: true if there was a change, false otherwise.


template <class OBJ, class KEY> int emBinarySearch(
	OBJ * array, int count, KEY key,
	int(*compare)(const OBJ * obj, KEY key, void * context),
	void * context
);
	// Perform a binary search in a sorted array of elements by comparing
	// the elements against a key.
	// Arguments:
	//   array   - The array.
	//   count   - Number of elements in the array.
	//   key     - The key to be searched for.
	//   compare - Function for comparing an element against the key.
	//             If KEY is 'const OBJ *', and if you want the operators
	//             '<' and '>' to be used, say:
	//               emStdComparer<OBJ>::Compare
	//             with OBJ replaced by the real type of the elements. The
	//             context argument is ignored then.
	//             Arguments:
	//               obj     - Pointer to the element.
	//               key     - The key.
	//               context - See below.
	//             Returns: Zero if the element matches the key, a value
	//               greater than zero if the element is greater, and a
	//               value less than zero if it is less.
	//   context - Any pointer to be forwarded to the compare function.
	// Returns:
	//   If a matching element could be found, the index of that element is
	//   returned. Otherwise a value less than zero is returned: the binary
	//   inversion of the index for insertion.


//==============================================================================
//================================== emArray ===================================
//==============================================================================

template <class OBJ> class emArray {

public:

	// Template class for a dynamic array with copy-on-write behavior. The
	// template parameter OBJ is the type of the elements. To avoid frequent
	// reallocations, the array manages a capacity (number of elements
	// memory is allocated for), which may be greater than the count (number
	// of valid elements). The capacity can be up to 3*count-1, or 2*count
	// after increasing only.

	emArray();
		// Construct an empty array.

	emArray(const emArray & array);
		// Construct a copied array.
		// Arguments:
		//   array - The array to be copied.

	emArray(const OBJ * array, int count, int tuningLevel=0);
		// Construct a copied array.
		// Arguments:
		//   array       - The array to be copied.
		//   count       - Number of elements in array.
		//   tuningLevel - Please read the comments on SetTuningLevel.

	emArray(const OBJ * array, int count, const OBJ * array2, int count2,
	        int tuningLevel=0);
		// Construct an array by copying two source arrays.
		// Arguments:
		//   array       - The array to be copied to the beginning.
		//   count       - Number of elements in array.
		//   array2      - The array to be copied to the end.
		//   count2      - Number of elements in array2.
		//   tuningLevel - Please read the comments on SetTuningLevel.

	emArray(const OBJ & obj, int count=1, int tuningLevel=0);
		// Construct an array by filling.
		// Arguments:
		//   obj         - The object to be copied to all elements.
		//   count       - Number of elements in the array.
		//   tuningLevel - Please read the comments on SetTuningLevel.

	~emArray();
		// Destructor.

	emArray & operator = (const emArray & array);
	emArray & operator = (const OBJ & obj);
		// Copy an array or an object to this array.

	int GetTuningLevel() const;
	void SetTuningLevel(int tuningLevel);
		// **************************************************
		// *** CAUTION: NEVER SET A TOO HIGH TUNING LEVEL ***
		// **************************************************
		// Get or set the tuning level. This has influence on the
		// performance. The maximum allowed tuning level depends
		// on the data type OBJ. Possible tuning levels are:
		//   0 - Default level. Can be used for every type.
		//   1 - Objects are moved in memory without calling copy
		//       constructors on the target and without calling
		//       destructors on the source. Can be used for
		//       emString, emArray, emImage... but NOT emList.
		//   2 - Like 1, but objects are copied in memory without
		//       calling copy constructors or copy operators.
		//   3 - Like 2, but destructors are never called. Could
		//       possibly be used for very simple classes which
		//       still have virtual methods.
		//   4 - Like 3, but constructors are never called. Can
		//       be used for primitive types like int, double,
		//       pointers and even emColor.

	int GetCount() const;
		// Get the number of elements in this array.

	void SetCount(int count, bool compact=false);
		// Set the number of elements in this array. Additional elements
		// are set to default state. But if OBJ is a primitive type, or
		// if the tuning level is 4, additional elements are not
		// initialized.
		// Arguments:
		//   count   - The new number of elements in the array.
		//   compact - Whether to make the capacity equal to the count.

	void Compact();
		// Make the capacity equal to the count.

	operator const OBJ * () const;
	const OBJ * Get() const;
		// Get a pointer to the first element in this array, that is,
		// get the array as a normal C array. At least because of the
		// copy-on-write feature, the pointer is valid only until
		// calling any non-const method or operator on this array, or
		// giving this array as a non-const argument to any call in the
		// world. Hint: Even methods like Add, Insert, Replace and
		// GetSubArray may make shallow copies, like the copy operator
		// and copy constructor do.

	const OBJ & operator [] (int index) const;
	const OBJ & Get(int index) const;
		// Get a reference to an element. The rules for the validity of
		// the reference are the same as with the pointer returned by
		// Get().
		// Arguments:
		//   index - The index of the desired element. This must be
		//           within the range of 0 to GetCount()-1.
		// Returns: The reference to the element.

	OBJ * GetWritable();
	OBJ & GetWritable(int index);
		// Like Get() and Get(index), but for modifying the elements.
		// There is no non-const version of the operator [], because
		// compilers would make use of it too often. The rules for the
		// validity of the pointer or reference are the same as with
		// Get(), but modification is allowed only until doing something
		// which could make a shallow copy of this array.

	void Set(int index, const OBJ & obj);
		// Set an element.
		// Arguments:
		//   index - The index of the element to be set. This must be
		//           within the range of 0 to GetCount()-1.
		//    obj  - The object to be copied to the element.

	void Add(const emArray & array, bool compact=false);
	void Add(const OBJ * array, int count, bool compact=false);
	void Add(const OBJ & obj, int count=1, bool compact=false);
	void AddNew(int count=1, bool compact=false);
		// Like the Insert methods, but with index=GetCount().

	emArray & operator += (const emArray & array);
	emArray & operator += (const OBJ & obj);
	emArray operator + (const emArray & array) const;
	emArray operator + (const OBJ & obj) const;
		// Similar to the Add methods...

	//friend emArray operator + (const OBJ & obj, const emArray & array);
		// This one even exists and can be used.
		// (Having the declaration here would not be portable)

	void Insert(int index, const emArray & array, bool compact=false);
	void Insert(int index, const OBJ * array, int count,
	            bool compact=false);
	void Insert(int index, const OBJ & obj, int count=1,
	            bool compact=false);
	void InsertNew(int index, int count=1, bool compact=false);
		// Like the Replace methods, but with remCount=0.

	void Replace(int index, int remCount, const emArray & array,
	             bool compact=false);
	void Replace(int index, int remCount, const OBJ * array, int count,
	             bool compact=false);
	void Replace(int index, int remCount, const OBJ & obj, int count=1,
	             bool compact=false);
	void ReplaceByNew(int index, int remCount, int count=1,
	                  bool compact=false);
		// Remove and/or insert elements at a particular position. The
		// memory areas of source and target may overlap. That means, an
		// array could be inserted into itself, elements could be moved
		// within the array, and so on. With ReplaceByNew, the new
		// elements are set to default state, but if OBJ is a primitive
		// type, or if the tuning level is 4, the new elements are not
		// initialized. index and remCount are clipped if they are out
		// of range.
		// Arguments:
		//   index    - Index of first element to be removed, and where
		//              new elements are to be inserted.
		//   remCount - Number of elements to be removed.
		//   array    - An array whose elements are to be copied to the
		//              new elements.
		//   obj      - An object to be copied for filling the new
		//              elements.
		//   count or array.GetCount() - Number of elements to be
		//              inserted.
		//   compact  - Whether to minimize the capacity.

	emArray GetSubArray(int index, int count, bool compact=false) const;
		// Create a sub-array from elements of this array.
		// Arguments:
		//   index   - Index of first element to be copied into the
		//             returned array.
		//   count   - Number of elements to be copied into the
		//             returned array
		//   compact - Whether to minimize the capacity.
		// Returns: The sub-array.

	emArray Extract(int index, int count, bool compact=false);
		// Like GetSubArray, but remove the affected elements from this
		// array.

	void Remove(int index, int count=1, bool compact=false);
		// Remove elements from this array.
		// Arguments:
		//   index   - Index of first element to be removed.
		//   count   - Number of elements to be removed.
		//   compact - Whether to minimize the capacity.

	void Empty(bool compact=false);
		// Remove all elements (set zero count).
		// Arguments:
		//   compact - Whether to minimize the capacity.

	bool IsEmpty() const;
		// Ask whether the number of elements is zero.

	int PointerToIndex(const OBJ * ptr) const;
		// Get the index of the element whose address is ptr.
		// Returns -1 if ptr is out of range.

	bool Sort(
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
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

	int BinarySearch(
		const OBJ & obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL
	) const;
		// Search for an element equal to a given object. The array must
		// already be sorted by the same compare function.
		// Arguments:
		//   obj     - An object which is equal to the searched element.
		//   compare - Please see the Sort method.
		//   context - Please see the Sort method.
		// Returns:
		//   If a matching element could be found, the index of that
		//   element is returned. Otherwise a value less than zero is
		//   returned: the binary inversion of the index for insertion.

	int BinarySearchByKey(
		void * key,
		int(*compareObjKey)(const OBJ * obj, void * key,
		                    void * context),
		void * context=NULL
	) const;
		// Like BinarySearch, but with comparing the elements against a
		// key, which is given as a 'void *' here (because some
		// compilers don't like nested templates).

	void BinaryInsert(
		const OBJ & obj,
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

	bool BinaryInsertIfNew(
		const OBJ & obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL,
		bool compact=false
	);
		// Like BinaryInsert, but return false and do nothing if there
		// is already an element which equals the given object.

	void BinaryInsertOrReplace(
		const OBJ & obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL,
		bool compact=false
	);
		// Like BinaryInsert, but if there is already an element which
		// equals the given object, copy the given object into that
		// element, instead of inserting an additional element.

	bool BinaryReplace(
		const OBJ & obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL,
		bool compact=false
	);
		// Like BinaryInsertOrReplace, but do nothing and return false
		// if there is no element which equals the given object.

	bool BinaryRemove(
		const OBJ & obj,
		int(*compare)(const OBJ * obj1, const OBJ * obj2,
		              void * context),
		void * context=NULL,
		bool compact=false
	);
	bool BinaryRemoveByKey(
		void * key,
		int(*compareObjKey)(const OBJ * obj, void * key,
		                    void * context),
		void * context=NULL,
		bool compact=false
	);
		// Like BinarySearch and BinarySearchByKey, but remove the found
		// element, or return false if no such element can been found.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this array.

	void MakeNonShared();
		// This must be called before handing the array to another
		// thread. This method is not recursive. So if the object class
		// even has such a method, you have to call it on every object
		// too.

private:

	struct EmptySharedData {
		int Count;
		int Capacity;
		short TuningLevel;
		short IsStaticEmpty;
		unsigned int RefCount;
	};
	struct SharedData : EmptySharedData {
		OBJ Obj[16];
	};

	void Construct(OBJ * array, const OBJ * src, bool srcIsArray,
	               int count);
	void Copy(OBJ * array, const OBJ * src, bool srcIsArray, int count);
	void Move(OBJ * array, OBJ * srcArray, int count);
	void Destruct(OBJ * array, int count);
	static SharedData * AllocData(int capacity, int tuningLevel);
	void FreeData();
	void MakeWritable();
	void PrivRep(int index, int remCount, const OBJ * src, bool srcIsArray,
	             int insCount, bool compact);

	SharedData * Data;
	static EmptySharedData EmptyData[5];
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//------------------------------ Array functions -------------------------------

template <class OBJ> bool emSortArray(
	OBJ * array, int count,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
)
{
	int autoIndices[384];
	int stackMem[128];
	int * stack, * indices, * s1, * s2, * t, * e;
	int i, j, k, l;
	OBJ * tmpArray;

	if (count<=1) return false;
	i=count+count/2;
	if (i<=(int)(sizeof(autoIndices)/sizeof(int))) indices=autoIndices;
	else indices=(int*)malloc(i*sizeof(int));
	for (stack=stackMem, stack[0]=0, i=0, j=0, k=count;;) {
		while (count>2) {
			stack+=4;
			stack[0]=i;
			stack[1]=count;
			stack[2]=j;
			stack[3]=k;
			l=count/2;
			count-=l;
			i+=l;
			j+=l;
		}
		t=indices+j;
		if (count<2) {
			t[0]=i;
		}
		else if (compare(array+i,array+i+1,context)<=0) {
			t[0]=i;
			t[1]=i+1;
		}
		else {
			t[0]=i+1;
			t[1]=i;
		}
		while (stack[0]<0) {
			count=stack[1];
			t=indices+stack[2];
			s1=indices+stack[3];
			s2=t+count/2;
			e=t+count;
			stack-=4;
			for (;;) {
				if (compare(array+*s1,array+*s2,context)<=0) {
					*t++=*s1++;
					if (t<s2) continue;
					break;
				}
				*t++=*s2++;
				if (s2<e) continue;
				do { *t++=*s1++; } while (t<s2);
				break;
			}
		}
		if (stack==stackMem) break;
		i=stack[0];
		count=stack[1]/2;
		j=stack[3];
		k=stack[2];
		stack[0]=-1;
	}
	tmpArray=(OBJ*)malloc(count*sizeof(OBJ));
	for (i=0; i<count; i++) {
		::new ((void*)(tmpArray+i)) OBJ(array[i]);
	}
	for (i=count-1, k=-1; i>=0; i--) {
		j=indices[i];
		if (i!=j) {
			k=0;
			array[i]=tmpArray[j];
		}
		tmpArray[j].~OBJ();
	}
	free((void*)tmpArray);
	if (indices!=autoIndices) free(indices);
	return k>=0;
}


template <class OBJ, class KEY> int emBinarySearch(
	OBJ * array, int count, KEY key,
	int(*compare)(const OBJ * obj, KEY key, void * context),
	void * context
)
{
	int i1,i2,i,d;

	i2=count;
	if (i2) {
		i1=0;
		for (;;) {
			i=(i1+i2)>>1;
			d=compare(array+i,key,context);
			if (d>0) {
				i2=i;
				if (i1<i2) continue;
				break;
			}
			if (d<0) {
				i1=i+1;
				if (i1<i2) continue;
				break;
			}
			return i;
		}
	}
	return ~i2;
}


//---------------------------------- emArray -----------------------------------

template <class OBJ> inline emArray<OBJ>::emArray()
{
	Data=(SharedData*)&EmptyData[0];
}

template <class OBJ> inline emArray<OBJ>::emArray(const emArray & array)
{
	Data=array.Data;
	Data->RefCount++;
}

template <class OBJ> emArray<OBJ>::emArray(
	const OBJ * array, int count, int tuningLevel
)
{
	if (count>0) {
		Data=AllocData(count,tuningLevel);
		Data->Count=count;
		Construct(Data->Obj,array,true,count);
	}
	else {
		Data=(SharedData*)&EmptyData[tuningLevel];
	}
}

template <class OBJ> emArray<OBJ>::emArray(
	const OBJ * array, int count, const OBJ * array2, int count2,
	int tuningLevel
)
{
	if (count<0) count=0;
	if (count2<0) count2=0;
	if (count+count2>0) {
		Data=AllocData(count+count2,tuningLevel);
		Data->Count=count+count2;
		Construct(Data->Obj,array,true,count);
		Construct(Data->Obj+count,array2,true,count2);
	}
	else {
		Data=(SharedData*)&EmptyData[tuningLevel];
	}
}

template <class OBJ> emArray<OBJ>::emArray(
	const OBJ & obj, int count, int tuningLevel
)
{
	if (count>0) {
		Data=AllocData(count,tuningLevel);
		Data->Count=count;
		Construct(Data->Obj,&obj,false,count);
	}
	else {
		Data=(SharedData*)&EmptyData[tuningLevel];
	}
}

template <class OBJ> inline emArray<OBJ>::~emArray()
{
	if (!--Data->RefCount) FreeData();
}

template <class OBJ> inline emArray<OBJ> & emArray<OBJ>::operator = (
	const emArray & array
)
{
	array.Data->RefCount++;
	if (!--Data->RefCount) FreeData();
	Data=array.Data;
	return *this;
}

template <class OBJ> inline emArray<OBJ> & emArray<OBJ>::operator = (
	const OBJ & obj
)
{
	PrivRep(0,Data->Count,&obj,false,1,false);
	return *this;
}

template <class OBJ> inline int emArray<OBJ>::GetTuningLevel() const
{
	return Data->TuningLevel;
}

template <class OBJ> void emArray<OBJ>::SetTuningLevel(int tuningLevel)
{
	if (Data->TuningLevel!=tuningLevel) {
		if (!Data->Count) {
			if (!--Data->RefCount) FreeData();
			Data=(SharedData*)&EmptyData[tuningLevel];
		}
		else {
			if (Data->RefCount>1) MakeWritable();
			Data->TuningLevel=(short)tuningLevel;
		}
	}
}

template <class OBJ> inline int emArray<OBJ>::GetCount() const
{
	return Data->Count;
}

template <class OBJ> void emArray<OBJ>::SetCount(int count, bool compact)
{
	if (count>Data->Count) {
		PrivRep(Data->Count,0,NULL,false,count-Data->Count,compact);
	}
	else {
		PrivRep(count,Data->Count-count,NULL,false,0,compact);
	}
}

template <class OBJ> inline void emArray<OBJ>::Compact()
{
	PrivRep(0,0,NULL,false,0,true);
}

template <class OBJ> inline emArray<OBJ>::operator const OBJ * () const
{
	return Data->Obj;
}

template <class OBJ> inline const OBJ * emArray<OBJ>::Get() const
{
	return Data->Obj;
}

template <class OBJ> inline const OBJ & emArray<OBJ>::operator [] (
	int index
) const
{
	return Data->Obj[index];
}

template <class OBJ> inline const OBJ & emArray<OBJ>::Get(int index) const
{
	return Data->Obj[index];
}

template <class OBJ> inline OBJ * emArray<OBJ>::GetWritable()
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Obj;
}

template <class OBJ> inline OBJ & emArray<OBJ>::GetWritable(int index)
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Obj[index];
}

template <class OBJ> inline void emArray<OBJ>::Set(int index, const OBJ & obj)
{
	if (Data->RefCount>1) MakeWritable();
	Data->Obj[index]=obj;
}

template <class OBJ> inline void emArray<OBJ>::Add(
	const emArray & array, bool compact
)
{
	Replace(Data->Count,0,array,compact);
}

template <class OBJ> inline void emArray<OBJ>::Add(
	const OBJ * array, int count, bool compact
)
{
	PrivRep(Data->Count,0,array,true,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::Add(
	const OBJ & obj, int count, bool compact
)
{
	PrivRep(Data->Count,0,&obj,false,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::AddNew(int count, bool compact)
{
	PrivRep(Data->Count,0,NULL,false,count,compact);
}

template <class OBJ> inline emArray<OBJ> & emArray<OBJ>::operator += (
	const emArray & array
)
{
	Replace(Data->Count,0,array,false);
	return *this;
}

template <class OBJ> inline emArray<OBJ> & emArray<OBJ>::operator += (
	const OBJ & obj
)
{
	PrivRep(Data->Count,0,&obj,false,1,false);
	return *this;
}

template <class OBJ> emArray<OBJ> emArray<OBJ>::operator + (
	const emArray & array
) const
{
	if (!array.Data->Count) return *this;
	if (!Data->Count) return array;
	return emArray<OBJ>(
		Data->Obj,Data->Count,
		array.Data->Obj,array.Data->Count,
		Data->TuningLevel
	);
}

template <class OBJ> inline emArray<OBJ> emArray<OBJ>::operator + (
	const OBJ & obj
) const
{
	return emArray<OBJ>(Data->Obj,Data->Count,&obj,1,Data->TuningLevel);
}

template <class OBJ> inline emArray<OBJ> operator + (
	const OBJ & obj, const emArray<OBJ> & array
)
{
	return emArray<OBJ>(
		&obj,1,array.Get(),array.GetCount(),array.GetTuningLevel()
	);
}

template <class OBJ> inline void emArray<OBJ>::Insert(
	int index, const emArray & array, bool compact
)
{
	Replace(index,0,array,compact);
}

template <class OBJ> inline void emArray<OBJ>::Insert(
	int index, const OBJ * array, int count, bool compact
)
{
	PrivRep(index,0,array,true,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::Insert(
	int index, const OBJ & obj, int count, bool compact
)
{
	PrivRep(index,0,&obj,false,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::InsertNew(
	int index, int count, bool compact
)
{
	PrivRep(index,0,NULL,false,count,compact);
}

template <class OBJ> void emArray<OBJ>::Replace(
	int index, int remCount, const emArray & array, bool compact
)
{
	if (
		index<=0 &&
		index+remCount>=Data->Count &&
		array.Data->TuningLevel==Data->TuningLevel &&
		(!compact || array.Data->Capacity==array.Data->Count)
	) {
		array.Data->RefCount++;
		if (!--Data->RefCount) FreeData();
		Data=array.Data;
	}
	else {
		PrivRep(index,remCount,array.Data->Obj,true,array.Data->Count,
		        compact);
	}
}

template <class OBJ> inline void emArray<OBJ>::Replace(
	int index, int remCount, const OBJ * array, int count, bool compact
)
{
	PrivRep(index,remCount,array,true,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::Replace(
	int index, int remCount, const OBJ & obj, int count, bool compact
)
{
	PrivRep(index,remCount,&obj,false,count,compact);
}

template <class OBJ> inline void emArray<OBJ>::ReplaceByNew(
	int index, int remCount, int count, bool compact
)
{
	PrivRep(index,remCount,NULL,false,count,compact);
}

template <class OBJ> emArray<OBJ> emArray<OBJ>::GetSubArray(
	int index, int count, bool compact
) const
{
	if (index<0) { count+=index; index=0; }
	if (count>Data->Count-index) count=Data->Count-index;
	if (count==Data->Count && (!compact || Data->Capacity==Data->Count)) {
		return *this;
	}
	else {
		return emArray<OBJ>(Data->Obj+index,count,Data->TuningLevel);
	}
}

template <class OBJ> emArray<OBJ> emArray<OBJ>::Extract(
	int index, int count, bool compact
)
{
	emArray<OBJ> result(GetSubArray(index,count,compact));
	PrivRep(index,count,NULL,false,0,compact);
	return result;
}

template <class OBJ> inline void emArray<OBJ>::Remove(
	int index, int count, bool compact
)
{
	PrivRep(index,count,NULL,false,0,compact);
}

template <class OBJ> inline void emArray<OBJ>::Empty(bool compact)
{
	PrivRep(0,Data->Count,NULL,false,0,compact);
}

template <class OBJ> inline bool emArray<OBJ>::IsEmpty() const
{
	return !Data->Count;
}

template <class OBJ> int emArray<OBJ>::PointerToIndex(const OBJ * ptr) const
{
	if (ptr<Data->Obj || ptr>=Data->Obj+Data->Count) return -1;
	return ptr-Data->Obj;
}

template <class OBJ> bool emArray<OBJ>::Sort(
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
)
{
	if (Data->RefCount>1) MakeWritable();
	return emSortArray(Data->Obj,Data->Count,compare,context);
}

template <class OBJ> inline int emArray<OBJ>::BinarySearch(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context
) const
{
	return emBinarySearch(Data->Obj,Data->Count,&obj,compare,context);
}

template <class OBJ> inline int emArray<OBJ>::BinarySearchByKey(
	void * key,
	int(*compareObjKey)(const OBJ * obj, void * key, void * context),
	void * context
) const
{
	return emBinarySearch(Data->Obj,Data->Count,key,compareObjKey,context);
}

template <class OBJ> void emArray<OBJ>::BinaryInsert(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearch(obj,compare,context);
	if (i<0) i=~i;
	PrivRep(i,0,&obj,false,1,compact);
}

template <class OBJ> bool emArray<OBJ>::BinaryInsertIfNew(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearch(obj,compare,context);
	if (i<0) {
		PrivRep(~i,0,&obj,false,1,compact);
		return true;
	}
	else {
		if (compact && Data->Count!=Data->Capacity) {
			PrivRep(0,0,NULL,false,0,true);
		}
		return false;
	}
}

template <class OBJ> void emArray<OBJ>::BinaryInsertOrReplace(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearch(obj,compare,context);
	if (i>=0) PrivRep(i,1,&obj,false,1,compact);
	else PrivRep(~i,0,&obj,false,1,compact);
}

template <class OBJ> bool emArray<OBJ>::BinaryReplace(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearch(obj,compare,context);
	if (i>=0) {
		PrivRep(i,1,&obj,false,1,compact);
		return true;
	}
	else {
		if (compact && Data->Count!=Data->Capacity) {
			PrivRep(0,0,NULL,false,0,true);
		}
		return false;
	}
}

template <class OBJ> bool emArray<OBJ>::BinaryRemove(
	const OBJ & obj,
	int(*compare)(const OBJ * obj1, const OBJ * obj2, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearch(obj,compare,context);
	if (i>=0) {
		PrivRep(i,1,NULL,false,0,compact);
		return true;
	}
	else {
		if (compact && Data->Count!=Data->Capacity) {
			PrivRep(0,0,NULL,false,0,true);
		}
		return false;
	}
}

template <class OBJ> bool emArray<OBJ>::BinaryRemoveByKey(
	void * key,
	int(*compareObjKey)(const OBJ * obj, void * key, void * context),
	void * context, bool compact
)
{
	int i;

	i=BinarySearchByKey(key,compareObjKey,context);
	if (i>=0) {
		PrivRep(i,1,NULL,false,0,compact);
		return true;
	}
	else {
		if (compact && Data->Count!=Data->Capacity) {
			PrivRep(0,0,NULL,false,0,true);
		}
		return false;
	}
}

template <class OBJ> unsigned int emArray<OBJ>::GetDataRefCount() const
{
	return Data->IsStaticEmpty ? UINT_MAX/2 : Data->RefCount;
}

template <class OBJ> inline void emArray<OBJ>::MakeNonShared()
{
	MakeWritable();
}

template <class OBJ> void emArray<OBJ>::Construct(
	OBJ * array, const OBJ * src, bool srcIsArray, int count
)
{
	if (count>0) {
		if (!src) {
			if (Data->TuningLevel<4) {
				do {
					count--;
					::new ((void*)(array+count)) OBJ();
				} while (count>0);
			}
		}
		else if (!srcIsArray) {
			do {
				count--;
				::new ((void*)(array+count)) OBJ(*src);
			} while (count>0);
		}
		else {
			if (Data->TuningLevel<2) {
				do {
					count--;
					::new ((void*)(array+count)) OBJ(src[count]);
				} while (count>0);
			}
			else {
				memcpy(array,src,count*sizeof(OBJ));
			}
		}
	}
}

template <class OBJ> void emArray<OBJ>::Copy(
	OBJ * array, const OBJ * src, bool srcIsArray, int count
)
{
	int i;

	if (count>0) {
		if (!src) {
			if (Data->TuningLevel<3) {
				do {
					count--;
					array[count].~OBJ();
					::new ((void*)(array+count)) OBJ();
				} while (count>0);
			}
			else if (Data->TuningLevel<4) {
				do {
					count--;
					::new ((void*)(array+count)) OBJ();
				} while (count>0);
			}
		}
		else if (!srcIsArray) {
			do {
				count--;
				array[count]=*src;
			} while (count>0);
		}
		else if (array!=src) {
			if (Data->TuningLevel<2) {
				if (array<src) {
					i=0;
					do {
						array[i]=src[i];
						i++;
					} while (i<count);
				}
				else {
					do {
						count--;
						array[count]=src[count];
					} while (count>0);
				}
			}
			else {
				memmove(array,src,count*sizeof(OBJ));
			}
		}
	}
}

template <class OBJ> void emArray<OBJ>::Move(
	OBJ * array, OBJ * srcArray, int count
)
{
	int i;

	if (count>0 && array!=srcArray) {
		if (Data->TuningLevel<1) {
			if (array<srcArray) {
				i=0;
				do {
					::new ((void*)(array+i)) OBJ(srcArray[i]);
					srcArray[i].~OBJ();
					i++;
				} while (i<count);
			}
			else {
				do {
					count--;
					::new ((void*)(array+count)) OBJ(srcArray[count]);
					srcArray[count].~OBJ();
				} while (count>0);
			}
		}
		else {
			memmove(array,srcArray,count*sizeof(OBJ));
		}
	}
}

template <class OBJ> void emArray<OBJ>::Destruct(OBJ * array, int count)
{
	if (Data->TuningLevel<3 && count>0) {
		do {
			count--;
			array[count].~OBJ();
		} while (count>0);
	}
}

template <class OBJ>
typename emArray<OBJ>::SharedData * emArray<OBJ>::AllocData(
	int capacity, int tuningLevel
)
{
	SharedData * d;

	d=(SharedData*)malloc(
		sizeof(SharedData)-sizeof(OBJ)*16+sizeof(OBJ)*capacity
	);
	d->Count=0;
	d->Capacity=capacity;
	d->TuningLevel=(short)tuningLevel;
	d->IsStaticEmpty=0;
	d->RefCount=1;
	return d;
}

template <class OBJ> void emArray<OBJ>::FreeData()
{
	int i;

	EmptyData[Data->TuningLevel].RefCount=UINT_MAX/2;

	// Never do a
	//  if (Data!=(SharedData*)&EmptyData[Data->TuningLevel])...
	// instead of
	//  if (!Data->IsStaticEmpty)...
	// because static member variables of template classes could exist
	// multiple times for the same final type (e.g. with Windows DLLs).
	if (!Data->IsStaticEmpty) {
		if (Data->TuningLevel<3) {
			i=Data->Count;
			while (--i>=0) Data->Obj[i].~OBJ();
		}
		free((void*)Data);
	}
}

template <class OBJ> void emArray<OBJ>::MakeWritable()
{
	SharedData * d;

	if (Data->RefCount>1 && !Data->IsStaticEmpty) {
		if (!Data->Count) {
			d=(SharedData*)&EmptyData[Data->TuningLevel];
		}
		else {
			d=AllocData(Data->Count,Data->TuningLevel);
			d->Count=Data->Count;
			Construct(d->Obj,Data->Obj,true,Data->Count);
		}
		Data->RefCount--;
		Data=d;
	}
}

template <class OBJ> void emArray<OBJ>::PrivRep(
	int index, int remCount, const OBJ * src, bool srcIsArray, int insCount,
	bool compact
)
{
	SharedData * d;
	int newCount,cap,l;

	d=Data;
	if ((unsigned int)index>(unsigned int)d->Count) {
		if (index<0) { remCount+=index; index=0; }
		else index=d->Count;
	}
	if ((unsigned int)remCount>(unsigned int)(d->Count-index)) {
		if (remCount<0) remCount=0;
		else remCount=d->Count-index;
	}
	if (insCount<0) insCount=0;
	if (!remCount && !insCount && (!compact || d->Count==d->Capacity)) {
		return;
	}
	newCount=d->Count+insCount-remCount;

	if (newCount<=0) {
		d=(SharedData*)&EmptyData[d->TuningLevel];
		if (!--Data->RefCount) FreeData();
		Data=d;
		return;
	}

	if (d->RefCount>1) {
		d=AllocData(newCount,d->TuningLevel);
		d->Count=newCount;
		if (index>0) Construct(d->Obj,Data->Obj,true,index);
		if (insCount>0) Construct(d->Obj+index,src,srcIsArray,insCount);
		l=newCount-index-insCount;
		if (l>0) {
			Construct(d->Obj+index+insCount,Data->Obj+index+remCount,true,l);
		}
		Data->RefCount--;
		Data=d;
		return;
	}

	if (compact) cap=newCount;
	else {
		cap=d->Capacity;
		if (cap<newCount || cap>=newCount*3) cap=newCount*2;
	}

	if (d->Capacity!=cap && d->TuningLevel<1) {
		d=AllocData(cap,d->TuningLevel);
		d->Count=newCount;
		if (insCount>0) Construct(d->Obj+index,src,srcIsArray,insCount);
		if (remCount>0) Destruct(Data->Obj+index,remCount);
		if (index>0) Move(d->Obj,Data->Obj,index);
		l=newCount-index-insCount;
		if (l>0) Move(d->Obj+index+insCount,Data->Obj+index+remCount,l);
		Data->Count=0;
		FreeData();
		Data=d;
		return;
	}

	if (insCount<=remCount) {
		if (insCount>0) Copy(d->Obj+index,src,srcIsArray,insCount);
		if (insCount<remCount) {
			l=newCount-index-insCount;
			if (l>0) Copy(d->Obj+index+insCount,d->Obj+index+remCount,true,l);
			Destruct(d->Obj+newCount,remCount-insCount);
		}
		if (d->Capacity!=cap) {
			d=(SharedData*)realloc(
				d,
				sizeof(SharedData)-sizeof(OBJ)*16+sizeof(OBJ)*cap
			);
			d->Capacity=cap;
			Data=d;
		}
		d->Count=newCount;
		return;
	}

	if (src<d->Obj || src>d->Obj+d->Count) {
		if (d->Capacity!=cap) {
			d=(SharedData*)realloc(
				d,
				sizeof(SharedData)-sizeof(OBJ)*16+sizeof(OBJ)*cap
			);
			d->Capacity=cap;
			Data=d;
		}
		if (remCount>0) {
			Copy(d->Obj+index,src,srcIsArray,remCount);
			if (srcIsArray) src+=remCount;
			index+=remCount;
			insCount-=remCount;
		}
		l=newCount-index-insCount;
		if (l>0) Move(d->Obj+index+insCount,d->Obj+index,l);
		Construct(d->Obj+index,src,srcIsArray,insCount);
		d->Count=newCount;
		return;
	}

	if (d->Capacity!=cap) {
		Data=(SharedData*)realloc(
			d,
			sizeof(SharedData)-sizeof(OBJ)*16+sizeof(OBJ)*cap
		);
		src+=Data->Obj-d->Obj;
		d=Data;
		d->Capacity=cap;
	}
	Construct(d->Obj+d->Count,NULL,false,insCount-remCount);
	d->Count=newCount;
	if (src<=d->Obj+index) {
		l=newCount-index-insCount;
		if (l>0) Copy(d->Obj+index+insCount,d->Obj+index+remCount,true,l);
		Copy(d->Obj+index,src,srcIsArray,insCount);
	}
	else {
		if (remCount>0) {
			Copy(d->Obj+index,src,srcIsArray,remCount);
			if (srcIsArray) src+=remCount;
			index+=remCount;
			insCount-=remCount;
		}
		l=newCount-index-insCount;
		if (l>0) Copy(d->Obj+index+insCount,d->Obj+index,true,l);
		if (src>=d->Obj+index) src+=insCount;
		Copy(d->Obj+index,src,srcIsArray,insCount);
	}
}

template <class OBJ>
typename emArray<OBJ>::EmptySharedData emArray<OBJ>::EmptyData[5]={
	{0,0,0,1,UINT_MAX/2},
	{0,0,1,1,UINT_MAX/2},
	{0,0,2,1,UINT_MAX/2},
	{0,0,3,1,UINT_MAX/2},
	{0,0,4,1,UINT_MAX/2},
};


#endif
