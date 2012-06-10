//------------------------------------------------------------------------------
// emRec.cpp - Recordable data structures
//
// Copyright (C) 2005-2010,2012 Oliver Hamann.
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

#include <emCore/emRec.h>


//==============================================================================
//================================= emRecNode ==================================
//==============================================================================

emRecNode::~emRecNode()
{
}


//==============================================================================
//=================================== emRec ====================================
//==============================================================================

emRec::emRec(emStructRec * parent, const char * varIdentifier)
{
	UpperNode=NULL;
	if (parent) parent->AddMember(this,varIdentifier);
}


emRec::~emRec()
{
	while (UpperNode && UpperNode->IsListener()) {
		((emRecListener*)UpperNode)->SetListenedRec(NULL);
	}
}


emRec * emRec::GetParent()
{
	emRecNode * n;

	for (n=UpperNode; n && n->IsListener(); n=n->UpperNode);
	return (emRec*)n;
}


const emRec * emRec::GetParent() const
{
	const emRecNode * n;

	for (n=UpperNode; n && n->IsListener(); n=n->UpperNode);
	return (const emRec*)n;
}


emRec * emRec::GetRoot()
{
	emRecNode * n;
	emRec * root;

	root=this;
	for (n=UpperNode; n; n=n->UpperNode) {
		if (!n->IsListener()) root=(emRec*)n;
	}
	return root;
}


const emRec * emRec::GetRoot() const
{
	const emRecNode * n;
	const emRec * root;

	root=this;
	for (n=UpperNode; n; n=n->UpperNode) {
		if (!n->IsListener()) root=(const emRec*)n;
	}
	return root;
}


const char * emRec::GetFormatName() const
{
	return NULL;
}


void emRec::TryLoad(const emString & filePath) throw(emString)
{
	emRecFileReader reader;

	reader.TryStartReading(*this,filePath);
	reader.TryFinishReading();
}


void emRec::TrySave(const emString & filePath) throw(emString)
{
	emRecFileWriter writer;

	writer.TryStartWriting(*this,filePath);
	writer.TryFinishWriting();
}


void emRec::TryLoadFromMem(const char * buf, int len) throw(emString)
{
	emRecMemReader reader;

	reader.TryStartReading(*this,buf,len);
	reader.TryFinishReading();
}


void emRec::TryLoadFromMem(const emArray<char> & buf) throw(emString)
{
	TryLoadFromMem(buf.Get(),buf.GetCount());
}


void emRec::SaveToMem(emArray<char> & buf)
{
	emRecMemWriter writer;

	try {
		writer.TryStartWriting(*this,buf);
		writer.TryFinishWriting();
	}
	catch (emString errorMessage) {
		emFatalError(
			"Unexpected error from emRecMemWriter: %s",
			errorMessage.Get()
		);
	}
}


void emRec::TryCopy(emRec & source) throw(emString)
{
	emArray<char> buf;

	buf.SetTuningLevel(4);
	source.SaveToMem(buf);
	TryLoadFromMem(buf);
}


void emRec::Copy(emRec & source)
{
	try {
		TryCopy(source);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
}


void emRec::CheckIdentifier(const char * identifier)
{
	int i;

	if (
		(identifier[0]>='a' && identifier[0]<='z') ||
		(identifier[0]>='A' && identifier[0]<='Z') ||
		identifier[0]=='_'
	) {
		i=0;
		do {
			i++;
			if (identifier[i]==0) return;
		} while (
			(identifier[i]>='a' && identifier[i]<='z') ||
			(identifier[i]>='A' && identifier[i]<='Z') ||
			(identifier[i]>='0' && identifier[i]<='9') ||
			identifier[i]=='_'
		);
	}
	emFatalError("emRec: '%s' is not a valid identifier.",identifier);
}


void emRec::BeTheParentOf(emRec * child)
{
	emRecNode * * pn;
	emRecNode * n;

	n=child;
	do {
		pn=&n->UpperNode;
		n=*pn;
	} while (n && n->IsListener());
	*pn=this;
}


bool emRec::IsListener() const
{
	return false;
}


void emRec::ChildChanged()
{
	if (!UpperNode) return;
	UpperNode->ChildChanged();
}


//==============================================================================
//=============================== emRecListener ================================
//==============================================================================

emRecListener::emRecListener(emRec * rec)
{
	UpperNode=NULL;
	Rec=NULL;
	SetListenedRec(rec);
}


emRecListener::~emRecListener()
{
	SetListenedRec(NULL);
}


void emRecListener::SetListenedRec(emRec * rec)
{
	emRecNode * * pn;
	emRecNode * n;

	if (Rec!=rec) {
		if (Rec) {
			n=Rec;
			do {
				pn=&n->UpperNode;
				n=*pn;
			} while (n!=this);
			*pn=UpperNode;
			UpperNode=NULL;
		}
		Rec=rec;
		if (Rec) {
			n=Rec;
			do {
				pn=&n->UpperNode;
				n=*pn;
			} while (n && n->IsListener());
			UpperNode=n;
			*pn=this;
		}
	}
}


bool emRecListener::IsListener() const
{
	return true;
}


void emRecListener::ChildChanged()
{
	OnRecChanged();
	if (!UpperNode) return;
	UpperNode->ChildChanged();
}


//==============================================================================
//================================= emBoolRec ==================================
//==============================================================================

emBoolRec::emBoolRec(bool defaultValue)
{
	DefaultValue=defaultValue;
	Value=DefaultValue;
}


emBoolRec::emBoolRec(
	emStructRec * parent, const char * varIdentifier, bool defaultValue
)
	: emRec(parent,varIdentifier)
{
	DefaultValue=defaultValue;
	Value=defaultValue;
}


void emBoolRec::Set(bool value)
{
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emBoolRec::Invert()
{
	Value=!Value;
	Changed();
}


void emBoolRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emBoolRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emBoolRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	int i;

	if (reader.TryPeekNext()==emRecReader::ET_INT) {
		i=reader.TryReadInt();
		if      (i==1) Set(true);
		else if (i==0) Set(false);
		else reader.ThrowSyntaxError();
	}
	else {
		idf=reader.TryReadIdentifier();
		if      (strcasecmp(idf,"yes"  )==0) Set(true);
		else if (strcasecmp(idf,"no"   )==0) Set(false);
		else if (strcasecmp(idf,"y"    )==0) Set(true);
		else if (strcasecmp(idf,"n"    )==0) Set(false);
		else if (strcasecmp(idf,"true" )==0) Set(true);
		else if (strcasecmp(idf,"false")==0) Set(false);
		else reader.ThrowSyntaxError();
	}
}


bool emBoolRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emBoolRec::QuitReading()
{
}


void emBoolRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteIdentifier(Value ? "yes" : "no");
}


bool emBoolRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emBoolRec::QuitWriting()
{
}


emUInt64 emBoolRec::CalcRecMemNeed() const
{
	return sizeof(emBoolRec);
}


//==============================================================================
//================================== emIntRec ==================================
//==============================================================================

emIntRec::emIntRec(int defaultValue, int minValue, int maxValue)
{
	if (maxValue<minValue) maxValue=minValue;
	if (defaultValue<minValue) defaultValue=minValue;
	if (defaultValue>maxValue) defaultValue=maxValue;
	DefaultValue=defaultValue;
	MinValue=minValue;
	MaxValue=maxValue;
	Value=defaultValue;
}


emIntRec::emIntRec(
	emStructRec * parent, const char * varIdentifier, int defaultValue,
	int minValue, int maxValue
)
	: emRec(parent,varIdentifier)
{
	if (maxValue<minValue) maxValue=minValue;
	if (defaultValue<minValue) defaultValue=minValue;
	if (defaultValue>maxValue) defaultValue=maxValue;
	DefaultValue=defaultValue;
	MinValue=minValue;
	MaxValue=maxValue;
	Value=defaultValue;
}


