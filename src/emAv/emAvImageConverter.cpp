//------------------------------------------------------------------------------
// emAvImageConverter.cpp
//
// Copyright (C) 2019-2020 Oliver Hamann.
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
#include <emCore/emCoreConfig.h>


emAvImageConverter::emAvImageConverter(emContext & context)
	: CoreConfig(emCoreConfig::Acquire(context.GetRootContext())),
	Format(0),
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
#	if EM_HAVE_X86_INTRINSICS
		CanCpuDoAvx2=emCanCpuDoAvx2();
#	endif
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
#			if EM_HAVE_X86_INTRINSICS
				if (CanCpuDoAvx2 && CoreConfig->AllowSIMD.Get()) {
					ConvertI420_AVX2(y1,y2);
					break;
				}
#			endif
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
	const emByte * sy, * syBeg, * su, * sv;
	emByte * t;
	int duv,cu,cv,cy,cr,cg,cb,cr1,cg1,cb1,c;

	while (y1<y2) {
		y2--;
		t=Image->GetWritableMap()+((y2+1)*Width-2)*3;
		syBeg=Plane+y2*BPL;
		sy=syBeg+Width-2;
		su=Plane2+(y2>>1)*BPL2+((Width-2)>>1);
		sv=Plane3+(y2>>1)*BPL2+((Width-2)>>1);
		duv=0;
		if (y2&1) {
			if (y2<Height-1) duv=BPL2;
		}
		else {
			if (y2>0) duv=-BPL2;
		}
		cu=su[0]*3+su[duv];
		cv=sv[0]*3+sv[duv];
		cr=409*cv+(128*4-16*298*4-409*512);
		cg=-100*cu-208*cv+(128*4-16*298*4+100*512+208*512);
		cb=516*cu+(128*4-16*298*4-516*512);
		do {
			cr1=cr;
			cg1=cg;
			cb1=cb;
			cu=su[0]*3+su[duv];
			cv=sv[0]*3+sv[duv];
			cr=409*cv+(128*4-16*298*4-409*512);
			cg=-100*cu-208*cv+(128*4-16*298*4+100*512+208*512);
			cb=516*cu+(128*4-16*298*4-516*512);

			cy=sy[1]*(298*8);
			c=(cr+cr1+cy)>>11;
			if ((unsigned)c>255) c=(-c)>>16;
			t[3]=(emByte)c;
			c=(cg+cg1+cy)>>11;
			if ((unsigned)c>255) c=(-c)>>16;
			t[4]=(emByte)c;
			c=(cb+cb1+cy)>>11;
			if ((unsigned)c>255) c=(-c)>>16;
			t[5]=(emByte)c;

			cy=sy[0]*(298*4);
			c=(cr+cy)>>10;
			if ((unsigned)c>255) c=(-c)>>16;
			t[0]=(emByte)c;
			c=(cg+cy)>>10;
			if ((unsigned)c>255) c=(-c)>>16;
			t[1]=(emByte)c;
			c=(cb+cy)>>10;
			if ((unsigned)c>255) c=(-c)>>16;
			t[2]=(emByte)c;

			su--;
			sv--;
			sy-=2;
			t-=6;
		} while (sy>=syBeg);
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
