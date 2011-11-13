//------------------------------------------------------------------------------
// emThread.h
//
// Copyright (C) 2009-2010 Oliver Hamann.
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

#ifndef emThread_h
#define emThread_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif

class emThreadPrivate;
struct emThreadEventReceivers;


//==============================================================================
//================================= emThreadId =================================
//==============================================================================

typedef emUInt64 emThreadId;


//==============================================================================
//================================== emThread ==================================
//==============================================================================

class emThread : public emUncopyable {

public:

	// Class for a thread of program execution.
	//
	// Some hints on multi-threading:
	// - Where not stated otherwise, all classes and functions of emCore are
	//   thread-reentrant but not thread-safe. This means that the classes
	//   and functions can be used by multiple threads simultaneously only
	//   if no shared data is accessed. Yes, even emContext is not
	//   thread-safe.
	// - The objects of container classes like emString, emArray, emList and
	//   emImage are sharing data implicitly. You cannot protect that data
	//   with a mutex. Instead, these classes have a method named
	//   MakeNonShared which must always be called before giving the object
	//   to another thread.
	// - Multi-threading provides many pitfalls, and the correctness of
	//   multi-threaded programs cannot be proved by testing very well.
	// - Alternative concepts are: emEngine, emProcess.

	emThread();
		// Construct a thread object where the thread is not yet
		// running.

	virtual ~emThread();
		// If the thread is still running, WaitForTermination() is
		// called.

	void Start(void * arg);
		// This is a short cut for Start(NULL,arg). Should only be used
		// if Run has been overloaded.

	void Start(int (* func)(void *), void * arg);
		// Start this thread. The thread calls the method Run, and the
		// default Implementation of that method calls the function
		// func. arg is a custom pointer forwarded to Run and func.

	static void StartUnmanaged(int (* func)(void *), void * arg);
		// This function is like the method Start, but the new thread is
		// not managed by an emThread object and the function func is
		// called directly instead of Run. Yes, the return value of func
		// has no meaning and exists only for compatibility with managed
		// threads.

	bool WaitForTermination(unsigned timeoutMS=UINT_MAX);
		// Wait for the thread to terminate.
		// Arguments:
		//   timeoutMS - Time-out in milliseconds. UINT_MAX means
		//               infinite.
		// Returns:
		//   true if the thread has terminated (or never started), or
		//   false on time-out.

	bool IsRunning();
		// Whether the thread has been started and not yet exited.

	int GetExitStatus() const;
		// Get the return value of a terminated thread.

	emThreadId GetThreadId() const;
		// Get the identity number of this thread, if running.

	static emThreadId GetCurrentThreadId();
		// Get the identity number of the calling thread.

	static void ExitCurrentThread(int exitStatus);
		// Exit the calling thread.

protected:

	virtual int Run(void * arg);
		// The default implementation calls the function given with the
		// Start method.

private:

	friend class emThreadPrivate;
	emThreadPrivate * P;
};

inline void emThread::Start(void * arg)
{
	Start(NULL,arg);
}

inline bool emThread::IsRunning()
{
	return WaitForTermination(0);
}


//==============================================================================
//============================ emThreadMutexLocker =============================
//==============================================================================

template <class OBJ> class emThreadMutexLocker : public emUncopyable {

public:

	// This is a template class for a locker which simply locks a mutex for
	// the time the locker exists. The template parameter OBJ describes the
	// class of the mutex.

	emThreadMutexLocker(OBJ & mutex);
		// Calls Lock() on the given mutex.
		// Arguments:
		//   mutex - The mutex to be locked by this locker.

	~emThreadMutexLocker();
		// Calls Unlock() on the mutex.

	OBJ & GetMutex();
		// Get the mutex which is locked by this locker.

private:

	OBJ & Mutex;
};

template <class OBJ> inline emThreadMutexLocker<OBJ>::emThreadMutexLocker(
	OBJ & mutex
)
	: Mutex(mutex)
{
	Mutex.Lock();
}

template <class OBJ> inline emThreadMutexLocker<OBJ>::~emThreadMutexLocker()
{
	Mutex.Unlock();
}

template <class OBJ> inline OBJ & emThreadMutexLocker<OBJ>::GetMutex()
{
	return Mutex;
}


//==============================================================================
//============================= emThreadMiniMutex ==============================
//==============================================================================

