//------------------------------------------------------------------------------
// emPainter.cpp
//
// Copyright (C) 2001,2003-2011,2016-2020 Oliver Hamann.
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

#include <emCore/emPainter.h>
#include "emPainter_ScTl.h"
#include <emCore/emCoreConfig.h>
#include <emCore/emFontCache.h>


emPainter::emPainter()
{
	Map=NULL;
	BytesPerRow=0;
	PixelFormat=NULL;
	ClipX1=0;
	ClipY1=0;
	ClipX2=0;
	ClipY2=0;
	OriginX=0;
	OriginY=0;
	ScaleX=0;
	ScaleY=0;
	UserSpaceMutex=NULL;
	USMLockedByThisThread=NULL;
}


emPainter::emPainter(const emPainter & painter)
{
	Map=painter.Map;
	BytesPerRow=painter.BytesPerRow;
	PixelFormat=painter.PixelFormat;
	if (PixelFormat) PixelFormat->RefCount++;
	ClipX1=painter.ClipX1;
	ClipY1=painter.ClipY1;
	ClipX2=painter.ClipX2;
	ClipY2=painter.ClipY2;
	OriginX=painter.OriginX;
	OriginY=painter.OriginY;
	ScaleX=painter.ScaleX;
	ScaleY=painter.ScaleY;
	UserSpaceMutex=painter.UserSpaceMutex;
	USMLockedByThisThread=painter.USMLockedByThisThread;
	Model=painter.Model;
}


emPainter::emPainter(
	const emPainter & painter, double clipX1, double clipY1, double clipX2,
	double clipY2
)
{
	Map=painter.Map;
	BytesPerRow=painter.BytesPerRow;
	PixelFormat=painter.PixelFormat;
	if (PixelFormat) PixelFormat->RefCount++;
	ClipX1= clipX1>painter.ClipX1 ? clipX1 : painter.ClipX1;
	ClipY1= clipY1>painter.ClipY1 ? clipY1 : painter.ClipY1;
	ClipX2= clipX2<painter.ClipX2 ? clipX2 : painter.ClipX2;
	ClipY2= clipY2<painter.ClipY2 ? clipY2 : painter.ClipY2;
	OriginX=painter.OriginX;
	OriginY=painter.OriginY;
	ScaleX=painter.ScaleX;
	ScaleY=painter.ScaleY;
	UserSpaceMutex=painter.UserSpaceMutex;
	USMLockedByThisThread=painter.USMLockedByThisThread;
	Model=painter.Model;
}


emPainter::emPainter(
	const emPainter & painter, double clipX1, double clipY1, double clipX2,
	double clipY2, double originX, double originY, double scaleX,
	double scaleY
)
{
	Map=painter.Map;
	BytesPerRow=painter.BytesPerRow;
	PixelFormat=painter.PixelFormat;
	if (PixelFormat) PixelFormat->RefCount++;
	ClipX1= clipX1>painter.ClipX1 ? clipX1 : painter.ClipX1;
	ClipY1= clipY1>painter.ClipY1 ? clipY1 : painter.ClipY1;
	ClipX2= clipX2<painter.ClipX2 ? clipX2 : painter.ClipX2;
	ClipY2= clipY2<painter.ClipY2 ? clipY2 : painter.ClipY2;
	OriginX=originX;
	OriginY=originY;
	ScaleX=scaleX;
	ScaleY=scaleY;
	UserSpaceMutex=painter.UserSpaceMutex;
	USMLockedByThisThread=painter.USMLockedByThisThread;
	Model=painter.Model;
}


emPainter::emPainter(
	emRootContext & rootContext,  void * map, int bytesPerRow,
	int bytesPerPixel, emUInt32 redMask, emUInt32 greenMask,
	emUInt32 blueMask, double clipX1, double clipY1, double clipX2,
	double clipY2, double originX, double originY, double scaleX,
	double scaleY, emThreadMiniMutex * userSpaceMutex,
	bool * usmLockedByThisThread
)
{
	emUInt32 redRange,greenRange,blueRange;
	int i,redShift,greenShift,blueShift,shift;
	SharedPixelFormat * pf;
	void * hash;
	emUInt32 c1,c2,c3,a1,a2,range;

	if (bytesPerPixel!=1 && bytesPerPixel!=2 && bytesPerPixel!=4) {
		emFatalError("emPainter: Illegal pixel format.");
	}

	if (
		clipX1<-32767.0 || clipX2>32767.0 || clipX2-clipX1>32767.0 ||
		clipY1<-32767.0 || clipY2>32767.0 || clipY2-clipY1>32767.0
	) {
		emFatalError("emPainter: Clip rect out of range (output image too large).");
	}

	Map=map;
	BytesPerRow=bytesPerRow;
	PixelFormat=NULL;
	ClipX1=clipX1;
	ClipY1=clipY1;
	ClipX2=clipX2;
	ClipY2=clipY2;
	OriginX=originX;
	OriginY=originY;
	ScaleX=scaleX;
	ScaleY=scaleY;
	UserSpaceMutex=userSpaceMutex;
	USMLockedByThisThread=usmLockedByThisThread;
	Model=SharedModel::Acquire(rootContext);

	redRange=redMask;
	redShift=0;
	if (redRange) while (!(redRange&1)) { redRange>>=1; redShift++; }
	greenRange=greenMask;
	greenShift=0;
	if (greenRange) while (!(greenRange&1)) { greenRange>>=1; greenShift++; }
	blueRange=blueMask;
	blueShift=0;
	if (blueRange) while (!(blueRange&1)) { blueRange>>=1; blueShift++; }

	// Search for a matching pixel format.
	for (pf=Model->PixelFormatList; pf!=NULL; pf=pf->Next) {
		if (
			pf->BytesPerPixel==bytesPerPixel &&
			pf->RedRange==redRange &&
			pf->GreenRange==greenRange &&
			pf->BlueRange==blueRange &&
			pf->RedShift==redShift &&
			pf->GreenShift==greenShift &&
			pf->BlueShift==blueShift
		) break;
	}
	if (!pf) {
		// Not found. Remove unused pixel formats here.
		Model->RemoveUnusedPixelFormats();
		// Create a new pixel format.
		pf=(SharedPixelFormat*)malloc(sizeof(SharedPixelFormat));
		pf->Next=Model->PixelFormatList;
		Model->PixelFormatList=pf;
		pf->RefCount=0;
		pf->BytesPerPixel=bytesPerPixel;
		pf->RedRange=redRange;
		pf->GreenRange=greenRange;
		pf->BlueRange=blueRange;
		pf->RedShift=redShift;
		pf->GreenShift=greenShift;
		pf->BlueShift=blueShift;
		pf->RedHash=malloc(256*256*bytesPerPixel);
		pf->GreenHash=malloc(256*256*bytesPerPixel);
		pf->BlueHash=malloc(256*256*bytesPerPixel);
		for (i=0; i<3; i++) {
			if (i==0) {
				hash=pf->RedHash;
				range=pf->RedRange;
				shift=pf->RedShift;
			}
			else if (i==1) {
				hash=pf->GreenHash;
				range=pf->GreenRange;
				shift=pf->GreenShift;
			}
			else {
				hash=pf->BlueHash;
				range=pf->BlueRange;
				shift=pf->BlueShift;
			}
			for (a1=0; a1<128; a1++) {
				c1=(a1*range+127)/255;
				for (a2=0; a2<128; a2++) {
					c2=(a2*range+127)/255;
					c3=(a1*a2*range+32512)/65025;
					if (bytesPerPixel==4) {
						((emUInt32*)hash)[(a1<<8)+a2]=c3<<shift;
						((emUInt32*)hash)[(a1<<8)+(255-a2)]=(c1-c3)<<shift;
						((emUInt32*)hash)[((255-a1)<<8)+a2]=(c2-c3)<<shift;
						((emUInt32*)hash)[((255-a1)<<8)+(255-a2)]=(range+c3-c1-c2)<<shift;
					}
					else if (bytesPerPixel==2) {
						((emUInt16*)hash)[(a1<<8)+a2]=(emUInt16)(c3<<shift);
						((emUInt16*)hash)[(a1<<8)+(255-a2)]=(emUInt16)((c1-c3)<<shift);
						((emUInt16*)hash)[((255-a1)<<8)+a2]=(emUInt16)((c2-c3)<<shift);
						((emUInt16*)hash)[((255-a1)<<8)+(255-a2)]=(emUInt16)((range+c3-c1-c2)<<shift);
					}
					else {
						((emByte*)hash)[(a1<<8)+a2]=(emByte)(c3<<shift);
						((emByte*)hash)[(a1<<8)+(255-a2)]=(emByte)((c1-c3)<<shift);
						((emByte*)hash)[((255-a1)<<8)+a2]=(emByte)((c2-c3)<<shift);
						((emByte*)hash)[((255-a1)<<8)+(255-a2)]=(emByte)((range+c3-c1-c2)<<shift);
					}
				}
			}
		}
		pf->OPFIndex=OPFI_NONE;
		if (bytesPerPixel==4 && redRange==255 && greenRange==255 && blueRange==255) {
			if (greenShift==8) {
				if (redShift==0 && blueShift==16) pf->OPFIndex=OPFI_8888_0BGR;
				else if (redShift==16 && blueShift==0) pf->OPFIndex=OPFI_8888_0RGB;
			}
			else if (greenShift==16) {
				if (redShift==8 && blueShift==24) pf->OPFIndex=OPFI_8888_BGR0;
				else if (redShift==24 && blueShift==8) pf->OPFIndex=OPFI_8888_RGB0;
			}
		}
	}
	// Use the found or created pixel format.
	pf->RefCount++;
	PixelFormat=pf;
}


