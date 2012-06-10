//------------------------------------------------------------------------------
// emStd2.h
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

#ifndef emStd2_h
#define emStd2_h

#include <sys/stat.h>

#ifndef emArray_h
#include <emCore/emArray.h>
#endif

#ifndef emString_h
#include <emCore/emString.h>
#endif


//==============================================================================
//=========================== Host, user, process id ===========================
//==============================================================================

emString emGetHostName();
	// Get the name of the computer.

emString emGetUserName();
	// Get the name of the user.

int emGetProcessId();
	// Get the identification number of the process.


//==============================================================================
//=============================== Time functions ===============================
//==============================================================================

void emSleepMS(int millisecs);
	// Sleep for the given number of milliseconds. Less or equal zero means
	// to yield the CPU to another process.

emUInt64 emGetClockMS();
	// Get a system clock time in milliseconds. It starts anywhere, but it
	// should never overflow.

emUInt64 emGetCPUTSC();
	// Get the state of the time stamp counter (TSC) of the CPU.
	// IMPORTANT: This only works with certain compiler and hardware.
	// Use for debugging/development only.


//==============================================================================
//================================ Error texts =================================
//==============================================================================

emString emGetErrorText(int errorNumber);
	// This is like the C function strerror, but it is thread-safe, and on
	// Windows it also supports error codes returned by GetLastError.


//==============================================================================
//============================ Files & directories =============================
//==============================================================================

// em_stat and em_lstat are like stat and lstat from sys/stat.h, but with 64-bit
// file size field if possible.
#if defined(__linux__) && !defined(__SUNPRO_CC)
#	define em_stat stat64
#	define em_lstat lstat64
#elif defined(_WIN32)
#	if defined(_MSC_VER) || defined(__GNUC__) || defined(__WATCOMC__)
#		define em_stat _stati64
#	else
#		define em_stat stat
#	endif
#	define em_lstat em_stat
#else
#	define em_stat stat
#	define em_lstat lstat
#endif


emString emGetParentPath(const char * path);
	// Get the parent path of a path.
	// Examples:
	//  "" => ""
	//  "/" => "/"
	//  "/x" => "/"
	//  "/x/y" => "/x"
	//  "/x/y///" => "/x"

emString emGetChildPath(const char * path, const char * name);
	// Join a path and a name.
	// Examples:
	//  "/x/y", "z" => "x/y/z"
	//  "/x/y/", "z" => "x/y/z"

emString emGetAbsolutePath(const emString & path, const char * cwd=NULL);
	// Get the absolute path of a path. Symbolic links are not resolved. The
	// argument cwd can be used to simulate another current working
	// directory.

const char * emGetNameInPath(const char * path);
	// Return a pointer to the last name in a path (similar to basename).

const char * emGetExtensionInPath(const char * path);
	// Return a pointer to the extension including the dot of the last name
	// in a path.

bool emIsExistingPath(const char * path);
bool emIsReadablePath(const char * path);
bool emIsWritablePath(const char * path);
bool emIsDirectory(const char * path);
bool emIsRegularFile(const char * path);
bool emIsSymLinkPath(const char * path);
	// Ask whether the given file path exists, whether it is readable and so
	// on.

emUInt64 emTryGetFileSize(const char * path) throw(emString);
	// Get the size of a file.

time_t emTryGetFileTime(const char * path) throw(emString);
	// Get the last modification time of a file.

emString emGetCurrentDirectory();
	// Get the absolute path of the current working directory.

typedef void * emDirHandle;
emDirHandle emTryOpenDir(const char * path) throw(emString);
emString emTryReadDir(emDirHandle dirHandle) throw(emString);
void emCloseDir(emDirHandle dirHandle);
	// Read a directory step by step. An empty string indicates the end.

emArray<emString> emTryLoadDir(const char * path) throw(emString);
	// Read a directory at once.

emArray<char> emTryLoadFile(const char * path) throw(emString);
void emTrySaveFile(const char * path,
                   const char * data, int len) throw(emString);
