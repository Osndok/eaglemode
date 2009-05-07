//------------------------------------------------------------------------------
// IOField.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 45>
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

#declare R=7;
#declare D=3;

object {
	difference {
		merge {
			ElliBorder2C(10,10,10,15,18,R)
			RectBorderC ( 0, 0,15,   18,R)
		}
		plane { <0,0,1> 0.1 }
		ElliBorder2C(10-D,10-D,10,15-D,18-D,R)
		RectBorderC ( 0  , 0  ,   15-D,18-D,R)
	}
	pigment {
		color rgbf 1
	}
	finish  {
		ambient 0.1
		brilliance 1
		diffuse 0.6
		specular 0.8
		roughness 0.01
		reflection 0.5
	}
	interior {
		ior 1.4
		fade_color <1, 1, 0.0>
	}
}

object {
	merge {
		plane { <0,0,1> 0 }
		ElliBorder2C(20,20,  2,11,12,R)
		RectBorderC (21,21,100,   12,R)
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

