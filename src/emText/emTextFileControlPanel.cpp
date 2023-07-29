//------------------------------------------------------------------------------
// emTextFileControlPanel.cpp
//
// Copyright (C) 2023 Oliver Hamann.
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

#include <emText/emTextFileControlPanel.h>


emTextFileControlPanel::emTextFileControlPanel(
	ParentArg parent, const emString & name, emTextFilePanel & filePanel
) :
	emLinearGroup(parent,name,"Text File"),
	FileModel(filePanel.GetFileModel()),
	FilePanel(&filePanel),
	CharEncoding(NULL),
	LineBreakEncoding(NULL),
	NumberOfLines(NULL),
	NumberOfColumns(NULL),
	Copy(NULL),
	SelectAll(NULL),
	ClearSelection(NULL)
{
	if (FileModel) {
		AddWakeUpSignal(FileModel->GetFileStateSignal());
		AddWakeUpSignal(FileModel->GetChangeSignal());
	}
	if (FilePanel) AddWakeUpSignal(FilePanel->GetSelectionSignal());
}


emTextFileControlPanel::~emTextFileControlPanel()
{
}


bool emTextFileControlPanel::Cycle()
{
	bool busy;

	busy=emLinearGroup::Cycle();

	if (
		FileModel && (
			IsSignaled(FileModel->GetFileStateSignal()) ||
			IsSignaled(FileModel->GetChangeSignal())
		)
	) {
		UpdateControls();
	}

	if (FilePanel) {
		if (IsSignaled(FilePanel->GetSelectionSignal())) {
			UpdateControls();
		}
		if (Copy && IsSignaled(Copy->GetClickSignal())) {
			FilePanel->CopySelectedTextToClipboard();
		}
		if (SelectAll && IsSignaled(SelectAll->GetClickSignal())) {
			FilePanel->SelectAll(true);
		}
		if (ClearSelection && IsSignaled(ClearSelection->GetClickSignal())) {
			FilePanel->EmptySelection();
		}
	}

	return busy;
}


void emTextFileControlPanel::AutoExpand()
{
	emRasterGroup * infos;
	emLinearGroup * selection;

	emLinearGroup::AutoExpand();

	SetChildWeight(1,0.2);

	infos=new emRasterGroup(this,"infos","Infos");
	infos->SetPrefChildTallness(0.1);
	infos->SetRowByRow();

	CharEncoding=new emTextField(
		infos,
		"enc",
		"Character Encoding"
	);

	LineBreakEncoding=new emTextField(
		infos,
		"lbenc",
		"Line Break Encoding"
	);

	NumberOfLines=new emTextField(
		infos,
		"lines",
		"Number of Lines"
	);

	NumberOfColumns=new emTextField(
		infos,
		"columns",
		"Number of Columns"
	);

	selection=new emLinearGroup(this,"selection","Selection");

	Copy=new emButton(
		selection,
		"copy",
		"Copy",
		"Copy the selected text to the clipboard.\n"
		"\n"
		"Hotkey: Ctrl+C"
	);
	AddWakeUpSignal(Copy->GetClickSignal());

	SelectAll=new emButton(
		selection,
		"selectAll",
		"Select All",
		"Select all text.\n"
		"\n"
		"Hotkey: Ctrl+A"
	);
	AddWakeUpSignal(SelectAll->GetClickSignal());

	ClearSelection=new emButton(
		selection,
		"clearSelection",
		"Clear Selection",
		"Deselect all text.\n"
		"\n"
		"Hotkey: Shift+Ctrl+A"
	);
	AddWakeUpSignal(ClearSelection->GetClickSignal());

	UpdateControls();
}


void emTextFileControlPanel::AutoShrink()
{
	CharEncoding=NULL;
	LineBreakEncoding=NULL;
	NumberOfLines=NULL;
	NumberOfColumns=NULL;
	Copy=NULL;
	SelectAll=NULL;
	ClearSelection=NULL;

	emLinearGroup::AutoShrink();
}


void emTextFileControlPanel::UpdateControls()
{
	const char * p;

	if (!IsAutoExpanded()) return;

	if (
		!FileModel || !FilePanel ||
		!FilePanel->IsVFSGood() ||
		FilePanel->IsHexView()
	) {
		CharEncoding->SetEnableSwitch(false);
		CharEncoding->SetText(emString());
		LineBreakEncoding->SetEnableSwitch(false);
		LineBreakEncoding->SetText(emString());
		NumberOfLines->SetEnableSwitch(false);
		NumberOfLines->SetText(emString());
		NumberOfColumns->SetEnableSwitch(false);
		NumberOfColumns->SetText(emString());
		Copy->SetEnableSwitch(false);
		SelectAll->SetEnableSwitch(false);
		ClearSelection->SetEnableSwitch(false);
		return;
	}

	CharEncoding->SetEnableSwitch(true);
	switch (FileModel->GetCharEncoding()) {
		case emTextFileModel::CE_7BIT   : p="7-Bit"    ; break;
		case emTextFileModel::CE_8BIT   : p="8-Bit"    ; break;
		case emTextFileModel::CE_UTF8   : p="UTF-8"    ; break;
		case emTextFileModel::CE_UTF16LE: p="UTF-16LE" ; break;
		case emTextFileModel::CE_UTF16BE: p="UTF-16BE" ; break;
		default                         : p="Binary"   ;
	}
	CharEncoding->SetText(p);

	LineBreakEncoding->SetEnableSwitch(true);
	switch (FileModel->GetLineBreakEncoding()) {
		case emTextFileModel::LBE_MAC  : p="MAC (CR)"  ; break;
		case emTextFileModel::LBE_DOS  : p="DOS (CRLF)"; break;
		case emTextFileModel::LBE_UNIX : p="UNIX (LF)" ; break;
		case emTextFileModel::LBE_MIXED: p="Mixed"     ; break;
		default                        : p="None"      ;
	}
	LineBreakEncoding->SetText(p);

	NumberOfLines->SetEnableSwitch(true);
	NumberOfLines->SetText(emString::Format("%d",FileModel->GetLineCount()));

	NumberOfColumns->SetEnableSwitch(true);
	NumberOfColumns->SetText(emString::Format("%d",FileModel->GetColumnCount()));

	Copy->SetEnableSwitch(!FilePanel->IsSelectionEmpty());

	SelectAll->SetEnableSwitch(
		FilePanel->GetSelectionStartIndex()!=0 ||
		FilePanel->GetSelectionEndIndex()!=FileModel->GetContent().GetCount()
	);

	ClearSelection->SetEnableSwitch(!FilePanel->IsSelectionEmpty());
}
