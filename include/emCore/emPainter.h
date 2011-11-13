//------------------------------------------------------------------------------
// emPainter.h
//
// Copyright (C) 2001,2003-2010 Oliver Hamann.
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

#ifndef emPainter_h
#define emPainter_h

#ifndef emFontCache_h
#include <emCore/emFontCache.h>
#endif


//==============================================================================
//================================= emPainter ==================================
//==============================================================================

class emPainter {

public:

	// Class with methods for painting pixel graphics with full
	// anti-aliasing and alpha blending. The anti-aliasing has been driven
	// very far here. Therefore the interface looks like for painting vector
	// graphics. But there is one topic a programmer should know about:
	//
	// "Canvas Color"
	// ==============
	//
	// Every paint method has an argument called canvasColor. For getting
	// best results, this argument has to be set carefully:
	//
	// Quick instructions
	// ------------------
	//
	// Whenever painting something over a uni-colored area, and if that
	// color is known, it should be given as the argument canvasColor.
	// Otherwise a non-opaque color should be given (e.g. 0).
	//
	// When wondering whether an area is uni-colored or not, you should not
	// care about rasterization by pixels. Just imagine all paint operations
	// would go to infinite pixel resolution.
	//
	// More details
	// ------------
	//
	// With a non-opaque canvas color, the classic formula for blending is
	// used:
	//
	//   targetNew = targetOld * (100% - alpha) + source * alpha
	//
	// Clearly: For anti-aliasing, the intersection of the area of the
	// painted object and the area of each pixel is calculated. This results
	// in an additional alpha value for each individual pixel. This
	// anti-alias-alpha-value is simply multiplied with the general alpha
	// value of the operation.
	//
	// Now, the problem is when painting two objects which are cutting the
	// same pixel. The resulting pixel value will not be correct. Here is an
	// example: Lets paint two white rectangles on black background. The
	// rectangles are not overlapping, but they have a common edge, which
	// runs through the center of a pixel (each rectangle gets 50% of the
	// pixel). When painting the first rectangle, the formula says for that
	// pixel: black * (100% - 50%) + white * 50%, which results "50% white".
	// That is correct. But when painting the second rectangle, we have:
	// 50% white * (100% - 50%) + white * 50%, which results 75% white. But
	// it should be 100%!
	//
	// With an opaque canvas color, the formula for blending is:
	//
	//   targetNew = targetOld + (source - canvasColor) * alpha
	//
	// This is faster, because it allows some pretty optimizations. But the
	// main advantage is that this formula does not make the error described
	// above. In our example, the first rectangle would result equal for the
	// considered pixel: black + (white - black) * 50% = 50% white. And when
	// painting the second rectangle, the formula says: 50% white + (white -
	// black) * 50% = 100% white, which is correct now. The formula is even
	// correct when working with other colors than just black and white,
	// when having alpha blending by the source, and when many painted
	// objects fall into a pixel.
	//
	// Of course, there are a lot of situation where the canvas color
	// technique cannot be used, because the painted areas are not
	// uni-colored, or their color is not known. But these are often
	// situations where the errors are not so spectacular. However, a last
	// chance could be the method PaintEdgeCorrection (read there).

	emPainter();
		// Construct a painter which paints nowhere.

	emPainter(const emPainter & painter);
		// Construct a painter by copying all the settings from another
		// painter.
		// Arguments:
		//   painter - A painter whose settings are to be copied.

	emPainter(const emPainter & painter, double clipX1, double clipY1,
	          double clipX2, double clipY2);
		// Construct a painter by copying all the settings from another
		// painter, but intersect the clipping rectangle with a given
		// clipping rectangle.
		// Arguments:
		//   painter   - A painter whose settings are to be copied.
		//   clipX1,clipY1,clipX2,clipY2 - The clipping rectangle in
		//               pixel coordinates. It will be intersected with
		//               the clipping rectangle from the source painter.

