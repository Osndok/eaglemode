//------------------------------------------------------------------------------
// emAvlTree.h
//
// Copyright (C) 2005-2008,2010,2014-2015 Oliver Hamann.
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

#ifndef emAvlTree_h
#define emAvlTree_h

#include <new>

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//============================== AVL-tree macros ===============================
//==============================================================================

// Here you can find data types and macro definitions for a highly optimized AVL
// tree implementation. For an example of how to use it, see the implementation
// of the template class emAvlTreeMap more below.

//----------------------------------- Types ------------------------------------

struct emAvlNode {
	emAvlNode * Left;
	emAvlNode * Right;
	int Balance;
};

typedef emAvlNode * emAvlTree;

struct emAvlIterator {
	const emAvlNode * nstack[64];
	int depth;
};

//--------------------------------- Utilities ----------------------------------

int emAvlCheck(const emAvlTree tree);
	// Check consistency of the tree and return its height. Exits this
	// process and prints a message on failure.

#define EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,NODE_POINTER) \
	((ELEMENT_CLASS*)(((char*)(NODE_POINTER)) \
	-offsetof(ELEMENT_CLASS,NODE_MEMBER)))

//---------------------- Macros for the insert algorithm -----------------------

#define EM_AVL_INSERT_VARS(ELEMENT_CLASS) \
	emAvlTree * tstack[64]; \
	int depth; \
	emAvlNode * node1, * node2, * node3; \
	emAvlTree * tree, * tree2; \
	ELEMENT_CLASS * element;

#define EM_AVL_INSERT_BEGIN_SEARCH(ELEMENT_CLASS,NODE_MEMBER,TREE) \
	tree=&TREE; \
	depth=0; \
	node1=*tree; \
	if (!node1) element=NULL; \
	else for (;;) { \
		element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node1);

#define EM_AVL_INSERT_GO_LEFT \
		{ \
			tstack[depth++]=tree; \
			tree=&node1->Left; \
			node1=*tree; \
			if (node1) continue; \
			element=NULL; \
		}

#define EM_AVL_INSERT_GO_RIGHT \
		{ \
			tstack[depth++]=tree; \
			tree=&node1->Right; \
			node1=*tree; \
			if (node1) continue; \
			element=NULL; \
		}

#define EM_AVL_INSERT_END_SEARCH \
		break; \
	}

#define EM_AVL_INSERT_NOW(NODE_MEMBER) \
	node1=&element->NODE_MEMBER; \
	node1->Left=NULL; \
	node1->Right=NULL; \
	node1->Balance=0; \
	*tree=node1; \
	if (depth>0) for (;;) { \
		tree2=tree; \
		tree=tstack[--depth]; \
		node1=*tree; \
		if (tree2==&node1->Left) { \
			if (node1->Balance==0) { \
				node1->Balance=-1; \
				if (depth>0) continue; \
			} \
			else if (node1->Balance>0) { \
				node1->Balance=0; \
			} \
			else { \
				node2=node1->Left; \
				if (node2->Balance<0) { \
					*tree=node2; \
					node1->Left=node2->Right; \
					node2->Right=node1; \
					node1->Balance=0; \
					node2->Balance=0; \
				} \
				else { \
					node3=node2->Right; \
					*tree=node3; \
					node1->Left=node3->Right; \
					node1->Balance=-(node3->Balance>>1); \
					node2->Balance=(-node3->Balance)>>1; \
					node2->Right=node3->Left; \
					node3->Left=node2; \
					node3->Right=node1; \
					node3->Balance=0; \
				} \
			} \
		} \
		else { \
			if (node1->Balance==0) { \
				node1->Balance=1; \
				if (depth>0) continue; \
			} \
			else if (node1->Balance<0) { \
				node1->Balance=0; \
			} \
			else { \
				node2=node1->Right; \
				if (node2->Balance>0) { \
					*tree=node2; \
					node1->Right=node2->Left; \
					node2->Left=node1; \
					node1->Balance=0; \
					node2->Balance=0; \
				} \
				else { \
					node3=node2->Left; \
					*tree=node3; \
					node1->Right=node3->Left; \
					node1->Balance=(-node3->Balance)>>1; \
					node2->Balance=-(node3->Balance>>1); \
					node2->Left=node3->Right; \
					node3->Right=node2; \
					node3->Left=node1; \
					node3->Balance=0; \
				} \
			} \
		} \
		break; \
	}

//---------------------- Macros for the remove algorithm -----------------------

#define EM_AVL_REMOVE_VARS(ELEMENT_CLASS) \
	emAvlTree * tstack[64]; \
	int depth, depth2; \
	emAvlNode * node1, * node2, * node3; \
	emAvlTree * tree, * tree2; \
	ELEMENT_CLASS * element;

#define EM_AVL_REMOVE_BEGIN(ELEMENT_CLASS,NODE_MEMBER,TREE) \
	tree=&TREE; \
	depth=0; \
	node1=*tree; \
	if (!node1) element=NULL; \
	else for (;;) { \
		element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node1);

