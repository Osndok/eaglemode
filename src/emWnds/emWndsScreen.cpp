//------------------------------------------------------------------------------
// emWndsScreen.cpp
//
// Copyright (C) 2006-2011,2015-2017 Oliver Hamann.
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

#include <emWnds/emWndsScreen.h>
#include <emWnds/emWndsScheduler.h>
#include <emWnds/emWndsViewRenderer.h>
#include <emWnds/emWndsWindowPort.h>

#ifndef IDC_HAND
#	define IDC_HAND MAKEINTRESOURCE(32649)
#endif

#ifndef MONITORINFOF_PRIMARY
#	define MONITORINFOF_PRIMARY 1
#endif


void emWndsScreen::Install(emContext & context)
{
	emWndsScreen * m;
	emString name;

	m=(emWndsScreen*)context.Lookup(typeid(emWndsScreen),name);
	if (!m) {
		m=new emWndsScreen(context,name);
		m->Register();
	}
	m->emScreen::Install();
}


void emWndsScreen::GetDesktopRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	if (pX) *pX=DesktopRect.left;
	if (pY) *pY=DesktopRect.top;
	if (pW) *pW=DesktopRect.right-DesktopRect.left;
	if (pH) *pH=DesktopRect.bottom-DesktopRect.top;
}


int emWndsScreen::GetMonitorCount() const
{
	return MonitorRects.GetCount();
}


void emWndsScreen::GetMonitorRect(
	int index, double * pX, double * pY, double * pW, double * pH
) const
{
	const RECT * rect;

	if (index<0 || index>=MonitorRects.GetCount()) {
		if (pX) *pX=0.0;
		if (pY) *pY=0.0;
		if (pW) *pW=0.0;
		if (pH) *pH=0.0;
	}
	else {
		rect=&MonitorRects[index];
		if (pX) *pX=rect->left;
		if (pY) *pY=rect->top;
		if (pW) *pW=rect->right-rect->left;
		if (pH) *pH=rect->bottom-rect->top;
	}
}


double emWndsScreen::GetDPI() const
{
	return DPI;
}


void emWndsScreen::MoveMousePointer(double dx, double dy)
{
	MouseWarpX+=dx;
	MouseWarpY+=dy;
}


void emWndsScreen::Beep()
{
	if (!::Beep(700,80)) ::MessageBeep((UINT)-1);
}


emWindowPort * emWndsScreen::CreateWindowPort(emWindow & window)
{
	return new emWndsWindowPort(window);
}


emWndsScreen::emWndsScreen(emContext & context, const emString & name)
	: emScreen(context,name)
{
	WNDCLASS wc;
	int i;

	InstanceListMutex.Lock();
	NextInstance=InstanceList;
	InstanceList=this;
	InstanceListMutex.Unlock();

	WindowProcRecursion=0;

	WndsScheduler=dynamic_cast<emWndsScheduler*>(&GetScheduler());
	if (!WndsScheduler) {
		emFatalError("emWndsScreen: Scheduler must be an emWndsScheduler.");
	}

	WCThread=new WaitCursorThread();

	i=0;
	do {
		i++;
		WinClassName     =emString::Format("emWnds%d",i);
		wc.style         =CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
		wc.lpfnWndProc   =(WNDPROC)WindowProc;
		wc.cbClsExtra    =0;
		wc.cbWndExtra    =0;
		wc.hInstance     =NULL;
		wc.hIcon         =NULL;
		wc.hCursor       =LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground =NULL;
		wc.lpszMenuName  =NULL;
		wc.lpszClassName =WinClassName.Get();
	} while (!RegisterClass(&wc));

	memset(&DesktopRect,0,sizeof(DesktopRect));
	DPI=1.0;
	PixelTallness=1.0;
	UpdateGeometry();
	GeometryUpdateTime=emGetClockMS();

	CursorMap.SetTuningLevel(4);

	InputStateToBeFlushed=false;

	InputStateClock=0;

	memset(Keymap,0,sizeof(Keymap));

	MouseWarpX=0.0;
	MouseWarpY=0.0;

	WinPorts.SetTuningLevel(4);

	ViewRenderer = new emWndsViewRenderer(*this);

	SetEnginePriority(emEngine::VERY_HIGH_PRIORITY);

	WakeUp();
}


