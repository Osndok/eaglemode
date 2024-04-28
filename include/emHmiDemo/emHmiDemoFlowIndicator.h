//------------------------------------------------------------------------------
// emHmiDemoFlowIndicator.h
//
// Copyright (C) 2012,2016,2024 Oliver Hamann.
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

#ifndef emHmiDemoFlowIndicator_h
#define emHmiDemoFlowIndicator_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoFlowIndicator : public emPanel {
public:
	emHmiDemoFlowIndicator(
		ParentArg parent, const emString & name, double rpm, int shape=0
	);
	virtual ~emHmiDemoFlowIndicator();
	void SetRPM(double rpm);
	void SetShape(int shape);
protected:
	virtual bool Cycle();
	virtual bool IsOpaque() const;
	virtual void Paint(const emPainter & painter, emColor canvasColor) const;
private:
	double RPM;
	int Shape;
	double Angle;
	emUInt64 Time;
	emTimer Timer;
	emColor Color1,Color2;
	emImage Border;
};


#endif
