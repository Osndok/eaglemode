//------------------------------------------------------------------------------
// emContext.h
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

#ifndef emContext_h
#define emContext_h

#ifndef emAvlTree_h
#include <emCore/emAvlTree.h>
#endif

#ifndef emCrossPtr_h
#include <emCore/emCrossPtr.h>
#endif

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

class emModel;
class emRootContext;


//==============================================================================
//================================= emContext ==================================
//==============================================================================

class emContext : public emEngine {

public:

	// Class for a context of models (read comments on emModel). It manages
	// an AVL tree for quickly refinding common models.

	emContext(emContext & parentContext);
		// Construct a child context. For constructing a root context
		// please see emRootContext.

	virtual ~emContext();
		// Destructor. This even deletes all common models in this
		// context, in an order respecting references between the
		// models. Child contexts and private models are not deleted by
		// this. They must have been destructed before or they must be
		// managed by common models.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// This means emCrossPtr<emContext> is possible.

	emRootContext & GetRootContext();
		// Get the root context.

	emContext * GetParentContext();
		// Get the parent context. Returns NULL if this is the root
		// context.

	emContext * GetFirstChildContext();
	emContext * GetLastChildContext();
	emContext * GetPrevContext();
	emContext * GetNextContext();
		// Get the first or last child context, or the previous or next
		// brother context, NULL if none.

	emModel * Lookup(const type_info & modelClass, const char * name);
		// Search for a common model within this context.
		// Arguments:
		//   modelClass - Final class of the model.
		//   name       - Name of the model.
		// Returns: The model, or NULL if not found.

	emModel * LookupInherited(const type_info & modelClass,
	                          const char * name);
		// Like Lookup, but if the model is not found in this context,
		// the parent context is searched, then the grad-parent, and so
		// on.

	emString GetListing() const;
		// Just for debugging: Get a listing of all common models.

	void GetModelInfo(int * pCommonCount, int * pPrivateCount=NULL,
	                  emModel * * * pArrayOfCommon=NULL);
		// Just for debugging: Get the number of common and private
		// models, and create an array of pointers to all common models.
		// The array must be deleted by the caller.

protected:

	virtual bool Cycle();
		// emContext has been derived from emEngine for convenience.
		// This default implementation does nothing and returns false.

private:

	friend class emModel;
	friend class emRootContext;

	emContext(emScheduler & scheduler);

	void RegisterModel(emModel * model);
	void UnregisterModel(emModel * model);
	static int CalcHashCode(const type_info & modelClass,
	                        const char * name);
	emModel * SearchUnused();
	emModel * SearchUnused(int minHash);
	emModel * SearchGarbage();
	emModel * SearchGarbage(int minHash);
	void CollectGarbage();

	class SharedTimingEngine : public emEngine {
	public:
		SharedTimingEngine(emRootContext & rootContext,
		                   unsigned int gcPeriod);
		virtual ~SharedTimingEngine();
		virtual bool Cycle();
		emRootContext & RootContext;
		emTimer SecsTimer;
		unsigned int GCPeriod;
		unsigned int SecsCounter;
		unsigned int TimeOfGC;
	};

	friend class SharedTimingEngine;

	emRootContext & RootContext;
	emCrossPtrList CrossPtrList;
	SharedTimingEngine * SharedTiming;
	emContext * ParentContext;
	emContext * FirstChildContext;
	emContext * LastChildContext;
	emContext * PrevContext;
	emContext * NextContext;
	emAvlTree AvlTree;
	unsigned int ModelCount;
	bool DoGCOnModels;
};


//==============================================================================
//=============================== emRootContext ================================
//==============================================================================

class emRootContext : public emContext {

public:

	emRootContext(emScheduler & scheduler);
		// Construct a root context.

};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline void emContext::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline emRootContext & emContext::GetRootContext()
{
	return RootContext;
}

inline emContext * emContext::GetParentContext()
{
	return ParentContext;
}

inline emContext * emContext::GetFirstChildContext()
{
	return FirstChildContext;
}

inline emContext * emContext::GetLastChildContext()
{
	return LastChildContext;
}

inline emContext * emContext::GetPrevContext()
{
	return PrevContext;
}

inline emContext * emContext::GetNextContext()
{
	return NextContext;
}

inline emRootContext::emRootContext(emScheduler & scheduler)
	: emContext(scheduler)
{
}


#endif
