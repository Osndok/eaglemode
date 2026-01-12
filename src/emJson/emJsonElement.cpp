//------------------------------------------------------------------------------
// emJsonElement.cpp
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

#include <emJson/emJsonElement.h>
#include <emJson/emJsonException.h>


emJsonElement::emJsonElement(
	emJsonElement * parent, emJsonElement * predecessor,
	const emString & sourceName, emJsonPosition position,
	TypeEnum type
) :
	Type(type),
	Parent(parent),
	FirstChild(NULL),
	Next(NULL),
	SourceName(sourceName),
	Position(position),
	ValuePosition(position)
{
	if (parent) {
		Alloc();
		if (predecessor) {
			EM_ASSERT(parent==predecessor->Parent);
			Next=predecessor->Next;
			predecessor->Next=this;
		}
		else {
			Next=parent->FirstChild;
			parent->FirstChild=this;
		}
	}
	else {
		EM_ASSERT(!predecessor);
	}
}


emJsonElement::~emJsonElement()
{
	emJsonElement * p;

	EM_ASSERT(!Parent);
	EM_ASSERT(!Next);

	while (FirstChild) {
		p=FirstChild;
		FirstChild=p->Next;
		p->Parent=NULL;
		p->Next=NULL;
		p->Free();
	}
}


bool emJsonElement::TryGetValueAsBool() const
{
	static const emJsonKeyMap<bool> boolKeyMap(
		"a Boolean value",
		"true" , true,
		"yes"  , true,
		"y"    , true,
		"1"    , true,
		"false", false,
		"no"   , false,
		"n"    , false,
		"0"    , false,
		NULL
	);

	return TryGetValueAsKey(boolKeyMap);
}


int emJsonElement::TryGetValueAsInt() const
{
	emInt64 i=TryGetValueAsInt64();
	if (i<INT_MIN || i>INT_MAX) {
		ThrowValueError("Signed integer out of range");
	}
	return (int)i;
}


unsigned int emJsonElement::TryGetValueAsUnsignedInt() const
{
	emUInt64 i=TryGetValueAsUInt64();
	if (i>UINT_MAX) {
		ThrowValueError("Unsigned integer out of range");
	}
	return (unsigned int)i;
}


emInt64 emJsonElement::TryGetValueAsInt64() const
{
	if (!IsValueType()) {
		ThrowExpectedValueError("an integer number");
	}

	if (!Value.IsEmpty()) {
		char * ep=NULL;
		errno=0;
		emInt64 i;
		if (sizeof(long) < 8)
			i=strtoll(Value.Get(),&ep,10);
		else
			i=strtol(Value.Get(),&ep,10);
		if (errno) ThrowValueError(emGetErrorText(errno));
		if (!*ep) return i;
	}
	ThrowValueError(
		emString::Format(
			"Cannot interpret as an integer number: %s",
			GetValueAdaptedForErrorMessage().Get()
		)
	);
}


emUInt64 emJsonElement::TryGetValueAsUInt64() const
{
	if (!IsValueType()) {
		ThrowExpectedValueError("an unsigned integer number");
	}

	if (!Value.IsEmpty()) {
		char * ep=NULL;
		errno=0;
		emUInt64 i;
		if (sizeof(unsigned long) < 8)
			i=strtoull(Value.Get(),&ep,10);
		else
			i=strtoul(Value.Get(),&ep,10);
		if (errno) ThrowValueError(emGetErrorText(errno));
		if (!*ep) return i;
	}
	ThrowValueError(
		emString::Format(
			"Cannot interpret as an unsigned integer number: %s",
			GetValueAdaptedForErrorMessage().Get()
		)
	);
}


double emJsonElement::TryGetValueAsDouble() const
{
	if (!IsValueType()) {
		ThrowExpectedValueError("an integer or a floating point number");
	}

	if (!Value.IsEmpty()) {
		char * ep=NULL;
		errno=0;
		double d=strtod(Value.Get(),&ep);
		if (errno) ThrowValueError(emGetErrorText(errno));
		if (!*ep) return d;
	}
	ThrowValueError(
		emString::Format(
			"Cannot interpret as an integer or a floating point number: %s",
			GetValueAdaptedForErrorMessage().Get()
		)
	);
}


emColor emJsonElement::TryGetValueAsColor() const
{
	if (Type!=T_STRING) ThrowExpectedValueError("a color");

	emColor c;
	try {
		c.TryParse(Value.Get());
	}
	catch (const emException & e) {
		ThrowValueError(e.GetText());
	}
	return c;
}


const emString & emJsonElement::TryGetValueAsString() const
{
	if (!IsValueType()) ThrowExpectedValueError("a string");
	return Value;
}


