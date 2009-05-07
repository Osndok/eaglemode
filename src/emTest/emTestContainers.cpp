//------------------------------------------------------------------------------
// emTestContainers.cpp
//
// Copyright (C) 2005-2009 Oliver Hamann.
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

#include <emCore/emStd2.h>
#include <emCore/emList.h>
#include <emCore/emAvlTree.h>

#define MY_ASSERT(c) \
	if (!(c)) emFatalError("%s, %d: assertion failed: %s",__FILE__,__LINE__,#c)


static void TestSortArray()
{
	printf("TestSortArray...\n");
	emArray<int> b;
	emUInt64 clk;
	int i;

	for (i=0; i<1000000; i++) b+=rand();
	clk=emGetClockMS();
	emSortArray(
		b.GetWritable(),
		b.GetCount(),
		emStdComparer<int>::Compare,
		(void*)NULL /* Watcom needs the (void*) */
	);
	printf("time = %dms\n",(int)(emGetClockMS()-clk));
	for (i=0; i<b.GetCount()-1; i++) {
		MY_ASSERT(b[i]<=b[i+1]);
	}
}


static void TestSortList()
{
	printf("TestSortList...\n");
	emUInt64 clk;
	emList<int> l;
	emList<int>::Iterator li;
	int i;

	for (i=0; i<1000000; i++) l+=rand();
	clk=emGetClockMS();
	l.Sort(emStdComparer<int>::Compare);
	printf("time = %dms\n",(int)(emGetClockMS()-clk));
	for (li.SetFirst(l); li; ++li) {
		if (l.GetNext(li)) {
			MY_ASSERT(*li.Get() <= *l.GetNext(li));
		}
	}
#if defined(_WIN32)
	printf("(destruction of large list may take a while)\n");
#endif
}


static void TestStringList()
{
	printf("TestStringList...\n");
	emList<emString> a, b;
	emList<emString>::Iterator i, i1, i2, i3;
	const emString * j;
	emString str;

	a="word";
	a+="helo";
	for (i.SetFirst(a); i; ++i) a.GetWritable(i)->Insert(3,'l');
	a.Sort(emStdComparer<emString>::Compare);
	for (i.SetLast(a); i; --i) a.InsertAfter(i," ");
	*a.GetLastWritable()="\n";
	for (str="", j=a.GetFirst(); j; j=a.GetNext(j)) str+=*j;
	MY_ASSERT(str=="hello world\n");

	a=emString("fox");
	i1.SetLast(a);
	a.InsertAtBeg(emString("the"));
	a.InsertAfter(a.GetFirst(),emString("quick"));
	a.InsertBefore(i1,emString("brown"));
	a.InsertBefore(i1,emString("jumps"));
	a.Add(emString("over"));
	a.MoveBefore(a.GetPrev(i1),i1);
	b=a;
	b.Remove(b.GetNext(b.GetFirst()),b.GetLast());
	b+=emString("dog");
	i2.SetLast(b);
	a.MoveToEnd(&b);
	a.InsertBefore(i2,emString("lazy"));
	for (str="", j=a.GetFirst(); j; j=a.GetNext(j)) {
		str+=*j;
		if (j!=a.GetLast()) str+=" ";
	}
	MY_ASSERT(str=="the quick brown fox jumps over the lazy dog");

	a.InsertBefore(a.GetAtIndex(a.GetCount()/2),a);
	for (str="", j=a.GetFirst(); j; j=a.GetNext(j)) {
		str+=*j;
		if (j!=a.GetLast()) str+=" ";
	}
	MY_ASSERT(str=="the quick brown fox the quick brown fox jumps over the lazy dog jumps over the lazy dog");

	a.Sort(emStdComparer<emString>::Compare);
	for (str="", j=a.GetFirst(); j; j=a.GetNext(j)) {
		str+=*j;
		if (j!=a.GetLast()) str+=" ";
	}
	MY_ASSERT(str=="brown brown dog dog fox fox jumps jumps lazy lazy over over quick quick the the the the");
}


static void TestCharArray()
{
	printf("TestCharArray...\n");
	emArray<char> a1, a2;
	int tuningLevel;

	for (tuningLevel=0; tuningLevel<=4; tuningLevel++) {
		a1.SetTuningLevel(tuningLevel);
		a1.Empty();
		a1+='a';
		a1+=a1;
		a1.Add("cd",2,true);
		a1.AddNew(1);
		a2=a1;
		MY_ASSERT(a2.GetTuningLevel()==tuningLevel);
		a1.GetWritable(a1.GetCount()-1)='e';
		a2.Set(a2.GetCount()-1,'E');
		a1+=a2.Extract(2,1);
		a1+=a2;
		MY_ASSERT(memcmp(a1.Get(),"aacdecaadE",10)==0);
		a1.Replace(1,1,"ABC",3);
		a1.Sort(emStdComparer<char>::Compare);
		a1.BinaryInsert('b',emStdComparer<char>::Compare);
		a1.Replace(3,4,a2);
		a1.Replace(4,3,a1.Get()+5,3);
		a1.Replace(4,3,a1.Get()+2,3);
		a1+=a1;
		a1.Insert(13,"xy",2);
		MY_ASSERT(memcmp(a1.Get(),"ABCaCadbccddexyABCaCadbccdde",28)==0);
		MY_ASSERT(a1.GetTuningLevel()==tuningLevel);
	}
}


static void TestStringArray()
{
	printf("TestStringArray...\n");
	emArray<emString> a1, a2;
	int tuningLevel,i;
	emString str;

	for (tuningLevel=0; tuningLevel<=1; tuningLevel++) {
		a1.SetTuningLevel(tuningLevel);
		a1=emString("brown");
		a1=emString("The")+a1;
		a1.Add(emString("fox"));
		a1=a1+emString("jumps");
		a1.Add(emString("over"));
		a1.Add(a1.Get(0));
		a1.Add(emString("lazy"));
		a1+=emString("cat");
		a1.Insert(1,emString("quick"));
		a1.Replace(a1.GetCount()-1,1,emString("dog"));
		a1.GetWritable(6).Replace(0,1,'t');
		a2=a1;
		MY_ASSERT(a2.GetTuningLevel()==tuningLevel);
		a1+=emString("|");
		a1=a1+a2;
		a2=a1.Extract(5,10);
		a1+=emString("|");
		a1+=a2;
		a1.SetCount(9);
		a1.SetCount(10);
		for (str=a1[0], i=1; i<a1.GetCount(); i++) str+=","+a1[i];
		MY_ASSERT(str=="The,quick,brown,fox,jumps,over,the,lazy,dog,");
		a1.Sort(emStdComparer<emString>::Compare);
		a1.BinaryInsert("hello",emStdComparer<emString>::Compare);
		a1.BinaryInsert("bello",emStdComparer<emString>::Compare);
		a1.BinaryInsert("hello",emStdComparer<emString>::Compare);
		for (str=a1[0], i=1; i<a1.GetCount(); i++) str+=","+a1[i];
		MY_ASSERT(str==",The,bello,brown,dog,fox,hello,hello,jumps,lazy,over,quick,the");
		MY_ASSERT(a1.GetTuningLevel()==tuningLevel);
	}
}


static void TestAvlTree()
{
	printf("TestAvlTree...\n");
	emAvlTreeExample<emString> t;
	emAvlTreeExample<emString>::Iterator i;
	emString str;

	t.Insert("the");
	t.Insert("quick");
	t.Insert("brown");
	t.Insert("fox");
	t.Insert("jumps");
	t.Insert("over");
	t.Insert("the");
	t.Insert("crazy");
	t.Insert("dog");
	if (t.Search("mouse")) t.Insert("cat");
	if (t.Search("crazy")) {
		t.Remove("crazy");
		t.Insert("lazy");
	}
	for (str="", i.StartFirst(t); i.Get(); i.Next()) str+=*i.Get();
	MY_ASSERT(str=="browndogfoxjumpslazyoverquickthe");
}


static void TestUtf8()
{
	printf("TestUtf8...\n");
	char tmp[256];
	int i,j,l,m,u;
	//wchar_t wcs[256];
	//size_t n;

	printf("emIsUtf8System(): %s\n",emIsUtf8System() ? "true" : "false");
	for (i=0; i<10000000; i++) {
		u=emGetIntRandom(1,INT_MAX);
		l=emEncodeUtf8Char(tmp,u);
		MY_ASSERT(l>=1);
		MY_ASSERT(l<=6);
		m=emDecodeUtf8Char(&j,tmp,l);
		MY_ASSERT(u==j);
		MY_ASSERT(m==l);
		//n=mbstowcs(wcs,tmp,l);
		//MY_ASSERT(n==1);
		//MY_ASSERT(u==wcs[0]);
	}
}


int main(int argc, char * argv[])
{
	emInitLocale();
	emEnableDLog();

	TestSortArray();
	TestSortList();
	TestStringList();
	TestCharArray();
	TestStringArray();
	TestAvlTree();
	TestUtf8();

	printf("Success\n");
	return 0;
}
