//------------------------------------------------------------------------------
// emX11Screen.cpp
//
// Copyright (C) 2005-2012 Oliver Hamann.
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

#include <emX11/emX11Screen.h>
#include <emX11/emX11Clipboard.h>
#include <emX11/emX11WindowPort.h>
#include <sys/shm.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "cursors/emCursorInvisible.xbm"


void emX11Screen::Install(emContext & context)
{
	emX11Screen * m;
	emString name;

	m=(emX11Screen*)context.Lookup(typeid(emX11Screen),name);
	if (!m) {
		m=new emX11Screen(context,name);
		m->Register();
	}
	m->emScreen::Install();
}


double emX11Screen::GetWidth()
{
	return Width;
}


double emX11Screen::GetHeight()
{
	return Height;
}


void emX11Screen::GetVisibleRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	XF86VidModeModeLine ml;
	int x,y,dc;
	bool b;

	if (HaveXF86VidMode) {
		memset(&ml,0,sizeof(ml));
		x=y=dc=0;
		XMutex.Lock();
		b=XF86VidModeGetModeLine(Disp,Scrn,&dc,&ml) &&
		  XF86VidModeGetViewPort(Disp,Scrn,&x,&y);
		XMutex.Unlock();
		if (b) {
			*pX=x;
			*pY=y;
			*pW=ml.hdisplay;
			*pH=ml.vdisplay;
			return;
		}
	}
	*pX=0.0;
	*pY=0.0;
	*pW=Width;
	*pH=Height;
}


double emX11Screen::GetDPI()
{
	return DPI;
}


void emX11Screen::MoveMousePointer(double dx, double dy)
{
	MouseWarpX+=dx;
	MouseWarpY+=dy;
}


void emX11Screen::Beep()
{
	XMutex.Lock();
	XBell(Disp,0);
	XMutex.Unlock();
}


void emX11Screen::DisableScreensaver()
{
	if (ScreensaverDisableCounter==0 && !ScreensaverDisableTimer.IsRunning()) {
		ScreensaverDisableTimer.Start(0);
	}
	ScreensaverDisableCounter++;
}


void emX11Screen::EnableScreensaver()
{
	ScreensaverDisableCounter--;
}


emWindowPort * emX11Screen::CreateWindowPort(emWindow & window)
{
	return new emX11WindowPort(window);
}


