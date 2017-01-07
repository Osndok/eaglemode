//------------------------------------------------------------------------------
// emWndsViewRenderer.h
//
// Copyright (C) 2016 Oliver Hamann.
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

#ifndef emWndsViewRenderer_h
#define emWndsViewRenderer_h

#ifndef emViewRenderer_h
#include <emCore/emViewRenderer.h>
#endif

#ifndef emWndsScreen_h
#include <emWnds/emWndsScreen.h>
#endif


class emWndsViewRenderer : public emViewRenderer {

public:

	emWndsViewRenderer(emWndsScreen & screen);
	virtual ~emWndsViewRenderer();

	void RenderView(
		const emViewPort & viewPort,
		const emClipRects<int> & invalidRects,
		HDC hdc
	);

protected:

	virtual void PrepareBuffers(
		int bufCount, int maxWidth, int maxHeight
	);

	virtual emPainter GetBufferPainter(
		int bufIndex, int x, int y, int w, int h
	);

	virtual void AsyncFlushBuffer(
		int bufIndex, int x, int y, int w, int h
	);

private:

	struct Buffer {
		int Width,Height;
		BITMAPINFOHEADER Info;
		emUInt32 * Map;
		emPainter Painter;
	};

	Buffer * CreateBuffer(int width, int height);
	void DestroyBuffer(Buffer * buf);

	emWndsScreen & Screen;

	emArray<Buffer*> Buffers;

	HDC CurrentHdc;
	int CurrentViewX,CurrentViewY;

	emThreadMutex GdiMutex;
};


#endif
