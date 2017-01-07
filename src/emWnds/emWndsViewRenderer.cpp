//------------------------------------------------------------------------------
// emWndsViewRenderer.cpp
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

#include <emWnds/emWndsViewRenderer.h>


emWndsViewRenderer::emWndsViewRenderer(emWndsScreen & screen)
	: emViewRenderer(screen.GetRootContext()),
	Screen(screen)
{
	CurrentHdc=NULL;
	CurrentViewX=0;
	CurrentViewY=0;
}


emWndsViewRenderer::~emWndsViewRenderer()
{
	int i;

	for (i=0; i<Buffers.GetCount(); i++) {
		DestroyBuffer(Buffers[i]);
	}
	Buffers.Clear();
}


void emWndsViewRenderer::RenderView(
	const emViewPort & viewPort,
	const emClipRects<int> & invalidRects,
	HDC hdc
)
{
	CurrentHdc=hdc;
	CurrentViewX=(int)viewPort.GetViewX();
	CurrentViewY=(int)viewPort.GetViewY();

	emViewRenderer::RenderView(viewPort, invalidRects);

	CurrentHdc=NULL;
	CurrentViewX=0;
	CurrentViewY=0;
}


void emWndsViewRenderer::PrepareBuffers(
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


emPainter emWndsViewRenderer::GetBufferPainter(
	int bufIndex, int x, int y, int w, int h
)
{
	return emPainter(Buffers[bufIndex]->Painter,0,0,w,h,-x,-y,1,1);
}


void emWndsViewRenderer::AsyncFlushBuffer(
	int bufIndex, int x, int y, int w, int h
)
{
	GdiMutex.Lock();

	StretchDIBits(
		CurrentHdc,
		x-CurrentViewX,y-CurrentViewY,
		w,h,
		0,0,
		w,h,
		Buffers[bufIndex]->Map,
		(BITMAPINFO*)&Buffers[bufIndex]->Info,
		0,
		SRCCOPY
	);

	GdiFlush();

	GdiMutex.Unlock();
}


emWndsViewRenderer::Buffer * emWndsViewRenderer::CreateBuffer(
	int width, int height
)
{
	Buffer * buf;

	buf=new Buffer;
	buf->Width=width;
	buf->Height=height;

	memset(&buf->Info,0,sizeof(BITMAPINFOHEADER));
	buf->Info.biSize=sizeof(BITMAPINFOHEADER);
	buf->Info.biWidth=width;
	buf->Info.biHeight=-height;
	buf->Info.biPlanes=1;
	buf->Info.biBitCount=32;
	buf->Info.biCompression=BI_RGB;

	buf->Map=new emUInt32[width*height];
	memset(buf->Map,0,width*height*4);

	buf->Painter=emPainter(
		Screen.GetRootContext(),
		buf->Map,
		width*4,
		4,
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0,
		0,
		width,
		height
	);

	return buf;
}


void emWndsViewRenderer::DestroyBuffer(Buffer * buf)
{
	if (buf->Map) {
		delete [] buf->Map;
	}

	delete buf;
}