emX11Screen::emX11Screen(emContext & context, const emString & name)
	: emScreen(context,name),
	ScreensaverDisableTimer(GetScheduler())
{
	const char * displayName, * modifiers;
	XVisualInfo * viList, viTemplate, * vi;
	int viCount,i,major,minor,bytesPerPixel,x,y,dc;
	Bool xshmCanDoPixmaps,allowTwoBuffers;
	XErrorHandler originalHandler;
	XF86VidModeModeLine ml;
	Status status;
	Bool xb;
	int eventBase,errorBase;

	displayName=XDisplayName(NULL);
	Disp=XOpenDisplay(displayName);
	if (!Disp) {
		emFatalError("Failed to open X display \"%s\".",displayName);
	}

	WCThread=new WaitCursorThread(XMutex,Disp);

	XMutex.Lock();
	xb=XSupportsLocale();
	XMutex.Unlock();
	if (!xb) {
		emWarning(
			"emX11Screen: X does not support current locale for display \"%s\".",
			displayName
		);
		InputMethod=NULL;
	}
	else {
		modifiers=getenv("XMODIFIERS");
		emDLog("emX11Screen: XMODIFIERS=%s",modifiers?modifiers:"<not defined>");
		XMutex.Lock();
		modifiers=XSetLocaleModifiers("");
		XMutex.Unlock();
		if (!modifiers) {
			XMutex.Lock();
			modifiers=XSetLocaleModifiers("@im=none");
			XMutex.Unlock();
		}
		if (!modifiers) {
			emWarning(
				"emX11Screen: Failed to set locale modifiers for display \"%s\".",
				displayName
			);
			InputMethod=NULL;
		}
		else {
			XMutex.Lock();
			InputMethod=XOpenIM(Disp,NULL,NULL,NULL);
			XMutex.Unlock();
			if (InputMethod==NULL) {
				emWarning(
					"emX11Screen: Failed to open X input method for display \"%s\".",
					displayName
				);
			}
		}
	}

	Scrn=DefaultScreen(Disp);

	Width=DisplayWidth(Disp,Scrn);
	Height=DisplayHeight(Disp,Scrn);

	DPI=Width*25.4/DisplayWidthMM(Disp,Scrn);
	PixelTallness=1.0; //??? DPI/(Height*25.4/DisplayHeightMM(Disp,Scrn));

	RootWin=RootWindow(Disp,Scrn);

	Visu=DefaultVisual(Disp,Scrn);
	VisuDepth=DefaultDepth(Disp,Scrn);
	if (Visu->c_class!=TrueColor || VisuDepth>32) {
		Visu=NULL;
		viTemplate.screen=Scrn;
		XMutex.Lock();
		viList=XGetVisualInfo(Disp,VisualScreenMask,&viTemplate,&viCount);
		XMutex.Unlock();
		for (i=0, vi=viList; i<viCount; i++, vi++) {
			if (vi->visual->c_class==TrueColor && vi->depth<=32) {
				Visu=vi->visual;
				VisuDepth=vi->depth;
				break;
			}
		}
		XMutex.Lock();
		XFree(viList);
		XMutex.Unlock();
		if (!Visu) {
			emFatalError(
				"No suitable true color visual available on X display \"%s\".",
				displayName
			);
		}
	}
	if      (VisuDepth<= 8) bytesPerPixel=1;
	else if (VisuDepth<=16) bytesPerPixel=2;
	else                    bytesPerPixel=4;

	XMutex.Lock();
	Colmap=XCreateColormap(Disp,RootWin,Visu,AllocNone);
	WM_PROTOCOLS=XInternAtom(Disp,"WM_PROTOCOLS",False);
	WM_DELETE_WINDOW=XInternAtom(Disp,"WM_DELETE_WINDOW",False);
	_NET_WM_ICON=XInternAtom(Disp,"_NET_WM_ICON",False);
	XMutex.Unlock();

	try {
		emX11_TryLoadLibXxf86vm();
	}
	catch (emString errorMessage) {
		emWarning("emX11Screen: %s",errorMessage.Get());
	}
	HaveXF86VidMode=false;
	if (emX11_IsLibXxf86vmLoaded()) {
		XMutex.Lock();
		XSync(Disp,False);
		ErrorHandlerMutex.Lock();
		ErrorHandlerCalled=false;
		originalHandler=XSetErrorHandler(ErrorHandler);
		xb=XF86VidModeQueryVersion(Disp,&major,&minor);
		if (xb && !ErrorHandlerCalled && (major>=1 || (major==0 && minor>=8))) {
			xb=XF86VidModeQueryExtension(Disp,&eventBase,&errorBase);
			if (xb && !ErrorHandlerCalled) {
				memset(&ml,0,sizeof(ml));
				dc=0;
				xb=XF86VidModeGetModeLine(Disp,Scrn,&dc,&ml);
				if (xb && !ErrorHandlerCalled) {
					x=y=0;
					xb=XF86VidModeGetViewPort(Disp,Scrn,&x,&y);
					if (xb && !ErrorHandlerCalled) {
						HaveXF86VidMode=true;
					}
				}
			}
		}
		XSync(Disp,False);
		XSetErrorHandler(originalHandler);
		ErrorHandlerMutex.Unlock();
		XMutex.Unlock();
	}
	if (!HaveXF86VidMode) emWarning("emX11Screen: no XF86VidMode");

	BufWidth=Width;

	// --- Buffer Configuration ---

	BufHeight=150;
		// Optimum depends on CPU cache size, pixel size, window width
		// and the type and complexity of painting. But there is really
		// no necessity for making this user-configurable or
		// auto-adapted. The constant value 150 is a good compromise: It
		// is a little too small for some type of extensive vector
		// painting, and it is a little too large for a CPU with even
		// 4 MB cache when painting high-res images.

	allowTwoBuffers=true;
		// On single processor systems with small CPU cache, this could
		// be set to false for a very little speed-up.

	// --- End of Buffer Configuration ---

	try {
		emX11_TryLoadLibXext();
	}
	catch (emString errorMessage) {
		emWarning("emX11Screen: %s",errorMessage.Get());
	}
	UsingXShm=false;
	XMutex.Lock();
	XSync(Disp,False);
	ErrorHandlerMutex.Lock();
	ErrorHandlerCalled=false;
	originalHandler=XSetErrorHandler(ErrorHandler);
	for (;;) {
		if (!emX11_IsLibXextLoaded()) break;
		if (!XShmQueryVersion(Disp,&major,&minor,&xshmCanDoPixmaps)) break;
		if (ErrorHandlerCalled || major<1 || (minor<1 && major<=1)) break;
		ShmCompletionEventType=XShmGetEventBase(Disp)+ShmCompletion;
		if (ErrorHandlerCalled) break;
		BufImg[0]=XShmCreateImage(
			Disp,
			Visu,
			VisuDepth,
			ZPixmap,
			NULL,
			&BufSeg[0],
			BufWidth,
			BufHeight
		);
		if (ErrorHandlerCalled || !BufImg[0]) break;
		if (
			BufImg[0]->bits_per_pixel!=bytesPerPixel<<3 ||
#			if EM_BYTE_ORDER==4321
				BufImg[0]->byte_order!=MSBFirst
#			elif EM_BYTE_ORDER==1234
				BufImg[0]->byte_order!=LSBFirst
#			else
#				error unexpected value for EM_BYTE_ORDER
#			endif
		) {
			XFree(BufImg[0]);
			break;
		}
		BufSeg[0].shmid=shmget(
			IPC_PRIVATE,
			BufImg[0]->bytes_per_line*BufImg[0]->height,
			IPC_CREAT|0777
		);
		if (BufSeg[0].shmid==-1) {
			XFree(BufImg[0]);
			break;
		}
		BufSeg[0].shmaddr=(char*)shmat(BufSeg[0].shmid,0,0);
		if (BufSeg[0].shmaddr==(char*)-1) {
			shmctl(BufSeg[0].shmid,IPC_RMID,0);
			XFree(BufImg[0]);
			break;
		}
		BufImg[0]->data=BufSeg[0].shmaddr;
		BufSeg[0].readOnly=True;
		status=XShmAttach(Disp,&BufSeg[0]);
		XSync(Disp,False);
		if (!status || ErrorHandlerCalled) {
			shmdt(BufSeg[0].shmaddr);
			shmctl(BufSeg[0].shmid,IPC_RMID,0);
			XFree(BufImg[0]);
			break;
		}
#if defined(__linux__)
		shmctl(BufSeg[0].shmid,IPC_RMID,0);
		BufSegAutoRemoved=true;
#else
		BufSegAutoRemoved=false;
#endif
		BufImg[1]=NULL;
		BufActive[0]=false;
		BufActive[1]=false;
		UsingXShm=true;
		if (!allowTwoBuffers) break;
		BufImg[1]=XShmCreateImage(
			Disp,
			Visu,
			VisuDepth,
			ZPixmap,
			NULL,
			&BufSeg[1],
			BufWidth,
			BufHeight
		);
		if (ErrorHandlerCalled || !BufImg[1]) {
			break;
		}
		BufSeg[1].shmid=shmget(
			IPC_PRIVATE,
			BufImg[1]->bytes_per_line*BufImg[1]->height,
			IPC_CREAT|0777
		);
		if (BufSeg[1].shmid==-1) {
			XFree(BufImg[1]);
			BufImg[1]=NULL;
			break;
		}
		BufSeg[1].shmaddr=(char*)shmat(BufSeg[1].shmid,0,0);
		if (BufSeg[1].shmaddr==(char*)-1) {
			shmctl(BufSeg[1].shmid,IPC_RMID,0);
			XFree(BufImg[1]);
			BufImg[1]=NULL;
			break;
		}
		BufImg[1]->data=BufSeg[1].shmaddr;
		BufSeg[1].readOnly=True;
		status=XShmAttach(Disp,&BufSeg[1]);
		XSync(Disp,False);
		if (!status || ErrorHandlerCalled) {
			shmdt(BufSeg[1].shmaddr);
			shmctl(BufSeg[1].shmid,IPC_RMID,0);
			XFree(BufImg[1]);
			BufImg[1]=NULL;
			break;
		}
		if (BufSegAutoRemoved) shmctl(BufSeg[1].shmid,IPC_RMID,0);
		break;
	}
	XSync(Disp,False);
	XSetErrorHandler(originalHandler);
	ErrorHandlerMutex.Unlock();
	XMutex.Unlock();

	if (!UsingXShm) {
		emWarning("emX11Screen: no XShm (=>slow)");
		XMutex.Lock();
		BufImg[0]=XCreateImage(
			Disp,
			Visu,
			VisuDepth,
			ZPixmap,
			0,
			(char*)malloc(BufHeight*BufWidth*bytesPerPixel),
			BufWidth,
			BufHeight,
			bytesPerPixel<<3,
			BufWidth*bytesPerPixel
		);
		XMutex.Unlock();
		if (
			bytesPerPixel==4 &&
			BufImg[0]->bits_per_pixel==24 &&
			BufImg[0]->bitmap_unit==32 &&
			BufImg[0]->bytes_per_line>=4*BufImg[0]->width
		) {
			//??? hack / workaround: Either there are buggy X-Servers,
			//??? or it is true that it is allowed to have XImages with
			//??? three bytes per pixel - no matter, force four bytes
			//??? per pixel here. Unfortunately, this does not work for
			//??? XShmCreateImage.
			BufImg[0]->bits_per_pixel=32;
		}
		BufImg[1]=NULL;
#		if EM_BYTE_ORDER==4321
			BufImg[0]->byte_order=MSBFirst;
#		elif EM_BYTE_ORDER==1234
			BufImg[0]->byte_order=LSBFirst;
#		else
#			error unexpected value for EM_BYTE_ORDER
#		endif
	}
	else if (allowTwoBuffers && !BufImg[1]) {
		emWarning("emX11Screen: no double buffering via XShm");
	}

	for (i=0; i<2; i++) {
		if (BufImg[i]) {
			memset(
				BufImg[i]->data,
				0,
				BufImg[i]->bytes_per_line*BufImg[i]->height
			);
			BufPainter[i]=emPainter(
				GetRootContext(),
				BufImg[i]->data+BufImg[i]->xoffset*bytesPerPixel,
				BufImg[i]->bytes_per_line,
				bytesPerPixel,
				BufImg[i]->red_mask,
				BufImg[i]->green_mask,
				BufImg[i]->blue_mask,
				0,
				0,
				BufImg[i]->width,
				BufImg[i]->height
			);
		}
	}

	CursorMap.SetTuningLevel(4);

	InputStateClock=0;

	memset(Keymap,0,sizeof(Keymap));

	MouseWarpX=0.0;
	MouseWarpY=0.0;

	LastKnownTime=CurrentTime;

	WinPorts.SetTuningLevel(4);

	GrabbingWinPort=NULL;

	Clipboard=NULL;

	ScreensaverDisableCounter=0;

	SetEnginePriority(emEngine::VERY_HIGH_PRIORITY);

	AddWakeUpSignal(ScreensaverDisableTimer.GetSignal());

	WakeUp();
}


