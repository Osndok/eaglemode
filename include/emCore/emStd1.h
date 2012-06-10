//------------------------------------------------------------------------------
// emStd1.h
//
// Copyright (C) 2004-2012 Oliver Hamann.
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

#ifndef emStd1_h
#define emStd1_h

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


//==============================================================================
//================================== Version ===================================
//==============================================================================

#define EM_MAJOR_VERSION 0
#define EM_MINOR_VERSION 84
#define EM_MICRO_VERSION 0
#define EM_VERSION_POSTFIX ""
	// Version numbers and postfix. Postfix is a string like ".rc1" or "".

const char * emGetVersion();
	// Version numbers and postfix as a string.

// The following static variable is built into every object file. Its
// constructor checks whether the object has been compiled with a binary
// compatible version of emCore. If not, emFatalError is called.
class emCompatibilityCheckerClass {
public:
	emCompatibilityCheckerClass(int maj, int min, int mic, const char * postfix);
};
static emCompatibilityCheckerClass emCompatibilityChecker(
	EM_MAJOR_VERSION,EM_MINOR_VERSION,EM_MICRO_VERSION,EM_VERSION_POSTFIX
);


//==============================================================================
//===================== Adaptations to compilers and OSes ======================
//==============================================================================

// We do not export variables from Windows DLLs.
#if defined(_WIN32) && !defined(__CYGWIN__)
#	ifndef EM_NO_DATA_EXPORT
#		define EM_NO_DATA_EXPORT
#	endif
#endif

// Get rid of warnings from GCC 3 about the offsetof macro.
// GCC 4 has the option -Wno-invalid-offsetof.
#if defined(__GNUC__) && __GNUC__==3
#	ifdef offsetof
#		undef offsetof
#		define offsetof(TYPE,MEMBER) (((size_t)&((TYPE*)256)->MEMBER)-256)
#	endif
#endif

// 'typename' is not supported by every compiler.
// ??? Watcom/OpenWatcom: __WATCOMC__==1100 does not have it, __WATCOMC__==1270
// ??? has it. For other versions I don't know it.
#if defined(__WATCOMC__) && __WATCOMC__<1270
#	ifndef typename
#		define typename
#	endif
#endif

// Imitation of some UNIX functions on Windows
#if defined(_WIN32)
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define snprintf _snprintf
#	define vsnprintf _vsnprintf
	char * em_asctime_r(const struct tm * ptm, char * buf);
#	define asctime_r em_asctime_r
	char * em_ctime_r(const time_t * ptime, char * buf);
#	define ctime_r em_ctime_r
	struct tm * em_gmtime_r(const time_t * ptime, struct tm * buf);
#	define gmtime_r em_gmtime_r
	struct tm * em_localtime_r(const time_t * ptime, struct tm * buf);
#	define localtime_r em_localtime_r
#endif

// Have M_PI
#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif


//==============================================================================
//=================== About RTTI (Run-Time Type Information) ===================
//==============================================================================

#if defined(_MSC_VER) || defined(__WATCOMC__)
#	include <typeinfo.h>
	inline const char * emRawNameOfTypeInfo(const type_info & t)
	{
		return t.raw_name();
	}
#else
#	include <typeinfo>
	using std::type_info;
	inline const char * emRawNameOfTypeInfo(const type_info & t)
	{
		return t.name();
	}
#endif


//==============================================================================
//========================= About function attributes ==========================
//==============================================================================

#if defined(__GNUC__)
#	define EM_FUNC_ATTR_PRINTF(pos) __attribute__((format(__printf__,pos,pos+1)))
#else
#	define EM_FUNC_ATTR_PRINTF(pos)
#endif


//==============================================================================
//========================= Byte order of the machine ==========================
//==============================================================================

// EM_BYTE_ORDER is 1234 (little endian) or 4321 (big endian) or maybe 3412.

#if defined(__FreeBSD__) || defined(ANDROID)
#	include <sys/endian.h>
#endif

#if defined(__BYTE_ORDER) &&\
    (__BYTE_ORDER==1234 || __BYTE_ORDER==4321 || __BYTE_ORDER==3412)
#	define EM_BYTE_ORDER __BYTE_ORDER
#elif defined(_BYTE_ORDER) &&\
      (_BYTE_ORDER==1234 || _BYTE_ORDER==4321 || _BYTE_ORDER==3412)
#	define EM_BYTE_ORDER _BYTE_ORDER
#elif defined(BYTE_ORDER) &&\
      (BYTE_ORDER==1234 || BYTE_ORDER==4321 || BYTE_ORDER==3412)
#	define EM_BYTE_ORDER BYTE_ORDER
#elif defined(_M_IX86) || defined(_M_AMD64) || defined(__i386__)
#	define EM_BYTE_ORDER 1234
#else
#	error "don't know how to define EM_BYTE_ORDER"
#endif


//==============================================================================
//============================= Integer data types =============================
//==============================================================================

