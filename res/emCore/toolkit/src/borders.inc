//==============================================================================
// borders.inc
// This is a POV-Ray include file defining lots of borders for designing user
// interfaces and similar things.
// Copyright (C) 2007-2008 Oliver Hamann.
//==============================================================================


// Meaning of the border arguments sx,sy,d,e,cx,cy.
// ################################################
//
// The border is symmetrical in both, x and y. sx is the position of the right
// inner edge of the border line, and sy is the position of the upper inner edge
// of the border line. d is the extend of the border line in x and even in y
// direction. e is the extend of the border line in z direction. (cx,cy) is the
// center of the quarter ellipse of the upper right corner. With cx>=sx+d or
// cy>=sy+d you get simple rectangular borders, just like with borders that do
// not have the arguments cx,cy. Borders where the argument cy is missing, have
// cy=sy-(sx-cx) implicitly.
//
// Just to make things clear: The bounding box of the whole object is:
//  x1=-sx-d, x2=sx+d
//  y1=-sy-d, y2=sy+d
//  z1=0 z2=e
//
//
//                          *****************************
//                   *******                             ********
//               ****    . . . . . . . . . . . . . . .           *****
//            ***  . . .                               . . .          ***
//          ** . .                                           . . .       **
//        **.                 ***************************          .       *
//      ***               ****                           ****        .      *
//     *  *            ******************************        ***       .    *
//    *  *           *****                           ****       **      .    *
//    *  *          ***                              |   ***      *     .    *
//   *   *       . *                                 |      **     *     .   *
//   *   *      .  *                                 |        *    *     .   *
//   *  *      .  *                                  |         *    *    .    *
//   *  *      .  *                                  +---------*    *    .    *
//  *   *     .   *                                cx,cy        *   *     .   *
//  *   *     .   *                          _                  *   *     .   *
//  *   *     .   *                    |     /|                 *   *     .   *
//  *   *     .   *                    |    /  Z                *   *     .   *
//  *   *     .   *                    |   /                    *   *----d----*
//  *   *     .   *                    |  /                     *  /*     .   *
//  *   *     .   *                    | /                      * e *     .   *
//  *   *     .   *                    |/                       */  *     .   *
//  *   *     .   *             -------+-----------------------|* - * -  -. - *------->
//  *   *     .   *                   /|                     sx *   *     .   *     X
//  *   *     .   *                  / |                        *   *     .   *
//  *   *     .   *                 /  |                        *   *     .   *
//  *   *     .   *                /   |                        *   *     .   *
//  *   *     .   *                    |                        *   *     .   *
//  *   *     .   *                    |                        *   *     .   *
//  *    *    .    *                   |                        *  *      .  *
//  *    *    .    *                   |                        *  *      .  *
//   *   *     .    *                  |                       *  *      .   *
//   *   *     .     **                |                       ***       .   *
//   *    *     .      ***             |                     ***         .  *
//   *    *       .       ****         _sy               ****.           .  *
//    *    *        .         *************************** .             .  *
//    *     **        . .                             . .               .**
//     *      ***         . . . . . . .|. . . . . . .                 ***
//      **       *****                                           ******
//        ***         *******          |                  ******* ***
//           *****           *****************************   *****
//                *******              |              *******
//                       *****************************
//                                     |
//                                     v Y



//------------------------ RectBorder ----------------------
// Simple border with rectangular cross-section.
//  ^
// Z|          d
//  |  *******************
//  |  *                 *
//  |  *                 *
//  |  *                 * e
//  |  *                 *
//  |  *                 *
// -+--*******************------->
//  |                         X & Y

#macro RectBorderE(sx,sy,d,e,cx,cy)
	difference {
		intersection {
			plane { x sx+d }
			plane { -x sx+d }
			plane { y sy+d }
			plane { -y sy+d }
			plane { z e }
			plane { -z 0 }
			#if (min(sx+d-cx,sy+d-cy) > 0)
				merge {
					intersection {
						plane { x cx }
						plane { -x cx }
					}
					intersection {
						plane { y cy }
						plane { -y cy }
					}
					cylinder {
						<0,0,0>
						<0,0,e>
						1.0
						scale <sx+d-cx,sy+d-cy,1>
						translate <cx,cy,0>
					}
					cylinder {
						<0,0,0>
						<0,0,e>
						1.0
						scale <sx+d-cx,sy+d-cy,1>
						translate <-cx,cy,0>
					}
					cylinder {
						<0,0,0>
						<0,0,e>
						1.0
						scale <sx+d-cx,sy+d-cy,1>
						translate <cx,-cy,0>
					}
					cylinder {
						<0,0,0>
						<0,0,e>
						1.0
						scale <sx+d-cx,sy+d-cy,1>
						translate <-cx,-cy,0>
					}
				}
			#end
		}
		intersection {
			plane { x sx }
			plane { -x sx }
			plane { y sy }
			plane { -y sy }
			#if (min(sx-cx, sy-cy) > 0)
				merge {
					intersection {
						plane { x cx }
						plane { -x cx }
					}
					intersection {
						plane { y cy }
						plane { -y cy }
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,-cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,-cy,0>
					}
				}
			#end
		}
	}
