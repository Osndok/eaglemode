//------------------------------------------------------------------------------
// emColorField.h
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

#ifndef emColorField_h
#define emColorField_h

#ifndef emScalarField_h
#include <emCore/emScalarField.h>
#endif

#ifndef emTextField_h
#include <emCore/emTextField.h>
#endif

#ifndef emRasterLayout_h
#include <emCore/emRasterLayout.h>
#endif


//==============================================================================
//================================ emColorField ================================
//==============================================================================

class emColorField : public emBorder {

public:

	// Class for a data field panel showing a color which can optionally be
	// edited by the user.

	emColorField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		emColor color=0x000000ff, bool editable=false,
		bool alphaEnabled=false
	);
		// Constructor.
		// Arguments:
		//   parent       - Parent for this panel (emPanel or emView).
		//   name         - The name for this panel.
		//   caption      - The label's caption, or empty.
		//   description  - The label's description, or empty.
		//   icon         - The label's icon, or empty.
		//   color        - Initial color.
		//   editable     - Whether the color can be edited by the user.
		//   alphaEnabled - Whether the alpha channel of the color is
		//                  relevant.

	virtual ~emColorField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the color can be edited by the user.

	bool IsAlphaEnabled() const;
	void SetAlphaEnabled(bool alphaEnabled=true);
		// Whether the alpha channel of the color is relevant.

	const emSignal & GetColorSignal() const;
		// This signal is signaled after each change of the color.

	emColor GetColor() const;
	void SetColor(emColor value);
		// Get/set the color.

protected:

	virtual void ColorChanged();
		// Called when the color has changed.

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();
	virtual void LayoutChildren();

	virtual bool HasHowTo() const;
	virtual emString GetHowTo() const;

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

private:

	void UpdateRGBAOutput();
	void UpdateHSVOutput(bool initial=false);
	void UpdateNameOutput();
	void UpdateExpAppearance();

	static void TextOfPercentValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfHueValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);

	struct Expansion {
		emRasterLayout * Layout;
		emScalarField * SfRed;
		emScalarField * SfGreen;
		emScalarField * SfBlue;
		emScalarField * SfAlpha;
		emScalarField * SfHue;
		emScalarField * SfSat;
		emScalarField * SfVal;
		emTextField * TfName;
		emInt64 RedOut,GreenOut,BlueOut,AlphaOut,HueOut,SatOut,ValOut;
		emString NameOut;
	};

	emSignal ColorSignal;
	Expansion * Exp;
	emColor Color;
	bool Editable;
	bool AlphaEnabled;
	bool Pressed;

	static const char * HowToColorField;
	static const char * HowToReadOnly;
};

inline bool emColorField::IsEditable() const
{
	return Editable;
}

inline bool emColorField::IsAlphaEnabled() const
{
	return AlphaEnabled;
}

inline const emSignal & emColorField::GetColorSignal() const
{
	return ColorSignal;
}

inline emColor emColorField::GetColor() const
{
	return Color;
}


#endif
