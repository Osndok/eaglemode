//------------------------------------------------------------------------------
// emImage.h
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

#ifndef emImage_h
#define emImage_h

#ifndef emColor_h
#include <emCore/emColor.h>
#endif

#ifndef emATMatrix_h
#include <emCore/emATMatrix.h>
#endif

class emRootContext;
class emPainter;


//==============================================================================
//================================== emImage ===================================
//==============================================================================

class emImage {

public:

	// Class for an image with copy-on-write behavior. Such an image
	// consists of a three-dimensional array of bytes. The dimensions are X,
	// Y and color channel.
	//
	// An image can have one, two, three or four channels. Each channel has
	// one byte per pixel. The meaning of the channels depends on the number
	// of channels:
	//
	//   ChannelCount = 1: Grey image without alpha
	//     channel 0: grey components.
	//   ChannelCount = 2: Grey image with alpha
	//     channel 0: grey components.
	//     channel 1: alpha components.
	//   ChannelCount = 3: Color image without alpha
	//     channel 0: red components.
	//     channel 1: green components.
	//     channel 2: blue components.
	//   ChannelCount = 4: Color image with alpha
	//     channel 0: red components.
	//     channel 1: green components.
	//     channel 2: blue components.
	//     channel 3: alpha components.
	//
	// Like in most computer graphics, the origin of the image is in the
	// upper-left corner, the X-axis points to the right, and the Y-axis
	// points down. Pixels are areas and not points.

	emImage();
		// Construct an empty image. It has Width==0, Height==0 and
		// ChannelCount==1.

	emImage(const emImage & img);
		// Construct a copied image.

	emImage(int width, int height, int channelCount);
		// Construct with the given dimensions. The pixel map is not
		// initialized.
		// Arguments:
		//   width        - Horizontal extent in pixels.
		//   height       - Vertical extent in pixels.
		//   channelCount - Number of channels (bytes per pixel, 1-4).

	~emImage();
		// Destructor.

	emImage & operator = (const emImage & img);
		// Copy an image.

	bool operator == (const emImage & image) const;
	bool operator != (const emImage & image) const;
		// Compare images.

	void Setup(int width, int height, int channelCount);
		// Set the dimensions of this image. This method does not care
		// about preserving the contents.
		// Arguments:
		//   width        - Horizontal extent in pixels.
		//   height       - Vertical extent in pixels.
		//   channelCount - Number of channels (bytes per pixel, 1-4).

	void SetUserMap(int width, int height, int channelCount, emByte * map);
		// Prepare this image for being an interface to a user allocated
		// pixel map. For example, this feature could be used for
		// exchanging image data between system processes through shared
		// memory. You should know about the following semantics: The
		// copy-on-write behavior is disabled for such a "user map
		// image", and the user map feature is never copied. That means,
		// when copying a user map image to another image, a deep copy
		// is performed immediately and the target image will be a
		// normal image. Each operation which actually changes the
		// dimensions of the image, will give up the user map feature
		// and revert to the normal behavior. This means for example,
		// calling Setup preserves the feature if the dimensions are not
		// really changed. Call Empty or '=' if you want to revert to
		// the normal behavior in any case.
		// Arguments:
		//   width        - Horizontal extent of the user map in pixels.
		//   height       - Vertical extent of the user map in pixels.
		//   channelCount - Number of channels in the user map (bytes
		//                  per pixel, 1-4).
		//   map          - The user allocated pixel map. It's what you
		//                  will get through GetMap(). If channelCount
		//                  is 2 or 4, the map address must be aligned
		//                  accordingly.

	bool HasUserMap() const;
		// Ask whether this image interfaces a user allocated pixel map.

