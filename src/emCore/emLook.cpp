//------------------------------------------------------------------------------
// emLook.cpp
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

#include <emCore/emLook.h>
#include <emCore/emBorder.h>


#ifdef EM_NO_DATA_EXPORT
emLook::emLook()
{
	Data=&DefaultData;
}
#endif


bool emLook::operator == (const emLook & look) const
{
	return
		Data==look.Data || (
			Data->BgColor==look.Data->BgColor &&
			Data->FgColor==look.Data->FgColor &&
			Data->ButtonBgColor==look.Data->ButtonBgColor &&
			Data->ButtonFgColor==look.Data->ButtonFgColor &&
			Data->InputBgColor==look.Data->InputBgColor &&
			Data->InputFgColor==look.Data->InputFgColor &&
			Data->InputHlColor==look.Data->InputHlColor &&
			Data->OutputBgColor==look.Data->OutputBgColor &&
			Data->OutputFgColor==look.Data->OutputFgColor &&
			Data->OutputHlColor==look.Data->OutputHlColor
		)
	;
}


void emLook::Apply(emPanel * panel, bool recursively) const
{
	emBorder * border;

	border=dynamic_cast<emBorder*>(panel);
	if (border) {
		// Remember: border->SetLook could be overloaded to stop the
		// recursion.
		border->SetLook(*this,recursively);
	}
	else if (recursively) {
		for (panel=panel->GetFirstChild(); panel; panel=panel->GetNext()) {
			Apply(panel,true);
		}
	}
}


void emLook::SetBgColor(emColor bgColor)
{
	if (Data->BgColor!=bgColor) {
		MakeWritable();
		Data->BgColor=bgColor;
	}
}


void emLook::SetFgColor(emColor fgColor)
{
	if (Data->FgColor!=fgColor) {
		MakeWritable();
		Data->FgColor=fgColor;
	}
}


void emLook::SetButtonBgColor(emColor buttonBgColor)
{
	if (Data->ButtonBgColor!=buttonBgColor) {
		MakeWritable();
		Data->ButtonBgColor=buttonBgColor;
	}
}


void emLook::SetButtonFgColor(emColor buttonFgColor)
{
	if (Data->ButtonFgColor!=buttonFgColor) {
		MakeWritable();
		Data->ButtonFgColor=buttonFgColor;
	}
}


void emLook::SetInputBgColor(emColor inputBgColor)
{
	if (Data->InputBgColor!=inputBgColor) {
		MakeWritable();
		Data->InputBgColor=inputBgColor;
	}
}


void emLook::SetInputFgColor(emColor inputFgColor)
{
	if (Data->InputFgColor!=inputFgColor) {
		MakeWritable();
		Data->InputFgColor=inputFgColor;
	}
}


void emLook::SetInputHlColor(emColor inputHlColor)
{
	if (Data->InputHlColor!=inputHlColor) {
		MakeWritable();
		Data->InputHlColor=inputHlColor;
	}
}


void emLook::SetOutputBgColor(emColor outputBgColor)
{
	if (Data->OutputBgColor!=outputBgColor) {
		MakeWritable();
		Data->OutputBgColor=outputBgColor;
	}
}


void emLook::SetOutputFgColor(emColor outputFgColor)
{
	if (Data->OutputFgColor!=outputFgColor) {
		MakeWritable();
		Data->OutputFgColor=outputFgColor;
	}
}


void emLook::SetOutputHlColor(emColor outputHlColor)
{
	if (Data->OutputHlColor!=outputHlColor) {
		MakeWritable();
		Data->OutputHlColor=outputHlColor;
	}
}


unsigned int emLook::GetDataRefCount() const
{
	return Data==&DefaultData ? UINT_MAX/2 : Data->RefCount;
}


void emLook::DeleteData()
{
	DefaultData.RefCount=UINT_MAX/2;
	if (Data!=&DefaultData) delete Data;
}


void emLook::MakeWritable()
{
	SharedData * d;

	if (Data->RefCount>1 || Data==&DefaultData) {
		d=new SharedData(*Data);
		d->RefCount=1;
		Data->RefCount--;
		Data=d;
	}
}


emLook::SharedData::SharedData()
	: RefCount(UINT_MAX/2),
	BgColor      (0x515E84FF),
	FgColor      (0xEFF0F4FF),
	ButtonBgColor(0x596790FF),
	ButtonFgColor(0xF2F2F7FF),
	InputBgColor (0xEFF0F4FF),
	InputFgColor (0x020E1DFF),
	InputHlColor (0x0038C0FF),
	OutputBgColor(0xA7A9B0FF),
	OutputFgColor(0x070B18FF),
	OutputHlColor(0x002B9AFF)
{
}


emLook::SharedData::SharedData(const SharedData & sd)
	: RefCount(sd.RefCount),
	BgColor(sd.BgColor),
	FgColor(sd.FgColor),
	ButtonBgColor(sd.ButtonBgColor),
	ButtonFgColor(sd.ButtonFgColor),
	InputBgColor(sd.InputBgColor),
	InputFgColor(sd.InputFgColor),
	InputHlColor(sd.InputHlColor),
	OutputBgColor(sd.OutputBgColor),
	OutputFgColor(sd.OutputFgColor),
	OutputHlColor(sd.OutputHlColor)
{
}


emLook::SharedData emLook::DefaultData;
