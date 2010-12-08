//------------------------------------------------------------------------------
// emMainConfig.h
//
// Copyright (C) 2010 Oliver Hamann.
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

#ifndef emMainConfig_h
#define emMainConfig_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


class emMainConfig : public emConfigModel, public emStructRec {

public:

	static emRef<emMainConfig> Acquire(emRootContext & rootContext);

	emBoolRec AutoHideControlView;

	emBoolRec AutoHideSlider;

	emDoubleRec ControlViewSize;

	virtual const char * GetFormatName() const;

protected:

	emMainConfig(emContext & context, const emString & name);
	virtual ~emMainConfig();
};


#endif
