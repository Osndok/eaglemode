//------------------------------------------------------------------------------
// emLook.h
//
// Copyright (C) 2005-2010,2014 Oliver Hamann.
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

#ifndef emLook_h
#define emLook_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//=================================== emLook ===================================
//==============================================================================

class emLook {

public:

	// Class for the look of toolkit panels. Currently, the look consists of
	// a set of colors only. Objects of this class have copy-on-write
	// behavior.

	emLook();
		// Construct a default look.

	emLook(const emLook & look);
		// Construct a copied look.

	~emLook();
		// Destructor.

	emLook & operator = (const emLook & look);
		// Copy a look.

	bool operator == (const emLook & look) const;
	bool operator != (const emLook & look) const;
		// Compare two looks.

	void Apply(emPanel * panel, bool recursively) const;
		// Apply this look to a panel or to all panels in a sub-tree.
		// Applying actually works for panels of class emBorder and
		// its derivatives only, but the recursion is not stopped by
		// other panel classes. However, the recursion can be stopped by
		// an overloaded implementation of emBorder::SetLook.
		// Arguments:
		//   panel       - The panel.
		//   recursively - Whether to recurse ancestor panels.

	emColor GetBgColor() const;
	emColor GetFgColor() const;
	void SetBgColor(emColor bgColor);
	void SetFgColor(emColor fgColor);
		// Get/set back- and foreground colors of borders, labels,
		// groups and similar things.

	emColor GetButtonBgColor() const;
	emColor GetButtonFgColor() const;
	void SetButtonBgColor(emColor buttonBgColor);
	void SetButtonFgColor(emColor buttonFgColor);
		// Get/set back- and foreground colors of button faces.

	emColor GetInputBgColor() const;
	emColor GetInputFgColor() const;
	emColor GetInputHlColor() const;
	void SetInputBgColor(emColor inputBgColor);
	void SetInputFgColor(emColor inputFgColor);
	void SetInputHlColor(emColor inputHlColor);
		// Get/set background, foreground and highlight (=selection)
		// colors of editable data fields.

	emColor GetOutputBgColor() const;
	emColor GetOutputFgColor() const;
	emColor GetOutputHlColor() const;
	void SetOutputBgColor(emColor outputBgColor);
	void SetOutputFgColor(emColor outputFgColor);
	void SetOutputHlColor(emColor outputHlColor);
		// Get/set background, foreground and highlight (=selection)
		// colors of read-only data fields.

	unsigned int GetDataRefCount() const;
		// Get number of references to the internal data of this object.

	void MakeNonShared();
		// This must be called before handing the look to another
		// thread.

private:

	void DeleteData();
	void MakeWritable();

	struct SharedData {
		SharedData();
		SharedData(const SharedData & sd);
		unsigned int RefCount;
		emColor BgColor;
		emColor FgColor;
		emColor ButtonBgColor;
		emColor ButtonFgColor;
		emColor InputBgColor;
		emColor InputFgColor;
		emColor InputHlColor;
		emColor OutputBgColor;
		emColor OutputFgColor;
		emColor OutputHlColor;
	};

	SharedData * Data;

	static SharedData DefaultData;
};

#ifndef EM_NO_DATA_EXPORT
inline emLook::emLook()
{
	Data=&DefaultData;
}
#endif

inline emLook::emLook(const emLook & look)
{
	Data=look.Data;
	Data->RefCount++;
}

inline emLook::~emLook()
{
	if (!--Data->RefCount) DeleteData();
}

inline emLook & emLook::operator = (const emLook & look)
{
	look.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=look.Data;
	return *this;
}

inline bool emLook::operator != (const emLook & look) const
{
	return !(*this==look);
}

inline emColor emLook::GetBgColor() const
{
	return Data->BgColor;
}

inline emColor emLook::GetFgColor() const
{
	return Data->FgColor;
}

inline emColor emLook::GetButtonBgColor() const
{
	return Data->ButtonBgColor;
}

inline emColor emLook::GetButtonFgColor() const
{
	return Data->ButtonFgColor;
}

inline emColor emLook::GetInputBgColor() const
{
	return Data->InputBgColor;
}

inline emColor emLook::GetInputFgColor() const
{
	return Data->InputFgColor;
}

inline emColor emLook::GetInputHlColor() const
{
	return Data->InputHlColor;
}

inline emColor emLook::GetOutputBgColor() const
{
	return Data->OutputBgColor;
}

inline emColor emLook::GetOutputFgColor() const
{
	return Data->OutputFgColor;
}

inline emColor emLook::GetOutputHlColor() const
{
	return Data->OutputHlColor;
}

inline void emLook::MakeNonShared()
{
	MakeWritable();
}


#endif
