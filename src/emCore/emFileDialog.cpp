//------------------------------------------------------------------------------
// emFileDialog.cpp
//
// Copyright (C) 2015-2016 Oliver Hamann.
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

#include <emCore/emFileDialog.h>
#include <emCore/emLabel.h>


emFileDialog::emFileDialog(
	emContext & parentContext, ModeType mode, ViewFlags viewFlags,
	WindowFlags windowFlags, const emString & wmResName
) :
	emDialog(parentContext,viewFlags,windowFlags,wmResName)
{
	Fsb=new emFileSelectionBox(GetContentPanel(),"fsb");
	Fsb->SetBorderType(emBorder::OBT_NONE,emBorder::IBT_NONE);

	Mode=mode;
	DirAllowed=false;

	AddOKButton();
	AddCancelButton();

	SetMode(mode);

	AddWakeUpSignal(Fsb->GetFileTriggerSignal());
}


emFileDialog::~emFileDialog()
{
}


void emFileDialog::SetMode(ModeType mode)
{
	emButton * okButton;

	Mode=mode;
	okButton=GetOKButton();
	switch (mode) {
	case M_SELECT:
		SetRootTitle("Files");
		if (okButton) okButton->SetCaption("OK");
		break;
	case M_OPEN:
		SetRootTitle("Open");
		if (okButton) okButton->SetCaption("Open");
		break;
	case M_SAVE:
		SetRootTitle("Save As");
		if (okButton) okButton->SetCaption("Save");
		break;
	}
}


void emFileDialog::SetDirectoryResultAllowed(bool dirAllowed)
{
	DirAllowed=dirAllowed;
}


bool emFileDialog::Cycle()
{
	bool busy;

	busy=emDialog::Cycle();

	if (IsSignaled(Fsb->GetFileTriggerSignal())) {
		Finish(POSITIVE);
	}

	if (OverwriteDialog && IsSignaled(OverwriteDialog->GetFinishSignal())) {
		switch (OverwriteDialog->GetResult()) {
		case POSITIVE:
			OverwriteConfirmed=OverwriteAsked;
			OverwriteAsked.Clear();
			delete OverwriteDialog.Get();
			Finish(POSITIVE);
			break;
		case NEGATIVE:
			OverwriteAsked.Clear();
			delete OverwriteDialog.Get();
			break;
		}
	}

	return busy;
}


bool emFileDialog::CheckFinish(int result)
{
	emArray<emString> names;
	emArray<emString> pathsToOverwrite;
	emString path,text;
	int i;

	if (!emDialog::CheckFinish(result)) return false;
	if (result == NEGATIVE) return true;

	if (!DirAllowed) {
		names=GetSelectedNames();
		if (names.GetCount()==0) {
			// GetSelectedPath() returns a directory...
			emDialog::ShowMessage(
				*this,
				"Error",
				emString::Format("No file selected")
			);
			return false;
		}
		for (i=0; i<names.GetCount(); i++) {
			path=emGetChildPath(GetParentDirectory(),names[i]);
			if (emIsDirectory(path)) {
				if (names.GetCount()==1) {
					Fsb->EnterSubDir(names[i]);
				}
				else {
					emDialog::ShowMessage(
						*this,
						"Error",
						emString::Format("Directory selected: %s",names[i].Get())
					);
				}
				return false;
			}
		}
	}

	if (Mode==M_OPEN) {
		names=GetSelectedNames();
		for (i=0; i<names.GetCount(); i++) {
			path=emGetChildPath(GetParentDirectory(),names[i]);
			if (!emIsExistingPath(path)) {
				emDialog::ShowMessage(
					*this,
					"Open Error",
					emString::Format(
						"The following file cannot be opened, because it does not exist:\n\n%s",
						path.Get()
					)
				);
				return false;
			}
		}
	}
	else if (Mode==M_SAVE) {
		names=GetSelectedNames();
		pathsToOverwrite.Clear();
		for (i=0; i<names.GetCount(); i++) {
			path=emGetChildPath(GetParentDirectory(),names[i]);
			if (emIsExistingPath(path)) {
				pathsToOverwrite.Add(path);
			}
		}
		if (!pathsToOverwrite.IsEmpty()) {
			if (pathsToOverwrite.GetCount() == 1) {
				text="Are you sure to overwrite the following already existing file?\n";
			}
			else {
				text="Are you sure to overwrite the following already existing files?\n";
			}
			for (i=0; i<pathsToOverwrite.GetCount(); i++) {
				text+="\n";
				text+=pathsToOverwrite[i];
			}
			if (text!=OverwriteConfirmed) {
				if (OverwriteDialog) delete OverwriteDialog.Get();
				OverwriteAsked=text;
				OverwriteDialog=new emDialog((emContext&)*this);
				OverwriteDialog->SetRootTitle("File Exists");
				new emLabel(
					OverwriteDialog->GetContentPanel(),
					"label",
					text
				);
				OverwriteDialog->AddOKCancelButtons();
				AddWakeUpSignal(OverwriteDialog->GetFinishSignal());
				return false;
			}
		}
		OverwriteConfirmed.Clear();
	}

	return true;
}