#end


#macro RectBorderC(sx,sy,d,e,cx)
	RectBorderE(sx,sy,d,e,cx,sy-(sx-cx))
#end


#macro RectBorder(sx,sy,d,e)
	difference {
		box {
			<sx+d,sy+d,0>
			<-sx-d,-sy-d,e>
		}
		intersection {
			plane { x sx }
			plane { -x sx }
			plane { y sy }
			plane { -y sy }
		}
	}
#end


//---------------------- ElliBorder ----------------------
// Border with elliptic cross-section.
//  ^
// Z|  |------ d --------|
//  |       *********      -
//  |    ***         ***   |
//  |   *               *  |
//  |  *                 * e
//  |   *               *  |
//  |    ***         ***   |
// -+-------*********---------->
//  |                         X & Y

#macro ElliBorderC(sx,sy,d,e,cx)
	#if (cx>sx)
		#if (cx<sx+d)
			#local cx=sx+d*0.5 ; //!!!
		#end
	#end
	#local cy=sy-(sx-cx) ;
	merge {
		#if (sx+d-cx>0)
			#if (sx-cx>=0)
				intersection {
					torus {
						sx+d*0.5-cx
						d*0.5
						rotate <90,0,0>
						scale <1,1,e/d>
						translate <cx,cy,e*0.5>
					}
					plane {
						<-1,0,0>
						(-cx-0.0001)
					}
					plane {
						<0,-1,0>
						(-cy-0.0001)
					}
				}
				intersection {
					torus {
						sx+d*0.5-cx
						d*0.5
						rotate <90,0,0>
						scale <1,1,e/d>
						translate <-cx,cy,e*0.5>
					}
					plane {
						<1,0,0>
						(-cx-0.0001)
					}
					plane {
						<0,-1,0>
						(-cy-0.0001)
					}
				}
				intersection {
					torus {
						sx+d*0.5-cx
						d*0.5
						rotate <90,0,0>
						scale <1,1,e/d>
						translate <cx,-cy,e*0.5>
					}
					plane {
						<-1,0,0>
						(-cx-0.0001)
					}
					plane {
						<0,1,0>
						(-cy-0.0001)
					}
				}
				intersection {
					torus {
						sx+d*0.5-cx
						d*0.5
						rotate <90,0,0>
						scale <1,1,e/d>
						translate <-cx,-cy,e*0.5>
					}
					plane {
						<1,0,0>
						(-cx-0.0001)
					}
					plane {
						<0,1,0>
						(-cy-0.0001)
					}
				}
			#else
				sphere {
					<0,0,0>
					0.5
					scale <d,d,e>
					translate <sx+d*0.5,sy+d*0.5,e*0.5>
				}
				sphere {
					<0,0,0>
					0.5
					scale <d,d,e>
					translate <-sx-d*0.5,sy+d*0.5,e*0.5>
				}
				sphere {
					<0,0,0>
					0.5
					scale <d,d,e>
					translate <sx+d*0.5,-sy-d*0.5,e*0.5>
				}
				sphere {
					<0,0,0>
					0.5
					scale <d,d,e>
					translate <-sx-d*0.5,-sy-d*0.5,e*0.5>
				}
			#end
		#end
		intersection {
			cylinder {
				<-sx-d,0,0>
				<sx+d,0,0>
				0.5
				scale <1,d,e>
				translate <0,sy+d*0.5,e*0.5>
			}
			plane {
				<1,-1,0>
				0
				translate <sx,sy,0>
			}
			plane {
				<-1,-1,0>
				0
				translate <-sx,sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<1,0,0>
					cx
				}
				plane {
					<-1,0,0>
					cx
				}
			#end
		}
		intersection {
			cylinder {
				<-sx-d,0,0>
				<sx+d,0,0>
				0.5
				scale <1,d,e>
				translate <0,-sy-d*0.5,e*0.5>
			}
			plane {
				<1,1,0>
				0
				translate <sx,-sy,0>
			}
			plane {
				<-1,1,0>
				0
				translate <-sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<1,0,0>
					cx
				}
				plane {
					<-1,0,0>
					cx
				}
			#end
		}
		intersection {
			cylinder {
				<0,-sy-d,0>
				<0,sy+d,0>
				0.5
				scale <d,1,e>
				translate <sx+d*0.5,0,e*0.5>
			}
			plane {
				<-1,1,0>
				0
				translate <sx,sy,0>
			}
			plane {
				<-1,-1,0>
				0
				translate <sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<0,1,0>
					cy
				}
				plane {
					<0,-1,0>
					cy
				}
			#end
		}
		intersection {
			cylinder {
				<0,-sy-d,0>
				<0,sy+d,0>
				0.5
				scale <d,1,e>
				translate <-sx-d*0.5,0,e*0.5>
			}
			plane {
				<1,1,0>
				0
				translate <-sx,sy,0>
			}
			plane {
				<1,-1,0>
				0
				translate <-sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<0,1,0>
					cy
				}
				plane {
					<0,-1,0>
					cy
				}
			#end
		}
	}
