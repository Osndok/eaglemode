//------------------------------------------------------------------------------
// emTestContainers.cpp
//
// Copyright (C) 2005-2009,2014-2016 Oliver Hamann.
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
#include <emCore/emAvlTreeMap.h>
#include <emCore/emAvlTreeSet.h>
#include <emCore/emAnything.h>

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
		a1.Clear();
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


static void TestAvlTreeMap()
{
	printf("TestAvlTreeMap...\n");
	emAvlTreeMap<emString,emString> m,m2;
	emAvlTreeMap<int,const char *> m3;
	emAvlTreeMap<emString,emString>::Iterator i,i2;

	m["1"]="a";
	MY_ASSERT(!m.IsEmpty());
	i.SetFirst(m);
	m.RemoveFirst();
	MY_ASSERT(m.IsEmpty());
	MY_ASSERT(i.Get()==NULL);
	m["3"]="c";
	i.SetFirst(m);
	m["4"]="D";
	i2.SetLast(m);
	m2=m;
	m["1b"]="N";
	m["3"]="C";
	MY_ASSERT((++i)->Key=="4");
	MY_ASSERT(i->Key=="4");
	MY_ASSERT(i2->Key=="4");
	MY_ASSERT(i==i2);
	MY_ASSERT((--i)->Key=="3");
	MY_ASSERT(i!=i2);
	MY_ASSERT((--i)->Key=="1b");
	m["5"]="Q";
	m["1"]="Z";
	m.RemoveLast();
	m["1"]="A";
	MY_ASSERT(m.GetValueWritable("2",false)==NULL);
	m.Remove("1b");
	MY_ASSERT(m.GetCount()==3);
	MY_ASSERT(m.GetValueWritable("2",true)->Get()==emString().Get());
	*m.GetValueWritable("2",false)="B";
	MY_ASSERT(m.GetCount()==4);
	MY_ASSERT((--i)->Key=="2");
	MY_ASSERT(m.GetFirst()->Key=="1");
	MY_ASSERT(m.GetLast()->Value=="D");
	MY_ASSERT(m.Get("B")==NULL);
	MY_ASSERT(m.Get("3")->Value=="C");
	MY_ASSERT((*m.GetValue("2"))=="B");
	MY_ASSERT((*m.GetKey("2"))=="2");
	i.Set(m2,"9");
	MY_ASSERT(!i);
	i.Set(m,"2");
	i++;
	MY_ASSERT(i);
	MY_ASSERT(i->Key=="3");
	MY_ASSERT(m2.GetCount()==2);
	MY_ASSERT(*m2.GetValue("3")=="c");
	MY_ASSERT(m2.Get("4")->Value=="D");
	for (i.SetLast(m); i; --i) {
		*m.GetValueWritable(i) = i->Value + m[i->Key];
	}
	MY_ASSERT(m["3"]=="CC");
	m.Clear();
	MY_ASSERT(m.GetCount()==0);
	m3[815]="foo";
	MY_ASSERT(strcmp(m3[815],"foo")==0);
}


static void TestAvlTreeSet()
{
	printf("TestAvlTreeSet...\n");

	typedef emAvlTreeSet<int> ISet;
	ISet s,s2;
	ISet::Iterator i,j;
	int k;

	s=ISet(2);
	s.Insert(4);
	s+=ISet(3)+7+(ISet(5)+8+9)-7-(ISet(8)+9);
	MY_ASSERT(s==4+ISet(5)+3+2);
	MY_ASSERT(s!=ISet());
	MY_ASSERT(s.GetCount()==4);
	MY_ASSERT(s.Contains(4));
	MY_ASSERT(!s.Contains(1));
	s-=ISet(3)|2;
	s=s|((8|ISet(5)|9)&(ISet(4)|5|6|7|8));
	MY_ASSERT(s==ISet(5)+4+8);
	i.SetFirst(s);
	++i;
	MY_ASSERT(*i.Get()==5);
	s&=4;
	MY_ASSERT(s==ISet(4));
	s-=5;
	MY_ASSERT(!s.IsEmpty());
	s-=4;
	MY_ASSERT(s.IsEmpty());
	s=s+0+2+4+6+8+10+11+14+16+18+20+22;
	*s.GetWritable(11,false)=12;
	s.RemoveFirst();
	s.RemoveLast();
	s2=s;
	s2.Intersect(ISet(2)+4+8+9);
	MY_ASSERT(s2==ISet(4)+2+8);
	for (i.SetFirst(s), k=2; i; ++i, k+=2) {
		MY_ASSERT(*i.Get()==k);
	}
	MY_ASSERT(k==22);
	MY_ASSERT(*s.Get(8)==8);
	MY_ASSERT(!s.Get(9));
	MY_ASSERT(*s.GetFirst()==2);
	MY_ASSERT(*s.GetLast()==20);
	MY_ASSERT(*s.GetNearestGreater(0)==2);
	MY_ASSERT(*s.GetNearestGreater(4)==6);
	MY_ASSERT(*s.GetNearestGreater(5)==6);
	MY_ASSERT(!s.GetNearestGreater(20));
	MY_ASSERT(*s.GetNearestGreaterOrEqual(4)==4);
	MY_ASSERT(*s.GetNearestGreaterOrEqual(5)==6);
	MY_ASSERT(*s.GetNearestLess(4)==2);
	MY_ASSERT(*s.GetNearestLess(5)==4);
	MY_ASSERT(*s.GetNearestLessOrEqual(4)==4);
	MY_ASSERT(*s.GetNearestLessOrEqual(5)==4);
	i.Set(s,s.Get(10));
	s2=s;
	j.Set(s,s.Get(12));
	s2+=11;
	MY_ASSERT(i!=j);
	s.Remove(i);
	MY_ASSERT(i==j);
	MY_ASSERT(*i.Get()==12);
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


static void TestAnything()
{
	emAnything a1,a2;

	MY_ASSERT(!emCastAnything<int>(a1));
	a1=emCastAnything<int>(4711);
	a2=a1;
	a1=emCastAnything<const char*>("Hello");
	MY_ASSERT(!emCastAnything<int>(a1));
	MY_ASSERT(!emCastAnything<const char*>(a2));
	MY_ASSERT(emCastAnything<const char*>(a1) && strcmp(*emCastAnything<const char*>(a1),"Hello")==0);
	MY_ASSERT(emCastAnything<int>(a2) && *emCastAnything<int>(a2)==4711);
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
	TestAvlTreeMap();
	TestAvlTreeSet();
	TestUtf8();
	TestAnything();

	printf("Success\n");
	return 0;
}
