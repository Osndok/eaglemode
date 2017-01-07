//------------------------------------------------------------------------------
// emX11ViewRenderer.h
//
// Copyright (C) 2016-2017 Oliver Hamann.
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

#ifndef emX11ViewRenderer_h
#define emX11ViewRenderer_h

#ifndef emViewRenderer_h
#include <emCore/emViewRenderer.h>
#endif

#ifndef emX11Screen_h
#include <emX11/emX11Screen.h>
#endif


class emX11ViewRenderer : public emViewRenderer {

public:

	emX11ViewRenderer(emX11Screen & screen);
	virtual ~emX11ViewRenderer();

	void RenderView(
		const emViewPort & viewPort,
		const emClipRects<int> & invalidRects,
		::Window win,
		GC gc
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
		int       Width,Height;
		bool      UsingXShm;
		XImage *  Img;
		XShmSegmentInfo Seg;
		bool      SegAutoRemoved;
		emPainter Painter;
	};

	struct WPData {
		emX11ViewRenderer * inst;
		int idx;
	};

	Buffer * CreateBuffer(int width, int height);
	void DestroyBuffer(Buffer * buf);

	void WaitBuf(int index);

	static Bool WaitPredicate(Display * display, XEvent * event, XPointer arg);

	static int ErrorHandler(Display * display, XErrorEvent * event);

	emX11Screen & Screen;
	emThreadMiniMutex & XMutex;
	Display * Disp;

	int BytesPerPixel;
	bool HaveXShm;
	int ShmCompletionEventType;

	emArray<Buffer*> Buffers;

	emThreadMutex FlushMutex;

	::Window CurrentWin;
	GC CurrentGc;
	int CurrentViewX,CurrentViewY;

	static emThreadMiniMutex ErrorHandlerMutex;
	static bool ErrorHandlerCalled;
};


#endif
