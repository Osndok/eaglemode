//------------------------------------------------------------------------------
// emTreeDumpFileModel.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef emTreeDumpFileModel_h
#define emTreeDumpFileModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif

#ifndef emTreeDumpRec_h
#include <emTreeDump/emTreeDumpRec.h>
#endif


class emTreeDumpFileModel : public emRecFileModel, public emTreeDumpRec {

public:

	static emRef<emTreeDumpFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

protected:

	emTreeDumpFileModel(emContext & context, const emString & name);
	virtual ~emTreeDumpFileModel();
};


#endif
