//------------------------------------------------------------------------------
// emWorldClockPanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emWorldClockPanel_h
#define emWorldClockPanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emTimeZonesModel_h
#include <emClock/emTimeZonesModel.h>
#endif

#ifndef emClockFileModel_h
#include <emClock/emClockFileModel.h>
#endif

class emClockPanel;


class emWorldClockPanel : public emFilePanel {

public:

	emWorldClockPanel(ParentArg parent, const emString & name,
	                  emClockFileModel * fileModel);

	virtual ~emWorldClockPanel();

	virtual emString GetTitle();

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque();

	virtual void Paint(const emPainter & painter, emColor canvasColor);

	virtual void LayoutChildren();

private:

	void TransformCoords(double * pX, double * pY,
	                     double latitude, double longitude) const;

	double CalcEarthWidth() const;
	double CalcEarthHeight() const;
	double CalcClockMinRadius() const;
	double CalcClockMaxRadius() const;

	void CreateOrDestroyChildren();

	void UpdateSunPosition();

	void PreparePolygons(bool shadowOnly);
	void PrepareWaterPolygon(int n);
	void PrepareLandPolygons();
	void PrepareShadowPolygon(int n);

	static int CmpClockPanelX(
		emClockPanel * const * p1, emClockPanel * const * p2,
		void * context
	);

	emRef<emClockFileModel> FileModel;
	emRef<emTimeZonesModel> TimeZonesModel;

	emArray<emClockPanel*> Children;

	double SunLatitude;
	double SunLongitude;

	emArray<double> WaterPolygon;
	emArray<emArray<double> > LandPolygons;
	emArray<double> ShadowPolygon;

	static const emInt16 MapData1[];
	static const emInt16 MapData2[];
	static const emInt16 MapData3[];
		// These arrays contain the polygons of all the continents and
		// islands of the world map, with different resolutions.
		// The format is:
		//   <polygon> <polygon> <polygon> ... 0
		// Where each <polygon> is:
		//   <n> <x1> <y1> <x2> <y2> ... <xn> <yn>
		// n is the number of vertices (>=3). Vertex coordinates are
		// x = longitude and y = latitude in degrees/100.
};


#endif
