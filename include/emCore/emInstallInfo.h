//------------------------------------------------------------------------------
// emInstallInfo.h
//
// Copyright (C) 2006-2008,2010-2011 Oliver Hamann.
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

#ifndef emInstallInfo_h
#define emInstallInfo_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif

class emContext;


//==============================================================================
//=========================== Install info functions ===========================
//==============================================================================

enum emInstallDirType {
	EM_IDT_BIN        = 0,
	EM_IDT_INCLUDE    = 1,
	EM_IDT_LIB        = 2,
	EM_IDT_HTML_DOC   = 3,
	EM_IDT_PS_DOC     = 4,
	EM_IDT_USER_CONFIG= 5,
	EM_IDT_HOST_CONFIG= 6,
	EM_IDT_TMP        = 7,
	EM_IDT_RES        = 8,
	EM_IDT_HOME       = 9,
	EM_NUMBER_OF_IDTS =10
};

emString emGetInstallPath(emInstallDirType idt, const char * prj,
                          const char * subPath=NULL);
	// Get the installation path of a file or directory.
	// Arguments:
	//   idt     - Type of installation directory.
	//   prj     - Name of the sub-project. Currently this is ignored for
	//             some install dir types, but that could change in the
	//             future, so please always set prj best possible.
	//   subPath - A sub-path to be appended, or NULL.
	// Returns: The installation path.
	//
	// The following table should make things clearer:
	//
	// Install Dir Type   | Current Implementation | A Future Example
	// -------------------+------------------------+------------------------
	// EM_IDT_BIN         | <em>/bin               | /usr/bin
	// EM_IDT_INCLUDE     | <em>/include/<prj>     | /usr/include/<prj>
	// EM_IDT_LIB         | <em>/lib               | /usr/lib
	// EM_IDT_HTML_DOC    | <em>/doc/html          | /usr/share/doc/em/<prj>/html
	// EM_IDT_PS_DOC      | <em>/doc/ps            | /usr/share/doc/em/<prj>/ps
	// EM_IDT_USER_CONFIG | <home>/.eaglemode/<prj>| <home>/.em/<prj>
	// EM_IDT_HOST_CONFIG | <em>/etc/<prj>         | /etc/em/<prj>
	// EM_IDT_TMP         | $TMPDIR or /tmp        | $TMPDIR or /tmp
	// EM_IDT_RES         | <em>/res/<prj>         | /usr/share/emRes/<prj>
	// EM_IDT_HOME        | <home>                 | <home>
	// Hint: The implementation for Windows is a little bit different.


emString emGetConfigDirOverloadable(emContext & context, const char * prj,
                                    const char * subDir=NULL);
	// This method returns either
	//   emGetInstallPath(EM_IDT_HOST_CONFIG,prj,subDir)
	// or
	//   emGetInstallPath(EM_IDT_USER_CONFIG,prj,subDir)
	// Idea is that the user can make a copy of a directory from host config
	// to user config in order to manipulate it there. But if that copy gets
	// outdated through an update of the other, it should no longer be used
	// and the user should be warned. Therefore, both directories must
	// contain a file named "version" which contains an integer version
	// number. The user directory is returned only if the versions are
	// equal, otherwise the host directory is returned. In addition, a
	// warning is shown to the user if the user directory exists and if its
	// version differs or cannot be read. The warning is made through a
	// dialog if an emScreen can be found in the given context (or higher),
	// otherwise the warning is reported through emWarning. If the version
	// file of the host directory cannot be read, emFatalError is called. It
	// is okay to call this function multiple times in a program run,
	// because it remembers each shown warning for not showing it again, as
	// long as the root context lives.


#endif
