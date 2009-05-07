//------------------------------------------------------------------------------
// Button.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 78>
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

#declare button_pos=33;
//#declare button_pos=11;
//#declare button_pos=9;

object {
	merge {
		RectBorderC  (32,32,100,20.1,14)
		BezierBorderC(32,32,1,22.1,23.1,90,0,14)
		BezierBorderC(33,33,100,23.1,23.1,0,0,14)

		BezierBorderC(14,14,18,button_pos,button_pos-6.5,0,-90,14)
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
