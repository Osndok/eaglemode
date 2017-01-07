//------------------------------------------------------------------------------
// emTunnel.h
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

#ifndef emTunnel_h
#define emTunnel_h

#ifndef emBorder_h
#include <emCore/emBorder.h>
#endif


//==============================================================================
//================================== emTunnel ==================================
//==============================================================================

class emTunnel : public emBorder {

public:

	// This panel shows a single child panel very small. Around that, a
	// decoration is painted which looks like a tunnel. Therefore the name
	// of this class. The single child panel is laid out automatically
	// whenever it is created by the user of this class.

	emTunnel(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.

	double GetChildTallness() const;
	void SetChildTallness(double childTallness);
		// Tallness for the child panel (end of tunnel). A value <=0.0
		// means to take the tallness of the content rectangle. That is
		// the default.

	double GetDepth() const;
	void SetDepth(double depth);
		// Depth of the tunnel. The formula is more or less:
		//   AreaOfEnd = AreaOfEntrance/((Depth+1)*(Depth+1))
		// The default is 10.0.

	virtual void GetChildRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) const;
		// Get coordinates and canvas color of the end of the tunnel.

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
	virtual void GetChildRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

protected:

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) const;

	virtual void LayoutChildren();

private:

	enum DoTunnelFunc {
		TUNNEL_FUNC_PAINT,
		TUNNEL_FUNC_CHILD_RECT
	};
	void DoTunnel(
		DoTunnelFunc func, const emPainter * painter,
		emColor canvasColor, double * pX, double * pY, double * pW,
		double * pH, emColor * pCanvasColor
	) const;

	double ChildTallness,Depth;
};

inline double emTunnel::GetChildTallness() const
{
	return ChildTallness;
}

inline double emTunnel::GetDepth() const
{
	return Depth;
}


#endif