emPainter & emPainter::operator = (const emPainter & painter)
{
	if (PixelFormat) PixelFormat->RefCount--;
	Map=painter.Map;
	BytesPerRow=painter.BytesPerRow;
	PixelFormat=painter.PixelFormat;
	if (PixelFormat) PixelFormat->RefCount++;
	ClipX1=painter.ClipX1;
	ClipY1=painter.ClipY1;
	ClipX2=painter.ClipX2;
	ClipY2=painter.ClipY2;
	OriginX=painter.OriginX;
	OriginY=painter.OriginY;
	ScaleX=painter.ScaleX;
	ScaleY=painter.ScaleY;
	UserSpaceMutex=painter.UserSpaceMutex;
	USMLockedByThisThread=painter.USMLockedByThisThread;
	Model=painter.Model;
	return *this;
}


double emPainter::RoundX(double x) const
{
	return (floor(x*ScaleX+OriginX+0.5)-OriginX)/ScaleX;
}


double emPainter::RoundY(double y) const
{
	return (floor(y*ScaleY+OriginY+0.5)-OriginY)/ScaleY;
}


double emPainter::RoundDownX(double x) const
{
	return (floor(x*ScaleX+OriginX)-OriginX)/ScaleX;
}


double emPainter::RoundDownY(double y) const
{
	return (floor(y*ScaleY+OriginY)-OriginY)/ScaleY;
}


double emPainter::RoundUpX(double x) const
{
	return (ceil(x*ScaleX+OriginX)-OriginX)/ScaleX;
}


double emPainter::RoundUpY(double y) const
{
	return (ceil(y*ScaleY+OriginY)-OriginY)/ScaleY;
}


void emPainter::SetUserSpaceMutex(
		emThreadMiniMutex * userSpaceMutex,
		bool * usmLockedByThisThread
)
{
	UserSpaceMutex=userSpaceMutex;
	USMLockedByThisThread=usmLockedByThisThread;
}


void emPainter::Clear(const emTexture & texture, emColor canvasColor) const
{
	PaintRect(
		(ClipX1-OriginX)/ScaleX,
		(ClipY1-OriginY)/ScaleY,
		(ClipX2-ClipX1)/ScaleX,
		(ClipY2-ClipY1)/ScaleY,
		texture,
		canvasColor
	);
}