#define EM_AVL_REMOVE_GO_LEFT \
		{ \
			tstack[depth++]=tree; \
			tree=&node1->Left; \
			node1=*tree; \
			if (node1) continue; \
			element=NULL; \
		}

#define EM_AVL_REMOVE_GO_RIGHT \
		{ \
			tstack[depth++]=tree; \
			tree=&node1->Right; \
			node1=*tree; \
			if (node1) continue; \
			element=NULL; \
		}

#define EM_AVL_REMOVE_NOW \
		{ \
			if (!node1->Right) { \
				*tree=node1->Left; \
			} \
			else if (!node1->Left) { \
				*tree=node1->Right; \
			} \
			else { \
				depth2=depth; \
				tstack[depth++]=tree; \
				tree=&node1->Left; \
				node2=*tree; \
				if (node2->Right) do { \
					tstack[depth++]=tree; \
					tree=&node2->Right; \
					node2=*tree; \
				} while (node2->Right); \
				*tree=node2->Left; \
				node2->Left=node1->Left; \
				node2->Right=node1->Right; \
				node2->Balance=node1->Balance; \
				*tstack[depth2]=node2; \
				tstack[depth]=tree; \
				tstack[depth2+1]=&node2->Left; \
				tree=tstack[depth]; \
			} \
			if (depth>0) for (;;) { \
				tree2=tree; \
				tree=tstack[--depth]; \
				node1=*tree; \
				if (tree2==&node1->Left) { \
					if (node1->Balance<0) { \
						node1->Balance=0; \
						if (depth>0) continue; \
					} \
					else if (node1->Balance==0) { \
						node1->Balance=1; \
					} \
					else { \
						node2=node1->Right; \
						if (node2->Balance>=0) { \
							*tree=node2; \
							node1->Right=node2->Left; \
							node2->Left=node1; \
							if (node2->Balance!=0) { \
								node1->Balance=0; \
								node2->Balance=0; \
								if (depth>0) continue; \
							} \
							else { \
								node1->Balance=1; \
								node2->Balance=-1; \
							} \
						} \
						else { \
							node3=node2->Left; \
							*tree=node3; \
							node1->Right=node3->Left; \
							node1->Balance=(-node3->Balance)>>1; \
							node2->Balance=-(node3->Balance>>1); \
							node2->Left=node3->Right; \
							node3->Left=node1; \
							node3->Right=node2; \
							node3->Balance=0; \
							if (depth>0) continue; \
						} \
					} \
				} \
				else { \
					if (node1->Balance>0) { \
						node1->Balance=0; \
						if (depth>0) continue; \
					} \
					else if (node1->Balance==0) { \
						node1->Balance=-1; \
					} \
					else { \
						node2=node1->Left; \
						if (node2->Balance<=0) { \
							*tree=node2; \
							node1->Left=node2->Right; \
							node2->Right=node1; \
							if (node2->Balance!=0) { \
								node1->Balance=0; \
								node2->Balance=0; \
								if (depth>0) continue; \
							} \
							else { \
								node1->Balance=-1; \
								node2->Balance=1; \
							} \
						} \
						else { \
							node3=node2->Right; \
							*tree=node3; \
							node1->Left=node3->Right; \
							node1->Balance=-(node3->Balance>>1); \
							node2->Balance=(-node3->Balance)>>1; \
							node2->Right=node3->Left; \
							node3->Right=node1; \
							node3->Left=node2; \
							node3->Balance=0; \
							if (depth>0) continue; \
						} \
					} \
				} \
				break; \
			} \
		}

#define EM_AVL_REMOVE_END \
		break; \
	}

//---------------------- Macros for the search algorithm -----------------------

#define EM_AVL_SEARCH_VARS(ELEMENT_CLASS) \
	const emAvlNode * node; \
	ELEMENT_CLASS * element;

#define EM_AVL_SEARCH_BEGIN(ELEMENT_CLASS,NODE_MEMBER,TREE) \
	node=TREE; \
	if (!node) element=NULL; \
	else for (;;) { \
		element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node);

#define EM_AVL_SEARCH_GO_LEFT \
		{ \
			node=node->Left; \
			if (node) continue; \
			element=NULL; \
		}

#define EM_AVL_SEARCH_GO_LEFT_OR_FOUND \
		{ \
			node=node->Left; \
			if (node) continue; \
		}

#define EM_AVL_SEARCH_GO_RIGHT \
		{ \
			node=node->Right; \
			if (node) continue; \
			element=NULL; \
		}

#define EM_AVL_SEARCH_GO_RIGHT_OR_FOUND \
		{ \
			node=node->Right; \
			if (node) continue; \
		}

#define EM_AVL_SEARCH_END \
		break; \
	}

//----------------------- Macros for the loop algorithm ------------------------

#define EM_AVL_LOOP_VARS(ELEMENT_CLASS) \
	const emAvlNode * nstack[64]; \
	const emAvlNode * node; \
	int depth; \
	ELEMENT_CLASS * element;

