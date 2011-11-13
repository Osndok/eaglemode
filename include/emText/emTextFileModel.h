//------------------------------------------------------------------------------
// emTextFileModel.h
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

	emUInt8 GetRelativeLineIndent(int lineIndex) const;
	emUInt8 GetRelativeLineWidth(int lineIndex) const;
		// Indent and width of a line in units of ColumnCount/255.

protected:

	emTextFileModel(emContext & context, const emString & name);
	virtual ~emTextFileModel();
	virtual void ResetData();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
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


#endif
