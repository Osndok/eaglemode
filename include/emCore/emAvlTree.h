//------------------------------------------------------------------------------
// emAvlTree.h
//
// Copyright (C) 2005-2008,2010 Oliver Hamann.
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

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//============================== AVL-tree macros ===============================
//==============================================================================

// Here you can find data types and macro definitions for a highly optimized AVL
// tree implementation. For an example of how to use it, see the implementation
// of the template class emAvlTreeExample more below.

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
//============================== emAvlTreeExample ==============================
//==============================================================================

template <class OBJ> class emAvlTreeExample {
public:
	class Iterator;
private:
	friend class Iterator;
	struct Element {
		OBJ Object;
		emAvlNode Node;
	};
	emAvlTree Tree;
public:

	// Template class for a sorted set of objects managed in an AVL tree.
	// The objects are sorted by the normal compare operators.
	//
	// This tree class is meant as a programming example and should not be
	// used - it may be removed one day.
	//
	//??? There are plans to define a better tree class which has the
	//??? following features:
	//???
	//???   * User-defined compare function.
	//???   * Nice interface for creating maps and sets.
	//???   * Copy-on-write mechanism.
	//???   * Both: stable iterators and fast unstable iterators.
	//???
	//??? The last feature would require to have parent pointers in the tree
	//??? nodes. But the EM_AVL macros do not support such parent pointers,
	//??? due to best performance in their original use (e.g. in emContext).
	//??? Therefore, an additional set of tree macros has to be developed
	//??? first - with parent pointers. Maybe it should be made with
	//??? Red/Black trees instead of AVL. Red/Black trees are faster in
	//??? removing elements for the cost of a possible worse balance.

	emAvlTreeExample();
	~emAvlTreeExample();

	bool Insert(const OBJ & object);
		// Insert a copy of the given object. If there is already an
		// object which equals the given object, nothing is changed and
		// false is returned.

	bool Remove(const OBJ & object);
		// Remove the object which equals the given object. If there is
		// no such object, nothing is changed and false is returned.

	void Empty();
		// Remove all objects.

	const OBJ * Search(const OBJ & object) const;
		// Search for the object which equals the given object, and
		// return a pointer to the found object. If there is no such
		// object, NULL is returned.

	int Count() const;
		// Count the number of objects.

	int Check() const;
		// Check consistency of the tree and return its height. Exits
		// this process and prints a message on failure.

	class Iterator {

	public:

		// IMPORTANT:
		//  - After construction, one of the Start methods has to be
		//    called before calling Next, Prev or Get.
		//  - If the tree changes while iterating (inserting/removing
		//    objects), the iterators have to be restarted!
		//  - When Get() returns NULL, the end has been reached. Next
		//    and Prev must not be called then.

		const OBJ * StartFirst(const emAvlTreeExample & treeSet);
		const OBJ * StartLast(const emAvlTreeExample & treeSet);
			// Start with the first or last object of the given
			// tree. Return that object or NULL.

		const OBJ * StartEqual(
			const emAvlTreeExample & treeSet, const OBJ & object
		);
		const OBJ * StartGreater(
			const emAvlTreeExample & treeSet, const OBJ & object
		);
		const OBJ * StartGreaterOrEqual(
			const emAvlTreeExample & treeSet, const OBJ & object
		);
		const OBJ * StartLess(
			const emAvlTreeExample & treeSet, const OBJ & object
		);
		const OBJ * StartLessOrEqual(
			const emAvlTreeExample & treeSet, const OBJ & object
		);
			// Start with an object which is equal, greater, not
			// less, less, or not greater than the given object.
			// Return that object, or NULL if not found.

		const OBJ * Get() const;
			// Get the current object, or NULL...

		const OBJ * Next();
		const OBJ * Prev();
			// Go to the next or previous object. Return that object
			// or NULL.

	private:
		typedef Element TreeElement;
		const OBJ * Current;
		emAvlIterator Iter;
	};
};

template <class OBJ> inline emAvlTreeExample<OBJ>::emAvlTreeExample()
{
	Tree=NULL;
}

template <class OBJ> inline emAvlTreeExample<OBJ>::~emAvlTreeExample()
{
	Empty();
}

template <class OBJ> bool emAvlTreeExample<OBJ>::Insert(const OBJ & object)
{
	EM_AVL_INSERT_VARS(Element)

	EM_AVL_INSERT_BEGIN_SEARCH(Element,Node,Tree)
		if (object<element->Object) EM_AVL_INSERT_GO_LEFT
		else if (object==element->Object) return false;
		else EM_AVL_INSERT_GO_RIGHT
	EM_AVL_INSERT_END_SEARCH
		element=new Element;
		element->Object=object;
	EM_AVL_INSERT_NOW(Node)
	return true;
}

