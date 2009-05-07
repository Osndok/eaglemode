//------------------------------------------------------------------------------
// borders-sample-1.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"
#include "colors.inc"
#include "metals.inc"
#include "glass.inc"


light_source {
	<-3000, -4000, 1000>
	color White
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

plane { z, 0.1
	pigment { rgb <0.75,0.75,0.75> }
	finish {
		ambient 0.2
		diffuse 0.8
	}
}

object {
	merge {
		BezierBorderC(49,29,1,3,4,90,-5,30)
		BezierBorderC(50,30,12,4,2,-5,-10,30)
		BezierBorderC(62,42,2,2,0,-10,-90,30)
	}
	texture { T_Chrome_3E }
}

object {
	merge {
		box { <-30.0001,-10.0001,0> <30.0001,10.0001,10> }
		BezierBorderC(30,10,19,10,4,0,-90,30)
	}
	texture { T_Vicksbottle_Glass }
}
