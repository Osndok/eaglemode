//------------------------------------------------------------------------------
// emRenderThreadPool.cpp
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

#include <emCore/emRenderThreadPool.h>


emRef<emRenderThreadPool> emRenderThreadPool::Acquire(
	emRootContext & rootContext
)
{
	EM_IMPL_ACQUIRE_COMMON(emRenderThreadPool,rootContext,"")
}


void emRenderThreadPool::CallParallel(Func func, void * data, int count)
{
	int i,n;

	n=emMin(ChildThreads.GetCount(), count-1);
	if (n>0) {
		Mutex.Lock();
		CurrentFunc=func;
		CurrentData=data;
		CurrentCount=count;
		CurrentStarted=0;
		ActivateEvent.Send(n);
		while (CurrentStarted<CurrentCount) {
			i=CurrentStarted;
			CurrentStarted++;
			Mutex.Unlock();
			CurrentFunc(CurrentData,i);
			Mutex.Lock();
		}
		Mutex.Unlock();
		DoneEvent.Receive(n);
	}
	else {
		for (i=0; i<count; i++) func(data,i);
	}
}


emRenderThreadPool::emRenderThreadPool(
	emContext & context, const emString & name
) : emModel(context,name)
{
	CoreConfig=emCoreConfig::Acquire(GetRootContext());

	TerminateChildThreads=false;
	CurrentFunc=NULL;
	CurrentData=NULL;
	CurrentCount=0;
	CurrentStarted=0;

	SetMinCommonLifetime(3);

	AddWakeUpSignal(CoreConfig->GetChangeSignal());

	UpdateThreadCount();
}


emRenderThreadPool::~emRenderThreadPool()
{
	DestroyChildThreads();
}


bool emRenderThreadPool::Cycle()
{
	bool busy;

	busy=emModel::Cycle();

	if (IsSignaled(CoreConfig->GetChangeSignal())) {
		UpdateThreadCount();
	}

	return busy;
}


void emRenderThreadPool::UpdateThreadCount()
{
	int n;

	n = emMin(
		emThread::GetHardwareThreadCount(),
		CoreConfig->MaxRenderThreads.Get()
	);

	n--;
	if (n < 0) n = 0;

	if (ChildThreads.GetCount() != n) {
		DestroyChildThreads();
		CreateChildThreads(n);
	}
}


void emRenderThreadPool::CreateChildThreads(int count)
{
	int i;

	Mutex.Lock();
	TerminateChildThreads=false;
	Mutex.Unlock();

	for (i=0; i<count; i++) {
		emThread * t = new emThread();
		t->Start(ChildThreadFunc,this);
		ChildThreads.Add(t);
	}

	emDLog("emRenderThreadPool: ThreadCount = %d", GetThreadCount());
}


void emRenderThreadPool::DestroyChildThreads()
{
	int i;

	Mutex.Lock();
	TerminateChildThreads=true;
	Mutex.Unlock();

	ActivateEvent.Send(ChildThreads.GetCount());

	for (i=0; i<ChildThreads.GetCount(); i++) {
		ChildThreads[i]->WaitForTermination();
		delete ChildThreads[i];
	}

	ChildThreads.Clear();
	TerminateChildThreads=false;
	ActivateEvent.Clear();
	DoneEvent.Clear();
}


int emRenderThreadPool::ChildThreadFunc(void * arg)
{
	return ((emRenderThreadPool*)arg)->ChildThreadRun();
}


int emRenderThreadPool::ChildThreadRun()
{
	int i;

	Mutex.Lock();
	while (!TerminateChildThreads) {
			Mutex.Unlock();
			ActivateEvent.Receive();
			Mutex.Lock();
			while (CurrentStarted<CurrentCount) {
				i=CurrentStarted;
				CurrentStarted++;
				Mutex.Unlock();
				CurrentFunc(CurrentData,i);
				Mutex.Lock();
			}
			DoneEvent.Send();
	}
	Mutex.Unlock();

	return 0;
}