template <class OBJ> bool emAvlTreeExample<OBJ>::Remove(const OBJ & object)
{
	EM_AVL_REMOVE_VARS(Element)

	EM_AVL_REMOVE_BEGIN(Element,Node,Tree)
		if (object<element->Object) {
			EM_AVL_REMOVE_GO_LEFT
		}
		else if (object==element->Object) {
			EM_AVL_REMOVE_NOW
			delete element;
			return true;
		}
		else {
			EM_AVL_REMOVE_GO_RIGHT
		}
	EM_AVL_REMOVE_END
	return false;
}

template <class OBJ> void emAvlTreeExample<OBJ>::Empty()
{
	while (Tree) {
		Remove(EM_AVL_ELEMENT(Element,Node,Tree)->Object);
	}
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Search(
	const OBJ & object
) const
{
	EM_AVL_SEARCH_VARS(Element)

	EM_AVL_SEARCH_BEGIN(Element,Node,Tree)
		if (object<element->Object) EM_AVL_SEARCH_GO_LEFT
		else if (object==element->Object) return &element->Object;
		else EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return NULL;
}

template <class OBJ> int emAvlTreeExample<OBJ>::Count() const
{
	EM_AVL_LOOP_VARS(Element)
	int count;

	count=0;
	EM_AVL_LOOP_START(Element,Node,Tree)
		count++;
	EM_AVL_LOOP_END
	return count;
}

template <class OBJ> inline int emAvlTreeExample<OBJ>::Check() const
{
	return emAvlCheck(Tree);
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartFirst(
	const emAvlTreeExample<OBJ> & treeSet
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_FIRST(TreeElement,Node,treeSet.Tree,Iter)
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartLast(
	const emAvlTreeExample<OBJ> & treeSet
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_LAST(TreeElement,Node,treeSet.Tree,Iter)
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartEqual(
	const emAvlTreeExample<OBJ> & treeSet, const OBJ & object
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_START_ANY_BEGIN(TreeElement,Node,treeSet.Tree,Iter)
		if (object<element->Object) EM_AVL_ITER_START_ANY_GO_LEFT(Iter)
		else if (object>element->Object) EM_AVL_ITER_START_ANY_GO_RIGHT(Iter)
	EM_AVL_ITER_START_ANY_END
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartGreater(
	const emAvlTreeExample<OBJ> & treeSet, const OBJ & object
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_START_ANY_BEGIN(TreeElement,Node,treeSet.Tree,Iter)
		if (object<element->Object) EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(Iter)
		else if (object>element->Object) EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(Iter)
	EM_AVL_ITER_START_ANY_END
	if (element && object>=element->Object) {
		EM_AVL_ITER_NEXT(TreeElement,Node,Iter)
	}
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartGreaterOrEqual(
	const emAvlTreeExample<OBJ> & treeSet, const OBJ & object
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_START_ANY_BEGIN(TreeElement,Node,treeSet.Tree,Iter)
		if (object<element->Object) EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(Iter)
		else if (object>element->Object) EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(Iter)
	EM_AVL_ITER_START_ANY_END
	if (element && object>element->Object) {
		EM_AVL_ITER_NEXT(TreeElement,Node,Iter)
	}
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartLess(
	const emAvlTreeExample<OBJ> & treeSet, const OBJ & object
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_START_ANY_BEGIN(TreeElement,Node,treeSet.Tree,Iter)
		if (object<element->Object) EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(Iter)
		else if (object>element->Object) EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(Iter)
	EM_AVL_ITER_START_ANY_END
	if (element && object<=element->Object) {
		EM_AVL_ITER_PREV(TreeElement,Node,Iter)
	}
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::StartLessOrEqual(
	const emAvlTreeExample<OBJ> & treeSet, const OBJ & object
)
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_START_ANY_BEGIN(TreeElement,Node,treeSet.Tree,Iter)
		if (object<element->Object) EM_AVL_ITER_START_ANY_GO_LEFT_OR_FOUND(Iter)
		else if (object>element->Object) EM_AVL_ITER_START_ANY_GO_RIGHT_OR_FOUND(Iter)
	EM_AVL_ITER_START_ANY_END
	if (element && object<element->Object) {
		EM_AVL_ITER_PREV(TreeElement,Node,Iter)
	}
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> inline const OBJ * emAvlTreeExample<OBJ>::Iterator::Get() const
{
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::Next()
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_NEXT(TreeElement,Node,Iter)
	Current= element ? &element->Object : NULL;
	return Current;
}

template <class OBJ> const OBJ * emAvlTreeExample<OBJ>::Iterator::Prev()
{
	EM_AVL_ITER_VARS(TreeElement)

	EM_AVL_ITER_PREV(TreeElement,Node,Iter)
	Current= element ? &element->Object : NULL;
	return Current;
}


#endif