emX11Screen::~emX11Screen()
{
	int i;

	delete WCThread;
	WCThread=NULL;

	XMutex.Lock();

	XSync(Disp,False);

	for (i=0; i<CursorMap.GetCount(); i++) {
		XFreeCursor(Disp,CursorMap[i].XCursor);
	}

	for (i=0; i<2; i++) {
		if (BufImg[i]) {
			if (UsingXShm) {
				XShmDetach(Disp,&BufSeg[i]);
				shmdt(BufSeg[i].shmaddr);
				if (!BufSegAutoRemoved) shmctl(BufSeg[i].shmid,IPC_RMID,0);
			}
			else {
				free(BufImg[i]->data);
			}
			XFree(BufImg[i]);
		}
	}

	XFreeColormap(Disp,Colmap);

	if (InputMethod) XCloseIM(InputMethod);

	XCloseDisplay(Disp);
}


bool emX11Screen::Cycle()
{
	XEvent event;
	Window win;
	int dx,dy,i;
	bool gotAnyWinPortEvent;

	WCThread->SignOfLife();
	if (WCThread->CursorToRestore()) {
		for (i=WinPorts.GetCount()-1; i>=0; i--) {
			WinPorts[i]->RestoreCursor();
		}
	}

	gotAnyWinPortEvent=false;

	XMutex.Lock();
	while (XPending(Disp)) {
		XNextEvent(Disp,&event);
		if (!XFilterEvent(&event,None)) {
			XMutex.Unlock();
			UpdateLastKnownTime(event);
			win=event.xany.window;
			if (Clipboard && win==Clipboard->Win) {
				Clipboard->HandleEvent(event);
			}
			else {
				for (i=WinPorts.GetCount()-1; i>=0; i--) {
					if (WinPorts[i]->Win==win) {
						WinPorts[i]->HandleEvent(event);
						gotAnyWinPortEvent=true;
						break;
					}
				}
			}
			XMutex.Lock();
			if (event.type==ButtonPress || event.type==ButtonRelease) {
				XAllowEvents(Disp,SyncPointer,CurrentTime);
			}
		}
	}
	XMutex.Unlock();

	if (gotAnyWinPortEvent) {
		UpdateKeymapAndInputState();
		for (i=0; i<WinPorts.GetCount();) {
			if (WinPorts[i]->FlushInputState()) {
				i=0; // Because the array may have been modified.
			}
			else {
				i++;
			}
		}
	}

	dx=(int)floor(MouseWarpX+0.5);
	dy=(int)floor(MouseWarpY+0.5);
	if (dx || dy) {
		XMutex.Lock();
		XWarpPointer(Disp,None,None,0,0,0,0,dx,dy);
		XMutex.Unlock();
		MouseWarpX-=dx;
		MouseWarpY-=dy;
	}

	if (
		ScreensaverDisableCounter>0 &&
		IsSignaled(ScreensaverDisableTimer.GetSignal())
	) {
		ScreensaverDisableTimer.Start(59000);
		emDLog("emX11Screen: Touching screensavers.");
		// Against the built-in screensaver of the X server:
		XMutex.Lock();
		XResetScreenSaver(Disp);
		XFlush(Disp);
		XMutex.Unlock();
		// Against xscreensaver:
		if (system("xscreensaver-command -deactivate >&- 2>&- &")==-1) {
			emDLog("Could not run xscreensaver-command: %s",emGetErrorText(errno).Get());
		}
	}

	return true;
}