void emIntRec::Set(int value)
{
	if (value<MinValue) value=MinValue;
	if (value>MaxValue) value=MaxValue;
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emIntRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emIntRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emIntRec::TryStartReading(emRecReader & reader) throw(emString)
{
	int i;

	i=reader.TryReadInt();
	if (i<MinValue) reader.ThrowElemError("Number too small.");
	if (i>MaxValue) reader.ThrowElemError("Number too large.");
	Set(i);
}


bool emIntRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emIntRec::QuitReading()
{
}


void emIntRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteInt(Value);
}


bool emIntRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emIntRec::QuitWriting()
{
}


emUInt64 emIntRec::CalcRecMemNeed() const
{
	return sizeof(emIntRec);
}


//==============================================================================
//================================ emDoubleRec =================================
//==============================================================================

emDoubleRec::emDoubleRec(double defaultValue, double minValue, double maxValue)
{
	if (maxValue<minValue) maxValue=minValue;
	if (defaultValue<minValue) defaultValue=minValue;
	if (defaultValue>maxValue) defaultValue=maxValue;
	DefaultValue=defaultValue;
	MinValue=minValue;
	MaxValue=maxValue;
	Value=defaultValue;
}


emDoubleRec::emDoubleRec(
	emStructRec * parent, const char * varIdentifier, double defaultValue,
	double minValue, double maxValue
)
	: emRec(parent,varIdentifier)
{
	if (maxValue<minValue) maxValue=minValue;
	if (defaultValue<minValue) defaultValue=minValue;
	if (defaultValue>maxValue) defaultValue=maxValue;
	DefaultValue=defaultValue;
	MinValue=minValue;
	MaxValue=maxValue;
	Value=defaultValue;
}


emDoubleRec::~emDoubleRec()
{
}


void emDoubleRec::Set(double value)
{
	if (value<MinValue) value=MinValue;
	if (value>MaxValue) value=MaxValue;
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emDoubleRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emDoubleRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emDoubleRec::TryStartReading(emRecReader & reader) throw(emString)
{
	double d;

	d=reader.TryReadDouble();
	if (d<MinValue) reader.ThrowElemError("Number too small.");
	if (d>MaxValue) reader.ThrowElemError("Number too large.");
	Set(d);
}


bool emDoubleRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emDoubleRec::QuitReading()
{
}


void emDoubleRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteDouble(Value);
}


bool emDoubleRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emDoubleRec::QuitWriting()
{
}


emUInt64 emDoubleRec::CalcRecMemNeed() const
{
	return sizeof(emDoubleRec);
}


//==============================================================================
//================================= emEnumRec ==================================
//==============================================================================

emEnumRec::emEnumRec(int defaultValue, const char * identifier0, ...)
{
	va_list args;

	va_start(args,identifier0);
	Init(defaultValue,identifier0,args);
	va_end(args);
}


emEnumRec::emEnumRec(
	emStructRec * parent, const char * varIdentifier, int defaultValue,
	const char * identifier0, ...
)
	: emRec(parent,varIdentifier)
{
	va_list args;

	va_start(args,identifier0);
	Init(defaultValue,identifier0,args);
	va_end(args);
}


emEnumRec::~emEnumRec()
{
	free(Identifiers);
}


void emEnumRec::Set(int value)
{
	if (value<0) value=0;
	if (value>=IdentifierCount) value=IdentifierCount-1;
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


const char * emEnumRec::GetIdentifierOf(int value)
{
	if (value<0) return NULL;
	if (value>=IdentifierCount) return NULL;
	return Identifiers[value];
}


int emEnumRec::GetValueOf(const char * identifier)
{
	int val;

	for (val=IdentifierCount-1; val>=0; val--) {
		if (strcasecmp(identifier,Identifiers[val])==0) break;
	}
	return val;
}


void emEnumRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emEnumRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emEnumRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	int val;

	if (reader.TryPeekNext()==emRecReader::ET_INT) {
		val=reader.TryReadInt();
		if (val<0 || val>=IdentifierCount) {
			reader.ThrowElemError("Value out of range.");
		}
	}
	else {
		idf=reader.TryReadIdentifier();
		val=GetValueOf(idf);
		if (val<0) reader.ThrowElemError("Unknown identifier.");
	}
	Set(val);
}


bool emEnumRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emEnumRec::QuitReading()
{
}


void emEnumRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteIdentifier(Identifiers[Value]);
}


bool emEnumRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emEnumRec::QuitWriting()
{
}


emUInt64 emEnumRec::CalcRecMemNeed() const
{
	return sizeof(emEnumRec) + IdentifierCount*sizeof(const char *);
}


void emEnumRec::Init(int defaultValue, const char * identifier0, va_list args)
{
	const char * idf[512];
	int cnt;

	idf[0]=identifier0;
	for (cnt=1; ; cnt++) {
		if (cnt>=(int)(sizeof(idf)/sizeof(const char *))) {
			emFatalError("emEnumRec: Too many identifiers.");
		}
		idf[cnt]=va_arg(args,const char *);
		if (!idf[cnt]) break;
		CheckIdentifier(idf[cnt]);
	}
	Identifiers=(const char * *)malloc(sizeof(const char*)*cnt);
	memcpy(Identifiers,idf,sizeof(const char *)*cnt);
	IdentifierCount=cnt;
	if (defaultValue<0) defaultValue=0;
	if (defaultValue>=IdentifierCount) defaultValue=IdentifierCount-1;
	DefaultValue=defaultValue;
	Value=DefaultValue;
}


//==============================================================================
//================================= emFlagsRec =================================
//==============================================================================

emFlagsRec::emFlagsRec(int defaultValue, const char * identifier0, ...)
{
	va_list args;

	va_start(args,identifier0);
	Init(defaultValue,identifier0,args);
	va_end(args);
}


emFlagsRec::emFlagsRec(
	emStructRec * parent, const char * varIdentifier, int defaultValue,
	const char * identifier0, ...
)
	: emRec(parent,varIdentifier)
{
	va_list args;

	va_start(args,identifier0);
	Init(defaultValue,identifier0,args);
	va_end(args);
}


emFlagsRec::~emFlagsRec()
{
	free(Identifiers);
}


void emFlagsRec::Set(int value)
{
	value&=(1<<IdentifierCount)-1;
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


const char * emFlagsRec::GetIdentifierOf(int bit)
{
	if (bit<0 || bit>=IdentifierCount) return NULL;
	return Identifiers[bit];
}


int emFlagsRec::GetBitOf(const char * identifier)
{
	int bit;

	for (bit=IdentifierCount-1; bit>=0; bit--) {
		if (strcasecmp(identifier,Identifiers[bit])==0) break;
	}
	return bit;
}


void emFlagsRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emFlagsRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emFlagsRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	int val, bit;

	if (reader.TryPeekNext()==emRecReader::ET_INT) {
		val=reader.TryReadInt();
		if (val&~((1<<IdentifierCount)-1)) {
			reader.ThrowElemError("Value out of range.");
		}
	}
	else {
		reader.TryReadCertainDelimiter('{');
		val=0;
		while (reader.TryPeekNext()==emRecReader::ET_IDENTIFIER) {
			idf=reader.TryReadIdentifier();
			bit=GetBitOf(idf);
			if (bit<0) reader.ThrowElemError("Unknown identifier.");
			val|=1<<bit;
		}
		reader.TryReadCertainDelimiter('}');
	}
	Set(val);
}


bool emFlagsRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emFlagsRec::QuitReading()
{
}


void emFlagsRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	bool addSpaceBeforeNext;
	int bit;

	writer.TryWriteDelimiter('{');
	addSpaceBeforeNext=false;
	for (bit=0; bit<IdentifierCount; bit++) {
		if (Value&(1<<bit)) {
			if (addSpaceBeforeNext) writer.TryWriteSpace();
			writer.TryWriteIdentifier(Identifiers[bit]);
			addSpaceBeforeNext=true;
		}
	}
	writer.TryWriteDelimiter('}');
}


