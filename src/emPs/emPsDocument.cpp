//------------------------------------------------------------------------------
// emPsDocument.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emPs/emPsDocument.h>


emPsDocument::emPsDocument()
{
	Data=&EmptyData;
}


emPsDocument::emPsDocument(const emPsDocument & doc)
{
	Data=doc.Data;
	Data->RefCount++;
}


emPsDocument::~emPsDocument()
{
	if (!--Data->RefCount) DeleteData();
}


emPsDocument & emPsDocument::operator = (const emPsDocument & doc)
{
	doc.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=doc.Data;
	return *this;
}


bool emPsDocument::operator == (const emPsDocument & doc) const
{
	if (Data==doc.Data) return true;
	if (Data->Adler32!=doc.Data->Adler32) return false;
	if (Data->Script.GetCount()!=doc.Data->Script.GetCount()) return false;
	if (Data->Script.Get()==doc.Data->Script.Get()) return true;
	if (
		memcmp(
			Data->Script.Get(),
			doc.Data->Script.Get(),
			Data->Script.GetCount()
		)!=0
	) return false;
	return true;
}


void emPsDocument::Empty()
{
	if (!--Data->RefCount) DeleteData();
	Data=&EmptyData;
}


void emPsDocument::TrySetScript(const emArray<char> & script) throw(emString)
{
	enum {
		IN_STARTUP,
		IN_PAGE,
		IN_TRAILER,
		IN_SUB_DOC
	} state,stateBackup;
	emArray<emString> mediaNames;
	emArray<double> mediaW,mediaH;
	const char * p, * p2, * end, * pLine;
	PageInfo * page;
	double version;
	bool isEps;
	int docNestings;
	emString str;
	bool landscape,orientationFound,orientationAtEnd;
	bool bbFound,bbAtEnd,pgSizeFound;
	double bbx1,bby1,bbx2,bby2,d1,d2,d3,d4,pgW,pgH;
	int i,j;

	//************************* IMPORTANT **************************
	//* When modifying this code, please keep in mind the behavior *
	//* of emPsRenderer: It feeds the PostScript interpreter       *
	//* (Ghostscript) once with the start-up script, and then with *
	//* the page scripts in any order and with any repetitions,    *
	//* while inserting commands for device setup and              *
	//* synchronization before and after each page script.         *
	//* Therefore, for example, typical EPS documents are not      *
	//* supported (would require a translation with ps2ps).        *
	//**************************************************************

	if (&script==&Data->Script) return;
	if (!--Data->RefCount) DeleteData();
	Data=new SharedData;
	Data->Script=script;

	p=Data->Script.Get();
	end=p+Data->Script.GetCount();

	if (
		!ParseExactly(&p,end,"%!PS-Adobe-") ||
		!ParseDoubleArg(&p,end,&version)
	) {
		Empty();
		throw emString("Unsupported PostScript document format.");
	}
	ParseArgSpaces(&p,end);
	isEps=ParseExactly(&p,end,"EPSF");

	state=IN_STARTUP;
	stateBackup=IN_STARTUP;
	docNestings=0;
	orientationFound=false;
	orientationAtEnd=false;
	landscape=false;
	bbFound=false;
	bbAtEnd=false;
	bbx1=0.0;
	bby1=0.0;
	bbx2=0.0;
	bby2=0.0;
	pgSizeFound=false;
	pgW=0.0;
	pgH=0.0;
	page=NULL;
	for (;;) {
		ParseToNextLine(&p,end);
		pLine=p;
		if (p>=end) {
			if (state!=IN_TRAILER) Data->TrailerPos=Data->Script.GetCount();
			break;
		}
		else if (p[0]!='%' || p+1>=end || p[1]!='%') {
		}
		else if (ParseExactly(&p,end,"%%Page:")) {
			if (
				(state==IN_STARTUP || state==IN_PAGE) &&
				ParseTextArg(&p,end,&str) &&
				ParseIntArg(&p,end,&i)
			) {
				j=Data->Pages.GetCount();
				Data->Pages.AddNew();
				page=&Data->Pages.GetWritable(j);
				page->Pos=pLine-Data->Script.Get();
				page->IsLandscape=0;
				page->OrientationFound=0;
				page->SizeFound=0;
				page->Width=0.0;
				page->Height=0.0;
				page->Label=str;
				state=IN_PAGE;
			}
		}
		else if (ParseExactly(&p,end,"%%Trailer")) {
			if (state==IN_STARTUP || state==IN_PAGE) {
				state=IN_TRAILER;
				Data->TrailerPos=pLine-Data->Script.Get();
			}
		}
		else if (ParseExactly(&p,end,"%%Orientation:")) {
			if (
				(
					(state==IN_STARTUP && !orientationAtEnd && !orientationFound) ||
					(state==IN_TRAILER && orientationAtEnd)
				) &&
				ParseTextArg(&p,end,&str)
			) {
				if (str=="Landscape") {
					landscape=true;
					orientationFound=true;
				}
				else if (str=="Portrait") {
					landscape=false;
					orientationFound=true;
				}
				else if (str=="atend") {
					orientationAtEnd=true;
				}
			}
		}
		else if (ParseExactly(&p,end,"%%PageOrientation:")) {
			if (ParseTextArg(&p,end,&str)) {
				if (state==IN_PAGE && !page->OrientationFound) {
					if (str=="Landscape") {
						page->IsLandscape=1;
						page->OrientationFound=1;
					}
					else if (str=="Portrait") {
						page->IsLandscape=0;
						page->OrientationFound=1;
					}
				}
				else if (state==IN_STARTUP) {
					if (str=="Landscape") {
						landscape=true;
						orientationFound=true;
						orientationAtEnd=false;
					}
					else if (str=="Portrait") {
						landscape=false;
						orientationFound=true;
						orientationAtEnd=false;
					}
				}
			}
		}
		else if (ParseExactly(&p,end,"%%BoundingBox:")) {
			if (
				(
					(state==IN_STARTUP && !bbAtEnd && !bbFound) ||
					(state==IN_TRAILER && bbAtEnd)
				) &&
				ParseDoubleArg(&p,end,&d1)
			) {
				if (
					ParseDoubleArg(&p,end,&d2) &&
					ParseDoubleArg(&p,end,&d3) &&
					ParseDoubleArg(&p,end,&d4)
				) {
					bbx1=d1;
					bby1=d2;
					bbx2=d3;
					bby2=d4;
					bbFound=true;
				}
			}
			else if (
				state==IN_STARTUP && !bbAtEnd && !bbFound &&
				ParseTextArg(&p,end,&str) && str=="atend"
			) {
				bbAtEnd=true;
			}
		}
		else if (ParseExactly(&p,end,"%%DocumentPaperSizes:")) {
			if (
				state==IN_STARTUP &&
				!pgSizeFound &&
				ParseTextArg(&p,end,&str) &&
				GetSizeOfStandardPaperType(str,&d1,&d2)
			) {
				pgW=d1;
				pgH=d2;
				pgSizeFound=true;
			}
		}
		else if (ParseExactly(&p,end,"%%PaperSize:")) {
			if (
				ParseTextArg(&p,end,&str) &&
				GetSizeOfStandardPaperType(str,&d1,&d2)
			) {
				if (state==IN_PAGE && !page->SizeFound) {
					page->Width=d1;
					page->Height=d2;
					page->SizeFound=true;
				}
				else if (state==IN_STARTUP) {
					pgW=d1;
					pgH=d2;
					pgSizeFound=true;
				}
			}
		}
		else if (ParseExactly(&p,end,"%%DocumentMedia:")) {
			if (state==IN_STARTUP) {
				for (;;) {
					if (!ParseTextArg(&p,end,&str)) break;
					if (!ParseDoubleArg(&p,end,&d1)) break;
					if (!ParseDoubleArg(&p,end,&d2)) break;
					mediaNames.Add(str);
					mediaW.Add(d1);
					mediaH.Add(d2);
					p2=p;
					ParseToNextLine(&p2,end);
					if (!ParseExactly(&p2,end,"%%+")) break;
					p=p2;
				}
			}
		}
		else if (ParseExactly(&p,end,"%%PageMedia:")) {
			if (ParseTextArg(&p,end,&str)) {
				for (i=0; i<mediaNames.GetCount(); i++) {
					if (mediaNames[i]==str) break;
				}
				if (i<mediaNames.GetCount()) {
					if (state==IN_PAGE && !page->SizeFound) {
						page->Width=mediaW[i];
						page->Height=mediaH[i];
						page->SizeFound=true;
					}
					else if (state==IN_STARTUP) {
						pgW=mediaW[i];
						pgH=mediaH[i];
						pgSizeFound=true;
					}
				}
			}
		}
		else if (ParseExactly(&p,end,"%%BeginDocument:")) {
			if (!docNestings) {
				stateBackup=state;
				state=IN_SUB_DOC;
			}
			docNestings++;
		}
		else if (ParseExactly(&p,end,"%%EndDocument")) {
			if (state==IN_SUB_DOC) {
				docNestings--;
				if (!docNestings) state=stateBackup;
			}
		}
		else if (ParseExactly(&p,end,"%%BeginBinary:")) {
			if (ParseIntArg(&p,end,&i)) {
				ParseToNextLine(&p,end);
				p+=i;
				if (p>end) p=end;
				while (p<end && !ParseExactly(&p,end,"%%EndBinary")) {
					ParseToNextLine(&p,end);
				}
			}
		}
		else if (ParseExactly(&p,end,"%%BeginData:")) {
			if (ParseIntArg(&p,end,&i)) {
				if (
					ParseTextArg(&p,end,&str) &&
					ParseTextArg(&p,end,&str) &&
					str=="Lines"
				) {
					ParseToNextLine(&p,end);
					while (i>0) {
						ParseToNextLine(&p,end);
						i--;
					}
				}
				else {
					ParseToNextLine(&p,end);
					p+=i;
					if (p>end) p=end;
				}
				while (p<end && !ParseExactly(&p,end,"%%EndData")) {
					ParseToNextLine(&p,end);
				}
			}
		}
	}

	// Any pages?
	if (Data->Pages.GetCount()<=0) {
		Empty();
		throw emString("Unsupported PostScript document structure.");
	}

	// Calc script lengths.
	if (Data->Pages.GetCount()>0) Data->StartupLen=Data->Pages[0].Pos;
	else Data->StartupLen=Data->TrailerPos;
	for (i=0; i<Data->Pages.GetCount(); i++) {
		if (i+1<Data->Pages.GetCount()) j=Data->Pages[i+1].Pos;
		else j=Data->TrailerPos;
		Data->Pages.GetWritable(i).Len=j-Data->Pages[i].Pos;
	}
	Data->TrailerLen=Data->Script.GetCount()-Data->TrailerPos;

	// Adapt page orientations and sizes.
	if (!pgSizeFound || pgW<1.0 || pgH<1.0) {
		if (mediaNames.GetCount()>0 && mediaW[0]>=1.0 && mediaH[0]>=1.0) {
			pgW=mediaW[0];
			pgH=mediaH[0];
		}
		else if (bbFound && bbx2>=1.0 && bby2>=1.0) {
			pgW=bbx2;
			pgH=bby2;
		}
		else {
			pgW=612;
			pgH=792;
		}
	}
	for (i=0; i<Data->Pages.GetCount(); i++) {
		page=&Data->Pages.GetWritable(i);
		if (!page->SizeFound || page->Width<1.0 || page->Height<1.0) {
			page->Width=pgW;
			page->Height=pgH;
		}
		if (!page->OrientationFound) {
			page->IsLandscape=landscape ? 1 : 0;
		}
		if (page->IsLandscape) {
			d1=page->Width;
			page->Width=page->Height;
			page->Height=d1;
		}
	}

	// Calc MaxPageWidth & MaxPageHeight
	Data->MaxPageWidth=1.0;
	Data->MaxPageHeight=1.0;
	for (i=0; i<Data->Pages.GetCount(); i++) {
		if (Data->MaxPageWidth<Data->Pages[i].Width) {
			Data->MaxPageWidth=Data->Pages[i].Width;
		}
		if (Data->MaxPageHeight<Data->Pages[i].Height) {
			Data->MaxPageHeight=Data->Pages[i].Height;
		}
	}

	// Compact the array of pages.
	Data->Pages.Compact();

	// Adler32 of whole script.
	Data->Adler32=emCalcAdler32(Data->Script.Get(),Data->Script.GetCount());
}