#end

#macro ElliBorder(sx,sy,d,e)
	ElliBorderC(sx,sy,d,e,sx+d+100)
#end


//---------------------- ElliBorder2 ----------------------
// Like ElliBorder, but merged with a RectBorder.
//  ^
// Z|  |------ d --------|
//  |       *********         -
//  |    ***         ***      |
//  |   *               *     |
//  |  *                 * -  e2
//  |  *                 * e1 |
//  |  *                 * |  |
// -+--*******************----->
//  |                         X or Y

#macro ElliBorder2C(sx,sy,d,e1,e2,cx)
	merge {
		object {
			ElliBorderC(sx,sy,d,(e2-e1)*2,cx)
			translate <0,0,e1*2-e2>
		}
		RectBorderC(sx,sy,d,e1,cx)
	}
#end

#macro ElliBorder2(sx,sy,d,e1,e2)
	merge {
		object {
			ElliBorder(sx,sy,d,(e2-e1)*2)
			translate <0,0,e1*2-e2>
		}
		RectBorder(sx,sy,d,e1)
	}
#end


//---------------------- SlopeBorder ----------------------
//
//  ^
// Z|    |------ d ---------|
//  |  - *****
//  |  | *    *****
//  |  | *         *****
//  | e1 *              ***** -
//  |  | *                  * e2
//  |  | *                  * |
// -+----********************----->
//  |                         X & Y

