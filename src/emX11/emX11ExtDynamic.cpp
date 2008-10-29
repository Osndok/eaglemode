//------------------------------------------------------------------------------
// emX11ExtDynamic.cpp
//
// Copyright (C) 2008 Oliver Hamann.
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

#include <emX11/emX11ExtDynamic.h>


//==============================================================================
//========================= Dynamic access to libXext ==========================
//==============================================================================

static const char * emX11_LibXextName=
#	if defined(_WIN32)
		"libXext.dll"
#	elif defined(__CYGWIN__)
		"cygXext-6.dll"
#	elif defined (__sun__)
		"libXext.so.0"
#	else
		"libXext.so.6"
#	endif
;


static const char * emX11_LibXextFuncNames[6]={
	"XShmAttach",
	"XShmCreateImage",
	"XShmDetach",
	"XShmGetEventBase",
	"XShmPutImage",
	"XShmQueryVersion"
};


void * emX11_LibXextFunctions[6]={
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


static bool emX11_LibXextLoaded=false;


void emX11_LoadLibXext() throw(emString)
{
	emLibHandle h;
	int i;

	// ??? not thread-save.
	if (!emX11_LibXextLoaded) {
		h=emTryOpenLib(emX11_LibXextName,true);
		for (i=0; i<(int)(sizeof(emX11_LibXextFunctions)/sizeof(void*)); i++) {
			emX11_LibXextFunctions[i]=emTryResolveSymbolFromLib(
				h,
				emX11_LibXextFuncNames[i]
			);
		}
		emX11_LibXextLoaded=true;
	}
}


bool emX11_IsLibXextLoaded()
{
	return emX11_LibXextLoaded;
}


//==============================================================================
//======================== Dynamic access to libXxf86vm ========================
//==============================================================================

static const char * emX11_LibXxf86vmName=
#	if defined(_WIN32) || defined(__CYGWIN__)
		"libXxf86vm.dll"
#	else
		"libXxf86vm.so.1"
#	endif
;


static const char * emX11_LibXxf86vmFuncNames[4]={
	"XF86VidModeGetModeLine",
	"XF86VidModeGetViewPort",
	"XF86VidModeQueryExtension",
	"XF86VidModeQueryVersion"
};


void * emX11_LibXxf86vmFunctions[4]={
	NULL,
	NULL,
	NULL,
	NULL
};


static bool emX11_LibXxf86vmLoaded=false;


void emX11_LoadLibXxf86vm() throw(emString)
{
	emLibHandle h;
	int i;

	// ??? not thread-save.
	if (!emX11_LibXxf86vmLoaded) {
		h=emTryOpenLib(emX11_LibXxf86vmName,true);
		for (i=0; i<(int)(sizeof(emX11_LibXxf86vmFunctions)/sizeof(void*)); i++) {
			emX11_LibXxf86vmFunctions[i]=emTryResolveSymbolFromLib(
				h,
				emX11_LibXxf86vmFuncNames[i]
			);
		}
		emX11_LibXxf86vmLoaded=true;
	}
}


bool emX11_IsLibXxf86vmLoaded()
{
	return emX11_LibXxf86vmLoaded;
}
