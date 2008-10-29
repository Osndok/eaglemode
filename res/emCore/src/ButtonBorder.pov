//------------------------------------------------------------------------------
// ButtonBorder.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 210>
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
		BezierBorderC(66,66,1,23.1,23,0,-10,5)
		BezierBorderC(67,67,6,23,22,-10,-10,5)
		BezierBorderC(73,73,2,22,20,-10,-90,5)
		RectBorderC(59,59,200,20.1,5)
		RectBorderC(5,5,61,23.1,5)
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