	emPainter(const emPainter & painter, double clipX1, double clipY1,
	          double clipX2, double clipY2, double originX,
	          double originY, double scaleX,double scaleY);
		// Construct a painter by copying all the settings from another
		// painter, but intersect the clipping rectangle with a given
		// clipping rectangle, and set another transformation.
		// Arguments:
		//   painter         - A painter whose settings are to be
		//                     copied.
		//   clipX1,clipY1,clipX2,clipY2 - The clipping rectangle in
		//                     pixel coordinates. It will be intersected
		//                     with the clipping rectangle from the
		//                     source painter.
		//   originX,originY - The new origin (see SetOrigin).
		//   scaleX,scaleY   - The new scale factors (see SetScaling).

	emPainter(emRootContext & rootContext, void * map, int bytesPerRow,
	          int bytesPerPixel, emUInt32 redMask, emUInt32 greenMask,
	          emUInt32 blueMask, double clipX1, double clipY1,
	          double clipX2, double clipY2, double originX=0,
	          double originY=0, double scaleX=1, double scaleY=1);
		// Construct a painter from scratch. The output bitmap is
		// addressed with:
		//   pixelValue = ((TYPE*)(map+y*bytesPerRow)))[x]
		// Where:
		//   x is the X-coordinate of the pixel in the range of
		//   (int)clipX1 to (int)ceil(clipX2)-1.
		//   y is the Y-coordinate of the pixel in the range of
		//   (int)clipY1 to (int)ceil(clipY2)-1.
		//   TYPE is an 8, 16 or 32-bit integer type, according to
		//   bytesPerPixel.
		//   pixelValue is the pixel color consisting of three
		//   channels, according to redMask, greenMask and blueMask.
		// Arguments:
		//   rootContext     - The root context.
		//   map             - Pointer to the output bitmap.
		//   bytesPerRow     - Size of a row in the map, in bytes.
		//   bytesPerPixel   - Size of a pixel in the map, in bytes.
		//                     This must be 1, 2 or 4.
		//   redMask         - A bit mask which denotes the red channel
		//                     within a pixel value.
		//   greenMask       - A bit mask which denotes the green
		//                     channel within a pixel value.
		//   blueMask        - A bit mask which denotes the blue channel
		//                     within a pixel value.
		//   clipX1,clipY1,clipX2,clipY2 - The clipping rectangle (see
		//                     SetClipping).
		//   originX,originY - The origin (see SetOrigin).
		//   scaleX,scaleY   - The scale factors (see SetScaling).

	// Even have a look at emImage::PreparePainter

	~emPainter();
		// Destructor.

	emPainter & operator = (const emPainter & painter);
		// Copy all the settings from another painter to this painter.

	double GetClipX1() const;
	double GetClipY1() const;
	double GetClipX2() const;
	double GetClipY2() const;
	void SetClipping(double clipX1, double clipY1, double clipX2,
	                 double clipY2);
		// Get or set the clipping rectangle. It is in pixel
		// coordinates. Note that these can be fractional numbers - a
		// paint operation on a partly clipped pixel will result in an
		// appropriate blending operation.

	double GetUserClipX1() const;
	double GetUserClipY1() const;
	double GetUserClipX2() const;
	double GetUserClipY2() const;
		// Get the clipping rectangle in user coordinates.

	double GetOriginX() const;
	double GetOriginY() const;
	double GetScaleX() const;
	double GetScaleY() const;
	void SetOrigin(double originX, double originY);
	void SetScaling(double scaleX, double scaleY);
	void SetTransformation(double originX, double originY,
	                       double scaleX, double scaleY);
		// Get or set the user coordinate system, which is used by all
		// the painting methods. The transformation of user coordinates
		// to pixel coordinates is:
		//   xPixels = xUser * ScaleX + OriginX
		//   yPixels = yUser * ScaleY + OriginY
		// The scale factors must be positive!

	double RoundX(double x) const;
	double RoundY(double y) const;
	double RoundDownX(double x) const;
	double RoundDownY(double y) const;
	double RoundUpX(double x) const;
	double RoundUpY(double y) const;
		// Round user coordinates to pixel boundary.


	//-------------------- Painting areas in uni-color ---------------------

	void Clear(emColor color=emColor::BLACK, emColor canvasColor=0) const;
		// Like PaintRect on the whole clipping rectangle.

	void PaintRect(double x, double y, double w, double h,
	               emColor color, emColor canvasColor=0) const;
		// Paint a rectangle.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the rectangle.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.

