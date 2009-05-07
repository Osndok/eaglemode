//------------------------------------------------------------------------------
// CheckBoxPressed.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 110>
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

//#declare button_pos=17;
#declare button_pos=5;

object {
	merge {
		BezierBorderC(30,30,10,button_pos+20,button_pos+10,0,-90,30)
		box { <-30.1,-30.1,button_pos> <30.1,30.1,button_pos+20> }
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
		plane { z, button_pos }
		BezierBorderC(40,40,10,20,30,90,0,30)
		RectBorderC(50,50,100,30,30)
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
