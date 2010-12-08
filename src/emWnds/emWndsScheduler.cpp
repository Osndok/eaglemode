//------------------------------------------------------------------------------
// emWndsScheduler.cpp
//
// Copyright (C) 2007-2010 Oliver Hamann.
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

#include <emWnds/emWndsScheduler.h>


emWndsScheduler::emWndsScheduler()
{
	NextInstance=NULL;
	State=NOT_RUNNING;
	TimerId=0;
	TerminationInitiated=false;
	DeadlineTime=0;
}


emWndsScheduler::~emWndsScheduler()
{
}


int emWndsScheduler::Run()
{
	emWndsScheduler * * pp;
	MSG msg;
	BOOL b;

	InstanceListMutex.Lock();
	NextInstance=InstanceList;
	InstanceList=this;
	InstanceListMutex.Unlock();

	TerminationInitiated=false;

	TimerId=SetTimer(NULL,0,10,(TIMERPROC)TimerProc);
	if (TimerId==0) {
		emFatalError(
			"emWndsScheduler: SetTimer failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}

	State=HANDLING_MESSAGES;

	for (;;) {
		b=GetMessage(&msg,NULL,0,0);
		if (!b) break;
		if (b==-1) {
			emFatalError(
				"emWndsScheduler: GetMessage failed: %s",
				emGetErrorText(GetLastError()).Get()
			);
		}
		// Please read the comments in emWndsScheduler::TimerProc.
		if (msg.message==WM_TIMER && msg.wParam==TimerId && msg.hwnd==NULL) {
			DoTimeSlice();
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	State=NOT_RUNNING;

	KillTimer(NULL,TimerId);
	TimerId=0;

	InstanceListMutex.Lock();
	for (pp=&InstanceList; *pp!=this; pp=&(*pp)->NextInstance);
	*pp=NextInstance;
	InstanceListMutex.Unlock();
	NextInstance=NULL;

	return msg.wParam;
}


bool emWndsScheduler::IsTimeSliceAtEnd() const
{
	return emGetClockMS()>=DeadlineTime;
}


void emWndsScheduler::InitiateTermination(int returnCode)
{
	if (!TerminationInitiated) {
		TerminationInitiated=true;
		PostQuitMessage(returnCode);
	}
}


void emWndsScheduler::DoTimeSlice()
{
	if (State==HANDLING_MESSAGES) {
		State=CALLING_ENGINES;
		DeadlineTime=emGetClockMS()+50;
		emScheduler::DoTimeSlice();
		State=HANDLING_MESSAGES;
	}
}


VOID CALLBACK emWndsScheduler::TimerProc(
	HWND hwnd, UINT uMsg, TimerIdType idEvent, DWORD dwTime
)
{
	emWndsScheduler * s;

	// Problems with exceptions here
	// -----------------------------
	//
	// This is not just about programmed C++ exception, but also about
	// things like a segmentation fault.
	//
	// Normally, uncaught exceptions are forwarded to WinMain, and an
	// application programmer may decide to override the default handling
	// (the typical dialog) with SetUnhandledExceptionFilter.
	//
	// But for timer messages, exceptions are caught and ignored by
	// DispatchMessage or DefWindowProc, unlike for normal messages.
	//
	// Because we do not want the program to continue after an exception, we
	// catch the exception here and do a very simple crash report. In
	// addition, most of our timer messages are already handled by the main
	// message loop in emWndsScheduler::Run, so that this function is called
	// only while an inner message loop runs, that is while the user moves
	// or resizes a window, or while a modal message box is shown.
	//
	// Note again that the crash report here cannot be overloaded with
	// SetUnhandledExceptionFilter. But it would be possible with
	// AddVectoredExceptionHandler.

	s=NULL;
	try {
		InstanceListMutex.Lock();
		for (s=InstanceList; s && s->TimerId!=idEvent; s=s->NextInstance);
		InstanceListMutex.Unlock();
		if (s) s->DoTimeSlice();
	}
	catch (...) {
		if (s) KillTimer(NULL,s->TimerId);
		emFatalError("uncaught exception");
	}
}


emThreadMiniMutex emWndsScheduler::InstanceListMutex;
emWndsScheduler * emWndsScheduler::InstanceList=NULL;
