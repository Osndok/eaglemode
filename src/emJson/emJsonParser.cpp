//------------------------------------------------------------------------------
// emJsonParser.cpp
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

#include <emJson/emJsonParser.h>
#include <emJson/emJsonException.h>
#include <ctype.h>


emJsonParser::emJsonParser(const emString & sourceName)
	: SourceName(sourceName),
	LexicalState(READY),
	SyntaxState(READY_FOR_ROOT),
	CurrentStringLen(0),
	CodeContextPos(0),
	CurrentUniCode(0),
	CurrentUniCodeLen(0),
	CurrentElement(NULL)
{
	CurrentStringBuf.SetTuningLevel(4);
	CurrentStringBuf.SetCount(256);
	memset(CodeContext,0,sizeof(CodeContext));
}


emJsonParser::~emJsonParser()
{
}


void emJsonParser::TryParse(const char * buf, int len, bool endOfFile)
{
	const char * pEnd=buf+len;
	while (buf<pEnd) {
		char c=*buf++;

		CodeContext[CodeContextPos]=c;
		CodeContextPos=(CodeContextPos+1)%sizeof(CodeContext);

		TryParseChar(c);

		PosTracker.Parse(c);
	}

	if (endOfFile) TryParseEndOfFile();
}


emRef<emJsonElement> emJsonParser::GetNextParsedRoot()
{
	emRef<emJsonElement> result;

	if (!CompleteRoots.IsEmpty()) {
		result=*CompleteRoots.GetFirst();
		CompleteRoots.RemoveFirst();
	}

	return result;
}


