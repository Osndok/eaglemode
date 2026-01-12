//------------------------------------------------------------------------------
// emFileStream.h
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

#ifndef emFileStream_h
#define emFileStream_h

#ifndef emOwnPtr_h
#include <emCore/emOwnPtr.h>
#endif

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


//==============================================================================
//================================ emFileStream ================================
//==============================================================================

class emFileStream {

public:

	// Class for a FILE* with an additional buffer. That buffer makes
	// reading and writing of many small blocks faster.

	emFileStream(FILE * file=NULL, size_t bufSize=8192);
	~emFileStream();

	void TryOpen(const char * filePath, const char * mode);
	void TryClose();
	void Close();

	bool IsOpen() const;

	emInt64 TryTell();
	void TrySeek(emInt64 pos);
	void TrySeekEnd(emInt64 posFromEnd=0);
	void TrySkip(emInt64 offset);

	void TryRead(void * buf, size_t len);
	size_t TryReadAtMost(void * buf, size_t maxLen);
	emString TryReadLine(bool removeLineBreak=false);
	int TryReadCharOrEOF();
	emInt8 TryReadInt8();
	emUInt8 TryReadUInt8();
	emInt16 TryReadInt16LE();
	emInt16 TryReadInt16BE();
	emUInt16 TryReadUInt16LE();
	emUInt16 TryReadUInt16BE();
	emInt32 TryReadInt32LE();
	emInt32 TryReadInt32BE();
	emUInt32 TryReadUInt32LE();
	emUInt32 TryReadUInt32BE();
	emInt64 TryReadInt64LE();
	emInt64 TryReadInt64BE();
	emUInt64 TryReadUInt64LE();
	emUInt64 TryReadUInt64BE();

	void TryWrite(const void * buf, size_t len);
	void TryWrite(const char * str);
	void TryWriteChar(int value);
	void TryWriteInt8(emInt8 value);
	void TryWriteUInt8(emUInt8 value);
	void TryWriteInt16LE(emInt16 value);
	void TryWriteInt16BE(emInt16 value);
	void TryWriteUInt16LE(emUInt16 value);
	void TryWriteUInt16BE(emUInt16 value);
	void TryWriteInt32LE(emInt32 value);
	void TryWriteInt32BE(emInt32 value);
	void TryWriteUInt32LE(emUInt32 value);
	void TryWriteUInt32BE(emUInt32 value);
	void TryWriteInt64LE(emInt64 value);
	void TryWriteInt64BE(emInt64 value);
	void TryWriteUInt64LE(emUInt64 value);
	void TryWriteUInt64BE(emUInt64 value);

	void TryFlush();

	FILE * TryGetFile();

private:

	void TryFlushAndResetBuffer();
	void TryFlushBuffer();
	const emByte * TryReadBuffered(size_t size);
	emByte * TryWriteBuffered(size_t size);
	emByte * TryFillBufferForReading(size_t minSize);
	emByte * TryPrepareBufferForWriting(size_t minSize);

	FILE * File;
	emInt64 FilePos;
	emOwnArrayPtr<emByte> Buf;
	size_t BufSize;
	emByte * BufPos;
	emByte * BufReadEnd;
	emByte * BufWriteEnd;
};


inline bool emFileStream::IsOpen() const
{
	return File!=NULL;
}

inline int emFileStream::TryReadCharOrEOF()
{
	emByte * p=BufPos;
	if (p>=BufReadEnd) {
		p=TryFillBufferForReading(0);
		if (p>=BufReadEnd) return -1;
	}
	BufPos=p+1;
	return p[0];
}

inline emInt8 emFileStream::TryReadInt8()
{
	return *TryReadBuffered(1);
}

inline emUInt8 emFileStream::TryReadUInt8()
{
	return *TryReadBuffered(1);
}

inline emInt16 emFileStream::TryReadInt16LE()
{
	return TryReadUInt16LE();
}

inline emInt16 emFileStream::TryReadInt16BE()
{
	return TryReadUInt16BE();
}

inline emUInt16 emFileStream::TryReadUInt16LE()
{
	const emByte * p=TryReadBuffered(2);
	return ((emUInt16)p[0]) | (((emUInt16)p[1])<<8);
}

inline emUInt16 emFileStream::TryReadUInt16BE()
{
	const emByte * p=TryReadBuffered(2);
	return (((emUInt16)p[0])<<8) | ((emUInt16)p[1]);
}

inline emInt32 emFileStream::TryReadInt32LE()
{
	return TryReadUInt32LE();
}

inline emInt32 emFileStream::TryReadInt32BE()
{
	return TryReadUInt32BE();
}

inline emInt64 emFileStream::TryReadInt64LE()
{
	return TryReadUInt64LE();
}

inline emInt64 emFileStream::TryReadInt64BE()
{
	return TryReadUInt64BE();
}

inline void emFileStream::TryWrite(const char * str)
{
	TryWrite(str,strlen(str));
}

inline void emFileStream::TryWriteChar(int value)
{
	*TryWriteBuffered(1)=(emByte)value;
}

inline void emFileStream::TryWriteInt8(emInt8 value)
{
	*TryWriteBuffered(1)=value;
}

inline void emFileStream::TryWriteUInt8(emUInt8 value)
{
	*TryWriteBuffered(1)=value;
}

inline void emFileStream::TryWriteInt16LE(emInt16 value)
{
	TryWriteUInt16LE(value);
}

inline void emFileStream::TryWriteInt16BE(emInt16 value)
{
	TryWriteUInt16BE(value);
}

inline void emFileStream::TryWriteUInt16LE(emUInt16 value)
{
	emByte * p=TryWriteBuffered(2);
	p[0]=(emByte)value;
	p[1]=(emByte)(value>>8);
}

inline void emFileStream::TryWriteUInt16BE(emUInt16 value)
{
	emByte * p=TryWriteBuffered(2);
	p[0]=(emByte)(value>>8);
	p[1]=(emByte)value;
}

inline void emFileStream::TryWriteInt32LE(emInt32 value)
{
	TryWriteUInt32LE(value);
}

inline void emFileStream::TryWriteInt32BE(emInt32 value)
{
	TryWriteUInt32BE(value);
}

inline void emFileStream::TryWriteInt64LE(emInt64 value)
{
	TryWriteUInt64LE(value);
}

inline void emFileStream::TryWriteInt64BE(emInt64 value)
{
	TryWriteUInt64BE(value);
}

inline const emByte * emFileStream::TryReadBuffered(size_t size)
{
	emByte * p=BufPos;
	if (p+size>BufReadEnd) p=TryFillBufferForReading(size);
	BufPos=p+size;
	return p;
}

inline emByte * emFileStream::TryWriteBuffered(size_t size)
{
	emByte * p=BufPos;
	if (p+size>BufWriteEnd) p=TryPrepareBufferForWriting(size);
	BufPos=p+size;
	return p;
}


#endif
