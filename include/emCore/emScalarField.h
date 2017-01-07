//------------------------------------------------------------------------------
// emScalarField.h
//
// Copyright (C) 2005-2010,2014,2016 Oliver Hamann.
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

#ifndef emScalarField_h
#define emScalarField_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//=============================== emScalarField ================================
//==============================================================================

class emScalarField : public emBorder {

public:

	// Class for a data field panel showing a scalar value which can
	// optionally be edited by the user. The scalar value is a 64-bit signed
	// integer number, but on the shown scale the values can be translated
	// to any texts (e.g. rational numbers, names ,...).

	emScalarField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		emInt64 minValue=0, emInt64 maxValue=10, emInt64 value=0,
		bool editable=false
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   minValue    - Minimum allowed value.
		//   maxValue    - Maximum allowed value.
		//   value       - Initial value.
		//   editable    - Whether the value can be edited by the user.

	virtual ~emScalarField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the value can be edited by the user.

	emInt64 GetMinValue() const;
	emInt64 GetMaxValue() const;
	void SetMinValue(emInt64 minValue);
	void SetMaxValue(emInt64 maxValue);
	void SetMinMaxValues(emInt64 minValue, emInt64 maxValue);
		// Get/set the range of possible values.

	const emSignal & GetValueSignal() const;
		// This signal is signaled after each change of the value.

	emInt64 GetValue() const;
	void SetValue(emInt64 value);
		// Get/set the value.

	const emArray<emUInt64> & GetScaleMarkIntervals() const;
	void SetScaleMarkIntervals(const emArray<emUInt64> & intervals);
	void SetScaleMarkIntervals(unsigned interval1, unsigned interval2, ...);
		// Get/set the layout of scale marks. It is an array of
		// intervals between the scale marks of different size levels.
		// The first interval is for the largest scale marks, the second
		// is for the second-largest scale marks and so on. Thus, the
		// array must be sorted from large to small. For example, a
		// classic centimeter rule would have {10,5,1}, with the values
		// in millimeters. The default is {1}. The arguments to the
		// ellipse version method must be terminated by a 0. Note that
		// the ellipse version can take only 32-bit intervals (I am not
		// sure whether an ellipse on emUInt64 would be portable).

	bool IsNeverHidingMarks() const;
	void SetNeverHideMarks(bool neverHide);
		// By default, marks with an interval greater than
		// (MaxValue - MinValue) are automatically not shown, so that
		// the other marks can be seen better. Setting true here
		// disables that automatism.

	virtual void TextOfValue(char * buf, int bufSize, emInt64 value,
	                         emUInt64 markInterval) const;
		// Convert a scale mark value to a null-terminated character
		// string for display on the scale. The default implementation
		// uses the callback function set with SetTextOfValueFunc.

	void SetTextOfValueFunc(
		void(*textOfValueFunc)(
			char * buf, int bufSize, emInt64 value,
			emUInt64 markInterval, void * context
		),
		void * context=NULL
	);
		// Set a function for converting a scale mark value to a
		// null-terminated character string for display on the scale.
		// The context argument is forwarded to the function for any
		// use. The default performs simple decimal conversion.

	static void DefaultTextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
		// This is the default text-of-value-function. It performs
		// decimal conversion.

	double GetTextBoxTallness() const;
	void SetTextBoxTallness(double textBoxTallness);
		// Tallness of the text box of a scale mark. The default is 0.5.

	emUInt64 GetKeyboardInterval() const;
	void SetKeyboardInterval(emUInt64 kbInterval);
		// How much to add and sub to the value by '+' and '-' keys. The
		// default is zero which means to choose a good interval
		// automatically.

protected:

	virtual void ValueChanged();
		// Called when the value has changed.

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo() const;
	virtual emString GetHowTo() const;

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

	virtual bool CheckMouse(double mx, double my, emInt64 * pValue) const;

private:

	enum DoScalarFieldFunc {
		SCALAR_FIELD_FUNC_PAINT,
		SCALAR_FIELD_FUNC_CHECK_MOUSE
	};
	void DoScalarField(
		DoScalarFieldFunc func, const emPainter * painter,
		emColor canvasColor,
		double mx, double my, emInt64 * pValue, bool * pHit
	) const;

	void StepByKeyboard(int dir);

	bool Editable;
	emSignal ValueSignal;
	emInt64 MinValue,MaxValue;
	emInt64 Value;
	emArray<emUInt64> ScaleMarkIntervals;
	bool MarksNeverHidden;
	void(*TextOfValueFunc)(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	void * TextOfValueFuncContext;
	double TextBoxTallness;
	emUInt64 KBInterval;
	bool Pressed;

	static const char * HowToScalarField;
	static const char * HowToReadOnly;
};

inline bool emScalarField::IsEditable() const
{
	return Editable;
}

inline emInt64 emScalarField::GetMinValue() const
{
	return MinValue;
}

inline emInt64 emScalarField::GetMaxValue() const
{
	return MaxValue;
}

inline const emSignal & emScalarField::GetValueSignal() const
{
	return ValueSignal;
}

inline emInt64 emScalarField::GetValue() const
{
	return Value;
}

inline const emArray<emUInt64> & emScalarField::GetScaleMarkIntervals() const
{
	return ScaleMarkIntervals;
}

inline bool emScalarField::IsNeverHidingMarks() const
{
	return MarksNeverHidden;
}

inline double emScalarField::GetTextBoxTallness() const
{
	return TextBoxTallness;
}

inline emUInt64 emScalarField::GetKeyboardInterval() const
{
	return KBInterval;
}


#endif
