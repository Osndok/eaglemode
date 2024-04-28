//------------------------------------------------------------------------------
// emHmiDemoAnalogDisplay.h
//
// Copyright (C) 2012,2014,2016,2024 Oliver Hamann.
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

#ifndef emHmiDemoAnalogDisplay_h
#define emHmiDemoAnalogDisplay_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoAnalogDisplay : public emBorder {
public:
	emHmiDemoAnalogDisplay(
		ParentArg parent, const emString & name,
		const emString & unit=emString(),
		emInt64 minValue=0, emInt64 maxValue=10, emInt64 value=0
	);
	virtual ~emHmiDemoAnalogDisplay();

	void SetMinValue(emInt64 minValue);
	void SetMaxValue(emInt64 maxValue);
	void SetMinMaxValues(emInt64 minValue, emInt64 maxValue);

	void SetValue(emInt64 value);

	void SetScaleMarkIntervals(
		emUInt64 scaleMarkInterval1, emUInt64 scaleMarkInterval2
	);

	void SetRadix(emUInt64 radix);
	void SetAnalogDigitsAfterRadix(unsigned analogDigitsAfterRadix);
	void SetDigitalDigitsAfterRadix(unsigned digitalDigitsAfterRadix);

	void SetTextBoxTallness(double textBoxTallness);

	void AddColoredRange(emInt64 startValue, emInt64 endValue, emColor color);

	void SetAnimation(emInt64 value, emInt64 amplitude, double frequency);

protected:
	virtual bool Cycle();
	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;
private:
	struct ColoredRange {
		emInt64 StartValue;
		emInt64 EndValue;
		emColor Color;
	};
	emString Unit;
	emInt64 MinValue,MaxValue;
	emInt64 Value;
	emUInt64 ScaleMarkInterval1;
	emUInt64 ScaleMarkInterval2;
	emUInt64 Radix;
	unsigned AnalogDigitsAfterRadix;
	unsigned DigitalDigitsAfterRadix;
	double TextBoxTallness;
	emArray<ColoredRange> ColoredRanges;
	emInt64 AnimValue;
	emInt64 AnimAmplitude;
	double AnimFrequency;
	emTimer Timer;
};


#endif
