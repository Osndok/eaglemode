//------------------------------------------------------------------------------
// emAvImageConverter.cpp
//
// Copyright (C) 2019 Oliver Hamann.
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

#include <emAv/emAvImageConverter.h>


emAvImageConverter::emAvImageConverter()
	: Format(0),
	Width(0),
	Height(0),
	BPL(0),
	BPL2(0),
	Plane(NULL),
	Plane2(NULL),
	Plane3(NULL),
	Image(NULL),
	RowsAtOnce(0),
	PosY(0)
{
}


emAvImageConverter::~emAvImageConverter()
{
}


void emAvImageConverter::SetSourceRGB(
	int width, int height, int bytesPerLine, const emByte * plane
)
{
	Format=0;
	Width=width;
	Height=height;
	BPL=bytesPerLine;
	Plane=plane;
}


void emAvImageConverter::SetSourceI420(
	int width, int height, int bytesPerLineY, int bytesPerLineUV,
	const emByte * planeY, const emByte * planeU, const emByte * planeV
)
{
	Format=1;
	Width=width&~1;
	Height=height&~1;
	BPL=bytesPerLineY;
	BPL2=bytesPerLineUV;
	Plane=planeY;
	Plane2=planeU;
	Plane3=planeV;
}


void emAvImageConverter::SetSourceYUY2(
	int width, int height, int bytesPerLine, const emByte * plane
)
{
	Format=2;
	Width=width&~1;
	Height=height;
	BPL=bytesPerLine;
	Plane=plane;
}


void emAvImageConverter::SetTarget(emImage * image)
{
	Image=image;
}


void emAvImageConverter::Convert(emRenderThreadPool * renderThreadPool)
{
	if (
		Image->GetWidth()!=Width|| Image->GetHeight()!=Height ||
		Image->GetChannelCount()!=3
	) {
		Image->Setup(Width,Height,3);
	}

	PosY=Height;

	if (Format==0 || Height<128) {
		RowsAtOnce=Height;
		ThreadRun();
		return;
	}

	RowsAtOnce=32;

	renderThreadPool->CallParallel(ThreadFunc,this);
}


void emAvImageConverter::ThreadFunc(void * data, int)
{
	((emAvImageConverter*)data)->ThreadRun();
}


void emAvImageConverter::ThreadRun()
{
	int y1,y2;

	Mutex.Lock();
	while (PosY>0) {
		y2=PosY;
		y1=emMax(y2-RowsAtOnce,0);
		PosY=y1;
		Mutex.Unlock();
		switch (Format) {
		case 0:
			ConvertRGB(y1,y2);
			break;
		case 1:
			ConvertI420(y1,y2);
			break;
		default:
			ConvertYUY2(y1,y2);
			break;
		}
		Mutex.Lock();
	}
	Mutex.Unlock();
}


void emAvImageConverter::ConvertRGB(int y1, int y2)
{
	const emByte * s;
	emByte * map, * t;

	map=Image->GetWritableMap();
	while (y1<y2) {
		y2--;
		s=Plane+y2*BPL;
		t=map+y2*Width*3;
		memcpy(t,s,Width*3);
	}
}


void emAvImageConverter::ConvertI420(int y1, int y2)
{
	const emByte * s1, * s2, * s3, * s4;
	emByte * map, * t1, * t2, * t3;
	int cy,cu,cv,cr,cg,cb,c;

	map=Image->GetWritableMap();
	y1/=2;
	y2/=2;
	while (y1<y2) {
		y2--;
		s1=Plane+y2*2*BPL;
		s2=s1+BPL;
		s3=Plane2+y2*BPL2;
		s4=Plane3+y2*BPL2;
		t1=map+y2*2*Width*3;
		t2=t1+Width*3;
		t3=t2+Width*3;
		do {
			cu=s3[0]-128;
			cv=s4[0]-128;
			cr=409*cv+(128-16*298);
			cg=-100*cu-208*cv+(128-16*298);
			cb=516*cu+(128-16*298);

			cy=s1[0]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[0]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[1]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[2]=(emByte)c;

			cy=s1[1]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[3]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[4]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t1[5]=(emByte)c;

			cy=s2[0]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[0]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[1]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[2]=(emByte)c;

			cy=s2[1]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[3]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[4]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t2[5]=(emByte)c;

			s1+=2;
			s2+=2;
			s3++;
			s4++;
			t1+=6;
			t2+=6;
		} while (t2<t3);
	}
}


void emAvImageConverter::ConvertYUY2(int y1, int y2)
{
	const emByte * s;
	emByte * map, * t, * te;
	int cy,cu,cv,cr,cg,cb,c;

	map=Image->GetWritableMap();
	while (y1<y2) {
		y2--;
		s=Plane+y2*BPL;
		t=map+y2*Width*3;
		te=t+Width*3;
		do {
			cu=s[1]-128;
			cv=s[3]-128;
			cr=409*cv+(128-16*298);
			cg=-100*cu-208*cv+(128-16*298);
			cb=516*cu+(128-16*298);

			cy=s[0]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[0]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[1]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[2]=(emByte)c;

			cy=s[2]*298;
			c=(cr+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[3]=(emByte)c;
			c=(cg+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[4]=(emByte)c;
			c=(cb+cy)>>8;
			if ((unsigned)c>255) c=(-c)>>16;
			t[5]=(emByte)c;

			s+=4;
			t+=6;
		} while (t<te);
	}
}
