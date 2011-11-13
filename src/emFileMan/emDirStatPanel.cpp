//------------------------------------------------------------------------------
// emDirStatPanel.cpp
//
// Copyright (C) 2007-2008,2010 Oliver Hamann.
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

#include <emFileMan/emDirStatPanel.h>


emDirStatPanel::emDirStatPanel(
	ParentArg parent, const emString & name, emDirModel * fileModel,
	bool updateFileModel
)
	: emFilePanel(parent,name)
{
	AddWakeUpSignal(GetVirFileStateSignal());
	SetFileModel(fileModel,updateFileModel);
	Config=emFileManViewConfig::Acquire(GetView());
	TotalCount=-1;
	FileCount=-1;
	SubDirCount=-1;
	OtherTypeCount=-1;
	HiddenCount=-1;

	AddWakeUpSignal(Config->GetChangeSignal());
}


void emDirStatPanel::SetFileModel(emFileModel * fileModel, bool updateFileModel)
{
	if (fileModel && (dynamic_cast<emDirModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}
	emFilePanel::SetFileModel(fileModel,updateFileModel);
}


bool emDirStatPanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();
	if (IsSignaled(GetVirFileStateSignal())) {
		UpdateStatistics();
		InvalidatePainting();
	}
	if (IsSignaled(Config->GetChangeSignal())) {
		InvalidatePainting();
	}
	return busy;
}


bool emDirStatPanel::IsOpaque()
{
	if (GetVirFileState()!=VFS_LOADED) return emFilePanel::IsOpaque();
	return Config->GetTheme().BackgroundColor.Get().IsOpaque();
}


void emDirStatPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	char tmp[1024];

	if (GetVirFileState()!=VFS_LOADED) {
		emFilePanel::Paint(painter,canvasColor);
		return;
	}

	painter.Clear(Config->GetTheme().BackgroundColor);

	sprintf(tmp,
		"Directory Statistics\n"
		"~~~~~~~~~~~~~~~~~~~~\n"
		"\n"
		"Total Entries : %5d\n"
		"\n"
		"Hidden Entries: %5d\n"
		"\n"
		"Regular Files : %5d\n"
		"Subdirectories: %5d\n"
		"Other Types   : %5d",
		TotalCount,
		HiddenCount,
		FileCount,
		SubDirCount,
		OtherTypeCount
	);
	painter.PaintTextBoxed(
		0.02, 0.02,
		1.0-0.04, GetHeight()-0.04,
		tmp,
		GetHeight(),
		Config->GetTheme().DirNameColor,
		canvasColor
	);
}


void emDirStatPanel::UpdateStatistics()
{
	const emDirEntry * de;
	const emDirModel * dm;
	int i;

	if (GetVirFileState()==VFS_LOADED) {
		dm=(const emDirModel*)GetFileModel();
		TotalCount=dm->GetEntryCount();
		FileCount=0;
		SubDirCount=0;
		OtherTypeCount=0;
		HiddenCount=0;
		for (i=0; i<TotalCount; i++) {
			de=&dm->GetEntry(i);
			if (de->IsRegularFile()) FileCount++;
			else if (de->IsDirectory()) SubDirCount++;
			else OtherTypeCount++;
			if (de->IsHidden()) HiddenCount++;
		}
	}
	else {
		TotalCount=-1;
		FileCount=-1;
		SubDirCount=-1;
		OtherTypeCount=-1;
		HiddenCount=-1;
	}
}
