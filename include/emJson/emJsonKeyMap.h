//------------------------------------------------------------------------------
// emJsonKeyMap.h
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

#ifndef emJsonKeyMap_h
#define emJsonKeyMap_h

#ifndef emOwnPtr_h
#include <emCore/emOwnPtr.h>
#endif

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//============================== emJsonKeyMapBase ==============================
//==============================================================================

class emJsonKeyMapBase : public emUncopyable {

public:

	emJsonKeyMapBase();
	~emJsonKeyMapBase();

	int TryFind(const char * str) const;

	const char * ToString(int key) const;

	const char * GetDesignation() const;

	emArray<int> CollectAllKeys() const;

protected:

	void Init(const char * designation, const char * str0, int key0, va_list args);

private:

	// This doesn't use emString because some instances of emJsonKeyMapBase
	// are static variables (thread safety...).

	struct Element {
		const char * Str;
		int Key;
	};

	static int CompareStrings(
		const Element * e1, const Element * e2, void * context
	);

	static int CompareKeys(
		const Element * const * e1, const Element * const * e2, void * context
	);

	const char * Designation;
	int Count;
	emOwnArrayPtr<Element> SortedByStrings;
	emOwnArrayPtr<const Element*> SortedByKeys;
};


//==============================================================================
//================================ emJsonKeyMap ================================
//==============================================================================

template <class KEY> class emJsonKeyMap : public emJsonKeyMapBase {

public:

	emJsonKeyMap(const char * designation, const char * str0, int key0, ...);

	KEY TryFind(const char * str) const;

	const char * ToString(KEY key) const;

	emString GetCommaSeparatedStringsForFilteredKeys(bool(*filter)(KEY key)) const;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

inline const char * emJsonKeyMapBase::GetDesignation() const
{
	return Designation;
}

template <class KEY> emJsonKeyMap<KEY>::emJsonKeyMap(
	const char * designation, const char * str0, int key0, ...
)
{
	va_list args;

	va_start(args,key0);
	Init(designation,str0,key0,args);
	va_end(args);
}


template <class KEY> inline KEY emJsonKeyMap<KEY>::TryFind(const char * str) const
{
	return (KEY)emJsonKeyMapBase::TryFind(str);
}


template <class KEY> inline const char * emJsonKeyMap<KEY>::ToString(KEY key) const
{
	return emJsonKeyMapBase::ToString((int)key);
}


template <class KEY>
emString emJsonKeyMap<KEY>::GetCommaSeparatedStringsForFilteredKeys
(
	bool(*filter)(KEY key)
) const
{
	emString result;
	for (int key: CollectAllKeys()) {
		if (filter((KEY)key)) {
			if (!result.IsEmpty()) result+=", ";
			result+=ToString((KEY)key);
		}
	}
	return result;
}


#endif