typedef signed   char emInt8;
typedef unsigned char emUInt8;
typedef emUInt8       emByte;

#if USHRT_MAX == 0xFFFF
	typedef signed   short emInt16;
	typedef unsigned short emUInt16;
#else
	typedef signed   __int16 emInt16;
	typedef unsigned __int16 emUInt16;
#endif

#if UINT_MAX == 0xFFFFFFFFL
	typedef signed   int  emInt32;
	typedef unsigned int  emUInt32;
#elif ULONG_MAX == 0xFFFFFFFFL
	typedef signed   long emInt32;
	typedef unsigned long emUInt32;
#else
	typedef signed   __int32 emInt32;
	typedef unsigned __int32 emUInt32;
#endif

#if defined(__GNUC__) || defined(__SUNPRO_CC)
#	if __WORDSIZE == 64
		typedef signed   long emInt64;
		typedef unsigned long emUInt64;
#	else
		typedef signed   long long emInt64;
		typedef unsigned long long emUInt64;
#	endif
#else
	typedef signed   __int64 emInt64;
	typedef unsigned __int64 emUInt64;
#endif


#define EM_INT8_MIN   ((emInt8)0x80)
#define EM_INT8_MAX   ((emInt8)0x7f)
#define EM_UINT8_MAX  ((emUInt8)0xff)
#define EM_INT16_MIN  ((emInt16)0x8000)
#define EM_INT16_MAX  ((emInt16)0x7fff)
#define EM_UINT16_MAX ((emUInt16)0xffff)
#define EM_INT32_MIN  ((emInt32)0x80000000)
#define EM_INT32_MAX  ((emInt32)0x7fffffff)
#define EM_UINT32_MAX ((emUInt32)0xffffffff)
#define EM_INT64_MIN  ((emInt64)((((emUInt64)0x80000000)<<32)))
#define EM_INT64_MAX  ((emInt64)(((((emUInt64)0x7fffffff)<<32)|0xffffffff)))
#define EM_UINT64_MAX ((emUInt64)(((((emUInt64)0xffffffff)<<32)|0xffffffff)))


int emStrToInt64(const char * str, int strLen, emInt64 * pVal);
int emStrToUInt64(const char * str, int strLen, emUInt64 * pVal);
	// Parse a decimal 64-bit integer. Returns the number of characters
	// parsed, or 0 on error.

int emInt64ToStr(char * buf, int bufLen, emInt64 val);
int emUInt64ToStr(char * buf, int bufLen, emUInt64 val);
	// Convert a 64-integer to a decimal string. Returns the number of
	// characters produced, or 0 if the buffer is too small.


//==============================================================================
//========================== Locale and UTF-8 support ==========================
//==============================================================================

void emInitLocale();
	// This must be called once by the main function of the program before
	// doing anything else.


//-------------------------- Low-level UTF-8 support ---------------------------

bool emIsUtf8System();
	// Ask whether it should be expected that filenames, text files,
	// environment variables and so on are encoded in UTF-8. Otherwise we
	// assume something like Latin-1. All strings in our program should be
	// encoded in the same way.

int emEncodeUtf8Char(char * utf8, int ucs4);
	// Encode a UTF-8 string from a single 31-bit Unicode character.
	// Arguments:
	//   utf8 - Buffer for returning the UTF-8 encoded string.
	//          It is _NOT_ terminated by a null. The buffer must
	//          have space for at least 6 bytes.
	//   ucs4 - The Unicode character.
	// Returns: Number of bytes written to the buffer (1 - 6).

int emDecodeUtf8Char(int * pUcs4, const char * utf8, int utf8Len=INT_MAX);
	// Decode one 31-bit Unicode character from a UTF-8 string.
	// Arguments:
	//   pUcs4   - Pointer for returning the Unicode character. On error,
	//             utf8[0] is set here. And if the UTF-8 string is empty,
	//             zero is set.
	//   utf8    - The UTF-8 string.
	//   utf8Len - Maximum number of bytes to be read from the UTF-8
	//             string, if it is not null-terminated.
	// Returns:
	//   On success, the number of bytes read from the UTF-8 string is
	//   returned (1 - 6). If the UTF-8 string is empty, zero is returned.
	//   And if the UTF-8 string does not contain a valid and complete code
	//   sequence, -1 is returned.

//-------------------------- High-level UTF-8 support --------------------------

int emEncodeChar(char * str, int ucs4);
	// Encode a string from a single 31-bit Unicode character. The type of
	// encoding depends on emIsUtf8System.
	// Arguments:
	//   str  - Buffer for returning the encoded string.
	//          It is _NOT_ terminated by a null. The buffer must
	//          have space for at least 6 bytes.
	//   ucs4 - The Unicode character.
	// Returns: Number of bytes written to the buffer (1 - 6).