emUInt64 emPsDocument::CalcMemoryNeed() const
{
	emUInt64 m;

	m=sizeof(SharedData);
	m+=Data->Script.GetCount();
	m+=Data->Pages.GetCount()*(sizeof(PageInfo)+20);
	return m;
}


unsigned int emPsDocument::GetDataRefCount() const
{
	return Data==&EmptyData ? UINT_MAX/2 : Data->RefCount;
}


void emPsDocument::ParseToNextLine(const char * * pPos, const char * end)
{
	const char * p;

	for (p=*pPos; p<end; p++) {
		if (*p==0x0a) {
			p++;
			break;
		}
		if (*p==0x0d) {
			p++;
			if (p<end && *p==0x0a) p++;
			break;
		}
	}
	*pPos=p;
}


bool emPsDocument::ParseExactly(
	const char * * pPos, const char * end, const char * str
)
{
	const char * p;

	for (p=*pPos; p<end && *p==*str; ) {
		str++;
		p++;
		if (!*str) {
			*pPos=p;
			return true;
		}
	}
	return false;
}


void emPsDocument::ParseArgSpaces(const char * * pPos, const char * end)
{
	const char * p;

	p=*pPos;
	while (p<end && ((unsigned char)*p)<=0x20 && *p!=0x0a && *p!=0x0d) p++;
	*pPos=p;
}


