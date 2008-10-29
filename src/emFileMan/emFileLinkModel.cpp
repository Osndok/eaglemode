//------------------------------------------------------------------------------
// emFileLinkModel.cpp
//
// Copyright (C) 2007-2008 Oliver Hamann.
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
#include <emFileMan/emFileLinkModel.h>


emRef<emFileLinkModel> emFileLinkModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emFileLinkModel,context,name,common)
}


const char * emFileLinkModel::GetFormatName() const
{
	return "emFileLink";
}


emString emFileLinkModel::GetFullPath() const
{
	emString basePath;

	switch (BasePathType.Get()) {
	case BPT_BIN:
		basePath=emGetInstallPath(EM_IDT_BIN,BasePathProject.Get());
		break;
	case BPT_INCLUDE:
		basePath=emGetInstallPath(EM_IDT_INCLUDE,BasePathProject.Get());
		break;
	case BPT_LIB:
		basePath=emGetInstallPath(EM_IDT_LIB,BasePathProject.Get());
		break;
	case BPT_HTML_DOC:
		basePath=emGetInstallPath(EM_IDT_HTML_DOC,BasePathProject.Get());
		break;
	case BPT_PS_DOC:
		basePath=emGetInstallPath(EM_IDT_PS_DOC,BasePathProject.Get());
		break;
	case BPT_USER_CONFIG:
		basePath=emGetInstallPath(EM_IDT_USER_CONFIG,BasePathProject.Get());
		break;
	case BPT_HOST_CONFIG:
		basePath=emGetInstallPath(EM_IDT_HOST_CONFIG,BasePathProject.Get());
		break;
	case BPT_TMP:
		basePath=emGetInstallPath(EM_IDT_TMP,BasePathProject.Get());
		break;
	case BPT_RES:
		basePath=emGetInstallPath(EM_IDT_RES,BasePathProject.Get());
		break;
	case BPT_HOME:
		basePath=emGetInstallPath(EM_IDT_HOME,BasePathProject.Get());
		break;
	default:
		basePath=emGetParentPath(GetFilePath());
		break;
	}
	return emGetAbsolutePath(Path.Get(),basePath);
}


emFileLinkModel::emFileLinkModel(emContext & context, const emString & name)
	: emRecFileModel(context,name),
	emStructRec(),
	BasePathType(
		this,
		"BasePathType",
		BPT_NONE,
		"None",
		"Bin",
		"Include",
		"Lib",
		"HtmlDoc",
		"PsDoc",
		"UserConfig",
		"HostConfig",
		"Tmp",
		"Res",
		"Home",
		NULL
	),
	BasePathProject(this,"BasePathProject"),
	Path(this,"Path"),
	HaveDirEntry(this,"HaveDirEntry",false)
{
	PostConstruct(*this);
}


emFileLinkModel::~emFileLinkModel()
{
}
