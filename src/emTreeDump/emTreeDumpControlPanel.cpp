//------------------------------------------------------------------------------
// emTreeDumpControlPanel.cpp
//
// Copyright (C) 2011,2014-2015 Oliver Hamann.
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

#include <emTreeDump/emTreeDumpControlPanel.h>
#include <emCore/emProcess.h>


emTreeDumpControlPanel::emTreeDumpControlPanel(
	ParentArg parent, const emString & name, emView & contentView,
	emTreeDumpRec * rec, const emString & dir
)
	: emRasterGroup(parent,name,"emTreeDump"),
	ContentView(contentView)
{
	Rec=rec;
	Dir=dir;
	SetMinCellCount(3);
}


emTreeDumpControlPanel::~emTreeDumpControlPanel()
{
}


bool emTreeDumpControlPanel::Cycle()
{
	emPanel * p;
	emButton * b;
	bool busy;
	int i;

	busy=emRasterGroup::Cycle();

	if (Rec) {
		for (p=GetFirstChild(); p; p=p->GetNext()) {
			b=dynamic_cast<emButton*>(p);
			if (b && IsSignaled(b->GetClickSignal())) {
				i=atoi(b->GetName());
				if (i>=0 && i<Rec->Commands.GetCount()) {
					RunCommand(i);
				}
			}
		}
	}

	return busy;
}


void emTreeDumpControlPanel::AutoExpand()
{
	emButton * b;
	int i;

	if (Rec) {
		for (i=0; i<Rec->Commands.GetCount(); i++) {
			b=new emButton(
				this,
				emString::Format("%d",i),
				Rec->Commands[i].Caption
			);
			AddWakeUpSignal(b->GetClickSignal());
		}
	}
}


void emTreeDumpControlPanel::RunCommand(int index)
{
	emArray<emString> args;
	emTreeDumpRec::CommandRec * c;
	int i;

	if (Rec && index>=0 && index<Rec->Commands.GetCount()) {
		c=&Rec->Commands[index];
		for (i=0; i<c->Args.GetCount(); i++) {
			args.Add(c->Args[i].Get());
		}
		try {
			emProcess::TryStartUnmanaged(args,emArray<emString>(),Dir);
		}
		catch (emException & exception) {
			emDialog::ShowMessage(ContentView,"Error",exception.GetText());
		}
	}
}
