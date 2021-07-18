//------------------------------------------------------------------------------
// emX11ExtDynamic.cpp
//
// Copyright (C) 2008-2009,2014,2016,2018-2019,2021 Oliver Hamann.
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
#include <emCore/emThread.h>


//==============================================================================
//========================= Dynamic access to libXext ==========================
//==============================================================================

static const char * const emX11_LibXextName=
#	if defined(_WIN32)
		"libXext.dll"
#	elif defined(__CYGWIN__)
		"cygXext-6.dll"
#	elif defined(__APPLE__)
		"/usr/X11/lib/libXext.6.dylib"
#	elif defined(__sun__)
		"libXext.so.0"
#	elif defined(__NetBSD__)
		"/usr/X11R7/lib/libXext.so"
#	else
		"libXext.so.6"
#	endif
;


static const char * const emX11_LibXextFuncNames[6]={
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


static emThreadMiniMutex emX11_LibXextLoadMutex;
static bool emX11_LibXextLoaded=false;


void emX11_TryLoadLibXext()
{
	emLibHandle h;
	int i;

	emX11_LibXextLoadMutex.Lock();
	if (!emX11_LibXextLoaded) {
		try {
			h=emTryOpenLib(emX11_LibXextName,true);
			for (i=0; i<(int)(sizeof(emX11_LibXextFunctions)/sizeof(void*)); i++) {
				emX11_LibXextFunctions[i]=emTryResolveSymbolFromLib(
					h,
					emX11_LibXextFuncNames[i]
				);
			}
		}
		catch (const emException & exception) {
			emX11_LibXextLoadMutex.Unlock();
			throw exception;
		}
		emX11_LibXextLoaded=true;
	}
	emX11_LibXextLoadMutex.Unlock();
}


bool emX11_IsLibXextLoaded()
{
	return emX11_LibXextLoaded;
}


//==============================================================================
//======================== Dynamic access to libXxf86vm ========================
//==============================================================================

static const char * const emX11_LibXxf86vmName=
#	if defined(_WIN32) || defined(__CYGWIN__)
		"libXxf86vm.dll"
#	elif defined(__APPLE__)
		"/usr/X11/lib/libXxf86vm.1.dylib"
#	elif defined(__NetBSD__)
		"/usr/X11R7/lib/libXxf86vm.so"
#	else
		"libXxf86vm.so.1"
#	endif
;


static const char * const emX11_LibXxf86vmFuncNames[4]={
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


static emThreadMiniMutex emX11_LibXxf86vmLoadMutex;
static bool emX11_LibXxf86vmLoaded=false;


void emX11_TryLoadLibXxf86vm()
{
	emLibHandle h;
	int i;

	emX11_LibXxf86vmLoadMutex.Lock();
	if (!emX11_LibXxf86vmLoaded) {
		try {
			h=emTryOpenLib(emX11_LibXxf86vmName,true);
			for (i=0; i<(int)(sizeof(emX11_LibXxf86vmFunctions)/sizeof(void*)); i++) {
				emX11_LibXxf86vmFunctions[i]=emTryResolveSymbolFromLib(
					h,
					emX11_LibXxf86vmFuncNames[i]
				);
			}
		}
		catch (const emException & exception) {
			emX11_LibXxf86vmLoadMutex.Unlock();
			throw exception;
		}
		emX11_LibXxf86vmLoaded=true;
	}
	emX11_LibXxf86vmLoadMutex.Unlock();
}


bool emX11_IsLibXxf86vmLoaded()
{
	return emX11_LibXxf86vmLoaded;
}


//==============================================================================
//======================= Dynamic access to libXinerama ========================
//==============================================================================

static const char * const emX11_LibXineramaName=
#	if defined(_WIN32)
		"libXinerama.dll"
#	elif defined(__CYGWIN__)
		"cygXinerama-1.dll"
#	elif defined(__APPLE__)
		"/usr/X11/lib/libXinerama.1.dylib"
#	elif defined(__NetBSD__)
		"/usr/X11R7/lib/libXinerama.so"
#	else
		"libXinerama.so.1"
#	endif
;


static const char * const emX11_LibXineramaFuncNames[4]={
	"XineramaQueryExtension",
	"XineramaQueryScreens",
	"XineramaQueryVersion"
};


void * emX11_LibXineramaFunctions[3]={
	NULL,
	NULL,
	NULL
};


static emThreadMiniMutex emX11_LibXineramaLoadMutex;
static bool emX11_LibXineramaLoaded=false;


void emX11_TryLoadLibXinerama()
{
	emLibHandle h;
	int i;

	emX11_LibXineramaLoadMutex.Lock();
	if (!emX11_LibXineramaLoaded) {
		try {
			h=emTryOpenLib(emX11_LibXineramaName,true);
			for (i=0; i<(int)(sizeof(emX11_LibXineramaFunctions)/sizeof(void*)); i++) {
				emX11_LibXineramaFunctions[i]=emTryResolveSymbolFromLib(
					h,
					emX11_LibXineramaFuncNames[i]
				);
			}
		}
		catch (const emException & exception) {
			emX11_LibXineramaLoadMutex.Unlock();
			throw exception;
		}
		emX11_LibXineramaLoaded=true;
	}
	emX11_LibXineramaLoadMutex.Unlock();
}


bool emX11_IsLibXineramaLoaded()
{
	return emX11_LibXineramaLoaded;
}
