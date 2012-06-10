//------------------------------------------------------------------------------
// emRec.h - Recordable data structures
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

#ifndef emRec_h
#define emRec_h

#ifndef emColor_h
#include <emCore/emColor.h>
#endif

class emRecReader;
class emRecWriter;
class emStructRec;

class emRecNode : public emUncopyable {
public:
	virtual ~emRecNode();
private:
	friend class emRec;
	friend class emRecListener;
	virtual bool IsListener() const = 0;
	virtual void ChildChanged() = 0;
	emRecNode * UpperNode;
};


//==============================================================================
//=================================== emRec ====================================
//==============================================================================

class emRec : public emRecNode {

public:

	// This is the abstract base class for all record classes. The record
	// classes can be used to design data structures whose contents can be
	// converted to and from ASCII text. A typical example is the definition
	// of a configuration file format. It takes just a few lines of
	// programming to define a new file format and a structured C++
	// interface with fastest get-methods and operators on the data fields.
	// A disadvantage is that the records may take round about ten times
	// more memory than a raw C data structure would take. Here comes a
	// tiny example:
	//
	// Assumed our data structure would look like this in simple C:
	//
	//   struct Person {
	//     char Name[64];
	//     int Age;
	//     unsigned Male : 1;
	//   };
	//
	//   struct Person Persons[2];
	//
	// With the record classes, it looks like this:
	//
	//   class Person : public emStructRec {
	//   public:
	//     Person();
	//     emStringRec Name;
	//     emIntRec Age;
	//     emBoolRec Male;
	//   };
	//
	//   Person::Person() :
	//     emStructRec(),
	//     Name(this,"name"),
	//     Age(this,"age"),
	//     Male(this,"male")
	//   {}
	//
	//   emTArrayRec<Person> Persons;
	//
	// This fills the data structure with some information and writes it to
	// a file:
	//
	//   int main(int argc, char * argv[])
	//   {
	//     Persons.SetCount(2);
	//     Persons[0].Name="Fred";
	//     Persons[0].Age=12;
	//     Persons[0].Male=true;
	//     Persons[1].Name="Clara";
	//     Persons[1].Age=11;
	//     Persons[1].Male=false;
	//     Persons.TrySave("test.rec");
	//   }
	//
	// Here is the resulting contents of the file test.rec:
	//
	//   {
	//     name = "Fred"
	//     age = 12
	//     male = yes
	//   }
	//   {
	//     name = "Clara"
	//     age = 11
	//     male = no
	//   }


	emRec();
		// Construct this record without being a member of an
		// emStructRec.

	emRec(emStructRec * parent, const char * varIdentifier);
		// Construct this record as a member of an emStructRec.
		// Arguments:
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.

	virtual ~emRec();
		// Destructor.

	emRec * GetParent();
	const emRec * GetParent() const;
		// Get the parent record. Returns NULL if this is a root record.
		// (This is not super-fast, because the chain of listeners is
		// walked to find the parent).

	emRec * GetRoot();
	const emRec * GetRoot() const;
		// Get the root record (this is not super-fast).

	virtual const char * GetFormatName() const;
		// If this record is the root of records defining a file format,
		// this method should be overloaded and it should return a handy
		// name for the file format. Thereby, emRecWriter and
		// emRecReader are writing and checking a file format magic at
		// the beginning of the file. The default implementation returns
		// NULL which means to have no file format magic.

	void TryLoad(const emString & filePath) throw(emString);
	void TrySave(const emString & filePath) throw(emString);
		// Load or save this tree of records from or to a file.
		// Arguments:
		//   filePath - Path/name of the file.

	void TryLoadFromMem(const char * buf, int len) throw(emString);
	void TryLoadFromMem(const emArray<char> & buf) throw(emString);
	void SaveToMem(emArray<char> & buf);
		// Load or save this tree of records from or to memory.
		// SaveToMem adds the data to the array without emptying it
		// first.

	void TryCopy(emRec & source) throw(emString);
	void Copy(emRec & source);
		// Copy the given tree of records to this tree of records via
		// the SaveToMem and TryLoadFromMem methods. This is slow and
		// works only if the records are compatible. The second version
		// calls emFatalError on error.

	virtual void SetToDefault() = 0;
		// Set this record to default state. When reading an
		// emStructRec, omitted members are set to their defaults.

	virtual bool IsSetToDefault() const = 0;
		// Ask whether this record is set to its default state. When
		// writing an emStructRec, members with default state may be
		// omitted.

