//------------------------------------------------------------------------------
// GroupBorder.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 104>
	look_at <0, 0, 0>
	sky <0,-1,0>
}

light_source {
	<-1000, -1500, 1700>
	#declare light_value=1.0;
	color rgb <light_value,light_value,light_value>
}

sky_sphere {
	pigment {
		gradient y
		color_map {
			[ 0.0  color rgb <0.8,0.8,1> ]
			[ 0.5  color rgb <0.5,0.5,0.5> ]
			[ 1.0  color rgb <0.08,0.08,0> ]
		}
		scale 2
		translate -1
	}
}

object {
	merge {
		BezierBorderC(42,42,1,23.1,23,0,-10,17)
		BezierBorderC(43,43,6,23,22,-10,-10,17)
		BezierBorderC(49,49,2,22,20,-10,-90,17)
		RectBorderC(35,35,100,20.1,17)
		RectBorderC(17,17,25,23.1,17)
	}
	pigment {
		color rgb <0.50, 0.50, 0.50>
	}
	finish {
		ambient 0.1
		brilliance 1
		diffuse 0.6
		metallic
		specular 0.8
		roughness 0.04
		reflection 0.5
	}
}
