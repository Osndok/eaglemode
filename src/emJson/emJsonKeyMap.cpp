//------------------------------------------------------------------------------
// emJsonKeyMap.cpp
//
// Copyright (C) 2025 Oliver Hamann.
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

#include <emJson/emJsonKeyMap.h>


//==============================================================================
//============================== emJsonKeyMapBase ==============================
//==============================================================================

emJsonKeyMapBase::emJsonKeyMapBase()
	: Designation("none"),
	Count(0)
{
}


emJsonKeyMapBase::~emJsonKeyMapBase()
{
}


int emJsonKeyMapBase::TryFind(const char * str) const
{
	const Element * array, * elem;
	int i,j,k,r;

	array=SortedByStrings.Get();

	for (i=0, j=Count;;) {
		if (i>=j) throw emException("Unknown identifier for %s: %s",Designation,str);
		k=(i+j)/2;
		elem=array+k;
		r=strcasecmp(elem->Str,str);
		if (r==0) return elem->Key;
		if (r<0) i=k+1; else j=k;
	}
}


const char * emJsonKeyMapBase::ToString(int key) const
{
	const Element * const * array;
	const Element * elem;
	int i,j,k;

	array=SortedByKeys.Get();

	for (i=0, j=Count;;) {
		if (i>=j) {
			emFatalError("emJsonKeyMapBase: No such key for %s: %d",Designation,key);
		}
		k=(i+j)/2;
		elem=array[k];
		if (elem->Key==key) break;
		if (elem->Key<key) i=k+1; else j=k;
	}

	while (k>0 && array[k-1]->Key==key) k--;

	for (i=k+1; i<Count && array[i]->Key==key; i++) {
		if (strlen(array[i]->Str)>strlen(array[k]->Str)) k=i;
	}

	return array[k]->Str;
}


emArray<int> emJsonKeyMapBase::CollectAllKeys() const
{
	emArray<int> result;
	int i,key;

	result.SetTuningLevel(4);
	for (i=0; i<Count; i++) {
		key=SortedByKeys[i]->Key;
		if (i==0 || SortedByKeys[i-1]->Key!=key) {
			result.Add(key);
		}
	}
	return result;
}


void emJsonKeyMapBase::Init(
	const char * designation, const char * str0, int key0, va_list args
)
{
	va_list args2;
	const char * str;
	int pass,key,count,i;

	Designation=designation;

	for (pass=0; pass<2; pass++) {
		va_copy(args2,args);
		for (count=0; ; count++) {
			if (count==0) {
				str=str0;
				key=key0;
			}
			else {
				str=va_arg(args2,const char *);
				if (!str) break;
				key=va_arg(args2,int);
			}
			if (pass==1) {
				SortedByStrings[count].Str=str;
				SortedByStrings[count].Key=key;
			}
		}
		va_end(args2);
		if (pass==0) SortedByStrings=new Element[count];
	}

	Count=count;

	emSortArray(SortedByStrings.Get(),count,CompareStrings,NULL);

	SortedByKeys=new const Element*[count];
	for (i=0; i<count; i++) {
		SortedByKeys[i]=&SortedByStrings[i];
	}
	emSortArray(SortedByKeys.Get(),count,CompareKeys,NULL);
}


int emJsonKeyMapBase::CompareStrings(
	const Element * e1, const Element * e2, void * context
)
{
	return strcasecmp(e1->Str,e2->Str);
}


int emJsonKeyMapBase::CompareKeys(
	const Element * const * e1, const Element * const * e2, void * context
)
{
	return (*e1)->Key - (*e2)->Key;
}