bool emPsDocument::ParseIntArg(
	const char * * pPos, const char * end, int * pArg
)
{
	const char * p;
	int s,v;

	ParseArgSpaces(pPos,end);
	p=*pPos;
	if (p<end && *p=='-') {
		s=-1;
		p++;
	}
	else{
		if (p<end && *p=='+') p++;
		s=1;
	}
	if (p>=end || *p<'0' || *p>'9') return false;
	v=*p-'0';
	p++;
	while (p<end && *p>='0' && *p<='9') {
		v=v*10+(*p-'0');
		p++;
	}
	*pArg=v*s;
	*pPos=p;
	return true;
}


bool emPsDocument::ParseDoubleArg(
	const char * * pPos, const char * end, double * pArg
)
{
	char str[64];
	char * se;
	const char * p;
	int l;
	double d;

	ParseArgSpaces(pPos,end);
	p=*pPos;
	for (l=0; p+l<end && l<(int)sizeof(str)-1; l++) {
		str[l]=p[l];
	}
	str[l]=0;
	se=str;
	d=strtod(str,&se);
	if (se<=str) return false;
	*pArg=d;
	*pPos=p+(se-str);
	return true;
}


bool emPsDocument::ParseTextArg(
	const char * * pPos, const char * end, emString * pArg
)
{
	emArray<char> buf;
	const char * p;
	int l,depth;
	char c;

	ParseArgSpaces(pPos,end);
	p=*pPos;

	if (*p!='(') {
		for (l=0; p+l<end && ((unsigned char)p[l])>0x20; l++);
		if (l<=0) return false;
		*pArg=emString(p,l);
		*pPos=p+l;
		return true;
	}

	buf.SetTuningLevel(4);
	depth=1;
	p++;
	for (;;) {
		if (p>=end) return false;
		c=*p++;
		if (c==0x0a || c==0x0d) return false;
		if (c=='(') {
			depth++;
		}
		else if (c==')') {
			depth--;
			if (depth<=0) break;
		}
		else if (c=='\\') {
			if (p>=end) return false;
			c=*p++;
			if (c==0x0a || c==0x0d) return false;
			if (c=='n') c='\n';
			else if (c=='r') c='\r';
			else if (c=='t') c='\t';
			else if (c=='b') c='\b';
			else if (c=='f') c='\f';
			else if (c>='0' && c<='7') {
				c-='0';
				if (p<end && *p>='0' && *p<='7') {
					c=(char)(c*8+((*p++)-'0'));
					if (p<end && *p>='0' && *p<='7') {
						c=(char)(c*8+((*p++)-'0'));
					}
				}
			}
		}
		buf+=c;
	}
	*pArg=emString(buf.Get(),buf.GetCount());
	*pPos=p;
	return true;
}