bool emFlagsRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emFlagsRec::QuitWriting()
{
}


emUInt64 emFlagsRec::CalcRecMemNeed() const
{
	return sizeof(emFlagsRec) + IdentifierCount*sizeof(const char *);
}


void emFlagsRec::Init(int defaultValue, const char * identifier0, va_list args)
{
	const char * idf[32];
	const char * p;
	int cnt;

	idf[0]=identifier0;
	for (cnt=1; ; cnt++) {
		p=va_arg(args,const char *);
		if (!p) break;
		if (cnt>=32) emFatalError("emFlagsRec: Too many identifiers.");
		CheckIdentifier(p);
		idf[cnt]=p;
	}
	Identifiers=(const char * *)malloc(sizeof(const char*)*cnt);
	memcpy(Identifiers,idf,sizeof(const char *)*cnt);
	IdentifierCount=cnt;
	defaultValue&=(1<<IdentifierCount)-1;
	DefaultValue=defaultValue;
	Value=DefaultValue;
}


//==============================================================================
//=============================== emAlignmentRec ===============================
//==============================================================================

emAlignmentRec::emAlignmentRec(emAlignment defaultValue)
{
	DefaultValue=defaultValue;
	Value=defaultValue;
}


emAlignmentRec::emAlignmentRec(
	emStructRec * parent, const char * varIdentifier, emAlignment defaultValue
)
	: emRec(parent,varIdentifier)
{
	DefaultValue=defaultValue;
	Value=defaultValue;
}


emAlignmentRec::~emAlignmentRec()
{
}


void emAlignmentRec::Set(emAlignment value)
{
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emAlignmentRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emAlignmentRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emAlignmentRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	char delimiter;
	emAlignment val;

	val=0;
	for (;;) {
		idf=reader.TryReadIdentifier();
		if      (strcasecmp(idf,"top"   )==0) val|=EM_ALIGN_TOP;
		else if (strcasecmp(idf,"bottom")==0) val|=EM_ALIGN_BOTTOM;
		else if (strcasecmp(idf,"left"  )==0) val|=EM_ALIGN_LEFT;
		else if (strcasecmp(idf,"right" )==0) val|=EM_ALIGN_RIGHT;
		else if (strcasecmp(idf,"center")==0) val|=EM_ALIGN_CENTER;
		else reader.ThrowElemError("Unknown alignment identifier.");
		if (reader.TryPeekNext(&delimiter)!=emRecReader::ET_DELIMITER) break;
		if (delimiter!='-') break;
		reader.TryReadCertainDelimiter('-');
	}
	Set(val);
}


bool emAlignmentRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emAlignmentRec::QuitReading()
{
}


void emAlignmentRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	bool someWritten;

	someWritten = false;
	if (Value&EM_ALIGN_TOP) {
		writer.TryWriteIdentifier("top");
		someWritten=true;
	}
	if (Value&EM_ALIGN_BOTTOM) {
		if (someWritten) writer.TryWriteDelimiter('-');
		writer.TryWriteIdentifier("bottom");
		someWritten=true;
	}
	if (Value&EM_ALIGN_LEFT) {
		if (someWritten) writer.TryWriteDelimiter('-');
		writer.TryWriteIdentifier("left");
		someWritten=true;
	}
	if (Value&EM_ALIGN_RIGHT) {
		if (someWritten) writer.TryWriteDelimiter('-');
		writer.TryWriteIdentifier("right");
		someWritten=true;
	}
	if (!someWritten) writer.TryWriteIdentifier("center");
}


bool emAlignmentRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emAlignmentRec::QuitWriting()
{
}


emUInt64 emAlignmentRec::CalcRecMemNeed() const
{
	return sizeof(emAlignmentRec);
}


//==============================================================================
//================================ emStringRec =================================
//==============================================================================

emStringRec::emStringRec(const emString & defaultValue)
	: DefaultValue(defaultValue), Value(defaultValue)
{
}


emStringRec::~emStringRec()
{
}


emStringRec::emStringRec(
	emStructRec * parent, const char * varIdentifier,
	const emString & defaultValue
)
	: emRec(parent,varIdentifier),
	DefaultValue(defaultValue),
	Value(defaultValue)
{
}


void emStringRec::Set(const emString & value)
{
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emStringRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emStringRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emStringRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * val;

	val=reader.TryReadQuoted();
	Set(val);
}


bool emStringRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emStringRec::QuitReading()
{
}


void emStringRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteQuoted(Value);
}


bool emStringRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emStringRec::QuitWriting()
{
}


emUInt64 emStringRec::CalcRecMemNeed() const
{
	return sizeof(emStringRec) + DefaultValue.GetLen() + Value.GetLen() + 32;
}


//==============================================================================
//================================= emColorRec =================================
//==============================================================================

emColorRec::emColorRec(emColor defaultValue, bool haveAlpha)
{
	if (!haveAlpha) defaultValue.SetAlpha(255);
	DefaultValue=defaultValue;
	Value=defaultValue;
	HaveAlpha=haveAlpha;
}


emColorRec::emColorRec(
	emStructRec * parent, const char * varIdentifier, emColor defaultValue,
	bool haveAlpha
)
	: emRec(parent,varIdentifier)
{
	if (!haveAlpha) defaultValue.SetAlpha(255);
	DefaultValue=defaultValue;
	Value=defaultValue;
	HaveAlpha=haveAlpha;
}


void emColorRec::Set(emColor value)
{
	if (!HaveAlpha) value.SetAlpha(255);
	if (Value!=value) {
		Value=value;
		Changed();
	}
}


void emColorRec::SetToDefault()
{
	Set(DefaultValue);
}


bool emColorRec::IsSetToDefault() const
{
	return Value==DefaultValue;
}


void emColorRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * str;
	emColor val;
	int i;

	if (reader.TryPeekNext()==emRecReader::ET_QUOTED) {
		str=reader.TryReadQuoted();
		try {
			val.TryParse(str);
		}
		catch (emString errorMessage) {
			reader.ThrowElemError(errorMessage);
		}
	}
	else {
		reader.TryReadCertainDelimiter('{');
		i=reader.TryReadInt();
		if (i<0 || i>255) reader.ThrowElemError("Value out of range.");
		val.SetRed((emByte)i);
		i=reader.TryReadInt();
		if (i<0 || i>255) reader.ThrowElemError("Value out of range.");
		val.SetGreen((emByte)i);
		i=reader.TryReadInt();
		if (i<0 || i>255) reader.ThrowElemError("Value out of range.");
		val.SetBlue((emByte)i);
		if (HaveAlpha) {
			i=reader.TryReadInt();
			if (i<0 || i>255) reader.ThrowElemError("Value out of range.");
			val.SetAlpha((emByte)i);
		}
		reader.TryReadCertainDelimiter('}');
	}
	Set(val);
}


bool emColorRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return true;
}


void emColorRec::QuitReading()
{
}


void emColorRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteDelimiter('{');
	writer.TryWriteInt(Value.GetRed());
	writer.TryWriteSpace();
	writer.TryWriteInt(Value.GetGreen());
	writer.TryWriteSpace();
	writer.TryWriteInt(Value.GetBlue());
	if (HaveAlpha) {
		writer.TryWriteSpace();
		writer.TryWriteInt(Value.GetAlpha());
	}
	writer.TryWriteDelimiter('}');
}


bool emColorRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return true;
}


void emColorRec::QuitWriting()
{
}


emUInt64 emColorRec::CalcRecMemNeed() const
{
	return sizeof(emColorRec);
}


//==============================================================================
//================================ emStructRec =================================
//==============================================================================

emStructRec::emStructRec()
{
	Count=0;
	Capacity=0;
	Members=NULL;
	RWState=NULL;
}


