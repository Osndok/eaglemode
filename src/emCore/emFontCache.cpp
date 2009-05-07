//------------------------------------------------------------------------------
// emFontCache.cpp
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

#include <emCore/emFontCache.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>


emRef<emFontCache> emFontCache::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emFontCache,rootContext,"")
}


void emFontCache::GetChar(
	int unicode, double tgtW, double tgtH, emImage * * ppImg,
	int * pImgX, int * pImgY, int * pImgW, int * pImgH
)
{
	Entry * entry;
	int i1,i2,i,cc,cw,ch;
	emUInt64 t;

	i1=0;
	i2=EntryCount;
	for (;;) {
		if (i1>=i2) {
			*ppImg=&ImgUnknownChar;
			*pImgX=0;
			*pImgY=0;
			*pImgW=ImgUnknownChar.GetWidth();
			*pImgH=ImgUnknownChar.GetHeight();
			return;
		}
		i=(i1+i2)>>1;
		entry=EntryArray[i];
		if (entry->FirstCode>unicode) i2=i;
		else if (entry->LastCode<unicode) i1=i+1;
		else break;
	}

	if (!entry->Loaded) {
		t=emGetClockMS()-LastLoadTime;
		if (t>0) {
			Stress*=pow(0.5,((double)t)/StressHalfLifePeriodMS);
			LastLoadTime+=t;
		}
		if (MemoryUse+entry->MemoryNeed>((emUInt64)MaxMegabytes)*1024*1024) {
			if (Stress*StressSensitivity>emMax(tgtW,tgtH)) {
				*ppImg=&ImgCostlyChar;
				*pImgX=0;
				*pImgY=0;
				*pImgW=ImgCostlyChar.GetWidth();
				*pImgH=ImgCostlyChar.GetHeight();
				return;
			}
			while (
				MemoryUse+entry->MemoryNeed>((emUInt64)MaxMegabytes)*1024*1024 &&
				LRURing.Next!=&LRURing
			) {
				UnloadEntry((Entry*)LRURing.Next);
			}
		}
		LoadEntry(entry);
		Stress+=1.0;
	}
	else {
		TouchEntry(entry);
	}

	*ppImg=&entry->Image;
	i=unicode-entry->FirstCode;
	cc=entry->ColumnCount;
	cw=entry->CharWidth;
	ch=entry->CharHeight;
	*pImgX=(i%cc)*cw;
	*pImgY=(i/cc)*ch;
	*pImgW=cw;
	*pImgH=ch;
}


emFontCache::emFontCache(emContext & context, const emString & name)
	: emModel(context,name)
{
	FontDir=emGetInstallPath(EM_IDT_RES,"emCore","font");
	ImgUnknownChar=emGetResImage(
		GetRootContext(),
		emGetChildPath(FontDir,"UnknownChar.tga"),
		1
	);
	ImgCostlyChar=emGetResImage(
		GetRootContext(),
		emGetChildPath(FontDir,"CostlyChar.tga"),
		1
	);
	EntryArray=NULL;
	EntryCount=0;
	LRURing.Next=&LRURing;
	LRURing.Prev=&LRURing;
	Stress=0.0;
	LastLoadTime=0;
	MemoryUse=0;
	LoadFontDir();
	SetMinCommonLifetime(20);
}


emFontCache::~emFontCache()
{
	Clear();
}


void emFontCache::LoadEntry(Entry * entry)
{
	RingNode * p;
	emArray<char> buf;

	if (!entry->Loaded) {
		emDLog("emFontCache: Loading %s",entry->FilePath.Get());
		try {
			buf=emTryLoadFile(entry->FilePath);
		}
		catch (emString errorMessage) {
			emFatalError("%s",errorMessage.Get());
		}
		try {
			entry->Image.TryParseTga(
				(const unsigned char*)buf.Get(),
				buf.GetCount()
			);
		}
		catch (emString errorMessage) {
			emFatalError(
				"Could not read font file \"%s\": %s",
				entry->FilePath.Get(),
				errorMessage.Get()
			);
		}
		if (entry->Image.GetChannelCount()>1) {
			emWarning(
				"Font file \"%s\" has more than one channel.",
				entry->FilePath.Get()
			);
		}
		buf.Empty();
		entry->ColumnCount=entry->Image.GetWidth()/entry->CharWidth;
		if (entry->ColumnCount<1) entry->ColumnCount=1;
		entry->MemoryNeed=((emUInt64)entry->Image.GetWidth())*entry->Image.GetHeight();
		p=LRURing.Prev;
		entry->Prev=p;
		p->Next=entry;
		entry->Next=&LRURing;
		LRURing.Prev=entry;
		entry->Loaded=true;
		MemoryUse+=entry->MemoryNeed;
	}
	else {
		TouchEntry(entry);
	}
}


void emFontCache::UnloadEntry(Entry * entry)
{
	RingNode * p, * n;

	if (entry->Loaded) {
		n=entry->Next;
		p=entry->Prev;
		n->Prev=p;
		p->Next=n;
		entry->Image.Empty();
		entry->Loaded=false;
		MemoryUse-=entry->MemoryNeed;
	}
}


void emFontCache::TouchEntry(Entry * entry)
{
	RingNode * p, * n;

	n=entry->Next;
	if (n!=&LRURing && entry->Loaded) {
		p=entry->Prev;
		n->Prev=p;
		p->Next=n;
		p=LRURing.Prev;
		entry->Prev=p;
		p->Next=entry;
		entry->Next=&LRURing;
		LRURing.Prev=entry;
	}
}


void emFontCache::LoadFontDir()
{
	emArray<emString> dir;
	emString name,path;
	Entry * entry;
	int i,j,l,fc,lc,cw,ch;

	Clear();
	try {
		dir=emTryLoadDir(FontDir);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
	dir.Sort(emStdComparer<emString>::Compare);
	EntryArray=new Entry*[dir.GetCount()];
	for (i=0; i<dir.GetCount(); i++) {
		name=dir[i];
		path=emGetChildPath(FontDir,name);
		l=name.GetLen();
		if (l<4 || strcasecmp(name.Get()+l-4,".tga")!=0) continue;
		if (sscanf(name.Get(),"%X-%X_%dx%d",&fc,&lc,&cw,&ch)<4) continue;
		if (fc>lc || cw<=0 || ch<=0) continue;
		entry=new Entry;
		entry->Next=NULL;
		entry->Prev=NULL;
		entry->FilePath=path;
		entry->FirstCode=fc;
		entry->LastCode=lc;
		entry->CharWidth=cw;
		entry->CharHeight=ch;
		entry->Loaded=false;
		entry->ColumnCount=1;
		entry->MemoryNeed=((emUInt64)(lc-fc+1))*cw*ch;
		for (j=EntryCount; j>0; j--) {
			if (EntryArray[j-1]->FirstCode<=fc) break;
			EntryArray[j]=EntryArray[j-1];
		}
		EntryArray[j]=entry;
		EntryCount++;
	}
}


void emFontCache::Clear()
{
	int i;

	if (EntryArray) {
		for (i=EntryCount-1; i>=0; i--) delete EntryArray[i];
		delete [] EntryArray;
	}
	EntryArray=NULL;
	EntryCount=0;
	LRURing.Next=&LRURing;
	LRURing.Prev=&LRURing;
	Stress=0.0;
	LastLoadTime=0;
	MemoryUse=0;
}


const double emFontCache::StressHalfLifePeriodMS = 3000;
const double emFontCache::StressSensitivity = 4;

const unsigned emFontCache::MaxMegabytes = 32;
