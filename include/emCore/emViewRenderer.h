//------------------------------------------------------------------------------
// emViewRenderer.h
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

#ifndef emViewRenderer_h
#define emViewRenderer_h

#ifndef emClipRects_h
#include <emCore/emClipRects.h>
#endif

#ifndef emRenderThreadPool_h
#include <emCore/emRenderThreadPool.h>
#endif

#ifndef emView_h
#include <emCore/emView.h>
#endif


//==============================================================================
//=============================== emViewRenderer ===============================
//==============================================================================

class emViewRenderer : public emUncopyable {

public:

	// Helper class for rendering views by multiple threads concurrently.
	// This uses emRenderThreadPool.

	emViewRenderer(emRootContext & rootContext);
	virtual ~emViewRenderer();

	void RenderView(
		const emViewPort & viewPort,
		const emClipRects<int> & invalidRects
	);
		// Render a view.
		// Arguments:
		//   viewPort     - The view port of the view.
		//   invalidRects - Which parts of the view are to be rendered, in
		//                  screen coordinates.

protected:

	virtual void PrepareBuffers(
		int bufCount, int maxWidth, int maxHeight
	) = 0;
		// Prepare image buffers for rendering. Called rarely.
		// Arguments:
		//   bufCount  - Number of buffers.
		//   maxWidth  - Maximum used width of the buffer, in pixels.
		//   maxHeight - Maximum used height of the buffer, in pixels.

	virtual emPainter GetBufferPainter(
		int bufIndex, int x, int y, int w, int h
	) = 0;
		// Get a painter for painting to a buffer.
		// Arguments:
		//   bufIndex  - Buffer index.
		//   x,y,w,h   - Rectangle on the screen to be updated.
		//               w and h are never larger than said with
		//               PrepareBuffers(...).

	virtual void AsyncFlushBuffer(
		int bufIndex, int x, int y, int w, int h
	) = 0;
		// Flush a buffer to the screen. This has to be thread-safe.
		// The Arguments:
		//   bufIndex  - Buffer index.
		//   x,y,w,h   - Rectangle on the screen to be updated (same
		//               as in call to GetBufferPainter(...) before).

private:

	struct TodoRect {
		int x,y,w,h;
	};

	static void ThreadFunc(void * data, int bufIndex);
	void ThreadRun(int bufIndex);

	emRef<emRenderThreadPool> ThreadPool;
	int BufCount;
	int BufWidth;
	int BufHeight;
	const emViewPort * CurrentViewPort;
	emThreadMiniMutex UserSpaceMutex;
	emArray<TodoRect> TodoRects;
	int TrIndex;
};


#endif
