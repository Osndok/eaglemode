//------------------------------------------------------------------------------
// emJsonException.cpp
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

#include <emJson/emJsonException.h>


emJsonException::emJsonException(
	const emString & sourceName, emJsonPosition position,
	const emString & description, const emString & lastSourceChars,
	bool isJsonSyntaxError
) :
	SourceName(sourceName),
	Position(position),
	Description(description),
	LastSourceChars(lastSourceChars),
	IsJsonSyntaxError(isJsonSyntaxError),
	TopLevelContextExists(false)
{
	UpdateText();
}


emJsonException::emJsonException(const emJsonException & other)
	: emException(other),
	SourceName(other.SourceName),
	Position(other.Position),
	Contexts(other.Contexts),
	Description(other.Description),
	LastSourceChars(other.LastSourceChars),
	IsJsonSyntaxError(other.IsJsonSyntaxError),
	TopLevelContextExists(other.TopLevelContextExists)
{
}


emJsonException::~emJsonException()
{
}


emJsonException & emJsonException::operator = (const emJsonException & other)
{
	if (this!=&other) {
		emException::operator=(other);
		SourceName=other.SourceName;
		Position=other.Position;
		Contexts=other.Contexts;
		Description=other.Description;
		LastSourceChars=other.LastSourceChars;
		IsJsonSyntaxError=other.IsJsonSyntaxError;
		TopLevelContextExists=other.TopLevelContextExists;
	}
	return *this;
}


void emJsonException::PrependContext(const emString & context)
{
	Contexts.InsertAtBeg(context);
	UpdateText();
}


void emJsonException::PrependTopLevelContext(const emString & context)
{
	PrependContext(context);
	TopLevelContextExists=true;
}


void emJsonException::UpdateText()
{
	emString text;
	int column=0;

	AddFragment(
		text,column,
		IsJsonSyntaxError?"JSON syntax error":"Error"
	);

	if (!SourceName.IsEmpty()) {
		AddFragment(text,column,"in");
		AddFragment(text,column,SourceName);
	}

	AddFragment(
		text,column,
		emString::Format("at %s:",Position.ToString().Get())
	);

	for (const auto & context: Contexts) {
		AddFragment(text,column,context);
	}

	AddFragment(text,column,Description);

	if (!LastSourceChars.IsEmpty()) {
		AddFragment(text,column,"Last characters until the error:",true);
		AddFragment(text,column,LastSourceChars,true);
	}

	SetText(text);
}


void emJsonException::AddFragment(
	emString & text, int & column, const char * str, bool forceNewLine
)
{
	int len=strlen(str);
	if (forceNewLine || (column>0 && column+1+len>80)) {
		text.Add('\n');
		column=0;
	}
	if (column>0) {
		text.Add(' ');
		column++;
	}
	text.Add(str);
	column+=len;
}
