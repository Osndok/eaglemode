//------------------------------------------------------------------------------
// emX11ExtDynamic.h
//
// Copyright (C) 2008-2009 Oliver Hamann.
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

#ifndef emX11ExtDynamic_h
#define emX11ExtDynamic_h

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//========================= Dynamic access to libXext ==========================
//==============================================================================

void emX11_TryLoadLibXext() throw(emString);
bool emX11_IsLibXextLoaded();

#define ShmCompletion 0

typedef struct {
	int type;
	unsigned long serial;
	Bool send_event;
	Display * display;
	Drawable drawable;
	int major_code;
	int minor_code;
	unsigned long shmseg;
	unsigned long offset;
} XShmCompletionEvent;

typedef struct {
	unsigned long shmseg;
	int shmid;
	char * shmaddr;
	Bool readOnly;
} XShmSegmentInfo;

extern void * emX11_LibXextFunctions[6];

#define XShmAttach (\
	(Status(*)(Display*,XShmSegmentInfo*))\
	emX11_LibXextFunctions[0]\
)
#define XShmCreateImage (\
	(XImage*(*)(Display*,Visual*,unsigned int,int,char*,XShmSegmentInfo*,\
	            unsigned int,unsigned int))\
	emX11_LibXextFunctions[1]\
)
#define XShmDetach (\
	(Status(*)(Display*,XShmSegmentInfo*))\
	emX11_LibXextFunctions[2]\
)
#define XShmGetEventBase (\
	(int(*)(Display*))\
	emX11_LibXextFunctions[3]\
)
#define XShmPutImage (\
	(Status(*)(Display*,Drawable,GC,XImage*,int,int,int,int,unsigned int,\
	           unsigned int,Bool))\
	emX11_LibXextFunctions[4]\
)
#define XShmQueryVersion (\
	(Bool(*)(Display*,int*,int*,Bool*))\
	emX11_LibXextFunctions[5]\
)


//==============================================================================
//======================== Dynamic access to libXxf86vm ========================
//==============================================================================

void emX11_TryLoadLibXxf86vm() throw(emString);
bool emX11_IsLibXxf86vmLoaded();

typedef struct {
	unsigned short hdisplay;
	unsigned short hsyncstart;
	unsigned short hsyncend;
	unsigned short htotal;
	unsigned short hskew;
	unsigned short vdisplay;
	unsigned short vsyncstart;
	unsigned short vsyncend;
	unsigned short vtotal;
	unsigned int flags;
	int privsize;
	void * c_private;
} XF86VidModeModeLine;

extern void * emX11_LibXxf86vmFunctions[4];

#define XF86VidModeGetModeLine (\
	(Bool(*)(Display*,int,int*,XF86VidModeModeLine*))\
	emX11_LibXxf86vmFunctions[0]\
)
#define XF86VidModeGetViewPort (\
	(Bool(*)(Display*,int,int*,int*))\
	emX11_LibXxf86vmFunctions[1]\
)
#define XF86VidModeQueryExtension (\
	(Bool(*)(Display*,int*,int*))\
	emX11_LibXxf86vmFunctions[2]\
)
#define XF86VidModeQueryVersion (\
	(Bool(*)(Display*,int*,int*))\
	emX11_LibXxf86vmFunctions[3]\
)


#endif
