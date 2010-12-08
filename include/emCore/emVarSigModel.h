//------------------------------------------------------------------------------
// emVarSigModel.h
//
// Copyright (C) 2007-2008,2010 Oliver Hamann.
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

#ifndef emVarSigModel_h
#define emVarSigModel_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//=============================== emVarSigModel ================================
//==============================================================================

template <class VAR> class emVarSigModel : public emModel {

public:

	// This is a combination of emVarModel and emSigModel (read them first).
	// The idea is to signal the signal whenever the variable is changed.

	static emRef<emVarSigModel<VAR> > Acquire(
		emContext & context, const emString & name, bool common=true
	);
		// Acquire an emVarSigModel.

	static emRef<emVarSigModel<VAR> > Lookup(
		emContext & context, const char * name
	);
		// Search a common emVarSigModel.

	static emRef<emVarSigModel<VAR> > LookupInherited(
		emContext & context, const char * name
	);
		// Search a common emVarSigModel even in ancestor contexts.

	static void Set(emContext & context, const emString & name,
	                const VAR & value, unsigned minLifetime);
		// Acquire a common emVarSigModel, set its variable to the given
		// value, signal the signal and call SetMinCommonLifetime with
		// the given minLifeTime.

	static void Remove(emContext & context, const char * name);
		// Remove a common emVarSigModel if it exits. Removing means to
		// make it a private model, for that it will be deleted as soon
		// as there are no references.

	static VAR Get(emContext & context, const char * name,
	               const VAR & defaultValue);
		// Get the value of the variable of a common emVarSigModel. If
		// the model does not exist, the given defaultValue is returned.

	static VAR GetAndRemove(emContext & context, const char * name,
	                        const VAR & defaultValue);
		// Like Get plus Remove.

	static VAR GetInherited(emContext & context, const char * name,
	                        const VAR & defaultValue);
		// Like Get, but the model is even searched in ancestor
		// contexts.

	VAR Var;
		// The variable. Remember to signal the signal on every change.

	emSignal Sig;
		// The signal.

protected:
	emVarSigModel(emContext & context, const emString & name);
};


template <class VAR> emRef<emVarSigModel<VAR> > emVarSigModel<VAR>::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emVarSigModel,context,name,common)
}


template <class VAR> emRef<emVarSigModel<VAR> > emVarSigModel<VAR>::Lookup(
	emContext & context, const char * name
)
{
	return emRef<emVarSigModel>(
		(emVarSigModel*)context.Lookup(typeid(emVarSigModel),name)
	);
}


template <class VAR> emRef<emVarSigModel<VAR> > emVarSigModel<VAR>::LookupInherited(
	emContext & context, const char * name
)
{
	return emRef<emVarSigModel>(
		(emVarSigModel*)context.LookupInherited(typeid(emVarSigModel),name)
	);
}


template <class VAR> void emVarSigModel<VAR>::Set(
	emContext & context, const emString & name, const VAR & value,
	unsigned minLifetime
)
{
	emRef<emVarSigModel> m;

	m=Acquire(context,name);
	m->Var=value;
	m->Signal(m->Sig);
	m->SetMinCommonLifetime(minLifetime);
}


template <class VAR> void emVarSigModel<VAR>::Remove(
	emContext & context, const char * name
)
{
	emRef<emVarSigModel> m;

	m=Lookup(context,name);
	if (m) m->Unregister();
}


template <class VAR> VAR emVarSigModel<VAR>::Get(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarSigModel> m;

	m=Lookup(context,name);
	if (!m) return defaultValue;
	return m->Var;
}


template <class VAR> VAR emVarSigModel<VAR>::GetAndRemove(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarSigModel> m;

	m=Lookup(context,name);
	if (!m) return defaultValue;
	m->Unregister();
	return m->Var;
}


template <class VAR> VAR emVarSigModel<VAR>::GetInherited(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarSigModel> m;

	m=LookupInherited(context,name);
	if (!m) return defaultValue;
	return m->Var;
}


template <class VAR> emVarSigModel<VAR>::emVarSigModel(
	emContext & context, const emString & name
)
	: emModel(context,name)
{
}


#endif