void emPainter::PaintRect(
	double x, double y, double w, double h, const emTexture & texture,
	emColor canvasColor
) const
{
	double x2,y2;
	int ix,ixe,iw,iy,iy2,ax1,ay1,ax2,ay2;
	bool wasInUserSpace;

	x=x*ScaleX+OriginX;
	x2=x+w*ScaleX;
	if (x<ClipX1) x=ClipX1;
	if (x2>ClipX2) x2=ClipX2;
	if (x>=x2) return;
	y=y*ScaleY+OriginY;
	y2=y+h*ScaleY;
	if (y<ClipY1) y=ClipY1;
	if (y2>ClipY2) y2=ClipY2;
	if (y>=y2) return;

	wasInUserSpace=LeaveUserSpace();

	ScanlineTool sct(*this);
	if (!sct.Init(texture,canvasColor)) {
		if (wasInUserSpace) EnterUserSpace();
		return;
	}

	ix=(int)(x*0x1000);
	ixe=((int)(x2*0x1000))+0xfff;
	ax1=0x1000-(ix&0xfff);
	ax2=(ixe&0xfff)+1;
	ix>>=12;
	ixe>>=12;
	iw=ixe-ix;
	if (iw<=1) {
		if (iw<=0) {
			if (wasInUserSpace) EnterUserSpace();
			return;
		}
		ax1+=ax2-0x1000;
	}

	iy=(int)(y*0x1000);
	iy2=(int)(y2*0x1000);
	ay1=0x1000-(iy&0xfff);
	ay2=iy2&0xfff;
	iy>>=12;
	iy2>>=12;
	if (iy>=iy2) {
		ay1+=ay2-0x1000;
		ay2=0;
		if (ay1<=0) {
			if (wasInUserSpace) EnterUserSpace();
			return;
		}
	}

	if (ay1<0x1000) {
		sct.PaintScanline(
			sct,ix,iy,iw,(ax1*ay1+0x7ff)>>12,ay1,(ax2*ay1+0x7ff)>>12
		);
		iy++;
	}
	while (iy<iy2) {
		sct.PaintScanline(sct,ix,iy,iw,ax1,0x1000,ax2);
		iy++;
	}
	if (ay2>0) {
		sct.PaintScanline(
			sct,ix,iy,iw,(ax1*ay2+0x7ff)>>12,ay2,(ax2*ay2+0x7ff)>>12
		);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintPolygon(
	const double xy[], int n, const emTexture & texture, emColor canvasColor
) const
{
	struct ScanEntry {
		double A0,A1,A2;
		ScanEntry * Next;
		int X;
	};
	struct SEChunk {
		SEChunk * Next;
		ScanEntry ScanEntries[2048];
	};
	static ScanEntry seTerminator={ 0.0, 0.0, 0.0, NULL, INT_MAX };
#if defined(_WIN32)
	ScanEntry * autoScanlines[64];
	ScanEntry autoScanEntries[128];
#else
	ScanEntry * autoScanlines[512];
	ScanEntry autoScanEntries[1024];
#endif
	ScanEntry * * slmem, * * scanlines, * * ppse;
	ScanEntry * freeScanEntries, * freeScanEntriesEnd, * pse;
	SEChunk * seChunks, * psec;
	const double * pxy;
	double minX,maxX,minY,maxY,x0,y0,x1,y1,x2,y2,dx,dy,ax,a0,a1,a2,va,t;
	double ex1[2],ey1[2],ex2[2],ey2[2];
	int i,alpha,alpha2,alpha3,ta,sly1,sly2,sx,sy,sx0,sy2;
	bool wasInUserSpace;

	if (n<3) return;

	minX=maxX=xy[0];
	minY=maxY=xy[1];
	for (pxy=xy+n*2-2; pxy>=xy; pxy-=2) {
		if      (maxX<pxy[0]) maxX=pxy[0];
		else if (minX>pxy[0]) minX=pxy[0];
		if      (maxY<pxy[1]) maxY=pxy[1];
		else if (minY>pxy[1]) minY=pxy[1];
	}
	minY=minY*ScaleY+OriginY;
	if (minY<ClipY1) minY=ClipY1;
	maxY=maxY*ScaleY+OriginY;
	if (maxY>ClipY2) maxY=ClipY2;
	if (minY>=maxY) return;
	minX=minX*ScaleX+OriginX;
	if (minX<ClipX1) minX=ClipX1;
	maxX=maxX*ScaleX+OriginX;
	if (maxX>ClipX2-0.0001) maxX=ClipX2-0.0001;
	if (minX>=maxX) return;

	wasInUserSpace=LeaveUserSpace();

	ScanlineTool sct(*this);
	if (!sct.Init(texture,canvasColor)) {
		if (wasInUserSpace) EnterUserSpace();
		return;
	}

	sly1=(int)minY;
	sly2=(int)ceil(maxY);

	freeScanEntries=autoScanEntries;
	freeScanEntriesEnd=freeScanEntries+sizeof(autoScanEntries)/sizeof(ScanEntry);
	seChunks=NULL;
	i=sly2-sly1+2;
	if (i<=(int)(sizeof(autoScanlines)/sizeof(ScanEntry*))) {
		slmem=autoScanlines;
	}
	else {
		slmem=(ScanEntry**)malloc(i*sizeof(ScanEntry*));
	}
	do {
		i--;
		slmem[i]=&seTerminator;
	} while (i>0);
	scanlines=slmem-sly1+1;

#	define PP_ADD_SCAN_ENTRY(x,y,a0,a1,a2) \
		ppse=&scanlines[y]; \
		pse=*ppse; \
		if (pse->X<x) { \
			do { \
				ppse=&pse->Next; \
				pse=*ppse; \
			} while (pse->X<x); \
		} \
		if (pse->X==x) { \
			pse->A0+=a0; \
			pse->A1+=a1; \
			pse->A2+=a2; \
		} \
		else { \
			if (freeScanEntries>=freeScanEntriesEnd) { \
				psec=(SEChunk*)malloc(sizeof(SEChunk)); \
				psec->Next=seChunks; \
				seChunks=psec; \
				freeScanEntries=psec->ScanEntries; \
				freeScanEntriesEnd=freeScanEntries+sizeof(psec->ScanEntries)/sizeof(ScanEntry); \
			} \
			pse=freeScanEntries; \
			freeScanEntries++; \
			pse->Next=*ppse; \
			*ppse=pse; \
			pse->A0=a0; \
			pse->A1=a1; \
			pse->A2=a2; \
			pse->X=x; \
		}

	x0=xy[0]*ScaleX+OriginX;
	y0=xy[1]*ScaleY+OriginY;
	for (pxy=xy+n*2-2; pxy>=xy; pxy-=2) {
		y1=y0;
		y0=pxy[1]*ScaleY+OriginY;
		if (y1>y0) {
			y2=y1;
			y1=y0;
			x2=x0;
			x1=x0=pxy[0]*ScaleX+OriginX;
			va=0x1000;
		}
		else {
			y2=y0;
			x1=x0;
			x2=x0=pxy[0]*ScaleX+OriginX;
			va=-0x1000;
		}
		if (y1>=maxY || y2<=minY) continue;
		if (y1<minY) {
			if (y2-y1>=0.0001) x1+=(minY-y1)*(x2-x1)/(y2-y1);
			y1=minY;
		}
		if (y2>maxY) {
			if (y2-y1>=0.0001) x2+=(maxY-y2)*(x2-x1)/(y2-y1);
			y2=maxY;
		}
		i=0;
		if (x1<x2) {
			if (x1<minX) {
				if (x2>minX && x2-x1>=0.0001) {
					ey1[0]=y1;
					y1+=(minX-x1)*(y2-y1)/(x2-x1);
					ey2[0]=y1;
					ex1[0]=ex2[0]=x1=minX;
					i=1;
				}
				else {
					x1=x2=minX;
				}
			}
			if (x2>maxX) {
				if (x1<maxX && x2-x1>=0.0001) {
					ey2[i]=y2;
					y2+=(maxX-x2)*(y2-y1)/(x2-x1);
					ey1[i]=y2;
					ex1[i]=ex2[i]=x2=maxX;
					i++;
				}
				else {
					x1=x2=maxX;
				}
			}
		}
		else {
			if (x1>maxX) {
				if (x2<maxX && x2-x1<=-0.0001) {
					ey1[0]=y1;
					y1+=(maxX-x1)*(y2-y1)/(x2-x1);
					ey2[0]=y1;
					ex1[0]=ex2[0]=x1=maxX;
					i=1;
				}
				else {
					x1=x2=maxX;
				}
			}
			if (x2<minX) {
				if (x1>minX && x2-x1<=-0.0001) {
					ey2[i]=y2;
					y2+=(minX-x2)*(y2-y1)/(x2-x1);
					ey1[i]=y2;
					ex1[i]=ex2[i]=x2=minX;
					i++;
				}
				else {
					x1=x2=minX;
				}
			}
		}
		for (;;) {
			dy=y2-y1;
			if (dy>=0.0001) {
				sy=(int)y1;
				sy2=((int)ceil(y2))-1;
				ax=floor(x1);
				sx=(int)ax;
				t=ax+1.0-x1;
				dx=x2-x1;
				if (dx>=0.0001 || dx<=-0.0001) {
					a2=va*dy/dx;
					a0=t*t*0.5*a2;
					a1=(t+0.5)*a2;
					dx/=dy;
					x1+=(sy+1-y1)*dx;
					for (;;) {
						if (sy>=sy2) {
							if (sy>sy2) break;
							x1=x2;
						}
						PP_ADD_SCAN_ENTRY(sx,sy,a0,a1,a2)
						ax=floor(x1);
						sx=(int)ax;
						t=ax+1.0-x1;
						a0=t*t*0.5*a2;
						a1=(t+0.5)*a2;
						PP_ADD_SCAN_ENTRY(sx,sy,(-a0),(-a1),(-a2))
						x1+=dx;
						sy++;
					}
				}
				else {
					a1=va*(sy+1-y1);
					for (;;) {
						if (sy>=sy2) {
							if (sy>sy2) break;
							a1-=va*(sy2+1-y2);
						}
						a0=t*a1;
						PP_ADD_SCAN_ENTRY(sx,sy,a0,a1,0.0)
						a1=va;
						sy++;
					}
				}
			}
			if (!i) break;
			i--;
			x1=ex1[i];
			y1=ey1[i];
			x2=ex2[i];
			y2=ey2[i];
		}
	}

	sy=sly1;
	do {
		pse=scanlines[sy];
		if (pse!=&seTerminator) {
			a1=0;
			a2=0;
			sx=pse->X;
			do {
				a0=a1;
				a1+=a2;
				if (pse->X==sx) {
					a0+=pse->A0;
					a1+=pse->A1;
					a2+=pse->A2;
					pse=pse->Next;
				}
				sx0=sx;
				sx++;
				alpha=(int)(a0>=0 ? 0.5+a0 : 0.5-a0);
				if (!alpha) {
					if (pse->X>sx && pse!=&seTerminator) {
						t=a1+a2*(pse->X-1-sx);
						ta=(int)(t>=0 ? 0.5+t : 0.5-t);
						if (alpha==ta) {
							a1=t+a2;
							sx=pse->X;
						}
					}
					continue;
				}
				if (pse==&seTerminator) {
					sct.PaintScanline(sct,sx0,sy,1,alpha,0,0);
					break;
				}
				a0=a1;
				a1+=a2;
				if (pse->X==sx) {
					a0+=pse->A0;
					a1+=pse->A1;
					a2+=pse->A2;
					pse=pse->Next;
				}
				sx++;
				alpha2=(int)(a0>=0 ? 0.5+a0 : 0.5-a0);
				if (!alpha2) {
					sct.PaintScanline(sct,sx0,sy,1,alpha,0,0);
					continue;
				}
				if (pse==&seTerminator) {
					sct.PaintScanline(sct,sx0,sy,2,alpha,0,alpha2);
					break;
				}
				if (pse->X>sx) {
					t=a1+a2*(pse->X-1-sx);
					ta=(int)(t>=0 ? 0.5+t : 0.5-t);
					if (alpha2==ta) {
						a1=t+a2;
						sx=pse->X;
					}
				}
				a0=a1;
				a1+=a2;
				if (pse->X==sx) {
					a0+=pse->A0;
					a1+=pse->A1;
					a2+=pse->A2;
					pse=pse->Next;
				}
				sx++;
				alpha3=(int)(a0>=0 ? 0.5+a0 : 0.5-a0);
				if (!alpha3) {
					sct.PaintScanline(sct,sx0,sy,sx-1-sx0,alpha,alpha2,alpha2);
				}
				else {
					sct.PaintScanline(sct,sx0,sy,sx-sx0,alpha,alpha2,alpha3);
				}
			} while (pse!=&seTerminator);
		}
		sy++;
	} while (sy<sly2);

	if (slmem!=autoScanlines) free(slmem);
	while (seChunks) {
		psec=seChunks;
		seChunks=psec->Next;
		free(psec);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintEdgeCorrection(
	double x1, double y1, double x2, double y2,
	emColor color1, emColor color2
) const
{
	void * h1R, * h1G, * h1B, * h2R, * h2G, * h2B, * p;
	double t,dx,dy,adx,gx,gy,cx1,cy1,cx2,cy2,px1,py1,px2,py2,qx1,qy1,qx2,qy2;
	double a1,a2,ac1,ac2;
	emUInt32 pix,rmsk,gmsk,bmsk;
	int sy,sx,bpp,rsh,gsh,bsh,alpha1,alpha2,alpha3;
	emColor tc;
	bool wasInUserSpace;

	x1=x1*ScaleX+OriginX;
	y1=y1*ScaleY+OriginY;
	x2=x2*ScaleX+OriginX;
	y2=y2*ScaleY+OriginY;

	if (y1>y2) {
		t=y1; y1=y2; y2=t;
		t=x1; x1=x2; x2=t;
		tc=color1; color1=color2; color2=tc;
	}

	dx=x2-x1;
	dy=y2-y1;
	adx=fabs(dx);
	if ( dy>=0.0001) gx=dx/dy; else gx=0.0;
	if (adx>=0.0001) gy=dy/dx; else gy=0.0;

	if (y1<ClipY1) {
		if (y2<=ClipY1) return;
		x1+=(ClipY1-y1)*gx;
		y1=ClipY1;
	}
	if (y2>ClipY2) {
		if (y1>=ClipY2) return;
		x2+=(ClipY2-y2)*gx;
		y2=ClipY2;
	}

	if (dx>=0) {
		if (x1<ClipX1) {
			if (x2<=ClipX1) return;
			y1+=(ClipX1-x1)*gy;
			x1=ClipX1;
		}
		if (x2>ClipX2) {
			if (x1>=ClipX2) return;
			y2+=(ClipX2-x2)*gy;
			x2=ClipX2;
		}
		sx=(int)x1;
		cx1=x1;
		cx2=x2;
	}
	else {
		if (x2<ClipX1) {
			if (x1<=ClipX1) return;
			y2+=(ClipX1-x2)*gy;
			x2=ClipX1;
		}
		if (x1>ClipX2) {
			if (x2>=ClipX2) return;
			y1+=(ClipX2-x1)*gy;
			x1=ClipX2;
		}
		sx=((int)ceil(x1))-1;
		cx1=x2;
		cx2=x1;
	}
	sy=(int)y1;
	cy1=y1;
	cy2=y2;
	if (adx>dy) {
		cy1=floor(cy1);
		cy2=ceil(cy2);
	}
	else {
		cx1=floor(cx1);
		cx2=ceil(cx2);
	}

	if (color1.IsTotallyTransparent() || color2.IsTotallyTransparent()) return;

	wasInUserSpace=LeaveUserSpace();

	ac1=color1.GetAlpha()*(1.0/255.0);
	ac2=color2.GetAlpha()*(1.0/255.0);

	bpp=PixelFormat->BytesPerPixel;
	h1R=((char*)PixelFormat->RedHash  )+(color1.GetRed()  <<8)*bpp;
	h1G=((char*)PixelFormat->GreenHash)+(color1.GetGreen()<<8)*bpp;
	h1B=((char*)PixelFormat->BlueHash )+(color1.GetBlue() <<8)*bpp;
	h2R=((char*)PixelFormat->RedHash  )+(color2.GetRed()  <<8)*bpp;
	h2G=((char*)PixelFormat->GreenHash)+(color2.GetGreen()<<8)*bpp;
	h2B=((char*)PixelFormat->BlueHash )+(color2.GetBlue() <<8)*bpp;

	rsh=PixelFormat->RedShift;
	gsh=PixelFormat->GreenShift;
	bsh=PixelFormat->BlueShift;
	rmsk=PixelFormat->RedRange;
	gmsk=PixelFormat->GreenRange;
	bmsk=PixelFormat->BlueRange;

	p=((char*)Map)+sy*(size_t)BytesPerRow;
	for (;;) {
		px1=sx;
		py1=sy;
		px2=px1+1.0;
		py2=py1+1.0;
		if (px1<cx1) px1=cx1;
		if (py1<cy1) py1=cy1;
		if (px2>cx2) px2=cx2;
		if (py2>cy2) py2=cy2;
		qx1=x1;
		qy1=y1;
		qx2=x2;
		qy2=y2;
		if (qy1<py1) { qx1+=(py1-qy1)*gx; qy1=py1; }
		if (qy2>py2) { qx2+=(py2-qy2)*gx; qy2=py2; }
		if (dx>=0) {
			if (qx1<px1) { qy1+=(px1-qx1)*gy; qx1=px1; }
			if (qx2>px2) { qy2+=(px2-qx2)*gy; qx2=px2; }
			a2=py2-qy2;
		}
		else {
			if (qx2<px1) { qy2+=(px1-qx2)*gy; qx2=px1; }
			if (qx1>px2) { qy1+=(px2-qx1)*gy; qx1=px2; }
			a2=qy1-py1;
		}
		a2=a2*(px2-px1)+(qy2-qy1)*((qx1+qx2)*0.5-px1);
		a1=(py2-py1)*(px2-px1)-a2;
		a1*=ac1;
		a2*=ac2;
		if (a1>=0.001 && a2>=0.001) {
			t=255.0/((1-a1)*(1-a2));
			alpha1=(int)(a1*a2*(1-a2)*t);
			alpha2=(int)(a1*a2*a2*t);
			alpha3=(int)((1-a1-a2)*t);

#			define PEC_TEMPLATE(PTYPE) \
				if (alpha3>0) { \
					pix=((PTYPE*)p)[sx]; \
					((PTYPE*)p)[sx]=(PTYPE)( \
						(((((pix>>rsh)&rmsk)*alpha3+127)/255)<<rsh) + \
						(((((pix>>gsh)&gmsk)*alpha3+127)/255)<<gsh) + \
						(((((pix>>bsh)&bmsk)*alpha3+127)/255)<<bsh) + \
						((PTYPE*)h1R)[alpha1] + \
						((PTYPE*)h1G)[alpha1] + \
						((PTYPE*)h1B)[alpha1] + \
						((PTYPE*)h2R)[alpha2] + \
						((PTYPE*)h2G)[alpha2] + \
						((PTYPE*)h2B)[alpha2] \
					); \
				} \
				else { \
					((PTYPE*)p)[sx]=(PTYPE)( \
						((PTYPE*)h1R)[alpha1] + \
						((PTYPE*)h1G)[alpha1] + \
						((PTYPE*)h1B)[alpha1] + \
						((PTYPE*)h2R)[alpha2] + \
						((PTYPE*)h2G)[alpha2] + \
						((PTYPE*)h2B)[alpha2] \
					); \
				}

			if (bpp==4) {
				PEC_TEMPLATE(emUInt32)
			}
			else if (bpp==2) {
				PEC_TEMPLATE(emUInt16)
			}
			else {
				PEC_TEMPLATE(emUInt8)
			}
		}
		if (dx>=0) {
			if ((sy+1-y1)*dx>(sx+1-x1)*dy) {
				sx++;
				if (sx<cx2) continue;
				break;
			}
		}
		else {
			if ((sy+1-y1)*dx<(sx-x1)*dy) {
				sx--;
				if (sx+1>cx1) continue;
				break;
			}
		}
		sy++;
		if (sy>=cy2) break;
		p=((char*)p)+BytesPerRow;
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintEllipse(
	double x, double y, double w, double h, const emTexture & texture,
	emColor canvasColor
) const
{
	double xy[256*2];
	double f,rx,ry;
	int i,n;
	bool wasInUserSpace;

	if (x*ScaleX+OriginX>=ClipX2) return;
	if ((x+w)*ScaleX+OriginX<=ClipX1) return;
	if (y*ScaleY+OriginY>=ClipY2) return;
	if ((y+h)*ScaleY+OriginY<=ClipY1) return;
	if (w<=0.0 || h<=0.0) return;

	wasInUserSpace=LeaveUserSpace();

	rx=w*0.5;
	ry=h*0.5;
	x+=rx;
	y+=ry;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=2*M_PI/n;
	for (i=0; i<n; i++) {
		xy[i*2]=cos(f*i)*rx+x;
		xy[i*2+1]=sin(f*i)*ry+y;
	}
	PaintPolygon(xy,n,texture,canvasColor);

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintEllipse(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, const emTexture & texture, emColor canvasColor
) const
{
	double xy[258*2];
	double f,rx,ry;
	int i,n;
	bool wasInUserSpace;

	startAngle*=M_PI/180.0;
	rangeAngle*=M_PI/180.0;
	if (rangeAngle<=0.0) {
		if (rangeAngle==0.0) return;
		startAngle+=rangeAngle;
		rangeAngle=-rangeAngle;
	}
	if (rangeAngle>=2*M_PI) {
		PaintEllipse(x,y,w,h,texture,canvasColor);
		return;
	}
	if (x*ScaleX+OriginX>=ClipX2) return;
	if ((x+w)*ScaleX+OriginX<=ClipX1) return;
	if (y*ScaleY+OriginY>=ClipY2) return;
	if ((y+h)*ScaleY+OriginY<=ClipY1) return;
	if (w<=0.0 || h<=0.0) return;

	wasInUserSpace=LeaveUserSpace();

	rx=w*0.5;
	ry=h*0.5;
	x+=rx;
	y+=ry;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f>256.0) f=256.0;
	f=f*rangeAngle/(2*M_PI);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=rangeAngle/n;
	for (i=0; i<=n; i++) {
		xy[i*2]=cos(startAngle+f*i)*rx+x;
		xy[i*2+1]=sin(startAngle+f*i)*ry+y;
	}
	xy[(n+1)*2]=x;
	xy[(n+1)*2+1]=y;
	PaintPolygon(xy,n+2,texture,canvasColor);

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintRoundRect(
	double x, double y, double w, double h, double rx, double ry,
	const emTexture & texture, emColor canvasColor
) const
{
	double xy[260*2];
	double x2,y2,f,dx,dy;
	int i,n;
	bool wasInUserSpace;

	if (w<=0.0) return;
	if (x*ScaleX+OriginX>=ClipX2) return;
	x2=x+w;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (h<=0.0) return;
	if (y*ScaleY+OriginY>=ClipY2) return;
	y2=y+h;
	if (y2*ScaleY+OriginY<=ClipY1) return;
	if (rx<=0.0 || ry<=0.0) {
		PaintRect(x,y,w,h,texture,canvasColor);
		return;
	}

	wasInUserSpace=LeaveUserSpace();

	if (rx>w*0.5) rx=w*0.5;
	if (ry>h*0.5) ry=h*0.5;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f>256.0) f=256.0;
	f*=0.25;
	if (f<=1.0) n=1;
	else if (f>=64.0) n=64;
	else n=(int)(f+0.5);
	f=0.5*M_PI/n;
	x+=rx; y+=ry;
	x2-=rx; y2-=ry;
	for (i=0; i<=n; i++) {
		dx=cos(f*i);
		dy=sin(f*i);
		xy[i*2]=x-dx*rx;
		xy[i*2+1]=y-dy*ry;
		xy[(n+1+i)*2]=x2+dy*rx;
		xy[(n+1+i)*2+1]=y-dx*ry;
		xy[(2*n+2+i)*2]=x2+dx*rx;
		xy[(2*n+2+i)*2+1]=y2+dy*ry;
		xy[(3*n+3+i)*2]=x-dy*rx;
		xy[(3*n+3+i)*2+1]=y2+dx*ry;
	}
	PaintPolygon(xy,4*n+4,texture,canvasColor);

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintLine(
	double x1, double y1, double x2, double y2, double thickness,
	LineCap cap1, LineCap cap2, emColor color, emColor canvasColor
) const
{
	double xy[258*2];
	double f,dx,dy,c,s;
	int i,m,n;
	bool wasInUserSpace;

	if (thickness<=0.0) return;
	f=thickness*0.71;
	dx=x2-x1;
	if (dx>0.0) {
		if ((x1-f)*ScaleX+OriginX>=ClipX2) return;
		if ((x2+f)*ScaleX+OriginX<=ClipX1) return;
	}
	else {
		if ((x2-f)*ScaleX+OriginX>=ClipX2) return;
		if ((x1+f)*ScaleX+OriginX<=ClipX1) return;
	}
	dy=y2-y1;
	if (dy>0.0) {
		if ((y1-f)*ScaleY+OriginY>=ClipY2) return;
		if ((y2+f)*ScaleY+OriginY<=ClipY1) return;
	}
	else {
		if ((y2-f)*ScaleY+OriginY>=ClipY2) return;
		if ((y1+f)*ScaleY+OriginY<=ClipY1) return;
	}

	wasInUserSpace=LeaveUserSpace();

	f=sqrt(dx*dx+dy*dy);
	if (f<1E-20) {
		dx=thickness*0.5;
		dy=0.0;
		x2=x1;
		y2=y1;
	}
	else {
		f=thickness*0.5/f;
		dx*=f;
		dy*=f;
	}
	switch (cap1) {
	case LC_FLAT:
		xy[0]=x1-dy;
		xy[1]=y1+dx;
		xy[2]=x1+dy;
		xy[3]=y1-dx;
		n=2;
		break;
	case LC_SQUARE:
		xy[0]=x1-dx-dy;
		xy[1]=y1-dy+dx;
		xy[2]=x1-dx+dy;
		xy[3]=y1-dy-dx;
		n=2;
		break;
	default:
		f=CircleQuality*sqrt(thickness*0.5*(ScaleX+ScaleY));
		if (f>=256.0) f=256.0;
		f*=0.5;
		if (f<=1.0) n=1;
		else if (f>=128.0) n=128;
		else n=(int)(f+0.5);
		f=M_PI/n;
		n++;
		for (i=0; i<n; i++) {
			c=cos(f*i);
			s=sin(f*i);
			xy[i*2]=x1-dy*c-dx*s;
			xy[i*2+1]=y1+dx*c-dy*s;
		}
		break;
	}
	switch (cap2) {
	case LC_FLAT:
		xy[n*2]=x2+dy;
		xy[n*2+1]=y2-dx;
		xy[(n+1)*2]=x2-dy;
		xy[(n+1)*2+1]=y2+dx;
		n+=2;
		break;
	case LC_SQUARE:
		xy[n*2]=x2+dx+dy;
		xy[n*2+1]=y2+dy-dx;
		xy[(n+1)*2]=x2+dx-dy;
		xy[(n+1)*2+1]=y2+dy+dx;
		n+=2;
		break;
	default:
		if (cap2==cap1) {
			for (i=0; i<n; i++) {
				xy[(n+i)*2]=x2+x1-xy[i*2];
				xy[(n+i)*2+1]=y2+y1-xy[i*2+1];
			}
			n*=2;
		}
		else {
			f=CircleQuality*sqrt(thickness*0.5*(ScaleX+ScaleY));
			if (f>=256.0) f=256.0;
			f*=0.5;
			if (f<=1.0) m=1;
			else if (f>=128.0) m=128;
			else m=(int)(f+0.5);
			f=M_PI/m;
			m++;
			for (i=0; i<m; i++) {
				c=cos(f*i);
				s=sin(f*i);
				xy[(n+i)*2]=x2+dy*c+dx*s;
				xy[(n+i)*2+1]=y2-dx*c+dy*s;
			}
			n+=m;
			break;
		}
	}
	PaintPolygon(xy,n,color,canvasColor);

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintRectOutline(
	double x, double y, double w, double h, double thickness,
	emColor color, emColor canvasColor
) const
{
	double xy[10*2];
	double x1,y1,x2,y2,t2;
	bool wasInUserSpace;

	if (thickness<=0.0) return;
	t2=thickness/2.0;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (x1>=x2) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;
	if (y1>=y2) return;
	xy[0]=x1; xy[1]=y1;
	xy[2]=x2; xy[3]=y1;
	xy[4]=x2; xy[5]=y2;
	xy[6]=x1; xy[7]=y2;
	x1+=thickness; y1+=thickness;
	x2-=thickness; y2-=thickness;
	if (x1>=x2 || y1>=y2) {
		PaintPolygon(xy,4,color,canvasColor);
		return;
	}

	wasInUserSpace=LeaveUserSpace();

	xy[ 8]=xy[0]; xy[ 9]=xy[1];
	xy[10]=x1;    xy[11]=y1;
	xy[12]=x1;    xy[13]=y2;
	xy[14]=x2;    xy[15]=y2;
	xy[16]=x2;    xy[17]=y1;
	xy[18]=x1;    xy[19]=y1;
	PaintPolygon(xy,10,color,canvasColor);

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintPolygonOutline(
	const double xy[], int n, double thickness, emColor color,
	emColor canvasColor
) const
{
	bool wasInUserSpace;
	int i;

	wasInUserSpace=LeaveUserSpace();

	for (i=0; i<n; i++) {
		PaintLine(
			xy[i*2],xy[i*2+1],
			xy[(i+1)%n*2],xy[(i+1)%n*2+1],
			thickness,
			color.GetAlpha()==255 ? LC_FLAT : LC_ROUND,
			LC_ROUND,
			color
		);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintEllipseOutline(
	double x, double y, double w, double h, double thickness,
	emColor color, emColor canvasColor
) const
{
	double xy[514*2];
	double x1,y1,x2,y2,f,t2,cx,cy,rx,ry;
	bool wasInUserSpace;
	int i,n,m;

	if (thickness<=0.0) return;
	t2=thickness/2.0;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (x1>=x2) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;
	if (y1>=y2) return;

	wasInUserSpace=LeaveUserSpace();

	cx=(x1+x2)*0.5;
	cy=(y1+y2)*0.5;
	rx=x2-cx;
	ry=y2-cy;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=2*M_PI/n;
	for (i=0; i<n; i++) {
		xy[i*2]=cos(f*i)*rx+cx;
		xy[i*2+1]=sin(f*i)*ry+cy;
	}
	rx-=thickness;
	ry-=thickness;
	if (rx<=0.0 || ry<=0.0) {
		PaintPolygon(xy,n,color,canvasColor);
	}
	else {
		xy[n*2]=xy[0];
		xy[n*2+1]=xy[1];
		f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
		if (f<=3.0) m=3;
		else if (f>=256.0) m=256;
		else m=(int)(f+0.5);
		f=2*M_PI/m;
		for (i=0; i<m; i++) {
			xy[(n+m+1-i)*2]=cos(f*i)*rx+cx;
			xy[(n+m+1-i)*2+1]=sin(f*i)*ry+cy;
		}
		xy[(n+1)*2]=xy[(n+m+1)*2];
		xy[(n+1)*2+1]=xy[(n+m+1)*2+1];
		PaintPolygon(xy,n+m+2,color,canvasColor);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintEllipseOutline(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, double thickness, emColor color,
	emColor canvasColor
) const
{
	double xy[514*2];
	double x1,y1,x2,y2,f,t2,cx,cy,rx,ry;
	bool wasInUserSpace;
	int i,m,n;

	startAngle*=M_PI/180.0;
	rangeAngle*=M_PI/180.0;
	if (rangeAngle<=0.0) {
		if (rangeAngle==0.0) return;
		startAngle+=rangeAngle;
		rangeAngle=-rangeAngle;
	}
	if (rangeAngle>=2*M_PI) {
		PaintEllipseOutline(x,y,w,h,thickness,color,canvasColor);
		return;
	}
	if (thickness<=0.0) return;
	t2=thickness/2.0;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (x1>=x2) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;
	if (y1>=y2) return;

	wasInUserSpace=LeaveUserSpace();

	cx=(x1+x2)*0.5;
	cy=(y1+y2)*0.5;
	rx=x2-cx;
	ry=y2-cy;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f>256.0) f=256.0;
	f=f*rangeAngle/(2*M_PI);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=rangeAngle/n;
	for (i=0; i<=n; i++) {
		xy[i*2]=cos(startAngle+f*i)*rx+cx;
		xy[i*2+1]=sin(startAngle+f*i)*ry+cy;
	}
	rx-=thickness;
	ry-=thickness;
	if (rx<=0.0 || ry<=0.0) {
		xy[(n+1)*2]=cx;
		xy[(n+1)*2+1]=cy;
		PaintPolygon(xy,n+2,color,canvasColor);
	}
	else {
		f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
		if (f>256.0) f=256.0;
		f=f*rangeAngle/(2*M_PI);
		if (f<=3.0) m=3;
		else if (f>=256.0) m=256;
		else m=(int)(f+0.5);
		f=rangeAngle/m;
		for (i=0; i<=m; i++) {
			xy[(n+m+1-i)*2]=cos(startAngle+f*i)*rx+cx;
			xy[(n+m+1-i)*2+1]=sin(startAngle+f*i)*ry+cy;
		}
		PaintPolygon(xy,n+m+2,color,canvasColor);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintRoundRectOutline(
	double x, double y, double w, double h, double rx, double ry,
	double thickness, emColor color, emColor canvasColor
) const
{
	double xy[522*2];
	double x1,y1,x2,y2,f,t2,dx,dy;
	bool wasInUserSpace;
	int i,m,n;

	if (thickness<=0.0) return;
	t2=thickness/2.0;
	rx+=t2; ry+=t2;
	if (rx<=0.0 || ry<=0.0) {
		PaintRectOutline(x,y,w,h,thickness,color,canvasColor);
		return;
	}
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (x1>=x2) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;
	if (y1>=y2) return;

	wasInUserSpace=LeaveUserSpace();

	if (rx>(x2-x1)*0.5) rx=(x2-x1)*0.5;
	if (ry>(y2-y1)*0.5) ry=(y2-y1)*0.5;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f>256.0) f=256.0;
	f*=0.25;
	if (f<=1.0) n=1;
	else if (f>=64.0) n=64;
	else n=(int)(f+0.5);
	f=0.5*M_PI/n;
	x1+=rx; y1+=ry;
	x2-=rx; y2-=ry;
	for (i=0; i<=n; i++) {
		dx=cos(f*i);
		dy=sin(f*i);
		xy[i*2]=x1-dx*rx;
		xy[i*2+1]=y1-dy*ry;
		xy[(n+1+i)*2]=x2+dy*rx;
		xy[(n+1+i)*2+1]=y1-dx*ry;
		xy[(2*n+2+i)*2]=x2+dx*rx;
		xy[(2*n+2+i)*2+1]=y2+dy*ry;
		xy[(3*n+3+i)*2]=x1-dy*rx;
		xy[(3*n+3+i)*2+1]=y2+dx*ry;
	}
	rx-=thickness;
	ry-=thickness;
	if (rx<0.0) {
		x1-=rx; x2+=rx;
		rx=0.0;
	}
	if (ry<0.0) {
		y1-=ry; y2+=ry;
		ry=0.0;
	}
	if (x1-rx>=x2+rx || y1-ry>=y2+ry) {
		PaintPolygon(xy,4*n+4,color,canvasColor);
	}
	else {
		xy[(4*n+4)*2]=xy[0];
		xy[(4*n+4)*2+1]=xy[1];
		f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
		if (f>256.0) f=256.0;
		f*=0.25;
		if (f<=1.0) m=1;
		else if (f>=64.0) m=64;
		else m=(int)(f+0.5);
		f=0.5*M_PI/m;
		for (i=0; i<=m; i++) {
			dx=cos(f*i);
			dy=sin(f*i);
			xy[(4*n+4*m+9-i)*2]=x1-dx*rx;
			xy[(4*n+4*m+9-i)*2+1]=y1-dy*ry;
			xy[(4*n+3*m+8-i)*2]=x2+dy*rx;
			xy[(4*n+3*m+8-i)*2+1]=y1-dx*ry;
			xy[(4*n+2*m+7-i)*2]=x2+dx*rx;
			xy[(4*n+2*m+7-i)*2+1]=y2+dy*ry;
			xy[(4*n+m+6-i)*2]=x1-dy*rx;
			xy[(4*n+m+6-i)*2+1]=y2+dx*ry;
		}
		xy[(4*n+5)*2]=xy[(4*n+4*m+9)*2];
		xy[(4*n+5)*2+1]=xy[(4*n+4*m+9)*2+1];
		PaintPolygon(xy,4*n+4*m+10,color,canvasColor);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintBorderImage(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, int srcX, int srcY, int srcW, int srcH,
	int srcL, int srcT, int srcR, int srcB, int alpha, emColor canvasColor,
	int whichSubRects
) const
{
	bool wasInUserSpace;
	double f;

	wasInUserSpace=LeaveUserSpace();

	if (!canvasColor.IsOpaque()) {
		f=RoundX(x+l)-x; if (f>0 && f<w-r) l=f;
		f=x+w-RoundX(x+w-r); if (f>0 && f<w-l) r=f;
		f=RoundY(y+t)-y; if (f>0 && f<h-b) t=f;
		f=y+h-RoundY(y+h-b); if (f>0 && f<h-t) b=f;
	}

	if (whichSubRects&(1<<8)) {
		PaintImage(
			x,y,l,t,
			img,
			srcX,srcY,srcL,srcT,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<5)) {
		PaintImage(
			x+l,y,w-l-r,t,
			img,
			srcX+srcL,srcY,srcW-srcL-srcR,srcT,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<2)) {
		PaintImage(
			x+w-r,y,r,t,
			img,
			srcX+srcW-srcR,srcY,srcR,srcT,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<7)) {
		PaintImage(
			x,y+t,l,h-t-b,
			img,
			srcX,srcY+srcT,srcL,srcH-srcT-srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<4)) {
		PaintImage(
			x+l,y+t,w-l-r,h-t-b,
			img,
			srcX+srcL,srcY+srcT,srcW-srcL-srcR,srcH-srcT-srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<1)) {
		PaintImage(
			x+w-r,y+t,r,h-t-b,
			img,
			srcX+srcW-srcR,srcY+srcT,srcR,srcH-srcT-srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<6)) {
		PaintImage(
			x,y+h-b,l,b,
			img,
			srcX,srcY+srcH-srcB,srcL,srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<3)) {
		PaintImage(
			x+l,y+h-b,w-l-r,b,
			img,
			srcX+srcL,srcY+srcH-srcB,srcW-srcL-srcR,srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<0)) {
		PaintImage(
			x+w-r,y+h-b,r,b,
			img,
			srcX+srcW-srcR,srcY+srcH-srcB,srcR,srcB,
			alpha,canvasColor,emTexture::EXTEND_EDGE
		);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintBorderImageColored(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, int srcX, int srcY, int srcW, int srcH,
	int srcL, int srcT, int srcR, int srcB, emColor color1, emColor color2,
	emColor canvasColor, int whichSubRects
) const
{
	bool wasInUserSpace;
	double f;

	wasInUserSpace=LeaveUserSpace();

	if (!canvasColor.IsOpaque()) {
		f=RoundX(x+l)-x; if (f>0 && f<w-r) l=f;
		f=x+w-RoundX(x+w-r); if (f>0 && f<w-l) r=f;
		f=RoundY(y+t)-y; if (f>0 && f<h-b) t=f;
		f=y+h-RoundY(y+h-b); if (f>0 && f<h-t) b=f;
	}

	if (whichSubRects&(1<<8)) {
		PaintImageColored(
			x,y,l,t,
			img,
			srcX,srcY,srcL,srcT,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<5)) {
		PaintImageColored(
			x+l,y,w-l-r,t,
			img,
			srcX+srcL,srcY,srcW-srcL-srcR,srcT,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<2)) {
		PaintImageColored(
			x+w-r,y,r,t,
			img,
			srcX+srcW-srcR,srcY,srcR,srcT,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<7)) {
		PaintImageColored(
			x,y+t,l,h-t-b,
			img,
			srcX,srcY+srcT,srcL,srcH-srcT-srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<4)) {
		PaintImageColored(
			x+l,y+t,w-l-r,h-t-b,
			img,
			srcX+srcL,srcY+srcT,srcW-srcL-srcR,srcH-srcT-srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<1)) {
		PaintImageColored(
			x+w-r,y+t,r,h-t-b,
			img,
			srcX+srcW-srcR,srcY+srcT,srcR,srcH-srcT-srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<6)) {
		PaintImageColored(
			x,y+h-b,l,b,
			img,
			srcX,srcY+srcH-srcB,srcL,srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<3)) {
		PaintImageColored(
			x+l,y+h-b,w-l-r,b,
			img,
			srcX+srcL,srcY+srcH-srcB,srcW-srcL-srcR,srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}
	if (whichSubRects&(1<<0)) {
		PaintImageColored(
			x+w-r,y+h-b,r,b,
			img,
			srcX+srcW-srcR,srcY+srcH-srcB,srcR,srcB,
			color1,color2,canvasColor,emTexture::EXTEND_EDGE
		);
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintText(
	double x, double y, const char * text, double charHeight,
	double widthScale, emColor color, emColor canvasColor,
	int textLen
) const
{
	double charWidth,showHeight,rcw,cx1,cx2,x1;
	int i,n,c,imgX,imgY,imgW,imgH;
	bool wasInUserSpace;
	emImage * pImg;

	if (
		y*ScaleY+OriginY>=ClipY2 ||
		(y+charHeight)*ScaleY+OriginY<=ClipY1 ||
		x>=(cx2=(ClipX2-OriginX)/ScaleX) ||
		ClipX1>=ClipX2 ||
		charHeight*ScaleY<=0.1 ||
		widthScale<=0.0
	) return;

	wasInUserSpace=LeaveUserSpace();

	rcw=charHeight/CharBoxTallness;
	charWidth=rcw*widthScale;
	cx1=(ClipX1-OriginX)/ScaleX;
	emMBState mbState;
	if (charHeight*ScaleY>=1.7) {
		for (i=0; i<textLen; i++) {
			c=(unsigned char)text[i];
			if (!c) break;
			if (c>=0x80) {
				n=emDecodeChar(&c,text+i,textLen-i,&mbState);
				if (n>1) i+=n-1;
			}
			x1=x;
			x+=charWidth;
			if (x>cx1) {
				if (x1>=cx2) break;
				Model->FontCache->GetChar(
					c,charWidth*ScaleX,charHeight*ScaleY,
					&pImg,&imgX,&imgY,&imgW,&imgH
				);
				showHeight=rcw*imgH/imgW;
				if (showHeight>charHeight) showHeight=charHeight;
				PaintImageColored(
					x1,y+(charHeight-showHeight)*0.5,
					charWidth,showHeight,
					*pImg,imgX,imgY,imgW,imgH,
					0,color,canvasColor,emTexture::EXTEND_ZERO
				);
			}
		}
	}
	else {
		color.SetAlpha((emByte)((color.GetAlpha()+2)/3));
		for (i=0, x1=x; i<textLen; i++) {
			c=(unsigned char)text[i];
			if (c<=0x20) {
				if (!c) break;
				if (x1<x && x>cx1) {
					if (x1>=cx2) break;
					PaintRect(x1,y,x-x1,charHeight,color,canvasColor);
				}
				x+=charWidth;
				x1=x;
			}
			else {
				if (c>=0x80) {
					n=emDecodeChar(&c,text+i,textLen-i,&mbState);
					if (n>1) i+=n-1;
				}
				x+=charWidth;
			}
		}
		if (x1<x && x>cx1 && x1<cx2) {
			PaintRect(x1,y,x-x1,charHeight,color,canvasColor);
		}
	}

	if (wasInUserSpace) EnterUserSpace();
}


void emPainter::PaintTextBoxed(
	double x, double y, double w, double h, const char * text,
	double maxCharHeight, emColor color, emColor canvasColor,
	emAlignment boxAlignment, emAlignment textAlignment,
	double minWidthScale, bool formatted, double relLineSpace,
	int textLen
) const
{
	double tx,ty,tw,th,cw,ch,ws;
	int c,i,j,k,n,cols;
	bool wasInUserSpace;

	ch=maxCharHeight;
	tw=GetTextSize(text,ch,formatted,relLineSpace,&th,textLen);
	if (tw<=0.0) return;

	wasInUserSpace=LeaveUserSpace();

	if (th>h) {
		ch*=h/th;
		tw*=h/th;
		th=h;
	}
	ws=w/tw;
	if (ws<1.0) {
		tw=w;
		if (ws<minWidthScale) {
			th*=ws/minWidthScale;
			ch*=ws/minWidthScale;
			ws=minWidthScale;
		}
	}
	else {
		ws=1.0;
		if (ws<minWidthScale) {
			ws=minWidthScale;
			tw*=ws;
			if (tw>w) {
				th*=w/tw;
				ch*=w/tw;
				tw=w;
			}
		}
	}
	if ((boxAlignment&EM_ALIGN_LEFT)==0) {
		if ((boxAlignment&EM_ALIGN_RIGHT)!=0) x+=w-tw;
		else x+=(w-tw)*0.5;
	}
	if ((boxAlignment&EM_ALIGN_TOP)==0) {
		if ((boxAlignment&EM_ALIGN_BOTTOM)!=0) y+=h-th+ch*relLineSpace;
		else y+=(h-th+ch*relLineSpace)*0.5;
	}
	if (formatted) {
		cw=ch*ws/CharBoxTallness;
		emMBState mbState;
		for (ty=y, i=0; ; ty+=ch*(1.0+relLineSpace)) {
			tx=x;
			if ((textAlignment&EM_ALIGN_LEFT)==0) {
				emMBState mbState2=mbState;
				for (j=i, cols=-j; j<textLen; j++) {
					c=(unsigned char)text[j];
					if (c<=0x0d) {
						if (c==0x09) {
							cols=((cols+j+8)&~7)-j;
						}
						else if (c==0x0a || c==0x0d || c==0) {
							break;
						}
					}
					else if (c>=0x80) {
						n=emDecodeChar(&c,text+j,textLen-j,&mbState2);
						if (n>1) {
							j+=n-1;
							cols-=n-1;
						}
					}
				}
				cols+=j;
				if ((textAlignment&EM_ALIGN_RIGHT)!=0) tx+=tw-cols*cw;
				else tx+=(tw-cols*cw)*0.5;
			}
			for (cols=0, j=i, k=-i; i<textLen; i++) {
				c=(unsigned char)text[i];
				if (c<=0x0d) {
					if (c==0x09) {
						if (j<i) {
							PaintText(tx+cols*cw,ty,text+j,ch,ws,color,canvasColor,i-j);
							cols+=k+i;
						}
						cols=(cols+8)&~7;
						j=i+1;
						k=-j;
					}
					else if (c==0x0a || c==0x0d || c==0) {
						break;
					}
				}
				else if (c>=0x80) {
					n=emDecodeChar(&c,text+i,textLen-i,&mbState);
					if (n>1) {
						i+=n-1;
						k-=n-1;
					}
				}
			}
			if (j<i) PaintText(tx+cols*cw,ty,text+j,ch,ws,color,canvasColor,i-j);
			if (i>=textLen || !text[i]) break;
			if (text[i]==0x0d && i+1<textLen && text[i+1]==0x0a) i++;
			i++;
		}
	}
	else {
		PaintText(x,y,text,ch,ws,color,canvasColor,textLen);
	}

	if (wasInUserSpace) EnterUserSpace();
}


double emPainter::GetTextSize(
	const char * text, double charHeight, bool formatted,
	double relLineSpace, double * pHeight, int textLen
)
{
	int i,n,c,columns,rows,rowcols;

	if (formatted) {
		emMBState mbState;
		for (i=0, columns=0, rows=1, rowcols=0; i<textLen; i++) {
			c=(unsigned char)text[i];
			if (c<=0x0d) {
				if (c==0x09) {
					rowcols=((rowcols+i+8)&~7)-i-1;
				}
				else if (c==0x0a) {
					rowcols+=i;
					if (columns<rowcols) columns=rowcols;
					rowcols=-i-1;
					rows++;
				}
				else if (c==0x0d) {
					rowcols+=i;
					if (columns<rowcols) columns=rowcols;
					if (i+1<textLen && text[i+1]==0x0a) i++;
					rowcols=-i-1;
					rows++;
				}
				else if (c==0) {
					break;
				}
			}
			else if (c>=0x80) {
				n=emDecodeChar(&c,text+i,textLen-i,&mbState);
				if (n>1) {
					i+=n-1;
					rowcols-=n-1;
				}
			}
		}
		rowcols+=i;
		if (columns<rowcols) columns=rowcols;
	}
	else {
		columns=emGetDecodedCharCount(text,textLen);
		rows=1;
	}
	if (pHeight) *pHeight=charHeight*(1.0+relLineSpace)*rows;
	return charHeight*columns/CharBoxTallness;
}


emRef<emPainter::SharedModel> emPainter::SharedModel::Acquire(
	emRootContext & rootContext
)
{
	EM_IMPL_ACQUIRE_COMMON(emPainter::SharedModel,rootContext,"")
}


void emPainter::SharedModel::RemoveUnusedPixelFormats()
{
	SharedPixelFormat * pf;
	SharedPixelFormat * * ppf;

	for (ppf=&PixelFormatList;;) {
		pf=*ppf;
		if (!pf) break;
		if (pf->RefCount<=0) {
			*ppf=pf->Next;
			free(pf->RedHash);
			free(pf->GreenHash);
			free(pf->BlueHash);
			free(pf);
		}
		else ppf=&pf->Next;
	}
}


emPainter::SharedModel::SharedModel(emContext & context, const emString & name)
	: emModel(context,name),
	CoreConfig(emCoreConfig::Acquire(GetRootContext())),
	FontCache(emFontCache::Acquire(GetRootContext())),
	PixelFormatList(NULL)
{
#	if EM_HAVE_X86_INTRINSICS
		CanCpuDoAvx2=emCanCpuDoAvx2();
		if (!CanCpuDoAvx2) {
			emWarning("emPainter: no AVX2 (=>slow)");
		}
#	endif
	SetMinCommonLifetime(UINT_MAX);
}


emPainter::SharedModel::~SharedModel()
{
	RemoveUnusedPixelFormats();
}


const double emPainter::CharBoxTallness=1.77;
const double emPainter::CircleQuality=4.5;
