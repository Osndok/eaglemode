#include "colors.inc"

camera {
	location <40, 150, -120>
	look_at <0, 0, 0>
	focal_point <0,0,0>
	aperture 0.9
	blur_samples 20
}

light_source {
	<-1100000, 1500000, 1700000>
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
	plane { y, 0 }
	texture {
		pigment { color rgb <0.10, 0.11, 0.15> }
		normal { bumps 0.02 scale 3 }
		finish {
			diffuse 0.2
			specular 0.2
			roughness 0.1
		}
	}
	translate <clock,0,0>
}

#macro Pie(sx,sz,sr,sh,sd,se)
	merge {
		cylinder {
			<sx, se, sz>,
	    <sx, sh-sd, sz>,
	    sr
		}
		cylinder {
			<sx, se, sz>,
	    <sx, sh, sz>,
	    sr-sd
		}
		torus {
			sr-sd
			sd
			translate <sx,sh-sd,sz>
		}
		torus {
			sr-se
			se
			translate <sx,se,sz>
		}
	}
#end

#declare cx=-4;
#while (cx < 5)
	#declare cz=-4;
	#while (cz < 5)
		object {
			Pie(cx*100,cz*100,37,6,2,1)
			texture {
				pigment { color rgb <0.7,0.64,0.47> }
				normal { bumps 0.02 scale 1 }
				finish {
					diffuse 1
					specular 0.2
					roughness 0.1
				}
			}
			translate <clock,0,0>
		}
		#declare cz=cz+1;
	#end
	#declare cx=cx+1;
#end