#macro SlopeBorderC(sx,sy,d,e1,e2,cx)
	#if (cx>sx)
		#if (cx<sx+d)
			#local cx=sx ; //!!!
		#end
	#end
	#local cy=sy-(sx-cx) ;
	#local e=max(e1,e2) ;
	difference {
		intersection {
			plane { x sx+d }
			plane { -x sx+d }
			plane { y sy+d }
			plane { -y sy+d }
			plane { z e }
			plane { -z 0 }
			#if (sx+d-cx > 0)
				merge {
					intersection {
						plane { x cx }
						plane { -x cx }
						#if (e1 >= e2)
							intersection {
								plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
								plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
							}
						#else
							merge {
								plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
								plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
							}
						#end
					}
					intersection {
						plane { y cy }
						plane { -y cy }
						#if (e1 >= e2)
							intersection {
								plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
								plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
							}
						#else
							merge {
								plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
								plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
							}
						#end
					}
					#if (e1 >= e2)
						cylinder {
							<0,0,0>
							<0,0,e2+0.000001>
							1.0
							scale <sx+d-cx,sy+d-cy,1>
							translate <cx,cy,0>
						}
						cone {
							<cx,cy,e2> (sx+d-cx)
							<cx,cy,e1> (sx-cx)
						}
						cylinder {
							<0,0,0>
							<0,0,e2+0.000001>
							1.0
							scale <sx+d-cx,sy+d-cy,1>
							translate <-cx,cy,0>
						}
						cone {
							<-cx,cy,e2> (sx+d-cx)
							<-cx,cy,e1> (sx-cx)
						}
						cylinder {
							<0,0,0>
							<0,0,e2+0.000001>
							1.0
							scale <sx+d-cx,sy+d-cy,1>
							translate <cx,-cy,0>
						}
						cone {
							<cx,-cy,e2> (sx+d-cx)
							<cx,-cy,e1> (sx-cx)
						}
						cylinder {
							<0,0,0>
							<0,0,e2+0.000001>
							1.0
							scale <sx+d-cx,sy+d-cy,1>
							translate <-cx,-cy,0>
						}
						cone {
							<-cx,-cy,e2> (sx+d-cx)
							<-cx,-cy,e1> (sx-cx)
						}
					#else
						difference {
							cylinder {
								<0,0,0>
								<0,0,e2-0.000001>
								1.0
								scale <sx+d-cx,sy+d-cy,1>
								translate <cx,cy,0>
							}
							cone {
								<cx,cy,e2> (sx+d-cx)
								<cx,cy,e1> (sx-cx)
							}
							plane {
								<1,0,0> cx
							}
							plane {
								<0,1,0> cy
							}
						}
						difference {
							cylinder {
								<0,0,0>
								<0,0,e2-0.000001>
								1.0
								scale <sx+d-cx,sy+d-cy,1>
								translate <-cx,cy,0>
							}
							cone {
								<-cx,cy,e2> (sx+d-cx)
								<-cx,cy,e1> (sx-cx)
							}
							plane {
								<-1,0,0> cx
							}
							plane {
								<0,1,0> cy
							}
						}
						difference {
							cylinder {
								<0,0,0>
								<0,0,e2-0.000001>
								1.0
								scale <sx+d-cx,sy+d-cy,1>
								translate <cx,-cy,0>
							}
							cone {
								<cx,-cy,e2> (sx+d-cx)
								<cx,-cy,e1> (sx-cx)
							}
							plane {
								<1,0,0> cx
							}
							plane {
								<0,-1,0> cy
							}
						}
						difference {
							cylinder {
								<0,0,0>
								<0,0,e2-0.000001>
								1.0
								scale <sx+d-cx,sy+d-cy,1>
								translate <-cx,-cy,0>
							}
							cone {
								<-cx,-cy,e2> (sx+d-cx)
								<-cx,-cy,e1> (sx-cx)
							}
							plane {
								<-1,0,0> cx
							}
							plane {
								<0,-1,0> cy
							}
						}
					#end
				}
			#else
				#if (e1 >= e2)
					intersection {
						plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
						plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
						plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
						plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
					}
				#else
					merge {
						plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
						plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
						plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
						plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
					}
				#end
			#end
		}
		intersection {
			plane { x sx }
			plane { -x sx }
			plane { y sy }
			plane { -y sy }
			#if (min(sx-cx, sy-cy) > 0)
				merge {
					intersection {
						plane { x cx }
						plane { -x cx }
					}
					intersection {
						plane { y cy }
						plane { -y cy }
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,-cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,-cy,0>
					}
				}
			#end
		}
	}
#end

#macro SlopeBorder(sx,sy,d,e1,e2)
	SlopeBorderC(sx,sy,d,e1,e2,sx+d+100)
#end


//---------------------- BezierBorder ----------------------
//
//  ^
// Z|    |--------------- d ----------------|   ...
//  |                                        ...   a2
//  |                                     ***-------
//  |                  ******         ****  *  |
//  |              ****      *********      *  |
//  |           ***                         *  |
//  |         **                            *  |
//  |        * .                            *  |
//  |       *   .                           *  |
//  |      * a1  .                          *  |
//  |     *      .                          *  e2
//  |  - *--------                          *  |
//  |  | *                                  *  |
//  |  | *                                  *  |
//  | e1 *                                  *  |
//  |  | *                                  *  |
//  |  | *                                  *  |
// -+----************************************---->
//  |                                          X & Y

