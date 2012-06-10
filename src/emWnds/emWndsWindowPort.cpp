//------------------------------------------------------------------------------
// emWndsWindowPort.cpp
//
// Copyright (C) 2006-2012 Oliver Hamann.
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

#include <emWnds/emWndsWindowPort.h>

#ifndef WM_MOUSEWHEEL
#	define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WM_UNICHAR
#	define WM_UNICHAR 0x0109
#endif


void emWndsWindowPort::WindowFlagsChanged()
{
	HWND oldHWnd;
	int i;

	oldHWnd=HWnd;
	HWnd=NULL;

	SetModalState(false);

	PreConstruct();

	for (i=0; i<Screen.WinPorts.GetCount(); i++) {
		if (
			Screen.WinPorts[i]->Owner==this &&
			Screen.WinPorts[i]->HWnd!=NULL
		) {
			// It is said that SetParent(...) must be used instead of
			// SetWindowLong(...GWL_HWNDPARENT...). But with SetParent the
			// window behaves like WS_CHILD always - even when playing
			// with ShowWindow and SetWindowLong(GWL_STYLE/GWL_EXSTYLE).
			// Another problem is: It does not work if the owner window
			// has WS_EX_TOPMOST (even BringWindowToTop does not help here).
#			if defined(_WIN64)
				SetWindowLongPtr(Screen.WinPorts[i]->HWnd,GWLP_HWNDPARENT,(LONG_PTR)HWnd);
#			else
				SetWindowLong(Screen.WinPorts[i]->HWnd,GWL_HWNDPARENT,(LONG)HWnd);
#			endif
		}
	}

	DestroyWindow(oldHWnd);
	if (BigIcon) { DestroyIcon(BigIcon); BigIcon=NULL; }
	if (SmallIcon) { DestroyIcon(SmallIcon); SmallIcon=NULL; }
}