	void TryParseXpm(const char * const * xpm,
	                 int channelCount=-1) throw(emString);
		// Set this image by parsing an X Pixmap (XPM). The idea is to
		// include an XPM file in the C++ source, and to convert it to
		// an emImage at run-time using this method.
		// Arguments:
		//   xpm          - The X Pixmap data array.
		//   channelCount - Channel count of the returned image (1-4),
		//                  or -1 to select the best channel count from
		//                  the X Pixmap.
		// Throws: An error message on failure.

	void TryParseTga(const unsigned char * tgaData, int tgaSize,
	                 int channelCount=-1) throw(emString);
		// -------------------------------------------------------------
		// This method is deprecated and should not be used any longer.
		// -------------------------------------------------------------
		// Set this image by parsing a Targa image (TGA). The idea is
		// to convert a run-length encoded TGA file to a C source file,
		// and to include that source file in the C++ source, and to
		// convert it to an emImage at run-time using this function.
		// Arguments:
		//   tgaData      - Byte array with the contents of a TGA file.
		//   tgaSize      - Number of bytes in the array.
		//   channelCount - Channel count of the returned image (1-4),
		//                  or -1 to select the best channel count from
		//                  the TGA image.
		// Throws: An error message on failure.

	void Empty();
		// Like Setup(0,0,1).

	bool IsEmpty() const;
		// Ask whether this image is empty. It is empty if at least one
		// of width and height is zero.

	int GetWidth() const;
	int GetHeight() const;
	int GetChannelCount() const;
		// Get the dimensions of this image.

	const emByte * GetMap() const;
		// Get a pointer to the pixel map of this image. If ChannelCount
		// is 2 or 4, the map address is aligned accordingly. At least
		// because of the copy-on-write feature, the pointer is valid
		// only until calling any non-const method or operator on this
		// image, or giving this image as a non-const argument to any
		// call in the world. Hint: Even methods like GetConverted,
		// GetCropped and so on may make shallow copies, like the copy
		// operator and copy constructor do.
		// Index to the map is:
		//   (y*GetWidth()+x)*GetChannelCount()+c
		// With:
		//   x: 0 to GetWidth()-1
		//   y: 0 to GetHeight()-1
		//   c: 0 to GetChannelCount()-1

	emByte * GetWritableMap();
		// Like GetMap(), but for modifying the image. The rules for
		// validity of the pointer are the same as with GetMap(), but:
		// The pointer must not be used for modifying after doing
		// something which could have made a shallow copy of this image.

	emColor GetPixel(int x, int y) const;
	void SetPixel(int x, int y, emColor color);
		// Get or set a pixel.

	emByte GetPixelChannel(int x, int y, int channel) const;
	void SetPixelChannel(int x, int y, int channel, emByte value);
		// Get or set one channel of a pixel.

	emColor GetPixelInterpolated(double x, double y, double w,
	                             double h, emColor bgColor) const;
		// Get an average color from a rectangular area. Performs
		// bi-linear interpolation or area-sampling.
		// Arguments:
		//   x,y,w,h - Upper-left corner and size of the rectangle.
		//   bgColor - A background color. It is used where the
		//             rectangle lies outside the image.
		// Returns: The interpolated color.

	void Fill(emColor color);
	void Fill(int x, int y, int w, int h, emColor color);
		// Fill the whole image or a rectangular area with the given
		// color.

	void FillChannel(int channel, emByte value=0);
	void FillChannel(int x, int y, int w, int h, int channel,
	                 emByte value);
		// Like Fill, but for modifying just a single channel.

	void Copy(int x, int y, const emImage & img);
	void Copy(int x, int y, const emImage & img,
	          int srcX, int srcY, int w, int h);
		// Copy a rectangular area of pixels from a source image to this
		// image. Source and target may overlap.
		// Arguments:
		//   x,y           - Upper-left corner of target rectangle on
		//                   this image.
		//   img           - The source image.
		//   srcX,srcY,w,h - Upper-left corner and size of the rectangle
		//                   on the source image, which is to be copied.
		//                   Without these arguments, the whole source
		//                   image is taken.

