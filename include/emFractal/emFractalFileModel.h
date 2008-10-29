//------------------------------------------------------------------------------
// emFractalFileModel.h
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#ifndef emFractalFileModel_h
#define emFractalFileModel_h

#ifndef emRecFileModel_h
#include <emCore/emRecFileModel.h>
#endif


class emFractalFileModel : public emRecFileModel, public emStructRec {

public:

	static emRef<emFractalFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	enum {
		MANDELBROT_TYPE=0,
		JULIA_TYPE=1,
		MULTI_JULIA_TYPE=2
	};
	emEnumRec Type;

	emDoubleRec JuliaX;
	emDoubleRec JuliaY;

	emIntRec Depth;

	class ColorRec : public emStructRec {
	public:
		ColorRec();
		emColorRec Color;
		emIntRec Fade;
	};
	emTArrayRec<ColorRec> Colors;

	virtual const char * GetFormatName() const;

protected:

	emFractalFileModel(emContext & context, const emString & name);
	virtual ~emFractalFileModel();
};


#endif
