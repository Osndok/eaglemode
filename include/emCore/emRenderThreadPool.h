//------------------------------------------------------------------------------
// emRenderThreadPool.h
//
// Copyright (C) 2016-2017 Oliver Hamann.
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

#ifndef emRenderThreadPool_h
#define emRenderThreadPool_h

#ifndef emCoreConfig_h
#include <emCore/emCoreConfig.h>
#endif

#ifndef emThread_h
#include <emCore/emThread.h>
#endif


//==============================================================================
//============================= emRenderThreadPool =============================
//==============================================================================

class emRenderThreadPool : public emModel {

public:

	// Class for a thread pool which can call a function concurrently.
	// Mainly, this is used for speeding up rendering of graphics. The
	// number of threads is the number of threads the hardware can run
	// concurrently, but limited through emCoreConfig::MaxRenderThreads.

	static emRef<emRenderThreadPool> Acquire(emRootContext & rootContext);
		// Acquire the instance in a root context.

	int GetThreadCount() const;
		// Get number of threads. This includes the calling thread.

	typedef void (*Func) (void * data, int index);
		// Data type for a function to be called concurrently.

	void CallParallel(Func func, void * data, int count);
		// Call a function concurrently. This is equivalent to:
		//   for (int i = 0; i < count; i++) func(data, i);
		// But parallelized best possible.

	void CallParallel(Func func, void * data);
		// Same as CallParallel(func,data,GetThreadCount());

protected:

	emRenderThreadPool(emContext & context, const emString & name);
	virtual ~emRenderThreadPool();

	virtual bool Cycle();

private:

	void UpdateThreadCount();
	void CreateChildThreads(int count);
	void DestroyChildThreads();

	static int ChildThreadFunc(void * arg);
	int ChildThreadRun();

	emRef<emCoreConfig> CoreConfig;
	emArray<emThread*> ChildThreads;
	bool TerminateChildThreads;
	Func CurrentFunc;
	void * CurrentData;
	int CurrentCount;
	int CurrentStarted;
	emThreadMiniMutex Mutex;
	emThreadEvent ActivateEvent;
	emThreadEvent DoneEvent;
};

inline int emRenderThreadPool::GetThreadCount() const
{
	return ChildThreads.GetCount() + 1;
}

inline void emRenderThreadPool::CallParallel(Func func, void * data)
{
	CallParallel(func,data,GetThreadCount());
}


#endif
