//------------------------------------------------------------------------------
// emOsmFileModel.cpp
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

#include <emOsm/emOsmFileModel.h>


emRef<emOsmFileModel> emOsmFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emOsmFileModel,context,name,common)
}


const char * emOsmFileModel::GetFormatName() const
{
	return "emOsm";
}


emOsmFileModel::emOsmFileModel(emContext & context, const emString & name)
	: emRecFileModel(context,name),
	emStructRec(),
	TilesUrl(this,"TilesUrl"),
	MaxZ(this,"MaxZ",18,0,30)
{
	PostConstruct(*this);
}


emOsmFileModel::~emOsmFileModel()
{
}