void emJsonParser::TryParseChar(char c)
{
	switch (LexicalState) {
	case READY:
L_READY:
		switch (c) {
		case ',':
			TryDoComma();
			break;
		case ':':
			TryDoColon();
			break;
		case '{':
			TryStartObject();
			break;
		case '}':
			TryEndObject();
			break;
		case '[':
			TryStartArray();
			break;
		case ']':
			TryEndArray();
			break;
		case '"':
			CurrentStringPos=PosTracker.GetPosition();
			CurrentStringLen=0;
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case '\'':
			CurrentStringPos=PosTracker.GetPosition();
			CurrentStringLen=0;
			LexicalState=SINGLE_QUOTED_STRING;
			break;
		case '-':
			CurrentStringPos=PosTracker.GetPosition();
			CurrentStringLen=0;
			AddToCurrentString(c);
			LexicalState=NUMBER_SIGN;
			break;
		case '0':
			CurrentStringPos=PosTracker.GetPosition();
			CurrentStringLen=0;
			AddToCurrentString(c);
			LexicalState=NUMBER_ZERO;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			CurrentStringPos=PosTracker.GetPosition();
			CurrentStringLen=0;
			AddToCurrentString(c);
			LexicalState=NUMBER_DIGIT;
			break;
		case '/':
			LexicalState=SLASH;
			break;
		case '#':
			LexicalState=COMMENT_TO_END_OF_LINE;
			break;
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			if ((c>='A' && c<='Z') || (c>='a' && c<='z')) {
				CurrentStringPos=PosTracker.GetPosition();
				CurrentStringLen=0;
				AddToCurrentString(c);
				LexicalState=KEYWORD;
			}
			else {
				ThrowUnexpectedCharacter("outside any string",c);
			}
		}
		break;
	case SLASH:
		switch (c) {
		case '/':
			LexicalState=COMMENT_TO_END_OF_LINE;
			break;
		case '*':
			LexicalState=C_COMMENT;
			break;
		default:
			ThrowUnexpectedCharacter("after slash",c);
		}
		break;
	case C_COMMENT:
		if (c=='*') LexicalState=C_COMMENT_ASTERISK;
		break;
	case C_COMMENT_ASTERISK:
		switch (c) {
		case '/':
			LexicalState=READY;
			break;
		case '*':
			break;
		default:
			LexicalState=C_COMMENT;
			break;
		}
		break;
	case COMMENT_TO_END_OF_LINE:
		switch (c) {
		case '\r':
		case '\n':
			LexicalState=READY;
			break;
		default:
			break;
		}
		break;
	case DOUBLE_QUOTED_STRING:
		switch (c) {
		case '"':
			TryDoString();
			LexicalState=READY;
			break;
		case '\\':
			LexicalState=DOUBLE_QUOTED_STRING_BACKSLASH;
			break;
		default:
			if (((unsigned char)c)<0x20) {
				ThrowUnexpectedCharacter("control character in double-quoted string",c);
			}
			AddToCurrentString(c);
			break;
		}
		break;
	case DOUBLE_QUOTED_STRING_BACKSLASH:
		switch (c) {
		case '"':
		case '\\':
		case '/':
			AddToCurrentString(c);
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 'b':
			AddToCurrentString('\b');
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 'f':
			AddToCurrentString('\f');
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 'n':
			AddToCurrentString('\n');
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 'r':
			AddToCurrentString('\r');
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 't':
			AddToCurrentString('\t');
			LexicalState=DOUBLE_QUOTED_STRING;
			break;
		case 'u':
			CurrentUniCode=0;
			CurrentUniCodeLen=0;
			LexicalState=DOUBLE_QUOTED_STRING_UNICODE;
			break;
		default:
			ThrowUnexpectedCharacter("after backslash in double-quoted string",c);
		}
		break;
	case DOUBLE_QUOTED_STRING_UNICODE:
		CurrentUniCode<<=4;
		if (c>='0' && c<='9') CurrentUniCode+=c-'0';
		else if (c>='A' && c<='F') CurrentUniCode+=10+c-'A';
		else if (c>='a' && c<='f') CurrentUniCode+=10+c-'a';
		else ThrowUnexpectedCharacter("in unicode after \\u",c);
		CurrentUniCodeLen++;
		if (CurrentUniCodeLen>=4) {
			char buf[EM_MB_LEN_MAX];
			int len=emEncodeChar(buf,CurrentUniCode);
			for (int i=0; i<len; i++) AddToCurrentString(buf[i]);
			LexicalState=DOUBLE_QUOTED_STRING;
		}
		break;
	case SINGLE_QUOTED_STRING:
		switch (c) {
		case '\'':
			TryDoString();
			LexicalState=READY;
			break;
		case '\\':
			LexicalState=SINGLE_QUOTED_STRING_BACKSLASH;
			break;
		default:
			AddToCurrentString(c);
			break;
		}
		break;
	case SINGLE_QUOTED_STRING_BACKSLASH:
		switch (c) {
		case '\'':
		case '\\':
			AddToCurrentString(c);
			LexicalState=SINGLE_QUOTED_STRING;
			break;
		default:
			AddToCurrentString('\\');
			AddToCurrentString(c);
			LexicalState=SINGLE_QUOTED_STRING;
			break;
		}
		break;
	case KEYWORD:
		if ((c>='A' && c<='Z') || (c>='a' && c<='z')) {
			AddToCurrentString(c);
		}
		else {
			TryDoKeyword();
			LexicalState=READY;
			goto L_READY;
		}
		break;
	case NUMBER_SIGN:
		if (c=='0') {
			AddToCurrentString(c);
			LexicalState=NUMBER_ZERO;
		}
		else if (c>='1' && c<='9') {
			AddToCurrentString(c);
			LexicalState=NUMBER_DIGIT;
		}
		else {
			ThrowUnexpectedCharacter("after sign",c);
		}
		break;
	case NUMBER_ZERO:
		if (c=='.') {
			AddToCurrentString(c);
			LexicalState=NUMBER_DOT;
		}
		else if (c=='E' || c=='e') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP;
		}
		else if (c>='0' && c<='9') {
			ThrowUnexpectedCharacter("digit after zero",c);
		}
		else {
			TryDoNumber();
			LexicalState=READY;
			goto L_READY;
		}
		break;
	case NUMBER_DIGIT:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
		}
		else if (c=='.') {
			AddToCurrentString(c);
			LexicalState=NUMBER_DOT;
		}
		else if (c=='E' || c=='e') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP;
		}
		else {
			TryDoNumber();
			LexicalState=READY;
			goto L_READY;
		}
		break;
	case NUMBER_DOT:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
			LexicalState=NUMBER_DOT_DIGIT;
		}
		else {
			ThrowUnexpectedCharacter("after dot",c);
		}
		break;
	case NUMBER_DOT_DIGIT:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
		}
		else if (c=='E' || c=='e') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP;
		}
		else {
			TryDoNumber();
			LexicalState=READY;
			goto L_READY;
		}
		break;
	case NUMBER_EXP:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP_DIGIT;
		}
		else if (c=='-' || c=='+') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP_SIGN;
		}
		else {
			ThrowUnexpectedCharacter("within number exponent",c);
		}
		break;
	case NUMBER_EXP_SIGN:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
			LexicalState=NUMBER_EXP_DIGIT;
		}
		else {
			ThrowUnexpectedCharacter("after sign of number exponent",c);
		}
		break;
	case NUMBER_EXP_DIGIT:
		if (c>='0' && c<='9') {
			AddToCurrentString(c);
		}
		else {
			TryDoNumber();
			LexicalState=READY;
			goto L_READY;
		}
		break;
	}
}


