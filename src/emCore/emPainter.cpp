//------------------------------------------------------------------------------
// emPainter.cpp
//
// Copyright (C) 2001,2003-2011,2016-2020,2022,2024 Oliver Hamann.
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

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	ScanlineTool sct(*this);
	if (!sct.Init(texture,canvasColor)) return;

	ix=(int)(x*0x1000);
	ixe=((int)(x2*0x1000))+0xfff;
	ax1=0x1000-(ix&0xfff);
	ax2=(ixe&0xfff)+1;
	ix>>=12;
	ixe>>=12;
	iw=ixe-ix;
	if (iw<=1) {
		if (iw<=0) return;
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
		if (ay1<=0) return;
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
	ScanEntry * autoScanlines[512];
	ScanEntry autoScanEntries[1024];
	ScanEntry * * slmem, * * scanlines, * * ppse;
	ScanEntry * freeScanEntries, * freeScanEntriesEnd, * pse;
	SEChunk * seChunks, * psec;
	const double * pxy;
	double minX,maxX,minY,maxY,x0,y0,x1,y1,x2,y2,dx,dy,ax,a0,a1,a2,va,t;
	double ex1[2],ey1[2],ex2[2],ey2[2];
	int i,alpha,alpha2,alpha3,ta,sly1,sly2,sx,sy,sx0,sy2;

	if (n<3) return;

	minX=maxX=xy[0];
	minY=maxY=xy[1];
	pxy=xy+n*2-2;
	do {
		if      (maxX<pxy[0]) maxX=pxy[0];
		else if (minX>pxy[0]) minX=pxy[0];
		if      (maxY<pxy[1]) maxY=pxy[1];
		else if (minY>pxy[1]) minY=pxy[1];
		pxy-=2;
	} while (pxy>xy);
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

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	ScanlineTool sct(*this);
	if (!sct.Init(texture,canvasColor)) return;

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

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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
}


void emPainter::PaintBezier(
	const double xy[], int n, const emTexture & texture, emColor canvasColor
) const
{
	const double * pxy;
	double * xyOut, * p, * pe;
	double autoMem[512*2];
	double minX,maxX,minY,maxY,cx1,cy1,cx2,cy2,t,dt,s,dx,dy;
	double x1,y1,x2,y2,x3,y3,x4,y4,ll,l,bx1,by1,bx2,by2,b,f;
	int nOut,xyOutSize,m;
	bool skip;

	if (n<3) return;
	n-=n%3;

	minX=maxX=xy[0];
	minY=maxY=xy[1];
	pxy=xy+n*2-2;
	do {
		if      (maxX<pxy[0]) maxX=pxy[0];
		else if (minX>pxy[0]) minX=pxy[0];
		if      (maxY<pxy[1]) maxY=pxy[1];
		else if (minY>pxy[1]) minY=pxy[1];
		pxy-=2;
	} while (pxy>xy);
	cx1=(ClipX1-OriginX)/ScaleX;
	if (maxX<=cx1) return;
	cx2=(ClipX2-OriginX)/ScaleX;
	if (minX>=cx2) return;
	cy1=(ClipY1-OriginY)/ScaleY;
	if (maxY<=cy1) return;
	cy2=(ClipY2-OriginY)/ScaleY;
	if (minY>=cy2) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	xyOut=autoMem;
	xyOutSize=sizeof(autoMem)/sizeof(double);
	nOut=0;

	s=ScaleX+ScaleY;
	pxy=xy;
	do {
		x1=pxy[0]; y1=pxy[1];
		x2=pxy[2]; y2=pxy[3];
		x3=pxy[4]; y3=pxy[5];
		pxy+=6;
		n-=3;
		if (n<=0) pxy=xy;
		x4=pxy[0]; y4=pxy[1];

		skip=
			(x1<=cx1 && x2<=cx1 && x3<=cx1 && x4<=cx1) ||
			(x1>=cx2 && x2>=cx2 && x3>=cx2 && x4>=cx2) ||
			(y1<=cy1 && y2<=cy1 && y3<=cy1 && y4<=cy1) ||
			(y1>=cy2 && y2>=cy2 && y3>=cy2 && y4>=cy2)
		;

		x2-=x1; y2-=y1;
		x3-=x1; y3-=y1;
		x4-=x1; y4-=y1;

		m=1;
		switch (0) {
		default:
			if (skip) break;
			ll=x4*x4+y4*y4;
			if (ll>1E-280) {
				l=sqrt(ll);
				if ((fabs(x2*y4-y2*x4)+fabs(x3*y4-y3*x4))*s<=l*0.01) break;
			}
			else {
				dx=x3-x2; dy=y3-y2;
				l=sqrt(dx*dx+dy*dy);
				if (l*s<=0.01) break;
				if (fabs(x2*dy-y2*dx)*s<=l*0.01) break;
			}
			// b = average of second derivation at start and end.
			bx1=x3-2*x2;    by1=y3-2*y2;
			bx2=x2-2*x3+x4; by2=y2-2*y3+y4;
			b=(sqrt(bx1*bx1+by1*by1)+sqrt(bx2*bx2+by2*by2))*3.0;
			f=CircleQuality*sqrt(b*0.0228*s);
			if (f>=500.0) m=500;
			else if (f>1.0) m=(int)(f+0.5);
		}

		while ((nOut+m)*2>xyOutSize) {
			xyOutSize*=2;
			if (xyOut==autoMem) {
				xyOut=(double*)malloc(xyOutSize*sizeof(double));
				memcpy(xyOut,autoMem,nOut*2*sizeof(double));
			}
			else {
				xyOut=(double*)realloc(xyOut,xyOutSize*sizeof(double));
			}
		}

		x2*=3.0; y2*=3.0;
		x3*=3.0; y3*=3.0;
		x4+=x2-x3; y4+=y2-y3;
		x3-=x2+x2; y3-=y2+y2;
		dt=1.0/m;
		t=0.0;
		p=xyOut+nOut*2;
		pe=p+m*2;
		do {
			p[0] = x1 + t*(x2 + t*(x3 + t*x4));
			p[1] = y1 + t*(y2 + t*(y3 + t*y4));
			p+=2;
			t+=dt;
		} while (p<pe);
		nOut+=m;
	} while (n>=3);

	PaintPolygon(xyOut,nOut,texture,canvasColor);

	if (xyOut!=autoMem) free(xyOut);
}


void emPainter::PaintEllipse(
	double x, double y, double w, double h, const emTexture & texture,
	emColor canvasColor
) const
{
	double xy[256*2];
	double f,rx,ry;
	int i,n;

	if (x*ScaleX+OriginX>=ClipX2) return;
	if ((x+w)*ScaleX+OriginX<=ClipX1) return;
	if (y*ScaleY+OriginY>=ClipY2) return;
	if ((y+h)*ScaleY+OriginY<=ClipY1) return;
	if (w<=0.0 || h<=0.0) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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
}


void emPainter::PaintEllipseSector(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, const emTexture & texture, emColor canvasColor
) const
{
	double xy[258*2];
	double f,rx,ry;
	int i,n;

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

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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
}