bool emPsDocument::GetSizeOfStandardPaperType(
	const char * name, double * pW, double * pH
)
{
	static const struct {
		const char * name;
		unsigned short w, h;
	} table[]={
		{       "10x14",  720, 1008 },
		{       "11x17",  792, 1224 },
		{          "a0", 2384, 3370 },
		{         "a10",   73,  105 },
		{          "a1", 1684, 2384 },
		{          "a2", 1191, 1684 },
		{          "a3",  842, 1191 },
		{          "a4",  595,  842 },
		{     "a4small",  595,  842 },
		{          "a5",  420,  595 },
		{          "a6",  297,  420 },
		{          "a7",  210,  297 },
		{          "a8",  148,  210 },
		{          "a9",  105,  148 },
		{       "archa",  648,  864 },
		{       "archb",  864, 1296 },
		{       "archc", 1296, 1728 },
		{       "archd", 1728, 2592 },
		{       "arche", 2592, 3456 },
		{          "b0", 2920, 4127 },
		{          "b1", 2064, 2920 },
		{          "b2", 1460, 2064 },
		{          "b3", 1032, 1460 },
		{          "b4",  729, 1032 },
		{          "b5",  516,  729 },
		{          "b6",  363,  516 },
		{          "c0", 2599, 3677 },
		{          "c1", 1837, 2599 },
		{          "c2", 1298, 1837 },
		{          "c3",  918, 1298 },
		{          "c4",  649,  918 },
		{          "c5",  459,  649 },
		{          "c6",  323,  459 },
		{       "com10",  297,  684 },
		{          "dl",  312,  624 },
		{   "executive",  540,  720 },
		{        "flsa",  612,  936 },
		{        "flse",  612,  936 },
		{       "folio",  612,  936 },
		{  "halfletter",  396,  612 },
		{       "isob0", 2835, 4008 },
		{       "isob1", 2004, 2835 },
		{       "isob2", 1417, 2004 },
		{       "isob3", 1001, 1417 },
		{       "isob4",  709, 1001 },
		{       "isob5",  499,  709 },
		{       "isob6",  354,  499 },
		{       "jisb0", 2920, 4127 },
		{       "jisb1", 2064, 2920 },
		{       "jisb2", 1460, 2064 },
		{       "jisb3", 1032, 1460 },
		{       "jisb4",  729, 1032 },
		{       "jisb5",  516,  729 },
		{       "jisb6",  363,  516 },
		{      "ledger", 1224,  792 },
		{       "legal",  612, 1008 },
		{      "letter",  612,  792 },
		{ "lettersmall",  612,  792 },
		{     "monarch",  279,  540 },
		{        "note",  540,  720 },
		{      "quarto",  610,  780 },
		{   "statement",  396,  612 },
		{     "tabloid",  792, 1224 },
		{          NULL,    0,    0 }
	};
	int i;

	for (i=0; table[i].name; i++) {
		if (strcasecmp(table[i].name,name)==0) {
			*pW=table[i].w;
			*pH=table[i].h;
			return true;
		}
	}
	return false;
}


void emPsDocument::DeleteData()
{
	EmptyData.RefCount=UINT_MAX/2;
	if (Data!=&EmptyData) delete Data;
}


emPsDocument::SharedData::SharedData()
{
	RefCount=1;
	MaxPageWidth=1;
	MaxPageHeight=1;
	StartupLen=0;
	TrailerPos=0;
	TrailerLen=0;
	Adler32=1;
}


emPsDocument::SharedData::~SharedData()
{
}


emPsDocument::SharedData emPsDocument::EmptyData;
