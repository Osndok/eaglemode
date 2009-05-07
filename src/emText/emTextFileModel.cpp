//------------------------------------------------------------------------------
// emTextFileModel.cpp
//
// Copyright (C) 2004-2009 Oliver Hamann.
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

#include <sys/stat.h>
#include <emText/emTextFileModel.h>


emRef<emTextFileModel> emTextFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emTextFileModel,context,name,common)
}


int emTextFileModel::GetLineEnd(int lineIndex) const
{
	int i, c;

	if (lineIndex+1<LineCount) {
		i=LineStarts[lineIndex+1]-1;
		if (Content[i]==0x0a && i>0 && Content[i-1]==0x0d) i--;
	}
	else {
		i=Content.GetCount();
		if (i>0) {
			c=Content[i-1];
			if (c==0x0d) i--;
			else if (c==0x0a) {
				i--;
				if (i>0 && Content[i-1]==0x0d) i--;
			}
		}
	}
	return i;
}


emTextFileModel::emTextFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	Content.SetTuningLevel(4);
	CharEncoding=CE_BINARY;
	LineBreakEncoding=LBE_NONE;
	LineCount=0;
	ColumnCount=0;
	LineStarts=NULL;
	RelativeLineIndents=NULL;
	RelativeLineWidths=NULL;
	L=NULL;
}


emTextFileModel::~emTextFileModel()
{
	emTextFileModel::QuitLoading();
	emTextFileModel::ResetData();
}


void emTextFileModel::ResetData()
{
	Content.Empty(true);
	CharEncoding=CE_BINARY;
	LineBreakEncoding=LBE_NONE;
	LineCount=0;
	ColumnCount=0;
	if (LineStarts) {
		delete [] LineStarts;
		LineStarts=NULL;
	}
	if (RelativeLineIndents) {
		delete [] RelativeLineIndents;
		RelativeLineIndents=NULL;
	}
	if (RelativeLineWidths) {
		delete [] RelativeLineWidths;
		RelativeLineWidths=NULL;
	}
}


void emTextFileModel::TryStartLoading() throw(emString)
{
	emInt64 l;

	L=new LoadingState;
	L->Stage=0;
	L->Progress=0.0;
	L->File=NULL;
	L->FileSize=0;
	L->FileRead=0;

	L->File=fopen(GetFilePath(),"rb");
	if (!L->File) goto Err;
	l=fseek(L->File,0,SEEK_END);
	if (l) goto Err;
	l=ftell(L->File);
	if (l<0) goto Err;
	L->FileSize=l;
	l=fseek(L->File,0,SEEK_SET);
	if (l) goto Err;
	return;

Err:
	throw emGetErrorText(errno);
}


