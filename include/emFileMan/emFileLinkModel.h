//------------------------------------------------------------------------------
// emFileLinkModel.h
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

#ifndef emFileLinkModel_h
#define emFileLinkModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif


class emFileLinkModel : public emRecFileModel, public emStructRec {

public:

	static emRef<emFileLinkModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	enum {
		BPT_NONE        =  0,
		BPT_BIN         =  1,
		BPT_INCLUDE     =  2,
		BPT_LIB         =  3,
		BPT_HTML_DOC    =  4,
		BPT_PS_DOC      =  5,
		BPT_USER_CONFIG =  6,
		BPT_HOST_CONFIG =  7,
		BPT_TMP         =  8,
		BPT_RES         =  9,
		BPT_HOME        = 10
	};
	emEnumRec BasePathType;

	emStringRec BasePathProject;

	emStringRec Path;

	emBoolRec HaveDirEntry;

	virtual const char * GetFormatName() const;

	emString GetFullPath() const;

protected:

	emFileLinkModel(emContext & context, const emString & name);
	virtual ~emFileLinkModel();

};


#endif
