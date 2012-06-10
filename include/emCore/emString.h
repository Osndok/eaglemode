//------------------------------------------------------------------------------
// emString.h
//
// Copyright (C) 2004-2011 Oliver Hamann.
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

#ifndef emString_h
#define emString_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//================================== emString ==================================
//==============================================================================

class emString {

public:

	// Class for a dynamically allocated null-terminated character string
	// with copy-on-write behavior. This string type is designed to minimize
	// memory consumption. That means, each change of the length requires
	// reallocation. If you plan to perform many length-changing operations
	// on a string, you may decide to do it with an emArray<char> for a
	// better performance.

	emString();
		// Construct an empty string.

	emString(const emString & s);
		// Construct a copied string.
		// Arguments:
		//   s - The string to be copied.

	emString(const char * p);
		// Construct a copied string.
		// Arguments:
		//   p - The null-terminated string to be copied.

	emString(const char * p, int len);
		// Construct a copied string.
		// Arguments:
		//   p   - The string to be copied.
		//   len - Number of bytes in p.

	emString(const char * p, int len, const char * p2, int len2);
		// Construct a string as a concatenated copy of two strings.
		// Arguments:
		//   p    - First source string.
		//   len  - Number of bytes in p.
		//   p2   - Second source string.
		//   len2 - Number of bytes in p2.

	emString(char c, int len=1);
		// Construct a string by filling it with a byte.
		// Arguments:
		//   c   - The byte to be used for filling.
		//   len - The length of the resulting string.

	~emString();
		// Destructor.

	static emString Format(const char * format, ...) EM_FUNC_ATTR_PRINTF(1);
		// This function creates a formatted string.
		// Arguments:
		//   format - The format (like with printf).
		//   ...    - Arguments to the format (like with printf).
		// Returns: The formatted string.

	emString & operator = (const emString & s);
	emString & operator = (const char * p);
	emString & operator = (char c);
		// Copy a string or a byte to this string.

	int GetCount() const;
	int GetLen() const;
		// Get the number of bytes in this string, excluding the
		// terminating null. This really counts the bytes using strlen.

	operator const char * () const;
	const char * Get() const;
		// Get a pointer to the internal null-terminated string for
		// reading. At least because of the copy-on-write feature, the
		// pointer is valid only until calling any non-const method or
		// operator on this string, or giving this string as a non-const
		// argument to any call in the world. Hint: Even methods like
		// Add, Insert, Replace and GetSubString may make shallow
		// copies, like the copy operator and copy constructor do.

	char operator [] (int index) const;
	char Get(int index) const;
		// Get one byte from this string.
		// Arguments:
		//   index - The index of the desired byte. This must be within
		//           the range of 0 to GetLen().
		// Returns: The byte.

	char * GetWritable();
	char & GetWritable(int index);
		// Like Get() and Get(index), but for modifying the bytes.
		// There is no non-const version of the operator [], because
		// compilers would make use of it too often. The rules for the
		// validity of the pointer or reference are the same as with
		// Get(), but modification is allowed only until doing something
		// which could make a shallow copy of this string.

	char * SetLenGetWritable(int len);
		// Like GetWritable(), but even prepare for a new length of the
		// string.
		// Arguments:
		//   len - The length of the string you plan to produce.
		// Returns:
		//   A pointer to the internal null-terminated string buffer for
		//   writing. The buffer contains up to len bytes of the
		//   original string, then there is a null, then there may be
		//   garbage (if len>GetLen()+1), and in any case there is a
		//   null at the end (at index len). You may modify all
		//   bytes within the index range of 0 to len-1.

	void Add(const emString & s);
	void Add(const char * p);
	void Add(const char * p, int len);
	void Add(char c, int len=1);
	emString & operator += (const emString & s);
	emString & operator += (const char * p);
	emString & operator += (char c);
		// Add a copy of something to the end of this string. Source and
		// target memory may overlap.
		// Arguments:
		//   s   - A source string.
		//   p   - A source string, null-terminated if len not given.
		//   c   - A source byte.
		//   len - Length of string p, or how often to add byte c.

	emString operator + (const emString & s) const;
	emString operator + (const char * p) const;
	emString operator + (char c) const;
	friend emString operator + (const char * p, const emString & s);
	friend emString operator + (char c, const emString & s);
		// Add strings...

	void Insert(int index, const emString & s);
	void Insert(int index, const char * p);
	void Insert(int index, const char * p, int len);
	void Insert(int index, char c, int len=1);
		// Insert a copy of something somewhere in this string. Source
		// and target memory may overlap.
		// Arguments:
		//   index - Position of insertion. This should be within the
		//           range of 0 to GetLen(), otherwise it is clipped
		//           accordingly.
		//   s     - A source string to be copied for insertion.
		//   p     - A source string to be copied for insertion,
		//           null-terminated if len not given.
		//   c     - A source byte to be copied for insertion.
		//   len   - Length of string p, or how often to insert
		//           byte c.

	void Replace(int index, int exLen, const emString & s);
	void Replace(int index, int exLen, const char * p);
	void Replace(int index, int exLen, const char * p, int len);
	void Replace(int index, int exLen, char c, int len=1);
		// Replace something in this string by a copy of something.
		// Source and target memory may overlap.
		// Arguments:
		//   index - Position of replacement. This should be within the
		//           range of 0 to GetLen()-exLen, otherwise index and
		//           exLen are clipped accordingly.
		//   exLen - Number of bytes to be removed.
		//   s     - A source string to be copied for insertion.
		//   p     - A source string to be copied for insertion,
		//           null-terminated if len not given.
		//   c     - A source byte to be copied for insertion.
		//   len   - Length of string p, or how often to insert
		//           byte c.

