//------------------------------------------------------------------------------
// emFontCache.h
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

#ifndef emFontCache_h
#define emFontCache_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emImage_h
#include <emCore/emImage.h>
#endif


class emFontCache : public emModel {

public:

	// This class is only for internal use by emPainter.

	static emRef<emFontCache> Acquire(emRootContext & rootContext);

	void GetChar(
		int unicode, double tgtW, double tgtH, emImage * * ppImg,
		int * pImgX, int * pImgY, int * pImgW, int * pImgH
	);

protected:

	emFontCache(emContext & context, const emString & name);
	virtual ~emFontCache();

private:

	struct RingNode {
		RingNode * Next;
		RingNode * Prev;
	};

	struct Entry : RingNode {
		emString FilePath;
		int FirstCode, LastCode;
		int CharWidth, CharHeight;
		bool Loaded;
		int ColumnCount;
		emUInt64 MemoryNeed;
		emImage Image;
	};

	void LoadEntry(Entry * entry);
	void UnloadEntry(Entry * entry);
	void TouchEntry(Entry * entry);
	void LoadFontDir();
	void Clear();

	emString FontDir;
	emImage ImgUnknownChar;
	emImage ImgCostlyChar;
	Entry * * EntryArray;
	int EntryCount;
	RingNode LRURing;
	double Stress;
	emUInt64 LastLoadTime;
	emUInt64 MemoryUse;

	static const double StressHalfLifePeriodMS;
	static const double StressSensitivity;
	static const unsigned MaxMegabytes;
};


#endif
