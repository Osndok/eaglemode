//------------------------------------------------------------------------------
// emFileManConfig.cpp
//
// Copyright (C) 2006-2008,2010 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emFileMan/emFileManConfig.h>


emRef<emFileManConfig> emFileManConfig::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emFileManConfig,rootContext,"")
}


const char * emFileManConfig::GetFormatName() const
{
	return "emFileManConfig";
}


emFileManConfig::emFileManConfig(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	SortCriterion(
		this,
		"SortCriterion",
		SORT_BY_NAME,
		"SORT_BY_NAME",
		"SORT_BY_ENDING",
		"SORT_BY_CLASS",
		"SORT_BY_VERSION",
		"SORT_BY_DATE",
		"SORT_BY_SIZE",
		NULL
	),
	NameSortingStyle(
		this,
		"NameSortingStyle",
		NSS_PER_LOCALE,
		"NSS_PER_LOCALE",
		"NSS_CASE_SENSITIVE",
		"NSS_CASE_INSENSITIVE",
		NULL
	),
	SortDirectoriesFirst(this,"SortDirectoriesFirst",false),
	ShowHiddenFiles(this,"ShowHiddenFiles",false),
	ThemeName(this,"ThemeName","Metal1"),
	Autosave(this,"Autosave",true)
{
	PostConstruct(
		*this,
		emGetInstallPath(EM_IDT_USER_CONFIG,"emFileMan","config.rec")
	);
	LoadOrInstall();
}


emFileManConfig::~emFileManConfig()
{
}