// - - - loop from first to last - - -

#define EM_AVL_LOOP_START(ELEMENT_CLASS,NODE_MEMBER,TREE) \
	node=TREE; \
	if (!node) element=NULL; \
	else { \
		depth=0; \
		if (node->Left) do { \
			nstack[depth++]=node; \
			node=node->Left; \
		} while (node->Left); \
		for (;;) { \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node);

#define EM_AVL_LOOP_END \
			node=node->Right; \
			if (node) { \
				if (node->Left) do { \
					nstack[depth++]=node; \
					node=node->Left; \
				} while (node->Left); \
				continue; \
			} \
			if (depth>0) { \
				depth--; \
				node=nstack[depth]; \
				continue; \
			} \
			element=NULL; \
			break; \
		} \
	}

// - - - loop from last to first - - -

#define EM_AVL_REV_LOOP_START(ELEMENT_CLASS,NODE_MEMBER,TREE) \
	node=TREE; \
	if (!node) element=NULL; \
	else { \
		depth=0; \
		if (node->Right) do { \
			nstack[depth++]=node; \
			node=node->Right; \
		} while (node->Right); \
		for (;;) { \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node);

#define EM_AVL_REV_LOOP_END \
			node=node->Left; \
			if (node) { \
				if (node->Right) do { \
					nstack[depth++]=node; \
					node=node->Right; \
				} while (node->Right); \
				continue; \
			} \
			if (depth>0) { \
				depth--; \
				node=nstack[depth]; \
				continue; \
			} \
			element=NULL; \
			break; \
		} \
	}

//---------------------- Macros for the iterate algorithm ----------------------

#define EM_AVL_ITER_VARS(ELEMENT_CLASS) \
	const emAvlNode * node; \
	ELEMENT_CLASS * element;


#define EM_AVL_ITER_FIRST(ELEMENT_CLASS,NODE_MEMBER,TREE,ITERATOR) \
	{ \
		node=TREE; \
		ITERATOR.depth=0; \
		if (!node) { \
			ITERATOR.nstack[0]=NULL; \
			element=NULL; \
		} \
		else { \
			if (node->Left) do { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Left; \
			} while (node->Left); \
			ITERATOR.nstack[ITERATOR.depth]=node; \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
		} \
	}

#define EM_AVL_ITER_LAST(ELEMENT_CLASS,NODE_MEMBER,TREE,ITERATOR) \
	{ \
		node=TREE; \
		ITERATOR.depth=0; \
		if (!node) { \
			ITERATOR.nstack[0]=NULL; \
			element=NULL; \
		} \
		else { \
			if (node->Right) do { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Right; \
			} while (node->Right); \
			ITERATOR.nstack[ITERATOR.depth]=node; \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
		} \
	}

#define EM_AVL_ITER_NEXT(ELEMENT_CLASS,NODE_MEMBER,ITERATOR) \
	{ \
		node=ITERATOR.nstack[ITERATOR.depth]; \
		if (node->Right) { \
			ITERATOR.depth++; \
			node=node->Right; \
			if (node->Left) do { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Left; \
			} while (node->Left); \
			ITERATOR.nstack[ITERATOR.depth]=node; \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
		} \
		else if (ITERATOR.depth<=0) { \
			ITERATOR.nstack[ITERATOR.depth]=NULL; \
			element=NULL; \
		} \
		else  { \
			for (;;) { \
				ITERATOR.depth--; \
				if (node==ITERATOR.nstack[ITERATOR.depth]->Right) { \
					node=ITERATOR.nstack[ITERATOR.depth]; \
					if (ITERATOR.depth>0) continue; \
					element=NULL; \
					break; \
				} \
				node=ITERATOR.nstack[ITERATOR.depth]; \
				element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
				break; \
			} \
		} \
	}

#define EM_AVL_ITER_PREV(ELEMENT_CLASS,NODE_MEMBER,ITERATOR) \
	{ \
		node=ITERATOR.nstack[ITERATOR.depth]; \
		if (node->Left) { \
			ITERATOR.depth++; \
			node=node->Left; \
			if (node->Right) do { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Right; \
			} while (node->Right); \
			ITERATOR.nstack[ITERATOR.depth]=node; \
			element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
		} \
		else if (ITERATOR.depth<=0) { \
			ITERATOR.nstack[ITERATOR.depth]=NULL; \
			element=NULL; \
		} \
		else  { \
			for (;;) { \
				ITERATOR.depth--; \
				if (node==ITERATOR.nstack[ITERATOR.depth]->Left) { \
					node=ITERATOR.nstack[ITERATOR.depth]; \
					if (ITERATOR.depth>0) continue; \
					element=NULL; \
					break; \
				} \
				node=ITERATOR.nstack[ITERATOR.depth]; \
				element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node); \
				break; \
			} \
		} \
	}

