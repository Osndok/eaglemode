/*------------------------------------------------------------------------------
// font2em.c
//
// Copyright (C) 2009-2010 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ft2build.h>
#include FT_FREETYPE_H


static struct {
	int FirstCode;
	int LastCode;
	const char * Name;
} CharsetTable[]={
	{  0x0000,  0x001F, "C0Controls"                          },
	{  0x0020,  0x007F, "BasicLatin"                          },
	{  0x00A0,  0x00FF, "Latin1Supplement"                    },
	{  0x0100,  0x017F, "LatinExtendedA"                      },
	{  0x0180,  0x024F, "LatinExtendedB"                      },
	{  0x0250,  0x02AF, "IPAExtensions"                       },
	{  0x02B0,  0x02FF, "SpacingModifierLetters"              },
	{  0x0300,  0x036F, "CombiningDiacriticalMarks"           },
	{  0x0370,  0x03FF, "GreekAndCoptic"                      },
	{  0x0400,  0x04FF, "Cyrillic"                            },
	{  0x0500,  0x052F, "CyrillicSupplement"                  },
	{  0x0530,  0x058F, "Armenian"                            },
	{  0x0590,  0x05FF, "Hebrew"                              },
	{  0x0600,  0x06FF, "Arabic"                              },
	{  0x0700,  0x074F, "Syriac"                              },
	{  0x0750,  0x077F, "ArabicSupplement"                    },
	{  0x0780,  0x07BF, "Thaana"                              },
	{  0x07C0,  0x07FF, "NKo"                                 },
	{  0x0900,  0x097F, "Devanagari"                          },
	{  0x0980,  0x09FF, "Bengali"                             },
	{  0x0A00,  0x0A7F, "Gurmukhi"                            },
	{  0x0A80,  0x0AFF, "Gujarati"                            },
	{  0x0B00,  0x0B7F, "Oriya"                               },
	{  0x0B80,  0x0BFF, "Tamil"                               },
	{  0x0C00,  0x0C7F, "Telugu"                              },
	{  0x0C80,  0x0CFF, "Kannada"                             },
	{  0x0D00,  0x0D7F, "Malayalam"                           },
	{  0x0D80,  0x0DFF, "Sinhala"                             },
	{  0x0E00,  0x0E7F, "Thai"                                },
	{  0x0E80,  0x0EFF, "Lao"                                 },
	{  0x0F00,  0x0FFF, "Tibetan"                             },
	{  0x1000,  0x109F, "Myanmar"                             },
	{  0x10A0,  0x10FF, "Georgian"                            },
	{  0x1100,  0x11FF, "HangulJamo"                          },
	{  0x1200,  0x137F, "Ethiopic"                            },
	{  0x1380,  0x139F, "EthiopicSupplement"                  },
	{  0x13A0,  0x13FF, "Cherokee"                            },
	{  0x1400,  0x167F, "CanadianSyllabics"                   },
	{  0x1680,  0x169F, "Ogham"                               },
	{  0x16A0,  0x16FF, "Runic"                               },
	{  0x1700,  0x171F, "Tagalog"                             },
	{  0x1720,  0x173F, "Hanunoo"                             },
	{  0x1740,  0x175F, "Buhid"                               },
	{  0x1760,  0x177F, "Tagbanwa"                            },
	{  0x1780,  0x17FF, "Khmer"                               },
	{  0x1800,  0x18AF, "Mongolian"                           },
	{  0x1900,  0x194F, "Limbu"                               },
	{  0x1950,  0x197F, "TaiLe"                               },
	{  0x1980,  0x19DF, "NewTaiLue"                           },
	{  0x19E0,  0x19FF, "KhmerSymbols"                        },
	{  0x1A00,  0x1A1F, "Buginese"                            },
	{  0x1B00,  0x1B7F, "Balinese"                            },
	{  0x1B80,  0x1BBF, "Sundanese"                           },
	{  0x1C00,  0x1C4F, "Lepcha"                              },
	{  0x1C50,  0x1C7F, "OlChiki"                             },
	{  0x1D00,  0x1D7F, "PhoneticExtensions"                  },
	{  0x1D80,  0x1DBF, "PhoneticExtensionsSupplement"        },
	{  0x1DC0,  0x1DFF, "CombiningDiacriticalMarksSupplement" },
	{  0x1E00,  0x1EFF, "LatinExtendedAdditional"             },
	{  0x1F00,  0x1FFF, "GreekExtended"                       },
	{  0x2000,  0x206F, "GeneralPunctuation"                  },
	{  0x2070,  0x209F, "SuperscriptsAndSubscripts"           },
	{  0x20A0,  0x20CF, "CurrencySymbols"                     },
	{  0x20D0,  0x20FF, "CombiningDiacriticalMarksforSymbols" },
	{  0x2100,  0x214F, "LetterlikeSymbols"                   },
	{  0x2150,  0x218F, "NumberForms"                         },
	{  0x2190,  0x21FF, "Arrows"                              },
	{  0x2200,  0x22FF, "MathematicalOperators"               },
	{  0x2300,  0x23FF, "MiscellaneousTechnical"              },
	{  0x2400,  0x243F, "ControlPictures"                     },
	{  0x2440,  0x245F, "OpticalCharacterRecognition"         },
	{  0x2460,  0x24FF, "EnclosedAlphanumerics"               },
	{  0x2500,  0x257F, "BoxDrawing"                          },
	{  0x2580,  0x259F, "BlockElements"                       },
	{  0x25A0,  0x25FF, "GeometricShapes"                     },
	{  0x2600,  0x26FF, "MiscellaneousSymbols"                },
	{  0x2700,  0x27BF, "Dingbats"                            },
	{  0x27C0,  0x27EF, "MiscellaneousMathematicalSymbolsA"   },
	{  0x27F0,  0x27FF, "SupplementalArrowsA"                 },
	{  0x2800,  0x28FF, "BraillePatterns"                     },
	{  0x2900,  0x297F, "SupplementalArrowsB"                 },
	{  0x2980,  0x29FF, "MiscellaneousMathematicalSymbolsB"   },
	{  0x2A00,  0x2AFF, "SupplementalMathematicalOperators"   },
	{  0x2B00,  0x2BFF, "MiscellaneousSymbolsAndArrows"       },
	{  0x2C00,  0x2C5F, "Glagolitic"                          },
	{  0x2C60,  0x2C7F, "LatinExtendedC"                      },
	{  0x2C80,  0x2CFF, "Coptic"                              },
	{  0x2D00,  0x2D2F, "GeorgianSupplement"                  },
	{  0x2D30,  0x2D7F, "Tifinagh"                            },
	{  0x2D80,  0x2DDF, "EthiopicExtended"                    },
	{  0x2DE0,  0x2DFF, "CyrillicExtendedA"                   },
	{  0x2E00,  0x2E7F, "SupplementalPunctuation"             },
	{  0x2E80,  0x2EFF, "CJKRadicalsSupplement"               },
	{  0x2F00,  0x2FDF, "KangxiRadicals"                      },
	{  0x2FF0,  0x2FFF, "IdeographicDescriptionCharacters"    },
	{  0x3000,  0x303F, "CJKSymbolsAndPunctuation"            },
	{  0x3040,  0x309F, "Hiragana"                            },
	{  0x30A0,  0x30FF, "Katakana"                            },
	{  0x3100,  0x312F, "Bopomofo"                            },
	{  0x3130,  0x318F, "HangulCompatibilityJamo"             },
	{  0x3190,  0x319F, "Kanbun"                              },
	{  0x31A0,  0x31BF, "BopomofoExtended"                    },
	{  0x31C0,  0x31EF, "CJKStrokes"                          },
	{  0x31F0,  0x31FF, "KatakanaPhoneticExtensions"          },
	{  0x3200,  0x32FF, "EnclosedCJKLettersAndMonths"         },
	{  0x3300,  0x33FF, "CJKCompatibility"                    },
	{  0x3400,  0x4DBF, "CJKUnifiedIdeographsExtensionA"      },
	{  0x4DC0,  0x4DFF, "YijingHexagramSymbols"               },
	{  0x4E00,  0x9FCF, "CJKUnifiedIdeographs"                },
	{  0xA000,  0xA48F, "YiSyllables"                         },
	{  0xA490,  0xA4CF, "YiRadicals"                          },
	{  0xA500,  0xA63F, "Vai"                                 },
	{  0xA640,  0xA69F, "CyrillicExtendedB"                   },
	{  0xA700,  0xA71F, "ModifierToneLetters"                 },
	{  0xA720,  0xA7FF, "LatinExtendedD"                      },
	{  0xA800,  0xA82F, "SylotiNagri"                         },
	{  0xA840,  0xA87F, "Phagspa"                             },
	{  0xA880,  0xA8DF, "Saurashtra"                          },
	{  0xA900,  0xA92F, "KayahLi"                             },
	{  0xA930,  0xA95F, "Rejang"                              },
	{  0xAA00,  0xAA5F, "Cham"                                },
	{  0xAC00,  0xD7AF, "HangulSyllables"                     },
	{  0xF900,  0xFAFF, "CJKCompatibilityIdeographs"          },
	{  0xFB00,  0xFB4F, "AlphabeticPresentationForms"         },
	{  0xFB50,  0xFDFF, "ArabicPresentationFormsA"            },
	{  0xFE00,  0xFE0F, "VariationSelectors"                  },
	{  0xFE10,  0xFE1F, "Verticalforms"                       },
	{  0xFE20,  0xFE2F, "CombiningHalfMarks"                  },
	{  0xFE30,  0xFE4F, "CJKCompatibilityForms"               },
	{  0xFE50,  0xFE6F, "SmallFormVariants"                   },
	{  0xFE70,  0xFEFF, "ArabicPresentationFormsB"            },
	{  0xFF00,  0xFFEF, "HalfwidthAndFullwidthForms"          },
	{  0xFFF0,  0xFFFF, "Specials"                            },
	{ 0x10000, 0x1007F, "LinearBSyllabary"                    },
	{ 0x10080, 0x100FF, "LinearBIdeograms"                    },
	{ 0x10100, 0x1013F, "AegeanNumbers"                       },
	{ 0x10140, 0x1018F, "AncientGreekNumbers"                 },
	{ 0x10190, 0x101CF, "AncientSymbols"                      },
	{ 0x101D0, 0x101FF, "PhaistosDisc"                        },
	{ 0x10280, 0x1029F, "Lycian"                              },
	{ 0x102A0, 0x102DF, "Carian"                              },
	{ 0x10300, 0x1032F, "OldItalic"                           },
	{ 0x10330, 0x1034F, "Gothic"                              },
	{ 0x10380, 0x1039F, "Ugaritic"                            },
	{ 0x103A0, 0x103DF, "OldPersian"                          },
	{ 0x10400, 0x1044F, "Deseret"                             },
	{ 0x10450, 0x1047F, "Shavian"                             },
	{ 0x10480, 0x104AF, "Osmanya"                             },
	{ 0x10800, 0x1083F, "CypriotSyllabary"                    },
	{ 0x10900, 0x1091F, "Phoenician"                          },
	{ 0x10920, 0x1093F, "Lydian"                              },
	{ 0x10A00, 0x10A5F, "Kharoshthi"                          },
	{ 0x12000, 0x123FF, "Cuneiform"                           },
	{ 0x12400, 0x1247F, "CuneiformNumbersAndPunctuation"      },
	{ 0x1D000, 0x1D0FF, "ByzantineMusicalSymbols"             },
	{ 0x1D100, 0x1D1FF, "MusicalSymbols"                      },
	{ 0x1D200, 0x1D24F, "AncientGreekMusicalNotation"         },
	{ 0x1D300, 0x1D35F, "TaiXuanJingSymbols"                  },
	{ 0x1D360, 0x1D37F, "CountingRodNumerals"                 },
	{ 0x1D400, 0x1D7FF, "MathematicalAlphanumericSymbols"     },
	{ 0x1F000, 0x1F02F, "MahjongTiles"                        },
	{ 0x1F030, 0x1F09F, "DominoTiles"                         },
	{ 0x20000, 0x2A6DF, "CJKUnifiedIdeographsExtensionB"      },
	{ 0x2F800, 0x2FA1F, "CJKCompatibilityIdeographsSupplement"},
	{ 0xE0000, 0xE007F, "Tags"                                },
	{ 0xE0100, 0xE01EF, "VariationSelectorsSupplement"        },
	{       0,       0, NULL                                  }
};


static void WriteGreyTGA(
	int width, int height, const unsigned char * map, const char * filePath
)
{
	FILE * f;
	int n,l;

	f=fopen(filePath,"wb");
	if (!f) {
		fprintf(stderr,"Failed to create \"%s\": %s\n",filePath,strerror(errno));
		exit(1);
	}
	fputc(0,f);
	fputc(0,f);
	fputc(11,f);
	fputc(0,f); fputc(0,f);
	fputc(0,f); fputc(0,f);
	fputc(0,f);
	fputc(0,f); fputc(0,f);
	fputc(0,f); fputc(0,f);
	fputc(width&255,f); fputc(width>>8,f);
	fputc(height&255,f); fputc(height>>8,f);
	fputc(8,f);
	fputc(32,f);
	for (n=width*height; n>0; n-=l, map+=l) {
		if (n>=2 && map[0]==map[1]) {
			for (l=2; l<128 && l<n && map[0]==map[l]; l++);
			fputc(127+l,f);
			fputc(map[0],f);
		}
		else {
			for (l=1; l<128 && l<n; l++) {
				if (
					l+1<n && map[l]==map[l+1] &&
					(l+2>=n || map[l+1]==map[l+2])
				) break;
			}
			fputc(l-1,f);
			fwrite(map,1,l,f);
		}
	}
	fclose(f);
}


static void PutFTBitmap(
	int width, int height, unsigned char * map,
	int clipX1, int clipY1, int clipX2, int clipY2,
	int posX, int posY, const FT_Bitmap * bitmap
)
{
	int srcX,srcY,tgtX,tgtY,val;

	if (clipX1<0) clipX1=0;
	if (clipY1<0) clipY1=0;
	if (clipX2>width) clipX2=width;
	if (clipY2>height) clipY2=height;

	for (srcY=bitmap->rows-1; srcY>=0; srcY--) {
		tgtY=srcY+posY;
		if (tgtY<clipY1 || tgtY>=clipY2) continue;
		for (srcX=bitmap->width-1; srcX>=0; srcX--) {
			tgtX=srcX+posX;
			if (tgtX<clipX1 || tgtX>=clipX2) continue;
			if (bitmap->pixel_mode==FT_PIXEL_MODE_GRAY) {
				val=bitmap->buffer[srcY*bitmap->pitch+srcX];
			}
			else if (bitmap->pixel_mode==FT_PIXEL_MODE_MONO) {
				val=bitmap->buffer[srcY*bitmap->pitch+(srcX>>3)];
				val=((val>>(7-(srcX&7)))&1)*255;
			}
			else {
				val=0;
			}
			val+=map[tgtY*width+tgtX];
			if (val>255) val=255; else if (val<0) val=0;
			map[tgtY*width+tgtX]=val;
		}
	}
}


static void PutUnknown(
	int width, int height, unsigned char * map,
	int x, int y, int w, int h
)
{
	static const int picW=64;
	static const int picH=64;
	static const char * pic=
		"0000000000000000000000000000000000000000000000000000000000000000"
		"0000000000000000000000000000000000000000000000000000000000000000"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000001661000000000000000000000000000000009999900"
		"0099999000000000000001661000000000000000000000000000000009999900"
		"0099999000000000000001661000000000000000000000000000000009999900"
		"0099999097018617999511661176189996101699612730056169997109999900"
		"0099999097018617612852661285088019705821763840167168118809999900"
		"0099999097018617611752661582088008816711664843367168107909999900"
		"0099999097018617611752688930088008816711664867677168107909999900"
		"0099999097018617611752685881088008816711664898897168107909999900"
		"0099999088118617611752661285088008805821763793596168107909999900"
		"0099999017999617611752661176188008801699611661284068107909999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000382000000000000000000000000000009999900"
		"0099999000000000000000000382000000000000000000000000000009999900"
		"0099999000000000000000000382000000000000000000000000000009999900"
		"0099999000000000000389830389983003898402977951000000000009999900"
		"0099999000000000003841582382059115304832997683000000000009999900"
		"0099999000000000003830000382039200002832961043000000000009999900"
		"0099999000000000003830000382039203899832940000000000000009999900"
		"0099999000000000003830000382039219502832940000000000000009999900"
		"0099999000000000003841582382039219502832940000000000000009999900"
		"0099999000000000000389830382039203899832940000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999000000000000000000000000000000000000000000000000009999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0099999999999999999999999999999999999999999999999999999999999900"
		"0000000000000000000000000000000000000000000000000000000000000000"
		"0000000000000000000000000000000000000000000000000000000000000000"
	;
	int xt,yt,xs,ys,xb,yb,xe,ye,t,v;

	if (w*picH<h*picW) { t=w*picH/picW; y+=(h-t)/2; h=t; }
	else               { t=h*picW/picH; x+=(w-t)/2; w=t; }
	for (xt=0; xt<w; xt++) for (yt=0; yt<h; yt++) {
		if (x+xt<0 || x+xt>=width) continue;
		if (y+yt<0 || y+yt>=height) continue;
		xb=xt*picW/w; xe=(xt+1)*picW/w; if (xe<=xb) xe++;
		yb=yt*picH/h; ye=(yt+1)*picH/h; if (ye<=yb) ye++;
		v=0;
		for (xs=xb; xs<xe; xs++) for (ys=yb; ys<ye; ys++) v+=pic[ys*picW+xs]-'0';
		map[(y+yt)*width+x+xt]=v*255/(11*(xe-xb)*(ye-yb));
	}
}


static void ConvertFont(
	const char * fontPath, int faceIndex,
	const char * targetDir, int desiredCharHeight
)
{
	FT_Face face;
	FT_Library lib;
	FT_UInt glyph;
	FT_Vector vec;
	char tmp[256];
	unsigned char * map;
	int i,j,k,l,x,y,err,numCodes,extraWidth,extraHeight;
	int charWidth,charHeight,mapWidth,mapHeight,originX,originY;

	err=FT_Init_FreeType(&lib);
	if (err) {
		fprintf(stderr,"FT_Init_FreeType failed: %d\n",err);
		exit(1);
	}

	err=FT_New_Face(lib,fontPath,faceIndex,&face);
	if (err) {
		fprintf(stderr,"FT_New_Face failed: %d\n",err);
		exit(1);
	}

	err=FT_Select_Charmap(face,FT_ENCODING_UNICODE);
	if (err) {
		fprintf(stderr,"FT_Select_Charmap failed: %d\n",err);
		exit(1);
	}

	if (
		(face->face_flags&FT_FACE_FLAG_SCALABLE)!=0 ||
		face->num_fixed_sizes==0
	) {
		charHeight=desiredCharHeight;
		extraHeight=charHeight*64/16;
		k=charHeight-extraHeight/64;
		for (i=0; i<2; i++) {
			err=FT_Set_Pixel_Sizes(face,0,k);
			if (err) {
				fprintf(stderr,"FT_Set_Pixel_Sizes failed: %d\n",err);
				exit(1);
			}
			k=k*(charHeight*64-extraHeight)/face->size->metrics.height;
		}
		j=face->size->metrics.max_advance;
		extraWidth=j/32;
		charWidth=(j+extraWidth+32)/64;
		originX=extraWidth/2;
		originY=extraHeight+face->size->metrics.ascender;
	}
	else {
		for (i=0, j=0, k=INT_MAX; i<face->num_fixed_sizes; i++) {
			l=desiredCharHeight-face->available_sizes[i].height;
			if (l<0) l=-l;
			if (k>l) { k=l; j=i; }
		}
		charHeight=face->available_sizes[j].height;
		err=FT_Select_Size(face,j);
		if (err) {
			fprintf(stderr,"FT_Select_Size failed: %d\n",err);
			exit(1);
		}
		charWidth=(face->size->metrics.max_advance+32)/64;
		originX=0;
		originY=face->size->metrics.ascender;
	}

	for (i=0; CharsetTable[i].Name; i++) {

		numCodes=CharsetTable[i].LastCode-CharsetTable[i].FirstCode+1;

		for (j=numCodes-1; j>=0; j--) {
			if (FT_Get_Char_Index(face,CharsetTable[i].FirstCode+j)) break;
		}
		if (j<0) continue;

		for (j=16; (numCodes+j-1)/j>j*2; j<<=1);
		mapWidth=charWidth*(j<numCodes?j:numCodes);
		mapHeight=charHeight*((numCodes+j-1)/j);
		map=(unsigned char*)calloc(mapWidth,mapHeight);

		for (x=0, y=0, j=0; j<numCodes; j++) {
			glyph=FT_Get_Char_Index(face,CharsetTable[i].FirstCode+j);
			if (glyph) {
				vec.x=originX%64;
				vec.y=-(originY%64);
				FT_Set_Transform(face,NULL,&vec); /* No effect on bitmap fonts. */
				err=FT_Load_Glyph(face,glyph,FT_LOAD_RENDER);
				if (err) {
					fprintf(stderr,"FT_Load_Char failed: %d\n",err);
					exit(1);
				}
				PutFTBitmap(
					mapWidth,mapHeight,map,
					x,y,x+charWidth,y+charHeight,
					x+originX/64+face->glyph->bitmap_left,
					y+originY/64-face->glyph->bitmap_top,
					&face->glyph->bitmap
				);
			}
			else {
				PutUnknown(
					mapWidth,mapHeight,map,
					x,y,charWidth,charHeight
				);
			}
			x+=charWidth;
			if (x>=mapWidth) { x=0; y+=charHeight; }
		}

		sprintf(
			tmp,
			"%s/%05X-%05X_%dx%d_%s_converted.tga",
			targetDir,
			CharsetTable[i].FirstCode,CharsetTable[i].LastCode,
			charWidth,charHeight,
			CharsetTable[i].Name
		);
		WriteGreyTGA(mapWidth,mapHeight,map,tmp);

		free(map);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(lib);
}


int main(int argc, char * * argv)
{
	const char * fontPath;
	const char * targetDir;
	int desiredCharHeight;

	if (argc<3 || argc>4) {
		fprintf(
			stderr,
			"Usage: %s <font file> <target dir> [<desired height>]\n"
			"Please read the docs!\n",
			argv[0]
		);
		exit(1);
	}
	fontPath=argv[1];
	targetDir=argv[2];
	desiredCharHeight=(argc>3 ? atoi(argv[3]) : 112);

	if (mkdir(targetDir,0777)!=0 && errno!=EEXIST) {
		fprintf(
			stderr,
			"Failed to create directory \"%s\": %s\n",
			targetDir,strerror(errno)
		);
		exit(1);
	}

	ConvertFont(fontPath,0,targetDir,desiredCharHeight);

	return 0;
}
