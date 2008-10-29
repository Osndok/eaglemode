//------------------------------------------------------------------------------
// borders-sample-4.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"
#include "glass.inc"


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

plane { z, 10
	pigment { rgb <1,1,1> }
	finish {
		ambient 0.3
		diffuse 0.9
	}
}

cylinder {
	<0,0,0>
	<0,0,10.1>
	23
	pigment { rgb <0.05,0.05,0.05> }
	finish {
		ambient 0.3
		diffuse 0.9
	}
}

object {
	merge {
		BezierBorderC(40,40,10,10,20,90,0,0)
		RectBorderC(50,50,100,20,0)
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
		cylinder {
			<0,0,0>
			<0,0,25>
			40
		}
		sphere {
			<0,0,0>
			40
			scale <1,1,0.2>
			translate <0,0,25>
		}
	}
	material {
		texture {
			T_Glass3
		}
	}
}
