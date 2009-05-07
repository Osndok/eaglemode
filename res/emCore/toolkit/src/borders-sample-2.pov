//------------------------------------------------------------------------------
// borders-sample-2.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


light_source {
	<-1000, -1000, 1000>
	#declare light_value=1.0;
	color rgb <light_value,light_value,light_value>
}

camera {
	orthographic
	location <0, 0, 102>
	look_at <0, 0, 0>
	sky <0,-1,0>
}

sky_sphere {
	pigment {
		gradient y
		color_map {
			[ 0.0  color rgb <0.8,0.8,1> ]
			[ 0.5  color rgb <0.5,0.5,0.5> ]
			[ 1.0  color rgb <0,0,0> ]
		}
		scale 2
		translate -1
	}
}

object {
	merge {
		RectBorderC(1,1,36,3,10)
		BezierBorderC(37,37,5, 3,0, 0,75, 10)
		BezierBorderC(40,40,5, 0,3, -75,0, 10)
		RectBorderC(45,45,100,3,10)
	}
	pigment { rgb <1,1,1> }
	finish {
		specular 0.8
		roughness 0.09
		ambient 0.2
		diffuse 0.4
	}
}

object {
	plane { z, 0.01 }
	pigment { rgb <0,0,0> }
	finish {
		specular 0.0
		roughness 0.09
		ambient 0.0
		diffuse 0.0
	}
}
