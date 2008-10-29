//------------------------------------------------------------------------------
// emPsDocument.h
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

#ifndef emPsDocument_h
#define emPsDocument_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


class emPsDocument {

public:

	emPsDocument();
	emPsDocument(const emPsDocument & doc);
	~emPsDocument();

	emPsDocument & operator = (const emPsDocument & doc);

	bool operator == (const emPsDocument & doc) const;
	bool operator != (const emPsDocument & doc) const;

	void Empty();

	void TrySetScript(const emArray<char> & script) throw(emString);
		// Set up the document from the given content of a PostScript
		// file. It must follow the PostScript Document Structuring
		// Conventions (the DSC comments are parsed by this method).

	const emArray<char> & GetScript() const;

	int GetPageCount() const;

	double GetMaxPageWidth() const;
	double GetMaxPageHeight() const;

	double GetPageWidth(int pageIndex) const;
	double GetPageHeight(int pageIndex) const;

	const emString & GetPageLabel(int pageIndex) const;

	bool IsLandscapePage(int pageIndex) const;
		// If so, width and height are already swapped.

	const char * GetStartupScriptPtr() const;
	int GetStartupScriptLen() const;

	const char * GetPageScriptPtr(int pageIndex) const;
	int GetPageScriptLen(int pageIndex) const;

	const char * GetTrailerScriptPtr() const;
	int GetTrailerScriptLen() const;

	emUInt64 CalcMemoryNeed() const;

	unsigned int GetDataRefCount() const;

private:

	static void ParseToNextLine(const char * * pPos, const char * end);

	static bool ParseExactly(const char * * pPos, const char * end,
	                         const char * str);

	static void ParseArgSpaces(const char * * pPos, const char * end);

	static bool ParseIntArg(const char * * pPos, const char * end,
	                        int * pArg);

	static bool ParseDoubleArg(const char * * pPos, const char * end,
	                           double * pArg);

	static bool ParseTextArg(const char * * pPos, const char * end,
	                         emString * pArg);

	static bool GetSizeOfStandardPaperType(const char * name, double * pW,
	                                       double * pH);

	void DeleteData();

	struct PageInfo {
		int Pos,Len;
		unsigned IsLandscape : 1;
		unsigned OrientationFound : 1;
		unsigned SizeFound : 1;
		double Width,Height;
		emString Label;
	};

	struct SharedData {
		SharedData();
		~SharedData();
		unsigned int RefCount;
		emArray<char> Script;
		emArray<PageInfo> Pages;
		double MaxPageWidth,MaxPageHeight;
		int StartupLen,TrailerPos,TrailerLen;
		emUInt32 Adler32;
	};

	SharedData * Data;

	static SharedData EmptyData;
};

inline bool emPsDocument::operator != (const emPsDocument & doc) const
{
	return !(*this==doc);
}

inline const emArray<char> & emPsDocument::GetScript() const
{
	return Data->Script;
}

inline int emPsDocument::GetPageCount() const
{
	return Data->Pages.GetCount();
}

inline double emPsDocument::GetMaxPageWidth() const
{
	return Data->MaxPageWidth;
}

inline double emPsDocument::GetMaxPageHeight() const
{
	return Data->MaxPageHeight;
}

inline double emPsDocument::GetPageWidth(int pageIndex) const
{
	return Data->Pages[pageIndex].Width;
}

inline double emPsDocument::GetPageHeight(int pageIndex) const
{
	return Data->Pages[pageIndex].Height;
}

inline const emString & emPsDocument::GetPageLabel(int pageIndex) const
{
	return Data->Pages[pageIndex].Label;
}

inline bool emPsDocument::IsLandscapePage(int pageIndex) const
{
	return Data->Pages[pageIndex].IsLandscape;
}

inline const char * emPsDocument::GetStartupScriptPtr() const
{
	return Data->Script.Get();
}

inline int emPsDocument::GetStartupScriptLen() const
{
	return Data->StartupLen;
}

inline const char * emPsDocument::GetPageScriptPtr(int pageIndex) const
{
	return Data->Script.Get()+Data->Pages[pageIndex].Pos;
}

inline int emPsDocument::GetPageScriptLen(int pageIndex) const
{
	return Data->Pages[pageIndex].Len;
}

inline const char * emPsDocument::GetTrailerScriptPtr() const
{
	return Data->Script.Get()+Data->TrailerPos;
}

inline int emPsDocument::GetTrailerScriptLen() const
{
	return Data->TrailerLen;
}


#endif
