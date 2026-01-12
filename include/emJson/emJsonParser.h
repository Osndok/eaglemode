//------------------------------------------------------------------------------
// emJsonParser.h
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

#ifndef emJsonParser_h
#define emJsonParser_h

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emJsonElement_h
#include <emJson/emJsonElement.h>
#endif


class emJsonParser {

public:

	emJsonParser(const emString & sourceName);
	~emJsonParser();

	void TryParse(const char * buf, int len, bool endOfFile=false);

	emRef<emJsonElement> GetNextParsedRoot();

private:

	enum LexicalStateEnum {
		READY,
		SLASH,
		C_COMMENT,
		C_COMMENT_ASTERISK,
		COMMENT_TO_END_OF_LINE,
		DOUBLE_QUOTED_STRING,
		DOUBLE_QUOTED_STRING_BACKSLASH,
		DOUBLE_QUOTED_STRING_UNICODE,
		SINGLE_QUOTED_STRING,
		SINGLE_QUOTED_STRING_BACKSLASH,
		KEYWORD,
		NUMBER_SIGN,
		NUMBER_ZERO,
		NUMBER_DIGIT,
		NUMBER_DOT,
		NUMBER_DOT_DIGIT,
		NUMBER_EXP,
		NUMBER_EXP_SIGN,
		NUMBER_EXP_DIGIT
	};

	enum SyntaxStateEnum {
		READY_FOR_ROOT,
		ARRAY,
		ARRAY_VALUE,
		ARRAY_COMMA,
		OBJECT,
		OBJECT_KEY,
		OBJECT_COLON,
		OBJECT_VALUE,
		OBJECT_COMMA
	};

	void TryParseChar(char c);
	void TryParseEndOfFile();

	void AddToCurrentString(char c);

	void FinishCurrentElement();

	void TryStartObject();
	void TryEndObject();
	void TryStartArray();
	void TryEndArray();
	void TryDoComma();
	void TryDoColon();
	void TryDoString();
	void TryDoKeyword();
	void TryDoNumber();
	void TryDoValue(emJsonElement::TypeEnum type, const emString & value);

	[[noreturn]] void ThrowExpectedButFound(const char * foundWhat);
	[[noreturn]] void ThrowUnexpectedCharacter(const char * desc, char c);
	[[noreturn]] void ThrowUnexpectedEndOfFile(const char * desc);
	[[noreturn]] void ThrowSyntaxError(const char * desc);

	emString SourceName;
	emList<emRef<emJsonElement>> CompleteRoots;
	emRef<emJsonElement> CurrentRoot;
	emJsonPositionTracker PosTracker;
	LexicalStateEnum LexicalState;
	SyntaxStateEnum SyntaxState;
	emJsonPosition CurrentStringPos;
	emArray<char> CurrentStringBuf;
	int CurrentStringLen;
	char CodeContext[1<<7];
	int CodeContextPos;
	int CurrentUniCode;
	int CurrentUniCodeLen;
	emJsonElement * CurrentElement;
};


#endif
