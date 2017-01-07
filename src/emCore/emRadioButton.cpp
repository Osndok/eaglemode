//------------------------------------------------------------------------------
// emRadioButton.cpp
//
// Copyright (C) 2005-2011,2014-2016 Oliver Hamann.
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

#include <emCore/emRadioButton.h>


emRadioButton::emRadioButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emCheckButton(parent,name,caption,description,icon)
{
	Mechanism * mechanism;

	SetShownRadioed(true);
	Mech=NULL;
	MechIndex=-1;
	if (GetParent()) {
		mechanism=dynamic_cast<emRadioButton::Mechanism*>(GetParent());
		if (mechanism) mechanism->Add(this);
	}
}


emRadioButton::~emRadioButton()
{
	if (Mech) Mech->Remove(this);
}


emRadioButton::Mechanism::Mechanism()
{
	Array.SetTuningLevel(4);
	CheckIndex=-1;
}


emRadioButton::Mechanism::~Mechanism()
{
	RemoveAll();
}


void emRadioButton::Mechanism::Add(emRadioButton * radioButton)
{
	if (radioButton->Mech) radioButton->Mech->Remove(radioButton);
	radioButton->Mech=this;
	radioButton->MechIndex=Array.GetCount();
	Array.Add(radioButton);
	if (radioButton->IsChecked()) {
		if (CheckIndex>=0) {
			radioButton->SetChecked(false);
		}
		else {
			CheckIndex=Array.GetCount()-1;
			CheckSignal.Signal(radioButton->GetScheduler());
			CheckChanged();
		}
	}
}


void emRadioButton::Mechanism::AddAll(emPanel * parent)
{
	emRadioButton * rb;
	emPanel * p;

	for (p=parent->GetFirstChild(); p; p=p->GetNext()) {
		rb=dynamic_cast<emRadioButton*>(p);
		if (rb) Add(rb);
	}
}


void emRadioButton::Mechanism::Remove(emRadioButton * radioButton)
{
	RemoveByIndex(GetIndexOf(radioButton));
}


void emRadioButton::Mechanism::RemoveByIndex(int index)
{
	emScheduler * scheduler;
	emRadioButton * rb;
	int i;

	if (index>=0 && index<Array.GetCount()) {
		rb=Array[index];
		rb->Mech=NULL;
		rb->MechIndex=-1;
		scheduler=&rb->GetScheduler();
		Array.Remove(index);
		for (i=Array.GetCount()-1; i>=index; i--) Array[i]->MechIndex=i;
		if (CheckIndex>=index) {
			if (CheckIndex==index) CheckIndex=-1;
			else CheckIndex--;
			CheckSignal.Signal(*scheduler);
			CheckChanged();
		}
	}
}


void emRadioButton::Mechanism::RemoveAll()
{
	emScheduler * scheduler;
	emRadioButton * rb;
	int i;

	i=Array.GetCount()-1;
	if (i>=0) {
		scheduler=&Array[0]->GetScheduler();
		do {
			rb=Array[i];
			rb->Mech=NULL;
			rb->MechIndex=-1;
			i--;
		} while (i>=0);
		Array.Clear();
		if (CheckIndex>=0) {
			CheckIndex=-1;
			CheckSignal.Signal(*scheduler);
			CheckChanged();
		}
	}
}


void emRadioButton::Mechanism::SetChecked(emRadioButton * radioButton)
{
	SetCheckIndex(GetIndexOf(radioButton));
}


void emRadioButton::Mechanism::SetCheckIndex(int index)
{
	emScheduler * scheduler;
	int old;

	if (index<-1 || index>=Array.GetCount()) index=-1;
	if (CheckIndex!=index) {
		// Remember, this could be called recursively!
		scheduler=&Array[0]->GetScheduler();
		if (CheckIndex>=0 && Array[CheckIndex]->IsChecked()) {
			old=CheckIndex;
			CheckIndex=-1;
			Array[old]->SetChecked(false);
			if (CheckIndex!=-1) return;
		}
		CheckIndex=index;
		if (CheckIndex>=0 && !Array[CheckIndex]->IsChecked()) {
			Array[CheckIndex]->SetChecked(true);
			if (CheckIndex!=index) return;
		}
		CheckSignal.Signal(*scheduler);
		CheckChanged();
	}
}


emRadioButton * emRadioButton::Mechanism::GetButton(int index) const
{
	if (index>=0 && index<Array.GetCount()) {
		return Array[index];
	}
	return NULL;
}



void emRadioButton::Mechanism::CheckChanged()
{
}


emRadioButton::LinearGroup::LinearGroup(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emLinearGroup(parent,name,caption,description,icon)
{
}


emRadioButton::LinearGroup::~LinearGroup()
{
}


emRadioButton::RasterGroup::RasterGroup(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emRasterGroup(parent,name,caption,description,icon)
{
}


emRadioButton::RasterGroup::~RasterGroup()
{
}


emRadioButton::Group::Group(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emGroup(parent,name,caption,description,icon,0)
{
}


emRadioButton::Group::~Group()
{
}


void emRadioButton::Clicked()
{
	if (Mech) Mech->SetChecked(this);
}


void emRadioButton::CheckChanged()
{
	if (Mech) {
		if (IsChecked()) Mech->SetChecked(this);
		else if (Mech->GetChecked()==this) Mech->SetChecked(NULL);
	}
}


emString emRadioButton::GetHowTo() const
{
	emString h;

	h=emCheckButton::GetHowTo();
	h+=HowToRadioButton;
	return h;
}


const char * emRadioButton::HowToRadioButton=
	"\n"
	"\n"
	"RADIO BUTTON\n"
	"\n"
	"This is a radio button. It is a check button with changed behavior: In a set of\n"
	"radio buttons, only one button can have checked state. When triggering a radio\n"
	"button, that button is checked and all the other radio buttons of the set are\n"
	"unchecked. There is no way to uncheck a radio button directly.\n"
;