emStructRec::emStructRec(emStructRec * parent, const char * varIdentifier)
	: emRec(parent,varIdentifier)
{
	Count=0;
	Capacity=0;
	Members=NULL;
	RWState=NULL;
}


emStructRec::~emStructRec()
{
	if (RWState) {
		free(RWState);
		RWState=NULL;
	}
	if (Members) {
		free(Members);
		Members=NULL;
	}
}


const char * emStructRec::GetIdentifierOf(int index)
{
	if (index<0 || index>=Count) return NULL;
	return Members[index].Identifier;
}


int emStructRec::GetIndexOf(emRec * member)
{
	int i;

	for (i=Count-1; i>=0; i--) {
		if (member==Members[i].Record) break;
	}
	return i;
}


int emStructRec::GetIndexOf(const char * identifier)
{
	int i;

	for (i=Count-1; i>=0; i--) {
		if (strcasecmp(identifier,Members[i].Identifier)==0) break;
	}
	return i;
}


bool emStructRec::ShallWriteOptionalOnly(const emRec * child) const
{
	return false;
}


void emStructRec::SetToDefault()
{
	int i;

	for (i=0; i<Count; i++) Members[i].Record->SetToDefault();
}


bool emStructRec::IsSetToDefault() const
{
	int i;

	for (i=0; i<Count; i++) {
		if (!Members[i].Record->IsSetToDefault()) return false;
	}
	return true;
}


void emStructRec::TryStartReading(emRecReader & reader) throw(emString)
{
	if (RWState) {
		free(RWState);
		RWState=NULL;
	}
	emStructRec::SetToDefault();
	if (reader.GetRootRec()!=this) {
		reader.TryReadCertainDelimiter('{');
	}
	RWState=(RWStateType*)malloc(sizeof(RWStateType)+Count);
	RWState->Pos=-1;
	RWState->ChildReady=true;
	memset(RWState->Map,0,Count);
}


bool emStructRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	char delimiter;

	if (!RWState->ChildReady) {
		if (Members[RWState->Pos].Record->TryContinueReading(reader)) {
			Members[RWState->Pos].Record->QuitReading();
			RWState->ChildReady=true;
		}
		return false;
	}
	if (reader.GetRootRec()!=this) {
		if (
			reader.TryPeekNext(&delimiter)==emRecReader::ET_DELIMITER &&
			delimiter=='}'
		) {
			reader.TryReadCertainDelimiter('}');
			return true;
		}
	}
	else {
		if (reader.TryPeekNext()==emRecReader::ET_END) {
			return true;
		}
	}
	idf=reader.TryReadIdentifier();
	RWState->Pos=GetIndexOf(idf);
	if (RWState->Pos<0) reader.ThrowElemError("Unknown identifier.");
	if (RWState->Map[RWState->Pos]) reader.ThrowElemError("re-assignment");
	reader.TryReadCertainDelimiter('=');
	Members[RWState->Pos].Record->TryStartReading(reader);
	RWState->ChildReady=false;
	RWState->Map[RWState->Pos]=1;
	return false;
}


void emStructRec::QuitReading()
{
	if (RWState) {
		if (!RWState->ChildReady) {
			Members[RWState->Pos].Record->QuitReading();
		}
		free(RWState);
		RWState=NULL;
	}
}


void emStructRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	if (RWState) {
		free(RWState);
		RWState=NULL;
	}
	if (writer.GetRootRec()!=this) {
		writer.TryWriteDelimiter('{');
		writer.IncIndent();
	}
	RWState=(RWStateType*)malloc(sizeof(RWStateType)+Count);
	RWState->Pos=-1;
	RWState->ChildReady=true;
	RWState->Empty=true;
	memset(RWState->Map,0,Count);
}


bool emStructRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	if (!RWState->ChildReady) {
		if (Members[RWState->Pos].Record->TryContinueWriting(writer)) {
			Members[RWState->Pos].Record->QuitWriting();
			RWState->ChildReady=true;
		}
		return false;
	}
	for (;;) {
		RWState->Pos++;
		if (RWState->Pos>=Count) {
			if (writer.GetRootRec()!=this) {
				writer.DecIndent();
				if (!RWState->Empty) {
					writer.TryWriteNewLine();
					writer.TryWriteIndent();
				}
				writer.TryWriteDelimiter('}');
			}
			return true;
		}
		RWState->Map[RWState->Pos]=1;
		if (
			!Members[RWState->Pos].Record->IsSetToDefault() ||
			!ShallWriteOptionalOnly(Members[RWState->Pos].Record)
		) {
			if (writer.GetRootRec()!=this || !RWState->Empty) {
				writer.TryWriteNewLine();
			}
			writer.TryWriteIndent();
			writer.TryWriteIdentifier(Members[RWState->Pos].Identifier);
			writer.TryWriteSpace();
			writer.TryWriteDelimiter('=');
			writer.TryWriteSpace();
			Members[RWState->Pos].Record->TryStartWriting(writer);
			RWState->ChildReady=false;
			RWState->Empty=false;
			return false;
		}
	}
}


void emStructRec::QuitWriting()
{
	if (RWState) {
		if (!RWState->ChildReady) {
			Members[RWState->Pos].Record->QuitWriting();
		}
		free(RWState);
		RWState=NULL;
	}
}


emUInt64 emStructRec::CalcRecMemNeed() const
{
	emUInt64 sum;
	int i;

	sum=sizeof(emStructRec)+sizeof(MemberType)*Capacity;
	for (i=0; i<Count; i++) sum+=Members[i].Record->CalcRecMemNeed();
	return sum;
}


void emStructRec::AddMember(emRec * member, const char * identifier)
{
	CheckIdentifier(identifier);
	if (Count>=Capacity) {
		Capacity=(Count+1)*2;
		Members=(MemberType*)realloc(Members,sizeof(MemberType)*Capacity);
	}
	Members[Count].Record=member;
	Members[Count].Identifier=identifier;
	Count++;
	BeTheParentOf(member);
	// Changed() is not called here because it should always be the
	// construction phase!
}


//==============================================================================
//================================= emUnionRec =================================
//==============================================================================

emUnionRec::emUnionRec(
	int defaultVariant, const char * identifier0,
	emRecAllocator allocator0, ...
)
{
	va_list args;

	va_start(args,allocator0);
	Init(defaultVariant,identifier0,allocator0,args);
	va_end(args);
}


emUnionRec::emUnionRec(
	emStructRec * parent, const char * varIdentifier, int defaultVariant,
	const char * identifier0, emRecAllocator allocator0, ...
)
	: emRec(parent,varIdentifier)
{
	va_list args;

	va_start(args,allocator0);
	Init(defaultVariant,identifier0,allocator0,args);
	va_end(args);
}


emUnionRec::~emUnionRec()
{
	delete Record;
	delete [] TypeArray;
}


void emUnionRec::SetVariant(int variant)
{
	if (variant<0) variant=0;
	if (variant>=VariantCount) variant=VariantCount-1;
	if (Variant!=variant) {
		Variant=variant;
		delete Record;
		Record=TypeArray[Variant].Allocator();
		BeTheParentOf(Record);
		Changed();
	}
}


const char * emUnionRec::GetIdentifierOf(int variant)
{
	if (variant<0) return NULL;
	if (variant>=VariantCount) return NULL;
	return TypeArray[variant].Identifier;
}


int emUnionRec::GetVariantOf(const char * identifier)
{
	int variant;

	for (variant=VariantCount-1; variant>=0; variant--) {
		if (strcasecmp(identifier,TypeArray[variant].Identifier)==0) break;
	}
	return variant;
}


void emUnionRec::SetToDefault()
{
	SetVariant(DefaultVariant);
	Record->SetToDefault();
}


bool emUnionRec::IsSetToDefault() const
{
	return Variant==DefaultVariant && Record->IsSetToDefault();
}


