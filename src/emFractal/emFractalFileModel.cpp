//------------------------------------------------------------------------------
// emFractalFileModel.cpp
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

#include <emFractal/emFractalFileModel.h>


emRef<emFractalFileModel> emFractalFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emFractalFileModel,context,name,common)
}


emFractalFileModel::ColorRec::ColorRec()
	: emStructRec(),
	Color(this,"color"),
	Fade(this,"fade",0,0,255)
{
}


const char * emFractalFileModel::GetFormatName() const
{
	return "emFractal";
}


emFractalFileModel::emFractalFileModel(
	emContext & context, const emString & name
)
	: emRecFileModel(context,name),
	emStructRec(),
	Type(this,"type",MANDELBROT_TYPE,"mandelbrot","julia","multi_julia",NULL),
	JuliaX(this,"julia_x",0.0),
	JuliaY(this,"julia_y",0.0),
	Depth(this,"depth",2000,1,10000),
	Colors(this,"colors",2,256)
{
	PostConstruct(*this);
}


emFractalFileModel::~emFractalFileModel()
{
}
