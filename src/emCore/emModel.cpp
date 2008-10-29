//------------------------------------------------------------------------------
// emModel.cpp
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#include <emCore/emModel.h>


emRef<emModel> emModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emModel,context,name,common)
}


void emModel::Free()
{
	RefCount--;
	if (RefCount<=0) {
		delete this;
	}
	else if (RefCount==1 && IsCommon()) {
		if (MinCommonLifetime==0) {
			Unregister();
		}
		else if (((int)MinCommonLifetime)>0) {
			TimeOfDeath=Context.SharedTiming->SecsCounter+MinCommonLifetime;
			Context.DoGCOnModels=true;
		}
	}
}


emModel::emModel(emContext & context, const emString & name)
	: emEngine(context.GetScheduler()),
	Context(context),
	Name(name)
{
	Context.ModelCount++;
	AvlHashCode=0;
	RefCount=0;
	MinCommonLifetime=0;
}


emModel::~emModel()
{
	if (RefCount!=0) {
		emFatalError("emModel: Non-zero RefCount at destruction.");
	}
	if (AvlHashCode!=0) {
		emFatalError("emModel: Still registered at destruction.");
	}
	Context.ModelCount--;
}


void emModel::SetMinCommonLifetime(unsigned seconds)
{
	if (MinCommonLifetime!=seconds) {
		MinCommonLifetime=seconds;
		if (((int)MinCommonLifetime)>=0 && RefCount==1 && IsCommon()) {
			TimeOfDeath=Context.SharedTiming->SecsCounter+MinCommonLifetime;
			Context.DoGCOnModels=true;
		}
	}
}


bool emModel::Cycle()
{
	return false;
}