#define EM_AVL_ITER_START_ANY_BEGIN(ELEMENT_CLASS,NODE_MEMBER,TREE,ITERATOR) \
	node=TREE; \
	ITERATOR.depth=0; \
	if (!node) { \
		ITERATOR.nstack[0]=NULL; \
		element=NULL; \
	} \
	else for (;;) { \
		ITERATOR.nstack[ITERATOR.depth]=node; \
		element=EM_AVL_ELEMENT(ELEMENT_CLASS,NODE_MEMBER,node);

#define EM_AVL_ITER_START_ANY_GO_LEFT(ITERATOR) \
		{ \
			ITERATOR.nstack[ITERATOR.depth++]=node; \
			node=node->Left; \
			if (node) continue; \
			ITERATOR.nstack[ITERATOR.depth]=NULL; \
			element=NULL; \
		}

#define EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(ITERATOR) \
		{ \
			if (node->Left) { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Left; \
				continue; \
			} \
		}

#define EM_AVL_ITER_START_ANY_GO_RIGHT(ITERATOR) \
		{ \
			ITERATOR.nstack[ITERATOR.depth++]=node; \
			node=node->Right; \
			if (node) continue; \
			ITERATOR.nstack[ITERATOR.depth]=NULL; \
			element=NULL; \
		}

#define EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(ITERATOR) \
		{ \
			if (node->Right) { \
				ITERATOR.nstack[ITERATOR.depth++]=node; \
				node=node->Right; \
				continue; \
			} \
		}

#define EM_AVL_ITER_START_ANY_END \
		break; \
	}


//------------ Macros for unions of variable sets of the algorithms ------------
// this is dirty, isn't it?

#define EM_AVL_INS_LOOP_VARS(ELEMENT_CLASS) \
	EM_AVL_INSERT_VARS(ELEMENT_CLASS) \
	const emAvlNode * nstack[64]; \
	const emAvlNode * node;

#define EM_AVL_INS_ITER_VARS(ELEMENT_CLASS) \
	EM_AVL_INSERT_VARS(ELEMENT_CLASS) \
	const emAvlNode * node;

//...to be continued...


//==============================================================================
//================================ emAvlTreeMap ================================
//==============================================================================