	virtual void TryStartReading(emRecReader & reader) throw(emString) = 0;
	virtual bool TryContinueReading(emRecReader & reader) throw(emString) = 0;
	virtual void QuitReading() = 0;
	virtual void TryStartWriting(emRecWriter & writer) throw(emString) = 0;
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString) = 0;
	virtual void QuitWriting() = 0;
		// This is only to be called by emRecReader/Writer and through
		// recursion in the tree. Try to read or write this tree of
		// records via the given reader or writer. First, the start
		// method is called, and then the continue method is called
		// again and again until it returns true. The quit method is
		// called at the end, even on error or when aborting. The quit
		// mechanism should be quite stable, because with
		// emRecFileModel, it could happen that quit is called in
		// response to OnChanged through a user modification.

	virtual emUInt64 CalcRecMemNeed() const = 0;
		// Calculate best known number of bytes, which are allocated for
		// this record, or which will be allocated after reading has
		// completed.

	static void CheckIdentifier(const char * identifier);
		// "identifiers" are names of variables, enumeration variants
		// and so on. The syntactical rules are the same as with
		// identifiers in the C programming language: begin with a
		// letter or underscore and continue with letter, underscore or
		// digit. But they are not case sensitive. This functions checks
		// whether the given string is a valid identifier. If not,
		// emFatalError is called.

protected:

	void Changed();
		// This must be called by derived classes after each change of
		// the record, even through TryStartReading and
		// TryContinueReading, but never through constructors or
		// destructors. It informs the listeners of all the records on
		// the path from this record up to the root record.

	void BeTheParentOf(emRec * child);
		// This makes this record the parent of the given child. It is
		// called by dynamic containers like emUnionRec and emArrayRec
		// on creation of children.

private:
	virtual bool IsListener() const;
	virtual void ChildChanged();
};

inline emRec::emRec()
{
	UpperNode=NULL;
}

inline void emRec::Changed()
{
	if (UpperNode) UpperNode->ChildChanged();
}


//==============================================================================
//=============================== emRecListener ================================
//==============================================================================

class emRecListener : public emRecNode {

public:

	// Abstract base class for a listener on a record and it descendants.

	emRecListener(emRec * rec=NULL);
	virtual ~emRecListener();

	const emRec * GetListenedRec() const;
	emRec * GetListenedRec();
	void SetListenedRec(emRec * rec);
		// Get/set the record to be listened. NULL means not to listen
		// any record. On deletion of the listened record, this is
		// automatically set to NULL.

protected:

	virtual void OnRecChanged() = 0;
		// Called on each change of the record and its descendants. This
		// is a synchronous call - the implementation must not modify
		// the records or the listeners.

private:
	virtual bool IsListener() const;
	virtual void ChildChanged();
	emRec * Rec;
};

inline const emRec * emRecListener::GetListenedRec() const
{
	return Rec;
}

inline emRec * emRecListener::GetListenedRec()
{
	return Rec;
}


//==============================================================================
//=============================== emRecAllocator ===============================
//==============================================================================

typedef emRec * (* emRecAllocator)();
	// Data type for a pointer to function which can allocate a new record.
	// This is used for dynamic containers like emUnionRec and emArrayRec.

#define EM_DEFAULT_REC_ALLOCATOR(REC) (&emDfltRecAllocImp<REC >::Allocate)
	// This macro expands to an emRecAllocator which allocates a record of
	// type REC with its default constructor.

// private stuff for the macro above.
template <class REC> class emDfltRecAllocImp : public emUnconstructable {
public:
	static emRec * Allocate() { return new REC(); }
};


//==============================================================================
//================================= emBoolRec ==================================
//==============================================================================

class emBoolRec : public emRec {

public:

	// Record class for a boolean value.

	emBoolRec(bool defaultValue=false);
	emBoolRec(emStructRec * parent, const char * varIdentifier,
	          bool defaultValue=false);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this boolean record.

	bool Get() const;
	operator bool () const;
		// Get the boolean value.

	void Set(bool value);
	emBoolRec & operator = (bool value);
		// Set the boolean value.

	void Invert();
		// Invert the boolean value.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:
	bool DefaultValue, Value;
};

inline bool emBoolRec::Get() const
{
	return Value;
}

inline emBoolRec::operator bool () const
{
	return Value;
}

inline emBoolRec & emBoolRec::operator = (bool value)
{
	Set(value);
	return *this;
}


//==============================================================================
//================================== emIntRec ==================================
//==============================================================================

class emIntRec : public emRec {

public:

	// Record class for an integer value.