void emUnionRec::TryStartReading(emRecReader & reader) throw(emString)
{
	const char * idf;
	int v;

	idf=reader.TryReadIdentifier();
	v=GetVariantOf(idf);
	if (v<0) reader.ThrowElemError("Unknown identifier.");
	SetVariant(v);
	reader.TryReadCertainDelimiter(':');
	Record->TryStartReading(reader);
}


bool emUnionRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	return Record->TryContinueReading(reader);
}


void emUnionRec::QuitReading()
{
	Record->QuitReading();
}


void emUnionRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	writer.TryWriteIdentifier(TypeArray[Variant].Identifier);
	writer.TryWriteDelimiter(':');
	writer.TryWriteSpace();
	Record->TryStartWriting(writer);
}


bool emUnionRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	return Record->TryContinueWriting(writer);
}


void emUnionRec::QuitWriting()
{
	Record->QuitWriting();
}


emUInt64 emUnionRec::CalcRecMemNeed() const
{
	return
		Record->CalcRecMemNeed() +
		sizeof(emUnionRec) +
		VariantCount*sizeof(VariantType);
}


void emUnionRec::Init(
	int defaultVariant, const char * identifier0, emRecAllocator allocator0,
	va_list args
)
{
	VariantType v[512];
	int cnt;

	v[0].Allocator=allocator0;
	v[0].Identifier=identifier0;
	for (cnt=1; ; cnt++) {
		if (cnt>=(int)(sizeof(v)/sizeof(VariantType))) {
			emFatalError("emUnionRec: Too many variants.");
		}
		v[cnt].Identifier=va_arg(args,const char *);
		if (!v[cnt].Identifier) break;
		v[cnt].Allocator=va_arg(args,emRecAllocator);
		if (!v[cnt].Allocator) break;
		CheckIdentifier(v[cnt].Identifier);
	}
	TypeArray=new VariantType[cnt];
	memcpy(TypeArray,v,sizeof(VariantType)*cnt);
	VariantCount=cnt;
	if (defaultVariant<0) defaultVariant=0;
	if (defaultVariant>=VariantCount) defaultVariant=VariantCount-1;
	DefaultVariant=defaultVariant;
	Variant=DefaultVariant;
	Record=TypeArray[Variant].Allocator();
	BeTheParentOf(Record);
}


//==============================================================================
//================================= emArrayRec =================================
//==============================================================================

emArrayRec::emArrayRec(emRecAllocator allocator, int minCount, int maxCount)
{
	Init(allocator,minCount,maxCount);
}


emArrayRec::emArrayRec(
	emStructRec * parent, const char * varIdentifier,
	emRecAllocator allocator, int minCount, int maxCount
)
	: emRec(parent,varIdentifier)
{
	Init(allocator,minCount,maxCount);
}


emArrayRec::~emArrayRec()
{
	int i;

	if (Array) {
		for (i=0; i<Count; i++) delete Array[i];
		free(Array);
	}
}


void emArrayRec::SetCount(int count)
{
	if (count<Count) Remove(count,Count-count);
	else Insert(Count,count-Count);
}


void emArrayRec::Insert(int index, int insCount)
{
	int i,n;

	if (insCount>MaxCount-Count) insCount=MaxCount-Count;
	if (insCount<=0) return;
	if (index<0) index=0;
	if (index>Count) index=Count;
	Count+=insCount;
	if (Capacity<Count) {
		Capacity=Count*2;
		if (Capacity>MaxCount) Capacity=MaxCount;
		Array=(emRec**)realloc(Array,sizeof(emRec*)*Capacity);
	}
	n=Count-index-insCount;
	if (n>0) memmove(Array+Count-n,Array+index,sizeof(emRec*)*n);
	for (i=index; i<index+insCount; i++) {
		Array[i]=Allocator();
		BeTheParentOf(Array[i]);
	}
	if (RWPos>=index) RWPos+=insCount;
	Changed();
}


void emArrayRec::Remove(int index, int remCount)
{
	int i,n;

	if (index<0) { remCount+=index; index=0; }
	if (remCount>Count-index) remCount=Count-index;
	if (remCount>Count-MinCount) remCount=Count-MinCount;
	if (remCount<=0) return;
	if (RWPos>=index) {
		if (RWPos>=index+remCount) {
			RWPos-=remCount;
		}
		else {
			RWPos=index-1;
			RWChildReady=true;
		}
	}
	for (i=index; i<index+remCount; i++) {
		delete Array[i];
	}
	n=Count-index-remCount;
	if (n>0) memmove(Array+index,Array+Count-n,sizeof(emRec*)*n);
	Count-=remCount;
	if (Capacity>=Count*3) {
		Capacity=Count*2;
		if (Capacity>MaxCount) Capacity=MaxCount;
		if (Capacity>0) {
			Array=(emRec**)realloc(Array,sizeof(emRec*)*Capacity);
		}
		else {
			free(Array);
			Array=NULL;
		}
	}
	Changed();
}


void emArrayRec::SetToDefault()
{
	int i;

	SetCount(MinCount);
	for (i=0; i<Count; i++) Array[i]->SetToDefault();
}


bool emArrayRec::IsSetToDefault() const
{
	int i;

	if (Count!=MinCount) return false;
	for (i=0; i<Count; i++) {
		if (!Array[i]->IsSetToDefault()) return false;
	}
	return true;
}


void emArrayRec::TryStartReading(emRecReader & reader) throw(emString)
{
	SetCount(MinCount);
	if (reader.GetRootRec()!=this) {
		reader.TryReadCertainDelimiter('{');
	}
	RWPos=-1;
	RWChildReady=true;
}


bool emArrayRec::TryContinueReading(emRecReader & reader) throw(emString)
{
	char delimiter;
	int i;

	if (!RWChildReady) {
		if (Array[RWPos]->TryContinueReading(reader)) {
			Array[RWPos]->QuitReading();
			RWChildReady=true;
		}
		return false;
	}
	RWPos++;
	if (reader.GetRootRec()!=this) {
		if (
			reader.TryPeekNext(&delimiter)==emRecReader::ET_DELIMITER &&
			delimiter=='}'
		) {
			reader.TryReadCertainDelimiter('}');
			if (RWPos<MinCount) reader.ThrowElemError("Too few elements.");
			return true;
		}
	}
	else {
		if (reader.TryPeekNext()==emRecReader::ET_END) {
			if (RWPos<MinCount) reader.ThrowElemError("Too few elements.");
			return true;
		}
	}
	if (RWPos>=MaxCount) reader.ThrowElemError("Too many elements.");
	if (RWPos>=Count) {
		i=RWPos;
		SetCount(RWPos+1);
		RWPos=i;
	}
	Array[RWPos]->TryStartReading(reader);
	RWChildReady=false;
	return false;
}


void emArrayRec::QuitReading()
{
	if (!RWChildReady) {
		if (RWPos>=0 && RWPos<Count) Array[RWPos]->QuitReading();
		RWChildReady=true;
	}
	RWPos=-1;
}


void emArrayRec::TryStartWriting(emRecWriter & writer) throw(emString)
{
	if (writer.GetRootRec()!=this) {
		writer.TryWriteDelimiter('{');
		writer.IncIndent();
	}
	RWPos=-1;
	RWChildReady=true;
}


bool emArrayRec::TryContinueWriting(emRecWriter & writer) throw(emString)
{
	if (!RWChildReady) {
		if (Array[RWPos]->TryContinueWriting(writer)) {
			Array[RWPos]->QuitWriting();
			RWChildReady=true;
		}
		return false;
	}
	RWPos++;
	if (RWPos<Count) {
		if (writer.GetRootRec()!=this || RWPos>0) writer.TryWriteNewLine();
		writer.TryWriteIndent();
		Array[RWPos]->TryStartWriting(writer);
		RWChildReady=false;
		return false;
	}
	if (writer.GetRootRec()!=this) {
		writer.DecIndent();
		if (Count>0) {
			writer.TryWriteNewLine();
			writer.TryWriteIndent();
		}
		writer.TryWriteDelimiter('}');
	}
	return true;
}