class emThreadMiniMutex : public emUncopyable {

public:

	// Class for a mutual exclusion variable with minimum costs. It could
	// also be called a "spin lock". This type of mutex may perform some
	// kind of busy waiting, and it may be unfair. Therefore it should be
	// used for very short critical sections only, so that it is very
	// unlikely that a thread really has to wait. This mutex also does not
	// support recursive locks.

	emThreadMiniMutex();
	~emThreadMiniMutex();

	void Lock();
		// Lock this mutex. Only one thread can have the mutex locked at
		// a time. Therefore this method may block in order to wait for
		// unlocked state. It may be a kind of busy waiting (yielding to
		// another thread again and again).

	void Unlock();
		// Unlock this mutex.

	typedef emThreadMutexLocker<emThreadMiniMutex> Locker;
		// A locker class for this mutex class.

private:

	union {
		volatile emInt64 Val;
		void * Ptr;
	};
};


//==============================================================================
//=============================== emThreadEvent ================================
//==============================================================================

class emThreadEvent : public emUncopyable {

public:

	// Class for an unary event that can be used for thread communication.
	// It is much like a classic semaphore, but the methods are named "Send"
	// and "Receive" instead of "Increment" and "Decrement".

	emThreadEvent();
		// Construct with no pending events.

	emThreadEvent(int count);
		// Construct with an initial number of pending event, or, if
		// count is negative, with an initial phantom receiver. See
		// SetCount.

	~emThreadEvent();
		// Destructor.

	emInt64 Send(emInt64 n=1);
		// Send this event n times. If one or more threads are blocked
		// in Receive, they are served accordingly. A negative n means
		// to decrease the internal counter of pending events (which can
		// be negative) - it acts like a phantom receiver which has
		// precedence over all the other receivers.
		// Arguments:
		//   n         - How many events to send. A negative n acts like
		//               a phantom receiver.
		// Returns:
		//   Same as GetCount.

	bool Receive(emInt64 n=1, unsigned timeoutMS=UINT_MAX);
		// Receive this event n times. If there are at least n pending
		// events, this method returns immediately. Otherwise this
		// thread is added to an internal queue and it sleeps until it
		// is removed from the queue by Send, or until the time-out
		// elapses.
		// Arguments:
		//   n         - How many events to receive. A negative n acts
		//               exactly like Send(-n).
		//   timeoutMS - Time-out in milliseconds. UINT_MAX means
		//               infinite.
		// Returns:
		//   true if the events have been received, or false on
		//   time-out. On time-out, absolute no event is removed from
		//   the internal counter.

	emInt64 GetCount() const;
		// Get the number of pending sendings of this event. A negative
		// value indicates waiting receivers.

	void SetCount(emInt64 count);
		// Same as Send(count-GetCount()), but atomically.

	void Clear();
		// Same as SetCount(0).

private:

	void UpdateReceivers();

	emThreadMiniMutex Mutex;
	emInt64 Count;
	emThreadEventReceivers * Receivers;
};

inline void emThreadEvent::Clear()
{
	SetCount(0);
}


//==============================================================================
//=============================== emThreadMutex ================================
//==============================================================================

class emThreadMutex : private emUncopyable {

public:

	// Class for a normal mutual exclusion variable. Its properties are:
	// - The lock methods support a time-out.
	// - Waiting is non-busy.
	// - Waiting is fair (first come, first serve).
	// - There are methods for solving the readers/writers problem. This
	//   means, the mutex can be locked by either a single thread for
	//   read/write access, or by one or more threads for read-only
	//   access. If you don't need that, simply don't use the *ReadOnly
	//   methods.
	// - This mutex does not support recursive locks. A thread would
	//   dead-lock when trying to lock the mutex more than once (except when
	//   locking for read-only access).

	emThreadMutex();
	~emThreadMutex();

	bool Lock(unsigned timeoutMS=UINT_MAX);
		// Lock this mutex (for read/write access). Only one thread can
		// have the mutex locked this way at a time, without any
		// read-access locks in parallel. Therefore this method may
		// block in order to wait for unlocked state.
		// Arguments:
		//   timeoutMS - Time-out in milliseconds. UINT_MAX means
		//               infinite.
		// Returns:
		//   true if the mutex has been locked, false on time-out.

	void Unlock();
		// Unlock this mutex (from read/write access).

