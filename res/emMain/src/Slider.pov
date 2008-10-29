//------------------------------------------------------------------------------
// Slider.pov
// Copyright (C) 2007-2008 Oliver Hamann.
//------------------------------------------------------------------------------

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

#macro Pile(v1,v2,r)
	merge {
		cylinder { v1 v2 r }
		sphere { v1 r }
		sphere { v2 r }
	}
#end

object {
	merge {
		Pile     ( <-12,-15,  0>, < 12,-15,  0>, 5 )
		Pile     ( <-12, 15,  0>, < 12, 15,  0>, 5 )
		cylinder { <-12, 15,  0>, <-12,-15,  0>, 5 }
		cylinder { < 12, 15,  0>, < 12,-15,  0>, 5 }
		box      { < 12, 15,  5>, <-12,-15, -5> }
		Pile(<-12,-15,4>, < 12,-15,4>, 2)
		Pile(<-12, -9,4>, < 12, -9,4>, 2)
		Pile(<-12, -3,4>, < 12, -3,4>, 2)
		Pile(<-12,  3,4>, < 12,  3,4>, 2)
		Pile(<-12,  9,4>, < 12,  9,4>, 2)
		Pile(<-12, 15,4>, < 12, 15,4>, 2)
		scale <1,1,0.6>
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