void emArrayRec::QuitWriting()
{
	if (!RWChildReady) {
		if (RWPos>=0 && RWPos<Count) Array[RWPos]->QuitWriting();
		RWChildReady=true;
	}
	RWPos=-1;
}


emUInt64 emArrayRec::CalcRecMemNeed() const
{
	emUInt64 sum;
	int i;

	sum=sizeof(emArrayRec)+sizeof(emRec*)*Capacity;
	for (i=0; i<Count; i++) sum+=Array[i]->CalcRecMemNeed();
	return sum;
}


void emArrayRec::Init(emRecAllocator allocator, int minCount, int maxCount)
{
	int i;

	if (minCount<0) minCount=0;
	if (maxCount<minCount) maxCount=minCount;
	Allocator=allocator;
	MinCount=minCount;
	MaxCount=maxCount;
	Count=MinCount;
	Capacity=Count*2;
	if (Capacity>MaxCount) Capacity=MaxCount;
	if (Capacity>0) {
		Array=(emRec**)malloc(sizeof(emRec*)*Capacity);
		for (i=0; i<Count; i++) {
			Array[i]=Allocator();
			BeTheParentOf(Array[i]);
		}
	}
	else {
		Array=NULL;
	}
	RWPos=-1;
	RWChildReady=true;
}


//==============================================================================
//================================ emRecReader =================================
//==============================================================================

emRecReader::emRecReader()
{
	Root=NULL;
	RootQuitPending=false;
	ClosePending=false;
	Line=1;
	NextEaten=true;
	NextLine=1;
	NextType=ET_END;
	NextDelimiter=0;
	NextBuf=NULL;
	NextBufSize=0;
	NextInt=0;
	NextDouble=0.0;
	NextChar=-1;
}


emRecReader::~emRecReader()
{
	// Never do a Root->QuitReading() here, because of destruction of
	// emRecFileModel...
	Root=NULL;
	if (NextBuf) {
		free(NextBuf);
		NextBuf=NULL;
	}
}


void emRecReader::TryStartReading(emRec & root) throw(emString)
{
	const char * formatName;
	emString magic;
	int mlen,rlen;

	try {
		ClosePending=true;
		Root=&root;
		Line=1;
		NextLine=1;
		NextEaten=true;
		NextChar=-1;
		formatName=Root->GetFormatName();
		if (formatName) {
			magic=emString("#%rec:")+formatName+"%";
			mlen=magic.GetLen();
			SetMinNextBufSize(mlen);
			rlen=TryRead(NextBuf,mlen);
			if (rlen!=mlen || memcmp(NextBuf,magic.Get(),mlen)!=0) {
				throw emString::Format(
					"File format of \"%s\" is not \"rec:%s\".",
					GetSourceName(),
					formatName
				);
			}
		}
		TryNextChar();
		RootQuitPending=true;
		Root->TryStartReading(*this);
	}
	catch (emString errorMessage) {
		QuitReading();
		throw errorMessage;
	}
}


bool emRecReader::TryContinueReading() throw(emString)
{
	try {
		if (Root) {
			if (!Root->TryContinueReading(*this)) return false;
			RootQuitPending=false;
			Root->QuitReading();
			if (NextEaten) TryParseNext();
			Line=NextLine;
			if (NextType!=ET_END) ThrowElemError("Unexpected content.");
			ClosePending=false;
			TryClose();
			QuitReading();
		}
		return true;
	}
	catch (emString errorMessage) {
		QuitReading();
		throw errorMessage;
	}
}


void emRecReader::TryFinishReading() throw(emString)
{
	for (;;) {
		if (TryContinueReading()) break;
	}
}


void emRecReader::QuitReading()
{
	if (Root && RootQuitPending) Root->QuitReading();
	if (ClosePending) {
		try {
			TryClose();
		}
		catch (emString) {
		}
	}
	Root=NULL;
	RootQuitPending=false;
	ClosePending=false;
	Line=1;
	NextEaten=true;
	NextLine=1;
	NextType=ET_END;
	NextDelimiter=0;
	if (NextBuf) {
		free(NextBuf);
		NextBuf=NULL;
		NextBufSize=0;
	}
	NextInt=0;
	NextDouble=0.0;
	NextChar=-1;
}


emRecReader::ElementType emRecReader::TryPeekNext(
	char * pDelimiter
) throw(emString)
{
	if (NextEaten) TryParseNext();
	if (pDelimiter) {
		if (NextType==ET_DELIMITER) *pDelimiter=NextDelimiter;
		else *pDelimiter=0;
	}
	return NextType;
}


char emRecReader::TryReadDelimiter() throw(emString)
{
	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType!=ET_DELIMITER) ThrowElemError("Delimiter expected.");
	return NextDelimiter;
}


void emRecReader::TryReadCertainDelimiter(char delimiter) throw(emString)
{
	char tmp[256];

	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType!=ET_DELIMITER || NextDelimiter!=delimiter) {
		sprintf(tmp,"'%c' expected.",delimiter);
		ThrowElemError(tmp);
	}
}


const char * emRecReader::TryReadIdentifier() throw(emString)
{
	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType!=ET_IDENTIFIER) ThrowElemError("Identifier expected.");
	return NextBuf;
}


int emRecReader::TryReadInt() throw(emString)
{
	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType!=ET_INT) ThrowElemError("Integer expected.");
	return NextInt;
}


double emRecReader::TryReadDouble() throw(emString)
{
	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType==ET_INT) return (double)NextInt;
	if (NextType==ET_DOUBLE) return NextDouble;
	ThrowElemError("Floating point number expected.");
	return 0;
}


const char * emRecReader::TryReadQuoted() throw(emString)
{
	if (NextEaten) TryParseNext();
	Line=NextLine;
	NextEaten=true;
	if (NextType!=ET_QUOTED) ThrowElemError("Quoted string expected.");
	return NextBuf;
}


void emRecReader::ThrowElemError(const char * text) const throw(emString)
{
	throw emString::Format("File \"%s\", line %d: %s",GetSourceName(),Line,text);
}


void emRecReader::ThrowSyntaxError() const throw(emString)
{
	ThrowElemError("Syntax error.");
}


void emRecReader::SetMinNextBufSize(int minSize)
{
	if (NextBufSize<minSize) {
		NextBufSize=minSize+256;
		NextBuf=(char*)realloc(NextBuf,NextBufSize);
	}
}


void emRecReader::TryNextChar() throw(emString)
{
	char buf[1];

	if (TryRead(buf,1)!=1) NextChar=-1;
	else NextChar=(int)((unsigned char)buf[0]);
}