	void CopyChannel(int x, int y, int channel, const emImage & img,
	                 int srcChannel);
	void CopyChannel(int x, int y, int channel, const emImage & img,
	                 int srcX, int srcY, int w, int h, int srcChannel);
		// Copy one channel of a rectangular area of pixels from a
		// source image to this image. Source and target may overlap.
		// Arguments:
		//   x,y           - Upper-left corner of target rectangle on
		//                   this image.
		//   channel       - Target channel on this image.
		//   img           - The source image.
		//   srcX,srcY,w,h - Upper-left corner and size of the rectangle
		//                   on the source image, which is to be copied.
		//                   Without these arguments, the whole source
		//                   image is taken.
		//   srcChannel    - Source channel on the given image.

	void CopyTransformed(int x, int y, int w, int h,
	                     const emATMatrix & atm, const emImage & img,
	                     bool interpolate=false, emColor bgColor=0);
		// Copy an image to this image while performing an affine
		// transformation. Source and target must not overlap.
		// Arguments:
		//   x,y,w,h     - A clipping rectangle for the operation on
		//                 this image. Exactly this area of pixels is
		//                 modified.
		//   atm         - A transformation matrix for transforming
		//                 source image coordinates to coordinates of
		//                 this image.
		//   img         - The source image.
		//   interpolate - Whether to perform bi-linear interpolation.
		//   bgColor     - A color to be used for areas outside the
		//                 source image.

	emImage GetTransformed(const emATMatrix & atm, bool interpolate=false,
	                       emColor bgColor=0, int channelCount=-1) const;
		// Get an affine transformed version of this image.
		// Arguments:
		//   atm          - A transformation matrix for transforming
		//                  coordinates of this image to coordinates of
		//                  the returned image. Here, any translation
		//                  will be ignored.
		//   interpolate  - Whether to perform bi-linear interpolation.
		//   bgColor      - A color to be used for areas outside the
		//                  source image.
		//   channelCount - Number of channels in the returned image
		//                  (1-4), or -1 for taking the channel count of
		//                  this image.
		// Returns: The resulting image.

	void CalcMinMaxRect(int * pX, int * pY, int * pW, int * pH,
	                    emColor bgColor) const;
		// Calculate the smallest rectangle which contains all pixels
		// which are not equal to the given background color.
		// Arguments:
		//   pX,pY,pW,pH - Pointers for returning the rectangle.
		//   bgColor     - The background color.

	void CalcChannelMinMaxRect(int * pX, int * pY, int * pW, int * pH,
	                           int channel, emByte bgValue) const;
		// Like CalcMinMaxRect, but for a single channel.
		// Arguments:
		//   pX,pY,pW,pH - Pointers for returning the rectangle.
		//   channel     - The channel to be inspected.
		//   bgValue     - The channel value of the background color.

	void CalcAlphaMinMaxRect(int * pX, int * pY, int * pW,
	                         int * pH) const;
		// Calculate the smallest rectangle which contains all pixels
		// which are not zero in the alpha channel.
		// Arguments:
		//   pX,pY,pW,pH - Pointers for returning the rectangle.

	emImage GetConverted(int channelCount) const;
		// Get a copy of this image, converted to the given number of
		// channels.

	emImage GetCropped(int x, int y, int w, int h,
	                   int channelCount=-1) const;
		// Get a sub-image of this image.
		// Arguments:
		//   x,y,w,h      - A rectangle on this image. It's the area to
		//                  be copied to the returned image, which will
		//                  have the size of that rectangle (after
		//                  clipping it by the image boundaries).
		//   channelCount - Number of channels of the returned image
		//                  (1-4), or -1 for taking the channel count of
		//                  this image.
		// Returns: The sub-image.

	emImage GetCroppedByAlpha(int channelCount=-1) const;
		// Like GetCropped with a rectangle returned by
		// GetAlphaMinMaxRect.