void emJsonParser::TryParseEndOfFile()
{
	switch (LexicalState) {
	case READY:
		break;
	case SLASH:
		ThrowUnexpectedEndOfFile("after slash");
	case C_COMMENT:
	case C_COMMENT_ASTERISK:
		ThrowUnexpectedEndOfFile("unterminated C comment");
	case COMMENT_TO_END_OF_LINE:
		LexicalState=READY;
		break;
	case DOUBLE_QUOTED_STRING:
	case DOUBLE_QUOTED_STRING_BACKSLASH:
	case DOUBLE_QUOTED_STRING_UNICODE:
		ThrowUnexpectedEndOfFile("unterminated double-quoted string");
	case SINGLE_QUOTED_STRING:
	case SINGLE_QUOTED_STRING_BACKSLASH:
		ThrowUnexpectedEndOfFile("unterminated single-quoted string");
	case KEYWORD:
		TryDoKeyword();
		LexicalState=READY;
		break;
	case NUMBER_ZERO:
	case NUMBER_DIGIT:
	case NUMBER_DOT_DIGIT:
	case NUMBER_EXP_DIGIT:
		TryDoNumber();
		LexicalState=READY;
		break;
	case NUMBER_SIGN:
		ThrowUnexpectedEndOfFile("incomplete number (missing digit after sign)");
	case NUMBER_DOT:
		ThrowUnexpectedEndOfFile("incomplete number (missing digit after dot)");
	case NUMBER_EXP:
		ThrowUnexpectedEndOfFile("incomplete number (within exponent)");
	case NUMBER_EXP_SIGN:
		ThrowUnexpectedEndOfFile(
			"incomplete number (missing digit after sign of exponent)"
		);
	}

	switch (SyntaxState) {
	case READY_FOR_ROOT:
		break;
	case ARRAY:
	case ARRAY_VALUE:
	case ARRAY_COMMA:
		ThrowUnexpectedEndOfFile("unterminated array");
	case OBJECT:
	case OBJECT_KEY:
	case OBJECT_COLON:
	case OBJECT_VALUE:
	case OBJECT_COMMA:
		ThrowUnexpectedEndOfFile("unterminated object");
	}
}


void emJsonParser::AddToCurrentString(char c)
{
	if (CurrentStringBuf.GetCount()<=CurrentStringLen) {
		CurrentStringBuf.SetCount(CurrentStringLen+256);
	}
	CurrentStringBuf.GetWritable()[CurrentStringLen++]=c;
}


void emJsonParser::FinishCurrentElement()
{
	emJsonElement * parent;

	parent=CurrentElement->GetParent();
	if (!parent) {
		CompleteRoots.Add(CurrentRoot);
		CurrentRoot.Reset();
		SyntaxState=READY_FOR_ROOT;
	}
	else if (parent->GetType()==emJsonElement::T_ARRAY) {
		SyntaxState=ARRAY_VALUE;
	}
	else if (parent->GetType()==emJsonElement::T_OBJECT) {
		SyntaxState=OBJECT_VALUE;
	}
	else {
		emFatalError("emJsonParser::FinishCurrentElement: bad element type");
	}
}


