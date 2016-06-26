//------------------------------------------------------------------------------
// emDirModel.h
//
// Copyright (C) 2005-2008,2014,2016 Oliver Hamann.
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

#ifndef emDirModel_h
#define emDirModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emDirEntry_h
#include <emFileMan/emDirEntry.h>
#endif


class emDirModel : public emFileModel {

public:

	static emRef<emDirModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

#if defined(_WIN32)
	static const char * const NAME_OF_DRIVE_LISTING;
#endif

	int GetEntryCount() const;
	const emDirEntry & GetEntry(int index) const;
	int GetEntryIndex(const char * fileName) const;

protected:

	emDirModel(emContext & context, const emString & name);
	virtual ~emDirModel();
	virtual void ResetData();
	virtual void TryStartLoading() throw(emException);
	virtual bool TryContinueLoading() throw(emException);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emException);
	virtual bool TryContinueSaving() throw(emException);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();
	virtual void TryFetchDate() throw(emException);
	virtual bool IsOutOfDate();

private:

	struct NameNode {
		emString Name;
		NameNode * Next;
	};
	struct NamesBlock {
		NamesBlock * Prev;
		enum { MaxNames=1024 };
		NameNode Names[MaxNames];
	};

	void AddName(const emString & name);

	static int CompareName(void * node1, void * node2, void * context);

	emDirHandle DirHandle;
	NamesBlock * CurrentBlock;
	int CurrentBlockFill;
	NameNode * Names;
	int NameCount;
	int EntryCount;
	emDirEntry * Entries;
};

inline int emDirModel::GetEntryCount() const
{
	return EntryCount;
}

inline const emDirEntry & emDirModel::GetEntry(int index) const
{
	return Entries[index];
}


#endif