	emIntRec(int defaultValue=0, int minValue=INT_MIN,
	         int maxValue=INT_MAX);
	emIntRec(emStructRec * parent, const char * varIdentifier,
	         int defaultValue=0, int minValue=INT_MIN,
	         int maxValue=INT_MAX);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this integer record.
		//   minValue      - The value will never be less than this.
		//   maxValue      - The value will never be greater than this.

	int Get() const;
	operator int () const;
		// Get the integer value.

	void Set(int value);
	emIntRec & operator = (int value);
		// Set the integer value. It is clipped if it would be less than
		// the minimum value or greater than the maximum value.

	int GetMinValue() const;
	int GetMaxValue() const;
		// Get minimum and maximum values.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:
	int DefaultValue, MinValue, MaxValue, Value;
};

inline int emIntRec::Get() const
{
	return Value;
}

inline emIntRec::operator int () const
{
	return Value;
}

inline emIntRec & emIntRec::operator = (int value)
{
	Set(value);
	return *this;
}

inline int emIntRec::GetMinValue() const
{
	return MinValue;
}

inline int emIntRec::GetMaxValue() const
{
	return MaxValue;
}


//==============================================================================
//================================ emDoubleRec =================================
//==============================================================================

class emDoubleRec : public emRec {

public:

	// Record class for a double value.

	emDoubleRec(double defaultValue=0.0, double minValue=-3.4E+38,
	            double maxValue=3.4E+38);
	emDoubleRec(emStructRec * parent, const char * varIdentifier,
	            double defaultValue=0.0, double minValue=-3.4E+38,
	            double maxValue=3.4E+38);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this double record.
		//   minValue      - The value will never be less than this.
		//   maxValue      - The value will never be greater than this.

	virtual ~emDoubleRec();
		// Destructor.

	double Get() const;
	operator double () const;
		// Get the value.

	void Set(double value);
	emDoubleRec & operator = (double value);
		// Set the double value. It is clipped if it would be less than
		// the minimum or greater than the maximum.

	double GetMinValue() const;
	double GetMaxValue() const;
		// Get minimum and maximum values.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:
	double DefaultValue, MinValue, MaxValue, Value;
};

inline double emDoubleRec::Get() const
{
	return Value;
}

inline emDoubleRec::operator double () const
{
	return Value;
}

inline emDoubleRec & emDoubleRec::operator = (double value)
{
	Set(value);
	return *this;
}

inline double emDoubleRec::GetMinValue() const
{
	return MinValue;
}

inline double emDoubleRec::GetMaxValue() const
{
	return MaxValue;
}


//==============================================================================
//================================= emEnumRec ==================================
//==============================================================================

class emEnumRec : public emRec {

public:

	// Record class for an enumeration value. It is like emIntRec, but each
	// possible value is given an identifier.

	emEnumRec(int defaultValue, const char * identifier0, ...);
	emEnumRec(emStructRec * parent, const char * varIdentifier,
	          int defaultValue, const char * identifier0, ...);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this enumeration
		//                   record.
		//   identifier0   - The identifier for value=0. The identifier
		//                   is not copied - the pointer must be valid
		//                   for the life time of this record.
		//   ...           - Any number of further identifiers for
		//                   values 1, 2, 3 and so on, terminated by a
		//                   NULL.

	virtual ~emEnumRec();
		// Destructor.

	int Get() const;
	operator int () const;
		// Get the value.

	void Set(int value);
	emEnumRec & operator = (int value);
		// Set the value. It is clipped if out of range.

	const char * GetIdentifier() const;
		// Get the identifier for the current value.

	int GetIdentifierCount();
		// Get number of possible values.

	const char * GetIdentifierOf(int value);
		// Get the identifier for the given value. Returns NULL if the
		// value is out of range.

	int GetValueOf(const char * identifier);
		// Get the value for the given identifier. Returns -1 if there
		// is no such identifier.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:

	void Init(int defaultValue, const char * identifier0, va_list args);

	const char * * Identifiers;
	int IdentifierCount, DefaultValue, Value;
};

inline int emEnumRec::Get() const
{
	return Value;
}

inline emEnumRec::operator int () const
{
	return Value;
}

inline emEnumRec & emEnumRec::operator = (int value)
{
	Set(value);
	return *this;
}

inline const char * emEnumRec::GetIdentifier() const
{
	return Identifiers[Value];
}

inline int emEnumRec::GetIdentifierCount()
{
	return IdentifierCount;
}


//==============================================================================
//================================= emFlagsRec =================================
//==============================================================================

class emFlagsRec : public emRec {

public:

	// Record class for a set of flags. It is a bit mask where each possible
	// bit is given an identifier.

	emFlagsRec(int defaultValue, const char * identifier0, ...);
	emFlagsRec(emStructRec * parent, const char * varIdentifier,
	          int defaultValue, const char * identifier0, ...);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this flags
		//                   record.
		//   identifier0   - The identifier for bit 0. The identifier
		//                   is not copied - the pointer must be valid
		//                   for the life time of this record.
		//   ...           - Any number of further identifiers for
		//                   bits 1, 2, 3, ... 31, terminated by a
		//                   NULL.

	virtual ~emFlagsRec();
		// Destructor.

	int Get() const;
	operator int () const;
		// Get the value. It's the bit mask.

	void Set(int value);
	emFlagsRec & operator = (int value);
		// Set the value. Undefined flag bits are set to zero.

	int GetIdentifierCount();
		// Get number of possible flag bits.

	const char * GetIdentifierOf(int bit);
		// Get the identifier for the given flag bit (0...31). Returns
		// NULL when no identifier has been defined for the bit.

	int GetBitOf(const char * identifier);
		// Get the flag bit for the given identifier. Returns -1 if
		// there is no such identifier.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:

	void Init(int defaultValue, const char * identifier0, va_list args);

	const char * * Identifiers;
	int IdentifierCount, DefaultValue, Value;
};

inline int emFlagsRec::Get() const
{
	return Value;
}

inline emFlagsRec::operator int () const
{
	return Value;
}

inline emFlagsRec & emFlagsRec::operator = (int value)
{
	Set(value);
	return *this;
}

inline int emFlagsRec::GetIdentifierCount()
{
	return IdentifierCount;
}


//==============================================================================
//=============================== emAlignmentRec ===============================
//==============================================================================

class emAlignmentRec : public emRec {

public:

	// Record class for an emAlignment value.

	emAlignmentRec(emAlignment defaultValue=EM_ALIGN_CENTER);
	emAlignmentRec(emStructRec * parent, const char * varIdentifier,
	               emAlignment defaultValue=EM_ALIGN_CENTER);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this record.

	virtual ~emAlignmentRec();
		// Destructor.

	emAlignment Get() const;
	operator emAlignment () const;
		// Get the alignment value.

	void Set(emAlignment value);
	emAlignmentRec & operator = (emAlignment value);
		// Set the alignment value.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:

	emAlignment DefaultValue, Value;
};

inline emAlignment emAlignmentRec::Get() const
{
	return Value;
}

inline emAlignmentRec::operator emAlignment () const
{
	return Value;
}

inline emAlignmentRec & emAlignmentRec::operator = (emAlignment value)
{
	Set(value);
	return *this;
}


//==============================================================================
//================================ emStringRec =================================
//==============================================================================

class emStringRec : public emRec {

public:

	// Record class for an emString value.

	emStringRec(const emString & defaultValue=emString());
	emStringRec(emStructRec * parent, const char * varIdentifier,
	            const emString & defaultValue=emString());
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this string record.

	virtual ~emStringRec();
		// Destructor.

	const emString & Get() const;
	operator const emString & () const;
		// Get the string value.

	void Set(const emString & value);
	emStringRec & operator = (const emString & value);
		// Set the string value.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:
	emString DefaultValue, Value;
};

inline const emString & emStringRec::Get() const
{
	return Value;
}

inline emStringRec::operator const emString & () const
{
	return Value;
}

inline emStringRec & emStringRec::operator = (const emString & value)
{
	Set(value);
	return *this;
}


//==============================================================================
//================================= emColorRec =================================
//==============================================================================

class emColorRec : public emRec {

public:

	// Record class for an emColor value.

	emColorRec(emColor defaultValue=emColor(0,0,0,255),
	           bool haveAlpha=false);
	emColorRec(emStructRec * parent, const char * varIdentifier,
	           emColor defaultValue=emColor(0,0,0,255),
	           bool haveAlpha=false);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   defaultValue  - The default value for this color record.
		//   haveAlpha     - false if the color has to be opaque.

	emColor Get() const;
	operator emColor () const;
		// Get the color value.

	void Set(emColor value);
	emColorRec & operator = (emColor value);
		// Set the color value. The alpha channel is set to 255 if this
		// color record has to be opaque.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:
	emColor DefaultValue, Value;
	bool HaveAlpha;
};

inline emColor emColorRec::Get() const
{
	return Value;
}

inline emColorRec::operator emColor () const
{
	return Value;
}

inline emColorRec & emColorRec::operator = (emColor value)
{
	Set(value);
	return *this;
}


