//------------------------------------------------------------------------------
// emString.cpp
//
// Copyright (C) 2004-2008,2011 Oliver Hamann.
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

#include <emCore/emString.h>


#ifdef EM_NO_DATA_EXPORT
emString::emString()
{
	Data=&EmptyData;
}
#endif


emString::emString(const char * p)
{
	int len;

	if (p && (len=strlen(p))>0) {
		Data=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		Data->RefCount=1;
		memcpy(Data->Buf,p,len+1);
	}
	else {
		Data=&EmptyData;
	}
}


emString::emString(const char * p, int len)
{
	if (p && len>0) {
		Data=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		Data->RefCount=1;
		memcpy(Data->Buf,p,len);
		Data->Buf[len]=0;
	}
	else {
		Data=&EmptyData;
	}
}


emString::emString(const char * p, int len, const char * p2, int len2)
{
	int tl;

	if (!p || len<0) len=0;
	if (!p2 || len2<0) len2=0;
	tl=len+len2;
	if (tl>0) {
		Data=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+tl);
		Data->RefCount=1;
		memcpy(Data->Buf,p,len);
		memcpy(Data->Buf+len,p2,len2);
		Data->Buf[tl]=0;
	}
	else {
		Data=&EmptyData;
	}
}


emString::emString(char c, int len)
{
	if (len>0) {
		Data=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		Data->RefCount=1;
		memset(Data->Buf,c,len);
		Data->Buf[len]=0;
	}
	else {
		Data=&EmptyData;
	}
}


emString emString::Format(const char * format, ...)
{
	SharedData * d;
	char autobuf[2048];
	char * buf;
	va_list args;
	int sz,len;

	buf=autobuf;
	sz=sizeof(autobuf);
	for (;;) {
		va_start(args,format);
		len=vsnprintf(buf,sz,format,args);
		va_end(args);
		if (len>=0 && len<sz) break;
		if (buf!=autobuf) free(buf);
		sz*=2;
		buf=(char*)malloc(sz);
	}
	if (len>0) {
		d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		d->RefCount=1;
		memcpy(d->Buf,buf,len+1);
	}
	else {
		d=&EmptyData;
	}
	if (buf!=autobuf) free(buf);
	return emString(d);
}


emString & emString::operator = (const char * p)
{
	int oldLen;

	if (p && *p) {
		oldLen=strlen(Data->Buf);
		PrivRep(oldLen,0,oldLen,p,strlen(p));
	}
	else {
		if (!--Data->RefCount) FreeData();
		Data=&EmptyData;
	}
	return *this;
}


emString & emString::operator = (char c)
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	PrivRep(oldLen,0,oldLen,c,1);
	return *this;
}


char * emString::SetLenGetWritable(int len)
{
	SharedData * d;
	int oldLen;

	if (len<0) len=0;
	oldLen=strlen(Data->Buf);
	if (Data->RefCount>1) {
		d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		if (oldLen>len) oldLen=len;
		if (oldLen>0) memcpy(d->Buf,Data->Buf,oldLen);
		d->Buf[oldLen]=0;
		d->Buf[len]=0;
		d->RefCount=1;
		Data->RefCount--;
		Data=d;
	}
	else if (len!=oldLen) {
		Data=(SharedData*)realloc(
			Data,
			sizeof(SharedData)-sizeof(unsigned int)+1+len
		);
		Data->Buf[len]=0;
	}
	return Data->Buf;
}


void emString::Add(const emString & s)
{
	int oldLen,len;

	oldLen=strlen(Data->Buf);
	if (oldLen) {
		len=strlen(s.Data->Buf);
		if (len>0) PrivRep(oldLen,oldLen,0,s.Data->Buf,len);
	}
	else {
		s.Data->RefCount++;
		if (!--Data->RefCount) FreeData();
		Data=s.Data;
	}
}


void emString::Add(const char * p)
{
	int oldLen;

	if (p && *p) {
		oldLen=strlen(Data->Buf);
		PrivRep(oldLen,oldLen,0,p,strlen(p));
	}
}


void emString::Add(const char * p, int len)
{
	int oldLen;

	if (p && len>0) {
		oldLen=strlen(Data->Buf);
		PrivRep(oldLen,oldLen,0,p,len);
	}
}


void emString::Add(char c, int len)
{
	int oldLen;

	if (len>0) {
		oldLen=strlen(Data->Buf);
		PrivRep(oldLen,oldLen,0,c,len);
	}
}


emString emString::operator + (const emString & s) const
{
	int l1,l2;

	l1=strlen(Data->Buf);
	if (!l1) return s;
	l2=strlen(s.Data->Buf);
	if (!l2) return *this;
	return emString(Data->Buf,l1,s.Data->Buf,l2);
}


