//------------------------------------------------------------------------------
// pieces.pov - Development of the pieces for SilChess
// Copyright (C) 2001, 2008 Oliver Hamann.
//------------------------------------------------------------------------------

#include "colors.inc"


camera {
	location <0, 1400, -1200>
	right <4/3, 0, 0>
	up <0, 1, 0>
	sky <0, 1, 0>
	direction <0, 0, 2.6>
	look_at <0, 0,-50>
}

/*
camera {
	location <0, 200, -1500>
	right <4/3, 0, 0>
	up <0, 1, 0>
	sky <0, 1, 0>
	direction <0, 0, 5.6>
	look_at <0, 60, 0>
}
*/

background { color <0.3,0.4,0.5> }

light_source { <-1000, 1400,  -500> colour White }

intersection {
	plane {  y, 0 }
	plane {  x, 400 }
	plane { -x, 400 }
	plane {  z, 400 }
	plane { -z, 400 }
	pigment {
		checker colour <0.2,0.4,0.4> colour <0.3,0.6,0.6>
		scale 100
	}
	finish {
		ambient 0.3
		diffuse 0.2
		reflection 0.5
	}
}


#declare Pawn = union {
	sphere { < 0, 0, 0>, 19 }
	sphere { < 0,16, 0>, 12 }
	sphere { < 0,37, 0>, 12 }
}

#declare Knight = union {
	sphere { <  0,  0,  0>, 22 }
	sphere { <  3, 25,  0>, 16 }
	sphere { <  6, 45,  0>, 18 }
	sphere { < -2, 43,  0>, 16 }
	sphere { <-10, 40,  0>, 14 }
	sphere { <-16, 37,  0>, 12 }
	sphere { < 14, 60,-10>,  6 }
	sphere { < 14, 60, 10>,  6 }
	rotate 45*y
}

#declare Bishop = union {
	sphere { < 0, 0, 0>, 22 }
	sphere { < 0,21, 0>, 15 }
	sphere { < 0,37, 0>, 14 }
	sphere { < 0,53, 0>, 17 }
	sphere { < 0,68, 0>,  9 }
}

#declare Rook = union {
	sphere { <  0,  0,  0>, 24 }
	sphere { <  0, 17,  0>, 20 }
	sphere { <  0, 35,  0>, 19 }
	sphere { <  0, 50,  0>, 21 }
	sphere { < 11, 60,  0>, 10 }
	sphere { <  8, 60,  8>, 10 }
	sphere { <  0, 60, 11>, 10 }
	sphere { < -8, 60,  8>, 10 }
	sphere { <-11, 60,  0>, 10 }
	sphere { < -8, 60, -8>, 10 }
	sphere { <  0, 60,-11>, 10 }
	sphere { <  8, 60, -8>, 10 }
}

#declare Queen = union {
	sphere { <  0,  0,  0>, 24 }
	sphere { <  0, 13,  0>, 18 }
	sphere { <  0, 37,  0>, 17 }
	sphere { <  0, 45,  0>, 19 }
	sphere { <  0, 54,  0>, 20 }
	sphere { <  0, 75,  0>, 12 }
	sphere { <  0, 84,  0>, 16 }
	sphere { <  6, 88,  0>, 12 }
	sphere { <  4, 88,  4>, 12 }
	sphere { <  0, 88,  6>, 12 }
	sphere { < -4, 88,  4>, 12 }
	sphere { < -6, 88,  0>, 12 }
	sphere { < -4, 88, -4>, 12 }
	sphere { <  0, 88, -6>, 12 }
	sphere { <  4, 88, -4>, 12 }
	sphere { < 14, 97,  0>, 4.5}
	sphere { < 10, 97, 10>, 4.5}
	sphere { <  0, 97, 14>, 4.5}
	sphere { <-10, 97, 10>, 4.5}
	sphere { <-14, 97,  0>, 4.5}
	sphere { <-10, 97,-10>, 4.5}
	sphere { <  0, 97,-14>, 4.5}
	sphere { < 10, 97,-10>, 4.5}
}

