//------------------------------------------------------------------------------
// emVarModel.h
//
// Copyright (C) 2006-2008,2010 Oliver Hamann.
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

#ifndef emVarModel_h
#define emVarModel_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//================================= emVarModel =================================
//==============================================================================

template <class VAR> class emVarModel : public emModel {

public:

	// Powerful template class for a model which holds a single variable.
	// The template parameter VAR denotes the type of the variable. Because
	// the model class could be the same in different applications, there
	// is an increased danger of model identification conflicts. So, if the
	// model is common and if VAR is not an application defined class,
	// please invent a smart model name. Best is to have the name of the
	// application's C++ class at the beginning of the model name (e.g.
	// "acmPacman::Speed").

	static emRef<emVarModel<VAR> > Acquire(
		emContext & context, const emString & name, bool common=true
	);
		// Acquire an emVarModel.

	static emRef<emVarModel<VAR> > Lookup(
		emContext & context, const char * name
	);
		// Search a common emVarModel.

	static emRef<emVarModel<VAR> > LookupInherited(
		emContext & context, const char * name
	);
		// Search a common emVarModel even in ancestor contexts.

	static void Set(emContext & context, const emString & name,
	                const VAR & value, unsigned minLifetime);
		// Acquire a common emVarModel, set its variable to the given
		// value and call SetMinCommonLifetime with the given
		// minLifeTime.

	static void Remove(emContext & context, const char * name);
		// Remove a common emVarModel if it exits. Removing means to
		// make it a private model, for that it will be deleted as soon
		// as there are no references.

	static VAR Get(emContext & context, const char * name,
	               const VAR & defaultValue);
		// Get the value of the variable of a common emVarModel. If the
		// model does not exist, the given defaultValue is returned.

	static VAR GetAndRemove(emContext & context, const char * name,
	                        const VAR & defaultValue);
		// Like Get plus Remove.

	static VAR GetInherited(emContext & context, const char * name,
	                        const VAR & defaultValue);
		// Like Get, but the model is even searched in ancestor
		// contexts.

	VAR Var;
		// The variable.

protected:
	emVarModel(emContext & context, const emString & name);
};


template <class VAR> emRef<emVarModel<VAR> > emVarModel<VAR>::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emVarModel,context,name,common)
}


template <class VAR> emRef<emVarModel<VAR> > emVarModel<VAR>::Lookup(
	emContext & context, const char * name
)
{
	return emRef<emVarModel>(
		(emVarModel*)context.Lookup(typeid(emVarModel),name)
	);
}


template <class VAR> emRef<emVarModel<VAR> > emVarModel<VAR>::LookupInherited(
	emContext & context, const char * name
)
{
	return emRef<emVarModel>(
		(emVarModel*)context.LookupInherited(typeid(emVarModel),name)
	);
}


template <class VAR> void emVarModel<VAR>::Set(
	emContext & context, const emString & name, const VAR & value,
	unsigned minLifetime
)
{
	emRef<emVarModel> m;

	m=Acquire(context,name);
	m->Var=value;
	m->SetMinCommonLifetime(minLifetime);
}


template <class VAR> void emVarModel<VAR>::Remove(
	emContext & context, const char * name
)
{
	emRef<emVarModel> m;

	m=Lookup(context,name);
	if (m) m->Unregister();
}


template <class VAR> VAR emVarModel<VAR>::Get(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarModel> m;

	m=Lookup(context,name);
	if (!m) return defaultValue;
	return m->Var;
}


template <class VAR> VAR emVarModel<VAR>::GetAndRemove(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarModel> m;

	m=Lookup(context,name);
	if (!m) return defaultValue;
	m->Unregister();
	return m->Var;
}


template <class VAR> VAR emVarModel<VAR>::GetInherited(
	emContext & context, const char * name, const VAR & defaultValue
)
{
	emRef<emVarModel> m;

	m=LookupInherited(context,name);
	if (!m) return defaultValue;
	return m->Var;
}


template <class VAR> emVarModel<VAR>::emVarModel(
	emContext & context, const emString & name
)
	: emModel(context,name)
{
}


#endif