//==============================================================================
//================================ emStructRec =================================
//==============================================================================

class emStructRec : public emRec {

public:

	// Base class for a structured record class. The idea is to give derived
	// classes some records as member variables.

	emStructRec();
	emStructRec(emStructRec * parent, const char * varIdentifier);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.

	virtual ~emStructRec();
		// Destructor.

	int GetCount() const;
		// Get number of members.

	const emRec & Get(int index) const;
	emRec & Get(int index);
	const emRec & operator [] (int index) const;
	emRec & operator [] (int index);
		// Get a reference to a member. The index must be within the
		// range of 0 to GetCount()-1.

	const char * GetIdentifierOf(int index);
		// Get the identifier for the given member index. Returns NULL
		// if the index is out of range.

	int GetIndexOf(emRec * member);
		// Get the member index for the given member pointer. Returns -1
		// if there is no such member.

	int GetIndexOf(const char * identifier);
		// Get the member index for the given identifier. Returns -1 if
		// there is no such identifier.

	virtual bool ShallWriteOptionalOnly(const emRec * child) const;
		// Whether the given member should not be written when it has
		// default state. The default implementation always returns
		// false.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private: friend class emRec;

	void AddMember(emRec * member, const char * identifier);

	struct MemberType {
		const char * Identifier;
		emRec * Record;
	};

	struct RWStateType {
		int Pos;
		bool ChildReady;
		bool Empty;
		emByte Map[1];
	};

	int Count, Capacity;
	MemberType * Members;
	RWStateType * RWState;
};

inline int emStructRec::GetCount() const
{
	return Count;
}

inline const emRec & emStructRec::Get(int index) const
{
	return *Members[index].Record;
}

inline emRec & emStructRec::Get(int index)
{
	return *Members[index].Record;
}

inline const emRec & emStructRec::operator [] (int index) const
{
	return *Members[index].Record;
}

inline emRec & emStructRec::operator [] (int index)
{
	return *Members[index].Record;
}


//==============================================================================
//================================= emUnionRec =================================
//==============================================================================

class emUnionRec : public emRec {

public:

	// Record class for a union. An instance of this class manages one child
	// record with variable type. The possible types are called the
	// "variants".

	emUnionRec(int defaultVariant,
	           const char * identifier0, emRecAllocator allocator0, ...);
	emUnionRec(emStructRec * parent, const char * varIdentifier,
	           int defaultVariant,
	           const char * identifier0, emRecAllocator allocator0, ...);
		// Construct this record.
		//   parent         - The emStructRec this record is a member
		//                    of.
		//   varIdentifier  - Identifier for this record within the
		//                    emStructRec. The string is not copied -
		//                    the pointer must be valid for the life
		//                    time of this record.
		//   defaultVariant - Index of the default variant.
		//   identifier0    - Identifier for the first variant.
		//                    The string is not copied - the pointer
		//                    must be valid for the life time of the
		//                    record.
		//   allocator0     - Allocator function for the first variant.
		//   ...            - Any number of further pairs of identifier
		//                    and allocator, terminated by a NULL.

	virtual ~emUnionRec();
		// Destructor.

	int GetVariant() const;
		// Get the current variant index.

	void SetVariant(int variant);
		// Set the current variant index. If it is a change, the old
		// child record is deleted and a new one is created by calling
		// the corresponding allocator function.

	emRec & Get();
	const emRec & Get() const;
		// Get a reference to the child record.

	int GetVariantCount() const;
		// Get number of variants.

	const char * GetIdentifierOf(int variant);
		// Get the identifier for the given variant index. Returns NULL
		// if there is no such variant.

	int GetVariantOf(const char * identifier);
		// Get the variant index for the given identifier. Returns -1 if
		// there is no such identifier.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:

	void Init(int defaultVariant, const char * identifier0,
	          emRecAllocator allocator0, va_list args);

	struct VariantType {
		const char * Identifier;
		emRecAllocator Allocator;
	};

	VariantType * TypeArray;
	int VariantCount, DefaultVariant, Variant;
	emRec * Record;
};

inline int emUnionRec::GetVariant() const
{
	return Variant;
}

inline emRec & emUnionRec::Get()
{
	return *Record;
}

inline const emRec & emUnionRec::Get() const
{
	return *Record;
}

inline int emUnionRec::GetVariantCount() const
{
	return VariantCount;
}


//==============================================================================
//================================= emArrayRec =================================
//==============================================================================

class emArrayRec : public emRec {

public:

	// Record class for an array of records. Please even see the template
	// version emTArrayRec more below.

	emArrayRec(emRecAllocator allocator, int minCount=0,
	           int maxCount=INT_MAX);
	emArrayRec(emStructRec * parent, const char * varIdentifier,
	           emRecAllocator allocator, int minCount=0,
	           int maxCount=INT_MAX);
		// Construct this record.
		//   parent        - The emStructRec this record is a member of.
		//   varIdentifier - Identifier for this record within the
		//                   emStructRec. The string is not copied - the
		//                   pointer must be valid for the life time of
		//                   this record.
		//   allocator     - Allocator function for elements of the
		//                   array.
		//   minCount      - Minimum number of elements. This is even
		//                   the default number of elements.
		//   maxCount      - Maximum number of elements.

	virtual ~emArrayRec();
		// Destructor.

	int GetCount() const;
		// Get number of elements.

	void SetCount(int count);
		// Set number of elements. It is clipped if out of range. New
		// elements are set to their defaults.

	void Insert(int index, int insCount=1);
	void Remove(int index, int remCount=1);
		// Insert or remove elements at a particular position. The
		// number of elements is clipped if the resulting total number
		// would be out of range. New elements are set to their
		// defaults.

	int GetMinCount() const;
	int GetMaxCount() const;
		// Get minimum and maximum number of elements.

	const emRec & Get(int index) const;
	emRec & Get(int index);
	const emRec & operator [] (int index) const;
	emRec & operator [] (int index);
		// Get a reference to an element. The index must be within the
		// range of 0 to GetCount()-1.

	virtual void SetToDefault();
	virtual bool IsSetToDefault() const;
	virtual void TryStartReading(emRecReader & reader) throw(emString);
	virtual bool TryContinueReading(emRecReader & reader) throw(emString);
	virtual void QuitReading();
	virtual void TryStartWriting(emRecWriter & writer) throw(emString);
	virtual bool TryContinueWriting(emRecWriter & writer) throw(emString);
	virtual void QuitWriting();
	virtual emUInt64 CalcRecMemNeed() const;
		// See emRec.

private:

	void Init(emRecAllocator allocator, int minCount, int maxCount);

	emRecAllocator Allocator;
	int MinCount, MaxCount, Count, Capacity, RWPos;
	emRec * * Array;
	bool RWChildReady;
};

inline int emArrayRec::GetCount() const
{
	return Count;
}

inline int emArrayRec::GetMinCount() const
{
	return MinCount;
}

inline int emArrayRec::GetMaxCount() const
{
	return MaxCount;
}

inline const emRec & emArrayRec::Get(int index) const
{
	return *Array[index];
}

inline emRec & emArrayRec::Get(int index)
{
	return *Array[index];
}

inline const emRec & emArrayRec::operator [] (int index) const
{
	return *Array[index];
}

inline emRec & emArrayRec::operator [] (int index)
{
	return *Array[index];
}


//==============================================================================
//================================ emTArrayRec =================================
//==============================================================================

template <class REC> class emTArrayRec : public emArrayRec {

public:

	// Template version of emArrayRec.

	emTArrayRec(int minCount=0, int maxCount=INT_MAX);
	emTArrayRec(emStructRec * parent, const char * varIdentifier,
	            int minCount=0, int maxCount=INT_MAX);
		// Like with emArrayRec, but the allocator is
		// EM_DEFAULT_REC_ALLOCATOR(REC).

	const REC & Get(int index) const;
	REC & Get(int index);
	const REC & operator [] (int index) const;
	REC & operator [] (int index);
		// Like with emArrayRec, but the results are casted...
};

template <class REC> inline emTArrayRec<REC>::emTArrayRec(
	int minCount, int maxCount
)
	: emArrayRec(EM_DEFAULT_REC_ALLOCATOR(REC),minCount,maxCount)
{
}

template <class REC> inline emTArrayRec<REC>::emTArrayRec(
	emStructRec * parent, const char * varIdentifier, int minCount,
	int maxCount
)
	: emArrayRec(parent,varIdentifier,EM_DEFAULT_REC_ALLOCATOR(REC),
	              minCount,maxCount)
{
}

template <class REC> inline const REC & emTArrayRec<REC>::Get(int index) const
{
	return (const REC &)emArrayRec::Get(index);
}

template <class REC> inline REC & emTArrayRec<REC>::Get(int index)
{
	return (REC &)emArrayRec::Get(index);
}

template <class REC> inline const REC & emTArrayRec<REC>::operator [] (
	int index
) const
{
	return (const REC &)emArrayRec::Get(index);
}

