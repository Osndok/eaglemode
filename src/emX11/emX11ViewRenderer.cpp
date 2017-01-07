//------------------------------------------------------------------------------
// emX11ViewRenderer.cpp
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

#include <emX11/emX11ViewRenderer.h>
#include <sys/shm.h>


emX11ViewRenderer::emX11ViewRenderer(emX11Screen & screen)
	: emViewRenderer(screen.GetRootContext()),
	Screen(screen),
	XMutex(screen.XMutex),
	Disp(screen.Disp)
{
	int major,minor;
	Bool xshmCanDoPixmaps;
	XErrorHandler originalHandler;

	if      (Screen.VisuDepth<= 8) BytesPerPixel=1;
	else if (Screen.VisuDepth<=16) BytesPerPixel=2;
	else                           BytesPerPixel=4;

	try {
		emX11_TryLoadLibXext();
	}
	catch (emException & exception) {
		emWarning("emX11ViewRenderer: %s",exception.GetText());
	}

	HaveXShm=false;
	ShmCompletionEventType=-1;
	XMutex.Lock();
	XSync(Disp,False);
	ErrorHandlerMutex.Lock();
	ErrorHandlerCalled=false;
	originalHandler=XSetErrorHandler(ErrorHandler);
	for (;;) {
		if (!emX11_IsLibXextLoaded()) break;
		if (!XShmQueryVersion(Disp,&major,&minor,&xshmCanDoPixmaps)) break;
		if (ErrorHandlerCalled || major<1 || (minor<1 && major<=1)) break;
		ShmCompletionEventType=XShmGetEventBase(Disp)+ShmCompletion;
		if (ErrorHandlerCalled) break;
		HaveXShm=true;
		break;
	}
	XSync(Disp,False);
	XSetErrorHandler(originalHandler);
	ErrorHandlerMutex.Unlock();
	XMutex.Unlock();

	CurrentWin=None;
	CurrentGc=None;
	CurrentViewX=0;
	CurrentViewY=0;

	if (!HaveXShm) {
		emWarning("emX11ViewRenderer: no XShm (=>slow)");
	}
}


emX11ViewRenderer::~emX11ViewRenderer()
{
	int i;

	for (i=0; i<Buffers.GetCount(); i++) {
		DestroyBuffer(Buffers[i]);
	}
	Buffers.Clear();
}


void emX11ViewRenderer::RenderView(
	const emViewPort & viewPort,
	const emClipRects<int> & invalidRects,
	::Window win,
	GC gc
)
{
	CurrentWin=win;
	CurrentGc=gc;
	CurrentViewX=(int)viewPort.GetViewX();
	CurrentViewY=(int)viewPort.GetViewY();

	emViewRenderer::RenderView(viewPort, invalidRects);

	CurrentWin=None;
	CurrentGc=None;
	CurrentViewX=0;
	CurrentViewY=0;
}


void emX11ViewRenderer::PrepareBuffers(
	int bufCount, int maxWidth, int maxHeight
)
{
	int i;

	for (i=0; i<Buffers.GetCount(); i++) {
		DestroyBuffer(Buffers[i]);
	}
	Buffers.SetCount(bufCount);
	for (i=0; i<bufCount; i++) {
		Buffers.GetWritable(i)=CreateBuffer(maxWidth,maxHeight);
	}
}


emPainter emX11ViewRenderer::GetBufferPainter(
	int bufIndex, int x, int y, int w, int h
)
{
	return emPainter(Buffers[bufIndex]->Painter,0,0,w,h,-x,-y,1,1);
}


void emX11ViewRenderer::AsyncFlushBuffer(
	int bufIndex, int x, int y, int w, int h
)
{
	FlushMutex.Lock();

	if (Buffers[bufIndex]->UsingXShm) {
		XMutex.Lock();
		XShmPutImage(
			Disp,
			CurrentWin,
			CurrentGc,
			Buffers[bufIndex]->Img,
			0,0,
			x-CurrentViewX,y-CurrentViewY,
			w,h,
			True
		);
		XMutex.Unlock();
		WaitBuf(bufIndex);
	}
	else {
		XMutex.Lock();
		XPutImage(
			Disp,
			CurrentWin,
			CurrentGc,
			Buffers[bufIndex]->Img,
			0,0,
			x-CurrentViewX,y-CurrentViewY,
			w,h
		);
		XMutex.Unlock();
	}

	FlushMutex.Unlock();
}