	void PaintPolygon(const double xy[], int n, emColor color,
	                  emColor canvasColor=0) const;
		// Paint a polygon. The polygon may have holes, and it does not
		// matter whether the edges run clockwise or counterclockwise.
		// But there must not be any crossings in the edges.
		// Arguments:
		//   xy[]        - Coordinates of the polygon vertices. The
		//                 array elements are:
		//                 x0, y0, x1, y1, x2, y2, ..., x(n-1), y(n-1)
		//   n           - Number of vertices.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.

	void PaintEdgeCorrection(double x1, double y1, double x2, double y2,
	                         emColor color1, emColor color2) const;
		// If you don't have a canvas color when painting adjacent
		// polygons (or rectangles), this method can be helpful in
		// reducing the visual errors at the edges. Just call this to
		// paint the correction over the contact edge of two polygons
		// which have already been painted without canvas color. color1
		// must be the color argument used for painting the first
		// polygon, and color2 must be the color argument used for the
		// other polygon which has been painted later. (x1,y1) and
		// (x2,y2) are the vertices of the contact edge, but the order
		// is important: When looking from (x1,y1) to (x2,y2), the first
		// polygon must be on the left.

	void PaintEllipse(double x, double y, double w, double h,
	                  emColor color, emColor canvasColor=0) const;
	void PaintEllipse(double x, double y, double w, double h,
	                  double startAngle, double rangeAngle,
	                  emColor color, emColor canvasColor=0) const;
		// Paint an ellipse or a sector of an ellipse.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the bounding
		//                 rectangle of the ellipse.
		//   startAngle  - Start angle of the sector in degrees. Zero
		//                 points to the right, 90 points down...
		//   rangeAngle  - Range angle of the sector.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.

	void PaintRoundRect(double x, double y, double w, double h,
	                    double rx, double ry, emColor color,
	                    emColor canvasColor=0) const;
		// Paint a rectangle with elliptic corners.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the rectangle.
		//   rx,ry       - Radiuses of the ellipses.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.


	//-------------------- Painting lines in uni-color ---------------------

	enum LineCap {
		LC_FLAT,
		LC_SQUARE,
		LC_ROUND
	};
	void PaintLine(double x1, double y1, double x2, double y2,
	               double thickness, LineCap cap1, LineCap cap2,
	               emColor color, emColor canvasColor=0) const;
		// Paint a line.
		// Arguments:
		//   x1,y1       - Coordinates of the first end of the line.
		//   x2,y2       - Coordinates of the other end of the line.
		//   thickness   - Width of the stroke.
		//   cap1,cap2   - How to paint the endings:
		//                  LC_FLAT: Do not extend the ending.
		//                  LC_SQUARE: Extend by thickness/2.
		//                  LC_ROUND: Have a semicircle.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.

	void PaintRectOutline(double x, double y, double w, double h,
	                      double thickness, emColor color,
	                      emColor canvasColor=0) const;
	void PaintPolygonOutline(const double xy[], int n, double thickness,
	                         emColor color, emColor canvasColor=0) const;
	void PaintEllipseOutline(double x, double y, double w, double h,
	                         double thickness, emColor color,
	                         emColor canvasColor=0) const;
	void PaintEllipseOutline(double x, double y, double w, double h,
	                         double startAngle, double rangeAngle,
	                         double thickness, emColor color,
	                         emColor canvasColor=0) const;
	void PaintRoundRectOutline(double x, double y, double w, double h,
	                           double rx, double ry, double thickness,
	                           emColor color, emColor canvasColor=0) const;
		// These are like PaintRect, PaintPolygon, PaintEllipse and
		// PaintRoundRect, but the objects are outlined instead of being
		// filled. The argument thickness is the width of the stroke.
		// The lines are centered on the boundary of the objects. The
		// second version of PaintEllipseOutline paints just an arc
		// (instead of the outline of a pie).
		// ??? BUG: PaintPolygonOutline currently always ignores the
		// canvasColor, and it does not produces correct results if the
		// alpha channel of the color is not 255.


	//------------------------ Painting from images ------------------------