bool emTextFileModel::TryContinueLoading() throw(emString)
{
	const char * p;
	int * s;
	int i,end,cnt,len,sa,sb,n,c,cTestUtf8,row,col,col1,col2;
	struct stat st;

	switch (L->Stage) {
	case 0:
		// Set size of Content.
		if (L->FileSize>(emUInt64)INT_MAX) throw emString("File too large.");
		Content.SetCount((int)L->FileSize,true);
		L->Stage=1;
		break;
	case 1:
		// Load up to FileSize bytes.
		len=(int)(L->FileSize-L->FileRead);
		if (len>4096) len=4096;
		len=(int)fread(Content.GetWritable()+(size_t)L->FileRead,1,len,L->File);
		if (len>0) {
			L->FileRead+=len;
		}
		else {
			L->FileSize=L->FileRead;
			Content.SetCount((int)L->FileRead,true);
			L->Stage=2;
		}
		L->Progress=75.0*L->FileRead/L->FileSize;
		break;
	case 2:
		// ??? hack/bug/workaround...
		// Decide whether to read beyond the end of the file. This is a
		// good idea for many files in the Linux proc file system,
		// because they have zero size while containing interesting
		// information. But there is at least one file which we should
		// not read: "/proc/kmsg". This file acts like an infinite read
		// pipe. In my opinion, the kernel should not report that file
		// as a regular file (and it should report the correct size for
		// all the other proc files, but I know this would be
		// difficult). Okay, the workaround here is: Read beyond the end
		// of the file only if size is zero and reading is allowed to
		// everyone.
		if (
			L->FileSize==0 &&
			stat(GetFilePath().Get(),&st)==0 &&
			(st.st_mode&0444)==0444
		) {
			L->Stage=3;
		}
		else {
			L->Stage=4;
		}
		break;
	case 3:
		// Read beyond the end of the file.
		len=fread(L->Buf,1,sizeof(L->Buf),L->File);
		if (len>0) {
			if (((emUInt64)Content.GetCount())+((emUInt64)len)>(emUInt64)INT_MAX) {
				throw emString("file too large");
			}
			Content.Add(L->Buf,len,true);
			L->FileRead+=len;
			L->FileSize=L->FileRead;
		}
		else {
			L->Stage=4;
		}
		break;
	case 4:
		// Prepare for character code statistics.
		memset(L->Statistics,0,sizeof(L->Statistics));
		L->Pos=0;
		L->Stage=5;
		break;
	case 5:
		// Create character code statistics.
		i=L->Pos;
		cnt=Content.GetCount();
		if (i<cnt) {
			s=L->Statistics;
			p=Content.Get();
			end=emMin(i+10000,cnt);
			for (; i<end; i++) s[(unsigned char)p[i]]++;
			L->Pos=i;
		}
		else {
			L->Stage=6;
		}
		L->Progress=75.0+5.0*L->Pos/Content.GetCount();
		break;
	case 6:
		// Test if binary or 7-bit. Otherwise prepare for UTF-8 detection.
		s=L->Statistics;
		sa=s[0]+s[1]+s[2]+s[3]+s[4]+s[5];
		sb=
			s[6]+s[14]+s[15]+s[16]+s[17]+s[18]+s[19]+s[20]+s[21]+s[22]+
			s[23]+s[24]+s[25]+s[26]+s[27]+s[28]+s[29]+s[30]+s[31]+s[127]
		;
		if (sa>0 || sb>Content.GetCount()/1000) {
			CharEncoding=CE_BINARY;
			L->Stage=14;
		}
		else {
			for (sa=0, i=128; i<256; i++) sa+=s[i];
			if (sa==0) {
				CharEncoding=CE_7BIT;
				L->Stage=8;
			}
			else {
				L->Stage=7;
				L->Pos=0;
			}
		}
		break;
	case 7:
		// UTF-8 detection.
		i=L->Pos;
		cnt=Content.GetCount();
		if (i<cnt) {
			p=Content.Get();
			end=emMin(i+10000,cnt);
			for (; i<end; i++) {
				if (((signed char)p[i])<0) {
					n=emDecodeUtf8Char(&c,p+i,cnt-i);
					n--;
					if (n<0) break;
					i+=n;
				}
			}
			if (i<end) {
				CharEncoding=CE_8BIT;
				L->Stage=8;
			}
			L->Pos=i;
		}
		else {
			CharEncoding=CE_UTF8;
			L->Stage=8;
		}
		L->Progress=80.0+5.0*L->Pos/Content.GetCount();
		break;
	case 8:
		// Prepare for detecting line breaks.
		LineCount=0;
		L->FoundCR=false;
		L->FoundLF=false;
		L->FoundCRLF=false;
		L->Pos=0;
		L->Stage=9;
		break;
	case 9:
		// Detect line breaks.
		i=L->Pos;
		p=Content.Get();
		cnt=Content.GetCount();
		if (i<cnt) {
			end=emMin(i+10000,cnt);
			for (; i<end; i++) {
				c=(unsigned char)p[i];
				if (c<=0x0d) {
					if (c==0x0d) {
						if (i+1<cnt && p[i+1]==0x0a) {
							L->FoundCRLF=true;
							i++;
						}
						else {
							L->FoundCR=true;
						}
						LineCount++;
					}
					else if (c==0x0a) {
						L->FoundLF=true;
						LineCount++;
					}
				}
			}
			L->Pos=i;
		}
		else {
			if (cnt>0 && p[cnt-1]!=0x0d && p[cnt-1]!=0x0a) LineCount++;
			if (!L->FoundCR && !L->FoundLF && !L->FoundCRLF) LineBreakEncoding=LBE_NONE;
			else if (L->FoundCR && !L->FoundLF && !L->FoundCRLF) LineBreakEncoding=LBE_MAC;
			else if (!L->FoundCR && L->FoundLF && !L->FoundCRLF) LineBreakEncoding=LBE_UNIX;
			else if (!L->FoundCR && !L->FoundLF && L->FoundCRLF) LineBreakEncoding=LBE_DOS;
			else LineBreakEncoding=LBE_MIXED;
			L->Stage=10;
		}
		L->Progress=85.0+5.0*L->Pos/Content.GetCount();
		break;
	case 10:
		// Prepare for detecting line starts and column count.
		LineStarts=new int[LineCount];
		if (LineCount>0) LineStarts[0]=0;
		L->Pos=0;
		L->Col=0;
		L->Row=1;
		L->Stage=11;
		break;
	case 11:
		// Detect line starts and column count.
		i=L->Pos;
		cnt=Content.GetCount();
		col=L->Col;
		row=L->Row;
		if (i<cnt) {
			cTestUtf8 = CharEncoding==CE_UTF8 ? 128 : 256;
			p=Content.Get();
			end=emMin(i+10000,cnt);
			for (; i<end; i++) {
				c=(unsigned char)p[i];
				if (c<=0x0d) {
					if (c==0x09) {
						col=(col+8)&~7;
					}
					else if (c==0x0a) {
						if (ColumnCount<col) ColumnCount=col;
						col=0;
						if (row<LineCount) LineStarts[row++]=i+1;
					}
					else if (c==0x0d) {
						if (ColumnCount<col) ColumnCount=col;
						col=0;
						if (i+1<cnt && p[i+1]==0x0a) i++;
						if (row<LineCount) LineStarts[row++]=i+1;
					}
					else {
						col++;
					}
				}
				else if (c>=cTestUtf8) {
					n=emDecodeUtf8Char(&c,p+i,cnt-i);
					if (n>1) i+=n-1;
					col++;
				}
				else {
					col++;
				}
			}
			L->Pos=i;
		}
		else {
			if (ColumnCount<col) ColumnCount=col;
			L->Stage=12;
		}
		L->Col=col;
		L->Row=row;
		L->Progress=90.0+5.0*L->Pos/Content.GetCount();
		break;
	case 12:
		// Prepare for calculating relative line indents and widths.
		RelativeLineIndents=new emUInt8[LineCount];
		RelativeLineWidths=new emUInt8[LineCount];
		L->Pos=0;
		L->Row=0;
		L->Col=0;
		L->Col1=ColumnCount;
		L->Col2=0;
		L->Stage=13;
		break;
	case 13:
		// Calculate relative line indents and widths.
		i=L->Pos;
		cnt=Content.GetCount();
		row=L->Row;
		col=L->Col;
		col1=L->Col1;
		col2=L->Col2;
		if (i<cnt) {
			cTestUtf8 = CharEncoding==CE_UTF8 ? 128 : 256;
			p=Content.Get();
			end=emMin(i+10000,cnt);
			for (; i<end; i++) {
				c=(unsigned char)p[i];
				if (c<=0x20) {
					if (c==0x09) {
						col=(col+8)&~7;
					}
					else if (c==0x0a) {
						if (row<LineCount) {
							if (col1>col) col1=col;
							if (col2<col1) col2=col1;
							RelativeLineIndents[row]=(emUInt8)(col1*256/(ColumnCount+1));
							RelativeLineWidths[row]=(emUInt8)((col2-col1)*256/(ColumnCount+1));
							row++;
						}
						col=0;
						col1=ColumnCount;
						col2=0;
					}
					else if (c==0x0d) {
						if (i+1<cnt && p[i+1]==0x0a) i++;
						if (row<LineCount) {
							if (col1>col) col1=col;
							if (col2<col1) col2=col1;
							RelativeLineIndents[row]=(emUInt8)(col1*256/(ColumnCount+1));
							RelativeLineWidths[row]=(emUInt8)((col2-col1)*256/(ColumnCount+1));
							row++;
						}
						col=0;
						col1=ColumnCount;
						col2=0;
					}
					else {
						col++;
					}
				}
				else {
					if (col1>col) col1=col;
					if (c>=cTestUtf8) {
						n=emDecodeUtf8Char(&c,p+i,cnt-i);
						if (n>1) i+=n-1;
					}
					col++;
					col2=col;
				}
			}
			L->Pos=i;
		}
		else {
			if (row<LineCount) {
				if (col1>col) col1=col;
				if (col2<col1) col2=col1;
				RelativeLineIndents[row]=(emUInt8)(col1*256/(ColumnCount+1));
				RelativeLineWidths[row]=(emUInt8)((col2-col1)*256/(ColumnCount+1));
			}
			L->Stage=14;
		}
		L->Row=row;
		L->Col=col;
		L->Col1=col1;
		L->Col2=col2;
		L->Progress=95.0+5.0*L->Pos/Content.GetCount();
		break;
	case 14:
		// Finished.
		L->Progress=100.0;
		return true;
	}
	return false;
}


void emTextFileModel::QuitLoading()
{
	if (L) {
		if (L->File) fclose(L->File);
		delete L;
		L=NULL;
	}
}


void emTextFileModel::TryStartSaving() throw(emString)
{
	throw emString("emTextFileModel: Saving not supported.");
}


bool emTextFileModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emTextFileModel::QuitSaving()
{
}


emUInt64 emTextFileModel::CalcMemoryNeed()
{
	emUInt64 m;

	if (L) m=L->FileSize; else m=Content.GetCount();

	if (CharEncoding!=CE_BINARY) {
		m+=6*LineCount;
	}

	return m;
}


double emTextFileModel::CalcFileProgress()
{
	return L ? L->Progress : 0.0;
}
