//------------------------------------------------------------------------------
// emFileManViewConfig.h
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#ifndef emFileManViewConfig_h
#define emFileManViewConfig_h

#ifndef emView_h
#include <emCore/emView.h>
#endif

#ifndef emFileManConfig_h
#include <emFileMan/emFileManConfig.h>
#endif

#ifndef emDirEntry_h
#include <emFileMan/emDirEntry.h>
#endif


class emFileManViewConfig : public emModel {

public:

	static emRef<emFileManViewConfig> Acquire(emView & view);

	const emSignal & GetChangeSignal() const;

	enum SortCriterionType {
		SORT_BY_NAME    = emFileManConfig::SORT_BY_NAME,
		SORT_BY_ENDING  = emFileManConfig::SORT_BY_ENDING,
		SORT_BY_CLASS   = emFileManConfig::SORT_BY_CLASS,
		SORT_BY_VERSION = emFileManConfig::SORT_BY_VERSION,
		SORT_BY_DATE    = emFileManConfig::SORT_BY_DATE,
		SORT_BY_SIZE    = emFileManConfig::SORT_BY_SIZE
	};

	SortCriterionType GetSortCriterion() const;
	void SetSortCriterion(SortCriterionType sc);

	enum NameSortingStyleType {
		NSS_PER_LOCALE       = emFileManConfig::NSS_PER_LOCALE,
		NSS_CASE_SENSITIVE   = emFileManConfig::NSS_CASE_SENSITIVE,
		NSS_CASE_INSENSITIVE = emFileManConfig::NSS_CASE_INSENSITIVE
	};

	NameSortingStyleType GetNameSortingStyle() const;
	void SetNameSortingStyle(NameSortingStyleType nss);

	bool GetSortDirectoriesFirst() const;
	void SetSortDirectoriesFirst(bool b);

	bool GetShowHiddenFiles() const;
	void SetShowHiddenFiles(bool b);

	void SaveAsDefault();

	int CompareDirEntries(const emDirEntry & e1, const emDirEntry & e2) const;

protected:

	emFileManViewConfig(emContext & context, const emString & name);
	virtual ~emFileManViewConfig();

private:

	int CompareNames(const char * n1, const char * n2) const;

	emSignal ChangeSignal;

	emRef<emFileManConfig> FileManConfig;

	SortCriterionType SortCriterion;
	NameSortingStyleType NameSortingStyle;
	bool SortDirectoriesFirst;
	bool ShowHiddenFiles;
};

inline const emSignal & emFileManViewConfig::GetChangeSignal() const
{
	return ChangeSignal;
}

inline emFileManViewConfig::SortCriterionType emFileManViewConfig::GetSortCriterion() const
{
	return SortCriterion;
}

inline emFileManViewConfig::NameSortingStyleType emFileManViewConfig::GetNameSortingStyle() const
{
	return NameSortingStyle;
}

inline bool emFileManViewConfig::GetSortDirectoriesFirst() const
{
	return SortDirectoriesFirst;
}

inline bool emFileManViewConfig::GetShowHiddenFiles() const
{
	return ShowHiddenFiles;
}


#endif