	void PaintShape(double x, double y, double w, double h,
	                const emImage & img, int channel=0,
	                emColor color=emColor::WHITE,
	                emColor canvasColor=0) const;
	void PaintShape(double x, double y, double w, double h,
	                const emImage & img, double srcX, double srcY,
	                double srcW, double srcH, int channel=0,
	                emColor color=emColor::WHITE,
	                emColor canvasColor=0) const;
		// Paint a rectangle from one channel of an image or sub-image.
		// This is just like PaintRect, but the image channel is used as
		// an additional alpha channel for blending.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the target
		//                 rectangle.
		//   img         - The image.
		//   srcX,srcY,srcW,srcH - Upper-left corner and size of the
		//                 source rectangle on the image. If these
		//                 arguments are missing, the whole image is
		//                 taken.
		//   channel     - The desired channel of the image.
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.

	void PaintImage(double x, double y, double w, double h,
	                const emImage & img, int alpha=255,
	                emColor canvasColor=0) const;
	void PaintImage(double x, double y, double w, double h,
	                const emImage & img, double srcX, double srcY,
	                double srcW, double srcH, int alpha=255,
	                emColor canvasColor=0) const;
		// Paint a rectangle from an image or from a sub-image.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the target
		//                 rectangle.
		//   img         - The image. If the image has an alpha channel,
		//                 it is used for blending.
		//   srcX,srcY,srcW,srcH - Upper-left corner and size of the
		//                 source rectangle on the image. If these
		//                 arguments are missing, the whole image is
		//                 taken.
		//   alpha       - An additional alpha value for blending
		//                 (0-255).
		//   canvasColor - Please read the general comments more above.

	void PaintBorderShape(
		double x, double y, double w, double h,
		double l, double t, double r, double b,
		const emImage & img,
		double srcL, double srcT, double srcR, double srcB,
		int channel=0,
		emColor color=emColor::WHITE, emColor canvasColor=0,
		int whichSubRects=0757
	) const;
	void PaintBorderShape(
		double x, double y, double w, double h,
		double l, double t, double r, double b,
		const emImage & img,
		double srcX, double srcY, double srcW, double srcH,
		double srcL, double srcT, double srcR, double srcB,
		int channel=0,
		emColor color=emColor::WHITE, emColor canvasColor=0,
		int whichSubRects=0757
	) const;
	void PaintBorderImage(
		double x, double y, double w, double h,
		double l, double t, double r, double b,
		const emImage & img,
		double srcL, double srcT, double srcR, double srcB,
		int alpha=255, emColor canvasColor=0,
		int whichSubRects=0757
	) const;
	void PaintBorderImage(
		double x, double y, double w, double h,
		double l, double t, double r, double b,
		const emImage & img,
		double srcX, double srcY, double srcW, double srcH,
		double srcL, double srcT, double srcR, double srcB,
		int alpha=255, emColor canvasColor=0,
		int whichSubRects=0757
	) const;
		// Like PaintShape and PaintImage, but with a special type of
		// scaling, typically used for painting borders. The rectangle
		// is divided into a grid of nine sub-rectangles: four corners,
		// four edges and an inner rectangle. The operation allows to
		// change the height of the upper and lower edges and the width
		// of the left and right edges, in relation to the size of the
		// whole rectangle. The other sub-rectangles are adapted
		// accordingly.
		// Arguments:
		//   x,y,w,h     - Upper-left corner and size of the target
		//                 rectangle.
		//   l,t,r,b     - Thickness of the left, top, right and bottom
		//                 edges on the target.
		//   img         - The image.
		//   srcX,srcY,srcW,srcH - Upper-left corner and size of the
		//                 source rectangle on the image. If these
		//                 arguments are missing, the whole image is
		//                 taken.
		//   srcL,srcT,srcR,srcB - Thickness of the left, top, right
		//                 and bottom edges on the image.
		//   channel     - The desired channel of the image.
		//   color       - The color (alpha part is used for blending).
		//   alpha       - An additional alpha value for blending
		//                 (0-255).
		//   canvasColor - Please read the general comments more above.
		//   whichSubRects - Which of the 9 sub-rectangles are to be
		//                 painted. This is a bit mask. Bit numbers are:
		//                  8 = upper left | 5 = upper | 2 = upper right
		//                  7 = left       | 4 = inner | 1 = right
		//                  6 = lower left | 3 = lower | 0 = lower right
		//                 The default of 0757 means to paint just the
		//                 corners and edges, not the inner part.


