//------------------------------------------------------------------------------
// emCheckButton.cpp
//
// Copyright (C) 2005-2011,2014,2016 Oliver Hamann.
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

#include <emCore/emCheckButton.h>


emCheckButton::emCheckButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emButton(parent,name,caption,description,icon)
{
	Checked=false;
}


emCheckButton::~emCheckButton()
{
}


void emCheckButton::SetChecked(bool checked)
{
	if (Checked!=checked) {
		Checked=checked;
		SetShownChecked(checked);
		InvalidatePainting();
		Signal(CheckSignal);
		CheckChanged();
	}
}


void emCheckButton::Clicked()
{
	SetChecked(!IsChecked());
}


void emCheckButton::CheckChanged()
{
}


emString emCheckButton::GetHowTo() const
{
	emString h;

	h=emButton::GetHowTo();
	h+=HowToCheckButton;
	if (IsChecked()) h+=HowToChecked;
	else h+=HowToNotChecked;
	return h;
}


const char * emCheckButton::HowToCheckButton=
	"\n"
	"\n"
	"CHECK BUTTON\n"
	"\n"
	"This button can have checked or unchecked state. Usually this is a yes-or-no\n"
	"answer to a question. Whenever the button is triggered, the check state toggles.\n"
;


const char * emCheckButton::HowToChecked=
	"\n"
	"\n"
	"CHECKED\n"
	"\n"
	"Currently this check button is checked.\n"
;


const char * emCheckButton::HowToNotChecked=
	"\n"
	"\n"
	"UNCHECKED\n"
	"\n"
	"Currently this check button is not checked.\n"
;