#macro BezierBorderC(sx,sy,d,e1,e2,a1,a2,cx)
	#if (cx>sx)
		#if (cx<sx+d)
			#local cx=sx ; //!!!
		#end
	#end
	#local a1=a1*pi/180;
	#local a2=a2*pi/180;
	#local p1x=cos(a1)*d*0.5 ;
	#local p1z=e1+sin(a1)*d*0.5 ;
	#local p2x=d-cos(a2)*d*0.5 ;
	#local p2z=e2-sin(a2)*d*0.5 ;
	#local r=sx-cx ;
	#local cy=sy-(sx-cx) ;
	#local e=max(e1,e2) ;
	merge {
		#if (sx+d-cx>0)
			intersection {
				lathe {
					bezier_spline
					16
					<r,e1> <r+p1x,p1z> <r+p2x,p2z> <r+d,e2>
					<r+d,e2> <r+d,e2> <r+d,0> <r+d,0>
					<r+d,0> <r+d,0> <r,0> <r,0>
					<r,0> <r,0> <r,e1> <r,e1>
					rotate <90,0,0>
					translate <cx,cy,0>
				}
				plane {
					<-1,0,0>
					(-cx+0.000001)
				}
				plane {
					<0,-1,0>
					(-cy+0.000001)
				}
			}
			intersection {
				lathe {
					bezier_spline
					16
					<r,e1> <r+p1x,p1z> <r+p2x,p2z> <r+d,e2>
					<r+d,e2> <r+d,e2> <r+d,0> <r+d,0>
					<r+d,0> <r+d,0> <r,0> <r,0>
					<r,0> <r,0> <r,e1> <r,e1>
					rotate <90,0,0>
					translate <-cx,cy,0>
				}
				plane {
					<1,0,0>
					(-cx+0.000001)
				}
				plane {
					<0,-1,0>
					(-cy+0.000001)
				}
			}
			intersection {
				lathe {
					bezier_spline
					16
					<r,e1> <r+p1x,p1z> <r+p2x,p2z> <r+d,e2>
					<r+d,e2> <r+d,e2> <r+d,0> <r+d,0>
					<r+d,0> <r+d,0> <r,0> <r,0>
					<r,0> <r,0> <r,e1> <r,e1>
					rotate <90,0,0>
					translate <cx,-cy,0>
				}
				plane {
					<-1,0,0>
					(-cx+0.000001)
				}
				plane {
					<0,1,0>
					(-cy+0.000001)
				}
			}
			intersection {
				lathe {
					bezier_spline
					16
					<r,e1> <r+p1x,p1z> <r+p2x,p2z> <r+d,e2>
					<r+d,e2> <r+d,e2> <r+d,0> <r+d,0>
					<r+d,0> <r+d,0> <r,0> <r,0>
					<r,0> <r,0> <r,e1> <r,e1>
					rotate <90,0,0>
					translate <-cx,-cy,0>
				}
				plane {
					<1,0,0>
					(-cx+0.000001)
				}
				plane {
					<0,1,0>
					(-cy+0.000001)
				}
			}
		#end
		intersection {
			prism {
				bezier_spline
				(-sx-d)
				(sx+d)
				16
				<0,e1> <p1x,p1z> <p2x,p2z> <d,e2>
				<d,e2> <d,e2> <d,0> <d,0>
				<d,0> <d,0> <0,0> <0,0>
				<0,0> <0,0> <0,e1> <0,e1>
				rotate <0,0,90>
				translate <0,sy,0>
			}
			plane {
				<1,-1,0>
				0
				translate <sx,sy,0>
			}
			plane {
				<-1,-1,0>
				0
				translate <-sx,sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<1,0,0>
					cx
				}
				plane {
					<-1,0,0>
					cx
				}
			#end
		}
		intersection {
			prism {
				bezier_spline
				(-sx-d)
				(sx+d)
				16
				<0,e1> <p1x,p1z> <p2x,p2z> <d,e2>
				<d,e2> <d,e2> <d,0> <d,0>
				<d,0> <d,0> <0,0> <0,0>
				<0,0> <0,0> <0,e1> <0,e1>
				rotate <0,0,270>
				translate <0,-sy,0>
			}
			plane {
				<1,1,0>
				0
				translate <sx,-sy,0>
			}
			plane {
				<-1,1,0>
				0
				translate <-sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<1,0,0>
					cx
				}
				plane {
					<-1,0,0>
					cx
				}
			#end
		}
		intersection {
			prism {
				bezier_spline
				(-sy-d)
				(sy+d)
				16
				<0,e1> <p1x,p1z> <p2x,p2z> <d,e2>
				<d,e2> <d,e2> <d,0> <d,0>
				<d,0> <d,0> <0,0> <0,0>
				<0,0> <0,0> <0,e1> <0,e1>
				translate <sx,0,0>
			}
			plane {
				<-1,1,0>
				0
				translate <sx,sy,0>
			}
			plane {
				<-1,-1,0>
				0
				translate <sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<0,1,0>
					cy
				}
				plane {
					<0,-1,0>
					cy
				}
			#end
		}
		intersection {
			prism {
				bezier_spline
				(-sy-d)
				(sy+d)
				16
				<0,e1> <p1x,p1z> <p2x,p2z> <d,e2>
				<d,e2> <d,e2> <d,0> <d,0>
				<d,0> <d,0> <0,0> <0,0>
				<0,0> <0,0> <0,e1> <0,e1>
				rotate <0,0,180>
				translate <-sx,0,0>
			}
			plane {
				<1,1,0>
				0
				translate <-sx,sy,0>
			}
			plane {
				<1,-1,0>
				0
				translate <-sx,-sy,0>
			}
			#if (sx+d-cx>0)
				plane {
					<0,1,0>
					cy
				}
				plane {
					<0,-1,0>
					cy
				}
			#end
		}
	}