	bool IsLocked() const;
		// Whether this mutex is currently locked (for read/write
		// access).

	bool LockReadOnly(unsigned timeoutMS=UINT_MAX);
		// Lock this mutex for read-only access. Multiple threads
		// can have locked the mutex this way simultaneously. If a
		// thread has locked the mutex for read/write access (with
		// Lock), this method may block in order to wait for unlocked
		// state.
		// Arguments:
		//   timeoutMS - Time-out in milliseconds. UINT_MAX means
		//               infinite.
		// Returns:
		//   true if the mutex has been locked, false on time-out.

	void UnlockReadOnly();
		// Unlock this mutex from read/write access.

	bool IsLockedAnyhow() const;
		// Whether this mutex is currently locked for any access.

	typedef emThreadMutexLocker<emThreadMutex> Locker;
		// A locker class for this mutex class.

	class ReadOnlyLocker : public emUncopyable {
	public:
		// A read-only locker class for this mutex class.
		ReadOnlyLocker(emThreadMutex & mutex);
		~ReadOnlyLocker();
		emThreadMutex & GetMutex();
	private:
		emThreadMutex & Mutex;
	};

private:

	enum { MAX_COUNT = 2147483647 };
	emThreadEvent Event;
};

inline bool emThreadMutex::Lock(unsigned timeoutMS)
{
	return Event.Receive(MAX_COUNT,timeoutMS);
}

inline bool emThreadMutex::IsLocked() const
{
	return Event.GetCount()<=0;
}

inline bool emThreadMutex::LockReadOnly(unsigned timeoutMS)
{
	return Event.Receive(1,timeoutMS);
}

inline bool emThreadMutex::IsLockedAnyhow() const
{
	return Event.GetCount()<MAX_COUNT;
}

inline emThreadMutex::ReadOnlyLocker::ReadOnlyLocker(emThreadMutex & mutex)
	: Mutex(mutex)
{
	Mutex.LockReadOnly();
}

inline emThreadMutex::ReadOnlyLocker::~ReadOnlyLocker()
{
	Mutex.UnlockReadOnly();
}

inline emThreadMutex & emThreadMutex::ReadOnlyLocker::GetMutex()
{
	return Mutex;
}


//==============================================================================
//=========================== emThreadRecursiveMutex ===========================
//==============================================================================

class emThreadRecursiveMutex : public emUncopyable {

public:

	// This is just like emThreadMutex, but it supports recursive locks, and
	// therefore it lacks the readers/writers solution (having the whole
	// luxury would be very costly).

	emThreadRecursiveMutex();
	~emThreadRecursiveMutex();

	bool Lock(unsigned timeoutMS=UINT_MAX);
	void Unlock();
	bool IsLocked() const;

	typedef emThreadMutexLocker<emThreadRecursiveMutex> Locker;

private:

	emThreadEvent Event;
	emThreadMiniMutex Mutex;
	emThreadId ThreadId;
	int LockCount;
};

inline bool emThreadRecursiveMutex::IsLocked() const
{
	return Event.GetCount()<=0;
}


//==============================================================================
//============================= emThreadInitMutex ==============================
//==============================================================================

class emThreadInitMutex : public emUncopyable {

public:

	// Class for a special mutex which helps to execute initialization code
	// once in a thread-safe way. It is best explained by a typical usage
	// example:
	//
	//   void MyFunc()
	//   {
	//     static emThreadInitMutex initMutex;
	//     static Something something;
	//
	//     if (initMutex.Begin()) {
	//       something.Initialize();
	//       initMutex.End();
	//     }
	//     something.Use();
	//   }
	//
	// Only the first call to MyFunc() executes something.Initialize(). If
	// further threads are calling MyFunc() simultaneously, they are blocked
	// in initMutex.Begin() until the first thread calls initMutex.End(),
	// so that it is guaranteed that something.Initialize() has finished
	// before something.Use() is called by any thread.
	//
	// Note that blocking by Begin() may perform some kind of busy waiting,
	// similar to emThreadMiniMutex.

	emThreadInitMutex();
	~emThreadInitMutex();

	bool Begin();
	void End();

private:

	bool BeginImp();

	emThreadMiniMutex Mutex;
	bool Active;
	volatile bool Done;
};

inline bool emThreadInitMutex::Begin()
{
	return Done ? false : BeginImp();
}


#endif