void emJsonParser::TryStartObject()
{
	switch (SyntaxState) {
	case READY_FOR_ROOT:
		CurrentRoot=new emJsonElement(
			NULL,NULL,SourceName,PosTracker.GetPosition(),
			emJsonElement::T_OBJECT
		);
		CurrentElement=CurrentRoot;
		SyntaxState=OBJECT;
		break;
	case ARRAY:
		CurrentElement=new emJsonElement(
			CurrentElement,NULL,SourceName,PosTracker.GetPosition(),
			emJsonElement::T_OBJECT
		);
		SyntaxState=OBJECT;
		break;
	case ARRAY_COMMA:
		CurrentElement=new emJsonElement(
			CurrentElement->GetParent(),CurrentElement,SourceName,
			PosTracker.GetPosition(),emJsonElement::T_OBJECT
		);
		SyntaxState=OBJECT;
		break;
	case OBJECT_COLON:
		CurrentElement->Type=emJsonElement::T_OBJECT;
		CurrentElement->ValuePosition=PosTracker.GetPosition();
		SyntaxState=OBJECT;
		break;
	default:
		ThrowExpectedButFound("an opening brace");
	}
}


void emJsonParser::TryEndObject()
{
	switch (SyntaxState) {
	case OBJECT:
		FinishCurrentElement();
		break;
	case OBJECT_VALUE:
		CurrentElement=CurrentElement->GetParent();
		FinishCurrentElement();
		break;
	default:
		ThrowExpectedButFound("a closing brace");
	}
}


void emJsonParser::TryStartArray()
{
	switch (SyntaxState) {
	case READY_FOR_ROOT:
		CurrentRoot=new emJsonElement(
			NULL,NULL,SourceName,PosTracker.GetPosition(),
			emJsonElement::T_ARRAY
		);
		CurrentElement=CurrentRoot;
		SyntaxState=ARRAY;
		break;
	case ARRAY:
		CurrentElement=new emJsonElement(
			CurrentElement,NULL,SourceName,PosTracker.GetPosition(),
			emJsonElement::T_ARRAY
		);
		SyntaxState=ARRAY;
		break;
	case ARRAY_COMMA:
		CurrentElement=new emJsonElement(
			CurrentElement->GetParent(),CurrentElement,SourceName,
			PosTracker.GetPosition(),emJsonElement::T_ARRAY
		);
		SyntaxState=ARRAY;
		break;
	case OBJECT_COLON:
		CurrentElement->Type=emJsonElement::T_ARRAY;
		CurrentElement->ValuePosition=PosTracker.GetPosition();
		SyntaxState=ARRAY;
		break;
	default:
		ThrowExpectedButFound("an opening bracket");
	}
}


void emJsonParser::TryEndArray()
{
	switch (SyntaxState) {
	case ARRAY:
		FinishCurrentElement();
		break;
	case ARRAY_VALUE:
		CurrentElement=CurrentElement->GetParent();
		FinishCurrentElement();
		break;
	default:
		ThrowExpectedButFound("a closing bracket");
	}
}


void emJsonParser::TryDoComma()
{
	switch (SyntaxState) {
	case ARRAY_VALUE:
		SyntaxState=ARRAY_COMMA;
		break;
	case OBJECT_VALUE:
		SyntaxState=OBJECT_COMMA;
		break;
	default:
		ThrowExpectedButFound("a comma");
	}
}


void emJsonParser::TryDoColon()
{
	switch (SyntaxState) {
	case OBJECT_KEY:
		SyntaxState=OBJECT_COLON;
		break;
	default:
		ThrowExpectedButFound("a colon");
	}
}


void emJsonParser::TryDoString()
{
	emString value(CurrentStringBuf.Get(),CurrentStringLen);

	switch (SyntaxState) {
	case OBJECT:
		CurrentElement=new emJsonElement(
			CurrentElement,NULL,SourceName,CurrentStringPos,
			emJsonElement::T_NULL
		);
		CurrentElement->Key=value;
		SyntaxState=OBJECT_KEY;
		break;
	case OBJECT_COMMA:
		CurrentElement=new emJsonElement(
			CurrentElement->GetParent(),CurrentElement,SourceName,
			CurrentStringPos,emJsonElement::T_NULL
		);
		CurrentElement->Key=value;
		SyntaxState=OBJECT_KEY;
		break;
	default:
		TryDoValue(emJsonElement::T_STRING,value);
		break;
	}
}


void emJsonParser::TryDoKeyword()
{
	emString value(CurrentStringBuf.Get(),CurrentStringLen);
	if (strcasecmp(value.Get(),"null")==0) {
		TryDoValue(emJsonElement::T_NULL,value);
	}
	else if (
		strcasecmp(value.Get(),"false")==0 ||
		strcasecmp(value.Get(),"true")==0
	) {
		TryDoValue(emJsonElement::T_BOOL,value);
	}
	else {
		ThrowSyntaxError(emString::Format("Unexpected keyword: %s",value.Get()));
	}
}


