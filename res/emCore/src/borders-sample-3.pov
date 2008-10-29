//------------------------------------------------------------------------------
// borders-sample-3.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


light_source {
	<-1000, -1000, 1500>
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

plane { z, 18
	pigment { rgb <1,1,1> }
	finish {
		ambient 0.3
		diffuse 0.9
	}
}

box {
	<-48,-12,0>
	<-46.5,12,18.1>
	pigment { rgb <0.05,0.05,0.05> }
	finish {
		ambient 0.3
		diffuse 0.9
	}
}

object {
	merge {
		BezierBorderC(55,15,3,20,23,90,0,51)
		RectBorderC(58,18,100,23,51)
	}

	pigment { rgb <0.5,0.5,0.5> }
	finish {
		specular 0.7
		roughness 0.08
		ambient 0.3
		diffuse 0.6
	}
}

object {
	merge {
		BezierBorderC(51,11,4,23,20,0,-90,51)
		box { <-51.1,-11.1,0> <51.1,11.1,23> }
	}
	pigment { color rgbf <0.98, 0.98, 0.98, 0.9> }
	finish  {
		ambient 0.1
		diffuse 0.1
		reflection 0.1
		specular 0.8
		roughness 0.011
		phong 1
		phong_size 400
	}
}
