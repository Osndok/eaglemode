//------------------------------------------------------------------------------
// emTexture.h
//
// Copyright (C) 2020,2022 Oliver Hamann.
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
//------------------------------------------------------------------------------

#ifndef emTexture_h
#define emTexture_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif


//==============================================================================
//================================= emTexture ==================================
//==============================================================================

class emTexture {

public:

	// Class for a texture. A texture in this sense describes how to fill an
	// area when painting figures with emPainter. Different types of
	// textures allow to paint colors, images, colored images, and color
	// gradients. Those types of textures can be easily constructed using
	// the following derived classes:
	//
	//   emColorTexture
	//   emImageTexture
	//   emImageColoredTexture
	//   emLinearGradientTexture
	//   emRadialGradientTexture
	//
	// These texture classes just exist for convenience and define only
	// constructors. All the attributes (and even the constructors) are
	// already defined here in emTexture, because emTexture ought to be a
	// simple non-polymorphic type.

	enum TextureType {
		// Data type for the type of texture.

		COLOR,
			// Uni-colored texture, constructed by emColorTexture.

		IMAGE,
			// Image texture, constructed by emImageTexture.

		IMAGE_COLORED,
			// Image colored texture, constructed by
			// emImageColoredTexture.

		LINEAR_GRADIENT,
			// Linear gradient texture, constructed by
			// emLinearGradientTexture.

		RADIAL_GRADIENT
			// Radial gradient texture, constructed by
			// emRadialGradientTexture.
	};

	enum ExtensionType {
		// Data type for describing what to paint beyond the edges of an
		// image, or how to "extend" the image. With emImageTexture or
		// emImageColoredTexture, an image (or a source rectangle of it)
		// is painted to a certain target rectangle. The extension
		// parameter controls what should happen when the painted figure
		// shows something outside that rectangle. Remember, even if the
		// figure is exactly that target rectangle, the interpolation
		// and anti-aliasing algorithms may look slightly beyond the
		// edges. In that case, it is important to choose correctly
		// between EXTEND_EDGE and EXTEND_ZERO. Usually EXTEND_EDGE is
		// good for painting images which fill the rectangle completely
		// (no alpha), and EXTEND_ZERO is good for painting icons and
		// text characters (things with transparency).

		EXTEND_TILED = 0,
			// The image simply is repeated.

		EXTEND_EDGE = 1,
			// The edges of the image are extended (stretched)
			// infinitely.

		EXTEND_ZERO = 2,
			// Black (zero) color is used, or - if the image has an
			// alpha channels - black with zero alpha.

		EXTEND_EDGE_OR_ZERO = 3
			// Automatically decide between EXTEND_EDGE and
			// EXTEND_ZERO: If the image has an alpha channel, or if
			// the texture type is IMAGE_COLORED and one of the
			// gradient colors has zero alpha, then EXTEND_ZERO is
			// used, otherwise EXTEND_EDGE.
	};

	enum DownscaleQualityType {
		// Data type for the quality of downscaling images. Something
		// like "3X3" means the maximum number of input pixels used to
		// area-sample an output pixel. If there are more input pixels,
		// they are reduced by nearest-pixel sampling on the fly.
		DQ_BY_CONFIG     = -1,
		DQ_NEAREST_PIXEL = 0,
		DQ_2X2           = 2,
		DQ_3X3           = 3,
		DQ_4X4           = 4,
		DQ_5X5           = 5,
		DQ_6X6           = 6
	};

	enum UpscaleQualityType {
		// Data type for the quality of upscaling images.
		UQ_BY_CONFIG           = -2,
		UQ_BY_CONFIG_FOR_VIDEO = -1,
		UQ_NEAREST_PIXEL       = 0,
		UQ_AREA_SAMPLING       = 1,
		UQ_BILINEAR            = 2,
		UQ_BICUBIC             = 3,
		UQ_LANCZOS             = 4,
		UQ_ADAPTIVE            = 5
	};