	emString GetSubString(int index, int len) const;
		// Get a sub-string.
		// Arguments:
		//   index - Index of the first byte of the sub-string.
		//           This should be within the range of 0 to
		//           GetLen()-len, otherwise index and len are clipped
		//           accordingly.
		//   len   - Length of the sub-string.
		// Returns: The sub-string.

	emString Extract(int index, int len);
		// Like GetSubString, but remove the sub-string from this
		// string.

	void Remove(int index, int len=1);
		// Like Extract, but without returning the sub-string.

	void Empty();
		// Empty this string.

	bool IsEmpty() const;
		// Ask whether this string is empty.

	bool operator == (const emString & s) const;
	bool operator != (const emString & s) const;
	bool operator <= (const emString & s) const;
	bool operator >= (const emString & s) const;
	bool operator < (const emString & s) const;
	bool operator > (const emString & s) const;
	bool operator == (const char * p) const;
	bool operator != (const char * p) const;
	bool operator <= (const char * p) const;
	bool operator >= (const char * p) const;
	bool operator < (const char * p) const;
	bool operator > (const char * p) const;
	friend bool operator == (const char * p, const emString & s);
	friend bool operator != (const char * p, const emString & s);
	friend bool operator <= (const char * p, const emString & s);
	friend bool operator >= (const char * p, const emString & s);
	friend bool operator < (const char * p, const emString & s);
	friend bool operator > (const char * p, const emString & s);
		// Compare null-terminated strings using strcmp.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this string.

	void MakeNonShared();
		// This must be called before handing the string to another
		// thread.

#if defined(ANDROID)
	// ??? This is a workaround: On Android, the program crashes if an
	// ??? exception is thrown with an instance of a non-polymorphic class
	// ??? from within a shared library and caught in another shared
	// ??? library. Therefore we make emString a polymorphic class for that
	// ??? case here. It could be that this problem is not just Android
	// ??? related.
	virtual void VirtualDummyMethod();
#endif

private:

	struct SharedData {
		unsigned int RefCount;
		char Buf[sizeof(unsigned int)];
	};

	emString(SharedData * d);
	void FreeData();
	void MakeWritable();
	void PrivRep(int oldLen, int index, int exLen, const char * p, int len);
	void PrivRep(int oldLen, int index, int exLen, char c, int len);

	static SharedData EmptyData;
	SharedData * Data;
};

#ifndef EM_NO_DATA_EXPORT
inline emString::emString()
{
	Data=&EmptyData;
}
#endif

inline emString::emString(const emString & s)
{
	Data=s.Data;
	Data->RefCount++;
}

inline emString::~emString()
{
	if (!--Data->RefCount) FreeData();
}

inline emString & emString::operator = (const emString & s)
{
	s.Data->RefCount++;
	if (!--Data->RefCount) FreeData();
	Data=s.Data;
	return *this;
}

inline int emString::GetCount() const
{
	return strlen(Data->Buf);
}

inline int emString::GetLen() const
{
	return strlen(Data->Buf);
}

inline emString::operator const char * () const
{
	return Data->Buf;
}

inline const char * emString::Get() const
{
	return Data->Buf;
}

inline char emString::operator [] (int index) const
{
	return Data->Buf[index];
}

inline char emString::Get(int index) const
{
	return Data->Buf[index];
}

inline char * emString::GetWritable()
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Buf;
}

inline char & emString::GetWritable(int index)
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Buf[index];
}

inline emString & emString::operator += (const emString & s)
{
	Add(s);
	return *this;
}

inline emString & emString::operator += (const char * p)
{
	Add(p);
	return *this;
}

inline emString & emString::operator += (char c)
{
	Add(c);
	return *this;
}

#ifndef EM_NO_DATA_EXPORT
inline void emString::Empty()
{
	if (!--Data->RefCount) FreeData();
	Data=&EmptyData;
}
#endif

inline bool emString::IsEmpty() const
{
	return !*Data->Buf;
}

inline bool emString::operator == (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)==0;
}

inline bool emString::operator != (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)!=0;
}

inline bool emString::operator <= (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)<=0;
}

inline bool emString::operator >= (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)>=0;
}

inline bool emString::operator < (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)<0;
}

inline bool emString::operator > (const emString & s) const
{
	return strcmp(Data->Buf,s.Data->Buf)>0;
}

inline bool emString::operator == (const char * p) const
{
	return strcmp(Data->Buf,p)==0;
}

inline bool emString::operator != (const char * p) const
{
	return strcmp(Data->Buf,p)!=0;
}

inline bool emString::operator <= (const char * p) const
{
	return strcmp(Data->Buf,p)<=0;
}

inline bool emString::operator >= (const char * p) const
{
	return strcmp(Data->Buf,p)>=0;
}

inline bool emString::operator < (const char * p) const
{
	return strcmp(Data->Buf,p)<0;
}

inline bool emString::operator > (const char * p) const
{
	return strcmp(Data->Buf,p)>0;
}

inline bool operator == (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)==0;
}

inline bool operator != (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)!=0;
}

inline bool operator <= (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)<=0;
}

inline bool operator >= (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)>=0;
}

inline bool operator < (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)<0;
}

inline bool operator > (const char * p, const emString & s)
{
	return strcmp(p,s.Data->Buf)>0;
}

inline void emString::MakeNonShared()
{
	MakeWritable();
}

inline emString::emString(SharedData * d)
{
	Data=d;
}


#endif