void emX11Screen::UpdateKeymapAndInputState()
{
	char newKeymap[32];

	memset(newKeymap,0,sizeof(newKeymap));
	XMutex.Lock();
	XQueryKeymap(Disp,newKeymap);
	XMutex.Unlock();
	if (memcmp(Keymap,newKeymap,sizeof(Keymap))!=0) {
		memcpy(Keymap,newKeymap,sizeof(Keymap));
		UpdateInputStateFromKeymap();
	}
}


void emX11Screen::UpdateInputStateFromKeymap()
{
	unsigned char keyStates[32];
	KeySym ks;
	int i,j,k;

	memset(keyStates,0,sizeof(keyStates));
	for (i=0; i<32; i++) {
		if (Keymap[i]) {
			for (j=0; j<8; j++) {
				if ((Keymap[i]&(1<<j))!=0) {
					XMutex.Lock();
					ks=XKeycodeToKeysym(Disp,i*8+j,0);
					XMutex.Unlock();
					k=(int)ConvertKey(ks);
					if (k!=EM_KEY_NONE) keyStates[k>>3]|=1<<(k&7);
				}
			}
		}
	}
	if (InputState.Get(EM_KEY_LEFT_BUTTON)) {
		keyStates[EM_KEY_LEFT_BUTTON>>3]|=1<<(EM_KEY_LEFT_BUTTON&7);
	}
	if (InputState.Get(EM_KEY_MIDDLE_BUTTON)) {
		keyStates[EM_KEY_MIDDLE_BUTTON>>3]|=1<<(EM_KEY_MIDDLE_BUTTON&7);
	}
	if (InputState.Get(EM_KEY_RIGHT_BUTTON)) {
		keyStates[EM_KEY_RIGHT_BUTTON>>3]|=1<<(EM_KEY_RIGHT_BUTTON&7);
	}
	if (memcmp(InputState.GetKeyStates(),keyStates,32)!=0) {
		memcpy(InputState.GetKeyStates(),keyStates,32);
		InputStateClock++;
	}
}


