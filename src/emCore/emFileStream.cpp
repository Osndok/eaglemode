//------------------------------------------------------------------------------
// emFileStream.cpp
//
// Copyright (C) 2025 Oliver Hamann.
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

#include <emCore/emFileStream.h>


emFileStream::emFileStream(FILE * file, size_t bufSize)
	: File(NULL),
	FilePos(-1),
	BufSize(bufSize),
	BufPos(NULL),
	BufReadEnd(NULL),
	BufWriteEnd(NULL)
{
	EM_ASSERT(BufSize>=8);
	if (file) {
		File=file;
		FilePos=-1;
		Buf=new emByte[BufSize];
		BufPos=Buf;
		BufReadEnd=Buf;
		BufWriteEnd=Buf+BufSize;
	}
}


emFileStream::~emFileStream()
{
	Close();
}


void emFileStream::TryOpen(const char * filePath, const char * mode)
{
	TryClose();
	File=fopen(filePath,mode);
	if (!File) throw emException(
		"Failed to open %s for mode %s: %s",filePath,mode,emGetErrorText(errno).Get()
	);
	FilePos=-1;
	Buf=new emByte[BufSize];
	BufPos=Buf;
	BufReadEnd=Buf;
	BufWriteEnd=Buf+BufSize;
}


void emFileStream::TryClose()
{
	if (!File) return;

	try {
		TryFlush();
	}
	catch (const emException &) {
		fclose(File);
		File=NULL;
		FilePos=-1;
		Buf.Reset();
		BufPos=NULL;
		BufReadEnd=NULL;
		BufWriteEnd=NULL;
		throw;
	}

	int r=fclose(File);
	File=NULL;
	FilePos=-1;
	Buf.Reset();
	BufPos=NULL;
	BufReadEnd=NULL;
	BufWriteEnd=NULL;
	if (r==-1) throw emException("%s",emGetErrorText(errno).Get());
}


void emFileStream::Close()
{
	try {
		TryClose();
	}
	catch (const emException & exception) {
		emWarning("emFileStream::Close() failed: %s",exception.GetText().Get());
	}
}


emInt64 emFileStream::TryTell()
{
	if (FilePos<0) {
		FilePos=ftell(File);
		if (FilePos==-1) throw emException("%s",emGetErrorText(errno).Get());
	}
	emInt64 pos=FilePos;
	if (Buf<BufReadEnd) pos-=BufReadEnd-BufPos;
	else pos+=BufPos-Buf;
	return pos;
}


void emFileStream::TrySeek(emInt64 pos)
{
	TrySkip(pos-TryTell());
}


void emFileStream::TrySeekEnd(emInt64 posFromEnd)
{
	TryFlushBuffer();
	int r=fseek(File,posFromEnd,SEEK_END);
	FilePos=-1;
	BufPos=Buf;
	BufReadEnd=Buf;
	BufWriteEnd=Buf+BufSize;
	if (r==-1) throw emException("%s",emGetErrorText(errno).Get());
}


void emFileStream::TrySkip(emInt64 offset)
{
	if (Buf<BufReadEnd) {
		emByte * newPos=BufPos+offset;
		if (newPos>=Buf && newPos<=BufReadEnd) {
			BufPos=newPos;
			return;
		}
		offset-=BufReadEnd-BufPos;
	}
	else {
		TryFlushBuffer();
	}
	int r=fseek(File,offset,SEEK_CUR);
	if (r==-1) throw emException("%s",emGetErrorText(errno).Get());
	if (FilePos>=0) FilePos+=offset;
	BufPos=Buf;
	BufReadEnd=Buf;
	BufWriteEnd=Buf+BufSize;
}


void emFileStream::TryRead(void * buf, size_t len)
{
	size_t l=TryReadAtMost(buf,len);
	if (l!=len) throw emException("Read beyond end of file");
}