emString emString::operator + (const char * p) const
{
	if (!p || !*p) return *this;
	return emString(Data->Buf,strlen(Data->Buf),p,strlen(p));
}


emString emString::operator + (char c) const
{
	return emString(Data->Buf,strlen(Data->Buf),&c,1);
}


emString operator + (const char * p, const emString & s)
{
	if (!p || !*p) return s;
	return emString(p,strlen(p),s.Data->Buf,strlen(s.Data->Buf));
}


emString operator + (char c, const emString & s)
{
	return emString(&c,1,s.Data->Buf,strlen(s.Data->Buf));
}


void emString::Insert(int index, const emString & s)
{
	int oldLen,len;

	oldLen=strlen(Data->Buf);
	if (oldLen) {
		len=strlen(s.Data->Buf);
		if (len>0) {
			if ((unsigned int)index>(unsigned int)oldLen) {
				if (index<0) index=0; else index=oldLen;
			}
			PrivRep(oldLen,index,0,s.Data->Buf,len);
		}
	}
	else {
		s.Data->RefCount++;
		if (!--Data->RefCount) FreeData();
		Data=s.Data;
	}
}


void emString::Insert(int index, const char * p)
{
	int oldLen;

	if (p && *p) {
		oldLen=strlen(Data->Buf);
		if ((unsigned int)index>(unsigned int)oldLen) {
			if (index<0) index=0; else index=oldLen;
		}
		PrivRep(oldLen,index,0,p,strlen(p));
	}
}


void emString::Insert(int index, const char * p, int len)
{
	int oldLen;

	if (p && len>0) {
		oldLen=strlen(Data->Buf);
		if ((unsigned int)index>(unsigned int)oldLen) {
			if (index<0) index=0; else index=oldLen;
		}
		PrivRep(oldLen,index,0,p,len);
	}
}


void emString::Insert(int index, char c, int len)
{
	int oldLen;

	if (len>0) {
		oldLen=strlen(Data->Buf);
		if ((unsigned int)index>(unsigned int)oldLen) {
			if (index<0) index=0; else index=oldLen;
		}
		PrivRep(oldLen,index,0,c,len);
	}
}


void emString::Replace(int index, int exLen, const emString & s)
{
	int oldLen,len;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { exLen+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)exLen>(unsigned int)(oldLen-index)) {
		if (exLen<0) exLen=0; else exLen=oldLen-index;
	}
	if (exLen==oldLen) {
		s.Data->RefCount++;
		if (!--Data->RefCount) FreeData();
		Data=s.Data;
	}
	else {
		len=strlen(s.Data->Buf);
		if (exLen || len) PrivRep(oldLen,index,exLen,s.Data->Buf,len);
	}
}


void emString::Replace(int index, int exLen, const char * p)
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { exLen+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)exLen>(unsigned int)(oldLen-index)) {
		if (exLen<0) exLen=0; else exLen=oldLen-index;
	}
	if (p && *p) PrivRep(oldLen,index,exLen,p,strlen(p));
	else if (exLen) PrivRep(oldLen,index,exLen,'\0',0);
}


void emString::Replace(int index, int exLen, const char * p, int len)
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { exLen+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)exLen>(unsigned int)(oldLen-index)) {
		if (exLen<0) exLen=0; else exLen=oldLen-index;
	}
	if (p && len>0) PrivRep(oldLen,index,exLen,p,len);
	else if (exLen) PrivRep(oldLen,index,exLen,'\0',0);
}


void emString::Replace(int index, int exLen, char c, int len)
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { exLen+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)exLen>(unsigned int)(oldLen-index)) {
		if (exLen<0) exLen=0; else exLen=oldLen-index;
	}
	if (len>0) PrivRep(oldLen,index,exLen,c,len);
	else if (exLen) PrivRep(oldLen,index,exLen,c,0);
}


emString emString::GetSubString(int index, int len) const
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { len+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)len>(unsigned int)(oldLen-index)) {
		if (len<0) len=0; else len=oldLen-index;
	}
	if (len==oldLen) return *this;
	else return emString(Data->Buf+index,len);
}


emString emString::Extract(int index, int len)
{
	SharedData * d;
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { len+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)len>(unsigned int)(oldLen-index)) {
		if (len<0) len=0; else len=oldLen-index;
	}
	if (!len) {
		d=&EmptyData;
	}
	else if (len==oldLen) {
		d=Data;
		Data=&EmptyData;
	}
	else {
		d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
		d->RefCount=1;
		memcpy(d->Buf,Data->Buf+index,len);
		d->Buf[len]=0;
		PrivRep(oldLen,index,len,'\0',0);
	}
	return emString(d);
}