void emX11Screen::UpdateLastKnownTime(const XEvent & event)
{
	Time t;

	switch (event.type) {
		case MotionNotify  : t=event.xmotion.time        ; break;
		case EnterNotify   : t=event.xcrossing.time      ; break;
		case LeaveNotify   : t=event.xcrossing.time      ; break;
		case ButtonPress   : t=event.xbutton.time        ; break;
		case ButtonRelease : t=event.xbutton.time        ; break;
		case KeyPress      : t=event.xkey.time           ; break;
		case KeyRelease    : t=event.xkey.time           ; break;
		case SelectionClear: t=event.xselectionclear.time; break;
		case PropertyNotify: t=event.xproperty.time      ; break;
		default: return;
	}
	if (t!=CurrentTime) LastKnownTime=t;
}


void emX11Screen::WaitBufs()
{
	union {
		XEvent x;
		XShmCompletionEvent xsc;
	} event;

	if (BufActive[0] || BufActive[1]) {
		for (;;) {
			XMutex.Lock();
			XIfEvent(Disp,&event.x,WaitPredicate,(XPointer)this);
			XMutex.Unlock();
			if (event.x.type==ShmCompletionEventType) {
				if (BufActive[0] && event.xsc.shmseg==BufSeg[0].shmseg) {
					BufActive[0]=false;
					break;
				}
				if (BufActive[1] && event.xsc.shmseg==BufSeg[1].shmseg) {
					BufActive[1]=false;
					break;
				}
			}
		}
	}
}


Bool emX11Screen::WaitPredicate(Display * display, XEvent * event, XPointer arg)
{
	emX11Screen * s;
	XShmCompletionEvent * e;

	s=(emX11Screen*)arg;
	if (event->type!=s->ShmCompletionEventType) return false;
	e=(XShmCompletionEvent*)event;
	if (s->BufActive[0] && e->shmseg==s->BufSeg[0].shmseg) return true;
	if (s->BufActive[1] && e->shmseg==s->BufSeg[1].shmseg) return true;
	return false;
}


int emX11Screen::CompareCurMapElemAgainstKey(
	const CursorMapElement * obj, void * key, void * context
)
{
	return obj->CursorId-*((int*)key);
}


::Cursor emX11Screen::GetXCursor(int cursorId)
{
	::Cursor c;
	int idx;

	idx=CursorMap.BinarySearchByKey(&cursorId,CompareCurMapElemAgainstKey);
	if (idx>=0) return CursorMap[idx].XCursor;
	switch (cursorId) {
		default:
		case emCursor::NORMAL:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_left_ptr);
			XMutex.Unlock();
			break;
		case emCursor::INVISIBLE:
			c=CreateXCursor(
				emCursorInvisible_width,
				emCursorInvisible_height,
				emCursorInvisible_bits,
				emCursorInvisible_width,
				emCursorInvisible_height,
				emCursorInvisible_bits,
				emCursorInvisible_x_hot,
				emCursorInvisible_y_hot
			);
			break;
		case emCursor::WAIT:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_watch);
			XMutex.Unlock();
			break;
		case emCursor::CROSSHAIR:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_crosshair);
			XMutex.Unlock();
			break;
		case emCursor::TEXT:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_xterm);
			XMutex.Unlock();
			break;
		case emCursor::HAND:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_hand1);
			XMutex.Unlock();
			break;
		case emCursor::LEFT_RIGHT_ARROW:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_sb_h_double_arrow);
			XMutex.Unlock();
			break;
		case emCursor::UP_DOWN_ARROW:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_sb_v_double_arrow);
			XMutex.Unlock();
			break;
		case emCursor::LEFT_RIGHT_UP_DOWN_ARROW:
			XMutex.Lock();
			c=XCreateFontCursor(Disp,XC_fleur);
			XMutex.Unlock();
			break;
	}
	idx=~idx;
	CursorMap.InsertNew(idx);
	CursorMap.GetWritable(idx).CursorId=cursorId;
	CursorMap.GetWritable(idx).XCursor=c;
	return c;
}


