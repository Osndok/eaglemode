//------------------------------------------------------------------------------
// emWndsScreen.h
//
// Copyright (C) 2006-2011,2016-2017 Oliver Hamann.
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

#ifndef emWndsScreen_h
#define emWndsScreen_h

#ifndef emWindow_h
#include <emCore/emWindow.h>
#endif

#ifndef emThread_h
#include <emCore/emThread.h>
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

class emWndsScheduler;
class emWndsViewRenderer;
class emWndsWindowPort;


class emWndsScreen : public emScreen {

public:

	static void Install(emContext & context);

	virtual void GetDesktopRect(
		double * pX, double * pY, double * pW, double * pH
	) const;

	virtual int GetMonitorCount() const;

	virtual void GetMonitorRect(
		int index, double * pX, double * pY, double * pW, double * pH
	) const;

	virtual double GetDPI() const;

	virtual void MoveMousePointer(double dx, double dy);

	virtual void Beep();

protected:

	virtual emWindowPort * CreateWindowPort(emWindow & window);

private:

	friend class emWndsWindowPort;

	emWndsScreen(emContext & context, const emString & name);
	virtual ~emWndsScreen();

	struct CursorMapElement {
		int CursorId;
		HCURSOR CursorHandle;
	};

	virtual bool Cycle();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
	                                   LPARAM lParam);

	void UpdateGeometry();
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
	                                     LPRECT lprcMonitor, LPARAM dwData);

	void UpdateKeymapAndInputState();
	void UpdateInputStateFromKeymap();

	static int CompareCurMapElemAgainstKey(
		const CursorMapElement * obj, void * key, void * context
	);

	HCURSOR GetCursorHandle(int cursorId);

	static emInputKey ConvertKey(unsigned vk, int * pVariant);

	class WaitCursorThread : private emThread
	{
	public:
		WaitCursorThread();
		virtual ~WaitCursorThread();
		void SignOfLife();
		bool CursorToRestore();
	private:
		virtual int Run(void * arg);
		DWORD ParentThreadId;
		emThreadEvent QuitEvent;
		emThreadMutex Mutex;
		emUInt64 Clock;
		bool CursorChanged;
	};

	static emThreadMiniMutex InstanceListMutex;
	static emWndsScreen * InstanceList;
	emWndsScreen * NextInstance;
	int WindowProcRecursion;
	emWndsScheduler * WndsScheduler;
	WaitCursorThread * WCThread;
	emString WinClassName;

	RECT DesktopRect;
	emArray<RECT> MonitorRects;
	double DPI;
	double PixelTallness;
	emUInt64 GeometryUpdateTime;

	emArray<CursorMapElement> CursorMap;
	bool InputStateToBeFlushed;
	emInputState InputState;
	emUInt64 InputStateClock;
	BYTE Keymap[256];
	double MouseWarpX,MouseWarpY;
	emArray<emWndsWindowPort*> WinPorts;

	emWndsViewRenderer * ViewRenderer;
};


#endif
