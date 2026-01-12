//------------------------------------------------------------------------------
// emJsonException.h
//
// Copyright (C) 2025-2026 Oliver Hamann.
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

#ifndef emJsonException_h
#define emJsonException_h

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emJsonPositionTracker_h
#include <emJson/emJsonPositionTracker.h>
#endif


class emJsonException : public emException {

public:

	emJsonException(
		const emString & sourceName,
		emJsonPosition position,
		const emString & description,
		const emString & lastSourceChars=emString(),
		bool isJsonSyntaxError=false
	);

	emJsonException(const emJsonException & other);

	virtual ~emJsonException();

	emJsonException & operator = (const emJsonException & other);

	void PrependContext(const emString & context);
	void PrependTopLevelContext(const emString & context);

	bool HasToplevelContext() const;

private:

	void UpdateText();
	static void AddFragment(
		emString & text, int & column, const char * str,
		bool forceNewLine=false
	);

	emString SourceName;
	emJsonPosition Position;
	emList<emString> Contexts;
	emString Description;
	emString LastSourceChars;
	bool IsJsonSyntaxError;
	bool TopLevelContextExists;
};


inline bool emJsonException::HasToplevelContext() const
{
	return TopLevelContextExists;
}


#endif