::Cursor emX11Screen::CreateXCursor(
	int srcWidth, int srcHeight, const unsigned char * srcBits,
	int mskWidth, int mskHeight, const unsigned char * mskBits,
	int hotX, int hotY
)
{
	::Cursor c;
	Pixmap src,msk;
	XColor fg, bg;

	fg.red  =0xffff;
	fg.green=0xffff;
	fg.blue =0xffff;
	fg.flags=DoRed|DoGreen|DoBlue;
	bg.red  =0x0000;
	bg.green=0x0000;
	bg.blue =0x0000;
	bg.flags=DoRed|DoGreen|DoBlue;
	XMutex.Lock();
	src=XCreateBitmapFromData(
		Disp,RootWin,(char*)srcBits,srcWidth,srcHeight
	);
	msk=XCreateBitmapFromData(
		Disp,RootWin,(char*)mskBits,mskWidth,mskHeight
	);
	c=XCreatePixmapCursor(Disp,src,msk,&fg,&bg,hotX,hotY);
	XFreePixmap(Disp,src);
	XFreePixmap(Disp,msk);
	XMutex.Unlock();
	return c;
}


emInputKey emX11Screen::ConvertKey(KeySym ks, int * pVariant)
{
	static struct {
		KeySym ks;
		emInputKey key;
		int variant;
	} table[] = {
		{ XK_Shift_L         , EM_KEY_SHIFT       , 0 },
		{ XK_Shift_R         , EM_KEY_SHIFT       , 1 },
		{ XK_Control_L       , EM_KEY_CTRL        , 0 },
		{ XK_Control_R       , EM_KEY_CTRL        , 1 },
		{ XK_Alt_L           , EM_KEY_ALT         , 0 },
		{ XK_Alt_R           , EM_KEY_ALT         , 1 },
		{ XK_Meta_L          , EM_KEY_META        , 0 },
		{ XK_Meta_R          , EM_KEY_META        , 1 },
		{ XK_Super_L         , EM_KEY_META        , 0 },
		{ XK_Super_R         , EM_KEY_META        , 1 },
		{ XK_Hyper_L         , EM_KEY_META        , 0 },
		{ XK_Hyper_R         , EM_KEY_META        , 1 },
		{ XK_ISO_Level3_Shift, EM_KEY_ALT_GR      , 0 },
		{ XK_Up              , EM_KEY_CURSOR_UP   , 0 },
		{ XK_KP_Up           , EM_KEY_CURSOR_UP   , 1 },
		{ XK_Down            , EM_KEY_CURSOR_DOWN , 0 },
		{ XK_KP_Down         , EM_KEY_CURSOR_DOWN , 1 },
		{ XK_KP_Begin        , EM_KEY_CURSOR_DOWN , 1 },
		{ XK_Left            , EM_KEY_CURSOR_LEFT , 0 },
		{ XK_KP_Left         , EM_KEY_CURSOR_LEFT , 1 },
		{ XK_Right           , EM_KEY_CURSOR_RIGHT, 0 },
		{ XK_KP_Right        , EM_KEY_CURSOR_RIGHT, 1 },
		{ XK_Page_Up         , EM_KEY_PAGE_UP     , 0 },
		{ XK_KP_Page_Up      , EM_KEY_PAGE_UP     , 1 },
		{ XK_Page_Down       , EM_KEY_PAGE_DOWN   , 0 },
		{ XK_KP_Page_Down    , EM_KEY_PAGE_DOWN   , 1 },
		{ XK_Home            , EM_KEY_HOME        , 0 },
		{ XK_KP_Home         , EM_KEY_HOME        , 1 },
		{ XK_End             , EM_KEY_END         , 0 },
		{ XK_KP_End          , EM_KEY_END         , 1 },
		{ XK_Print           , EM_KEY_PRINT       , 0 },
		{ XK_Pause           , EM_KEY_PAUSE       , 0 },
		{ XK_Menu            , EM_KEY_MENU        , 0 },
		{ XK_Insert          , EM_KEY_INSERT      , 0 },
		{ XK_KP_Insert       , EM_KEY_INSERT      , 1 },
		{ XK_Delete          , EM_KEY_DELETE      , 0 },
		{ XK_KP_Delete       , EM_KEY_DELETE      , 1 },
		{ XK_BackSpace       , EM_KEY_BACKSPACE   , 0 },
		{ XK_Tab             , EM_KEY_TAB         , 0 },
		{ XK_ISO_Left_Tab    , EM_KEY_TAB         , 0 }, // Comes when Shift pressed
		{ XK_KP_Tab          , EM_KEY_TAB         , 1 },
		{ XK_Return          , EM_KEY_ENTER       , 0 },
		{ XK_KP_Enter        , EM_KEY_ENTER       , 1 },
		{ XK_Escape          , EM_KEY_ESCAPE      , 0 },
		{ XK_space           , EM_KEY_SPACE       , 0 },
		{ XK_KP_Space        , EM_KEY_SPACE       , 1 },
		{ XK_0               , EM_KEY_0           , 0 },
		{ XK_KP_0            , EM_KEY_0           , 1 },
		{ XK_1               , EM_KEY_1           , 0 },
		{ XK_KP_1            , EM_KEY_1           , 1 },
		{ XK_2               , EM_KEY_2           , 0 },
		{ XK_KP_2            , EM_KEY_2           , 1 },
		{ XK_3               , EM_KEY_3           , 0 },
		{ XK_KP_3            , EM_KEY_3           , 1 },
		{ XK_4               , EM_KEY_4           , 0 },
		{ XK_KP_4            , EM_KEY_4           , 1 },
		{ XK_5               , EM_KEY_5           , 0 },
		{ XK_KP_5            , EM_KEY_5           , 1 },
		{ XK_6               , EM_KEY_6           , 0 },
		{ XK_KP_6            , EM_KEY_6           , 1 },
		{ XK_7               , EM_KEY_7           , 0 },
		{ XK_KP_7            , EM_KEY_7           , 1 },
		{ XK_8               , EM_KEY_8           , 0 },
		{ XK_KP_8            , EM_KEY_8           , 1 },
		{ XK_9               , EM_KEY_9           , 0 },
		{ XK_KP_9            , EM_KEY_9           , 1 },
		{ XK_A               , EM_KEY_A           , 0 },
		{ XK_a               , EM_KEY_A           , 0 },
		{ XK_B               , EM_KEY_B           , 0 },
		{ XK_b               , EM_KEY_B           , 0 },
		{ XK_C               , EM_KEY_C           , 0 },
		{ XK_c               , EM_KEY_C           , 0 },
		{ XK_D               , EM_KEY_D           , 0 },
		{ XK_d               , EM_KEY_D           , 0 },
		{ XK_E               , EM_KEY_E           , 0 },
		{ XK_e               , EM_KEY_E           , 0 },
		{ XK_F               , EM_KEY_F           , 0 },
		{ XK_f               , EM_KEY_F           , 0 },
		{ XK_G               , EM_KEY_G           , 0 },
		{ XK_g               , EM_KEY_G           , 0 },
		{ XK_H               , EM_KEY_H           , 0 },
		{ XK_h               , EM_KEY_H           , 0 },
		{ XK_I               , EM_KEY_I           , 0 },
		{ XK_i               , EM_KEY_I           , 0 },
		{ XK_J               , EM_KEY_J           , 0 },
		{ XK_j               , EM_KEY_J           , 0 },
		{ XK_K               , EM_KEY_K           , 0 },
		{ XK_k               , EM_KEY_K           , 0 },
		{ XK_L               , EM_KEY_L           , 0 },
		{ XK_l               , EM_KEY_L           , 0 },
		{ XK_M               , EM_KEY_M           , 0 },
		{ XK_m               , EM_KEY_M           , 0 },
		{ XK_N               , EM_KEY_N           , 0 },
		{ XK_n               , EM_KEY_N           , 0 },
		{ XK_O               , EM_KEY_O           , 0 },
		{ XK_o               , EM_KEY_O           , 0 },
		{ XK_P               , EM_KEY_P           , 0 },
		{ XK_p               , EM_KEY_P           , 0 },
		{ XK_Q               , EM_KEY_Q           , 0 },
		{ XK_q               , EM_KEY_Q           , 0 },
		{ XK_R               , EM_KEY_R           , 0 },
		{ XK_r               , EM_KEY_R           , 0 },
		{ XK_S               , EM_KEY_S           , 0 },
		{ XK_s               , EM_KEY_S           , 0 },
		{ XK_T               , EM_KEY_T           , 0 },
		{ XK_t               , EM_KEY_T           , 0 },
		{ XK_U               , EM_KEY_U           , 0 },
		{ XK_u               , EM_KEY_U           , 0 },
		{ XK_V               , EM_KEY_V           , 0 },
		{ XK_v               , EM_KEY_V           , 0 },
		{ XK_W               , EM_KEY_W           , 0 },
		{ XK_w               , EM_KEY_W           , 0 },
		{ XK_X               , EM_KEY_X           , 0 },
		{ XK_x               , EM_KEY_X           , 0 },
		{ XK_Y               , EM_KEY_Y           , 0 },
		{ XK_y               , EM_KEY_Y           , 0 },
		{ XK_Z               , EM_KEY_Z           , 0 },
		{ XK_z               , EM_KEY_Z           , 0 },
		{ XK_F1              , EM_KEY_F1          , 0 },
		{ XK_KP_F1           , EM_KEY_F1          , 1 },
		{ XK_F2              , EM_KEY_F2          , 0 },
		{ XK_KP_F2           , EM_KEY_F2          , 1 },
		{ XK_F3              , EM_KEY_F3          , 0 },
		{ XK_KP_F3           , EM_KEY_F3          , 1 },
		{ XK_F4              , EM_KEY_F4          , 0 },
		{ XK_KP_F4           , EM_KEY_F4          , 1 },
		{ XK_F5              , EM_KEY_F5          , 0 },
		{ XK_F6              , EM_KEY_F6          , 0 },
		{ XK_F7              , EM_KEY_F7          , 0 },
		{ XK_F8              , EM_KEY_F8          , 0 },
		{ XK_F9              , EM_KEY_F9          , 0 },
		{ XK_F10             , EM_KEY_F10         , 0 },
		{ XK_F11             , EM_KEY_F11         , 0 },
		{ XK_F12             , EM_KEY_F12         , 0 },
		{ XK_KP_Multiply     , EM_KEY_NONE        , 1 },
		{ XK_KP_Add          , EM_KEY_NONE        , 1 },
		{ XK_KP_Separator    , EM_KEY_NONE        , 1 },
		{ XK_KP_Subtract     , EM_KEY_NONE        , 1 },
		{ XK_KP_Decimal      , EM_KEY_NONE        , 1 },
		{ XK_KP_Divide       , EM_KEY_NONE        , 1 },
		{ 0                  , EM_KEY_NONE        , 0 }
	};
	int i;

	for (i=0; ; i++) {
		if (table[i].ks==ks || table[i].ks==0) break;
	}
	if (pVariant) *pVariant=table[i].variant;
	return table[i].key;
}


