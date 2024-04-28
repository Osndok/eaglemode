//------------------------------------------------------------------------------
// emHmiDemoFile.cpp
//
// Copyright (C) 2012,2014-2015,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoFile.h>


emHmiDemoFile::emHmiDemoFile(
	ParentArg parent, const emString & name, const emString & path
)
	: emBorder(parent,name)
{
	FpPluginList=emFpPluginList::Acquire(GetRootContext());
	Path=path;
	SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
}


emHmiDemoFile::emHmiDemoFile(
	ParentArg parent, const emString & name, emInstallDirType idt,
	const char * prj, const char * subPath,
	const char * subPath2
)
	: emBorder(parent,name)
{
	FpPluginList=emFpPluginList::Acquire(GetRootContext());
	Path=emGetInstallPath(idt,prj,subPath);
	if (subPath2) Path=emGetChildPath(Path,subPath2);
	SetBorderType(OBT_INSTRUMENT,IBT_OUTPUT_FIELD);
}


emHmiDemoFile::~emHmiDemoFile()
{
}


void emHmiDemoFile::AutoExpand()
{
	emPanel * p;

	p=FpPluginList->CreateFilePanel(this,"file",Path);
	p->SetFocusable(false);
}


void emHmiDemoFile::LayoutChildren()
{
	double x,y,w,h,r,d;
	emColor cc;
	emPanel * p;

	if (
		strcasecmp(emGetNameInPath(Path),"graph1.pdf")==0 ||
		strcasecmp(emGetNameInPath(Path),"graph2.pdf")==0 ||
		strcasecmp(emGetNameInPath(Path),"table.pdf")==0
	) {
		d=0.1;
	}
	else {
		d=1.0;
	}

	p=GetChild("file");
	if (p) {
		GetContentRoundRect(&x,&y,&w,&h,&r,&cc);
		p->Layout(
			x+r*d,
			y+r*d,
			w-r*d*2,
			h-r*d*2,
			cc
		);
	}
}
