//------------------------------------------------------------------------------
// emTextFileModel.h
//
// Copyright (C) 2004-2008,2010,2014,2017-2018,2023 Oliver Hamann.
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

#ifndef emTextFileModel_h
#define emTextFileModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif


class emTextFileModel : public emFileModel {

public:

	static emRef<emTextFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	// All the following getters may be called by multiple threads
	// concurrently. See emTextFilePanel::Paint(..).

	const emArray<char> & GetContent() const;

	enum CEType {
		CE_BINARY,
		CE_7BIT,
		CE_8BIT,
		CE_UTF8,
		CE_UTF16LE,
		CE_UTF16BE
	};
	CEType GetCharEncoding() const;

	bool IsSameCharEncoding() const;

	int DecodeChar(int * pUcs4, int index, emMBState * mbState) const;

	int ConvertToCurrentLocale(
		char * tgt, int tgtSize,
		const char * * pSrc, const char * srcEnd,
		emMBState * mbState
	) const;

	emString ConvertToCurrentLocale(const char * src, const char * srcEnd) const;

	enum LBEType {
		LBE_NONE,
		LBE_DOS,
		LBE_MAC,
		LBE_UNIX,
		LBE_MIXED
	};
	LBEType GetLineBreakEncoding() const;

	int GetLineCount() const;

	int GetColumnCount() const;

	int GetLineStart(int lineIndex) const;
		// Index to the content: first character of a line.

	int GetLineEnd(int lineIndex) const;
		// Index to the content: one after last character of a line
		// without line feed.

	int ColRow2Index(double column, double row, bool forCursor) const;
	int Index2Row(int index) const;
	void Index2ColRow(int index, int * pColumn, int * pRow) const;
	int GetNextWordBoundaryIndex(int index) const;
	int GetPrevWordBoundaryIndex(int index) const;
	int GetNextRowIndex(int index) const;
	int GetPrevRowIndex(int index) const;

	emUInt8 GetRelativeLineIndent(int lineIndex) const;
	emUInt8 GetRelativeLineWidth(int lineIndex) const;
		// Indent and width of a line in units of ColumnCount/255.

	const emSignal & GetChangeSignal() const;

protected:

	emTextFileModel(emContext & context, const emString & name);
	virtual ~emTextFileModel();
	virtual void ResetData();
	virtual void TryStartLoading();
	virtual bool TryContinueLoading();
	virtual void QuitLoading();
	virtual void TryStartSaving();
	virtual bool TryContinueSaving();
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	emArray<char> Content;

	CEType CharEncoding;
	LBEType LineBreakEncoding;
	int LineCount;
	int ColumnCount;
	int * LineStarts;
	emUInt8 * RelativeLineIndents;
	emUInt8 * RelativeLineWidths;
	emSignal ChangeSignal;

	struct LoadingState {
		int Stage;
		double Progress;
		FILE * File;
		emUInt64 FileSize;
		emUInt64 FileRead;
		char Buf[4096];
		int Statistics[256];
		int StartPos,Pos,Row,Col,Col1,Col2;
		bool FoundCR,FoundLF,FoundCRLF;
		emMBState MBState;
	};
	LoadingState * L;
};

inline const emArray<char> & emTextFileModel::GetContent() const
{
	return Content;
}

inline emTextFileModel::CEType emTextFileModel::GetCharEncoding() const
{
	return CharEncoding;
}

inline emTextFileModel::LBEType emTextFileModel::GetLineBreakEncoding() const
{
	return LineBreakEncoding;
}

inline int emTextFileModel::GetLineCount() const
{
	return LineCount;
}

inline int emTextFileModel::GetColumnCount() const
{
	return ColumnCount;
}

inline int emTextFileModel::GetLineStart(int lineIndex) const
{
	return LineStarts[lineIndex];
}

inline emUInt8 emTextFileModel::GetRelativeLineIndent(int lineIndex) const
{
	return RelativeLineIndents[lineIndex];
}

inline emUInt8 emTextFileModel::GetRelativeLineWidth(int lineIndex) const
{
	return RelativeLineWidths[lineIndex];
}

inline const emSignal & emTextFileModel::GetChangeSignal() const
{
	return ChangeSignal;
}


#endif
