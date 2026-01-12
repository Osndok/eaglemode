//------------------------------------------------------------------------------
// emJsonElement.h
//
// Copyright (C) 2025-2026 Oliver Hamann.
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

#ifndef emJsonElement_h
#define emJsonElement_h

#ifndef emColor_h
#include <emCore/emColor.h>
#endif

#ifndef emRef_h
#include <emCore/emRef.h>
#endif

#ifndef emJsonKeyMap_h
#include <emJson/emJsonKeyMap.h>
#endif

#ifndef emJsonPositionTracker_h
#include <emJson/emJsonPositionTracker.h>
#endif


class emJsonElement : public emRefTarget, public emUncopyable {

public:

	enum TypeEnum {
		T_NULL,
		T_BOOL,
		T_NUMBER,
		T_STRING,
		T_ARRAY,
		T_OBJECT
	};

	emJsonElement(
		emJsonElement * parent, emJsonElement * predecessor,
		const emString & sourceName, emJsonPosition position,
		TypeEnum type
	);

	virtual ~emJsonElement();

	const emJsonPosition & GetPosition() const;
	const emJsonPosition & GetValuePosition() const;

	const emString & GetKey() const;

	template <class KEY> KEY TryGetKey(const emJsonKeyMap<KEY> & keyMap) const;

	TypeEnum GetType() const;

	bool IsValueType() const;

	bool TryGetValueAsBool() const;
	int TryGetValueAsInt() const;
	unsigned int TryGetValueAsUnsignedInt() const;
	emInt64 TryGetValueAsInt64() const;
	emUInt64 TryGetValueAsUInt64() const;
	double TryGetValueAsDouble() const;
	emColor TryGetValueAsColor() const;
	const emString & TryGetValueAsString() const;

	template <class KEY> KEY TryGetValueAsKey(
		const emJsonKeyMap<KEY> & keyMap
	) const;

	emArray<int> TryGetAsArrayOfInt() const;
	emArray<emInt64> TryGetAsArrayOfInt64() const;
	emArray<emUInt64> TryGetAsArrayOfUInt64() const;
	emArray<double> TryGetAsArrayOfDouble() const;
	emArray<emString> TryGetAsArrayOfString() const;

	emJsonElement * GetParent() const;
	emJsonElement * GetFirstChild() const;
	emJsonElement * GetNext() const;

	[[noreturn]] void ThrowKeyError(const char * message) const;
	[[noreturn]] void ThrowValueError(const char * message) const;
	[[noreturn]] void ThrowExpectedValueError(const char * typeName) const;

	emString GetValueAdaptedForErrorMessage(int maxLen=32) const;

private: friend class emJsonParser;

	TypeEnum Type;
	emJsonElement * Parent;
	emJsonElement * FirstChild;
	emJsonElement * Next;
	emString SourceName;
	emJsonPosition Position;
	emJsonPosition ValuePosition;
	emString Key;
	emString Value;
};


inline const emJsonPosition & emJsonElement::GetPosition() const
{
	return Position;
}

inline const emJsonPosition & emJsonElement::GetValuePosition() const
{
	return ValuePosition;
}

inline const emString & emJsonElement::GetKey() const
{
	return Key;
}

template <class KEY> KEY emJsonElement::TryGetKey(
	const emJsonKeyMap<KEY> & keyMap
) const
{
	try {
		return keyMap.TryFind(Key);
	}
	catch (const emException & e) {
		ThrowKeyError(e.GetText());
	}
}

inline emJsonElement::TypeEnum emJsonElement::GetType() const
{
	return Type;
}

inline bool emJsonElement::IsValueType() const
{
	return Type==T_STRING || Type==T_NUMBER || Type==T_BOOL;
}

template <class KEY> KEY emJsonElement::TryGetValueAsKey(
	const emJsonKeyMap<KEY> & keyMap
) const
{
	if (!IsValueType()) {
		ThrowExpectedValueError(keyMap.GetDesignation());
	}
	try {
		return keyMap.TryFind(Value.Get());
	}
	catch (const emException & e) {
		ThrowValueError(e.GetText());
	}
}

inline emJsonElement * emJsonElement::GetParent() const
{
	return Parent;
}

inline emJsonElement * emJsonElement::GetFirstChild() const
{
	return FirstChild;
}

inline emJsonElement * emJsonElement::GetNext() const
{
	return Next;
}


#endif