size_t emFileStream::TryReadAtMost(void * buf, size_t maxLen)
{
	size_t resultingLen=BufReadEnd-BufPos;
	if (resultingLen>maxLen) resultingLen=maxLen;
	if (resultingLen>0) {
		memcpy(buf,BufPos,resultingLen);
		BufPos+=resultingLen;
		buf=(char*)buf+resultingLen;
		maxLen-=resultingLen;
	}
	if (maxLen>0) {
		if (maxLen<=BufSize/4) {
			TryFillBufferForReading(0);
			size_t len=BufReadEnd-BufPos;
			if (len>maxLen) len=maxLen;
			memcpy(buf,BufPos,len);
			BufPos+=len;
			resultingLen+=len;
		}
		else {
			TryFlushBuffer();
			size_t r=fread(buf,1,maxLen,File);
			if (ferror(File)) throw emException("%s",emGetErrorText(errno).Get());
			if (FilePos>=0) FilePos+=r;
			resultingLen+=r;
		}
	}
	return resultingLen;
}


emString emFileStream::TryReadLine(bool removeLineBreak)
{
	emByte * p, * q, * r, * e;
	emString str;

	if (BufPos>=BufReadEnd)
		TryFillBufferForReading(0);

	for (;;) {
		p=BufPos;
		e=BufReadEnd;
		if (p==e) return str;
		q=p;
		do {
			if (*q==0x0d || *q==0x0a) goto L_CR_OR_LF_FOUND;
			q++;
		} while (q<e);
		str.Add((const char *)p,e-p);
		BufPos=e;
		TryFillBufferForReading(0);
	}

L_CR_OR_LF_FOUND:
	r=q+1;
	if (*q==0x0d) {
		if (r==e) {
			BufPos=r;
			str.Add((const char *)p,(removeLineBreak?q:r)-p);
			TryFillBufferForReading(0);
			p=q=r=BufPos;
			e=BufReadEnd;
			if (p<e && *p==0x0a) r++;
		}
		else if (*r==0x0a) {
			r++;
		}
	}
	BufPos=r;
	str.Add((const char *)p,(removeLineBreak?q:r)-p);
	return str;
}


emUInt32 emFileStream::TryReadUInt32LE()
{
	const emByte * p=TryReadBuffered(4);
	return
		((emUInt32)p[0]) |
		(((emUInt32)p[1])<<8) |
		(((emUInt32)p[2])<<16) |
		(((emUInt32)p[3])<<24)
	;
}


emUInt32 emFileStream::TryReadUInt32BE()
{
	const emByte * p=TryReadBuffered(4);
	return
		(((emUInt32)p[0])<<24) |
		(((emUInt32)p[1])<<16) |
		(((emUInt32)p[2])<<8) |
		((emUInt32)p[3])
	;
}


emUInt64 emFileStream::TryReadUInt64LE()
{
	const emByte * p=TryReadBuffered(8);
	return
		((emUInt64)p[0]) |
		(((emUInt64)p[1])<<8) |
		(((emUInt64)p[2])<<16) |
		(((emUInt64)p[3])<<24) |
		(((emUInt64)p[4])<<32) |
		(((emUInt64)p[5])<<40) |
		(((emUInt64)p[6])<<48) |
		(((emUInt64)p[7])<<56)
	;
}


emUInt64 emFileStream::TryReadUInt64BE()
{
	const emByte * p=TryReadBuffered(8);
	return
		(((emUInt64)p[0])<<56) |
		(((emUInt64)p[1])<<48) |
		(((emUInt64)p[2])<<40) |
		(((emUInt64)p[3])<<32) |
		(((emUInt64)p[4])<<24) |
		(((emUInt64)p[5])<<16) |
		(((emUInt64)p[6])<<8) |
		((emUInt64)p[7])
	;
}


void emFileStream::TryWrite(const void * buf, size_t len)
{
	if (len==0) return;

	if (Buf<BufPos && BufPos<BufWriteEnd) {
		size_t l=BufWriteEnd-BufPos;
		if (l>len) l=len;
		memcpy(BufPos,buf,l);
		BufPos+=l;
		len-=l;
		if (len==0) return;
		buf=(const char*)buf+l;
	}

	if (len<=BufSize/4) {
		memcpy(TryWriteBuffered(len),buf,len);
	}
	else {
		TryFlushAndResetBuffer();
		size_t r=fwrite(buf,1,len,File);
		if (FilePos>=0) FilePos+=r;
		if (r!=len) throw emException("%s",emGetErrorText(errno).Get());
	}
}


