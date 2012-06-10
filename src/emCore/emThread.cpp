//------------------------------------------------------------------------------
// emThread.cpp
//
// Copyright (C) 2009,2011 Oliver Hamann.
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

#include <emCore/emThread.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#	include <windows.h>
#else
#	include <unistd.h>
#	include <pthread.h>
#	include <sys/time.h>
#	if defined(__linux__)
#		include <features.h>
		// Originally, GLIBC has eventfd since version 2.7. But some
		// distributions do not have the header file before 2.8.
#		if (__GLIBC__==2 && __GLIBC_MINOR__>=8) || __GLIBC__>=3
#			include <sys/eventfd.h>
#			define HAVE_EVENTFD_H
#		endif
#	endif
#endif


//==============================================================================
//================================== emThread ==================================
//==============================================================================

class emThreadPrivate {
public:
	emThread * Thread;
	int (* Func)(void *);
	void * Arg;
	bool Closed;
	int ExitStatus;
#if defined(_WIN32) || defined(__CYGWIN__)
	DWORD ThreadId;
	HANDLE ThreadHandle;
#else
	emThreadEvent ExitEvent;
	pthread_t ThreadId;
#endif

	inline int Run()
	{
		return Thread->Run(Arg);
	}
};


struct emThreadProcUnmanagedData {
	int (* Func)(void *);
	void * Arg;
};


#if defined(_WIN32) || defined(__CYGWIN__)

	static DWORD WINAPI emThreadProc(LPVOID p)
	{
		return ((emThreadPrivate*)p)->Run();
	}

	static DWORD WINAPI emThreadProcUnmanaged(LPVOID p)
	{
		int (* func)(void *);
		void * arg;

		func=((emThreadProcUnmanagedData*)p)->Func;
		arg=((emThreadProcUnmanagedData*)p)->Arg;
		free(p);
		return (DWORD)func(arg);
	}

	static inline emThreadId emThread_ConvertId(DWORD threadId)
	{
		return (emThreadId)threadId;
	}

#else

	static void emThreadProcEnd(void * p)
	{
		((emThreadPrivate*)p)->ExitEvent.Send();
	}

	static void * emThreadProc(void * p)
	{
		union { void * a; int b; } s;

		pthread_cleanup_push(emThreadProcEnd,p);
		s.a=NULL;
		s.b=((emThreadPrivate*)p)->Run();
		pthread_cleanup_pop(1);
		return s.a;
	}

	static void * emThreadProcUnmanaged(void * p)
	{
		union { void * a; int b; } s;
		int (* func)(void *);
		void * arg;

		func=((emThreadProcUnmanagedData*)p)->Func;
		arg=((emThreadProcUnmanagedData*)p)->Arg;
		free(p);
		s.a=NULL;
		s.b=func(arg);
		return s.a;
	}

	static emThreadId emThread_ConvertId(pthread_t threadId)
	{
		emThreadId id;

		if (sizeof(pthread_t)>sizeof(emThreadId)) {
			emFatalError("emThread: sizeof(pthread_t) too large.");
		}
		id=0;
		memcpy(&id,&threadId,sizeof(pthread_t));
		return id;
	}

#endif


emThread::emThread()
{
	P=NULL;
}


emThread::~emThread()
{
	WaitForTermination();
	if (P) delete P;
}


