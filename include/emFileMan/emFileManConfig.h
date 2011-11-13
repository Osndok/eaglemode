//------------------------------------------------------------------------------
// emFileManConfig.h
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

#ifndef emFileManConfig_h
#define emFileManConfig_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


class emFileManConfig : public emConfigModel, public emStructRec {

public:

	static emRef<emFileManConfig> Acquire(emRootContext & rootContext);

	enum {
		SORT_BY_NAME    = 0,
		SORT_BY_ENDING  = 1,
		SORT_BY_CLASS   = 2,
		SORT_BY_VERSION = 3,
		SORT_BY_DATE    = 4,
		SORT_BY_SIZE    = 5
	};
	emEnumRec SortCriterion;

	enum {
		NSS_PER_LOCALE        = 0,
		NSS_CASE_SENSITIVE    = 1,
		NSS_CASE_INSENSITIVE  = 2
	};
	emEnumRec NameSortingStyle;

	emBoolRec SortDirectoriesFirst;

	emBoolRec ShowHiddenFiles;

	emStringRec ThemeName;

	emBoolRec Autosave;

	virtual const char * GetFormatName() const;

protected:

	emFileManConfig(emContext & context, const emString & name);
	virtual ~emFileManConfig();
};


#endif
