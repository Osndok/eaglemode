//------------------------------------------------------------------------------
// emModel.h
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

#ifndef emModel_h
#define emModel_h

#ifndef emRef_h
#include <emCore/emRef.h>
#endif

#ifndef emContext_h
#include <emCore/emContext.h>
#endif


//==============================================================================
//================================== emModel ===================================
//==============================================================================

class emModel : public emEngine {

public:

	// emModel is a base class for shared data and logics.
	//
	// Model references
	// ----------------
	// Models must be referred through instances of the template class
	// emRef. A model deletes itself automatically as soon as there are no
	// remaining references. It is even possible to have weak references
	// by making use of emCrossPtr.
	//
	// Model context
	// -------------
	// Each model exists within a context (see emContext, emRootContext and
	// emView). Contexts are used for grouping and refinding models. They
	// are making up a tree. Models in a root context are something like
	// "global variables".
	//
	// Common models
	// -------------
	// Models can be registered within their context so that they can be
	// refound. Registered models are called common models. The identity of
	// a common model must be unique and consists of:
	//   - The final class of the model (determined through the RTTI
	//     function typeid).
	//   - The context of the model.
	//   - The name of the model.
	// The name is a character string whose meaning is defined by derived
	// model classes. It may even be generated from model specific
	// arguments.
	//
	// Hint: a typedef does not change the result of typeid() and therefore
	// it has no effect on the identity of a model. This should not be
	// forgotten when doing something like:
	//   typedef emVarModel<int> MyIntModel;
	// Best is not to typedef models. Make derivatives instead.
	//
	// Private models
	// --------------
	// Models which are not registered for refinding, are called private
	// models. They do not need to have a unique identification and they are
	// referring a context just for convenience.
	//
	// Lifetime of common models
	// -------------------------
	// Common models can live longer than their user references. The method
	// SetMinCommonLifetime allows to set a minimum time for which the
	// context should hold the model after all other references have gone.
	// Thereby, models can easily be designed for being caches, resources,
	// configurations and so on.
	//
	// Get or create a model: the Acquire function
	// -------------------------------------------
	// Each derived model class which is not just a base class must have a
	// static method named Acquire. That function returns a reference to an
	// existing or newly created model. Constructors and destructors of
	// models are never to be called by users and should always be
	// protected.

	static emRef<emModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);
		// This is an example for the Acquire function (it does not make
		// sense here because this is just a base class). The behavior
		// should always be: If common==true, search for an already
		// registered model of the same class, context and name, and if
		// it is found, return a reference to that model. Otherwise
		// create a new model, register it if common==true, and return a
		// reference to it.
		// Arguments:
		//   context - The context of the model
		//   name    - The name of the model.
		//   common  - true for refinding or creating a common model,
		//             false for creating a private model.
		// Returns: A reference to the found or created model.

	// The following two macros can be used to implement Acquire functions.
	// The first macro is for the general case, and the second is for
	// classes of models which are always common.
#	define EM_IMPL_ACQUIRE(CLASS, CONTEXT, NAME, COMMON) \
		CLASS * m; \
		if (!COMMON) m=new CLASS(CONTEXT,NAME); \
		else { \
			m=(CLASS*)CONTEXT.Lookup(typeid(CLASS),NAME); \
			if (!m) { m=new CLASS(CONTEXT,NAME); m->Register(); } \
		} \
		return emRef<CLASS >(m);
#	define EM_IMPL_ACQUIRE_COMMON(CLASS, CONTEXT, NAME) \
		CLASS * m; \
		m=(CLASS*)CONTEXT.Lookup(typeid(CLASS),NAME); \
		if (!m) { m=new CLASS(CONTEXT,NAME); m->Register(); } \
		return emRef<CLASS >(m);

	const emRootContext & GetRootContext() const;
	emRootContext & GetRootContext();
		// Get the root context.

	const emContext & GetContext() const;
	emContext & GetContext();
		// Get the context of this model.

	const emString & GetName() const;
		// Get the name of this model.

	bool IsCommon() const;
		// Ask whether this is a common model.

	bool IsRegistered() const;
		// Just a synonym for IsCommon().

	unsigned GetMinCommonLifetime() const;
	void SetMinCommonLifetime(unsigned seconds);
		// Minimum lifetime after the common model is no longer in use.
		// Zero means this model is deleted immediately when there are
		// no other references than the context. Everything greater
		// UINT_MAX/2 (or (int)seconds<0) means infinite (lifetime of
		// context). The default is zero. For private models, this
		// parameter has no meaning. Normally, SetMinCommonLifetime
		// should be called only by model constructors, not by model
		// users. Or lets say user should not shorten the lifetime.

	void Alloc();
	void Free();
		// Increment or decrement the reference counter. Free() deletes
		// this model if the reference counter gets zero. These methods
		// are usually only to be called by emRef and emContext.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// This means emCrossPtr<emModel> is possible.

protected:

	emModel(emContext & context, const emString & name);
		// Constructor.
		// Arguments:
		//   context - The context for this model
		//   name    - The name for this model.

	virtual ~emModel();
		// Destructor.

	void Register();
	void Unregister();
		// Register or unregister this model in its context.
		// IMPORTANT:
		//  - These methods must never be called from constructors or
		//    destructors of the model (directly or indirectly), because
		//    the final class is part of the registration identity, and
		//    typeid(this) does not return the final class while
		//    constructing or destructing a base class.
		//  - There must not be more than one registered model of the
		//    same class, context and name.
		//  - Usually, Register() should only be called by the Acquire
		//    functions, and Unregister() should never be called by any
		//    derivatives.

	virtual bool Cycle();
		// emModel has been derived from emEngine for convenience. This
		// default implementation does nothing and returns false.

private: friend class emContext;

	emContext & Context;
		// The context of this model.

	emString Name;
		// The name of this model.

	emCrossPtrList CrossPtrList;
		// List of cross pointers to this model.

	emAvlNode AvlNode;
		// Node for this model in the AVL tree of common models of the
		// context.

	int AvlHashCode;
		// Hash index for this model in the AVL tree of the context.
		// Or zero if this is not a common model.

	int RefCount;
		// Number of references to this model (including the context
		// when this is a common model).

	unsigned MinCommonLifetime;
		// If this is a common model: Number of seconds the context
		// should keep this model alive when there are no other
		// references. (int)MinCommonLifetime<0 means infinite.

	unsigned TimeOfDeath;
		// If this is a common model which is only referred by the
		// context: Planed time of death (in units of
		// Context.SharedTiming->SecsCounter)
};

inline const emRootContext & emModel::GetRootContext() const
{
	return Context.RootContext;
}

inline emRootContext & emModel::GetRootContext()
{
	return Context.RootContext;
}

inline const emContext & emModel::GetContext() const
{
	return Context;
}

inline emContext & emModel::GetContext()
{
	return Context;
}

inline const emString & emModel::GetName() const
{
	return Name;
}

inline bool emModel::IsCommon() const
{
	return AvlHashCode!=0;
}

inline bool emModel::IsRegistered() const
{
	return AvlHashCode!=0;
}

inline unsigned emModel::GetMinCommonLifetime() const
{
	return MinCommonLifetime;
}

inline void emModel::Alloc()
{
	RefCount++;
}

inline void emModel::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline void emModel::Register()
{
	Context.RegisterModel(this);
}

inline void emModel::Unregister()
{
	Context.UnregisterModel(this);
}


#endif