#declare King = union {
	sphere { <  0,  0,  0>, 25 }
	sphere { <  0, 25,  0>, 18 }
	sphere { <  0, 37,  0>, 23 }
	sphere { <  0, 44,  0>, 22 }
	sphere { <  0, 51,  0>, 20 }
	sphere { <  0, 71,  0>, 15 }
	sphere { <  5, 82,  0>, 16 }
	sphere { <  0, 82,  5>, 16 }
	sphere { < -5, 82,  0>, 16 }
	sphere { <  0, 82, -5>, 16 }
	sphere { <  0, 97,  0>,  4 }
	sphere { <  5,102,  0>,  4 }
	sphere { <  0,102,  0>,  4 }
	sphere { < -5,102,  0>,  4 }
	sphere { <  0,107,  0>,  4 }
}

#declare TBlack = texture {
	pigment { color Black }
	finish {
		ambient 0.4
		diffuse 0.6
		reflection 0.4
		phong 0.005
		phong_size 0.1
	}
}

#declare TWhite = texture {
	pigment { color White }
	finish {
		ambient 0.4
		diffuse 0.6
		reflection 0.4
		phong 0.005
		phong_size 0.1
	}
}

object { Pawn   texture { TWhite } translate <-150, 0,-50> }
object { Knight texture { TWhite } translate < -90, 0,-50> }
object { Bishop texture { TWhite } translate < -30, 0,-50> }
object { Rook   texture { TWhite } translate <  30, 0,-50> }
object { Queen  texture { TWhite } translate <  90, 0,-50> }
object { King   texture { TWhite } translate < 150, 0,-50> }

object { King   texture { TBlack } translate <-150, 0, 50> }
object { Queen  texture { TBlack } translate < -90, 0, 50> }
object { Rook   texture { TBlack } translate < -30, 0, 50> }
object { Bishop texture { TBlack } translate <  30, 0, 50> }
object { Knight texture { TBlack } translate <  90, 0, 50> }
object { Pawn   texture { TBlack } translate < 150, 0, 50> }

object { Rook   texture { TWhite } translate <-350, 0,-350> }
object { Knight texture { TWhite } translate <-250, 0,-350> }
object { Bishop texture { TWhite } translate <-150, 0,-350> }
object { Queen  texture { TWhite } translate < -50, 0,-350> }
object { King   texture { TWhite } translate <  50, 0,-350> }
object { Bishop texture { TWhite } translate < 150, 0,-350> }
object { Knight texture { TWhite } translate < 250, 0,-350> }
object { Rook   texture { TWhite } translate < 350, 0,-350> }
object { Pawn   texture { TWhite } translate <-350, 0,-250> }
object { Pawn   texture { TWhite } translate <-250, 0,-250> }
object { Pawn   texture { TWhite } translate <-150, 0,-250> }
object { Pawn   texture { TWhite } translate < -50, 0,-250> }
object { Pawn   texture { TWhite } translate <  50, 0,-250> }
object { Pawn   texture { TWhite } translate < 150, 0,-250> }
object { Pawn   texture { TWhite } translate < 250, 0,-250> }
object { Pawn   texture { TWhite } translate < 350, 0,-250> }
object { Rook   texture { TBlack } translate <-350, 0, 350> }
object { Knight texture { TBlack } translate <-250, 0, 350> }
object { Bishop texture { TBlack } translate <-150, 0, 350> }
object { Queen  texture { TBlack } translate < -50, 0, 350> }
object { King   texture { TBlack } translate <  50, 0, 350> }
object { Bishop texture { TBlack } translate < 150, 0, 350> }
object { Knight texture { TBlack } translate < 250, 0, 350> }
object { Rook   texture { TBlack } translate < 350, 0, 350> }
object { Pawn   texture { TBlack } translate <-350, 0, 250> }
object { Pawn   texture { TBlack } translate <-250, 0, 250> }
object { Pawn   texture { TBlack } translate <-150, 0, 250> }
object { Pawn   texture { TBlack } translate < -50, 0, 250> }
object { Pawn   texture { TBlack } translate <  50, 0, 250> }
object { Pawn   texture { TBlack } translate < 150, 0, 250> }
object { Pawn   texture { TBlack } translate < 250, 0, 250> }
object { Pawn   texture { TBlack } translate < 350, 0, 250> }
