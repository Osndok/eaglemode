#include "borders.inc"


camera {
	orthographic
	location <0, 0, 1650>
	look_at <0, 0, 0>
	sky <0,-1,0>
}

light_source {
	<-1100000, -1500000, 1700000>
	#declare light_value=1.0;
	color rgb <light_value,light_value,light_value>
}

sky_sphere {
	pigment {
		gradient y
		color_map {
			[ 0.0  color rgb <0.8,0.8,0.8> ]
			[ 0.5  color rgb <0.5,0.5,0.5> ]
			[ 1.0  color rgb <0.08,0.08,0.08> ]
		}
		scale 2
		translate -1
	}
}

object {
	plane { z, 0 }
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

#declare obj_texture =
texture {
	pigment {
		color rgbf 1
	}
	finish  {
		ambient 0.01
		brilliance 1
		diffuse 0.01
		specular 2.0
		roughness 0.02
		reflection 0.15
	}
}

#declare obj_interior =
interior {
	ior 1.7
	caustics 1.0
	//dispersion 1.05
	//dispersion_samples : 7
	fade_distance 4.0
	fade_power 2.0
	fade_color <1.0, 0.0, 0.0>
}

