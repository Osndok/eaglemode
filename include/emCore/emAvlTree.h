//------------------------------------------------------------------------------
// emAvlTree.h
//
// Copyright (C) 2005-2008,2010,2014-2016 Oliver Hamann.
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
// tree implementation. For an example of how to use it, please see the
// implementation of emAvlTreeMap or emAvlTreeSet.

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


#endif