int emX11Screen::ErrorHandler(Display * display, XErrorEvent * event)
{
	ErrorHandlerCalled=true;
	return 0;
}


emX11Screen::WaitCursorThread::WaitCursorThread(
	emThreadMiniMutex & xMutex, Display * disp
)
	: XMutex(xMutex)
{
	Disp=disp;
	Windows.SetTuningLevel(4);
	Clock=emGetClockMS();
	CursorChanged=false;
	Start(NULL);
}


emX11Screen::WaitCursorThread::~WaitCursorThread()
{
	QuitEvent.Send();
	WaitForTermination();
}


void emX11Screen::WaitCursorThread::AddWindow(::Window win)
{
	DataMutex.Lock();
	Windows.BinaryInsertIfNew(win,emStdComparer<Window>::Compare);
	DataMutex.Unlock();
}


void emX11Screen::WaitCursorThread::RemoveWindow(::Window win)
{
	DataMutex.Lock();
	Windows.BinaryRemove(win,emStdComparer<Window>::Compare);
	DataMutex.Unlock();
}


void emX11Screen::WaitCursorThread::SignOfLife()
{
	DataMutex.Lock();
	Clock=emGetClockMS();
	DataMutex.Unlock();
}


bool emX11Screen::WaitCursorThread::CursorToRestore()
{
	bool b;

	DataMutex.Lock();
	b=CursorChanged;
	CursorChanged=false;
	DataMutex.Unlock();
	return b;
}