void emWndsWindowPort::SetPosSize(
	double x, double y, PosSizeArgSpec posSpec,
	double w, double h, PosSizeArgSpec sizeSpec
)
{
	RECT rw,rc;
	POINT pnt;

	if ((GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0) {
		posSpec=PSAS_IGNORE;
		sizeSpec=PSAS_IGNORE;
	}
	if (posSpec==PSAS_IGNORE) {
		x=GetViewX();
		y=GetViewY();
	}
	else {
		if (posSpec==PSAS_WINDOW) {
			pnt.x=0;
			pnt.y=0;
			ClientToScreen(HWnd,&pnt);
			GetWindowRect(HWnd,&rw);
			x+=pnt.x-rw.left;
			y+=pnt.y-rw.top;
		}
		x=floor(x+0.5);
		y=floor(y+0.5);
		PosForced=true;
		PosPending=true;
	}
	if (sizeSpec==PSAS_IGNORE) {
		w=GetViewWidth();
		h=GetViewHeight();
	}
	else {
		if (sizeSpec==PSAS_WINDOW) {
			GetClientRect(HWnd,&rc);
			GetWindowRect(HWnd,&rw);
			w-=rw.right-rw.left-rc.right;
			h-=rw.bottom-rw.top-rc.bottom;
		}
		w=floor(w+0.5);
		h=floor(h+0.5);
		if (w<MinPaneW) w=MinPaneW;
		if (h<MinPaneH) h=MinPaneH;
		SizeForced=true;
		SizePending=true;
	}
	SetViewGeometry(x,y,w,h,Screen.PixelTallness);
	WakeUp();
}


void emWndsWindowPort::GetBorderSizes(
	double * pL, double * pT, double * pR, double * pB
)
{
	RECT rw,rc;
	POINT pnt;

	pnt.x=0;
	pnt.y=0;
	ClientToScreen(HWnd,&pnt);
	GetClientRect(HWnd,&rc);
	GetWindowRect(HWnd,&rw);
	*pL=pnt.x-rw.left;
	*pT=pnt.y-rw.top;
	*pR=rw.right-pnt.x-rc.right;
	*pB=rw.bottom-pnt.y-rc.bottom;
}


void emWndsWindowPort::RequestFocus()
{
	if (!Focused) {
		if (PostConstructed) {
			SetActiveWindow(HWnd);
		}
		Focused=true;
		SetViewFocused(true);
	}
}


void emWndsWindowPort::Raise()
{
	RequestFocus(); // okay?
}


emUInt64 emWndsWindowPort::GetInputClockMS()
{
	return emGetClockMS(); // ???
}


void emWndsWindowPort::InvalidateTitle()
{
	TitlePending=true;
	WakeUp();
}


void emWndsWindowPort::InvalidateIcon()
{
	IconPending=true;
	WakeUp();
}


void emWndsWindowPort::InvalidateCursor()
{
	CursorPending=true;
	WakeUp();
}


void emWndsWindowPort::InvalidatePainting(double x, double y, double w, double h)
{
	double x2,y2;

	x2=x+w;
	if (x2>ClipX2) x2=ClipX2;
	if (x<ClipX1) x=ClipX1;
	if (x>=x2) return;
	y2=y+h;
	if (y2>ClipY2) y2=ClipY2;
	if (y<ClipY1) y=ClipY1;
	if (y>=y2) return;
	InvalidRects.Unite((int)x,(int)y,(int)ceil(x2),(int)ceil(y2));
	if (InvalidRects.GetCount()>64) InvalidRects.SetToMinMax();
	WakeUp();
}


emWndsWindowPort::emWndsWindowPort(emWindow & window)
	: emWindowPort(window),
	emEngine(window.GetScheduler()),
	Screen((emWndsScreen&)window.GetScreen())
{
	emContext * c;
	emWndsWindowPort * wp;
	emWindow * w;

	Owner=NULL;
	for (c=window.GetParentContext(); c; c=c->GetParentContext()) {
		w=dynamic_cast<emWindow*>(c);
		if (!w) continue;
		if (&w->GetScreen()!=(emScreen*)&Screen) break;
		wp=dynamic_cast<emWndsWindowPort*>(&(w->GetWindowPort()));
		if (!wp) continue;
		Owner=wp;
		break;
	}
	HWnd=NULL;
	MinPaneW=1;
	MinPaneH=1;
	PaneX=0;
	PaneY=0;
	PaneW=1;
	PaneH=1;
	ClipX1=PaneX;
	ClipY1=PaneY;
	ClipX2=PaneX+PaneW;
	ClipY2=PaneY+PaneH;
	CursorShown=false;
	PostConstructed=false;
	Mapped=false;
	Focused=false;
	PosForced=false;
	PosPending=false;
	SizeForced=false;
	SizePending=false;
	TitlePending=false;
	IconPending=false;
	CursorPending=false;
	InputStateClock=0;
	LastButtonPress=EM_KEY_NONE;
	LastButtonPressTime=0;
	LastButtonPressX=0.0;
	LastButtonPressY=0.0;
	LastButtonPressRepeat=0;
	RepeatKey=EM_KEY_NONE;
	KeyRepeat=0;
	MouseWheelRemainder=0;
	ModalState=false;
	ModalDescendants=0;
	BigIcon=NULL;
	SmallIcon=NULL;

	Screen.WinPorts.Add(this);

	SetEnginePriority(emEngine::VERY_LOW_PRIORITY);

	PreConstruct();
}


emWndsWindowPort::~emWndsWindowPort()
{
	int i;

	SetModalState(false);
	for (i=Screen.WinPorts.GetCount()-1; i>=0; i--) {
		if (Screen.WinPorts[i]==this) {
			Screen.WinPorts.Remove(i);
			break;
		}
	}
	DestroyWindow(HWnd);
	if (BigIcon) DestroyIcon(BigIcon);
	if (SmallIcon) DestroyIcon(SmallIcon);
}


void emWndsWindowPort::PreConstruct()
{
	double vrx,vry,vrw,vrh,d;
	DWORD exStyleFlags,styleFlags;
	int x,y,w,h;
	bool haveBorder;

	if ((GetWindowFlags()&emWindow::WF_FULLSCREEN)!=0) {
		Screen.LeaveFullscreenModes(&GetWindow());
		Screen.GetVisibleRect(&vrx,&vry,&vrw,&vrh);
		MinPaneW=1;
		MinPaneH=1;
		PaneX=(int)(vrx+0.5);
		PaneY=(int)(vry+0.5);
		PaneW=(int)(vrw+0.5);
		PaneH=(int)(vrh+0.5);
		haveBorder=false;
	}
	else if ((GetWindowFlags()&(emWindow::WF_POPUP|emWindow::WF_UNDECORATED))!=0) {
		Screen.GetVisibleRect(&vrx,&vry,&vrw,&vrh);
		MinPaneW=1;
		MinPaneH=1;
		PaneX=(int)(vrx+vrw*emGetDblRandom(0.22,0.28)+0.5);
		PaneY=(int)(vry+vrh*emGetDblRandom(0.22,0.28)+0.5);
		PaneW=(int)(vrw*0.5+0.5);
		PaneH=(int)(vrh*0.5+0.5);
		haveBorder=false;
	}
	else {
		if (!Owner) Screen.LeaveFullscreenModes(&GetWindow());
		Screen.GetVisibleRect(&vrx,&vry,&vrw,&vrh);
		MinPaneW=32;
		MinPaneH=32;
		if (!Owner && (GetWindowFlags()&emWindow::WF_MODAL)==0) {
			d=emMin(vrw,vrh)*0.08;
			PaneX=(int)(vrx+d*emGetDblRandom(0.5,1.5)+0.5);
			PaneY=(int)(vry+d*emGetDblRandom(0.8,1.2)+0.5);
			PaneW=(int)(vrw-d*2.0+0.5);
			PaneH=(int)(vrh-d*2.0+0.5);
		}
		else {
			PaneX=(int)(vrx+vrw*emGetDblRandom(0.22,0.28)+0.5);
			PaneY=(int)(vry+vrh*emGetDblRandom(0.22,0.28)+0.5);
			PaneW=(int)(vrw*0.5+0.5);
			PaneH=(int)(vrh*0.5+0.5);
		}
		haveBorder=true;
	}

	ClipX1=PaneX;
	ClipY1=PaneY;
	ClipX2=PaneX+PaneW;
	ClipY2=PaneY+PaneH;
	PosForced=false;
	PosPending=false;
	SizeForced=false;
	SizePending=false;
	InvalidRects.Set(PaneX,PaneY,PaneX+PaneW,PaneY+PaneH);
	Title.Empty();
	TitlePending=true;
	IconPending=true;
	Cursor=-1;
	CursorPending=true;
	CursorShown=false;
	PostConstructed=false;
	Mapped=false;
	Focused=true;
	InputStateClock=0;
	LastButtonPress=EM_KEY_NONE;
	RepeatKey=EM_KEY_NONE;
	MouseWheelRemainder=0;

	if (haveBorder) {
		if (Owner) {
			exStyleFlags=0;
			styleFlags=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX;
			x=CW_USEDEFAULT;
			y=CW_USEDEFAULT;
			w=PaneW;
			h=PaneH;
		}
		else {
			exStyleFlags=0;
			styleFlags=WS_OVERLAPPEDWINDOW;
			x=CW_USEDEFAULT;
			y=CW_USEDEFAULT;
			w=CW_USEDEFAULT;
			h=CW_USEDEFAULT;
		}
	}
	else {
		if (Owner) {
			// WS_EX_TOOLWINDOW prevents from getting a taskbar entry
			exStyleFlags=WS_EX_TOPMOST|WS_EX_TOOLWINDOW;
			styleFlags=WS_POPUP;
			x=PaneX;
			y=PaneY;
			w=PaneW;
			h=PaneH;
		}
		else {
			exStyleFlags=0;
			styleFlags=WS_POPUP;
			x=PaneX;
			y=PaneY;
			w=PaneW;
			h=PaneH;
		}
	}

	HWnd=CreateWindowEx(
		exStyleFlags,
		Screen.WinClassName.Get(),
		Title.Get(),
		styleFlags,
		x,
		y,
		w,
		h,
		Owner ? Owner->HWnd : NULL,
		NULL,
		NULL,
		NULL
	);
	if (!HWnd) {
		emFatalError(
			"emWndsWindowPort: CreateWindow failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

	if (ModalDescendants>0) EnableWindow(HWnd,FALSE);

	SetViewFocused(Focused);
	SetViewGeometry(PaneX,PaneY,PaneW,PaneH,Screen.PixelTallness);

	WakeUp();
}


void emWndsWindowPort::PostConstruct()
{
	ShowWindow(HWnd,SW_SHOWNORMAL);

	if ((GetWindowFlags()&emWindow::WF_MODAL)!=0) {
		SetModalState(true);
	}
}


LRESULT emWndsWindowPort::WindowProc(
	UINT uMsg, WPARAM wParam, LPARAM lParam, bool goodCall
)
{
	emInputEvent inputEvent;
	PAINTSTRUCT ps;
	RECT rw,rc;
	emWndsWindowPort * wp, * wp2;
	HWND hwnd;
	POINT pnt;
	POINT * pPnt;
	HDC hdc;
	emInputKey key;
	char tmp[256];
	double mx,my;
	emUInt64 clk;
	int i,x,y,w,h,mask,repeat,variant;
	bool dblClick,doIt;

	// Remember:
	// - Calling InputToView may delete this window port.
	// - Several Windows API functions are calling the WindowProc
	//   synchronously.

	switch (uMsg) {
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		mx=PaneX+((short)LOWORD(lParam))+Screen.MouseWarpX;
		my=PaneY+((short)HIWORD(lParam))+Screen.MouseWarpY;
		if (
			Screen.InputState.GetMouseX()!=mx ||
			Screen.InputState.GetMouseY()!=my
		) {
			Screen.InputState.SetMouse(mx,my);
			Screen.InputStateClock++;
		}
		switch (uMsg) {
		case WM_LBUTTONDOWN  : key=EM_KEY_LEFT_BUTTON  ; dblClick=false; break;
		case WM_MBUTTONDOWN  : key=EM_KEY_MIDDLE_BUTTON; dblClick=false; break;
		case WM_RBUTTONDOWN  : key=EM_KEY_RIGHT_BUTTON ; dblClick=false; break;
		case WM_LBUTTONDBLCLK: key=EM_KEY_LEFT_BUTTON  ; dblClick=true ; break;
		case WM_MBUTTONDBLCLK: key=EM_KEY_MIDDLE_BUTTON; dblClick=true ; break;
		case WM_RBUTTONDBLCLK: key=EM_KEY_RIGHT_BUTTON ; dblClick=true ; break;
		default              : key=EM_KEY_NONE         ; dblClick=false; break;
		}
		if (key==EM_KEY_NONE) {
			for (i=0; i<3; i++) {
				if      (i==0) { key=EM_KEY_LEFT_BUTTON  ; mask=MK_LBUTTON; }
				else if (i==1) { key=EM_KEY_MIDDLE_BUTTON; mask=MK_MBUTTON; }
				else           { key=EM_KEY_RIGHT_BUTTON ; mask=MK_RBUTTON; }
				if (Screen.InputState.Get(key) && (wParam&mask)==0) {
					Screen.InputState.Set(key,false);
					Screen.InputStateClock++;
				}
			}
			return 0;
		}
		for (i=Screen.WinPorts.GetCount()-1; i>=0; i--) {
			wp=Screen.WinPorts[i];
			if (
				(wp->GetWindowFlags()&emWindow::WF_POPUP)!=0 &&
				wp!=this && !wp->IsAncestorOf(this)
			) {
				wp->SignalWindowClosing();
			}
		}
		if (Screen.InputState.Get(key)) return 0;
		Screen.InputState.Set(key,true);
		Screen.InputStateClock++;
		if (!GetCapture()) SetCapture(HWnd);
		if (!goodCall) return 0;
		if (!Focused) {
			RequestFocus();
			Screen.UpdateKeymapAndInputState();
		}
		clk=emGetClockMS();
		if (dblClick) {
			if (
				(LastButtonPressRepeat&1)==0 &&
				key==LastButtonPress
			) {
				repeat=LastButtonPressRepeat+1;
			}
			else {
				repeat=0;
			}
		}
		else {
			if (
				(LastButtonPressRepeat&1)!=0 &&
				key==LastButtonPress &&
				clk>LastButtonPressTime &&
				clk-LastButtonPressTime<=330 &&
				fabs(mx-LastButtonPressX)<=10 &&
				fabs(my-LastButtonPressY)<=10
			) {
				repeat=LastButtonPressRepeat+1;
			}
			else {
				repeat=0;
			}
		}
		LastButtonPress=key;
		LastButtonPressTime=clk;
		LastButtonPressX=mx;
		LastButtonPressY=my;
		LastButtonPressRepeat=repeat;
		inputEvent.Setup(key,"",repeat,0);
		InputStateClock=Screen.InputStateClock;
		InputToView(inputEvent,Screen.InputState);
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		mx=PaneX+((short)LOWORD(lParam))+Screen.MouseWarpX;
		my=PaneY+((short)HIWORD(lParam))+Screen.MouseWarpY;
		if (
			Screen.InputState.GetMouseX()!=mx ||
			Screen.InputState.GetMouseY()!=my
		) {
			Screen.InputState.SetMouse(mx,my);
			Screen.InputStateClock++;
		}
		if (uMsg==WM_LBUTTONUP)      key=EM_KEY_LEFT_BUTTON;
		else if (uMsg==WM_MBUTTONUP) key=EM_KEY_MIDDLE_BUTTON;
		else                         key=EM_KEY_RIGHT_BUTTON;
		if (Screen.InputState.Get(key)) {
			Screen.InputState.Set(key,false);
			Screen.InputStateClock++;
			doIt=true;
		}
		else {
			doIt=false;
		}
		if (
			GetCapture()==HWnd &&
			!Screen.InputState.Get(EM_KEY_LEFT_BUTTON) &&
			!Screen.InputState.Get(EM_KEY_MIDDLE_BUTTON) &&
			!Screen.InputState.Get(EM_KEY_RIGHT_BUTTON)
		) {
			ReleaseCapture();
		}
		if (doIt && goodCall) {
			inputEvent.Eat();
			InputStateClock=Screen.InputStateClock;
			InputToView(inputEvent,Screen.InputState);
			return 0;
		}
		return 0;
	case WM_MOUSEWHEEL:
		mx=((short)LOWORD(lParam))+Screen.MouseWarpX;
		my=((short)HIWORD(lParam))+Screen.MouseWarpY;
		if (
			Screen.InputState.GetMouseX()!=mx ||
			Screen.InputState.GetMouseY()!=my
		) {
			Screen.InputState.SetMouse(mx,my);
			Screen.InputStateClock++;
		}
		pnt.x=(short)LOWORD(lParam);
		pnt.y=(short)HIWORD(lParam);
		hwnd=WindowFromPoint(pnt);
		wp=NULL;
		for (i=Screen.WinPorts.GetCount()-1; i>=0; i--) {
			if (Screen.WinPorts[i]->HWnd==hwnd) {
				wp=Screen.WinPorts[i];
				break;
			}
		}
		for (i=Screen.WinPorts.GetCount()-1; i>=0; i--) {
			wp2=Screen.WinPorts[i];
			if (
				(wp2->GetWindowFlags()&emWindow::WF_POPUP)!=0 &&
				wp2->HWnd && wp2!=wp && (!wp || !wp2->IsAncestorOf(wp))
			) {
				wp2->SignalWindowClosing();
			}
		}
		if (wp==this) {
			if (goodCall) {
				if (!Focused) {
					RequestFocus();
					Screen.UpdateKeymapAndInputState();
				}
				if (mx>=PaneX && mx<PaneX+PaneW && my>=PaneY && my<PaneY+PaneH) {
					MouseWheelRemainder+=(short)HIWORD(wParam);
					y=MouseWheelRemainder/120;
					MouseWheelRemainder-=y*120;
					if (y!=0) {
						if (y>0) key=EM_KEY_WHEEL_UP;
						else key=EM_KEY_WHEEL_DOWN;
						inputEvent.Setup(key,"",0,0);
						InputStateClock=Screen.InputStateClock;
						InputToView(inputEvent,Screen.InputState);
						return 0;
					}
				}
			}
		}
		else {
			// Here we have a mouse wheel event while the mouse is in
			// another window. And we transfer the focus and the mouse
			// wheel event to that window, even if it is a foreign
			// application. This is highly incompatible to the normal
			// mouse wheel behavior in Windows.
			if (IsWindowEnabled(hwnd)) {
				if (wp) SetActiveWindow(hwnd);
				else SetForegroundWindow(hwnd);
				PostMessage(hwnd,uMsg,wParam,lParam);
			}
		}
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam<256 && (Screen.Keymap[wParam]&0x80)==0) {
			Screen.Keymap[wParam]|=0x80;
			Screen.UpdateInputStateFromKeymap();
		}
		key=emWndsScreen::ConvertKey(wParam,&variant);
		if (key!=EM_KEY_NONE && goodCall) {
			repeat=0;
			if (Screen.InputState.Get(key) && RepeatKey==key) {
				repeat=KeyRepeat+1;
			}
			RepeatKey=key;
			KeyRepeat=repeat;
			inputEvent.Setup(key,"",repeat,variant);
			InputStateClock=Screen.InputStateClock;
			InputToView(inputEvent,Screen.InputState);
			return 0;
		}
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		RepeatKey=EM_KEY_NONE;
		if (wParam<256 && (Screen.Keymap[wParam]&0x80)!=0) {
			Screen.Keymap[wParam]&=(BYTE)~0x80;
			Screen.UpdateInputStateFromKeymap();
		}
		return 0;
	case WM_CHAR:
		//??? Would be nice if this could be merged with the key event.
		if (wParam>=32 && wParam<255 && wParam!=127) {
			if (goodCall) {
				repeat=0; //???
				tmp[0]=(char)wParam;
				tmp[1]=0;
				inputEvent.Setup(EM_KEY_NONE,tmp,repeat,0);
				InputStateClock=Screen.InputStateClock;
				InputToView(inputEvent,Screen.InputState);
				return 0;
			}
		}
		return 0;
	case WM_SYSCHAR:
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	case WM_UNICHAR:
		return 0;
	case WM_PAINT:
		hdc=BeginPaint(HWnd,&ps);
		if (hdc) {
			x=ps.rcPaint.left;
			y=ps.rcPaint.top;
			w=ps.rcPaint.right-ps.rcPaint.left;
			h=ps.rcPaint.bottom-ps.rcPaint.top;
			InvalidatePainting(PaneX+x,PaneY+y,w,h);
			EndPaint(HWnd,&ps);
		}
		return 0;
	case WM_SETFOCUS:
		if (!GetCapture()) {
			if (
				Screen.InputState.Get(EM_KEY_LEFT_BUTTON) ||
				Screen.InputState.Get(EM_KEY_MIDDLE_BUTTON) ||
				Screen.InputState.Get(EM_KEY_RIGHT_BUTTON)
			) {
				SetCapture(HWnd);
			}
		}
		Screen.UpdateKeymapAndInputState();
		RepeatKey=EM_KEY_NONE;
		MouseWheelRemainder=0;
		if (!Focused) {
			Focused=true;
			SetViewFocused(true);
		}
		return 0;
	case WM_KILLFOCUS:
		if (GetCapture()==HWnd) ReleaseCapture();
		if (Focused) {
			Focused=false;
			SetViewFocused(false);
			if ((GetWindowFlags()&emWindow::WF_POPUP)!=0) SignalWindowClosing();
		}
		LastButtonPress=EM_KEY_NONE;
		RepeatKey=EM_KEY_NONE;
		MouseWheelRemainder=0;
		return 0;
	case WM_SETCURSOR:
		if ((HWND)wParam!=HWnd || LOWORD(lParam)!=1) {
			CursorShown=false;
			return DefWindowProc(HWnd,uMsg,wParam,lParam);
		}
		CursorShown=true;
		SetCursor(Screen.GetCursorHandle(Cursor));
		return TRUE;
	case WM_WINDOWPOSCHANGED:
		GetClientRect(HWnd,&rc);
		pnt.x=0;
		pnt.y=0;
		ClientToScreen(HWnd,&pnt);
		x=pnt.x;
		y=pnt.y;
		w=rc.right-rc.left;
		h=rc.bottom-rc.top;
		if (w<MinPaneW) w=MinPaneW;
		if (h<MinPaneH) h=MinPaneH;
		if (PaneX!=x || PaneY!=y || PaneW!=w || PaneH!=h) {
			PaneX=x;
			PaneY=y;
			PaneW=w;
			PaneH=h;
			ClipX1=PaneX;
			ClipY1=PaneY;
			ClipX2=PaneX+PaneW;
			ClipY2=PaneY+PaneH;
			InvalidRects.Set(PaneX,PaneY,PaneX+PaneW,PaneY+PaneH);
			WakeUp();
			if (!PosPending && !SizePending) {
				SetViewGeometry(
					PaneX,PaneY,
					PaneW,PaneH,
					Screen.PixelTallness
				);
			}
			else if (!PosPending) {
				SetViewGeometry(
					PaneX,PaneY,
					GetViewWidth(),GetViewHeight(),
					Screen.PixelTallness
				);
			}
			else if (!SizePending) {
				SetViewGeometry(
					GetViewX(),GetViewY(),
					PaneW,PaneH,
					Screen.PixelTallness
				);
			}
			Screen.InputStateClock++;
		}
		return 0;
	case WM_GETMINMAXINFO:
		DefWindowProc(HWnd,uMsg,wParam,lParam);
		GetClientRect(HWnd,&rc);
		GetWindowRect(HWnd,&rw);
		w=MinPaneW+rw.right-rw.left-rc.right+rc.left;
		h=MinPaneH+rw.bottom-rw.top-rc.bottom+rc.top;
		pPnt=&((MINMAXINFO*)lParam)->ptMinTrackSize;
		if (pPnt->x<w) pPnt->x=w;
		if (pPnt->y<h) pPnt->y=h;
		return 0;
	case WM_SHOWWINDOW:
		if (wParam) {
			if (!Mapped) {
				Mapped=true;
				WakeUp();
			}
		}
		else {
			if (Mapped) {
				Mapped=false;
			}
		}
		return DefWindowProc(HWnd,uMsg,wParam,lParam);
	case WM_CLOSE:
		SignalWindowClosing();
		return 0;
	default:
		return DefWindowProc(HWnd,uMsg,wParam,lParam);
	}
}


bool emWndsWindowPort::FlushInputState()
{
	if (Focused && InputStateClock!=Screen.InputStateClock) {
		InputStateClock=Screen.InputStateClock;
		emInputEvent inputEvent;
		InputToView(inputEvent,Screen.InputState);
		return true;
	}
	else {
		return false;
	}
}


void emWndsWindowPort::RestoreCursor()
{
	if (CursorShown) SetCursor(Screen.GetCursorHandle(Cursor));
}


bool emWndsWindowPort::Cycle()
{
	emString str;
	emCursor cur;
	RECT rw,rc;
	POINT pnt;
	int x,y,w,h;
	UINT flags;

	if (PosPending || SizePending) {
		x=(int)GetViewX();
		y=(int)GetViewY();
		w=(int)GetViewWidth();
		h=(int)GetViewHeight();
		GetWindowRect(HWnd,&rw);
		GetClientRect(HWnd,&rc);
		pnt.x=rw.left;
		pnt.y=rw.top;
		ScreenToClient(HWnd,&pnt);
		x+=pnt.x;
		y+=pnt.y;
		w+=(rw.right-rw.left)-(rc.right-rc.left);
		h+=(rw.bottom-rw.top)-(rc.bottom-rc.top);
		flags=SWP_NOCOPYBITS|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER;
		if (!PosForced) flags|=SWP_NOMOVE;
		if (!SizeForced) flags|=SWP_NOSIZE;
		SetWindowPos(
			HWnd,
			NULL,
			x,y,w,h,flags
		);
		PosPending=false;
		SizePending=false;
	}

	if (TitlePending) {
		str=GetWindowTitle();
		if (Title!=str) {
			Title=str;
			SetWindowText(HWnd,Title.Get());
		}
		TitlePending=false;
	}

	if (IconPending) {
		SetIconProperty(GetWindowIcon());
		TitlePending=false;
	}

	if (CursorPending) {
		cur=GetViewCursor();
		if (Cursor!=cur) {
			Cursor=cur;
			if (CursorShown) SetCursor(Screen.GetCursorHandle(Cursor));
		}
		CursorPending=false;
	}

	if (!PostConstructed) {
		PostConstruct();
		PostConstructed=true;
	}

	if (!InvalidRects.IsEmpty() && Mapped) UpdatePainting();

	return false;
}


void emWndsWindowPort::UpdatePainting()
{
	const emClipRects<int>::Rect * r;
	HDC hdc;
	int bufIdx,rx1,ry1,rx2,ry2,x,y,w,h;

	if (InvalidRects.IsEmpty()) return;
	InvalidRects.Sort();
	hdc=GetDC(HWnd);
	bufIdx=0;
	for (r=InvalidRects.GetFirst(); r; r=r->GetNext()) {
		rx1=r->GetX1();
		ry1=r->GetY1();
		rx2=r->GetX2();
		ry2=r->GetY2();
		y=ry1;
		do {
			h=ry2-y;
			if (h>Screen.BufHeight) h=Screen.BufHeight;
			x=rx1;
			do {
				w=rx2-x;
				if (w>Screen.BufWidth) w=Screen.BufWidth;
				PaintView(emPainter(Screen.BufPainter[bufIdx],0,0,w,h,-x,-y,1,1),0);
				Screen.BeginSendBuf(hdc,bufIdx,x-PaneX,y-PaneY,w,h);
				bufIdx^=1;
				x+=w;
			} while (x<rx2);
			y+=h;
		} while (y<ry2);
	}
	Screen.WaitSendBuf();
	InvalidRects.Empty();
}


bool emWndsWindowPort::IsAncestorOf(emWndsWindowPort * wp)
{
	while (wp) {
		wp=wp->Owner;
		if (wp==this) return true;
	}
	return false;
}


void emWndsWindowPort::SetModalState(bool modalState)
{
	emWndsWindowPort * wp;

	if (ModalState!=modalState) {
		for (wp=Owner; wp; wp=wp->Owner) {
			if (modalState) {
				if (wp->ModalDescendants==0) EnableWindow(wp->HWnd,FALSE);
				wp->ModalDescendants++;
			}
			else {
				wp->ModalDescendants--;
				if (wp->ModalDescendants==0) EnableWindow(wp->HWnd,TRUE);
			}
		}
		ModalState=modalState;
	}
}


void emWndsWindowPort::SetIconProperty(const emImage & icon)
{
	HICON hIcon;

	if (icon.IsEmpty()) return;

	hIcon=Image2Icon(
		icon,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON)
	);
	if (hIcon) {
		SendMessage(HWnd,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)hIcon);
		if (BigIcon) DestroyIcon(BigIcon);
		BigIcon=hIcon;
	}

	hIcon=Image2Icon(
		icon,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON)
	);
	if (hIcon) {
		SendMessage(HWnd,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)hIcon);
		if (SmallIcon) DestroyIcon(SmallIcon);
		SmallIcon=hIcon;
	}
}