emWndsScreen::~emWndsScreen()
{
	emWndsScreen * * pp;

	delete WCThread;
	WCThread=NULL;

	delete ViewRenderer;
	ViewRenderer=NULL;

	UnregisterClass(WinClassName.Get(),NULL);
	InstanceListMutex.Lock();
	for (pp=&InstanceList; *pp!=this; pp=&(*pp)->NextInstance);
	*pp=NextInstance;
	InstanceListMutex.Unlock();
}


bool emWndsScreen::Cycle()
{
	POINT pnt;
	emUInt64 clk;
	int i,dx,dy;

	WCThread->SignOfLife();
	if (WCThread->CursorToRestore()) {
		for (i=WinPorts.GetCount()-1; i>=0; i--) {
			WinPorts[i]->RestoreCursor();
		}
	}

	clk=emGetClockMS();
	if (clk-GeometryUpdateTime>1500) {
		UpdateGeometry();
		GeometryUpdateTime=clk;
	}

	if (InputStateToBeFlushed) {
		UpdateKeymapAndInputState();
		InputStateToBeFlushed=false;
		for (i=0;;) {
			if (i>=WinPorts.GetCount()) break;
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
		if (GetCursorPos(&pnt)) SetCursorPos(pnt.x+dx,pnt.y+dy);
		MouseWarpX-=dx;
		MouseWarpY-=dy;
	}

	return true;
}


LRESULT CALLBACK emWndsScreen::WindowProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
)
{
	emWndsScreen * s;
	LRESULT res;
	int i;

	InstanceListMutex.Lock();
	for (s=InstanceList, i=-1; s; s=s->NextInstance) {
		for (i=s->WinPorts.GetCount()-1; i>=0; i--) {
			if (s->WinPorts[i]->HWnd==hWnd) break;
		}
		if (i>=0) break;
	}
	InstanceListMutex.Unlock();

	if (s && i>=0) {
		s->InputStateToBeFlushed=true;
		s->WindowProcRecursion++;
		res=s->WinPorts[i]->WindowProc(
			uMsg,
			wParam,
			lParam,
			s->WindowProcRecursion==1 &&
			s->WndsScheduler->GetState() ==
				emWndsScheduler::HANDLING_MESSAGES
		);
		s->WindowProcRecursion--;
	}
	else {
		res=DefWindowProc(hWnd,uMsg,wParam,lParam);
	}

	return res;
}


void emWndsScreen::UpdateGeometry()
{
	emArray<RECT> oldRects;
	RECT oldRect;
	const RECT * rp;
	bool anyChange;
	int i;

	anyChange=false;

	oldRect=DesktopRect;
	DesktopRect.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
	DesktopRect.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
	DesktopRect.right  = DesktopRect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	DesktopRect.bottom = DesktopRect.top  + GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (memcmp(&DesktopRect,&oldRect,sizeof(RECT))!=0) {
		anyChange=true;
	}

	oldRects=MonitorRects;
	MonitorRects.Clear();
	EnumDisplayMonitors(NULL,NULL,MonitorEnumProc,(LPARAM)this);
	if (MonitorRects.IsEmpty()) {
		MonitorRects.Add(DesktopRect);
	}
	if (MonitorRects.GetCount()!=oldRects.GetCount()) {
		anyChange=true;
	}
	if (!anyChange) {
		for (i=0; i<MonitorRects.GetCount(); i++) {
			if (memcmp(&MonitorRects[i],&oldRects[i],sizeof(RECT))!=0) {
				anyChange=true;
				break;
			}
		}
	}

	DPI=75.0; //???
	PixelTallness=1.0; //???

	if (anyChange) {
		SignalGeometrySignal();

		rp=&DesktopRect;
		emDLog(
			"emWndsScreen::UpdateGeometry: Desktop: x=%ld y=%ld w=%ld h=%ld",
			(long)rp->left, (long)rp->top, (long)(rp->right-rp->left),
			(long)(rp->bottom-rp->top)
		);
		for (i=0; i<MonitorRects.GetCount(); i++) {
			rp=&MonitorRects[i];
			emDLog(
				"emWndsScreen::UpdateGeometry: Monitor %d: x=%ld y=%ld w=%ld h=%ld",
				i, (long)rp->left, (long)rp->top, (long)(rp->right-rp->left),
				(long)(rp->bottom-rp->top)
			);
		}
	}
}