void emPainter::PaintRoundRect(
	double x, double y, double w, double h, double rx, double ry,
	const emTexture & texture, emColor canvasColor
) const
{
	double xy[260*2];
	double x2,y2,f,dx,dy;
	int i,n;

	if (w<=0.0) return;
	if (x*ScaleX+OriginX>=ClipX2) return;
	x2=x+w;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	if (h<=0.0) return;
	if (y*ScaleY+OriginY>=ClipY2) return;
	y2=y+h;
	if (y2*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	if (rx<=0.0 || ry<=0.0) {
		PaintRect(x,y,w,h,texture,canvasColor);
		return;
	}

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
}


void emPainter::PaintLine(
	double x1, double y1, double x2, double y2, double thickness,
	const emStroke & stroke, const emStrokeEnd & strokeStart,
	const emStrokeEnd & strokeEnd, emColor canvasColor
) const
{
	double xy[2*2];
	double r,dx,dy,ll,l,nx,ny;

	if (thickness<=0.0) return;
	r=CalculateLinePointMinMaxRadius(thickness,stroke,strokeStart,strokeEnd);
	dx=x2-x1;
	if (dx>0.0) {
		if ((x1-r)*ScaleX+OriginX>=ClipX2) return;
		if ((x2+r)*ScaleX+OriginX<=ClipX1) return;
	}
	else {
		if ((x2-r)*ScaleX+OriginX>=ClipX2) return;
		if ((x1+r)*ScaleX+OriginX<=ClipX1) return;
	}
	dy=y2-y1;
	if (dy>0.0) {
		if ((y1-r)*ScaleY+OriginY>=ClipY2) return;
		if ((y2+r)*ScaleY+OriginY<=ClipY1) return;
	}
	else {
		if ((y2-r)*ScaleY+OriginY>=ClipY2) return;
		if ((y1+r)*ScaleY+OriginY<=ClipY1) return;
	}

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	xy[0]=x1;
	xy[1]=y1;
	xy[2]=x2;
	xy[3]=y2;

	if (strokeStart.IsDecorated() || strokeEnd.IsDecorated()) {
		ll=dx*dx+dy*dy;
		if (ll>1E-280) {
			l=sqrt(ll);
			nx=dx/l;
			ny=dy/l;
		}
		else {
			nx=1.0;
			ny=0.0;
		}
		PaintPolylineWithArrowsAlterBuf(
			xy,2,nx,ny,-nx,-ny,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
	else {
		PaintPolylineWithoutArrows(
			xy,2,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
}


void emPainter::PaintPolyline(
	const double xy[], int n, double thickness, const emStroke & stroke,
	const emStrokeEnd & strokeStart, const emStrokeEnd & strokeEnd,
	emColor canvasColor
) const
{
	const double * pxy, * pxye;
	double minX,maxX,minY,maxY,r,nx1,ny1,nx2,ny2,dx,dy,ll,l;
	bool withArrows;

	if (n<=0 || thickness<=0.0) return;

	minX=maxX=xy[0];
	minY=maxY=xy[1];
	pxy=xy+n*2-2;
	do {
		if      (maxX<pxy[0]) maxX=pxy[0];
		else if (minX>pxy[0]) minX=pxy[0];
		if      (maxY<pxy[1]) maxY=pxy[1];
		else if (minY>pxy[1]) minY=pxy[1];
		pxy-=2;
	} while (pxy>xy);
	r=CalculateLinePointMinMaxRadius(thickness,stroke,strokeStart,strokeEnd);
	if ((maxX+r)*ScaleX+OriginX<=ClipX1) return;
	if ((minX-r)*ScaleX+OriginX>=ClipX2) return;
	if ((maxY+r)*ScaleY+OriginY<=ClipY1) return;
	if ((minY-r)*ScaleY+OriginY>=ClipY2) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	nx1=nx2=1.0;
	ny1=ny2=0.0;
	withArrows=false;
	if (strokeStart.IsDecorated()) {
		withArrows=true;
		for (pxy=xy+2, pxye=xy+2*n; pxy<pxye; pxy+=2) {
			dx=pxy[0]-xy[0];
			dy=pxy[1]-xy[1];
			ll=dx*dx+dy*dy;
			if (ll<=1E-280) continue;
			l=sqrt(ll);
			nx1=dx/l;
			ny1=dy/l;
			break;
		}
	}
	if (strokeEnd.IsDecorated()) {
		withArrows=true;
		pxye=xy+2*(n-1);
		for (pxy=pxye-2; pxy>=xy; pxy-=2) {
			dx=pxy[0]-pxye[0];
			dy=pxy[1]-pxye[1];
			ll=dx*dx+dy*dy;
			if (ll<=1E-280) continue;
			l=sqrt(ll);
			nx2=dx/l;
			ny2=dy/l;
			break;
		}
	}

	if (withArrows) {
		PaintPolylineWithArrows(
			xy,n,nx1,ny1,nx2,ny2,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
	else {
		PaintPolylineWithoutArrows(
			xy,n,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
}


void emPainter::PaintBezierLine(
	const double xy[], int n, double thickness, const emStroke & stroke,
	const emStrokeEnd & strokeStart, const emStrokeEnd & strokeEnd,
	emColor canvasColor
) const
{
	const double * pxy, * pxye;
	double * xyOut, * p, * pe;
	double autoMem[512*2];
	double minX,maxX,minY,maxY,r,cx1,cy1,cx2,cy2,t,dt,s,b,f,ll,l,dx,dy;
	double x1,y1,x2,y2,x3,y3,x4,y4,bx1,by1,bx2,by2,nx1,ny1,nx2,ny2;
	int nOut,xyOutSize,m,i;
	bool skip,withArrows;

	if (n<=0 || thickness<=0.0) return;
	if (strokeStart.Type==emStrokeEnd::NO_END) {
		n-=n%3;
	}
	else {
		n-=(n-1)%3;
	}
	if (n<3) {
		PaintPolyline(xy,1,thickness,stroke,strokeStart,strokeEnd,canvasColor);
		return;
	}

	minX=maxX=xy[0];
	minY=maxY=xy[1];
	pxy=xy+n*2-2;
	do {
		if      (maxX<pxy[0]) maxX=pxy[0];
		else if (minX>pxy[0]) minX=pxy[0];
		if      (maxY<pxy[1]) maxY=pxy[1];
		else if (minY>pxy[1]) minY=pxy[1];
		pxy-=2;
	} while (pxy>xy);
	r=CalculateLinePointMinMaxRadius(thickness,stroke,strokeStart,strokeEnd);
	cx1=(ClipX1-OriginX)/ScaleX-r;
	if (maxX<=cx1) return;
	cx2=(ClipX2-OriginX)/ScaleX+r;
	if (minX>=cx2) return;
	cy1=(ClipY1-OriginY)/ScaleY-r;
	if (maxY<=cy1) return;
	cy2=(ClipY2-OriginY)/ScaleY+r;
	if (minY>=cy2) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	xyOut=autoMem;
	xyOutSize=sizeof(autoMem)/sizeof(double);
	nOut=0;

	s=ScaleX+ScaleY;
	pxy=xy;
	i=n;
	do {
		x1=pxy[0]; y1=pxy[1];
		x2=pxy[2]; y2=pxy[3];
		x3=pxy[4]; y3=pxy[5];
		pxy+=6;
		i-=3;
		if (i<=0) pxy=xy;
		x4=pxy[0]; y4=pxy[1];

		skip=
			stroke.DashType==emStroke::SOLID && (
				(x1<=cx1 && x2<=cx1 && x3<=cx1 && x4<=cx1) ||
				(x1>=cx2 && x2>=cx2 && x3>=cx2 && x4>=cx2) ||
				(y1<=cy1 && y2<=cy1 && y3<=cy1 && y4<=cy1) ||
				(y1>=cy2 && y2>=cy2 && y3>=cy2 && y4>=cy2)
			)
		;

		x2-=x1; y2-=y1;
		x3-=x1; y3-=y1;
		x4-=x1; y4-=y1;

		m=1;
		switch (0) {
		default:
			if (skip) break;
			ll=x4*x4+y4*y4;
			if (ll>1E-280) {
				l=sqrt(ll);
				if ((fabs(x2*y4-y2*x4)+fabs(x3*y4-y3*x4))*s<=l*0.01) break;
			}
			else {
				dx=x3-x2; dy=y3-y2;
				l=sqrt(dx*dx+dy*dy);
				if (l*s<=0.01) break;
				if (fabs(x2*dy-y2*dx)*s<=l*0.01) break;
			}
			// b = average of second derivation at start and end.
			bx1=x3-2*x2;    by1=y3-2*y2;
			bx2=x2-2*x3+x4; by2=y2-2*y3+y4;
			b=(sqrt(bx1*bx1+by1*by1)+sqrt(bx2*bx2+by2*by2))*3.0;
			f=CircleQuality*sqrt((b*0.0228+thickness*0.04)*s);
			if (f>=500.0) m=500;
			else if (f>1.0) m=(int)(f+0.5);
		}

		dt=1.0/m;
		if (i==1) m++;

		while ((nOut+m)*2>xyOutSize) {
			xyOutSize*=2;
			if (xyOut==autoMem) {
				xyOut=(double*)malloc(xyOutSize*sizeof(double));
				memcpy(xyOut,autoMem,nOut*2*sizeof(double));
			}
			else {
				xyOut=(double*)realloc(xyOut,xyOutSize*sizeof(double));
			}
		}

		x2*=3.0; y2*=3.0;
		x3*=3.0; y3*=3.0;
		x4+=x2-x3; y4+=y2-y3;
		x3-=x2+x2; y3-=y2+y2;
		t=0.0;
		p=xyOut+nOut*2;
		pe=p+m*2;
		do {
			p[0] = x1 + t*(x2 + t*(x3 + t*x4));
			p[1] = y1 + t*(y2 + t*(y3 + t*y4));
			p+=2;
			t+=dt;
		} while (p<pe);
		nOut+=m;
	} while (i>=3);

	nx1=nx2=1.0;
	ny1=ny2=0.0;
	withArrows=false;
	if (strokeStart.IsDecorated()) {
		withArrows=true;
		for (pxy=xy+2, pxye=xy+2*n; pxy<pxye; pxy+=2) {
			dx=pxy[0]-xy[0];
			dy=pxy[1]-xy[1];
			ll=dx*dx+dy*dy;
			if (ll<=1E-280) continue;
			l=sqrt(ll);
			nx1=dx/l;
			ny1=dy/l;
			break;
		}
	}
	if (strokeEnd.IsDecorated()) {
		withArrows=true;
		pxye=xy+2*(n-1);
		for (pxy=pxye-2; pxy>=xy; pxy-=2) {
			dx=pxy[0]-pxye[0];
			dy=pxy[1]-pxye[1];
			ll=dx*dx+dy*dy;
			if (ll<=1E-280) continue;
			l=sqrt(ll);
			nx2=dx/l;
			ny2=dy/l;
			break;
		}
	}

	if (withArrows) {
		PaintPolylineWithArrowsAlterBuf(
			xyOut,nOut,nx1,ny1,nx2,ny2,thickness,stroke,strokeStart,strokeEnd,
			canvasColor
		);
	}
	else {
		PaintPolylineWithoutArrows(
			xyOut,nOut,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}

	if (xyOut!=autoMem) free(xyOut);
}


void emPainter::PaintEllipseArc(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, double thickness, const emStroke & stroke,
	const emStrokeEnd & strokeStart, const emStrokeEnd & strokeEnd,
	emColor canvasColor
) const
{
	double xy[257*2];
	double r,f,t2,cx,cy,rx,ry,nx1,ny1,nx2,ny2,tnx,tny,ll,l;
	double absRangeAngle;
	int i,n;
	bool withArrows;

	startAngle*=M_PI/180.0;
	rangeAngle*=M_PI/180.0;
	if (rangeAngle==0.0) return;
	absRangeAngle=fabs(rangeAngle);
	if (absRangeAngle>=2*M_PI) {
		PaintEllipseOutline(x,y,w,h,thickness,stroke,canvasColor);
		return;
	}
	if (thickness<=0.0) return;
	if (w<0.0) w=0.0;
	if (h<0.0) h=0.0;
	r=CalculateLinePointMinMaxRadius(thickness,stroke,strokeStart,strokeEnd);
	if ((x-r)*ScaleX+OriginX>=ClipX2) return;
	if ((x+w+r)*ScaleX+OriginX<=ClipX1) return;
	if ((y-r)*ScaleY+OriginY>=ClipY2) return;
	if ((y+h+r)*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	rx=w*0.5;
	ry=h*0.5;
	cx=x+rx;
	cy=y+ry;
	t2=thickness*0.5;
	f=CircleQuality*sqrt((rx+t2)*ScaleX+(ry+t2)*ScaleY);
	if (f>256.0) f=256.0;
	f=f*absRangeAngle/(2*M_PI);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=rangeAngle/n;
	n++;
	for (i=0; i<n; i++) {
		xy[i*2]=cos(startAngle+f*i)*rx+cx;
		xy[i*2+1]=sin(startAngle+f*i)*ry+cy;
	}

	nx1=nx2=1.0;
	ny1=ny2=0.0;
	withArrows=false;
	if (strokeStart.IsDecorated()) {
		withArrows=true;
		nx1=-sin(startAngle);
		ny1=cos(startAngle);
		if (rangeAngle<0.0) { nx1=-nx1; ny1=-ny1; }
		tnx=nx1*rx;
		tny=ny1*ry;
		ll=tnx*tnx+tny*tny;
		if (ll>1E-280) {
			l=sqrt(ll);
			nx1=tnx/l;
			ny1=tny/l;
		}
	}
	if (strokeEnd.IsDecorated()) {
		withArrows=true;
		nx2=sin(startAngle+rangeAngle);
		ny2=-cos(startAngle+rangeAngle);
		if (rangeAngle<0.0) { nx2=-nx2; ny2=-ny2; }
		tnx=nx2*rx;
		tny=ny2*ry;
		ll=tnx*tnx+tny*tny;
		if (ll>1E-280) {
			l=sqrt(ll);
			nx2=tnx/l;
			ny2=tny/l;
		}
	}

	if (w<thickness || h<thickness) canvasColor=0;

	if (withArrows) {
		PaintPolylineWithArrowsAlterBuf(
			xy,n,nx1,ny1,nx2,ny2,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
	else {
		PaintPolylineWithoutArrows(
			xy,n,thickness,stroke,strokeStart,strokeEnd,canvasColor
		);
	}
}


void emPainter::PaintRectOutline(
	double x, double y, double w, double h, double thickness,
	const emStroke & stroke, emColor canvasColor
) const
{
	double xy[10*2];
	double x1,y1,x2,y2,t2;

	if (thickness<=0.0) return;
	if (w<0.0) w=0.0;
	if (h<0.0) h=0.0;
	t2=thickness*0.5;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	if (stroke.Rounded || stroke.DashType!=emStroke::SOLID) {
		if ((w<=thickness || h<=thickness) && stroke.DashType==emStroke::SOLID) {
			PaintRoundRect(x1,y1,x2-x1,y2-y1,t2,t2,stroke.Color,canvasColor);
			return;
		}
		xy[0]=x;   xy[1]=y;
		xy[2]=x+w; xy[3]=y;
		xy[4]=x+w; xy[5]=y+h;
		xy[6]=x;   xy[7]=y+h;
		if (w<thickness || h<thickness) canvasColor=0;
		PaintPolylineWithoutArrows(xy,4,thickness,stroke,NoEnd,NoEnd,canvasColor);
		return;
	}

	xy[0]=x1; xy[1]=y1;
	xy[2]=x2; xy[3]=y1;
	xy[4]=x2; xy[5]=y2;
	xy[6]=x1; xy[7]=y2;
	x1+=thickness; y1+=thickness;
	x2-=thickness; y2-=thickness;
	if (x1>=x2 || y1>=y2) {
		PaintPolygon(xy,4,stroke.Color,canvasColor);
		return;
	}
	xy[ 8]=xy[0]; xy[ 9]=xy[1];
	xy[10]=x1;    xy[11]=y1;
	xy[12]=x1;    xy[13]=y2;
	xy[14]=x2;    xy[15]=y2;
	xy[16]=x2;    xy[17]=y1;
	xy[18]=x1;    xy[19]=y1;
	PaintPolygon(xy,10,stroke.Color,canvasColor);
}


void emPainter::PaintEllipseOutline(
	double x, double y, double w, double h, double thickness,
	const emStroke & stroke, emColor canvasColor
) const
{
	double xy[514*2];
	double x1,y1,x2,y2,f,t2,cx,cy,rx,ry;
	int i,n,m;

	if (thickness<=0.0) return;
	if (w<0.0) w=0.0;
	if (h<0.0) h=0.0;
	t2=thickness*0.5;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	cx=(x1+x2)*0.5;
	cy=(y1+y2)*0.5;
	rx=x2-cx;
	ry=y2-cy;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=2*M_PI/n;

	if (stroke.DashType!=emStroke::SOLID) {
		rx-=t2;
		ry-=t2;
		for (i=0; i<n; i++) {
			xy[i*2]=cos(f*i)*rx+cx;
			xy[i*2+1]=sin(f*i)*ry+cy;
		}
		if (w<thickness || h<thickness) canvasColor=0;
		PaintPolylineWithoutArrows(xy,n,thickness,stroke,NoEnd,NoEnd,canvasColor);
		return;
	}

	for (i=0; i<n; i++) {
		xy[i*2]=cos(f*i)*rx+cx;
		xy[i*2+1]=sin(f*i)*ry+cy;
	}
	rx-=thickness;
	ry-=thickness;
	if (rx<=0.0 || ry<=0.0) {
		PaintPolygon(xy,n,stroke.Color,canvasColor);
		return;
	}
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
	PaintPolygon(xy,n+m+2,stroke.Color,canvasColor);
}


void emPainter::PaintEllipseSectorOutline(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, double thickness, const emStroke & stroke,
	emColor canvasColor
) const
{
	double xy[258*2];
	double * p;
	double r,f,t2,cx,cy,rx,ry;
	int i,n;

	startAngle*=M_PI/180.0;
	rangeAngle*=M_PI/180.0;
	if (rangeAngle<=0.0) {
		if (rangeAngle==0.0) return;
		startAngle+=rangeAngle;
		rangeAngle=-rangeAngle;
	}
	if (rangeAngle>=2*M_PI) {
		PaintEllipseOutline(x,y,w,h,thickness,stroke,canvasColor);
		return;
	}
	if (thickness<=0.0) return;
	if (w<0.0) w=0.0;
	if (h<0.0) h=0.0;
	r=CalculateLinePointMinMaxRadius(thickness,stroke,NoEnd,NoEnd);
	if ((x-r)*ScaleX+OriginX>=ClipX2) return;
	if ((x+w+r)*ScaleX+OriginX<=ClipX1) return;
	if ((y-r)*ScaleY+OriginY>=ClipY2) return;
	if ((y+h+r)*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	rx=w*0.5;
	ry=h*0.5;
	cx=x+rx;
	cy=y+ry;
	t2=thickness*0.5;
	f=CircleQuality*sqrt((rx+t2)*ScaleX+(ry+t2)*ScaleY);
	if (f>256.0) f=256.0;
	f=f*rangeAngle/(2*M_PI);
	if (f<=3.0) n=3;
	else if (f>=256.0) n=256;
	else n=(int)(f+0.5);
	f=rangeAngle/n;
	p=xy;
	*p++=cx;
	*p++=cy;
	for (i=0; i<=n; i++) {
		*p++=cos(startAngle+f*i)*rx+cx;
		*p++=sin(startAngle+f*i)*ry+cy;
	}
	if (w<thickness || h<thickness) canvasColor=0;
	PaintPolylineWithoutArrows(
		xy,n+2,thickness,stroke,NoEnd,NoEnd,canvasColor
	);
}


void emPainter::PaintRoundRectOutline(
	double x, double y, double w, double h, double rx, double ry,
	double thickness, const emStroke & stroke, emColor canvasColor
) const
{
	double xy[522*2];
	double x1,y1,x2,y2,f,t2,dx,dy;
	int i,m,n;

	if (thickness<=0.0) return;
	if (w<0.0) w=0.0;
	if (h<0.0) h=0.0;
	t2=thickness*0.5;
	x1=x-t2;
	if (x1*ScaleX+OriginX>=ClipX2) return;
	x2=x+w+t2;
	if (x2*ScaleX+OriginX<=ClipX1) return;
	y1=y-t2;
	if (y1*ScaleY+OriginY>=ClipY2) return;
	y2=y+h+t2;
	if (y2*ScaleY+OriginY<=ClipY1) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

	if (rx>w*0.5) rx=w*0.5;
	if (ry>h*0.5) ry=h*0.5;
	if (rx<=0.0 || ry<=0.0) {
		PaintRectOutline(x,y,w,h,thickness,stroke,canvasColor);
		return;
	}

	rx+=t2; ry+=t2;
	f=CircleQuality*sqrt(rx*ScaleX+ry*ScaleY);
	if (f>256.0) f=256.0;
	f*=0.25;
	if (f<=1.0) n=1;
	else if (f>=64.0) n=64;
	else n=(int)(f+0.5);
	f=0.5*M_PI/n;

	x1+=rx; y1+=ry;
	x2-=rx; y2-=ry;

	if (stroke.DashType!=emStroke::SOLID) {
		rx-=t2; ry-=t2;
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
		if (w<thickness || h<thickness) canvasColor=0;
		PaintPolylineWithoutArrows(xy,4*n+4,thickness,stroke,NoEnd,NoEnd,canvasColor);
		return;
	}

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
		PaintPolygon(xy,4*n+4,stroke.Color,canvasColor);
		return;
	}
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
	PaintPolygon(xy,4*n+4*m+10,stroke.Color,canvasColor);
}


void emPainter::PaintBorderImage(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, int srcX, int srcY, int srcW, int srcH,
	int srcL, int srcT, int srcR, int srcB, int alpha, emColor canvasColor,
	int whichSubRects
) const
{
	double f;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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
}


void emPainter::PaintBorderImageColored(
	double x, double y, double w, double h, double l, double t, double r,
	double b, const emImage & img, int srcX, int srcY, int srcW, int srcH,
	int srcL, int srcT, int srcR, int srcB, emColor color1, emColor color2,
	emColor canvasColor, int whichSubRects
) const
{
	double f;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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
}


void emPainter::PaintText(
	double x, double y, const char * text, double charHeight,
	double widthScale, emColor color, emColor canvasColor,
	int textLen
) const
{
	double charWidth,showHeight,rcw,cx1,cx2,x1;
	int i,n,c,imgX,imgY,imgW,imgH;
	emImage * pImg;

	if (
		y*ScaleY+OriginY>=ClipY2 ||
		(y+charHeight)*ScaleY+OriginY<=ClipY1 ||
		x>=(cx2=(ClipX2-OriginX)/ScaleX) ||
		ClipX1>=ClipX2 ||
		charHeight*ScaleY<=0.1 ||
		widthScale<=0.0
	) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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

	ch=maxCharHeight;
	tw=GetTextSize(text,ch,formatted,relLineSpace,&th,textLen);
	if (tw<=0.0) return;

	UserSpaceLeaveGuard userSpaceLeaveGuard(*this);

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


void emPainter::PaintEllipse(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, const emTexture & texture, emColor canvasColor
) const
{
	PaintEllipseSector(
		x,y,w,h,startAngle,rangeAngle,texture,canvasColor
	);
}


void emPainter::PaintLine(
	double x1, double y1, double x2, double y2, double thickness,
	LineCap cap1, LineCap cap2, emColor color, emColor canvasColor
) const
{
	PaintLine(
		x1,y1,x2,y2,thickness,
		emStroke(color,cap1==LC_ROUND || cap2==LC_ROUND),
		cap1==LC_FLAT ? ButtEnd : CapEnd,
		cap1==LC_FLAT ? ButtEnd : CapEnd,
		canvasColor
	);
}


void emPainter::PaintEllipseOutline(
	double x, double y, double w, double h, double startAngle,
	double rangeAngle, double thickness, emColor color,
	emColor canvasColor
) const
{
	PaintEllipseArc(
		x,y,w,h,startAngle,rangeAngle,thickness,color,
		ButtEnd,ButtEnd,canvasColor
	);
}


void emPainter::PaintPolylineWithArrows(
	const double xy[], int n, double nx1, double ny1, double nx2, double ny2,
	double thickness, const emStroke & stroke, const emStrokeEnd & strokeStart,
	const emStrokeEnd & strokeEnd, emColor canvasColor
) const
{
	double * xyOut;
	double autoMem[512*2];

	xyOut=autoMem;
	if (n*2*sizeof(double)>(int)sizeof(autoMem)) {
		xyOut=(double*)malloc(n*2*sizeof(double));
	}
	memcpy(xyOut,xy,n*2*sizeof(double));

	PaintPolylineWithArrowsAlterBuf(
		xyOut,n,nx1,ny1,nx2,ny2,thickness,stroke,strokeStart,
		strokeEnd,canvasColor
	);

	if (xyOut!=autoMem) free(xyOut);
}


void emPainter::PaintPolylineWithArrowsAlterBuf(
	double xy[], int n, double nx1, double ny1, double nx2, double ny2,
	double thickness, const emStroke & stroke, const emStrokeEnd & strokeStart,
	const emStrokeEnd & strokeEnd, emColor canvasColor
) const
{
	double * p1, * p2;
	double x1,y1,x2,y2,ex1,ey1,ex2,ey2,t;
	bool haveStartArrow,haveEndArrow;

	if (n<=0) return;

	p1=xy;
	p2=xy+(n-1)*2;

	if (strokeStart.IsDecorated()) {
		haveStartArrow=true;
		x1=p1[0];
		y1=p1[1];
		for (; p1<p2; p1+=2) {
			ex1=p1[0]-x1;
			ey1=p1[1]-y1;
			ex2=p1[2]-x1;
			ey2=p1[3]-y1;
			t=CutLineAtArrow(
				ex1*nx1+ey1*ny1,
				ey1*nx1-ex1*ny1,
				ex2*nx1+ey2*ny1,
				ey2*nx1-ex2*ny1,
				thickness,
				stroke,
				strokeStart
			);
			if (t<1.0) {
				p1[0]=(1.0-t)*p1[0]+t*p1[2];
				p1[1]=(1.0-t)*p1[1]+t*p1[3];
				break;
			}
		}
	}
	else {
		haveStartArrow=false;
		x1=y1=0.0;
	}

	if (strokeEnd.IsDecorated()) {
		haveEndArrow=true;
		x2=p2[0];
		y2=p2[1];
		for (; p2>p1; p2-=2) {
			ex1=p2[0]-x2;
			ey1=p2[1]-y2;
			ex2=p2[-2]-x2;
			ey2=p2[-1]-y2;
			t=CutLineAtArrow(
				ex1*nx2+ey1*ny2,
				ey1*nx2-ex1*ny2,
				ex2*nx2+ey2*ny2,
				ey2*nx2-ex2*ny2,
				thickness,
				stroke,
				strokeEnd
			);
			if (t<1.0) {
				p2[0]=(1.0-t)*p2[0]+t*p2[-2];
				p2[1]=(1.0-t)*p2[1]+t*p2[-1];
				break;
			}
		}
	}
	else {
		haveEndArrow=false;
		x2=y2=0.0;
	}

	n=(p2-p1)/2+1;
	PaintPolylineWithoutArrows(
		p1,n,thickness,stroke,strokeStart,strokeEnd,canvasColor
	);

	if (haveStartArrow) {
		PaintArrow(x1,y1,nx1,ny1,thickness,stroke,strokeStart,0);
	}
	if (haveEndArrow) {
		PaintArrow(x2,y2,nx2,ny2,thickness,stroke,strokeEnd,0);
	}
}


double emPainter::CalculateLinePointMinMaxRadius(
	double thickness, const emStroke & stroke, const emStrokeEnd & strokeStart,
	const emStrokeEnd & strokeEnd
) {
	double r,t,w,l;

	r=thickness*0.5;
	if (!stroke.Rounded) r*=emMax(1.415,MaxMiter);

	t=r;
	if (strokeStart.IsDecorated()) {
		w=strokeStart.WidthFactor*0.5;
		l=strokeStart.LengthFactor;
		r=t+t+sqrt(w*w+l*l)*thickness*ArrowBaseSize;
	}
	if (strokeEnd.IsDecorated()) {
		w=strokeEnd.WidthFactor*0.5;
		l=strokeEnd.LengthFactor;
		r=emMax(r,t+t+sqrt(w*w+l*l)*thickness*ArrowBaseSize);
	}

	return r;
}


double emPainter::CutLineAtArrow(
	double x1, double y1, double x2, double y2, double thickness,
	const emStroke & stroke, const emStrokeEnd & strokeEnd
)
{
	double r,l,l2,s,sinA,t,u,dx,dy,dr,d,d1,d2,b;
	bool semi;

	r=fabs(thickness*ArrowBaseSize*0.5*strokeEnd.WidthFactor);
	if (r<=1E-140) return 0.0;
	l=thickness*ArrowBaseSize*strokeEnd.LengthFactor;
	if (l<=1E-140) return 0.0;

	switch (strokeEnd.Type) {
		case emStrokeEnd::BUTT:
		case emStrokeEnd::CAP:
			break;
		case emStrokeEnd::ARROW:
			d=thickness*0.5;
			b=l/r;
			s=sqrt(1+b*b)*d;
			b*=ArrowNotch;
			u=sqrt(1+b*b)*d;
			l2=l-(s+u)/(1-ArrowNotch);
			r*=l2/l;
			l=l2;
			goto L_ARROW;
		case emStrokeEnd::CONTOUR_ARROW:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			goto L_ARROW;
		case emStrokeEnd::LINE_ARROW:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			l2=s*1.5;
			r*=l2/l;
			l=l2;
			s=0.0;
			goto L_TRIANGLE;
		case emStrokeEnd::TRIANGLE:
			d=thickness*0.5;
			b=l/r;
			s=sqrt(1+b*b)*d;
			l2=l-s-d;
			r*=l2/l;
			l=l2;
			goto L_TRIANGLE;
		case emStrokeEnd::CONTOUR_TRIANGLE:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			goto L_TRIANGLE;
		case emStrokeEnd::SQUARE:
			s=thickness*0.5;
			r=emMax(0.0,r-s);
			l=emMax(0.0,l-thickness);
			goto L_SQUARE;
		case emStrokeEnd::CONTOUR_SQUARE:
			s=thickness*0.5;
			goto L_SQUARE;
		case emStrokeEnd::HALF_SQUARE:
			s=thickness*0.5;
			l=emMax(l*0.5-s,thickness*0.0001);
			goto L_SQUARE;
		case emStrokeEnd::CIRCLE:
			s=thickness*0.5;
			r=emMax(0.0,r-s);
			l=emMax(0.0,l-thickness);
			semi=false;
			goto L_CIRCLE;
		case emStrokeEnd::CONTOUR_CIRCLE:
			s=thickness*0.5;
			semi=false;
			goto L_CIRCLE;
		case emStrokeEnd::HALF_CIRCLE:
			if (stroke.Rounded) s=thickness*0.5; else s=0.0;
			s-=l*0.5;
			semi=true;
			goto L_CIRCLE;
		case emStrokeEnd::DIAMOND:
			s=sqrt(r*r+l*l*0.25)/r*thickness*0.5;
			l2=l-s-s;
			r*=l2/l;
			l=l2;
			semi=false;
			goto L_DIAMOND;
		case emStrokeEnd::CONTOUR_DIAMOND:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l*0.25+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			semi=false;
			goto L_DIAMOND;
		case emStrokeEnd::HALF_DIAMOND:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l*0.25+r*r);
				s*=sinA+sqrt(1-sinA);
			}
			s-=l*0.5;
			semi=true;
			goto L_DIAMOND;
		case emStrokeEnd::STROKE:
			l=thickness*(fabs(strokeEnd.LengthFactor)-1.0);
			if (l<0.0) l=0.0;
			s=-l*0.5;
			goto L_SQUARE;
		case emStrokeEnd::NO_END:
			break;
	}
	return 0.0;

L_ARROW:
	x1-=s;
	x2-=s;
	dx=x2-x1;
	dy=y2-y1;
	dr=r/l;
	l2=(1.0-ArrowNotch)*l;
	d2=r/(l-l2);
	t=1.0;
	if (dy-d2*dx<-1E-140) {
		if (y1<=d2*(x1-l2)) t=0.0;
		else if (y2<(x2-l2)*d2) t=(d2*(x1-l2)-y1)/(dy-d2*dx);
	}
	u=1.0;
	if (dy+d2*dx>1E-140) {
		if (y1>=-d2*(x1-l2)) u=0.0;
		else if (y2>-(x2-l2)*d2) u=(-d2*(x1-l2)-y1)/(dy+d2*dx);
	}
	if (t<u) t=u;
	if (dy-dr*dx>1E-140) {
		if (y1>=dr*x1) return 0.0;
		if (y2>x2*dr) t=emMin(t,(dr*x1-y1)/(dy-dr*dx));
	}
	if (dy+dr*dx<-1E-140) {
		if (y1<=-dr*x1) return 0.0;
		if (y2<-x2*dr) t=emMin(t,(-dr*x1-y1)/(dy+dr*dx));
	}
	return t;

L_TRIANGLE:
	x1-=s;
	x2-=s;
	dx=x2-x1;
	dy=y2-y1;
	dr=r/l;
	t=1.0;
	if (dx>1E-140) {
		if (x1>=l) return 0.0;
		if (x2>l) t=(l-x1)/dx;
	}
	if (dy-dr*dx>1E-140) {
		if (y1>=dr*x1) return 0.0;
		if (y2>x2*dr) t=emMin(t,(dr*x1-y1)/(dy-dr*dx));
	}
	if (dy+dr*dx<-1E-140) {
		if (y1<=-dr*x1) return 0.0;
		if (y2<-x2*dr) t=emMin(t,(-dr*x1-y1)/(dy+dr*dx));
	}
	return t;

L_SQUARE:
	x1-=s;
	x2-=s;
	dx=x2-x1;
	dy=y2-y1;
	t=1.0;
	if (dx>1E-140) {
		if (x1>=l) return 0.0;
		if (x2>l) t=(l-x1)/dx;
	}
	else if (dx<-1E-140) {
		if (x1<=0.0) return 0.0;
		if (x2<0.0) t=-x1/dx;
	}
	if (dy>1E-140) {
		if (y1>=r) return 0.0;
		if (y2>r) t=emMin(t,(r-y1)/dy);
	}
	else if (dy<-1E-140) {
		if (y1<=-r) return 0.0;
		if (y2<-r) t=emMin(t,(-r-y1)/dy);
	}
	return t;

L_CIRCLE:
	x1=(x1-s)*2.0/l-1.0;
	x2=(x2-s)*2.0/l-1.0;
	y1/=r;
	y2/=r;
	dx=x2-x1;
	dy=y2-y1;
	d=dx*dx+dy*dy;
	if (d<=1E-140) return 1.0;
	d1=x1*x1+y1*y1;
	d2=x2*x2+y2*y2;
	u=(x1*dx+y1*dy)/d;
	t=(1.0-d1)/d+u*u;
	if (t<0.0) {
		if (d1<d2) return 0.0;
		return 1.0;
	}
	t=emMax(0.0,emMin(1.0,sqrt(t)-u));
	if (semi && dx<-1E-140) {
		if (x1<=0.0) return 0.0;
		if (x2<0.0) t=emMin(t,-x1/dx);
	}
	return t;

L_DIAMOND:
	x1-=s;
	x2-=s;
	dx=x2-x1;
	dy=y2-y1;
	dr=2.0*r/l;
	t=1.0;
	if (dy-dr*dx>1E-140) {
		if (y1>=dr*x1) return 0.0;
		if (y2>x2*dr) t=(dr*x1-y1)/(dy-dr*dx);
	}
	if (dy+dr*dx<-1E-140) {
		if (y1<=-dr*x1) return 0.0;
		if (y2<-x2*dr) t=emMin(t,(-dr*x1-y1)/(dy+dr*dx));
	}
	if (dy-dr*dx<-1E-140) {
		if (y1<=dr*(x1-l)) return 0.0;
		if (y2<(x2-l)*dr) t=emMin(t,(dr*(x1-l)-y1)/(dy-dr*dx));
	}
	if (dy+dr*dx>1E-140) {
		if (y1>=-dr*(x1-l)) return 0.0;
		if (y2>-(x2-l)*dr) t=emMin(t,(-dr*(x1-l)-y1)/(dy+dr*dx));
	}
	if (semi && dx<-1E-140) {
		if (x1<=l*0.5) return 0.0;
		if (x2<l*0.5) t=emMin(t,(l*0.5-x1)/dx);
	}
	return t;
}


void emPainter::PaintArrow(
	double x, double y, double nx, double ny, double thickness,
	const emStroke & stroke, const emStrokeEnd & strokeEnd, emColor canvasColor
) const
{
	static const double bc=4.0/3.0*tan(M_PI/8.0);
	double xy[12*2];
	double r,l,s,sinA;

	r=fabs(thickness*ArrowBaseSize*0.5*strokeEnd.WidthFactor);
	if (r<=1E-140) return;
	l=thickness*ArrowBaseSize*strokeEnd.LengthFactor;
	if (l<0.0) {
		l=-l;
		nx=-nx;
		ny=-ny;
	}
	if (l<=1E-140) return;

	emStroke arrowStroke=stroke;
	arrowStroke.DashType=emStroke::SOLID;

	switch (strokeEnd.Type) {
		case emStrokeEnd::BUTT:
		case emStrokeEnd::CAP:
			break;
		case emStrokeEnd::ARROW:
			xy[0]=x;
			xy[1]=y;
			xy[2]=x+l*nx+r*ny;
			xy[3]=y+l*ny-r*nx;
			xy[4]=x+(1.0-ArrowNotch)*l*nx;
			xy[5]=y+(1.0-ArrowNotch)*l*ny;
			xy[6]=x+l*nx-r*ny;
			xy[7]=y+l*ny+r*nx;
			PaintPolygon(xy,4,stroke.Color,canvasColor);
			break;
		case emStrokeEnd::CONTOUR_ARROW:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			xy[0]=x+s*nx;
			xy[1]=y+s*ny;
			xy[2]=x+(s+l)*nx+r*ny;
			xy[3]=y+(s+l)*ny-r*nx;
			xy[4]=x+(s+(1.0-ArrowNotch)*l)*nx;
			xy[5]=y+(s+(1.0-ArrowNotch)*l)*ny;
			xy[6]=x+(s+l)*nx-r*ny;
			xy[7]=y+(s+l)*ny+r*nx;
			PaintPolygon(xy,4,strokeEnd.InnerColor,canvasColor);
			PaintPolygonOutline(xy,4,thickness,arrowStroke);
			break;
		case emStrokeEnd::LINE_ARROW:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			xy[0]=x+(s+l)*nx-r*ny;
			xy[1]=y+(s+l)*ny+r*nx;
			xy[2]=x+s*nx;
			xy[3]=y+s*ny;
			xy[4]=x+(s+l)*nx+r*ny;
			xy[5]=y+(s+l)*ny-r*nx;
			PaintPolyline(xy,3,thickness,arrowStroke,CapEnd,CapEnd,canvasColor);
			break;
		case emStrokeEnd::TRIANGLE:
			xy[0]=x;
			xy[1]=y;
			xy[2]=x+l*nx+r*ny;
			xy[3]=y+l*ny-r*nx;
			xy[4]=x+l*nx-r*ny;
			xy[5]=y+l*ny+r*nx;
			PaintPolygon(xy,3,stroke.Color,canvasColor);
			break;
		case emStrokeEnd::CONTOUR_TRIANGLE:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			xy[0]=x+s*nx;
			xy[1]=y+s*ny;
			xy[2]=x+(s+l)*nx+r*ny;
			xy[3]=y+(s+l)*ny-r*nx;
			xy[4]=x+(s+l)*nx-r*ny;
			xy[5]=y+(s+l)*ny+r*nx;
			PaintPolygon(xy,3,strokeEnd.InnerColor,canvasColor);
			PaintPolygonOutline(xy,3,thickness,arrowStroke);
			break;
		case emStrokeEnd::SQUARE:
			xy[0]=x+r*ny;
			xy[1]=y-r*nx;
			xy[2]=x+l*nx+r*ny;
			xy[3]=y+l*ny-r*nx;
			xy[4]=x+l*nx-r*ny;
			xy[5]=y+l*ny+r*nx;
			xy[6]=x-r*ny;
			xy[7]=y+r*nx;
			PaintPolygon(xy,4,stroke.Color,canvasColor);
			break;
		case emStrokeEnd::CONTOUR_SQUARE:
			s=thickness*0.5;
			xy[0]=x+s*nx+r*ny;
			xy[1]=y+s*ny-r*nx;
			xy[2]=x+(s+l)*nx+r*ny;
			xy[3]=y+(s+l)*ny-r*nx;
			xy[4]=x+(s+l)*nx-r*ny;
			xy[5]=y+(s+l)*ny+r*nx;
			xy[6]=x+s*nx-r*ny;
			xy[7]=y+s*ny+r*nx;
			PaintPolygon(xy,4,strokeEnd.InnerColor,canvasColor);
			PaintPolygonOutline(xy,4,thickness,arrowStroke);
			break;
		case emStrokeEnd::HALF_SQUARE:
			s=thickness*0.5;
			l=emMax(l*0.5-s,thickness*0.0001);
			xy[0]=x+s*nx+r*ny;
			xy[1]=y+s*ny-r*nx;
			xy[2]=x+(s+l)*nx+r*ny;
			xy[3]=y+(s+l)*ny-r*nx;
			xy[4]=x+(s+l)*nx-r*ny;
			xy[5]=y+(s+l)*ny+r*nx;
			xy[6]=x+s*nx-r*ny;
			xy[7]=y+s*ny+r*nx;
			PaintPolyline(xy,4,thickness,arrowStroke,CapEnd,CapEnd,canvasColor);
			break;
		case emStrokeEnd::CIRCLE:
			xy[ 0]=x;
			xy[ 1]=y;
			xy[ 2]=x+bc*r*ny;
			xy[ 3]=y-bc*r*nx;
			xy[ 4]=x+(1.0-bc)*0.5*l*nx+r*ny;
			xy[ 5]=y+(1.0-bc)*0.5*l*ny-r*nx;
			xy[ 6]=x+0.5*l*nx+r*ny;
			xy[ 7]=y+0.5*l*ny-r*nx;
			xy[ 8]=x+(1.0+bc)*0.5*l*nx+r*ny;
			xy[ 9]=y+(1.0+bc)*0.5*l*ny-r*nx;
			xy[10]=x+l*nx+bc*r*ny;
			xy[11]=y+l*ny-bc*r*nx;
			xy[12]=x+l*nx;
			xy[13]=y+l*ny;
			xy[14]=x+l*nx-bc*r*ny;
			xy[15]=y+l*ny+bc*r*nx;
			xy[16]=x+(1.0+bc)*0.5*l*nx-r*ny;
			xy[17]=y+(1.0+bc)*0.5*l*ny+r*nx;
			xy[18]=x+0.5*l*nx-r*ny;
			xy[19]=y+0.5*l*ny+r*nx;
			xy[20]=x+(1.0-bc)*0.5*l*nx-r*ny;
			xy[21]=y+(1.0-bc)*0.5*l*ny+r*nx;
			xy[22]=x-bc*r*ny;
			xy[23]=y+bc*r*nx;
			PaintBezier(xy,12,stroke.Color,canvasColor);
			break;
		case emStrokeEnd::CONTOUR_CIRCLE:
			s=thickness*0.5;
			xy[ 0]=x+s*nx;
			xy[ 1]=y+s*ny;
			xy[ 2]=x+s*nx+bc*r*ny;
			xy[ 3]=y+s*ny-bc*r*nx;
			xy[ 4]=x+(s+(1.0-bc)*0.5*l)*nx+r*ny;
			xy[ 5]=y+(s+(1.0-bc)*0.5*l)*ny-r*nx;
			xy[ 6]=x+(s+0.5*l)*nx+r*ny;
			xy[ 7]=y+(s+0.5*l)*ny-r*nx;
			xy[ 8]=x+(s+(1.0+bc)*0.5*l)*nx+r*ny;
			xy[ 9]=y+(s+(1.0+bc)*0.5*l)*ny-r*nx;
			xy[10]=x+(s+l)*nx+bc*r*ny;
			xy[11]=y+(s+l)*ny-bc*r*nx;
			xy[12]=x+(s+l)*nx;
			xy[13]=y+(s+l)*ny;
			xy[14]=x+(s+l)*nx-bc*r*ny;
			xy[15]=y+(s+l)*ny+bc*r*nx;
			xy[16]=x+(s+(1.0+bc)*0.5*l)*nx-r*ny;
			xy[17]=y+(s+(1.0+bc)*0.5*l)*ny+r*nx;
			xy[18]=x+(s+0.5*l)*nx-r*ny;
			xy[19]=y+(s+0.5*l)*ny+r*nx;
			xy[20]=x+(s+(1.0-bc)*0.5*l)*nx-r*ny;
			xy[21]=y+(s+(1.0-bc)*0.5*l)*ny+r*nx;
			xy[22]=x+s*nx-bc*r*ny;
			xy[23]=y+s*ny+bc*r*nx;
			PaintBezier(xy,12,strokeEnd.InnerColor,canvasColor);
			PaintBezierOutline(xy,12,thickness,arrowStroke);
			break;
		case emStrokeEnd::HALF_CIRCLE:
			if (stroke.Rounded) s=thickness*0.5; else s=0.0;
			xy[ 0]=x+s*nx+r*ny;
			xy[ 1]=y+s*ny-r*nx;
			xy[ 2]=x+(s+bc*0.5*l)*nx+r*ny;
			xy[ 3]=y+(s+bc*0.5*l)*ny-r*nx;
			xy[ 4]=x+(s+0.5*l)*nx+bc*r*ny;
			xy[ 5]=y+(s+0.5*l)*ny-bc*r*nx;
			xy[ 6]=x+(s+0.5*l)*nx;
			xy[ 7]=y+(s+0.5*l)*ny;
			xy[ 8]=x+(s+0.5*l)*nx-bc*r*ny;
			xy[ 9]=y+(s+0.5*l)*ny+bc*r*nx;
			xy[10]=x+(s+bc*0.5*l)*nx-r*ny;
			xy[11]=y+(s+bc*0.5*l)*ny+r*nx;
			xy[12]=x+s*nx-r*ny;
			xy[13]=y+s*ny+r*nx;
			PaintBezierLine(
				xy,7,thickness,arrowStroke,
				stroke.Rounded ? CapEnd : ButtEnd,
				stroke.Rounded ? CapEnd : ButtEnd,
				canvasColor
			);
			break;
		case emStrokeEnd::DIAMOND:
			xy[0]=x;
			xy[1]=y;
			xy[2]=x+0.5*l*nx+r*ny;
			xy[3]=y+0.5*l*ny-r*nx;
			xy[4]=x+l*nx;
			xy[5]=y+l*ny;
			xy[6]=x+0.5*l*nx-r*ny;
			xy[7]=y+0.5*l*ny+r*nx;
			PaintPolygon(xy,4,stroke.Color,canvasColor);
			break;
		case emStrokeEnd::CONTOUR_DIAMOND:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l*0.25+r*r);
				if (MaxMiter*sinA<1.0) s*=sinA; else s/=sinA;
			}
			xy[0]=x+s*nx;
			xy[1]=y+s*ny;
			xy[2]=x+(s+0.5*l)*nx+r*ny;
			xy[3]=y+(s+0.5*l)*ny-r*nx;
			xy[4]=x+(s+l)*nx;
			xy[5]=y+(s+l)*ny;
			xy[6]=x+(s+0.5*l)*nx-r*ny;
			xy[7]=y+(s+0.5*l)*ny+r*nx;
			PaintPolygon(xy,4,strokeEnd.InnerColor,canvasColor);
			PaintPolygonOutline(xy,4,thickness,arrowStroke);
			break;
		case emStrokeEnd::HALF_DIAMOND:
			s=thickness*0.5;
			if (!stroke.Rounded) {
				sinA=r/sqrt(l*l*0.25+r*r);
				s*=sinA+sqrt(1-sinA);
			}
			xy[0]=x+s*nx+r*ny;
			xy[1]=y+s*ny-r*nx;
			xy[2]=x+(s+0.5*l)*nx;
			xy[3]=y+(s+0.5*l)*ny;
			xy[4]=x+s*nx-r*ny;
			xy[5]=y+s*ny+r*nx;
			PaintPolyline(xy,3,thickness,arrowStroke,CapEnd,CapEnd,canvasColor);
			break;
		case emStrokeEnd::STROKE:
			xy[0]=x+r*ny;
			xy[1]=y-r*nx;
			xy[2]=x-r*ny;
			xy[3]=y+r*nx;
			PaintPolyline(
				xy,2,
				thickness*fabs(strokeEnd.LengthFactor),
				arrowStroke,CapEnd,CapEnd,canvasColor
			);
			break;
		case emStrokeEnd::NO_END:
			break;
	}
}


void emPainter::PaintDashedPolyline(
	const double xy[], int n, double thickness, const emStroke & stroke,
	const emStrokeEnd & strokeStart, const emStrokeEnd & strokeEnd,
	emColor canvasColor
) const
{
	double autoMem[256*2];
	double * xyOut;
	double minDashLen,prefDashLen,dashLen,dotLen,prefGapLen,gapLen;
	double minPhaseLen,prefPhaseLen,phaseLen,totalLen,ll,l;
	double remainingSegmentLen,remainingEdgeLen,endExtra;
	double x,y,x1,y1,x2,y2,dx,dy,nx,ny,t,u;
	double minX,maxX,minY,maxY,r,cx1,cy1,cx2,cy2;
	int i,numEdges,nOut,xyOutSize,strokeCount,maxStrokeCount,strokeNumber;
	bool rounded,haveDashes,haveDots,haveDashesAndDots,isEndless;
	bool isInStroke,endOfStrokeReached;

	if (n<2) {
		PaintSolidPolyline(xy,n,thickness,stroke,strokeStart,strokeEnd,canvasColor);
		return;
	}

	rounded=stroke.Rounded;
	haveDashes=(stroke.DashType!=emStroke::DOTTED);
	haveDots=(stroke.DashType!=emStroke::DASHED);
	haveDashesAndDots=(haveDashes && haveDots);
	isEndless=(strokeStart.Type==emStrokeEnd::NO_END);

	if (haveDashes) {
		minDashLen=thickness*(rounded?1.0+MinRelSegLen:MinRelSegLen);
		prefDashLen=emMax(minDashLen,thickness*5.0*stroke.DashLengthFactor);
	}
	else {
		minDashLen=0.0;
		prefDashLen=0.0;
	}
	if (haveDots) dotLen=thickness*(1.0+MinRelSegLen); else dotLen=0.0;
	prefGapLen=emMax(0.0,thickness*5.0*stroke.GapLengthFactor);
	minPhaseLen=minDashLen+dotLen;
	prefPhaseLen=prefDashLen+dotLen+prefGapLen;

	numEdges=isEndless?n:n-1;
	totalLen=0.0;
	x2=xy[0];
	y2=xy[1];
	for (i=1; i<=numEdges; i++) {
		x1=x2;
		y1=y2;
		x2=xy[i%n*2];
		y2=xy[i%n*2+1];
		dx=x2-x1;
		dy=y2-y1;
		totalLen+=sqrt(dx*dx+dy*dy);
	}

	if (isEndless) {
		maxStrokeCount=(int)(emMin(MaxDashes,totalLen/minPhaseLen));
		if (maxStrokeCount<1) {
			PaintSolidPolyline(xy,n,thickness,stroke,strokeStart,strokeEnd,canvasColor);
			return;
		}
		strokeCount=(int)(emMin(MaxDashes,totalLen/prefPhaseLen+0.5));
		if (strokeCount<1) strokeCount=1;
		if (strokeCount>maxStrokeCount) strokeCount=maxStrokeCount;
		endExtra=0.0;
		t=totalLen/strokeCount-dotLen;
		dashLen=emMax(minDashLen,t/(prefPhaseLen-dotLen)*prefDashLen);
		gapLen=t-dashLen;
	}
	else {
		t=totalLen;
		if (haveDashes) t+=emMin(thickness,minDashLen); else t+=thickness;
		if (haveDashesAndDots) t+=dotLen;
		maxStrokeCount=(int)(emMin(MaxDashes,t/minPhaseLen));
		if (maxStrokeCount<2) {
			PaintSolidPolyline(xy,n,thickness,stroke,strokeStart,strokeEnd,canvasColor);
			return;
		}
		t=totalLen+prefGapLen;
		if (haveDashes) t+=emMin(thickness,prefDashLen); else t+=thickness;
		if (haveDashesAndDots) t+=dotLen;
		strokeCount=(int)(emMin(MaxDashes,t/prefPhaseLen+0.5));
		if (strokeCount<2) strokeCount=2;
		if (strokeCount>maxStrokeCount) strokeCount=maxStrokeCount;
		endExtra=thickness;
		if (haveDashes) {
			t=totalLen+endExtra;
			if (haveDots) t-=(strokeCount-1)*dotLen;
			u=strokeCount*prefDashLen+(strokeCount-1)*prefGapLen;
			dashLen=emMax(minDashLen,t/u*prefDashLen);
			if (dashLen<endExtra) {
				t-=endExtra;
				u-=prefDashLen;
				dashLen=emMax(minDashLen,t/u*prefDashLen);
				endExtra=dashLen;
			}
		}
		else {
			dashLen=0.0;
		}
		t=totalLen+endExtra-strokeCount*(dashLen+dotLen);
		if (haveDashesAndDots) t+=dotLen;
		gapLen=t/(strokeCount-1);
		endExtra*=0.5;
	}

	t=gapLen;
	if (rounded) t+=thickness*0.215;
	if (t*(ScaleX+ScaleY)*0.5<1.2) {
		phaseLen=dashLen+dotLen+gapLen;
		t=emMin(1.0,(phaseLen-t)/phaseLen);
		if (t<=0.0) return;
		emStroke stroke2(stroke);
		stroke2.Color.SetAlpha((emByte)(stroke.Color.GetAlpha()*t+0.5));
		PaintSolidPolyline(xy,n,thickness,stroke2,strokeStart,strokeEnd,canvasColor);
		return;
	}

	if (haveDashesAndDots) {
		gapLen*=0.5;
		strokeCount*=2;
		if (!isEndless) strokeCount--;
	}

	if (rounded) {
		endExtra=0.0;
		if (haveDashes) dashLen-=thickness;
		if (haveDots) dotLen-=thickness;
		gapLen+=thickness;
	}

	r=CalculateLinePointMinMaxRadius(thickness,stroke,CapEnd,CapEnd);
	cx1=(ClipX1-OriginX)/ScaleX-r;
	cy1=(ClipY1-OriginY)/ScaleY-r;
	cx2=(ClipX2-OriginX)/ScaleX+r;
	cy2=(ClipY2-OriginY)/ScaleY+r;

	isInStroke=false;
	endOfStrokeReached=false;
	strokeNumber=1;
	remainingSegmentLen=0.0;
	remainingEdgeLen=0.0;
	i=0;
	x1=0.0;
	y1=0.0;
	x2=xy[0];
	y2=xy[1];
	nx=1.0;
	ny=0.0;
	minX=minY=maxX=maxY=0.0;
	xyOut=autoMem;
	xyOutSize=sizeof(autoMem)/sizeof(double);
	nOut=0;
	if (isEndless) {
		x1=xy[n*2-2];
		y1=xy[n*2-1];
		dx=x2-x1;
		dy=y2-y1;
		ll=dx*dx+dy*dy;
		if (ll>1E-280) {
			l=sqrt(ll);
			remainingEdgeLen=emMin(l,(haveDashes?dashLen:dotLen)*0.5);
			nx=dx/l;
			ny=dy/l;
			i--;
		}
	}
	for (;;) {
		while (remainingEdgeLen<=1E-140 && i<numEdges) {
			i++;
			x1=x2;
			y1=y2;
			x2=xy[i%n*2];
			y2=xy[i%n*2+1];
			dx=x2-x1;
			dy=y2-y1;
			ll=dx*dx+dy*dy;
			l=sqrt(ll);
			remainingEdgeLen+=l;
			if (ll>1E-280) {
				nx=dx/l;
				ny=dy/l;
			}
		}

		if (remainingSegmentLen<remainingEdgeLen) {
			remainingEdgeLen-=remainingSegmentLen;
			remainingSegmentLen=0.0;
			endOfStrokeReached=true;
		}
		else {
			remainingSegmentLen-=remainingEdgeLen;
			remainingEdgeLen=0.0;
			if (i>=numEdges) {
				if (!isInStroke) break;
				endOfStrokeReached=true;
			}
			if (!isInStroke) continue;
		}

		x=x2-nx*remainingEdgeLen;
		y=y2-ny*remainingEdgeLen;
		if (nOut==0) {
			minX=maxX=x;
			minY=maxY=y;
		}
		else {
			if (minX>x) minX=x; else if (maxX<x) maxX=x;
			if (minY>y) minY=y; else if (maxY<y) maxY=y;
		}

		if ((nOut+1)*2>xyOutSize) {
			xyOutSize*=2;
			if (xyOut==autoMem) {
				xyOut=(double*)malloc(xyOutSize*sizeof(double));
				memcpy(xyOut,autoMem,nOut*2*sizeof(double));
			}
			else {
				xyOut=(double*)realloc(xyOut,xyOutSize*sizeof(double));
			}
		}
		xyOut[nOut*2]=x;
		xyOut[nOut*2+1]=y;
		nOut++;

		if (!isInStroke) {
			isInStroke=true;
			endOfStrokeReached=false;
			if (haveDashes && (!haveDots || (strokeNumber&1)!=0)) {
				remainingSegmentLen=dashLen;
			}
			else {
				remainingSegmentLen=dotLen;
			}
			if (strokeNumber==1) remainingSegmentLen-=endExtra;
			continue;
		}

		if (!endOfStrokeReached) continue;

		if (minX<cx2 && minY<cy2 && maxX>cx1 && maxY>cy1) {
			PaintSolidPolyline(
				xyOut,nOut,thickness,stroke,
				!isEndless && strokeNumber==1 ? strokeStart :
					rounded ? CapEnd : ButtEnd,
				!isEndless && strokeNumber==strokeCount ? strokeEnd :
					rounded ? CapEnd : ButtEnd,
				canvasColor
			);
		}

		if (strokeNumber>=strokeCount) break;
		strokeNumber++;
		isInStroke=false;
		remainingSegmentLen=gapLen;
		nOut=0;
	}
}


void emPainter::PaintSolidPolyline(
	const double xy[], int n, double thickness, const emStroke & stroke,
	const emStrokeEnd & strokeStart, const emStrokeEnd & strokeEnd,
	emColor canvasColor
) const
{
	enum {
		VTX_IS_START             = 1<<0,
		VTX_IS_END               = 1<<1,
		VTX_IS_NEAR_START_OR_END = 1<<2,
		VTX_DISALLOW_OUTER_MITER = 1<<3
	};

	struct Vertex {
		int Dir;      // 0=turn right, 1=turn left, -1=other
		int Flags;    // Combination of VTX_ flags
		double X,Y;   // Vertex coordinates
		double NX,NY; // Unit vector of outgoing edge
		double EL[2]; // Remaining length of right and left outgoing edge
		double NN;    // Scalar product of this NX,NY and previous NX,NY
		double MX,MY; // Miter vector
	};

	const double * pxy, * pxye;
	Vertex * vtx, * vLast, * v1, * v2, * e1, * e2;
	double * autoOutMem, * xyOut, * pOut;
	Vertex autoVtxAndOutMem[640];
	double minSegLen,x1,y1,x2,y2,dx,dy,ll,l,d,sd,a,s,c,f,mx,my,m,maxM,nm;
	int i,k,dir,nOut,midOut,xyOutSize,usedOut;
	emStrokeEnd::TypeEnum st;

	if (n<=0) return;

	if (n*sizeof(Vertex)<=sizeof(autoVtxAndOutMem)) {
		vtx=autoVtxAndOutMem;
		autoOutMem=(double*)(autoVtxAndOutMem+n);
		xyOutSize=(sizeof(autoVtxAndOutMem)-n*sizeof(Vertex))/sizeof(double);
	}
	else {
		vtx=(Vertex*)malloc(n*sizeof(Vertex));
		autoOutMem=(double*)autoVtxAndOutMem;
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wsizeof-array-div"
		xyOutSize=sizeof(autoVtxAndOutMem)/sizeof(double);
#		pragma GCC diagnostic pop
	}

	minSegLen=MinRelSegLen*thickness*1.01;
	pxy=xy;
	pxye=xy+2*n;
	vLast=vtx;
	x1=*pxy++;
	y1=*pxy++;
	while (pxy<pxye) {
		x2=*pxy++;
		y2=*pxy++;
		dx=x2-x1;
		dy=y2-y1;
		l=sqrt(dx*dx+dy*dy);
		if (
			l>=minSegLen || (
				l>1E-140 &&
				vLast==vtx &&
				pxy==pxye && (
					!stroke.Rounded ||
					strokeStart.Type!=emStrokeEnd::CAP ||
					strokeEnd.Type!=emStrokeEnd::CAP
				)
			)
		) {
			vLast->Flags=0;
			vLast->X=x1;
			vLast->Y=y1;
			vLast->NX=dx/l;
			vLast->NY=dy/l;
			vLast->EL[0]=l;
			vLast->EL[1]=l;
			vLast++;
			x1=x2;
			y1=y2;
		}
	}
	vLast->Flags=0;
	vLast->X=x1;
	vLast->Y=y1;
	vLast->NX=1.0;
	vLast->NY=0.0;
	vLast->EL[0]=0.0;
	vLast->EL[1]=0.0;

	if (strokeStart.Type==emStrokeEnd::NO_END) {
		x2=xy[0];
		y2=xy[1];
		for (;;) {
			dx=x2-x1;
			dy=y2-y1;
			ll=dx*dx+dy*dy;
			if (ll>1E-280) {
				l=sqrt(ll);
				vLast->NX=dx/l;
				vLast->NY=dy/l;
				vLast->EL[0]=l;
				vLast->EL[1]=l;
				break;
			}
			if (vLast==vtx) break;
			vLast--;
			x1=vLast->X;
			y1=vLast->Y;
		}
		v1=vLast;
		v2=vtx;
	}
	else {
		vtx->Flags=VTX_IS_START;
		vLast->Flags|=VTX_IS_END;
		vtx->Dir=-1;
		vLast->Dir=-1;
		v1=vLast-2;
		v2=vLast-1;
		if (v1>=vtx) {
			vtx[1].Flags=VTX_IS_NEAR_START_OR_END;
			v2->Flags=VTX_IS_NEAR_START_OR_END;
		}
	}

	d=thickness*0.5;
	if (v1>=vtx) {
		maxM=MaxMiter*d;
		do {
			mx=v1->NX-v2->NX;
			my=v1->NY-v2->NY;
			ll=mx*mx+my*my;
			if (ll<=1E-280) {
				v2->Dir=-1;
				continue;
			}
			l=sqrt(ll);
			mx/=l;
			my/=l;
			nm=v1->NX*mx+v1->NY*my;
			m=d/sqrt(emMax(1E-40,1.0-nm*nm));
			nm*=m;
			mx*=m;
			my*=m;
			v2->MX=mx;
			v2->MY=my;
			if (m>maxM) v2->Flags|=VTX_DISALLOW_OUTER_MITER;
			dir=(v1->NX*v2->NY-v1->NY*v2->NX<0.0 ? 1 : 0);
			v2->Dir=dir;
			v1->EL[dir]-=nm;
			v2->EL[dir]+=v2->NX*mx+v2->NY*my;
			v2->NN=v1->NX*v2->NX+v1->NY*v2->NY;
		} while (v2=v1, v1--, v1>=vtx);
	}

	xyOut=autoOutMem;
	pOut=xyOut;

	dir=0;
	sd=d;
	midOut=0;
	v1=e2=vtx;
	e1=vLast;
	for (;;) {
		usedOut=pOut-xyOut;
		if (usedOut+2*131>xyOutSize) {
			xyOutSize*=2; if (xyOutSize<1024*2) xyOutSize=1024*2;
			if (xyOut==autoOutMem) {
				xyOut=(double*)malloc(xyOutSize*sizeof(double));
				memcpy(xyOut,autoOutMem,usedOut*sizeof(double));
			}
			else {
				xyOut=(double*)realloc(xyOut,xyOutSize*sizeof(double));
			}
			pOut=xyOut+usedOut;
		}

		if (v1->Dir==dir) {
			if (e1->EL[dir]>0.0) {
				if (e2->EL[dir]>0.0) goto L_INNER_MITER;
			}
			else if (e2->EL[dir]<=0.0) {
				if (v1->NN<0.5) goto L_BEVEL; else goto L_NEXT;
			}
			if ((v1->Flags&VTX_IS_NEAR_START_OR_END)!=0 && v1->NN>=-0.5) {
				goto L_INNER_MITER;
			}
			goto L_BEVEL;
		}

		if (v1->Dir<0) {
			if ((v1->Flags&(VTX_IS_START|VTX_IS_END))!=0) {
				if (dir==0) {
					if ((v1->Flags&VTX_IS_END)==0) goto L_NEXT;
					st=strokeEnd.Type;
				}
				else {
					if ((v1->Flags&VTX_IS_START)==0) goto L_NEXT;
					st=strokeStart.Type;
				}
				if (st!=emStrokeEnd::CAP) goto L_BUTT;
				if (!stroke.Rounded) goto L_NRCAP;
				f=CircleQuality*sqrt(d*(ScaleX+ScaleY))*0.5;
				if (f<1.5) goto L_BUTT;
				a=M_PI;
				goto L_ROUND;
			}
			goto L_NEXT;
		}

		if (stroke.Rounded && v1->NN<1.0) {
			if (v1->NN>-1.0) a=acos(v1->NN); else a=M_PI;
			f=CircleQuality*sqrt(d*(ScaleX+ScaleY))*a/(2*M_PI);
			if (f>=0.5) {
				if (f<1.5) goto L_BEVEL;
				goto L_ROUND;
			}
		}

		if ((v1->Flags&VTX_DISALLOW_OUTER_MITER)==0) goto L_OUTER_MITER;
L_BEVEL:
		pOut[0]=v1->X-sd*e1->NY;
		pOut[1]=v1->Y+sd*e1->NX;
		pOut[2]=v1->X-sd*e2->NY;
		pOut[3]=v1->Y+sd*e2->NX;
		pOut+=4;
		goto L_NEXT;
L_BUTT:
		pOut[0]=v1->X-sd*e1->NY;
		pOut[1]=v1->Y+sd*e1->NX;
		pOut[2]=v1->X+sd*e1->NY;
		pOut[3]=v1->Y-sd*e1->NX;
		pOut+=4;
		goto L_NEXT;
L_NRCAP:
		pOut[0]=v1->X+sd*(e1->NX-e1->NY);
		pOut[1]=v1->Y+sd*(e1->NY+e1->NX);
		pOut[2]=v1->X+sd*(e1->NX+e1->NY);
		pOut[3]=v1->Y+sd*(e1->NY-e1->NX);
		pOut+=4;
		goto L_NEXT;
L_ROUND:
		if (f<=1.0) k=1;
		else if (f>=128.0) k=128;
		else k=(int)(f+0.5);
		f=a/k;
		for (i=0; i<=k; i++) {
			c=cos(f*i);
			s=sin(f*i);
			pOut[0]=v1->X+sd*(s*e1->NX-c*e1->NY);
			pOut[1]=v1->Y+sd*(s*e1->NY+c*e1->NX);
			pOut+=2;
		}
		goto L_NEXT;
L_OUTER_MITER:
		pOut[0]=v1->X+v1->MX;
		pOut[1]=v1->Y+v1->MY;
		pOut+=2;
		goto L_NEXT;
L_INNER_MITER:
		pOut[0]=v1->X-v1->MX;
		pOut[1]=v1->Y-v1->MY;
		pOut+=2;
L_NEXT:
		if (dir==0) {
			e1=e2;
			e2++;
			v1=e2;
			if (e2<=vLast) continue;
			dir=1;
			sd=-sd;
			midOut=pOut-xyOut;
			v1=e1=e2=vLast;
			if (vtx<vLast) e2--;
		}
		else {
			if (v1<=vtx) break;
			v1=e1=e2;
			e2--;
			if (e2<vtx) e2=vLast;
		}
	}

	if (
		strokeStart.Type==emStrokeEnd::NO_END &&
		midOut>0 && xyOut+midOut<pOut
	) {
		pOut[0]=xyOut[midOut];
		pOut[1]=xyOut[midOut+1];
		pOut[2]=xyOut[midOut-2];
		pOut[3]=xyOut[midOut-1];
		pOut+=4;
	}

	nOut=(pOut-xyOut)/2;

	if (vtx!=autoVtxAndOutMem) free(vtx);

	PaintPolygon(xyOut,nOut,stroke.Color,canvasColor);

	if (xyOut!=autoOutMem) free(xyOut);
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


const emStrokeEnd emPainter::ButtEnd(emStrokeEnd::BUTT);
const emStrokeEnd emPainter::CapEnd(emStrokeEnd::CAP);
const emStrokeEnd emPainter::NoEnd(emStrokeEnd::NO_END);
const double emPainter::CharBoxTallness=1.77;
const double emPainter::CircleQuality=4.5;
const double emPainter::MaxMiter=5.0;
const double emPainter::ArrowBaseSize=10.0;
const double emPainter::ArrowNotch=0.3;
const double emPainter::MaxDashes=1E5;
const double emPainter::MinRelSegLen=0.001;
