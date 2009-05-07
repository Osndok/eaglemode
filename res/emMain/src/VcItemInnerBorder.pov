//------------------------------------------------------------------------------
// VcItemInnerBorder.pov
// Copyright (C) 2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "../../emCore/toolkit/src/borders.inc"


camera {
	orthographic
	location <0, 0, 40>
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
		plane { <0,0,1> 0 }
		object {
			BezierBorderC(1,6,3,0,2,90,0,100)
			scale <6,1,1>
		}
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
