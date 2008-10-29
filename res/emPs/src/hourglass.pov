//------------------------------------------------------------------------------
// hourglass.pov
// Copyright (C) 2006, 2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "colors.inc"


camera {
	orthographic
	location <0, 0, -45>
	look_at <0, 0, 0>
}


light_source {
	<-500, 500, -2000>
	color rgb<1.0, 1.0, 1.0>
	spotlight
	radius 50
	point_at <0, 0, 0>
}


plane {
	z,
	10
	pigment { color rgb<1.0, 1.0, 1.0> }
	finish { ambient 0.5 }
}


sky_sphere {
	pigment {
		gradient y
		color_map {[0, 1  color Gray60 color Gray90]}
		rotate x*30
	}
}


#declare HGShape=
	poly {
		4,
		<
			0,   0,   0,   0,   0,  0,   0,   0,   0,  1,
			0,   0,   0,   0,   0,  0,   0,   0,   0,  0,
			1,   0,   0,   0,   0, -1,   0,   0,   0,  0,
			0,   0,   1,   0,   0
		>
		sturm
	}


object {
	object { HGShape scale 19 }
	texture {
		finish {
			ambient 0.01
			diffuse 0.01
			reflection .25
			specular 10
			roughness 0.001
		}
		pigment { color rgbf <0.7, 0.7, 0.8, 0.9> }
	}
}


#declare PassedTime=8; // 0


object {
	union {
		intersection {
			object { HGShape scale 18.8 }
			cone { <0, -100-17+PassedTime, 0>, 200, <0, -17+PassedTime, 0>, 0 }
		}
		intersection {
			object { HGShape scale 18.8 }
			difference {
				cylinder { <0,  0, 0>, <0,  12, 0>, 20 }
				cone { <0, 100+10-PassedTime, 0>, 200, <0, 10-PassedTime, 0>, 0 }
			}
		}
	}
	pigment { color rgb<0.9, 0.9, 0.3> }
	finish { ambient 0.5 }
}