int emX11Screen::WaitCursorThread::Run(void * arg)
{
	static const emUInt64 blockTimeMS=125;
	::Cursor cur;
	emUInt64 t;
	int i;

	XMutex.Lock();
	cur=XCreateFontCursor(Disp,XC_watch);
	XMutex.Unlock();

	do {
		DataMutex.Lock();
		t=Clock;
		DataMutex.Unlock();
		t=emGetClockMS()-t;
		if (t<blockTimeMS) {
			t=blockTimeMS-t+1;
		}
		else {
			emDLog("emX11Screen::WaitCursorThread: blocking detected");
			DataMutex.Lock();
			for (i=Windows.GetCount()-1; i>=0; i--) {
				XMutex.Lock();
				XDefineCursor(Disp,Windows[i],cur);
				XMutex.Unlock();
			}
			CursorChanged=true;
			DataMutex.Unlock();
			XMutex.Lock();
			XFlush(Disp);
			XMutex.Unlock();
			t=blockTimeMS;
		}
	}	while (!QuitEvent.Receive(1,t));

	XMutex.Lock();
	XFreeCursor(Disp,cur);
	XMutex.Unlock();

	return 0;
}


emThreadMiniMutex emX11Screen::ErrorHandlerMutex;
bool emX11Screen::ErrorHandlerCalled=false;