template <class KEY, class VALUE> class emAvlTreeMap {

public:

	// Template class for an AVL tree where the elements consist of
	// key/value pairs and are sorted by the keys. emAvlTreeMap has
	// copy-on-write behavior and stable iterators. The template parameter
	// KEY describes the type of the keys, and the template parameter VALUE
	// describes the type of the values. Keys are compared with the normal
	// comparison operators (==, <=, >...).

	emAvlTreeMap();
		// Construct an empty map.

	emAvlTreeMap(const emAvlTreeMap & src);
		// Construct a copied map.

	emAvlTreeMap(const KEY & key, const VALUE & value);
		// Construct a map with one element.

	~emAvlTreeMap();
		// Destructor.

	emAvlTreeMap & operator = (const emAvlTreeMap & map);
		// Make this map a copy of the given map.

	struct Element {
		// Datatype for an element of the map.

		KEY Key;
			// The key of the element.

		VALUE Value;
			// The value of the element.

		emAvlNode AvlNode;
			// Private stuff.
	};

	bool Contains(const KEY & key) const;
		// Ask whether the map contains an element whose key equals the
		// given key.

	const Element * Get(const KEY & key) const;
		// Get a pointer to the element whose key equals the given key.
		// If there is no such element, NULL is returned. At least
		// because of the copy-on-write feature, the pointer is valid
		// only until calling any non-const method or operator on this
		// map, or giving this map as a non-const argument to any call
		// in the world.

	const Element * GetFirst() const;
	const Element * GetLast() const;
		// Get the element with the smallest or largest key. If the map
		// is empty, NULL is returned. The rules for the validity of the
		// pointer are the same as with the Get(key).

	const KEY * GetKey(const KEY & key) const;
	const KEY * GetKey(const Element * elem) const;
		// Get a pointer to the key of an element. If there is no such
		// element, NULL is returned. The rules for the validity of the
		// pointer are the same as with the Get(key).

	KEY * GetKeyWritable(const KEY & key);
	KEY * GetKeyWritable(const Element * elem);
		// Get a non-const version of a pointer to the key of an element.
		// The pointer may be used for modifying the key in a way that
		// the order is not disturbed. The rules for the validity of the
		// pointer are the same as with the GetKey methods, but: The
		// pointer must not be used for modifying after doing something
		// which could have made a shallow copy of this list.

	const VALUE * GetValue(const KEY & key) const;
	const VALUE * GetValue(const Element * elem) const;
		// Get a pointer to the value of an element. If there is no such
		// element, NULL is returned. The rules for the validity of the
		// pointer are the same as with the Get(key).

	VALUE * GetValueWritable(const KEY & key, bool insertIfNew);
	VALUE * GetValueWritable(const Element * elem);
		// Get a non-const version of a pointer to the value of an
		// element. If insertIfNew is true, the element is created if it
		// is not found. The pointer may be used for modifying the
		// value. The rules for the validity of the pointer are the same
		// as with the GetValue methods, but: The pointer must not be
		// used for modifying after doing something which could have
		// made a shallow copy of this list.

	void SetValue(const KEY & key, const VALUE & value, bool insertIfNew);
	void SetValue(const Element * elem, const VALUE & value);
		// Set the value of an element. If insertIfNew is true, the
		// element is created if it is not found.

	const VALUE & operator [] (const KEY & key) const;
	VALUE & operator [] (const KEY & key);
		// Same as *GetValue(key) or *GetValueWritable(key,true).

	void Insert(const KEY & key, const VALUE & value);
		// Same as SetValue(key,value,true).

	void RemoveFirst();
	void RemoveLast();
	void Remove(const KEY & key);
	void Remove(const Element * elem);
		// Remove (and delete) the first element, the last element, the element
		// that matches a key, or a given element. If the element does not exist,
		// nothing is removed.

	void Clear();
		// Remove (and delete) all elements of this map.

	bool IsEmpty() const;
		// Ask whether this map has no elements.

	int GetCount() const;
		// Compute the number of elements.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this map.

	void MakeNonShared();
		// This must be called before handing the map to another thread.
		// This method is not recursive. So if the key or value classes
		// even have such a method, you have to call that manually.

	class Iterator {

	public:

		// Class for a stable pointer to an element of a map.
		// "stable" means:
		// * If the address of an element changes through the
		//   copy-on-write mechanism, iterators pointing to that element
		//   are adapted proper.
		// * If an element is removed from a map,
		//   iterators pointing to that element are set to the next
		//   element, or NULL if it was the last element.
		// * If the assignment operator '=' is called on a map, all
		//   iterators which were pointing to elements of the map are
		//   set to NULL. This is even true if the map is assigned to
		//   itself.
		// This kind of iterator needs little more than 500 bytes of
		// memory, because it manages a stack of tree nodes for the
		// current position (yes, the AVL nodes do not have parent
		// pointers...).
		// Modifying the map while an iterator is active slows down the
		// iterator, because it has to find the tree position again on
		// next increment or decrement.
		// Note the auto-cast operator to a 'const Element *'. Wherever
		// there is an argument 'const Element *' in the methods of
		// emAvlTreeMap, you can even give an instance of this class as
		// the argument.

		Iterator();
			// Construct a "NULL pointer".

		Iterator(const Iterator & iter);
			// Construct a copied iterator.

		Iterator(const emAvlTreeMap<KEY,VALUE> & map, const KEY & key);
		Iterator(const emAvlTreeMap<KEY,VALUE> & map, const Element * elem);
			// Construct an iterator pointing to a particular
			// element.

		~Iterator();
			// Destructor.

		Iterator & operator = (const Iterator & iter);
			// Copy an iterator.

		operator const Element * () const;
		const Element * operator * () const;
		const Element * operator -> () const;
		const Element * Get() const;
			// Get the element pointer. It is NULL if this iterator
			// does not point to any element.

		const Element * Set(const Iterator & iter);
			// Copy the given iterator and return the element
			// pointer.

		const Element * Set(const emAvlTreeMap<KEY,VALUE> & map, const KEY & key);
		const Element * Set(const emAvlTreeMap<KEY,VALUE> & map, const Element * elem);
			// Set this iterator to the given element of the given
			// map and return the element pointer.

		const Element * SetFirst(const emAvlTreeMap<KEY,VALUE> & map);
		const Element * SetLast(const emAvlTreeMap<KEY,VALUE> & map);
			// Set this iterator to the first or last element of the
			// given map and return the element pointer.

		const Element * SetNext();
		const Element * SetPrev();
		const Element * operator ++();
		const Element * operator --();
			// Set this iterator to the next or previous element and
			// return the new element pointer. This must be called
			// only if the old element pointer is not NULL.

		const Element * operator ++(int);
		const Element * operator --(int);
			// Like above, but return the old element pointer.

		bool operator == (const Iterator & iter) const;
		bool operator != (const Iterator & iter) const;
		bool operator == (const Element * elem) const;
		bool operator != (const Element * elem) const;
			// Ordinary compare operators.

		const emAvlTreeMap<KEY,VALUE> * GetMap() const;
			// Get a pointer to the map this iterator is currently
			// attached to. Returns NULL if not attached to any
			// map. (See comments on Detach()).

		void Detach();
			// Detach this iterator from its map and point to NULL.
			// Note: to care about the iterators, each emAvlTreeMap
			// has a single linked list of its iterators. The
			// mechanism is lazy, that means, an iterator may stay
			// in the map even when not pointing to any element,
			// just for quick re-use. On the other hand, such
			// iterators are still costing a tiny number of CPU
			// cycles whenever the map is modified.

	private:
		friend class emAvlTreeMap<KEY,VALUE>;
		void SetMap(const emAvlTreeMap<KEY,VALUE> * map);
		const Element * Pos;
		emAvlIterator AvlIter;
		bool AvlIterValid;
		emAvlTreeMap<KEY,VALUE> * Map;
		Iterator * NextIter; // Undefined if Map==NULL
	};

private:
	friend class Iterator;

	struct SharedData {
		emAvlTree AvlTree;
		bool IsStaticEmpty;
		unsigned int RefCount;
	};

	void MakeWritable(const Element * * preserve=NULL);
	void DeleteData();
	emAvlNode * CloneTree(emAvlNode * tree, const Element * * preserve);
	static void DeleteTree(emAvlNode * tree);

	SharedData * Data;
	Iterator * Iterators;
	static SharedData EmptyData;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

template <class KEY, class VALUE> inline
emAvlTreeMap<KEY,VALUE>::emAvlTreeMap()
{
	Iterators=NULL;
	Data=&EmptyData;
}

template <class KEY, class VALUE> inline
emAvlTreeMap<KEY,VALUE>::emAvlTreeMap(const emAvlTreeMap & src)
{
	Iterators=NULL;
	Data=src.Data;
	Data->RefCount++;
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::emAvlTreeMap(const KEY & key, const VALUE & value)
{
	Iterators=NULL;
	Data=&EmptyData;
	SetValue(key,value,true);
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::~emAvlTreeMap()
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) { i->Pos=NULL; i->Map=NULL; }
	if (!--Data->RefCount) DeleteData();
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE> & emAvlTreeMap<KEY,VALUE>::operator = (
	const emAvlTreeMap & map
)
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	map.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=map.Data;
	return *this;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::Contains(const KEY & key) const
{
	return Get(key)!=NULL;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element * emAvlTreeMap<KEY,VALUE>::Get(
	const KEY & key
) const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		if (key<element->Key) EM_AVL_SEARCH_GO_LEFT
		else if (key>element->Key) EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return element;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::GetFirst() const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		EM_AVL_SEARCH_GO_LEFT_OR_FOUND
	EM_AVL_SEARCH_END
	return element;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::GetLast() const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,AvlNode,Data->AvlTree)
		EM_AVL_SEARCH_GO_RIGHT_OR_FOUND
	EM_AVL_SEARCH_END
	return element;
}

template <class KEY, class VALUE> inline
const KEY * emAvlTreeMap<KEY,VALUE>::GetKey(const KEY & key) const
{
	const Element * elem = Get(key);
	return elem ? &elem->Key : NULL;
}

template <class KEY, class VALUE> inline
const KEY * emAvlTreeMap<KEY,VALUE>::GetKey(const Element * elem) const
{
	return elem ? &elem->Key : NULL;
}

template <class KEY, class VALUE>
KEY * emAvlTreeMap<KEY,VALUE>::GetKeyWritable(const KEY & key)
{
	const Element * elem = Get(key);
	if (!elem) return NULL;
	if (Data->RefCount>1) MakeWritable(&elem);
	return (KEY*)&elem->Key;
}

template <class KEY, class VALUE>
KEY * emAvlTreeMap<KEY,VALUE>::GetKeyWritable(const Element * elem)
{
	if (!elem) return NULL;
	if (Data->RefCount>1) MakeWritable(&elem);
	return (KEY*)&elem->Key;
}

template <class KEY, class VALUE>
inline const VALUE * emAvlTreeMap<KEY,VALUE>::GetValue(const KEY & key) const
{
	const Element * elem = Get(key);
	return elem ? &elem->Value : NULL;
}

template <class KEY, class VALUE>
inline const VALUE * emAvlTreeMap<KEY,VALUE>::GetValue(
	const Element * elem
) const
{
	return elem ? &elem->Value : NULL;
}

template <class KEY, class VALUE>
VALUE * emAvlTreeMap<KEY,VALUE>::GetValueWritable(
	const KEY & key, bool insertIfNew
)
{
	if (insertIfNew) {
		if (Data->RefCount>1 || Data->IsStaticEmpty) MakeWritable();
		EM_AVL_INSERT_VARS(Element)
		EM_AVL_INSERT_BEGIN_SEARCH(Element,AvlNode,Data->AvlTree)
			if (key<element->Key) EM_AVL_INSERT_GO_LEFT
			else if (key>element->Key) EM_AVL_INSERT_GO_RIGHT
			else return &element->Value;
		EM_AVL_INSERT_END_SEARCH
		element=(Element*)malloc(sizeof(Element));
		::new ((void*)(&element->Key)) KEY(key);
		::new ((void*)(&element->Value)) VALUE();
		Iterator * i;
		for (i=Iterators; i; i=i->NextIter) {
			i->AvlIterValid=false;
		}
		EM_AVL_INSERT_NOW(AvlNode)
		return &element->Value;
	}
	else {
		const Element * elem = Get(key);
		if (!elem) return NULL;
		if (Data->RefCount>1) MakeWritable(&elem);
		return (VALUE*)&elem->Value;
	}
}

template <class KEY, class VALUE>
VALUE * emAvlTreeMap<KEY,VALUE>::GetValueWritable(const Element * elem)
{
	if (!elem) return NULL;
	if (Data->RefCount>1) MakeWritable(&elem);
	return (VALUE*)&elem->Value;
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::SetValue(
	const KEY & key, const VALUE & value, bool insertIfNew
)
{
	VALUE * p=GetValueWritable(key,insertIfNew);
	if (p) *p=value;
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::SetValue(
	const Element * elem, const VALUE & value
)
{
	if (elem) *GetValueWritable(elem)=value;
}

template <class KEY, class VALUE> inline
const VALUE & emAvlTreeMap<KEY,VALUE>::operator [] (const KEY & key) const
{
	return *GetValue(key);
}

template <class KEY, class VALUE> inline
VALUE & emAvlTreeMap<KEY,VALUE>::operator [] (const KEY & key)
{
	return *GetValueWritable(key,true);
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::Insert(const KEY & key, const VALUE & value)
{
	SetValue(key,value,true);
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::RemoveFirst()
{
	Remove(GetFirst());
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::RemoveLast()
{
	Remove(GetLast());
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::Remove(const KEY & key)
{
	EM_AVL_REMOVE_VARS(Element)
	Iterator * i;

	if (Data->RefCount>1 && !Data->IsStaticEmpty) MakeWritable();
	EM_AVL_REMOVE_BEGIN(Element,AvlNode,Data->AvlTree)
		if (key<element->Key) {
			EM_AVL_REMOVE_GO_LEFT
		}
		else if (key>element->Key) {
			EM_AVL_REMOVE_GO_RIGHT
		}
		else {
			for (i=Iterators; i; i=i->NextIter) {
				if (i->Pos==element) i->SetNext();
				i->AvlIterValid=false;
			}
			EM_AVL_REMOVE_NOW
			delete element;
		}
	EM_AVL_REMOVE_END
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::Remove(const Element * elem)
{
	if (elem) Remove(elem->Key);
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::Clear()
{
	Iterator * i;

	for (i=Iterators; i; i=i->NextIter) i->Pos=NULL;
	if (!--Data->RefCount) DeleteData();
	Data=&EmptyData;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::IsEmpty() const
{
	return !Data->AvlTree;
}

template <class KEY, class VALUE>
int emAvlTreeMap<KEY,VALUE>::GetCount() const
{
	EM_AVL_LOOP_VARS(Element)
	int count;

	count=0;
	EM_AVL_LOOP_START(Element,AvlNode,Data->AvlTree)
		count++;
	EM_AVL_LOOP_END
	return count;
}

template <class KEY, class VALUE>
unsigned int emAvlTreeMap<KEY,VALUE>::GetDataRefCount() const
{
	return Data->IsStaticEmpty ? UINT_MAX/2 : Data->RefCount;
}

template <class KEY, class VALUE> inline
void emAvlTreeMap<KEY,VALUE>::MakeNonShared()
{
	MakeWritable();
}

template <class KEY, class VALUE> inline
emAvlTreeMap<KEY,VALUE>::Iterator::Iterator()
{
	Pos=NULL;
	Map=NULL;
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::Iterator::Iterator(
	const typename emAvlTreeMap<KEY,VALUE>::Iterator & iter
)
{
	Pos=NULL;
	Map=NULL;
	Set(iter);
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::Iterator::Iterator(
	const emAvlTreeMap<KEY,VALUE> & map, const KEY & key
)
{
	Pos=NULL;
	Map=NULL;
	Set(map,key);
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::Iterator::Iterator(
	const emAvlTreeMap<KEY,VALUE> & map,
	const typename emAvlTreeMap<KEY,VALUE>::Element * elem
)
{
	Pos=NULL;
	Map=NULL;
	Set(map,elem);
}

template <class KEY, class VALUE>
emAvlTreeMap<KEY,VALUE>::Iterator::~Iterator()
{
	Iterator * * pi;

	if (Map) {
		for (pi=&Map->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
	}
}

template <class KEY, class VALUE> inline
typename emAvlTreeMap<KEY,VALUE>::Iterator &
	emAvlTreeMap<KEY,VALUE>::Iterator::operator = (const Iterator & iter)
{
	Set(iter);
	return *this;
}

template <class KEY, class VALUE> inline
emAvlTreeMap<KEY,VALUE>::Iterator::operator const typename emAvlTreeMap<KEY,VALUE>::Element * () const
{
	return Pos;
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator * () const
{
	return Pos;
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator -> () const
{
	return Pos;
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::Get() const
{
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::Set(
	const Iterator & iter
)
{
	SetMap(iter.Map);
	if (Pos!=iter.Pos) {
		AvlIterValid=false;
		Pos=iter.Pos;
	}
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::Set(
	const emAvlTreeMap<KEY,VALUE> & map, const KEY & key
)
{
	EM_AVL_ITER_VARS(Element)

	SetMap(&map);
	EM_AVL_ITER_START_ANY_BEGIN(Element,AvlNode,Map->Data->AvlTree,AvlIter)
		if (key<element->Key) EM_AVL_ITER_START_ANY_GO_LEFT(AvlIter)
		else if (key>element->Key) EM_AVL_ITER_START_ANY_GO_RIGHT(AvlIter)
	EM_AVL_ITER_START_ANY_END
	AvlIterValid=true;
	Pos=element;
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::Set(
	const emAvlTreeMap<KEY,VALUE> & map, const Element * elem
)
{
	SetMap(&map);
	if (Pos!=elem) {
		AvlIterValid=false;
		Pos=elem;
	}
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::SetFirst(
	const emAvlTreeMap<KEY,VALUE> & map
)
{
	EM_AVL_ITER_VARS(Element)

	SetMap(&map);
	EM_AVL_ITER_FIRST(Element,AvlNode,Map->Data->AvlTree,AvlIter)
	AvlIterValid=true;
	Pos=element;
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::SetLast(
	const emAvlTreeMap<KEY,VALUE> & map
)
{
	EM_AVL_ITER_VARS(Element)

	SetMap(&map);
	EM_AVL_ITER_LAST(Element,AvlNode,Map->Data->AvlTree,AvlIter)
	AvlIterValid=true;
	Pos=element;
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::SetNext()
{
	EM_AVL_ITER_VARS(Element)

	if (Pos) {
		if (!AvlIterValid) Set(*Map,Pos->Key);
		EM_AVL_ITER_NEXT(Element,AvlNode,AvlIter)
		Pos=element;
	}
	return Pos;
}

template <class KEY, class VALUE>
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::SetPrev()
{
	EM_AVL_ITER_VARS(Element)

	if (Pos) {
		if (!AvlIterValid) Set(*Map,Pos->Key);
		EM_AVL_ITER_PREV(Element,AvlNode,AvlIter)
		Pos=element;
	}
	return Pos;
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator ++()
{
	return SetNext();
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator --()
{
	return SetPrev();
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator ++(int)
{
	const Element * res=Pos;
	SetNext();
	return res;
}

template <class KEY, class VALUE> inline
const typename emAvlTreeMap<KEY,VALUE>::Element *
emAvlTreeMap<KEY,VALUE>::Iterator::operator --(int)
{
	const Element * res=Pos;
	SetPrev();
	return res;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::Iterator::operator == (
	const Iterator & iter
) const
{
	return Pos==iter.Pos;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::Iterator::operator != (
	const Iterator & iter
) const
{
	return Pos!=iter.Pos;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::Iterator::operator == (
	const Element * elem
) const
{
	return Pos==elem;
}

template <class KEY, class VALUE> inline
bool emAvlTreeMap<KEY,VALUE>::Iterator::operator != (
	const Element * elem
) const
{
	return Pos!=elem;
}

template <class KEY, class VALUE> inline
const emAvlTreeMap<KEY,VALUE> *
emAvlTreeMap<KEY,VALUE>::Iterator::GetMap() const
{
	return Map;
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::Iterator::Detach()
{
	Iterator * * pi;

	if (Map) {
		for (pi=&Map->Iterators; *pi!=this; pi=&(*pi)->NextIter);
		*pi=NextIter;
		Map=NULL;
		Pos=NULL;
	}
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::Iterator::SetMap(
	const emAvlTreeMap<KEY,VALUE> * map
)
{
	Iterator * * pi;

	if (Map!=map) {
		if (Map) {
			for (pi=&Map->Iterators; *pi!=this; pi=&(*pi)->NextIter);
			*pi=NextIter;
		}
		Map=(emAvlTreeMap<KEY,VALUE>*)map;
		if (Map) {
			NextIter=Map->Iterators;
			Map->Iterators=this;
		}
	}
}

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::MakeWritable(const Element * * preserve)
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

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::DeleteData()
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

template <class KEY, class VALUE>
emAvlNode * emAvlTreeMap<KEY,VALUE>::CloneTree(
	emAvlNode * tree, const Element * * preserve
)
{
	Element * e1, * e2;
	Iterator * i;

	e1=EM_AVL_ELEMENT(Element,AvlNode,tree);
	e2=new Element(*e1);
	if (preserve && *preserve==e1) *preserve=e2;
	for (i=Iterators; i; i=i->NextIter) {
		if (i->Pos==e1) {
			i->Pos=e2;
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

template <class KEY, class VALUE>
void emAvlTreeMap<KEY,VALUE>::DeleteTree(emAvlNode * tree)
{
	Element * element=EM_AVL_ELEMENT(Element,AvlNode,tree);
	if (element->AvlNode.Left ) DeleteTree(element->AvlNode.Left );
	if (element->AvlNode.Right) DeleteTree(element->AvlNode.Right);
	delete element;
}

template <class KEY, class VALUE>
typename emAvlTreeMap<KEY,VALUE>::SharedData emAvlTreeMap<KEY,VALUE>::EmptyData=
{
	NULL,true,UINT_MAX/2
};


#endif