emX11ViewRenderer::Buffer * emX11ViewRenderer::CreateBuffer(
	int width, int height
)
{
	XErrorHandler originalHandler;
	Status status;
	Buffer * buf;

	buf=new Buffer;
	buf->Width=width;
	buf->Height=height;
	buf->UsingXShm=false;
	if (HaveXShm) {
		XMutex.Lock();
		XSync(Disp,False);
		ErrorHandlerMutex.Lock();
		ErrorHandlerCalled=false;
		originalHandler=XSetErrorHandler(ErrorHandler);
		for (;;) {
			buf->Img=XShmCreateImage(
				Disp,
				Screen.Visu,
				Screen.VisuDepth,
				ZPixmap,
				NULL,
				&buf->Seg,
				width,
				height
			);
			if (ErrorHandlerCalled || !buf->Img) break;
			if (
				buf->Img->bits_per_pixel!=BytesPerPixel<<3 ||
#				if EM_BYTE_ORDER==4321
					buf->Img->byte_order!=MSBFirst
#				elif EM_BYTE_ORDER==1234
					buf->Img->byte_order!=LSBFirst
#				else
#					error unexpected value for EM_BYTE_ORDER
#				endif
			) {
				XFree(buf->Img);
				break;
			}
			buf->Seg.shmid=shmget(
				IPC_PRIVATE,
				buf->Img->bytes_per_line*buf->Img->height,
				IPC_CREAT|0777
			);
			if (buf->Seg.shmid==-1) {
				XFree(buf->Img);
				break;
			}
			buf->Seg.shmaddr=(char*)shmat(buf->Seg.shmid,0,0);
			if (buf->Seg.shmaddr==(char*)-1) {
				shmctl(buf->Seg.shmid,IPC_RMID,0);
				XFree(buf->Img);
				break;
			}
			buf->Img->data=buf->Seg.shmaddr;
			buf->Seg.readOnly=True;
			status=XShmAttach(Disp,&buf->Seg);
			XSync(Disp,False);
			if (!status || ErrorHandlerCalled) {
				shmdt(buf->Seg.shmaddr);
				shmctl(buf->Seg.shmid,IPC_RMID,0);
				XFree(buf->Img);
				break;
			}
#			if defined(__linux__)
				shmctl(buf->Seg.shmid,IPC_RMID,0);
				buf->SegAutoRemoved=true;
#			else
				buf->SegAutoRemoved=false;
#			endif
			buf->UsingXShm=true;
			break;
		}
		XSync(Disp,False);
		XSetErrorHandler(originalHandler);
		ErrorHandlerMutex.Unlock();
		XMutex.Unlock();
	}

	if (!buf->UsingXShm) {
		if (HaveXShm) {
			emWarning("emX11ViewRenderer: XShm failed");
			HaveXShm=false;
		}
		XMutex.Lock();
		buf->Img=XCreateImage(
			Disp,
			Screen.Visu,
			Screen.VisuDepth,
			ZPixmap,
			0,
			(char*)malloc(height*width*BytesPerPixel),
			width,
			height,
			BytesPerPixel<<3,
			width*BytesPerPixel
		);
		XMutex.Unlock();
		if (
			BytesPerPixel==4 &&
			buf->Img->bits_per_pixel==24 &&
			buf->Img->bitmap_unit==32 &&
			buf->Img->bytes_per_line>=4*buf->Img->width
		) {
			//??? hack / workaround: Either there are buggy X-Servers,
			//??? or it is true that it is allowed to have XImages with
			//??? three bytes per pixel - no matter, force four bytes
			//??? per pixel here. Unfortunately, this does not work for
			//??? XShmCreateImage.
			buf->Img->bits_per_pixel=32;
		}
#		if EM_BYTE_ORDER==4321
			buf->Img->byte_order=MSBFirst;
#		elif EM_BYTE_ORDER==1234
			buf->Img->byte_order=LSBFirst;
#		else
#			error unexpected value for EM_BYTE_ORDER
#		endif
	}

	memset(
		buf->Img->data,
		0,
		buf->Img->bytes_per_line*buf->Img->height
	);

	buf->Painter=emPainter(
		Screen.GetRootContext(),
		buf->Img->data+buf->Img->xoffset*BytesPerPixel,
		buf->Img->bytes_per_line,
		BytesPerPixel,
		buf->Img->red_mask,
		buf->Img->green_mask,
		buf->Img->blue_mask,
		0,
		0,
		buf->Img->width,
		buf->Img->height
	);

	return buf;
}


void emX11ViewRenderer::DestroyBuffer(Buffer * buf)
{
	XMutex.Lock();

	XSync(Disp,False);

	if (buf->Img) {
		if (buf->UsingXShm) {
			XShmDetach(Disp,&buf->Seg);
			shmdt(buf->Seg.shmaddr);
			if (!buf->SegAutoRemoved) shmctl(buf->Seg.shmid,IPC_RMID,0);
		}
		else {
			free(buf->Img->data);
		}
		XFree(buf->Img);
	}

	XMutex.Unlock();

	delete buf;
}


void emX11ViewRenderer::WaitBuf(int index)
{
	union {
		XEvent x;
		XShmCompletionEvent xsc;
	} event;
	WPData data;

	data.inst=this;
	data.idx=index;

	for (;;) {
		XMutex.Lock();
		XIfEvent(Disp,&event.x,WaitPredicate,(XPointer)&data);
		XMutex.Unlock();
		if (event.x.type==ShmCompletionEventType) {
			if (event.xsc.shmseg==Buffers[index]->Seg.shmseg) {
				break;
			}
		}
	}
}


Bool emX11ViewRenderer::WaitPredicate(
	Display * display, XEvent * event, XPointer arg
)
{
	const WPData * d;
	XShmCompletionEvent * e;

	d=(const WPData *)arg;
	if (event->type!=d->inst->ShmCompletionEventType) return false;
	e=(XShmCompletionEvent*)event;
	if (e->shmseg==d->inst->Buffers[d->idx]->Seg.shmseg) return true;
	return false;
}


int emX11ViewRenderer::ErrorHandler(Display * display, XErrorEvent * event)
{
	ErrorHandlerCalled=true;
	return 0;
}


emThreadMiniMutex emX11ViewRenderer::ErrorHandlerMutex;
bool emX11ViewRenderer::ErrorHandlerCalled=false;