int emDecodeChar(int * pUcs4, const char * str, int strLen=INT_MAX);
	// Decode one 31-bit Unicode character from a string. The type of
	// decoding depends on emIsUtf8System. Error characters in a UTF-8
	// string are interpreted as Latin-1 here.
	// Arguments:
	//   pUcs4  - Pointer for returning the Unicode character. On error,
	//            str[0] is set here. And if the encoded string is empty,
	//            zero is set.
	//   str    - The encoded string.
	//   strLen - Maximum number of bytes to be read from the encoded
	//            string, if it is not null-terminated.
	// Returns: Number of bytes read from the encoded string (0 - 6).

int emGetDecodedCharCount(const char * str, int strLen=INT_MAX);
	// Get the number of 31-bit Unicode characters which could be decoded
	// from a string. The type of decoding depends on emIsUtf8System. Error
	// characters in a UTF-8 string are interpreted as Latin-1 here.
	// Arguments:
	//   str    - The encoded string.
	//   strLen - Maximum number of bytes to be read from the encoded
	//            string, if it is not null-terminated.
	// Returns: Number of characters which could be decoded from the string.


//==============================================================================
//========================= Logs, warnings and errors ==========================
//==============================================================================

void emLog(const char * format, ...) EM_FUNC_ATTR_PRINTF(1);
	// Output a debug or warning message. A line break is appended. The
	// message is written to stderr, but if under Windows and stderr fails,
	// the message is written to a log file named "emCoreBasedAppLog.log" in
	// a temp directory (the directory returned by the Win-API function
	// GetTempPath).

void emEnableDLog(bool devLogEnabled=true);
bool emIsDLogEnabled();
	// Whether messages for development and debugging should be made. This
	// is to be modified by the main program only, maybe through a command
	// line option. The default is 'false'.

void emDLog(const char * format, ...) EM_FUNC_ATTR_PRINTF(1);
	// Like emLog, but the call is ignored if emIsDLogEnabled()==false.

void emWarning(const char * format, ...) EM_FUNC_ATTR_PRINTF(1);
	// Like emLog, but "WARNING: " is prepended.

void emFatalError(const char * format, ...) EM_FUNC_ATTR_PRINTF(1);
	// Output an error message and exit this process. The message is written
	// to stderr (a line break is appended). Optionally it is also shown in
	// a GUI message box - see emSetFatalErrorGraphical.

void emSetFatalErrorGraphical(bool graphical);
	// Whether to show the fatal error message in a dialog (default: false).


//==============================================================================
//================================ emUncopyable ================================
//==============================================================================

class emUncopyable
	// Objects of this class can never be copied.
{
public:
	inline emUncopyable() {}
private:
	inline emUncopyable(const emUncopyable &) {}
	inline emUncopyable & operator = (const emUncopyable &) { return *this; }
};


//==============================================================================
//============================= emUnconstructable ==============================
//==============================================================================

class emUnconstructable
	// Objects of this class can never be constructed.
{
public:
	static inline void DummyMethod() {}
private:
	inline emUnconstructable() {}
};


//==============================================================================
//=============================== emMin & emMax ================================
//==============================================================================

template <class OBJ> inline OBJ emMin(OBJ a, OBJ b) { return a<b ? a : b; }
template <class OBJ> inline OBJ emMax(OBJ a, OBJ b) { return a>b ? a : b; }
	// Return the minimum or maximum of two things.


//==============================================================================
//=============================== emStdComparer ================================
//==============================================================================

template <class OBJ> class emStdComparer : public emUnconstructable {
public:
	// The following functions can be used as an argument for certain
	// functions about sorting and searching (e.g. emSortArray).
	static int Compare(const OBJ * a, const OBJ * b, void * context)
	{
		if (*a<*b) return -1;
		if (*a>*b) return 1;
		return 0;
	}
	static int ReverseCompare(const OBJ * a, const OBJ * b, void * context)
	{
		if (*a<*b) return 1;
		if (*a>*b) return -1;
		return 0;
	}
};


//==============================================================================
//================================ emAlignment =================================
//==============================================================================

enum {
	EM_ALIGN_CENTER       = 0,
	EM_ALIGN_TOP          = (1<<0),
	EM_ALIGN_BOTTOM       = (1<<1),
	EM_ALIGN_LEFT         = (1<<2),
	EM_ALIGN_RIGHT        = (1<<3),
	EM_ALIGN_TOP_LEFT     = EM_ALIGN_TOP    | EM_ALIGN_LEFT,
	EM_ALIGN_TOP_RIGHT    = EM_ALIGN_TOP    | EM_ALIGN_RIGHT,
	EM_ALIGN_BOTTOM_LEFT  = EM_ALIGN_BOTTOM | EM_ALIGN_LEFT,
	EM_ALIGN_BOTTOM_RIGHT = EM_ALIGN_BOTTOM | EM_ALIGN_RIGHT
};
typedef emByte emAlignment;
	// Data type for the alignment of something within a rectangle.

const char * emAlignmentToString(emAlignment alignment);
emAlignment emStringToAlignment(const char * str);
	// Convert an alignment to and from string representation.


#endif