HICON emWndsWindowPort::Image2Icon(const emImage & image, int width, int height)
{
	emImage transformedImage;
	BITMAPV4HEADER bi;
	ICONINFO iconInfo;
	HBITMAP hBitmap,hMonoBitmap;
	HICON hIcon;
	HDC hdc;
	emUInt32 * data, * t, * e;
	const emByte * s;

	//??? Windows 2000 ignores the alpha channel.

	if (image.IsEmpty()) return NULL;

	transformedImage.Setup(width,height,4);
	transformedImage.CopyTransformed(
		0,0,width,height,
		emScaleATM(
			((double)width)/image.GetWidth(),
			((double)height)/image.GetHeight()
		),
		image,
		true
	);

	memset(&bi,0,sizeof(bi));
	bi.bV4Size       =sizeof(bi);
	bi.bV4Width      =width;
	bi.bV4Height     =-height;
	bi.bV4Planes     =1;
	bi.bV4BitCount   =32;
	bi.bV4V4Compression=BI_BITFIELDS;
	bi.bV4RedMask    =0x00FF0000;
	bi.bV4GreenMask  =0x0000FF00;
	bi.bV4BlueMask   =0x000000FF;
	bi.bV4AlphaMask  =0xFF000000;

	hdc=GetDC(NULL);
	data=NULL;
	hBitmap=CreateDIBSection(
		hdc,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS,
		(void**)(void*)&data,
		NULL,
		0
	);
	ReleaseDC(NULL,hdc);

	if (!hBitmap) {
		emWarning("emWndsWindowPort::Image2Icon: CreateDIBSection failed.");
		return NULL;
	}

	s=transformedImage.GetMap();
	t=data;
	e=data+width*height;
	while (t<e) {
		*t=
			(((emUInt32)s[0])<<16) |
			(((emUInt32)s[1])<<8)  |
			s[2]                   |
			(((emUInt32)s[3])<<24)
		;
		t++;
		s+=4;
	}

	hMonoBitmap=CreateBitmap(width,height,1,1,NULL);
	if (!hMonoBitmap) {
		emWarning("emWndsWindowPort::Image2Icon: CreateBitmap failed.");
		DeleteObject(hBitmap);
		return NULL;
	}

	iconInfo.fIcon   =TRUE;
	iconInfo.xHotspot=0;
	iconInfo.yHotspot=0;
	iconInfo.hbmMask =hMonoBitmap;
	iconInfo.hbmColor=hBitmap;
	hIcon=CreateIconIndirect(&iconInfo);

	DeleteObject(hBitmap);
	DeleteObject(hMonoBitmap);

	if (!hIcon) {
		emWarning("emWndsWindowPort::Image2Icon: CreateIconIndirect failed.");
		return NULL;
	}

	return hIcon;
}