void emTrySaveFile(const char * path,
                   const emArray<char> & data) throw(emString);
	// Read or write a file at once.

void emTryMakeDirectories(const char * path, int mode=0777) throw(emString);
	// Create a directory and its ancestors, as far as they do not exist.
	// On Windows, the mode argument is ignored.

void emTryRemoveFileOrTree(const char * path, bool force=false) throw(emString);
	// Delete a file or a directory recursively. force=true means to defeat
	// file permissions if possible.

void emTryCopyFileOrTree(const char * targetPath,
                         const char * sourcePath) throw(emString);
	// Copy a file or a directory recursively. This does not copy any file
	// attributes (maybe a future version will do so).


//==============================================================================
//=========================== Dynamic link libraries ===========================
//==============================================================================

typedef void * emLibHandle;
	// Data type for a handle on an opened dynamic library.

emLibHandle emTryOpenLib(const char * libName, bool isFilename) throw(emString);
	// Open a dynamic library.
	// Arguments:
	//   libName    - Name of the dynamic library.
	//   isFilename - false if libName is just a pure name, which has to be
	//                extended for making a file name (e.g. "Test" =>
	//                "libTest.so" or "Test.dll"). true if the name is
	//                already a file name or a file path.
	// Returns:
	//   An abstract handle for the opened library.

void * emTryResolveSymbolFromLib(emLibHandle handle,
                                 const char * symbol) throw(emString);
	// Get the address of a symbol in a dynamic library.
	// Hint: C++ symbols have a compiler specific encoding. Best is to use
	// C symbols only.

void emCloseLib(emLibHandle handle);
	// Close a dynamic library.

void * emTryResolveSymbol(const char * libName, bool isFilename,
                          const char * symbol) throw(emString);
	// Similar to emTryOpenLib plus emTryResolveSymbolFromLib, but the
	// library is never closed.


//==============================================================================
//=========================== Pseudo random numbers ============================
//==============================================================================

int          emGetIntRandom   (int          minimum, int          maximum);
unsigned int emGetUIntRandom  (unsigned int minimum, unsigned int maximum);
emInt64      emGetInt64Random (emInt64      minimum, emInt64      maximum);
emUInt64     emGetUInt64Random(emUInt64     minimum, emUInt64     maximum);
double       emGetDblRandom   (double       minimum, double       maximum);
	// Get a pseudo random number within the given range.
	// Cryptographically, this is absolutely not secure.


//==============================================================================
//========================== Checksums and hash codes ==========================
//==============================================================================

emUInt32 emCalcAdler32(const char * src, int srcLen, emUInt32 start=1);
emUInt32 emCalcCRC32(const char * src, int srcLen, emUInt32 start=0);
emUInt64 emCalcCRC64(const char * src, int srcLen, emUInt64 start=0);
	// Calculate an Adler-32, CRC-32 or CRC-64 "checksum" from some data.
	// Cryptographically, this is absolutely not secure.
	// Arguments:
	//   src     - Pointer to source data.
	//   srcLen  - Number of bytes in the source.
	//   start   - Start value (could be the result from a previous call).
	// Returns:
	//   The CRC value.

int emCalcHashCode(const char * str, int start=0);
	// Calculate another kind of "checksum" from a string (simple & fast).
	// Cryptographically, this is absolutely not secure.
	// Arguments:
	//   src     - The string.
	//   start   - Start value (could be the result from a previous call).
	// Returns:
	//   The hash code.

emString emCalcHashName(const char * src, int srcLen, int hashLen);
	// Calculate an any-length hash code ("checksum") from some data. The
	// result is a character string consisting of letters and digits. It's
	// okay to ignore letter case in comparisons. Cryptographically, this is
	// absolutely not secure.
	// Arguments:
	//   src     - Pointer to source data.
	//   srcLen  - Number of bytes in the source.
	//   hashLen - Length of the resulting hash name.
	// Returns:
	//   The hash name. It has hashLen characters and consists of
	//   letters (A-Z) and digits (0-9) only.


#endif
