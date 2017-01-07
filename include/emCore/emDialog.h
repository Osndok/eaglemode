//------------------------------------------------------------------------------
// emDialog.h
//
// Copyright (C) 2005-2010,2014-2016 Oliver Hamann.
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

#ifndef emDialog_h
#define emDialog_h

#ifndef emButton_h
#include <emCore/emButton.h>
#endif

#ifndef emLinearLayout_h
#include <emCore/emLinearLayout.h>
#endif


//==============================================================================
//================================== emDialog ==================================
//==============================================================================

class emDialog : public emWindow {

public:

	// Class for a dialog window. Such a dialog has a content area and a
	// button area. The content area is an emLinearLayout which can be given
	// individual child panels. The button area can have buttons like "OK"
	// and "Cancel" for finishing the dialog.

	emDialog(
		emContext & parentContext,
		ViewFlags viewFlags=VF_POPUP_ZOOM|VF_ROOT_SAME_TALLNESS,
		WindowFlags windowFlags=WF_MODAL,
		const emString & wmResName="emDialog"
	);
		// Like the constructor of emWindow, but see that the default
		// argument values are different (it's a modal dialog with
		// popup-zoom by default).

	virtual ~emDialog();
		// Destructor.

	void SetRootTitle(const emString & title);
		// Set the title for this dialog. More precise, set the title
		// for the private root panel of this view. If you create some
		// content panel with another title, and if it gets focus, that
		// title is shown. The default title is an empty string.

	emLinearLayout * GetContentPanel() const;
		// This panel makes up the content area of the dialog, not
		// including the buttons. For convenience, it is an emLinearLayout
		// with default properties, except that the inner border is set
		// to emBorder::IBT_CUSTOM_RECT. You may change the properties
		// as you wish, and you should give it one or more child panels
		// as the content.

	void AddPositiveButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
	void AddNegativeButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
	void AddCustomButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Add a button to the button area. These buttons are finishing
		// the dialog. For the meaning of "Positive", "Negative" and
		// "Custom", please see GetResult().

	void AddOKButton();
	void AddCancelButton();
	void AddOKCancelButtons();
		// AddOKButton() is like AddPositiveButton("OK").
		// AddCancelButton() is like AddNegativeButton("Cancel").
		// AddOKCancelButtons() is like AddOKButton() plus
		// AddCancelButton().

	emButton * GetButton(int index) const;
		// Get a button. The index is: 0 for the first added button, 1
		// for the second added button, and so on.

	emButton * GetButtonForResult(int result) const;
		// Get the first button, whose result is the given result.

	emButton * GetOKButton() const;
		// Get the first button with positive result.

	emButton * GetCancelButton() const;
		// Get the first button with negative result.

	const emSignal & GetFinishSignal() const;
		// Signaled when any of the buttons has been triggered, or by
		// pressing the Enter key or the Escape key, or by the window
		// close signal. It is okay not to destruct the dialog and to
		// wait for another finish signal.

	enum {
		// Possible results:
		POSITIVE=1, // Positive button triggered or Enter key pressed.
		NEGATIVE=0, // Negative button triggered or Escape key pressed
		            // or window-closing commanded (see GetCloseSignal).
		CUSTOM1 =2, // First custom button triggered.
		CUSTOM2 =3, // Second custom button triggered.
		CUSTOM3 =4  // ...
		// Continued (customIndex=result+1-CUSTOM1)
	};
	int GetResult() const;
		// The result should be asked after the finish signal has been
		// signaled. Before that, the result is not valid.

	bool Finish(int result);
		// Finish this dialog with the given result programmatically.
		// Returns true on success, or false if the finishing was aborted
		// by a call to CheckFinish(result).

	void EnableAutoDeletion(bool autoDelete=true);
	bool IsAutoDeletionEnabled() const;
		// Whether to delete this object automatically a few time slices
		// after the dialog has finished.

	static void ShowMessage(
		emContext & parentContext,
		const emString & title,
		const emString & message,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// This function creates a modal dialog with an emLabel as the
		// content, and with an OK button. The dialog deletes itself
		// when finished. The argument 'message' is the caption of the
		// label.

protected:

	virtual bool CheckFinish(int result);
		// Check whether finishing is allowed with the given result at
		// this moment. The default implementation always returns true.

	virtual void Finished(int result);
		// Like the finish signal. Default implementation does nothing.
		// It's allowed to delete (destruct) this dialog herein.

private:

	bool PrivateCycle();

	class DlgButton : public emButton {
	public:
		DlgButton(
			ParentArg parent, const emString & name,
			const emString & caption,
			const emString & description,
			const emImage & icon,
			int result
		);
		int GetResult() const;
	protected:
		virtual void Clicked();
	private:
		int Result;
	};

	class DlgPanel : public emBorder {
	public:
		DlgPanel(ParentArg parent, const emString & name);
		virtual ~DlgPanel();
		void SetTitle(const emString & title);
		virtual emString GetTitle() const;
		emString Title;
		emLinearLayout * ContentPanel;
		emLinearLayout * ButtonsPanel;
	protected:
		virtual void Input(
			emInputEvent & event, const emInputState & state,
			double mx, double my
		);
		virtual void LayoutChildren();
	};

	class PrivateEngineClass : public emEngine {
	public:
		PrivateEngineClass(emDialog & dlg);
	protected:
		virtual bool Cycle();
		emDialog & Dlg;
	};
	friend class PrivateEngineClass;

	PrivateEngineClass PrivateEngine;
	emSignal FinishSignal;
	int Result;
	int ButtonNum,CustomRes;
	int FinishState;
	bool ADEnabled;
};

inline emLinearLayout * emDialog::GetContentPanel() const
{
	return ((DlgPanel*)GetRootPanel())->ContentPanel;
}

inline emButton * emDialog::GetOKButton() const
{
	return GetButtonForResult(POSITIVE);
}

inline emButton * emDialog::GetCancelButton() const
{
	return GetButtonForResult(NEGATIVE);
}

inline const emSignal & emDialog::GetFinishSignal() const
{
	return FinishSignal;
}

inline int emDialog::GetResult() const
{
	return Result;
}

inline bool emDialog::IsAutoDeletionEnabled() const
{
	return ADEnabled;
}

inline int emDialog::DlgButton::GetResult() const
{
	return Result;
}


#endif
