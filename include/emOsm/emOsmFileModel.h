//------------------------------------------------------------------------------
// emOsmFileModel.h
//
// Copyright (C) 2012,2024 Oliver Hamann.
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

#ifndef emOsmFileModel_h
#define emOsmFileModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif


class emOsmFileModel : public emRecFileModel, public emStructRec {

public:

	static emRef<emOsmFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	emStringRec TilesUrl;
	emIntRec MaxZ;

	virtual const char * GetFormatName() const;

protected:

	emOsmFileModel(emContext & context, const emString & name);
	virtual ~emOsmFileModel();

};


#endif