template <class REC> inline REC & emTArrayRec<REC>::operator [] (int index)
{
	return (REC &)emArrayRec::Get(index);
}


//==============================================================================
//================================ emRecReader =================================
//==============================================================================

class emRecReader : public emUncopyable {

public:

	// Abstract base class for reading a tree of records from a source.

	emRecReader();
	virtual ~emRecReader();

	void TryStartReading(emRec & root) throw(emString);
		// Start reading.
		// Arguments:
		//   root - The root of the records to filled.

	bool TryContinueReading() throw(emString);
		// Continue reading. Returns true when ready, otherwise
		// TryContinueReading has to be called again.

	void TryFinishReading() throw(emString);
		// Continue reading until ready.

	void QuitReading();
		// Abort any reading.

	// - - The following things are for implementing emRec derivatives - -

	enum ElementType {
		ET_DELIMITER,
		ET_IDENTIFIER,
		ET_INT,
		ET_DOUBLE,
		ET_QUOTED,
		ET_END
	};

	ElementType TryPeekNext(char * pDelimiter=NULL) throw(emString);
		// Peek for the type of the next syntactical element to be read.
		// If it is ET_DELIMITER and if pDelimiter is not NULL,
		// *pDelimiter is set to the delimiter character,

	char TryReadDelimiter() throw(emString);
		// Read the next syntactical element as a delimiter or throw an
		// error. Currently, a delimiter can be any character except for
		// white space, letters, digits, '#', '_' and '"'. The
		// characters '.', '-' and '+' can be delimiters only if not
		// confusable with numbers.

	void TryReadCertainDelimiter(char delimiter) throw(emString);
		// Read the next syntactical element as the given delimiter or
		// throw an error.

	const char * TryReadIdentifier() throw(emString);
		// Read the next syntactical element as an identifier or throw
		// an error.

	int TryReadInt() throw(emString);
		// Read the next syntactical element as an integer value or
		// throw an error.

	double TryReadDouble() throw(emString);
		// Read the next syntactical element as a double value or throw
		// an error.

	const char * TryReadQuoted() throw(emString);
		// Read and unquote the next syntactical element as a quoted
		// string or throw an error.

	void ThrowElemError(const char * text) const throw(emString);
		// Throw an error containing the file name, the current line
		// number and the given text.

	void ThrowSyntaxError() const throw(emString);
		// Like ThrowElemError("syntax error")

	const emRec * GetRootRec() const;
		// Get the root record of the tree currently read or NULL.

protected:

	virtual int TryRead(char * buf, int maxLen) throw(emString) = 0;
		// Read up to maxLen bytes from the source into the given
		// buffer, or throw an error. Return the number of bytes read,
		// or 0 if the end of source has been reached.

	virtual void TryClose() throw(emString) = 0;
		// Close the source, or throw an error.

	virtual const char * GetSourceName() const = 0;
		// Get the file name or another identification. This is required
		// for making error messages.

private:

	void SetMinNextBufSize(int minSize);
	void TryNextChar() throw(emString);
	void TryParseNext() throw(emString);

	emRec * Root;
	bool RootQuitPending;
	bool ClosePending;
	int Line;
	bool NextEaten;
	int NextLine;
	ElementType NextType;
	char NextDelimiter;
	char * NextBuf;
	int NextBufSize;
	int NextInt;
	double NextDouble;
	int NextChar;
};

inline const emRec * emRecReader::GetRootRec() const
{
	return Root;
}


//==============================================================================
//================================ emRecWriter =================================
//==============================================================================

class emRecWriter : public emUncopyable {

public:

	// Abstract base class for writing a tree of records to a target.

	emRecWriter();
	virtual ~emRecWriter();

	void TryStartWriting(emRec & root) throw(emString);
		// Start writing.
		// Arguments:
		//   root - The root of the records to be written.

	bool TryContinueWriting() throw(emString);
		// Continue writing. Returns true when ready, otherwise
		// TryContinueWriting has to be called again.

	void TryFinishWriting() throw(emString);
		// Continue writing until ready.

	void QuitWriting();
		// Abort any writing.

	// - - The following things are for implementing emRec derivatives - -

	void TryWriteDelimiter(char c) throw(emString);
		// Write the given delimiter character.

	void TryWriteIdentifier(const char * idf) throw(emString);
		// Write the given identifier.

	void TryWriteInt(int i) throw(emString);
		// Write the given integer value.

	void TryWriteDouble(double d) throw(emString);
		// Write the given double value.