	//--------------------------- Painting texts ---------------------------

	void PaintText(double x, double y, const char * text, double charHeight,
	               double widthScale, emColor color, emColor canvasColor=0,
	               int textLen=INT_MAX) const;
		// Paint a single line of raw text. Any formattings are not
		// interpreted.
		// Arguments:
		//   x,y         - Upper-left corner of the first character.
		//   text        - The character string, terminated by a
		//                 null-character or through the textLen
		//                 argument.
		//   charHeight  - The character height. This includes ascenders
		//                 and descenders.
		//   widthScale  - Factor for making the characters wider (>1.0)
		//                 or less wide (<1.0).
		//   color       - The color (alpha part is used for blending).
		//   canvasColor - Please read the general comments more above.
		//   textLen     - Length of the character string if not
		//                 null-terminated.

	void PaintTextBoxed(double x, double y, double w, double h,
	                    const char * text, double maxCharHeight,
	                    emColor color, emColor canvasColor=0,
	                    emAlignment boxAlignment=EM_ALIGN_CENTER,
	                    emAlignment textAlignment=EM_ALIGN_LEFT,
	                    double minWidthScale=0.5, bool formatted=true,
	                    double relLineSpace=0.0, int textLen=INT_MAX) const;
		// Paint a text fitted into a rectangle, with or without
		// formatting.
		// Arguments:
		//   x,y,w,h       - Upper-left corner and size of the
		//                   rectangle.
		//   text          - The character string, terminated by a
		//                   null-character or through the textLen
		//                   argument.
		//   maxCharHeight - The maximum character height. This includes
		//                   ascenders and descenders. The actual
		//                   character height may get smaller, so that
		//                   the text fits into the rectangle.
		//   color         - The color (alpha part is used for
		//                   blending).
		//   canvasColor   - Please read the general comments more
		//                   above.
		//   boxAlignment  - How to align the text as a whole within the
		//                   rectangle.
		//   textAlignment - How to align individual lines within the
		//                   text horizontally. The top and bottom flags
		//                   are ignored here.
		//   minWidthScale - Minimum factor for making the characters
		//                   wider (>1.0) or less wide (<1.0). The
		//                   implicit maximum is the maximum of 1.0 and
		//                   minWidthScale. The maximum is preferred,
		//                   but for fitting the text into the
		//                   rectangle, the factor may get smaller down
		//                   to minWidthScale.
		//   formatted     - Whether to interpret formatting characters
		//                   (new-line and tabulator).
		//   relLineSpace  - Vertical space between text lines, in units
		//                   of character heights.
		//   textLen       - Length of the character string if not
		//                   null-terminated.

	static double GetTextSize(const char * text, double charHeight,
	                          bool formatted=true, double relLineSpace=0.0,
	                          double * pHeight=NULL, int textLen=INT_MAX);
		// Calculate the width and height of a text.
		// Arguments:
		//   text         - The character string, terminated by a
		//                  null-character or through the textLen
		//                  argument.
		//   charHeight   - The character height. This includes
		//                  ascenders and descenders.
		//   formatted    - Whether to interpret formatting characters
		//                  (new-line and tabulator).
		//   relLineSpace - Vertical space between text lines, in units
		//                  of character heights.
		//   pHeight      - Pointer for returning the height of the
		//                  text, or NULL.
		//   textLen      - Length of the character string if not
		//                  null-terminated.
		// Returns: The width of the text.


	//----------------------------------------------------------------------

private:

	struct SharedPixelFormat {
		SharedPixelFormat * Next;
		int RefCount;
		int BytesPerPixel;
		emUInt32 RedRange,GreenRange,BlueRange;
		int RedShift,GreenShift,BlueShift;
		void * RedHash;   // Index bits: rrrrrrrraaaaaaaa or aaaaaaaarrrrrrrr
		void * GreenHash; // Index bits: ggggggggaaaaaaaa or aaaaaaaagggggggg
		void * BlueHash;  // Index bits: bbbbbbbbaaaaaaaa or aaaaaaaabbbbbbbb
	};

