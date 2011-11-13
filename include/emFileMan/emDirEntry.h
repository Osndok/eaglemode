//------------------------------------------------------------------------------
// emDirEntry.h
//
// Copyright (C) 2005-2008,2010-2011 Oliver Hamann.
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

#ifndef emDirEntry_h
#define emDirEntry_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif


class emDirEntry {

public:

	emDirEntry();
	emDirEntry(const emString & path);
	emDirEntry(const emString & parentPath, const emString & name);
	emDirEntry(const emDirEntry & dirEntry);
	~emDirEntry();

	emDirEntry & operator = (const emDirEntry & dirEntry);

	bool operator == (const emDirEntry & dirEntry) const;
	bool operator != (const emDirEntry & dirEntry) const;

	void Load(const emString & path);
	void Load(const emString & parentPath, const emString & name);
	void Clear();

	const emString & GetPath() const;
	const emString & GetName() const;
	const emString & GetTargetPath() const;

	bool IsSymbolicLink() const;
	bool IsDirectory() const;
	bool IsRegularFile() const;
	bool IsHidden() const;

	const struct em_stat * GetStat() const;
	const struct em_stat * GetLStat() const;

	const emString & GetOwner() const;
	const emString & GetGroup() const;

	int GetTargetPathErrNo() const;
	int GetStatErrNo() const;
	int GetLStatErrNo() const;

#if defined(_WIN32)
	emUInt32 GetWndsFileAttributes() const;
#endif

	unsigned int GetDataRefCount() const;

private:

	void PrivLoad(const emString & path, const emString & name);
	void FreeData();

	struct SharedData {
		SharedData();
		~SharedData();
		unsigned int RefCount;
		int StatErrNo;
		int LStatErrNo;
		int TargetPathErrNo;
		emString Path;
		emString Name;
		emString TargetPath;
		emString Owner;
		emString Group;
		bool Hidden;
		struct em_stat Stat;
		struct em_stat * LStat;
#if defined(_WIN32)
		emUInt32 WndsFileAttributes;
#endif
	};

	SharedData * Data;
	static SharedData EmptyData;
};

inline bool emDirEntry::operator != (const emDirEntry & dirEntry) const
{
	return !(*this==dirEntry);
}

inline const emString & emDirEntry::GetPath() const
{
	return Data->Path;
}

inline const emString & emDirEntry::GetName() const
{
	return Data->Name;
}

inline const emString & emDirEntry::GetTargetPath() const
{
	return Data->TargetPath;
}

inline bool emDirEntry::IsSymbolicLink() const
{
#	if defined(_WIN32)
		return false;
#	else
		return S_ISLNK(Data->LStat->st_mode)!=0;
#	endif
}

inline bool emDirEntry::IsDirectory() const
{
	return (Data->Stat.st_mode&S_IFMT)==S_IFDIR;
}

inline bool emDirEntry::IsRegularFile() const
{
	return (Data->Stat.st_mode&S_IFMT)==S_IFREG;
}

inline bool emDirEntry::IsHidden() const
{
	return Data->Hidden;
}

inline const struct em_stat * emDirEntry::GetStat() const
{
	return &Data->Stat;
}

inline const struct em_stat * emDirEntry::GetLStat() const
{
	return Data->LStat;
}

inline const emString & emDirEntry::GetOwner() const
{
	return Data->Owner;
}

inline const emString & emDirEntry::GetGroup() const
{
	return Data->Group;
}

inline int emDirEntry::GetTargetPathErrNo() const
{
	return Data->TargetPathErrNo;
}

inline int emDirEntry::GetStatErrNo() const
{
	return Data->StatErrNo;
}

inline int emDirEntry::GetLStatErrNo() const
{
	return Data->LStatErrNo;
}

#if defined(_WIN32)
inline emUInt32 emDirEntry::GetWndsFileAttributes() const
{
	return Data->WndsFileAttributes;
}
#endif


#endif