	bool PreparePainter(emPainter * painter, emRootContext & rootContext,
	                    double clipX1=0.0, double clipY1=0.0,
	                    double clipX2=3E9, double clipY2=3E9,
	                    double originX=0.0, double originY=0.0,
	                    double scaleX=1.0, double scaleY=1.0);
		// Prepare the given painter for painting to this image with
		// the given clipping and transformation. IMPORTANT: Currently,
		// the image must have 4 channels, otherwise painting would not
		// be possible. But the alpha channel cannot be painted and may
		// be damaged by the painter. In a near future version of
		// emPainter, the image may have to have 3 channels. And in a
		// far future version, any channel count may be acceptable. If
		// painting is not possible due to the channel count, the
		// painter is disabled and false is returned. If you want to
		// paint to an image in a portable way, please poll for a
		// suitable channel count via IsChannelCountPaintable and
		// prepare the image accordingly. After painting, you may
		// convert the image to the desired channel count, or you could
		// copy channels around. The rules for usability of the painter
		// are like with a pointer returned by GetWritableMap().

	static bool IsChannelCountPaintable(int channelCount);
		// Ask whether PreparePainter would be successful for an image
		// of the given channel count.

	unsigned int GetDataRefCount() const;
		// Get number of references to the data behind this image.

	void MakeNonShared();
		// This must be called before handing the image to another
		// thread.

private:

	void MakeWritable();
	void FreeData();

	struct SharedData {
		unsigned int RefCount;
		int Width;
		int Height;
		emByte ChannelCount;
		emByte IsUsersMap;
		emByte * Map;
		// From here on comes the non-user map.
	};

	SharedData * Data;

	static SharedData EmptyData;
};

#ifndef EM_NO_DATA_EXPORT
inline emImage::emImage()
{
	Data=&EmptyData;
}
#endif

inline emImage::emImage(const emImage & img)
{
	Data=img.Data;
	Data->RefCount++;
	if (Data->IsUsersMap) MakeWritable();
}

inline emImage::~emImage()
{
	if (!--Data->RefCount) FreeData();
}

inline bool emImage::operator != (const emImage & image) const
{
	return !(*this == image);
}

inline bool emImage::HasUserMap() const
{
	return Data->IsUsersMap!=0;
}

#ifndef EM_NO_DATA_EXPORT
inline void emImage::Empty()
{
	if (!--Data->RefCount) FreeData();
	Data=&EmptyData;
}
#endif

inline bool emImage::IsEmpty() const
{
	return Data->Width==0 || Data->Height==0;
}

inline int emImage::GetWidth() const
{
	return Data->Width;
}

inline int emImage::GetHeight() const
{
	return Data->Height;
}

inline int emImage::GetChannelCount() const
{
	return Data->ChannelCount;
}

inline const emByte * emImage::GetMap() const
{
	return Data->Map;
}

inline emByte * emImage::GetWritableMap()
{
	if (Data->RefCount>1) MakeWritable();
	return Data->Map;
}

inline void emImage::Fill(emColor color)
{
	Fill(0,0,Data->Width,Data->Height,color);
}

inline void emImage::FillChannel(int channel, emByte value)
{
	FillChannel(0,0,Data->Width,Data->Height,channel,value);
}

inline void emImage::Copy(int x, int y, const emImage & img)
{
	Copy(x,y,img,0,0,img.Data->Width,img.Data->Height);
}

inline void emImage::CopyChannel(
	int x, int y, int channel, const emImage & img, int srcChannel
)
{
	CopyChannel(
		x,y,channel,img,
		0,0,img.Data->Width,img.Data->Height,
		srcChannel
	);
}

inline emImage emImage::GetConverted(int channelCount) const
{
	return GetCropped(0,0,Data->Width,Data->Height,channelCount);
}

inline void emImage::MakeNonShared()
{
	MakeWritable();
}


#endif
