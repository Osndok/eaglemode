//------------------------------------------------------------------------------
// emOsmConfig.h
//
// Copyright (C) 2024 Oliver Hamann.
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

#ifndef emOsmConfig_h
#define emOsmConfig_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


class emOsmConfig : public emConfigModel, public emStructRec {

public:

	static emRef<emOsmConfig> Acquire(emRootContext & rootContext);

	emIntRec MaxCacheMegabytes;
	emIntRec MaxCacheAgeDays;

	virtual const char * GetFormatName() const;

	static const char * TryGetCacheDirectory();

protected:

	emOsmConfig(emContext & context, const emString & name);
	virtual ~emOsmConfig();
};


#endif