void emJsonParser::TryDoNumber()
{
	emString value(CurrentStringBuf.Get(),CurrentStringLen);
	TryDoValue(emJsonElement::T_NUMBER,value);
}


void emJsonParser::TryDoValue(
	emJsonElement::TypeEnum type, const emString & value
)
{
	switch (SyntaxState) {
	case READY_FOR_ROOT:
		CurrentRoot=new emJsonElement(NULL,NULL,SourceName,CurrentStringPos,type);
		CurrentRoot->Value=value;
		CompleteRoots.Add(CurrentRoot);
		CurrentRoot.Reset();
		break;
	case ARRAY:
		CurrentElement=new emJsonElement(
			CurrentElement,NULL,SourceName,CurrentStringPos,type
		);
		CurrentElement->Value=value;
		SyntaxState=ARRAY_VALUE;
		break;
	case ARRAY_COMMA:
		CurrentElement=new emJsonElement(
			CurrentElement->GetParent(),CurrentElement,SourceName,
			CurrentStringPos,type
		);
		CurrentElement->Value=value;
		SyntaxState=ARRAY_VALUE;
		break;
	case OBJECT_COLON:
		CurrentElement->Type=type;
		CurrentElement->Value=value;
		CurrentElement->ValuePosition=CurrentStringPos;
		SyntaxState=OBJECT_VALUE;
		break;
	default:
		switch (type) {
		case emJsonElement::T_NULL:   ThrowExpectedButFound("a null");
		case emJsonElement::T_BOOL:   ThrowExpectedButFound("a Boolean value");
		case emJsonElement::T_NUMBER: ThrowExpectedButFound("a number");
		case emJsonElement::T_STRING: ThrowExpectedButFound("a string");
		default:                      ThrowExpectedButFound("something else");
		}
	}
}


void emJsonParser::ThrowExpectedButFound(const char * foundWhat)
{
	const char * expected;

	switch (SyntaxState) {
	case READY_FOR_ROOT:
		expected="a root element";
		break;
	case ARRAY:
		expected="an element or a closing bracket after the opening bracket";
		break;
	case ARRAY_VALUE:
		expected="a comma or a closing bracket after the element of an array";
		break;
	case ARRAY_COMMA:
		expected="an element after the comma";
		break;
	case OBJECT:
		expected="a key string or a closing brace after the opening brace";
		break;
	case OBJECT_KEY:
		expected="a colon after the key string";
		break;
	case OBJECT_COLON:
		expected="an element after the colon";
		break;
	case OBJECT_VALUE:
		expected="a comma or a closing brace after the element of an object";
		break;
	case OBJECT_COMMA:
		expected="a key string after the comma";
		break;
	default:
		expected="an element";
		break;
	}
	ThrowSyntaxError(emString::Format(
		"Expected %s, but found %s.",expected,foundWhat
	));
}


void emJsonParser::ThrowUnexpectedCharacter(const char * desc, char c)
{
	char buf[32];

	if (c>0x20 && c<0x7f) sprintf(buf,"'%c'",c);
	else sprintf(buf,"0x%02X",(unsigned)(unsigned char)c);

	ThrowSyntaxError(emString::Format("Unexpected character %s (%s)",buf,desc));
}


void emJsonParser::ThrowUnexpectedEndOfFile(const char * desc)
{
	ThrowSyntaxError(emString::Format("Unexpected end of file: %s",desc));
}


void emJsonParser::ThrowSyntaxError(const char * desc)
{
	char lastChars[sizeof(CodeContext)+1];
	int i,n,l;
	char c;

	n=(int)sizeof(CodeContext);
	for (i=0, l=0; i<n; i++) {
		c=CodeContext[(CodeContextPos+i)%n];
		if (c=='\n' || c=='\r') {
			if (n-i>70) {
				l=0;
				continue;
			}
		}
		else if ((unsigned char)c<32 && c!='\t') {
			continue;
		}
		lastChars[l++]=c;
	}
	lastChars[l]=0;
	if (l>80) memmove(lastChars,lastChars+l-80,81);

	throw emJsonException(
		SourceName,PosTracker.GetPosition(),desc,lastChars,true
	);
}