void emString::Remove(int index, int len)
{
	int oldLen;

	oldLen=strlen(Data->Buf);
	if ((unsigned int)index>(unsigned int)oldLen) {
		if (index<0) { len+=index; index=0; } else index=oldLen;
	}
	if ((unsigned int)len>(unsigned int)(oldLen-index)) {
		if (len<0) len=0; else len=oldLen-index;
	}
	if (len) PrivRep(oldLen,index,len,'\0',0);
}


#ifdef EM_NO_DATA_EXPORT
void emString::Empty()
{
	if (!--Data->RefCount) FreeData();
	Data=&EmptyData;
}
#endif


unsigned int emString::GetDataRefCount() const
{
	return Data==&EmptyData ? UINT_MAX/2 : Data->RefCount;
}


#if defined(ANDROID)
void emString::VirtualDummyMethod()
{
}
#endif


void emString::FreeData()
{
	EmptyData.RefCount=UINT_MAX/2;
	if (Data!=&EmptyData) free(Data);
}


void emString::MakeWritable()
{
	SharedData * d;
	int len;

	if (Data->RefCount>1) {
		len=strlen(Data->Buf);
		if (len<=0) {
			d=&EmptyData;
		}
		else {
			d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+len);
			memcpy(d->Buf,Data->Buf,len+1);
			d->RefCount=1;
		}
		Data->RefCount--;
		Data=d;
	}
}


void emString::PrivRep(int oldLen, int index, int exLen, const char * p, int len)
{
	SharedData * d;
	int newLen;

	newLen=oldLen-exLen+len;
	if (newLen<=0) {
		if (!--Data->RefCount) FreeData();
		Data=&EmptyData;
	}
	else if (Data->RefCount>1) {
		d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+newLen);
		if (index>0) memcpy(d->Buf,Data->Buf,index);
		if (len>0) memcpy(d->Buf+index,p,len);
		memcpy(d->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
		d->RefCount=1;
		Data->RefCount--;
		Data=d;
	}
	else if (newLen<=oldLen) {
		if (len>0) memmove(Data->Buf+index,p,len);
		if (newLen<oldLen) {
			memmove(Data->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
			Data=(SharedData*)realloc(
				Data,
				sizeof(SharedData)-sizeof(unsigned int)+1+newLen
			);
		}
	}
	else if (p<Data->Buf || p>Data->Buf+oldLen) {
		Data=(SharedData*)realloc(
			Data,
			sizeof(SharedData)-sizeof(unsigned int)+1+newLen
		);
		memmove(Data->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
		memcpy(Data->Buf+index,p,len);
	}
	else {
		d=(SharedData*)realloc(
			Data,
			sizeof(SharedData)-sizeof(unsigned int)+1+newLen
		);
		p+=((char*)d)-((char*)Data);
		if (p<=d->Buf+index) {
			memmove(d->Buf+index+len,d->Buf+index+exLen,oldLen-index-exLen+1);
			if (p!=d->Buf+index) memmove(d->Buf+index,p,len);
		}
		else {
			if (exLen>0) memmove(d->Buf+index,p,exLen);
			memmove(d->Buf+index+len,d->Buf+index+exLen,oldLen-index-exLen+1);
			memcpy(d->Buf+index+exLen,p+len,len-exLen);
		}
		Data=d;
	}
}


void emString::PrivRep(int oldLen, int index, int exLen, char c, int len)
{
	SharedData * d;
	int newLen;

	newLen=oldLen-exLen+len;
	if (newLen<=0) {
		if (!--Data->RefCount) FreeData();
		Data=&EmptyData;
	}
	else if (Data->RefCount>1) {
		d=(SharedData*)malloc(sizeof(SharedData)-sizeof(unsigned int)+1+newLen);
		if (index>0) memcpy(d->Buf,Data->Buf,index);
		if (len>0) memset(d->Buf+index,c,len);
		memcpy(d->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
		d->RefCount=1;
		Data->RefCount--;
		Data=d;
	}
	else if (newLen<=oldLen) {
		if (len>0) memset(Data->Buf+index,c,len);
		if (newLen<oldLen) {
			memmove(Data->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
			Data=(SharedData*)realloc(
				Data,
				sizeof(SharedData)-sizeof(unsigned int)+1+newLen
			);
		}
	}
	else {
		Data=(SharedData*)realloc(
			Data,
			sizeof(SharedData)-sizeof(unsigned int)+1+newLen
		);
		memmove(Data->Buf+index+len,Data->Buf+index+exLen,oldLen-index-exLen+1);
		memset(Data->Buf+index,c,len);
	}
}


emString::SharedData emString::EmptyData = {UINT_MAX/2,""};
