//------------------------------------------------------------------------------
// SplitterPressed.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "borders.inc"


camera {
	orthographic
	location <0, 0, 36>
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

#declare quarter = merge {
		bicubic_patch {
			type 1 flatness 0.001
			u_steps 4 v_steps 4
			uv_vectors
			<0,0> <1,0> <1,1> <0,1>
			< 0, 0, 0> < 5, 0, 0> < 5, 0, 5> <10, 0, 5>
			< 0, 5, 0> < 5, 5, 0> < 5, 5, 5> <10, 5, 5>
			< 0, 5, 5> < 5, 5, 5> < 8, 8, 5> <10, 5, 5>
			< 0,10, 5> < 5,10, 5> < 5,10, 5> <10,10, 5>
			uv_mapping
		}
		bicubic_patch {
			type 1 flatness 0.001
			u_steps 4 v_steps 4
			uv_vectors
			<0,0> <1,0> <1,1> <0,1>
			<10, 0, 5> <15, 0, 5> <17, 0, 0> <17, 0, 0>
			<10, 0, 5> <15, 0, 5> <17, 0, 0> <17, 0, 0>
			<10,10, 5> <15,10, 5> <17,10, 0> <17,10, 0>
			<10,10, 5> <15,10, 5> <17,10, 0> <17,10, 0>
			uv_mapping
		}
		bicubic_patch {
			type 1 flatness 0.001
			u_steps 4 v_steps 4
			uv_vectors
			<0,0> <1,0> <1,1> <0,1>
			< 0,10, 5> < 0,10, 5> <10,10, 5> <10,10, 5>
			< 0,15, 5> < 0,15, 5> <10,15, 5> <10,15, 5>
			< 0,17, 0> < 0,17, 0> <10,17, 0> <10,17, 0>
			< 0,17, 0> < 0,17, 0> <10,17, 0> <10,17, 0>
			uv_mapping
		}
		bicubic_patch {
			type 1 flatness 0.001
			u_steps 4 v_steps 4
			uv_vectors
			<0,0> <1,0> <1,1> <0,1>
			<10,10, 5> <15,10, 5> <17,10, 0> <17,10, 0>
			<10,15, 5> <15,15, 5> <17,15, 0> <17,15, 0>
			<10,17, 0> <15,17, 0> <17,17, 0> <17,17, 0>
			<10,17, 0> <15,17, 0> <17,17, 0> <17,17, 0>
			uv_mapping
		}
}

//#declare button_pos=19.5;
#declare button_pos=13;

object {
	merge {
		RectBorder(17,17,100,20)
		object { quarter scale < 1, 1,0.8> translate <0,0,button_pos> }
		object { quarter scale <-1, 1,0.8> translate <0,0,button_pos> }
		object { quarter scale < 1,-1,0.8> translate <0,0,button_pos> }
		object { quarter scale <-1,-1,0.8> translate <0,0,button_pos> }
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
