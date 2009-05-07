//------------------------------------------------------------------------------
// emTestThreads.cpp
//
// Copyright (C) 2009 Oliver Hamann.
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

#define MY_ASSERT(c) \
	if (!(c)) emFatalError("%s, %d: assertion failed: %s",__FILE__,__LINE__,#c)


//----------------------------------- Test1 ------------------------------------

static emThreadEvent Event1;


static int ThreadFunc1(void * arg)
{
	MY_ASSERT(strcmp((char*)arg,"hello 1")==0);
	emSleepMS(1500);
	MY_ASSERT(Event1.GetCount()==-1);
	Event1.Send();
	return 0;
}


static void Test1()
{
	printf("Test1...\n");
	emThread::StartUnmanaged(ThreadFunc1,(void*)"hello 1");
	MY_ASSERT(!Event1.Receive(1,1000));
	MY_ASSERT(Event1.Receive(1,1000));
}


//----------------------------------- Test2 ------------------------------------

static int ThreadFunc2a(void * arg)
{
	MY_ASSERT(strcmp((char*)arg,"hello 2a")==0);
	emSleepMS(1500);
	return 4711;
}


static int ThreadFunc2b(void * arg)
{
	MY_ASSERT(emThread::GetCurrentThreadId()==((emThread*)arg)->GetThreadId());
	emThread::ExitCurrentThread(-12343782);
	return 0;
}


static void Test2()
{
	printf("Test2...\n");
	emThread t;

	t.Start(ThreadFunc2a,(void*)"hello 2a");
	MY_ASSERT(!t.WaitForTermination(1000));
	MY_ASSERT(t.WaitForTermination(1000));
	MY_ASSERT(t.GetExitStatus()==4711);

	t.Start(ThreadFunc2b,(void*)&t);
	MY_ASSERT(t.WaitForTermination(500));
	MY_ASSERT(t.GetExitStatus()==-12343782);
}


//----------------------------------- Test3 ------------------------------------

static void Test3()
{
	printf("Test3...\n");
	emThreadEvent e;
	emThreadMutex m;
	emThreadRecursiveMutex rm;

	MY_ASSERT(!e.Receive(1,0));
	e.Send(1);
	MY_ASSERT(!e.Receive(2,0));
	MY_ASSERT(e.Receive(1,0));
	e.Send(2);
	MY_ASSERT(e.Receive(1,0));
	MY_ASSERT(e.Receive(1,0));
	MY_ASSERT(!e.Receive(1,0));
	e.Send(-1);
	MY_ASSERT(e.Receive(-1,0));
	MY_ASSERT(!e.Receive(1,0));
	e.Send(37494661);
	e.Send(9342378);
	MY_ASSERT(e.Receive(238475,0));
	MY_ASSERT(e.GetCount()==37494661+9342378-238475);
	MY_ASSERT(e.Receive(e.GetCount(),0));
	MY_ASSERT(e.GetCount()==0);
	e.Send(7);
	e.Clear();
	MY_ASSERT(e.GetCount()==0);

	MY_ASSERT(m.Lock(0));
	MY_ASSERT(m.IsLocked());
	MY_ASSERT(m.IsLockedAnyhow());
	MY_ASSERT(!m.Lock(0));
	MY_ASSERT(!m.LockReadOnly(0));
	m.Unlock();
	MY_ASSERT(!m.IsLocked());
	MY_ASSERT(!m.IsLockedAnyhow());
	MY_ASSERT(m.LockReadOnly(0));
	MY_ASSERT(m.LockReadOnly(0));
	MY_ASSERT(m.LockReadOnly(0));
	MY_ASSERT(!m.IsLocked());
	MY_ASSERT(m.IsLockedAnyhow());
	MY_ASSERT(!m.Lock(0));
	m.UnlockReadOnly();
	MY_ASSERT(!m.Lock(0));
	m.UnlockReadOnly();
	MY_ASSERT(!m.Lock(0));
	m.UnlockReadOnly();
	MY_ASSERT(m.Lock(0));

	MY_ASSERT(rm.Lock(0));
	MY_ASSERT(rm.IsLocked());
	MY_ASSERT(rm.Lock(0));
	MY_ASSERT(rm.IsLocked());
	rm.Unlock();
	MY_ASSERT(rm.IsLocked());
	rm.Unlock();
	MY_ASSERT(!rm.IsLocked());
}


//----------------------------------- Test4 ------------------------------------

static emThreadMiniMutex Mutex4a;
static emThreadMutex Mutex4b;
static emThreadRecursiveMutex Mutex4c;
static int Int4;

static int ThreadFunc4(void * arg)
{
	Mutex4a.Lock();
	MY_ASSERT(Int4++==1);
	emSleepMS(200);
	MY_ASSERT(Int4++==2);
	Mutex4b.Lock();
	Mutex4a.Unlock();
	emSleepMS(100);
	Mutex4a.Lock();
	Mutex4a.Unlock();
	MY_ASSERT(Int4++==5);
	emSleepMS(100);
	MY_ASSERT(Int4++==6);
	Mutex4b.Unlock();
	emSleepMS(100);
	MY_ASSERT(!Mutex4b.LockReadOnly(100));
	MY_ASSERT(Mutex4b.LockReadOnly(1000));
	Mutex4c.Lock();
	Mutex4b.UnlockReadOnly();
	MY_ASSERT(Int4++==9);
	emSleepMS(200);
	MY_ASSERT(Int4++==10);
	Mutex4c.Unlock();

	return 0;
}


static void Test4()
{
	printf("Test4...\n");
	emThread t;

	Int4=0;
	Mutex4a.Lock();
	t.Start(ThreadFunc4,NULL);
	emSleepMS(100);
	MY_ASSERT(Int4++==0);
	Mutex4a.Unlock();
	emSleepMS(100);
	Mutex4a.Lock();
	MY_ASSERT(Int4++==3);
	emSleepMS(100);
	MY_ASSERT(Int4++==4);
	Mutex4a.Unlock();
	Mutex4b.Lock();
	MY_ASSERT(Int4++==7);
	emSleepMS(500);
	MY_ASSERT(Int4++==8);
	Mutex4b.Unlock();
	emSleepMS(100);
	Mutex4c.Lock();
	MY_ASSERT(Int4++==11);
	MY_ASSERT(Mutex4b.Lock(0));
	MY_ASSERT(t.WaitForTermination(1000)==true);
}


//-------------------------- Test5 (Readers/Writers) ---------------------------

static emThreadEvent QuitEvent5;
static emThreadMutex RWMutex5;
static emThreadMutex Mutex5;
static int Readers5=0;
static int Writers5=0;
static int MaxReaders5=0;

static int ThreadFunc5(void * arg)
{
	char threadIdStr[256];
	emUInt64 id;
	bool quit;

	id=emThread::GetCurrentThreadId();
	sprintf(threadIdStr,"0x%08X%08X",(unsigned)(id>>32),(unsigned)(id&0xffffffff));

	do {
		if (emGetIntRandom(0,4)==0) {
			RWMutex5.Lock();
				Mutex5.Lock();
					printf("%s: start writing\n",threadIdStr);
					Writers5++;
					MY_ASSERT(Readers5==0);
					MY_ASSERT(Writers5==1);
				Mutex5.Unlock();
				quit=QuitEvent5.Receive(1,emGetIntRandom(50,150));
				Mutex5.Lock();
					printf("%s: stop writing\n",threadIdStr);
					Writers5--;
					MY_ASSERT(Readers5==0);
					MY_ASSERT(Writers5==0);
				Mutex5.Unlock();
			RWMutex5.Unlock();
		}
		else {
			RWMutex5.LockReadOnly();
				Mutex5.Lock();
					printf("%s: start reading\n",threadIdStr);
					Readers5++;
					if (MaxReaders5<Readers5) MaxReaders5=Readers5;
					MY_ASSERT(Readers5>=1);
					MY_ASSERT(Writers5==0);
				Mutex5.Unlock();
				quit=QuitEvent5.Receive(1,emGetIntRandom(50,150));
				Mutex5.Lock();
					printf("%s: stop reading\n",threadIdStr);
					Readers5--;
					MY_ASSERT(Readers5>=0);
					MY_ASSERT(Writers5==0);
				Mutex5.Unlock();
			RWMutex5.UnlockReadOnly();
		}
	} while (!quit);
	return 0;
}


static void Test5()
{
	printf("Test5...\n");
#	define Test5_Threads 20
	emThread t[Test5_Threads];
	int i;

	MaxReaders5=0;
	for (i=0; i<Test5_Threads; i++) t[i].Start(ThreadFunc5,NULL);
	emSleepMS(3000);
	QuitEvent5.Send(Test5_Threads);
	for (i=0; i<Test5_Threads; i++) t[i].WaitForTermination();
	MY_ASSERT(Readers5==0);
	MY_ASSERT(Writers5==0);
	MY_ASSERT(MaxReaders5>1);
}

//------------------------ Test6 (Dining Philosophers) -------------------------

emThreadMutex PrintDataMutex;
emThreadEvent PrintEvent;

class DiningPhilosopher : private emThread {

	emThreadMutex & LeftChopstick;
	emThreadMutex & RightChopstick;
	emThreadEvent StopEvent;
	bool Thinking;
	bool Eating;

public:

	DiningPhilosopher(emThreadMutex & leftChopstick, emThreadMutex & rightChopstick)
		: LeftChopstick(leftChopstick),
		RightChopstick(rightChopstick)
	{
		Thinking=false;
		Eating=false;
		Start(NULL);
	}

	virtual ~DiningPhilosopher()
	{
		StopEvent.Send();
		WaitForTermination();
	}

	bool IsThinking()
	{
		return Thinking;
	}

	bool IsEating()
	{
		return Eating;
	}

private:

	virtual int Run(void * arg)
	{
		for (;;) {
			// --- Think ---
			Thinking=true;
			PrintEvent.Send();
			if (StopEvent.Receive(1,emGetIntRandom(100,200))) break;
			// --- Take chopsticks ---
			Thinking=false;
			PrintEvent.Send();
			for (;;) {
				LeftChopstick.Lock();
				if (RightChopstick.Lock(emGetIntRandom(50,250))) break;
				LeftChopstick.Unlock();
				RightChopstick.Lock();
				if (LeftChopstick.Lock(emGetIntRandom(50,250))) break;
				RightChopstick.Unlock();
			}
			// --- Eat ---
			Eating=true;
			PrintEvent.Send();
			if (StopEvent.Receive(1,emGetIntRandom(100,150))) {
				LeftChopstick.Unlock();
				RightChopstick.Unlock();
				break;
			}
			// --- Release chopsticks ---
			PrintDataMutex.Lock();
			Eating=false;
			LeftChopstick.Unlock();
			RightChopstick.Unlock();
			Thinking=true;
			PrintDataMutex.Unlock();
		}
		return 0;
	}
};


static void Test6()
{
	printf("Test6...\n");
	emThreadMutex c[5];
	DiningPhilosopher * p[5];
	char str[256],str2[256];
	emUInt64 clk;
	int i;

	for (i=0; i<5; i++) p[i]=new DiningPhilosopher(c[i],c[(i+1)%5]);

	clk=emGetClockMS();
	str2[0]=0;
	while (emGetClockMS()-clk<3000) {
		PrintEvent.Receive();
		strcpy(str,
			"    ?    \n"
			"  -   -  \n"
			"?       ?\n"
			" -     - \n"
			" ?  -  ? \n"
		);
		PrintDataMutex.Lock();
		MY_ASSERT(!p[0]->IsEating() || !p[1]->IsEating());
		MY_ASSERT(!p[1]->IsEating() || !p[2]->IsEating());
		MY_ASSERT(!p[2]->IsEating() || !p[3]->IsEating());
		MY_ASSERT(!p[3]->IsEating() || !p[4]->IsEating());
		MY_ASSERT(!p[4]->IsEating() || !p[0]->IsEating());
		if (p[0]->IsThinking()) str[04]='T';
		if (p[1]->IsThinking()) str[28]='T';
		if (p[2]->IsThinking()) str[47]='T';
		if (p[3]->IsThinking()) str[41]='T';
		if (p[4]->IsThinking()) str[20]='T';
		if (p[0]->IsEating()) str[04]='E';
		if (p[1]->IsEating()) str[28]='E';
		if (p[2]->IsEating()) str[47]='E';
		if (p[3]->IsEating()) str[41]='E';
		if (p[4]->IsEating()) str[20]='E';
		if (c[0].IsLocked()) str[12]=' ';
		if (c[1].IsLocked()) str[16]=' ';
		if (c[2].IsLocked()) str[37]=' ';
		if (c[3].IsLocked()) str[44]=' ';
		if (c[4].IsLocked()) str[31]=' ';
		PrintDataMutex.Unlock();
		if (strcmp(str,str2)!=0) {
			strcpy(str2,str);
			printf("\n\n%s",str);
		}
	}

	for (i=0; i<5; i++) delete p[i];
}


//------------------------------------ main ------------------------------------

int main(int argc, char * argv[])
{
	int which;

	emInitLocale();
	emEnableDLog();

	if (argc>=2) which=atoi(argv[1]); else which=-1;

	if (which<0 || which==1) Test1();
	if (which<0 || which==2) Test2();
	if (which<0 || which==3) Test3();
	if (which<0 || which==4) Test4();
	if (which<0 || which==5) Test5();
	if (which<0 || which==6) Test6();

	printf("Success\n");
	return 0;
}