emArray<int> emJsonElement::TryGetAsArrayOfInt() const
{
	emArray<int> result;
	result.SetTuningLevel(4);
	if (IsValueType()) {
		result+=TryGetValueAsInt();
	}
	else if (Type==T_ARRAY) {
		for (const emJsonElement * c=GetFirstChild(); c; c=c->GetNext()) {
			result+=c->TryGetValueAsInt();
		}
	}
	else {
		ThrowExpectedValueError("an array of integers or a single integer");
	}
	return result;
}


emArray<emInt64> emJsonElement::TryGetAsArrayOfInt64() const
{
	emArray<emInt64> result;
	result.SetTuningLevel(4);
	if (IsValueType()) {
		result+=TryGetValueAsInt64();
	}
	else if (Type==T_ARRAY) {
		for (const emJsonElement * c=GetFirstChild(); c; c=c->GetNext()) {
			result+=c->TryGetValueAsInt64();
		}
	}
	else {
		ThrowExpectedValueError("an array of integers or a single integer");
	}
	return result;
}


emArray<emUInt64> emJsonElement::TryGetAsArrayOfUInt64() const
{
	emArray<emUInt64> result;
	result.SetTuningLevel(4);
	if (IsValueType()) {
		result+=TryGetValueAsUInt64();
	}
	else if (Type==T_ARRAY) {
		for (const emJsonElement * c=GetFirstChild(); c; c=c->GetNext()) {
			result+=c->TryGetValueAsUInt64();
		}
	}
	else {
		ThrowExpectedValueError(
			"an array of unsigned integers or a single unsigned integer"
		);
	}
	return result;
}


emArray<double> emJsonElement::TryGetAsArrayOfDouble() const
{
	emArray<double> result;
	result.SetTuningLevel(4);
	if (IsValueType()) {
		result+=TryGetValueAsDouble();
	}
	else if (Type==T_ARRAY) {
		for (const emJsonElement * c=GetFirstChild(); c; c=c->GetNext()) {
			result+=c->TryGetValueAsDouble();
		}
	}
	else {
		ThrowExpectedValueError("an array of numbers or a single number");
	}
	return result;
}


emArray<emString> emJsonElement::TryGetAsArrayOfString() const
{
	emArray<emString> result;
	result.SetTuningLevel(1);
	if (IsValueType()) {
		result+=TryGetValueAsString();
	}
	else if (Type==T_ARRAY) {
		for (const emJsonElement * c=GetFirstChild(); c; c=c->GetNext()) {
			result+=c->TryGetValueAsString();
		}
	}
	else {
		ThrowExpectedValueError("an array of strings or a single string");
	}
	return result;
}


void emJsonElement::ThrowKeyError(const char * message) const
{
	throw emJsonException(SourceName,Position,message);
}


void emJsonElement::ThrowValueError(const char * message) const
{
	emJsonException exception(SourceName,ValuePosition,message);

	if (Parent) {
		if (Parent->Type==T_OBJECT) {
			exception.PrependContext(
				emString::Format("In an attribute named %s:",Key.Get())
			);
		}
		else if (Parent->Type==T_ARRAY) {
			unsigned index=0;
			for (
				const emJsonElement * element=Parent->FirstChild;
				element!=this;
				element=element->Next
			) {
				index++;
			}
			if (Parent->Parent && Parent->Parent->Type==T_OBJECT) {
				exception.PrependContext(emString::Format(
					"In the array at index %u:",index
				));
				exception.PrependContext(emString::Format(
					"In an attribute named %s:",Parent->Key.Get()
				));
			}
			else {
				exception.PrependContext(emString::Format(
					"In an array at index %u:",index
				));
			}
		}
	}

	throw exception;
}


void emJsonElement::ThrowExpectedValueError(const char * typeName) const
{
	const char * found;

	switch (Type) {
		case T_NULL:
			found="a null";
			break;
		case T_BOOL:
			found="a Boolean value";
			break;
		case T_NUMBER:
			found="a number";
			break;
		case T_STRING:
			found="a string";
			break;
		case T_ARRAY:
			found="an array ('[')";
			break;
		case T_OBJECT:
			found="an object ('{')";
			break;
		default:
			found="nothing";
			break;
	}
	ThrowValueError(emString::Format("Expected %s, but found %s.",typeName,found));
}


emString emJsonElement::GetValueAdaptedForErrorMessage(int maxLen) const
{
	emString str=Value;

	for (int i=0; str[i]; i++) {
		if ((unsigned char)str[i]<32) {
			str.Replace(i,1,' ');
		}
	}

	for (;;) {
		const char * p=strstr(str.Get(),"  ");
		if (!p) break;
		str.Remove(p-str.Get());
	}

	if (maxLen<3) maxLen=3;
	if (str.GetLen()>maxLen) {
		str.Replace(maxLen-3,str.GetLen()-maxLen+3,"...");
	}

	return str;
}