void emFileStream::TryWriteUInt32LE(emUInt32 value)
{
	emByte * p=TryWriteBuffered(4);
	p[0]=(emByte)value;
	p[1]=(emByte)(value>>8);
	p[2]=(emByte)(value>>16);
	p[3]=(emByte)(value>>24);
}


void emFileStream::TryWriteUInt32BE(emUInt32 value)
{
	emByte * p=TryWriteBuffered(4);
	p[0]=(emByte)(value>>24);
	p[1]=(emByte)(value>>16);
	p[2]=(emByte)(value>>8);
	p[3]=(emByte)value;
}


void emFileStream::TryWriteUInt64LE(emUInt64 value)
{
	emByte * p=TryWriteBuffered(8);
	p[0]=(emByte)value;
	p[1]=(emByte)(value>>8);
	p[2]=(emByte)(value>>16);
	p[3]=(emByte)(value>>24);
	p[4]=(emByte)(value>>32);
	p[5]=(emByte)(value>>40);
	p[6]=(emByte)(value>>48);
	p[7]=(emByte)(value>>56);
}


void emFileStream::TryWriteUInt64BE(emUInt64 value)
{
	emByte * p=TryWriteBuffered(8);
	p[0]=(emByte)(value>>56);
	p[1]=(emByte)(value>>48);
	p[2]=(emByte)(value>>40);
	p[3]=(emByte)(value>>32);
	p[4]=(emByte)(value>>24);
	p[5]=(emByte)(value>>16);
	p[6]=(emByte)(value>>8);
	p[7]=(emByte)value;
}


void emFileStream::TryFlush()
{
	TryFlushBuffer();
	if (fflush(File)!=0) throw emException("%s",emGetErrorText(errno).Get());
}


FILE * emFileStream::TryGetFile()
{
	if (File) {
		TryFlushAndResetBuffer();
		FilePos=-1;
	}
	return File;
}


void emFileStream::TryFlushAndResetBuffer()
{
	if (Buf<BufWriteEnd) {
		TryFlushBuffer();
	}
	else {
		emInt64 unread=BufReadEnd-BufPos;
		if (unread>0) {
			int r=fseek(File,-unread,SEEK_CUR);
			if (r==-1) throw emException("%s",emGetErrorText(errno).Get());
			if (FilePos>=0) FilePos-=unread;
		}
		BufPos=Buf;
		BufReadEnd=Buf;
		BufWriteEnd=Buf+BufSize;
	}
}


void emFileStream::TryFlushBuffer()
{
	if (Buf<BufWriteEnd) {
		size_t l=BufPos-Buf;
		if (l!=0) {
			size_t r=fwrite(Buf.Get(),1,l,File);
			if (FilePos>=0) FilePos+=r;
			BufPos=Buf;
			if (r!=l) throw emException("%s",emGetErrorText(errno).Get());
		}
	}
}


emByte * emFileStream::TryFillBufferForReading(size_t minSize)
{
	if (minSize>BufSize) emFatalError("emFileStream: Buffer too small");
	size_t t=BufReadEnd-BufPos;
	if (t>0) memmove(Buf.Get(),BufPos,t);
	else if (Buf<BufWriteEnd) TryFlushBuffer();
	size_t r=fread(Buf.Get()+t,1,BufSize-t,File);
	if (FilePos>=0) FilePos+=r;
	BufPos=Buf;
	BufReadEnd=Buf+t+r;
	BufWriteEnd=Buf;
	if (ferror(File)) throw emException("%s",emGetErrorText(errno).Get());
	if (t+r<minSize) throw emException("Read beyond end of file");
	return BufPos;
}


emByte * emFileStream::TryPrepareBufferForWriting(size_t minSize)
{
	TryFlushAndResetBuffer();
	if (minSize>BufSize) emFatalError("emFileStream: Buffer too small");
	return BufPos;
}