#end

#macro BezierBorder(sx,sy,d,e1,e2,a1,a2)
	BezierBorderC(sx,sy,d,e1,e2,a1,a2,sx+d+100)
#end


//--------------------- ConeCornerBorder ----------------------
// Here the outer edge is always a rectangle.
//  ^
// Z|    |------ d ---------|
//  |  - *****
//  |  | *    *****
//  |  | *         *****
//  | e1 *              ***** -
//  |  | *                  * e2
//  |  | *                  * |
// -+----********************----->
//  |                         X & Y

#macro ConeCornerBorderC(sx,sy,d,e1,e2,cx)
	#if (cx>sx)
		#local cx=sx ; //!!!
	#end
	#local cy=sy-(sx-cx) ;
	#local e=max(e1,e2) ;
	#local k=0.5 ;
	difference {
		intersection {
			plane { x sx+d }
			plane { -x sx+d }
			plane { y sy+d }
			plane { -y sy+d }
			plane { z e }
			plane { -z 0 }
			#if (e1 >= e2)
				difference {
					intersection {
						plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
						plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
						plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
						plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
						plane { <0,0,1> e1*0.9999999 }
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> < k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							sx-cx, 0, 0,
							d, d, e1-e2,
							0, sy-cy, 0,
							-sx-d, -sy-d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> <k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							-sx+cx, 0, 0,
							-d, d, e1-e2,
							0, sy-cy, 0,
							sx+d, -sy-d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> <k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							sx-cx, 0, 0,
							d, -d, e1-e2,
							0, -sy+cy, 0,
							-sx-d, sy+d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> < k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							-sx+cx, 0, 0,
							-d, -d, e1-e2,
							0, -sy+cy, 0,
							sx+d, sy+d,e2
						>
					}
				}
			#else
				merge {
					plane { <e1-e2,0,d> 0 translate <sx,0,e1> }
					plane { <e2-e1,0,d> 0 translate <-sx,0,e1> }
					plane { <0,e1-e2,d> 0 translate <0,sy,e1> }
					plane { <0,e2-e1,d> 0 translate <0,-sy,e1> }
					plane { <0,0,1> e1 }
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> < k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							sx-cx, 0, 0,
							d, d, e1-e2,
							0, sy-cy, 0,
							-sx-d, -sy-d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> <k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							-sx+cx, 0, 0,
							-d, d, e1-e2,
							0, sy-cy, 0,
							sx+d, -sy-d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> <k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							sx-cx, 0, 0,
							d, -d, e1-e2,
							0, -sy+cy, 0,
							-sx-d, sy+d,e2
						>
					}
					prism {
						conic_sweep bezier_spline 0 1 20
						< 0, 1> < 0, k> < k, 0> < 1, 0>
						< 1, 0> < 1, 0> < 1,-1> < 1,-1>
						< 1,-1> < 1,-1> <-1,-1> <-1,-1>
						<-1,-1> <-1,-1> <-1, 1> <-1, 1>
						<-1, 1> <-1, 1> < 0, 1> < 0, 1>
						matrix <
							-sx+cx, 0, 0,
							-d, -d, e1-e2,
							0, -sy+cy, 0,
							sx+d, sy+d,e2
						>
					}
				}
			#end
		}
		intersection {
			plane { x sx }
			plane { -x sx }
			plane { y sy }
			plane { -y sy }
			#if (min(sx-cx, sy-cy) > 0)
				merge {
					intersection {
						plane { x cx }
						plane { -x cx }
					}
					intersection {
						plane { y cy }
						plane { -y cy }
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <cx,-cy,0>
					}
					cylinder {
						<0,0,-1>
						<0,0,e+1>
						1.0
						scale <sx-cx,sy-cy,1>
						translate <-cx,-cy,0>
					}
				}
			#end
		}
	}
#end