void emRecReader::TryParseNext() throw(emString)
{
	int i,j,k,n;

	NextEaten=false;

	// Parse white spaces, comments and end-of-file.
	for (;;) {
		if (NextChar<=0x20) {
			if (NextChar==0x20 || NextChar==0x09) {
				TryNextChar();
			}
			else if (NextChar==0x0a) {
				NextLine++;
				TryNextChar();
			}
			else if (NextChar==0x0d) {
				NextLine++;
				TryNextChar();
				if (NextChar==0x0a) TryNextChar();
			}
			else if (NextChar==-1) {
				NextType=ET_END;
				return;
			}
			else {
				TryNextChar();
			}
		}
		else if (NextChar=='#') {
			do {
				TryNextChar();
			} while (NextChar!='\n' && NextChar!='\r' && NextChar!=-1);
		}
		else {
			break;
		}
	}

	// Parse identifier.
	if (
		(NextChar>='a' && NextChar<='z') ||
		(NextChar>='A' && NextChar<='Z') ||
		NextChar=='_'
	) {
		n=0;
		do {
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			TryNextChar();
		} while (
			(NextChar>='a' && NextChar<='z') ||
			(NextChar>='A' && NextChar<='Z') ||
			(NextChar>='0' && NextChar<='9') ||
			NextChar=='_'
		);
		SetMinNextBufSize(n+1);
		NextBuf[n]=0;
		NextType=ET_IDENTIFIER;
		return;
	}

	// Parse quoted String.
	if (NextChar=='"') {
		for (n=0; ; ) {
			TryNextChar();
			if (NextChar=='"') {
				TryNextChar();
				break;
			}
			if (NextChar=='\\') {
				SetMinNextBufSize(n+1);
				NextBuf[n++]='\\';
				TryNextChar();
			}
			if (NextChar==-1) {
				Line=NextLine;
				ThrowElemError("Unterminated string.");
			}
			if (
				NextChar==0x0d ||
				(NextChar==0x0a && (n<=0 || NextBuf[n-1]!=0x0d))
			) NextLine++;
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			if (n>10000000) {
				Line=NextLine;
				ThrowElemError("String too long.");
			}
		}
		SetMinNextBufSize(n+1);
		NextBuf[n]=0;
		for (j=0, i=0; i<n; i++) {
			if (NextBuf[i]=='\\') {
				i++;
				if      (NextBuf[i]=='a') NextBuf[j++]=0x07;
				else if (NextBuf[i]=='b') NextBuf[j++]=0x08;
				else if (NextBuf[i]=='e') NextBuf[j++]=0x1b;
				else if (NextBuf[i]=='f') NextBuf[j++]=0x0c;
				else if (NextBuf[i]=='n') NextBuf[j++]=0x0a;
				else if (NextBuf[i]=='r') NextBuf[j++]=0x0d;
				else if (NextBuf[i]=='t') NextBuf[j++]=0x09;
				else if (NextBuf[i]=='v') NextBuf[j++]=0x0b;
				else if (NextBuf[i]=='x') {
					i++;
					if (NextBuf[i]>='0' && NextBuf[i]<='9') {
						k=NextBuf[i]-'0';
					}
					else if (NextBuf[i]>='a' && NextBuf[i]<='f') {
						k=NextBuf[i]-('a'-10);
					}
					else if (NextBuf[i]>='A' && NextBuf[i]<='F') {
						k=NextBuf[i]-('A'-10);
					}
					else {
						k=-1;
					}
					if (k>=0) {
						i++;
						if (NextBuf[i]>='0' && NextBuf[i]<='9') {
							k=(k<<4)|(NextBuf[i]-'0');
						}
						else if (NextBuf[i]>='a' && NextBuf[i]<='f') {
							k=(k<<4)|(NextBuf[i]-('a'-10));
						}
						else if (NextBuf[i]>='A' && NextBuf[i]<='F') {
							k=(k<<4)|(NextBuf[i]-('A'-10));
						}
						else {
							i--;
						}
						NextBuf[j++]=(char)k;
					}
					else {
						NextBuf[j++]='\\';
						NextBuf[j++]='x';
						i--;
					}
				}
				else if (NextBuf[i]>='0' && NextBuf[i]<='7') {
					k=NextBuf[i]-'0';
					i++;
					if (NextBuf[i]>='0' && NextBuf[i]<='7') {
						k=(k<<3)|(NextBuf[i]-'0');
						i++;
						if (NextBuf[i]>='0' && NextBuf[i]<='7') {
							k=(k<<3)|(NextBuf[i]-'0');
							i++;
						}
					}
					NextBuf[j++]=(char)k;
					i--;
				}
				else {
					NextBuf[j++]=NextBuf[i];
				}
			}
			else {
				NextBuf[j++]=NextBuf[i];
			}
		}
		NextBuf[j]=0;
		NextType=ET_QUOTED;
		return;
	}

	// Parse numeric or fall back to a delimiter.
	if (
		(NextChar>='0' && NextChar<='9') ||
		NextChar=='-' || NextChar=='+' || NextChar=='.'
	) {
		n=0;
		i=0;
		k=0;
		if (NextChar=='+' || NextChar=='-') {
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			TryNextChar();
		}
		while (NextChar>='0' && NextChar<='9') {
			i=1;
			if (n>100) {
				Line=NextLine;
				ThrowElemError("Numeric constant too long.");
			}
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			TryNextChar();
		}
		if (NextChar=='.') {
			k=1;
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			TryNextChar();
			while (NextChar>='0' && NextChar<='9') {
				i=1;
				if (n>100) {
					Line=NextLine;
					ThrowElemError("Numeric constant too long.");
				}
				SetMinNextBufSize(n+1);
				NextBuf[n++]=(char)NextChar;
				TryNextChar();
			}
		}
		if (NextChar=='E' || NextChar=='e') {
			k=1;
			SetMinNextBufSize(n+1);
			NextBuf[n++]=(char)NextChar;
			TryNextChar();
			if (NextChar=='+' || NextChar=='-') {
				SetMinNextBufSize(n+1);
				NextBuf[n++]=(char)NextChar;
				TryNextChar();
			}
			if (NextChar<'0' || NextChar>'9') {
				Line=NextLine;
				ThrowSyntaxError();
			}
			while (NextChar>='0' && NextChar<='9') {
				if (n>100) {
					Line=NextLine;
					ThrowElemError("Numeric constant too long.");
				}
				SetMinNextBufSize(n+1);
				NextBuf[n++]=(char)NextChar;
				TryNextChar();
			}
		}
		SetMinNextBufSize(n+1);
		NextBuf[n]=0;
		if (n==1) {
			if (NextBuf[0]=='.' || NextBuf[0]=='-' || NextBuf[0]=='+') {
				NextDelimiter=NextBuf[0];
				NextType=ET_DELIMITER;
				return;
			}
		}
		if (i==0) {
			Line=NextLine;
			ThrowSyntaxError();
		}
		if (NextBuf[0]=='+') i=1; else i=0;
		if (k) {
			k=sscanf(NextBuf+i,"%lf",&NextDouble);
			NextType=ET_DOUBLE;
		}
		else {
			k=sscanf(NextBuf+i,"%d",&NextInt);
			NextType=ET_INT;
		}
		if (k!=1) {
			Line=NextLine;
			ThrowSyntaxError();
		}
		return;
	}

	// Everything else is a delimiter...
	NextDelimiter=(char)NextChar;
	NextType=ET_DELIMITER;
	TryNextChar();
}


//==============================================================================
//================================ emRecWriter =================================
//==============================================================================

emRecWriter::emRecWriter()
{
	Root=NULL;
	RootQuitPending=false;
	ClosePending=false;
	Indent=0;
}


emRecWriter::~emRecWriter()
{
	// Never do a Root->QuitWriting() here, because of destruction of
	// emRecFileModel...
	Root=NULL;
}


void emRecWriter::TryStartWriting(emRec & root) throw(emString)
{
	const char * formatName;

	try {
		ClosePending=true;
		Root=&root;
		Indent=0;
		formatName=Root->GetFormatName();
		if (formatName) {
			TryWriteString("#%rec:");
			TryWriteString(formatName);
			TryWriteString("%#\n\n");
		}
		RootQuitPending=true;
		Root->TryStartWriting(*this);
	}
	catch (emString errorMessage) {
		QuitWriting();
		throw errorMessage;
	}
}


bool emRecWriter::TryContinueWriting() throw(emString)
{
	try {
		if (Root) {
			if (!Root->TryContinueWriting(*this)) return false;
			RootQuitPending=false;
			Root->QuitWriting();
			TryWriteNewLine();
			ClosePending=false;
			TryClose();
			QuitWriting();
		}
		return true;
	}
	catch (emString errorMessage) {
		QuitWriting();
		throw errorMessage;
	}
}


void emRecWriter::TryFinishWriting() throw(emString)
{
	for (;;) {
		if (TryContinueWriting()) break;
	}
}