	void TryWriteQuoted(const char * q) throw(emString);
		// Quote and write the given string.

	void TryWriteSpace() throw(emString);
		// Write a space character.

	void TryWriteNewLine() throw(emString);
		// Write a new-line character.

	void TryWriteIndent() throw(emString);
		// Write an indent (to be called only at beginning of a new
		// line). It is a number of tabulator characters.

	void IncIndent();
	void DecIndent();
		// Increase or decrease the number of tabulator characters to be
		// written by TryWriteIndent.

	const emRec * GetRootRec() const;
		// Get the root record of the tree currently written or NULL.

protected:

	virtual void TryWrite(const char * buf, int len) throw(emString) = 0;
		// Write len bytes from the given buffer to the target, or throw
		// an error.

	virtual void TryClose() throw(emString) = 0;
		// Flush and close the target, or throw an error.

private:

	void TryWriteChar(char c);
	void TryWriteString(const char * s);

	emRec * Root;
	bool RootQuitPending;
	bool ClosePending;
	int Indent;
};

inline void emRecWriter::IncIndent()
{
	Indent++;
}

inline void emRecWriter::DecIndent()
{
	Indent--;
}

inline const emRec * emRecWriter::GetRootRec() const
{
	return Root;
}


//==============================================================================
//============================== emRecFileReader ===============================
//==============================================================================

class emRecFileReader : public emRecReader {

public:

	// Class for reading a tree of records from a regular file.

	emRecFileReader();
	virtual ~emRecFileReader();

	void TryStartReading(emRec & root,
	                     const emString & filePath) throw(emString);
		// Start reading.
		// Arguments:
		//   root     - The root of the tree of records to be read from
		//              the file.
		//   filePath - Path/name of the file.

	emUInt64 GetFileSize() const;
		// Size of the file.

	emUInt64 GetFilePos() const;
		// Read position in the file.

	double GetProgress() const;
		// Progress in percent.

protected:

	virtual int TryRead(char * buf, int maxLen) throw(emString);
	virtual void TryClose() throw(emString);
	virtual const char * GetSourceName() const;

private:

	emString FilePath;
	FILE * File;
	emUInt64 FileSize, FilePos;
};

inline emUInt64 emRecFileReader::GetFileSize() const
{
	return FileSize;
}

inline emUInt64 emRecFileReader::GetFilePos() const
{
	return FilePos;
}


//==============================================================================
//============================== emRecFileWriter ===============================
//==============================================================================

class emRecFileWriter : public emRecWriter {

public:

	// Class for writing a tree of records to a regular file.

	emRecFileWriter();
	virtual ~emRecFileWriter();

	void TryStartWriting(emRec & root,
	                     const emString & filePath) throw(emString);
		// Start writing.
		// Arguments:
		//   root     - The root of the tree of records to be written to
		//              the file.
		//   filePath - Path/name of the file.

protected:

	virtual void TryWrite(const char * buf, int len) throw(emString);
	virtual void TryClose() throw(emString);

private:

	emString FilePath;
	FILE * File;
};


//==============================================================================
//=============================== emRecMemReader ===============================
//==============================================================================

class emRecMemReader : public emRecReader {

public:

	// Class for reading a tree of records from memory.

	emRecMemReader();

	void TryStartReading(emRec & root, const char * buf,
	                     int len) throw(emString);
		// Start reading.
		// Arguments:
		//   root  - The root of the tree of records to be read from
		//           memory.
		//   buf   - Memory buffer to be read, must be valid until
		//           reading has finished.
		//   len   - Number of bytes in the buffer.

protected:

	virtual int TryRead(char * buf, int maxLen) throw(emString);
	virtual void TryClose() throw(emString);
	virtual const char * GetSourceName() const;

private:

	const char * MemPos, * MemEnd;
};


//==============================================================================
//=============================== emRecMemWriter ===============================
//==============================================================================

class emRecMemWriter : public emRecWriter {

public:

	// Class for writing a tree of records to memory.

	emRecMemWriter();
	virtual ~emRecMemWriter();

	void TryStartWriting(emRec & root,
	                     emArray<char> & buf) throw(emString);
		// Start writing. This and the other Try methods should never
		// fail.
		// Arguments:
		//   root - The root of the tree of records to be written to
		//          memory.
		//   buf  - Memory buffer where the output is to be added. The
		//          reference must be valid until the writing has
		//          finished.

protected:

	virtual void TryWrite(const char * buf, int len) throw(emString);
	virtual void TryClose() throw(emString);

private:

	emArray<char> * Buf;
};


#endif
