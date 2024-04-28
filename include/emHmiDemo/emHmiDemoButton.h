//------------------------------------------------------------------------------
// emHmiDemoButton.h
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

#ifndef emHmiDemoButton_h
#define emHmiDemoButton_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoButton : public emBorder {
public:
	emHmiDemoButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
	virtual ~emHmiDemoButton();

	bool IsChecked() const;
	void SetChecked(bool checked=true);

	void SetAnimation(double blinkFrequency);

protected:
	virtual bool Cycle();
	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;
private:
	bool Checked;
	emImage ButtonOffImage;
	emImage ButtonOnImage;
	emImage ButtonLightImage;
	double BlinkFrequency;
	emTimer Timer;
};


inline bool emHmiDemoButton::IsChecked() const
{
	return Checked;
}


#endif