void emRecWriter::QuitWriting()
{
	if (Root && RootQuitPending) Root->QuitWriting();
	if (ClosePending) {
		try {
			TryClose();
		}
		catch (emString) {
		}
	}
	Root=NULL;
	RootQuitPending=false;
	ClosePending=false;
	Indent=0;
}


void emRecWriter::TryWriteDelimiter(char c) throw(emString)
{
	TryWriteChar(c);
}


void emRecWriter::TryWriteIdentifier(const char * idf) throw(emString)
{
	TryWriteString(idf);
}


void emRecWriter::TryWriteInt(int i) throw(emString)
{
	char tmp[256];

	sprintf(tmp,"%d",i);
	TryWriteString(tmp);
}


void emRecWriter::TryWriteDouble(double d) throw(emString)
{
	char tmp[256];

	sprintf(tmp,"%.9G",d);
	if (!strchr(tmp,'.') && !strchr(tmp,'E') && !strchr(tmp,'e')) {
		strcat(tmp,".0");
	}
	TryWriteString(tmp);
}


void emRecWriter::TryWriteQuoted(const char * q) throw(emString)
{
	char c;

	TryWriteChar('"');
	for (;;) {
		c=*q++;
		if (!c) break;
		if (c>=0x20 && c<=0x7e) {
			if (c=='"' || c=='\\') TryWriteChar('\\');
			TryWriteChar(c);
		}
		else if (((unsigned char)c)>=0xa0) {
			TryWriteChar(c);
		}
		else if (c=='\a') TryWriteString("\\a");
		else if (c=='\b') TryWriteString("\\b");
		else if (c=='\f') TryWriteString("\\f");
		else if (c=='\n') TryWriteString("\\n");
		else if (c=='\r') TryWriteString("\\r");
		else if (c=='\t') TryWriteString("\\t");
		else if (c=='\v') TryWriteString("\\v");
		else {
			TryWriteChar('\\');
			TryWriteChar((char)(((c>>6)&7)+'0'));
			TryWriteChar((char)(((c>>3)&7)+'0'));
			TryWriteChar((char)(( c    &7)+'0'));
		}
	}
	TryWriteChar('"');
}


void emRecWriter::TryWriteSpace() throw(emString)
{
	TryWriteChar(' ');
}


void emRecWriter::TryWriteNewLine() throw(emString)
{
	TryWriteChar('\n');
}


void emRecWriter::TryWriteIndent() throw(emString)
{
	int i;

	for (i=0; i<Indent; i++) TryWriteChar('\t');
}


void emRecWriter::TryWriteChar(char c)
{
	TryWrite(&c,1);
}


void emRecWriter::TryWriteString(const char * s)
{
	TryWrite(s,strlen(s));
}


//==============================================================================
//============================== emRecFileReader ===============================
//==============================================================================

emRecFileReader::emRecFileReader()
{
	File=NULL;
	FileSize=0;
	FilePos=0;
}


emRecFileReader::~emRecFileReader()
{
	if (File) fclose(File);
}


void emRecFileReader::TryStartReading(
	emRec & root, const emString & filePath
) throw(emString)
{
	emInt64 l;

	if (File) {
		fclose(File);
		File=NULL;
	}

	FilePath=filePath;

	File=fopen(FilePath.Get(),"rb");
	if (!File) goto Err;
	l=fseek(File,0,SEEK_END);
	if (l) goto Err;
	l=ftell(File);
	if (l<0) goto Err;
	FileSize=l;
	l=fseek(File,0,SEEK_SET);
	if (l) goto Err;

	FilePos=0;

	emRecReader::TryStartReading(root);

	return;

Err:
	if (File) {
		fclose(File);
		File=NULL;
	}
	throw emString::Format(
		"Failed to read \"%s\": %s",
		FilePath.Get(),
		emGetErrorText(errno).Get()
	);
}


double emRecFileReader::GetProgress() const
{
	if (!File || !FileSize) return 0.0;
	if (FilePos>=FileSize) return 100.0;
	return ((double)FilePos)*100.0/FileSize;
}


int emRecFileReader::TryRead(char * buf, int maxLen) throw(emString)
{
	size_t sz;
	int len;

	if (!File) return 0;
	len=0;
	do {
		sz=fread(buf+len,1,maxLen-len,File);
		if (ferror(File)) {
			throw emString::Format(
				"Failed to read \"%s\": %s",
				FilePath.Get(),
				emGetErrorText(errno).Get()
			);
		}
		len+=sz;
	} while (len<maxLen && !feof(File));
	FilePos+=len;
	return len;
}


void emRecFileReader::TryClose() throw(emString)
{
	int i;

	if (!File) return;
	i=fclose(File);
	File=NULL;
	if (i!=0) {
		throw emString::Format(
			"Failed to read \"%s\": %s",
			FilePath.Get(),
			emGetErrorText(errno).Get()
		);
	}
}


const char * emRecFileReader::GetSourceName() const
{
	return FilePath.Get();
}


//==============================================================================
//============================== emRecFileWriter ===============================
//==============================================================================

emRecFileWriter::emRecFileWriter()
{
	File=NULL;
}


emRecFileWriter::~emRecFileWriter()
{
	if (File) fclose(File);
}


void emRecFileWriter::TryStartWriting(
	emRec & root, const emString & filePath
) throw(emString)
{
	if (File) {
		fclose(File);
		File=NULL;
	}
	FilePath=filePath;
	File=fopen(FilePath.Get(),"wb");
	if (!File) {
		throw emString::Format(
			"Failed to open \"%s\" for writing: %s",
			FilePath.Get(),
			emGetErrorText(errno).Get()
		);
	}
	emRecWriter::TryStartWriting(root);
}


void emRecFileWriter::TryWrite(const char * buf, int len) throw(emString)
{
	size_t sz;

	if (!File) return;
	do {
		sz=fwrite(buf,1,len,File);
		if (ferror(File)) {
			throw emString::Format(
				"Failed to write \"%s\": %s",
				FilePath.Get(),
				emGetErrorText(errno).Get()
			);
		}
		len-=sz;
		buf+=sz;
	} while (len>0);
}


void emRecFileWriter::TryClose() throw(emString)
{
	int i;

	if (!File) return;
	i=fclose(File);
	File=NULL;
	if (i!=0) {
		throw emString::Format(
			"Failed to write \"%s\": %s",
			FilePath.Get(),
			emGetErrorText(errno).Get()
		);
	}
}


//==============================================================================
//=============================== emRecMemReader ===============================
//==============================================================================

emRecMemReader::emRecMemReader()
{
	MemPos=NULL;
	MemEnd=NULL;
}


void emRecMemReader::TryStartReading(
	emRec & root, const char * buf, int len
) throw(emString)
{
	MemPos=buf;
	MemEnd=buf+len;
	emRecReader::TryStartReading(root);
}


int emRecMemReader::TryRead(char * buf, int maxLen) throw(emString)
{
	int len;

	len=MemEnd-MemPos;
	if (len>maxLen) len=maxLen;
	if (len>0) {
		memcpy(buf,MemPos,len);
		MemPos+=len;
	}
	return len;
}


void emRecMemReader::TryClose() throw(emString)
{
	MemPos=NULL;
	MemEnd=NULL;
}


const char * emRecMemReader::GetSourceName() const
{
	return "rec memory buffer";
}


//==============================================================================
//=============================== emRecMemWriter ===============================
//==============================================================================

emRecMemWriter::emRecMemWriter()
{
	Buf=NULL;
}


emRecMemWriter::~emRecMemWriter()
{
}


void emRecMemWriter::TryStartWriting(
	emRec & root, emArray<char> & buf
) throw(emString)
{
	Buf=&buf;
	emRecWriter::TryStartWriting(root);
}


void emRecMemWriter::TryWrite(const char * buf, int len) throw(emString)
{
	if (!Buf) return;
	Buf->Add(buf,len);
}


void emRecMemWriter::TryClose() throw(emString)
{
	Buf=NULL;
}