BOOL CALLBACK emWndsScreen::MonitorEnumProc(
	HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData
)
{
	emWndsScreen * screen;
	MONITORINFO info;

	screen=(emWndsScreen*)dwData;
	memset(&info,0,sizeof(info));
	info.cbSize=sizeof(info);
	if (GetMonitorInfo(hMonitor,&info)) {
		if ((info.dwFlags&MONITORINFOF_PRIMARY)!=0) {
			screen->MonitorRects.Insert(0,info.rcMonitor);
		}
		else {
			screen->MonitorRects.Add(info.rcMonitor);
		}
	}
	return TRUE;
}


void emWndsScreen::UpdateKeymapAndInputState()
{
	BYTE newKeymap[256];

	memset(newKeymap,0,sizeof(newKeymap));
	if (GetKeyboardState(newKeymap) && memcmp(Keymap,newKeymap,sizeof(Keymap))!=0) {
		memcpy(Keymap,newKeymap,sizeof(Keymap));
		UpdateInputStateFromKeymap();
	}
}


void emWndsScreen::UpdateInputStateFromKeymap()
{
	unsigned char keyStates[32];
	int i,k;

	memset(keyStates,0,sizeof(keyStates));
	for (i=0; i<(int)sizeof(Keymap); i++) if ((Keymap[i]&0x80)!=0) {
		k=(int)ConvertKey(i,NULL);
		if (k!=EM_KEY_NONE) keyStates[k>>3]|=(unsigned char)(1<<(k&7));
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
	if (InputState.Get(EM_KEY_WHEEL_UP)) {
		keyStates[EM_KEY_WHEEL_UP>>3]|=1<<(EM_KEY_WHEEL_UP&7);
	}
	if (InputState.Get(EM_KEY_WHEEL_DOWN)) {
		keyStates[EM_KEY_WHEEL_DOWN>>3]|=1<<(EM_KEY_WHEEL_DOWN&7);
	}
	if (InputState.Get(EM_KEY_WHEEL_LEFT)) {
		keyStates[EM_KEY_WHEEL_LEFT>>3]|=1<<(EM_KEY_WHEEL_LEFT&7);
	}
	if (InputState.Get(EM_KEY_WHEEL_RIGHT)) {
		keyStates[EM_KEY_WHEEL_RIGHT>>3]|=1<<(EM_KEY_WHEEL_RIGHT&7);
	}
	if (InputState.Get(EM_KEY_BACK_BUTTON)) {
		keyStates[EM_KEY_BACK_BUTTON>>3]|=1<<(EM_KEY_BACK_BUTTON&7);
	}
	if (InputState.Get(EM_KEY_FORWARD_BUTTON)) {
		keyStates[EM_KEY_FORWARD_BUTTON>>3]|=1<<(EM_KEY_FORWARD_BUTTON&7);
	}
	if (InputState.Get(EM_KEY_TOUCH)) {
		keyStates[EM_KEY_TOUCH>>3]|=1<<(EM_KEY_TOUCH&7);
	}
	if (memcmp(InputState.GetKeyStates(),keyStates,32)!=0) {
		memcpy(InputState.GetKeyStates(),keyStates,32);
		InputStateClock++;
	}
}


int emWndsScreen::CompareCurMapElemAgainstKey(
	const CursorMapElement * obj, void * key, void * context
)
{
	return obj->CursorId-*((int*)key);
}


HCURSOR emWndsScreen::GetCursorHandle(int cursorId)
{
	HCURSOR hcur;
	int idx;

	idx=CursorMap.BinarySearchByKey(&cursorId,CompareCurMapElemAgainstKey);
	if (idx>=0) return CursorMap[idx].CursorHandle;
	switch (cursorId) {
		default:
		case emCursor::NORMAL:
			hcur=LoadCursor(NULL,IDC_ARROW);
			break;
		case emCursor::INVISIBLE:
			hcur=NULL;
			break;
		case emCursor::WAIT:
			hcur=LoadCursor(NULL,IDC_WAIT);
			break;
		case emCursor::CROSSHAIR:
			hcur=LoadCursor(NULL,IDC_CROSS);
			break;
		case emCursor::TEXT:
			hcur=LoadCursor(NULL,IDC_IBEAM);
			break;
		case emCursor::HAND:
			hcur=LoadCursor(NULL,IDC_HAND);
			break;
		case emCursor::LEFT_RIGHT_ARROW:
			hcur=LoadCursor(NULL,IDC_SIZEWE);
			break;
		case emCursor::UP_DOWN_ARROW:
			hcur=LoadCursor(NULL,IDC_SIZENS);
			break;
		case emCursor::LEFT_RIGHT_UP_DOWN_ARROW:
			hcur=LoadCursor(NULL,IDC_SIZEALL);
			break;
	}
	idx=~idx;
	CursorMap.InsertNew(idx);
	CursorMap.GetWritable(idx).CursorId=cursorId;
	CursorMap.GetWritable(idx).CursorHandle=hcur;
	return hcur;
}


emInputKey emWndsScreen::ConvertKey(unsigned vk, int * pVariant)
{
	static struct {
		unsigned vk;
		emInputKey key;
		int variant;
	} table[] = {
		{ VK_SHIFT          , EM_KEY_SHIFT         , 0 },
		{ VK_CONTROL        , EM_KEY_CTRL          , 0 },
		{ VK_MENU           , EM_KEY_ALT           , 0 },
		{ VK_LWIN           , EM_KEY_META          , 0 },
		{ VK_RWIN           , EM_KEY_META          , 1 },
		{ VK_UP             , EM_KEY_CURSOR_UP     , 0 },
		{ VK_DOWN           , EM_KEY_CURSOR_DOWN   , 0 },
		{ VK_LEFT           , EM_KEY_CURSOR_LEFT   , 0 },
		{ VK_RIGHT          , EM_KEY_CURSOR_RIGHT  , 0 },
		{ VK_PRIOR          , EM_KEY_PAGE_UP       , 0 },
		{ VK_NEXT           , EM_KEY_PAGE_DOWN     , 0 },
		{ VK_HOME           , EM_KEY_HOME          , 0 },
		{ VK_END            , EM_KEY_END           , 0 },
		{ VK_PRINT          , EM_KEY_PRINT         , 0 },
		{ VK_PAUSE          , EM_KEY_PAUSE         , 0 },
		// VK_APPS generates Shift+F10  { VK_APPS           , EM_KEY_MENU          , 0 },
		{ VK_INSERT         , EM_KEY_INSERT        , 0 },
		{ VK_DELETE         , EM_KEY_DELETE        , 0 },
		{ VK_BACK           , EM_KEY_BACKSPACE     , 0 },
		{ VK_TAB            , EM_KEY_TAB           , 0 },
		{ VK_RETURN         , EM_KEY_ENTER         , 0 },
		{ VK_ESCAPE         , EM_KEY_ESCAPE        , 0 },
		{ VK_SPACE          , EM_KEY_SPACE         , 0 },
		{ '0'               , EM_KEY_0             , 0 },
		{ VK_NUMPAD0        , EM_KEY_0             , 1 },
		{ '1'               , EM_KEY_1             , 0 },
		{ VK_NUMPAD1        , EM_KEY_1             , 1 },
		{ '2'               , EM_KEY_2             , 0 },
		{ VK_NUMPAD2        , EM_KEY_2             , 1 },
		{ '3'               , EM_KEY_3             , 0 },
		{ VK_NUMPAD3        , EM_KEY_3             , 1 },
		{ '4'               , EM_KEY_4             , 0 },
		{ VK_NUMPAD4        , EM_KEY_4             , 1 },
		{ '5'               , EM_KEY_5             , 0 },
		{ VK_NUMPAD5        , EM_KEY_5             , 1 },
		{ '6'               , EM_KEY_6             , 0 },
		{ VK_NUMPAD6        , EM_KEY_6             , 1 },
		{ '7'               , EM_KEY_7             , 0 },
		{ VK_NUMPAD7        , EM_KEY_7             , 1 },
		{ '8'               , EM_KEY_8             , 0 },
		{ VK_NUMPAD8        , EM_KEY_8             , 1 },
		{ '9'               , EM_KEY_9             , 0 },
		{ VK_NUMPAD9        , EM_KEY_9             , 1 },
		{ 'A'               , EM_KEY_A             , 0 },
		{ 'B'               , EM_KEY_B             , 0 },
		{ 'C'               , EM_KEY_C             , 0 },
		{ 'D'               , EM_KEY_D             , 0 },
		{ 'E'               , EM_KEY_E             , 0 },
		{ 'F'               , EM_KEY_F             , 0 },
		{ 'G'               , EM_KEY_G             , 0 },
		{ 'H'               , EM_KEY_H             , 0 },
		{ 'I'               , EM_KEY_I             , 0 },
		{ 'J'               , EM_KEY_J             , 0 },
		{ 'K'               , EM_KEY_K             , 0 },
		{ 'L'               , EM_KEY_L             , 0 },
		{ 'M'               , EM_KEY_M             , 0 },
		{ 'N'               , EM_KEY_N             , 0 },
		{ 'O'               , EM_KEY_O             , 0 },
		{ 'P'               , EM_KEY_P             , 0 },
		{ 'Q'               , EM_KEY_Q             , 0 },
		{ 'R'               , EM_KEY_R             , 0 },
		{ 'S'               , EM_KEY_S             , 0 },
		{ 'T'               , EM_KEY_T             , 0 },
		{ 'U'               , EM_KEY_U             , 0 },
		{ 'V'               , EM_KEY_V             , 0 },
		{ 'W'               , EM_KEY_W             , 0 },
		{ 'X'               , EM_KEY_X             , 0 },
		{ 'Y'               , EM_KEY_Y             , 0 },
		{ 'Z'               , EM_KEY_Z             , 0 },
		{ VK_F1             , EM_KEY_F1            , 0 },
		{ VK_F2             , EM_KEY_F2            , 0 },
		{ VK_F3             , EM_KEY_F3            , 0 },
		{ VK_F4             , EM_KEY_F4            , 0 },
		{ VK_F5             , EM_KEY_F5            , 0 },
		{ VK_F6             , EM_KEY_F6            , 0 },
		{ VK_F7             , EM_KEY_F7            , 0 },
		{ VK_F8             , EM_KEY_F8            , 0 },
		{ VK_F9             , EM_KEY_F9            , 0 },
		{ VK_F10            , EM_KEY_F10           , 0 },
		{ VK_F11            , EM_KEY_F11           , 0 },
		{ VK_F12            , EM_KEY_F12           , 0 },
		{ 0                 , EM_KEY_NONE          , 0 }
	};
	int i;

	for (i=0; ; i++) {
		if (table[i].vk==vk || table[i].vk==0) break;
	}
	if (pVariant) *pVariant=table[i].variant;
	return table[i].key;
}


emWndsScreen::WaitCursorThread::WaitCursorThread()
{
	ParentThreadId=::GetCurrentThreadId();
	Clock=emGetClockMS();
	CursorChanged=false;
	Start(NULL);
}


emWndsScreen::WaitCursorThread::~WaitCursorThread()
{
	QuitEvent.Send();
	WaitForTermination();
}


void emWndsScreen::WaitCursorThread::SignOfLife()
{
	Mutex.Lock();
	Clock=emGetClockMS();
	Mutex.Unlock();
}


bool emWndsScreen::WaitCursorThread::CursorToRestore()
{
	bool b;

	Mutex.Lock();
	b=CursorChanged;
	CursorChanged=false;
	Mutex.Unlock();
	return b;
}


int emWndsScreen::WaitCursorThread::Run(void * arg)
{
	static const emUInt64 blockTimeMS=125;
	HCURSOR hcur;
	emUInt64 t;

	hcur=LoadCursor(NULL,IDC_WAIT);

	if (!AttachThreadInput(::GetCurrentThreadId(),ParentThreadId,TRUE)) {
		emFatalError(
			"emWndsScreen::WaitCursorThread: AttachThreadInput failed: %s",
			emGetErrorText(::GetLastError()).Get()
		);
	}

	do {
		Mutex.Lock();
		t=Clock;
		Mutex.Unlock();
		t=emGetClockMS()-t;
		if (t<blockTimeMS) {
			t=blockTimeMS-t+1;
		}
		else {
			emDLog("emWndsScreen::WaitCursorThread: blocking detected");
			Mutex.Lock();
			SetCursor(hcur);
			CursorChanged=true;
			Mutex.Unlock();
			t=blockTimeMS;
		}
	} while (!QuitEvent.Receive(1,(unsigned)t));
	return 0;
}


emThreadMiniMutex emWndsScreen::InstanceListMutex;
emWndsScreen * emWndsScreen::InstanceList = NULL;
