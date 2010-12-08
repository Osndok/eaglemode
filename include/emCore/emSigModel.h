//------------------------------------------------------------------------------
// emSigModel.h
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

#ifndef emSigModel_h
#define emSigModel_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif


//==============================================================================
//================================= emSigModel =================================
//==============================================================================

class emSigModel : public emModel {

public:

	// Class for model which holds a single signal. Because the model class
	// is the same in different applications, there is an increased danger
	// of model identification conflicts. So, if the model is common, please
	// invent a smart model name. Best is to have the name of the
	// application's C++ class at the beginning of the model name (e.g.
	// "acmPacman::SpeedSignal").

	static emRef<emSigModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);
		// Acquire an emSigModel.

	static emRef<emSigModel> Lookup(
		emContext & context, const char * name
	);
		// Search a common emSigModel.

	static emRef<emSigModel> LookupInherited(
		emContext & context, const char * name
	);
		// Search a common emSigModel even in ancestor contexts.

	emSignal Sig;
		// The signal.

protected:

	emSigModel(emContext & context, const emString & name);

	virtual ~emSigModel();

};


#endif