	emTexture(emColor color=emColor::GRAY);
	emTexture(emUInt32 packedColor);
		// Construct an uni-colored texture. Please see emColorTexture
		// for details.

	emTexture(double x, double y, double w, double h,
	          const emImage & image, int alpha=255,
	          ExtensionType extension=EXTEND_TILED,
	          DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	          UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
	emTexture(double x, double y, double w, double h,
	          const emImage & image, int srcX, int srcY,
	          int srcW, int srcH, int alpha=255,
	          ExtensionType extension=EXTEND_TILED,
	          DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	          UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
		// Construct an image texture. Please see emImageTexture
		// for details.

	emTexture(double x, double y, double w, double h,
	          const emImage & image,
	          emColor color1, emColor color2,
	          ExtensionType extension=EXTEND_TILED,
	          DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	          UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
	emTexture(double x, double y, double w, double h,
	          const emImage & image,
	          int srcX, int srcY, int srcW, int srcH,
	          emColor color1, emColor color2,
	          ExtensionType extension=EXTEND_TILED,
	          DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	          UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
		// Construct an image colored texture. Please see
		// emImageColoredTexture for details.

	emTexture(double x1, double y1, emColor color1,
	          double x2, double y2, emColor color2);
		// Construct a linear gradient texture. Please see
		// emLinearGradientTexture for details.

	emTexture(double x, double y, double w, double h,
	          emColor color1, emColor color2);
		// Construct a radial gradient texture. Please see
		// emRadialGradientTexture for details.

	emTexture(const emTexture& other);
		// Construct a copied texture.

	emTexture& operator = (const emTexture& other);
		// Copy a texture.

	TextureType GetType() const;
		// Get the type of the texture.

	double GetX() const;
	void SetX(double x);
	double GetY() const;
	void SetY(double y);
	double GetW() const;
	void SetW(double w);
	double GetH() const;
	void SetH(double h);
		// Get or set the position and size of the target rectangle.
		// This is valid only if the texture type is IMAGE,
		// IMAGE_COLORED, or RADIAL_GRADIENT.

	double GetX1() const;
	void SetX1(double x1);
	double GetY1() const;
	void SetY1(double y1);
	double GetX2() const;
	void SetX2(double x2);
	double GetY2() const;
	void SetY2(double y2);
		// Get or set the first and second target point of a gradient.
		// This is valid only if the texture type is LINEAR_GRADIENT.

	emColor GetColor() const;
	void SetColor(emColor color);
		// Get or set the color. This is valid only if the texture type
		// is COLOR.

	emColor GetColor1() const;
	void SetColor1(emColor color1);
	emColor GetColor2() const;
	void SetColor2(emColor color2);
		// Get or set the First and second color of a gradient. This is
		// valid only if the texture type is IMAGE_COLORED,
		// LINEAR_GRADIENT, or RADIAL_GRADIENT.

	const emImage & GetImage() const;
	void SetImage(const emImage & image);
		// Get or set the image. The image reference must be valid for the life time of the
		// texture. This is valid only if the texture type is IMAGE or IMAGE_COLORED.

	int GetSrcX() const;
	void SetSrcX(int srcX);
	int GetSrcY() const;
	void SetSrcY(int srcY);
	int GetSrcW() const;
	void SetSrcW(int srcW);
	int GetSrcH() const;
	void SetSrcH(int srcH);
		// Get or set the source rectangle on the image. This is valid
		// only if the texture type is IMAGE or IMAGE_COLORED.

	int GetAlpha() const;
	void SetAlpha(int alpha);
		// Get or set the alpha value for image blending (0..255). This
		// is valid only if the texture type is IMAGE.

	ExtensionType GetExtension() const;
	void SetExtension(ExtensionType extension);
		// Get or set how to extend the image. This is valid only if the
		// texture type is IMAGE or IMAGE_COLORED.

	DownscaleQualityType GetDownscaleQuality() const;
	void SetDownscaleQuality(DownscaleQualityType downscaleQuality);
		// Get or set the quality of downscaling the image. This is
		// valid only if the texture type is IMAGE or IMAGE_COLORED.

	UpscaleQualityType GetUpscaleQuality() const;
	void SetUpscaleQuality(UpscaleQualityType upscaleQuality);
		// Get or set the quality of upscaling the image. This is valid
		// only if the texture type is IMAGE or IMAGE_COLORED.

private:

	friend class emPainter;

	// *** WARNING for future extensions: Copy constructor+operator do a memcpy!
	TextureType Type;
	ExtensionType Extension;
	DownscaleQualityType DownscaleQuality;
	UpscaleQualityType UpscaleQuality;
	union { emColor Color; emColor Color1; };
	union { emColor Color2; int Alpha; };
	const emImage * Image;
	int SrcX,SrcY,SrcW,SrcH;
	union { double X; double X1; };
	union { double Y; double Y1; };
	union { double W; double X2; };
	union { double H; double Y2; };
};


//==============================================================================
//=============================== emColorTexture ===============================
//==============================================================================

class emColorTexture : public emTexture {

public:

	emColorTexture(emColor color);
		// Construct an uni-colored texture.
		// Arguments:
		//   color - The color.
};


//==============================================================================
//=============================== emImageTexture ===============================
//==============================================================================

class emImageTexture : public emTexture {

public:

	emImageTexture(double x, double y, double w, double h,
	               const emImage & image, int alpha=255,
	               ExtensionType extension=EXTEND_TILED,
	               DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	               UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
	emImageTexture(double x, double y, double w, double h,
	               const emImage & image, int srcX, int srcY,
	               int srcW, int srcH, int alpha=255,
	               ExtensionType extension=EXTEND_TILED,
	               DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	               UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
		// Construct an image texture. This type of texture simply
		// paints an image.
		// Arguments:
		//   x,y,w,h             - Upper-left corner and size of the
		//                         target rectangle. The image (or a
		//                         source rectangle of it) is fitted
		//                         into this rectangle.
		//   image               - The image. If the image has an alpha
		//                         channel, it is used for blending. The
		//                         image reference must be valid for the
		//                         life time of the texture.
		//   srcX,srcY,srcW,srcH - Upper-left corner and size of the
		//                         source rectangle on the image. If
		//                         these arguments are missing, the
		//                         whole image is taken.
		//   alpha               - An additional alpha value for
		//                         blending (0-255).
		//   extension             What is painted beyond the target
		//                         rectangle (x,y,w,h). Please see the
		//                         comments on emTexture::ExtensionType
		//                         for details.
		//   downscaleQuality    - Quality of downscaling the image.
		//                         Please see the comments on
		//                         emTexture::DownscaleQualityType for
		//                         details.
		//   upscaleQuality      - Quality of upscaling the image.
		//                         Please see the comments on
		//                         emTexture::UpscaleQualityType for
		//                         details.
};


//==============================================================================
//=========================== emImageColoredTexture ============================
//==============================================================================

class emImageColoredTexture : public emTexture {

public:

	emImageColoredTexture(double x, double y, double w, double h,
	                      const emImage & image,
	                      emColor color1, emColor color2,
	                      ExtensionType extension=EXTEND_TILED,
	                      DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	                      UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
	emImageColoredTexture(double x, double y, double w, double h,
	                      const emImage & image,
	                      int srcX, int srcY, int srcW, int srcH,
	                      emColor color1, emColor color2,
	                      ExtensionType extension=EXTEND_TILED,
	                      DownscaleQualityType downscaleQuality=DQ_BY_CONFIG,
	                      UpscaleQualityType upscaleQuality=UQ_BY_CONFIG);
		// Construct an image colored texture. This is like an image
		// texture, but the colors of the image are modified by two
		// given colors. One could also say this paints a color gradient
		// where the image color values are used as the gradient
		// parameter. The general formula for calculating the color is:
		//
		//    output = color1 + input * (color2 - color1)
		//
		// If the image has only one color channel (gray image), that
		// channel is used as the input parameter to the formula.
		// Otherwise the calculation happens for red, green and blue
		// separately. This may not be very useful. Ideally, the image
		// should always have just one color channel (and optionally an
		// alpha channel, which is used for blending).
		//
		// Arguments:
		//   x,y,w,h             - Upper-left corner and size of the
		//                         target rectangle. The image (or a
		//                         source rectangle of it) is fitted
		//                         into this rectangle.
		//   image               - The image. If the image has an alpha
		//                         channel, it is used for blending. The
		//                         image reference must be valid for the
		//                         life time of the texture.
		//   srcX,srcY,srcW,srcH - Upper-left corner and size of the
		//                         source rectangle on the image. If
		//                         these arguments are missing, the
		//                         whole image is taken.
		//   color1              - First color of gradient.
		//   color2              - Second color of gradient.
		//   extension             What is painted beyond the target
		//                         rectangle (x,y,w,h). Please see the
		//                         comments on emTexture::ExtensionType
		//                         for details.
		//   downscaleQuality    - Quality of downscaling the image.
		//                         Please see the comments on
		//                         emTexture::DownscaleQualityType for
		//                         details.
		//   upscaleQuality      - Quality of upscaling the image.
		//                         Please see the comments on
		//                         emTexture::UpscaleQualityType for
		//                         details.
};


//==============================================================================
//========================== emLinearGradientTexture ===========================
//==============================================================================

class emLinearGradientTexture : public emTexture {

public:

	emLinearGradientTexture(double x1, double y1, emColor color1,
	                        double x2, double y2, emColor color2);
		// Construct a linear gradient texture. This defines a start
		// point with a start color and an end point with an end color.
		// The color gradient runs on the line from that start point to
		// the end point and in the same way on every possible parallel
		// line. In other words: Imagine a straight line at the start
		// point that is perpendicular to the start-end direction. Call
		// it the start straight. And also imagine an end straight in
		// the same way at the end point. Then the color gradient runs
		// linear from the start straight to the end straight. Outside
		// that area, the start color is used beyond the start, and the
		// end color is use beyond the end.
		// Arguments:
		//   x1,y1               - Start point of gradient.
		//   color1              - Start color of gradient.
		//   x2,y2               - End point of gradient.
		//   color2              - End color of gradient.
};


//==============================================================================
//========================== emRadialGradientTexture ===========================
//==============================================================================

class emRadialGradientTexture : public emTexture {

public:

	emRadialGradientTexture(double x, double y, double w, double h,
	                        emColor color1, emColor color2);
		// Construct a radial gradient texture. This defines an ellipse
		// (or circle) on the output area, as well as two colors. The
		// color gradient runs from inside out, with the start color at
		// the center and the end color at the outer edge. Outside the
		// ellipse, everything is filled with the end color.
		// Arguments:
		//   x,y,w,h             - Upper-left corner and size of the
		//                         target ellipse.
		//   color1              - Start color of gradient, used at the
		//                         center of the ellipse.
		//   color2              - End color of gradient, used at the
		//                         outer edge of the ellipse.
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//--------------------------------- emTexture ----------------------------------

inline emTexture::emTexture(emColor color)
	: Type(COLOR),Color(color)
{
}

inline emTexture::emTexture(emUInt32 packedColor)
	: Type(COLOR),Color(packedColor)
{
}

inline emTexture::emTexture(
	double x, double y, double w, double h, const emImage & image,
	int alpha, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	Type(IMAGE),Extension(extension),DownscaleQuality(downscaleQuality),
	UpscaleQuality(upscaleQuality),Alpha(alpha),Image(&image),
	SrcX(0),SrcY(0),SrcW(image.GetWidth()),SrcH(image.GetHeight()),
	X(x),Y(y),W(w),H(h)
{
}

inline emTexture::emTexture(
	double x, double y, double w, double h, const emImage & image, int srcX,
	int srcY, int srcW, int srcH, int alpha, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	Type(IMAGE),Extension(extension),DownscaleQuality(downscaleQuality),
	UpscaleQuality(upscaleQuality),Alpha(alpha),Image(&image),
	SrcX(srcX),SrcY(srcY),SrcW(srcW),SrcH(srcH),
	X(x),Y(y),W(w),H(h)
{
}

inline emTexture::emTexture(
	double x, double y, double w, double h, const emImage & image,
	emColor color1, emColor color2, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	Type(IMAGE_COLORED),Extension(extension),DownscaleQuality(downscaleQuality),
	UpscaleQuality(upscaleQuality),Color1(color1),Color2(color2),
	Image(&image),SrcX(0),SrcY(0),SrcW(image.GetWidth()),
	SrcH(image.GetHeight()),X(x),Y(y),W(w),H(h)
{
}

inline emTexture::emTexture(
	double x, double y, double w, double h, const emImage & image,
	int srcX, int srcY, int srcW, int srcH, emColor color1, emColor color2,
	ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	Type(IMAGE_COLORED),Extension(extension),DownscaleQuality(downscaleQuality),
	UpscaleQuality(upscaleQuality),Color1(color1),Color2(color2),
	Image(&image),SrcX(srcX),SrcY(srcY),SrcW(srcW),SrcH(srcH),
	X(x),Y(y),W(w),H(h)
{
}

inline emTexture::emTexture(
	double x1, double y1, emColor color1, double x2, double y2, emColor color2
) :
	Type(LINEAR_GRADIENT),Color1(color1),Color2(color2),
	X1(x1),Y1(y1),X2(x2),Y2(y2)
{
}

inline emTexture::emTexture(
	double x, double y, double w, double h, emColor color1, emColor color2
) :
	Type(RADIAL_GRADIENT),Color1(color1),Color2(color2),X(x),Y(y),W(w),H(h)
{
}

inline emTexture::emTexture(const emTexture& other)
{
	memcpy((void*)this,(const void*)&other,sizeof(emTexture));
}

inline emTexture& emTexture::operator = (const emTexture& other)
{
	memcpy((void*)this,(const void*)&other,sizeof(emTexture));
	return *this;
}

inline emTexture::TextureType emTexture::GetType() const
{
	return Type;
}

inline double emTexture::GetX() const
{
	return X;
}

inline void emTexture::SetX(double x)
{
	X=x;
}

inline double emTexture::GetY() const
{
	return Y;
}

inline void emTexture::SetY(double y)
{
	Y=y;
}

inline double emTexture::GetW() const
{
	return W;
}

inline void emTexture::SetW(double w)
{
	W=w;
}

inline double emTexture::GetH() const
{
	return H;
}

inline void emTexture::SetH(double h)
{
	H=h;
}

inline double emTexture::GetX1() const
{
	return X1;
}

inline void emTexture::SetX1(double x1)
{
	X1=x1;
}

inline double emTexture::GetY1() const
{
	return Y1;
}

inline void emTexture::SetY1(double y1)
{
	Y1=y1;
}

inline double emTexture::GetX2() const
{
	return X2;
}

inline void emTexture::SetX2(double x2)
{
	X2=x2;
}

inline double emTexture::GetY2() const
{
	return Y2;
}

inline void emTexture::SetY2(double y2)
{
	Y2=y2;
}

inline emColor emTexture::GetColor() const
{
	return Color;
}

inline void emTexture::SetColor(emColor color)
{
	Color=color;
}

inline emColor emTexture::GetColor1() const
{
	return Color1;
}

inline void emTexture::SetColor1(emColor color1)
{
	Color1=color1;
}

inline emColor emTexture::GetColor2() const
{
	return Color2;
}

inline void emTexture::SetColor2(emColor color2)
{
	Color2=color2;
}

inline const emImage & emTexture::GetImage() const
{
	return *Image;
}

inline void emTexture::SetImage(const emImage & image)
{
	Image=&image;
}

inline int emTexture::GetSrcX() const
{
	return SrcX;
}

inline void emTexture::SetSrcX(int srcX)
{
	SrcX=srcX;
}

inline int emTexture::GetSrcY() const
{
	return SrcY;
}

inline void emTexture::SetSrcY(int srcY)
{
	SrcY=srcY;
}

inline int emTexture::GetSrcW() const
{
	return SrcW;
}

inline void emTexture::SetSrcW(int srcW)
{
	SrcW=srcW;
}

inline int emTexture::GetSrcH() const
{
	return SrcH;
}

inline void emTexture::SetSrcH(int srcH)
{
	SrcH=srcH;
}

inline int emTexture::GetAlpha() const
{
	return Alpha;
}

inline void emTexture::SetAlpha(int alpha)
{
	Alpha=alpha;
}

inline emTexture::ExtensionType emTexture::GetExtension() const
{
	return Extension;
}

inline void emTexture::SetExtension(ExtensionType extension)
{
	Extension=extension;
}

inline emTexture::DownscaleQualityType emTexture::GetDownscaleQuality() const
{
	return DownscaleQuality;
}

inline void emTexture::SetDownscaleQuality(DownscaleQualityType downscaleQuality)
{
	DownscaleQuality=downscaleQuality;
}

inline emTexture::UpscaleQualityType emTexture::GetUpscaleQuality() const
{
	return UpscaleQuality;
}

inline void emTexture::SetUpscaleQuality(UpscaleQualityType upscaleQuality)
{
	UpscaleQuality=upscaleQuality;
}


//------------------------------- emColorTexture -------------------------------

inline emColorTexture::emColorTexture(emColor color)
	: emTexture(color)
{
}


//------------------------------- emImageTexture -------------------------------

inline emImageTexture::emImageTexture(
	double x, double y, double w, double h, const emImage & image,
	int alpha, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	emTexture(x,y,w,h,image,alpha,extension,downscaleQuality,upscaleQuality)
{
}

inline emImageTexture::emImageTexture(
	double x, double y, double w, double h, const emImage & image, int srcX,
	int srcY, int srcW, int srcH, int alpha, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	emTexture(x,y,w,h,image,srcX,srcY,srcW,srcH,alpha,extension,
	          downscaleQuality,upscaleQuality)
{
}


//--------------------------- emImageColoredTexture ----------------------------

inline emImageColoredTexture::emImageColoredTexture(
	double x, double y, double w, double h, const emImage & image,
	emColor color1, emColor color2, ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	emTexture(x,y,w,h,image,color1,color2,extension,downscaleQuality,
	          upscaleQuality)
{
}

inline emImageColoredTexture::emImageColoredTexture(
	double x, double y, double w, double h, const emImage & image,
	int srcX, int srcY, int srcW, int srcH, emColor color1, emColor color2,
	ExtensionType extension,
	DownscaleQualityType downscaleQuality, UpscaleQualityType upscaleQuality
) :
	emTexture(x,y,w,h,image,srcX,srcY,srcW,srcH,color1,color2,extension,
	          downscaleQuality,upscaleQuality)
{
}


//-------------------------- emLinearGradientTexture ---------------------------

inline emLinearGradientTexture::emLinearGradientTexture(
	double x1, double y1, emColor color1, double x2, double y2, emColor color2
) :
	emTexture(x1,y1,color1,x2,y2,color2)
{
}


//-------------------------- emRadialGradientTexture ---------------------------

inline emRadialGradientTexture::emRadialGradientTexture(
	double x, double y, double w, double h, emColor color1, emColor color2
) :
	emTexture(x,y,w,h,color1,color2)
{
}

#endif
