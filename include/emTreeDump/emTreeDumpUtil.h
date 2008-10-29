//------------------------------------------------------------------------------
// emTreeDumpUtil.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef emTreeDumpUtil_h
#define emTreeDumpUtil_h

#ifndef emContext_h
#include <emCore/emContext.h>
#endif

#ifndef emTreeDumpRec_h
#include <emTreeDump/emTreeDumpRec.h>
#endif


void emTreeDumpFromObject(emEngine * object, emTreeDumpRec * rec);

void emTreeDumpFromRootContext(
	emRootContext * rootContext, emTreeDumpRec * rec
);

void emTryTreeDumpFileFromRootContext(
	emRootContext * rootContext, const char * filename
) throw(emString);

extern "C" {
	bool emTreeDumpFileFromRootContext(
		emRootContext * rootContext, const char * filename,
		emString * errorBuf
	);
}


#endif