void emThread::Start(int (* func)(void *), void * arg)
{
	WaitForTermination();
	if (!P) {
		P=new emThreadPrivate;
		P->Thread=this;
	}
	P->Func=func;
	P->Arg=arg;
	P->Closed=false;
	P->ExitStatus=0;
#if defined(_WIN32) || defined(__CYGWIN__)
	P->ThreadId=0;
	P->ThreadHandle=CreateThread(
		NULL,
		0, //??? stack size
		emThreadProc,
		(LPVOID)P,
		0,
		&P->ThreadId
	);
	if (P->ThreadHandle==NULL) {
		emFatalError(
			"emThread: CreateThread failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
#else
	int e=pthread_create(&P->ThreadId,NULL,emThreadProc,(void*)P);
	if (e) {
		emFatalError(
			"emThread: pthread_create failed: %s",
			emGetErrorText(e).Get()
		);
	}
#endif
}


void emThread::StartUnmanaged(int (* func)(void *), void * arg)
{
	emThreadProcUnmanagedData * data;

	data=(emThreadProcUnmanagedData*)malloc(sizeof(emThreadProcUnmanagedData));
	data->Func=func;
	data->Arg=arg;
#if defined(_WIN32) || defined(__CYGWIN__)
	HANDLE h=CreateThread(
		NULL,
		0, //??? stack size
		emThreadProcUnmanaged,
		(LPVOID)data,
		0,
		NULL
	);
	if (h==NULL) {
		emFatalError(
			"emThread: CreateThread failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
	CloseHandle(h);
#else
	pthread_t t;
	int e=pthread_create(&t,NULL,emThreadProcUnmanaged,data);
	if (e) {
		emFatalError(
			"emThread: pthread_create failed: %s",
			emGetErrorText(e).Get()
		);
	}
	e=pthread_detach(t);
	if (e) {
		emFatalError(
			"emThread: pthread_detach failed: %s",
			emGetErrorText(e).Get()
		);
	}
#endif
}


bool emThread::WaitForTermination(unsigned timeoutMS)
{
#if defined(_WIN32) || defined(__CYGWIN__)
	DWORD res,ec;

	if (!P || P->Closed) return true;
	res=WaitForSingleObject(
		P->ThreadHandle,
		timeoutMS==UINT_MAX ? INFINITE : (DWORD)timeoutMS
	);
	if (res!=WAIT_OBJECT_0) {
		if (res!=WAIT_TIMEOUT) {
			emFatalError(
				"emThread: WaitForSingleObject failed: %s",
				emGetErrorText(GetLastError()).Get()
			);
		}
		return false;
	}
	GetExitCodeThread(P->ThreadHandle,&ec);
	P->ExitStatus=(int)ec;
	CloseHandle(P->ThreadHandle);
	P->Closed=true;
	return true;
#else
	union { void * a; int b; } s;
	int e;

	if (!P || P->Closed) return true;
	if (!P->ExitEvent.Receive(1,timeoutMS)) return false;
	s.b=0;
	e=pthread_join(P->ThreadId,&s.a);
	if (e) {
		emFatalError(
			"emThread: pthread_join failed: %s",
			emGetErrorText(e).Get()
		);
	}
	P->ExitStatus=s.b;
	P->Closed=true;
	return true;
#endif
}


int emThread::GetExitStatus() const
{
	if (!P) return 0;
	return P->ExitStatus;
}


emThreadId emThread::GetThreadId() const
{
	if (!P) return 0;
	return emThread_ConvertId(P->ThreadId);
}


emThreadId emThread::GetCurrentThreadId()
{
#if defined(_WIN32) || defined(__CYGWIN__)
	return emThread_ConvertId(::GetCurrentThreadId());
#else
	return emThread_ConvertId(pthread_self());
#endif
}


void emThread::ExitCurrentThread(int exitStatus)
{
#if defined(_WIN32) || defined(__CYGWIN__)
	::ExitThread((DWORD)exitStatus);
#else
	union { void * a; int b; } s;
	s.a=NULL;
	s.b=exitStatus;
	pthread_exit(s.a);
#endif
}


int emThread::Run(void * arg)
{
	if (P->Func) return P->Func(arg);
	else return 0;
}


//==============================================================================
//============================= emThreadMiniMutex ==============================
//==============================================================================

//--------------------------------- emTMM_Xchg ---------------------------------

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

	typedef int emTMM_XchgType;

	static inline emTMM_XchgType emTMM_Xchg(
		volatile emTMM_XchgType * ptr, emTMM_XchgType val
	)
	{
		asm volatile (
#			if defined(__x86_64__)
				"xchgl (%%rdi),%%eax\n"
#			else
				"xchgl (%%edi),%%eax\n"
#			endif
			: "=a"(val) : "a"(val), "D"(ptr) : "memory"
		);
		return val;
	}

#elif defined(_WIN32) || defined(__CYGWIN__)

	typedef LONG emTMM_XchgType;

	static inline emTMM_XchgType emTMM_Xchg(
		volatile emTMM_XchgType * ptr, emTMM_XchgType val
	)
	{
		return InterlockedExchange((LONG*)ptr,val);
	}

#else

#	define NO_emTMM_Xchg

#endif


//----------------------------- emThreadMiniMutex ------------------------------

#if !defined(NO_emTMM_Xchg)

	emThreadMiniMutex::emThreadMiniMutex()
	{
		Val=0;
	}

	emThreadMiniMutex::~emThreadMiniMutex()
	{
	}

	void emThreadMiniMutex::Lock()
	{
		while (emTMM_Xchg((volatile emTMM_XchgType*)(void*)&Val,1)!=0) {
			emSleepMS(0); // Yields to another thread.
		}
	}

	void emThreadMiniMutex::Unlock()
	{
		emTMM_Xchg((volatile emTMM_XchgType*)(void*)&Val,0);
		// Just saying "Val=0" here would not be enough, because with
		// some CPUs a memory barrier or fence is needed, which should
		// be implemented in emTMM_Xchg.
	}

#elif !defined(ANDROID)

	emThreadMiniMutex::emThreadMiniMutex()
	{
		int e;

		if (sizeof(pthread_spinlock_t)>sizeof(Val)) {
			emFatalError(
				"emThreadMiniMutex: sizeof(pthread_spinlock_t) too large."
			);
		}
		e=pthread_spin_init(
			(pthread_spinlock_t*)(void*)&Val,
			PTHREAD_PROCESS_PRIVATE
		);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_spin_init failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

	emThreadMiniMutex::~emThreadMiniMutex()
	{
		int e;

		e=pthread_spin_destroy((pthread_spinlock_t*)(void*)&Val);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_spin_destroy failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

	void emThreadMiniMutex::Lock()
	{
		int e;

		e=pthread_spin_lock((pthread_spinlock_t*)(void*)&Val);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_spin_lock failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

	void emThreadMiniMutex::Unlock()
	{
		int e;

		e=pthread_spin_unlock((pthread_spinlock_t*)(void*)&Val);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_spin_unlock failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

#else

	emThreadMiniMutex::emThreadMiniMutex()
	{
		int e;

		Ptr=malloc(sizeof(pthread_mutex_t));
		e=pthread_mutex_init((pthread_mutex_t*)Ptr,NULL);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_mutex_init failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

	emThreadMiniMutex::~emThreadMiniMutex()
	{
		int e;

		e=pthread_mutex_destroy((pthread_mutex_t*)Ptr);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_mutex_destroy failed: %s",
				emGetErrorText(e).Get()
			);
		}
		free(Ptr);
	}

	void emThreadMiniMutex::Lock()
	{
		int e;

		e=pthread_mutex_lock((pthread_mutex_t*)Ptr);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_mutex_lock failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

	void emThreadMiniMutex::Unlock()
	{
		int e;

		e=pthread_mutex_unlock((pthread_mutex_t*)Ptr);
		if (e) {
			emFatalError(
				"emThreadMiniMutex: pthread_mutex_unlock failed: %s",
				emGetErrorText(e).Get()
			);
		}
	}

#endif


//==============================================================================
//=============================== emThreadEvent ================================
//==============================================================================

struct emThreadEventReceiver {
	emThreadEventReceiver * Next;
	emThreadEventReceiver * Prev;
	emInt64 N;
#if defined(_WIN32) || defined(__CYGWIN__)
	HANDLE EventHandle;
#else
	int Pipe[2]; // { readfd, writefd } or { eventfd, -1 }
#endif
};


struct emThreadEventReceivers {
	emThreadEventReceiver * Ring;
	emInt64 NTotal;
	unsigned RefCount;
};


emThreadEvent::emThreadEvent()
{
	Count=0;
	Receivers=NULL;
}


emThreadEvent::emThreadEvent(int count)
{
	Count=count;
	Receivers=NULL;
}


emThreadEvent::~emThreadEvent()
{
	while (Receivers) {
		Mutex.Lock();
		if (Receivers && Receivers->NTotal) {
			emFatalError("emThreadEvent: destructor called while receiver waiting");
		}
		Mutex.Unlock();
		emSleepMS(0);
	}
}


emInt64 emThreadEvent::Send(emInt64 n)
{
	Mutex.Lock();
	n+=Count;
	Count=n;
	if (Receivers) UpdateReceivers();
	Mutex.Unlock();
	return n;
}


bool emThreadEvent::Receive(emInt64 n, unsigned timeoutMS)
{
	emThreadEventReceiver r;
	bool result;
#if defined(_WIN32) || defined(__CYGWIN__)
	DWORD res;
#else
	timeval tv;
	timeval * ptv;
	fd_set rset;
#endif

	Mutex.Lock();
	Count-=n;
	if (Count>=0 || n<=0) {
		if (n<0 && Receivers) UpdateReceivers();
		Mutex.Unlock();
		return true;
	}
	if (!timeoutMS) {
		Count+=n;
		Mutex.Unlock();
		return false;
	}
	if (!Receivers) {
		Receivers=(emThreadEventReceivers*)malloc(
			sizeof(emThreadEventReceivers)
		);
		Receivers->Ring=NULL;
		Receivers->NTotal=0;
		Receivers->RefCount=0;
	}
	Receivers->NTotal+=n;
	Receivers->RefCount++;
	if (!Receivers->Ring) {
		Receivers->Ring=&r;
		r.Next=&r;
		r.Prev=&r;
	}
	else {
		r.Next=Receivers->Ring;
		r.Prev=r.Next->Prev;
		r.Next->Prev=&r;
		r.Prev->Next=&r;
	}
	r.N=n;

	// The following stuff could also be implemented on
	// pthread_cond_timedwait() where available. But then we would have to
	// use clock_gettime() too, and that would require us to link the rt
	// library. Besides, if the pthread_cond solution is ever made, don't
	// forget also to manage a pthread_mutex to avoid a race condition, and
	// don't forget to use the monotonic clock instead of the real one (see
	// pthread_condattr_setclock).
#if defined(_WIN32) || defined(__CYGWIN__)
	r.EventHandle=CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!r.EventHandle) {
		emFatalError(
			"emThreadEvent: CreateEvent failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
	Mutex.Unlock();
	res=WaitForSingleObject(
		r.EventHandle,
		timeoutMS==UINT_MAX ? INFINITE : (DWORD)timeoutMS
	);
	if (res!=WAIT_OBJECT_0 && res!=WAIT_TIMEOUT) {
		emFatalError(
			"emThreadEvent: WaitForSingleObject failed: %s",
			emGetErrorText(GetLastError()).Get()
		);
	}
	Mutex.Lock();
	CloseHandle(r.EventHandle);
#else
#	if defined(HAVE_EVENTFD_H)
	r.Pipe[0]=eventfd(0,0);
	if (r.Pipe[0]!=-1) {
		static bool logged=false;
		if (!logged) {
			logged=true;
			emDLog("emThreadEvent: eventfd works :-)");
		}
		r.Pipe[1]=-1;
	}
	else
#	endif
	if (pipe(r.Pipe)!=0) {
		emFatalError(
			"emThreadEvent: pipe failed: %s",
			emGetErrorText(errno).Get()
		);
	}
	Mutex.Unlock();
	if (timeoutMS==UINT_MAX) {
		ptv=NULL;
	}
	else {
		tv.tv_sec=(time_t)(timeoutMS/1000);
		tv.tv_usec=(time_t)((timeoutMS%1000)*1000);
		ptv=&tv;
	}
#	if defined(__sun) && defined(__SVR4)
		int retr=0;
#	endif
	for (;;) {
		FD_ZERO(&rset);
		FD_SET(r.Pipe[0],&rset);
#		if defined(__sun) && defined(__SVR4)
			errno=0;
#		endif
		if (select(r.Pipe[0]+1,&rset,NULL,NULL,ptv)>=0) break;
#		if defined(__sun) && defined(__SVR4)
			if (errno==0) {
				if (retr<100) {
					emWarning("emThreadEvent: select returned bad status without setting errno - retrying.");
					retr++;
					continue;
				}
				else {
					emFatalError("emThreadEvent: select continuously returned bad status without setting errno.");
				}
			}
#		endif
		if (errno!=EINTR) {
			emFatalError(
				"emThreadEvent: select failed: %s",
				emGetErrorText(errno).Get()
			);
		}
	}
	Mutex.Lock();
	close(r.Pipe[0]);
	if (r.Pipe[1]!=-1) close(r.Pipe[1]);
#endif

	if (!r.N) {
		result=true;
	}
	else {
		result=false;
		Count+=r.N;
		Receivers->NTotal-=r.N;
		if (r.Next==&r) {
			Receivers->Ring=NULL;
		}
		else {
			r.Next->Prev=r.Prev;
			r.Prev->Next=r.Next;
			if (Receivers->Ring==&r) {
				Receivers->Ring=r.Next;
				UpdateReceivers();
			}
		}
	}
	if (!--Receivers->RefCount) {
		free(Receivers);
		Receivers=NULL;
	}
	Mutex.Unlock();
	return result;
}


emInt64 emThreadEvent::GetCount() const
{
	emThreadEvent * t;
	emInt64 cnt;

	t=(emThreadEvent*)this;
	t->Mutex.Lock();
	cnt=t->Count;
	t->Mutex.Unlock();
	return cnt;
}


void emThreadEvent::SetCount(emInt64 count)
{
	Mutex.Lock();
	if (Count<count && Receivers) {
		Count=count;
		UpdateReceivers();
	}
	else {
		Count=count;
	}
	Mutex.Unlock();
}


void emThreadEvent::UpdateReceivers()
{
	emThreadEventReceiver * r, * rn, * rp;
	emInt64 e, n;

	e=Count+Receivers->NTotal;
	for (;;) {
		r=Receivers->Ring;
		if (!r) break;
		n=r->N;
		e-=n;
		if (e<0) break;
		Receivers->NTotal-=n;
		rn=r->Next;
		if (rn==r) {
			Receivers->Ring=NULL;
		}
		else {
			Receivers->Ring=rn;
			rp=r->Prev;
			rn->Prev=rp;
			rp->Next=rn;
		}
		r->N=0;
#if defined(_WIN32) || defined(__CYGWIN__)
		if (!SetEvent(r->EventHandle)) {
			emFatalError(
				"emThreadEvent: SetEvent failed: %s",
				emGetErrorText(GetLastError()).Get()
			);
		}
#elif defined(HAVE_EVENTFD_H)
		if (r->Pipe[1]==-1) {
			errno=0;
			if (write(r->Pipe[0],"xxxxxxxx",8)!=8) {
				emFatalError(
					"emThreadEvent: Could not write to event fd: %s",
					emGetErrorText(errno).Get()
				);
			}
		}
		else {
			close(r->Pipe[1]);
			r->Pipe[1]=-1;
		}
#else
		close(r->Pipe[1]);
		r->Pipe[1]=-1;
#endif
	}
}


//==============================================================================
//=============================== emThreadMutex ================================
//==============================================================================

emThreadMutex::emThreadMutex()
	: Event(MAX_COUNT)
{
}


emThreadMutex::~emThreadMutex()
{
}


void emThreadMutex::Unlock()
{
	if (Event.Send(MAX_COUNT)>MAX_COUNT) {
		emFatalError("emThreadMutex: unlock without lock.");
	}
}


void emThreadMutex::UnlockReadOnly()
{
	if (Event.Send(1)>MAX_COUNT) {
		emFatalError("emThreadMutex: read-only unlock without lock.");
	}
}


//==============================================================================
//=========================== emThreadRecursiveMutex ===========================
//==============================================================================

emThreadRecursiveMutex::emThreadRecursiveMutex()
	: Event(1)
{
	LockCount=0;
}


emThreadRecursiveMutex::~emThreadRecursiveMutex()
{
}


bool emThreadRecursiveMutex::Lock(unsigned timeoutMS)
{
	emThreadId id;

	id=emThread::GetCurrentThreadId();
	Mutex.Lock();
	if (LockCount>0 && ThreadId==id) {
		LockCount++;
	}
	else {
		if (LockCount>0 || !Event.Receive(1,0)) {
			Mutex.Unlock();
			if (!timeoutMS || !Event.Receive(1,timeoutMS)) return false;
			Mutex.Lock();
		}
		LockCount=1;
		ThreadId=id;
	}
	Mutex.Unlock();
	return true;
}


void emThreadRecursiveMutex::Unlock()
{
	Mutex.Lock();
	if (LockCount<=0) {
		emFatalError("emThreadRecursiveMutex: unlock without lock.");
	}
	if (!--LockCount) Event.Send(1);
	Mutex.Unlock();
}


//==============================================================================
//============================= emThreadInitMutex ==============================
//==============================================================================

emThreadInitMutex::emThreadInitMutex()
{
	Active=false;
	Done=false;
}


emThreadInitMutex::~emThreadInitMutex()
{
}


void emThreadInitMutex::End()
{
	Mutex.Lock();
	if (!Active) emFatalError("emThreadInitMutex: End without Begin");
	Active=false;
	Done=true;
	Mutex.Unlock();
}


bool emThreadInitMutex::BeginImp()
{
	Mutex.Lock();
	if (!Done) {
		if (!Active) {
			Active=true;
			Mutex.Unlock();
			return true;
		}
		do {
			Mutex.Unlock();
			emSleepMS(0);
			Mutex.Lock();
		} while(Active);
	}
	Mutex.Unlock();
	return false;
}
