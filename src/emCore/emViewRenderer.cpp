//------------------------------------------------------------------------------
// emViewRenderer.cpp
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

#include <emCore/emViewRenderer.h>


emViewRenderer::emViewRenderer(emRootContext & rootContext)
{
	ThreadPool=emRenderThreadPool::Acquire(rootContext);
	BufCount=0;

	// --- Buffer Configuration ---

	BufWidth=4096;
		// For best performance, this should not be smaller than the
		// largest expected screen width.

	BufHeight=32;
		// Optimum depends on CPU cache size, thread count, pixel size,
		// window width and the type and complexity of painting. The
		// constant value was determined by some tests and calculations,
		// as a reasonable average.

	// --- End of Buffer Configuration ---

	CurrentViewPort=NULL;
	TrIndex=0;
}


emViewRenderer::~emViewRenderer()
{
}


void emViewRenderer::RenderView(
	const emViewPort & viewPort,
	const emClipRects<int> & invalidRects
)
{
	const emClipRects<int>::Rect * r;
	const TodoRect * t;
	TodoRect tr;
	int rx1,ry1,rx2,ry2,x,y,w,h,threads;

	if (invalidRects.IsEmpty()) return;

	threads = ThreadPool->GetThreadCount();
	if (BufCount!=threads) {
		BufCount=threads;
		PrepareBuffers(BufCount,BufWidth,BufHeight);
	}

	CurrentViewPort=&viewPort;
	TodoRects.Clear();
	TrIndex=0;

	for (r=invalidRects.GetFirst(); r; r=r->GetNext()) {
		rx1=r->GetX1();
		ry1=r->GetY1();
		rx2=r->GetX2();
		ry2=r->GetY2();
		y=ry1;
		do {
			h=ry2-y;
			if (h>BufHeight) h=BufHeight;
			x=rx1;
			do {
				w=rx2-x;
				if (w>BufWidth) w=BufWidth;
				tr.x=x;
				tr.y=y;
				tr.w=w;
				tr.h=h;
				TodoRects.Add(tr);
				x+=w;
			} while (x<rx2);
			y+=h;
		} while (y<ry2);
	}

	if (BufCount>1) {
		ThreadPool->CallParallel(ThreadFunc,this,BufCount);
	}
	else {
		while (TrIndex<TodoRects.GetCount()) {
			t=&TodoRects[TrIndex];
			TrIndex++;
			{
				emPainter painter = GetBufferPainter(0,t->x,t->y,t->w,t->h);
				painter.SetUserSpaceMutex(NULL,NULL);
				CurrentViewPort->PaintView(painter,0);
			}
			AsyncFlushBuffer(0,t->x,t->y,t->w,t->h);
		}
	}

	CurrentViewPort=NULL;
	TodoRects.Clear();
	TrIndex=0;
}


void emViewRenderer::ThreadFunc(void * data, int bufIndex)
{
	((emViewRenderer*)data)->ThreadRun(bufIndex);
}


void emViewRenderer::ThreadRun(int bufIndex)
{
	bool usmLockedByThisThread;
	const TodoRect * t;

	UserSpaceMutex.Lock();
	while (TrIndex<TodoRects.GetCount()) {
		t=&TodoRects[TrIndex];
		TrIndex++;
		{
			emPainter painter = GetBufferPainter(bufIndex,t->x,t->y,t->w,t->h);
			usmLockedByThisThread=true;
			painter.SetUserSpaceMutex(&UserSpaceMutex,&usmLockedByThisThread);
			CurrentViewPort->PaintView(painter,0);
		}
		UserSpaceMutex.Unlock();
		AsyncFlushBuffer(bufIndex,t->x,t->y,t->w,t->h);
		UserSpaceMutex.Lock();
	}
	UserSpaceMutex.Unlock();
}