	void * Map;
	int BytesPerRow;
	SharedPixelFormat * PixelFormat;
	double ClipX1, ClipY1, ClipX2, ClipY2;
	double OriginX, OriginY, ScaleX, ScaleY;
	emRef<emFontCache> FontCache;

	static const double CharBoxTallness;

	static const unsigned ImageDownscaleQuality;
		// Quality of downscaling images. Higher value means more
		// quality but less performance: Before interpolation, the
		// source is downscaled by integer-nearest-pixel to about:
		// sizeOfSource <= ImageDownscaleQuality * sizeOfTarget

	static const double CircleQuality;
};

inline emPainter::~emPainter()
{
	if (PixelFormat) PixelFormat->RefCount--;
	// Do not free unused shared pixel formats here. So, it can be re-used
	// quickly on next construction.
}

inline double emPainter::GetClipX1() const
{
	return ClipX1;
}

inline double emPainter::GetClipY1() const
{
	return ClipY1;
}

inline double emPainter::GetClipX2() const
{
	return ClipX2;
}

inline double emPainter::GetClipY2() const
{
	return ClipY2;
}

inline void emPainter::SetClipping(double clipX1, double clipY1,
                                   double clipX2, double clipY2)
{
	ClipX1=clipX1;
	ClipY1=clipY1;
	ClipX2=clipX2;
	ClipY2=clipY2;
}

inline double emPainter::GetUserClipX1() const
{
	return (ClipX1-OriginX)/ScaleX;
}

inline double emPainter::GetUserClipY1() const
{
	return (ClipY1-OriginY)/ScaleY;
}

inline double emPainter::GetUserClipX2() const
{
	return (ClipX2-OriginX)/ScaleX;
}

inline double emPainter::GetUserClipY2() const
{
	return (ClipY2-OriginY)/ScaleY;
}

inline double emPainter::GetOriginX() const
{
	return OriginX;
}

inline double emPainter::GetOriginY() const
{
	return OriginY;
}

inline double emPainter::GetScaleX() const
{
	return ScaleX;
}

inline double emPainter::GetScaleY() const
{
	return ScaleY;
}

inline void emPainter::SetOrigin(double originX, double originY)
{
	OriginX=originX;
	OriginY=originY;
}

inline void emPainter::SetScaling(double scaleX, double scaleY)
{
	ScaleX=scaleX;
	ScaleY=scaleY;
}

inline void emPainter::SetTransformation(
	double originX, double originY, double scaleX, double scaleY
)
{
	OriginX=originX;
	OriginY=originY;
	ScaleX=scaleX;
	ScaleY=scaleY;
}

inline void emPainter::PaintShape(
	double x, double y, double w, double h, const emImage & img,
	int channel, emColor color, emColor canvasColor
) const
{
	PaintShape(x,y,w,h,img,0,0,img.GetWidth(),img.GetHeight(),
	           channel,color,canvasColor);
}

inline void emPainter::PaintImage(
	double x, double y, double w, double h, const emImage & img,
	int alpha, emColor canvasColor
) const
{
	PaintImage(x,y,w,h,img,0,0,img.GetWidth(),img.GetHeight(),alpha,
	           canvasColor);
}

inline void emPainter::PaintBorderShape(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, double srcL, double srcT, double srcR,
	double srcB, int channel, emColor color, emColor canvasColor,
	int whichSubRects
) const
{
	PaintBorderShape(
		x,y,w,h,l,t,r,b,img,0,0,img.GetWidth(),img.GetHeight(),
		srcL,srcT,srcR,srcB,channel,color,canvasColor,whichSubRects
	);
}

inline void emPainter::PaintBorderImage(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, double srcL, double srcT, double srcR,
	double srcB, int alpha, emColor canvasColor, int whichSubRects
) const
{
	PaintBorderImage(
		x,y,w,h,l,t,r,b,img,0,0,img.GetWidth(),img.GetHeight(),
		srcL,srcT,srcR,srcB,alpha,canvasColor,whichSubRects
	);
}


#endif
