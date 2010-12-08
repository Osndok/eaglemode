//------------------------------------------------------------------------------
// emImage.cpp
//
// Copyright (C) 2001,2003-2010 Oliver Hamann.
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

#include <emCore/emImage.h>
#include <emCore/emPainter.h>


#ifdef EM_NO_DATA_EXPORT
emImage::emImage()
{
	Data=&EmptyData;
}
#endif


emImage::emImage(int width, int height, int channelCount)
{
	Data=&EmptyData;
	Setup(width,height,channelCount);
}


emImage & emImage::operator = (const emImage & img)
{
	img.Data->RefCount++;
	if (!--Data->RefCount) FreeData();
	Data=img.Data;
	if (Data->IsUsersMap) MakeWritable();
	return *this;
}


bool emImage::operator == (const emImage & image) const
{
	size_t mapSize;

	if (Data==image.Data) return true;
	if (Data->Width!=image.Data->Width) return false;
	if (Data->Height!=image.Data->Height) return false;
	if (Data->ChannelCount!=image.Data->ChannelCount) return false;
	if (Data->Map==image.Data->Map) return true;
	mapSize=Data->Width*Data->Height*Data->ChannelCount;
	if (!mapSize) return true;
	if (memcmp(Data->Map,image.Data->Map,mapSize)!=0) return false;
	return true;
}


void emImage::Setup(int width, int height, int channelCount)
{
	if (width<0) width=0;
	if (height<0) height=0;
	if (channelCount<1) channelCount=1;
	if (channelCount>4) channelCount=4;
	if (width!=Data->Width || height!=Data->Height ||
	    channelCount!=Data->ChannelCount) {
		if (!--Data->RefCount) FreeData();
		if (width==0 && height==0 && channelCount==1) {
			Data=&EmptyData;
		}
		else {
			Data=(SharedData*)malloc(sizeof(SharedData)+width*height*channelCount);
			Data->RefCount=1;
			Data->Width=width;
			Data->Height=height;
			Data->ChannelCount=(emByte)channelCount;
			Data->IsUsersMap=0;
			Data->Map=((emByte*)Data)+sizeof(SharedData);
		}
	}
}


void emImage::SetUserMap(int width, int height, int channelCount, emByte * map)
{
	if (width<0) width=0;
	if (height<0) height=0;
	if (channelCount<1) channelCount=1;
	if (channelCount>4) channelCount=4;
	if (!Data->IsUsersMap) {
		if (!--Data->RefCount) FreeData();
		Data=(SharedData*)malloc(sizeof(SharedData));
		Data->RefCount=1;
		Data->IsUsersMap=1;
	}
	Data->Width=width;
	Data->Height=height;
	Data->ChannelCount=(emByte)channelCount;
	Data->Map=map;
}


void emImage::TryParseXpm(
	const char * const * xpm, int channelCount
) throw(emString)
{
	char tmp[256];
	const char * s, * s2, * s3, * s4, * s5;
	emByte * p;
	emUInt32 * syms;
	emUInt32 sym;
	emColor * cols;
	emColor col;
	bool colFound;
	int width,height,num_colors,sym_size,n,i,j,k,x,y;

	syms=NULL;
	cols=NULL;

	s=*(xpm++);
	if (!s) goto L_Error;
	width=(int)strtol(s,(char**)&s,0);
	if (width<1) goto L_Error;
	height=(int)strtol(s,(char**)&s,0);
	if (height<1) goto L_Error;
	num_colors=(int)strtol(s,(char**)&s,0);
	if (num_colors<1) goto L_Error;
	sym_size=(int)strtol(s,(char**)&s,0);
	if (sym_size<1 || sym_size>4) goto L_Error;
	syms=new emUInt32[num_colors];
	cols=new emColor[num_colors];
	for (n=0; n<num_colors; n++) {
		s=*(xpm++);
		if (!s) goto L_Error;
		for (sym=0, i=0; i<sym_size; i++) {
			if (!s[i]) goto L_Error;
			sym=(sym<<8)|(emByte)s[i];
		}
		s+=i;
		while ((emByte)*s<=32 && *s) s++;
		s5=s+strlen(s);
		colFound=false;
		for (i=0; i<5 && !colFound; i++) {
			for (s2=s5; !colFound;) {
				s4=s2;
				for (;;) {
					s2--;
					if (s2<s) {
						j=-1;
						s3=NULL;
						break;
					}
					if (s2==s || (emByte)s2[-1]<=32) {
						if (*s2=='c' && (emByte)s2[1]<=32) {
							j=0;
							s3=s2+2;
							break;
						}
						if (*s2=='g' && (emByte)s2[1]<=32) {
							j=1;
							s3=s2+2;
							break;
						}
						if (*s2=='g' && s2[1]=='4' && (emByte)s2[2]<=32) {
							j=2;
							s3=s2+3;
							break;
						}
						if (*s2=='m' && (emByte)s2[1]<=32) {
							j=3;
							s3=s2+2;
							break;
						}
						if (*s2=='s' && (emByte)s2[1]<=32) {
							j=4;
							s3=s2+2;
							break;
						}
					}
				}
				if (j<0) break;
				if (i==j) {
					while ((emByte)*s3<=32 && *s3) s3++;
					while (s4>s3 && (emByte)s4[-1]<=32) s4--;
					k=s4-s3;
					if (k>0 && k+1<=(int)sizeof(tmp)) {
						memcpy(tmp,s3,k);
						tmp[k]=0;
						try {
							col.TryParse(tmp);
							colFound=true;
						}
						catch (emString) {
						}
					}
				}
			}
		}
		if (!colFound) goto L_Error;
		i=0;
		j=n;
		while (i<j) {
			k=(i+j)/2;
			if (syms[k]>sym) j=k;
			else i=k+1;
		}
		if (i<n) {
			memmove(syms+i+1,syms+i,sizeof(emUInt32)*(n-i));
			memmove(cols+i+1,cols+i,sizeof(emColor)*(n-i));
		}
		syms[i]=sym;
		cols[i]=col;
	}
	if (channelCount<1 || channelCount>4) {
		channelCount=1;
		for (i=0; i<num_colors; i++) {
			if (!cols[i].IsGrey()) { channelCount+=2; break; }
		}
		for (i=0; i<num_colors; i++) {
			if (cols[i].GetAlpha()!=255) { channelCount++; break; }
		}
	}
	Setup(width,height,channelCount);
	p=GetWritableMap();
	for (y=0; y<height; y++) {
		for (x=0, s=*(xpm++); x<width; x++, s+=sym_size) {
			if (!s) goto L_Error;
			for (i=0, sym=0; i<sym_size; i++) {
				if (!s[i]) goto L_Error;
				sym=(sym<<8)|(emByte)s[i];
			}
			i=0;
			j=num_colors;
			for (;;) {
				if (i>=j) goto L_Error;
				k=(i+j)/2;
				if (syms[k]>sym) j=k;
				else if (syms[k]<sym) i=k+1;
				else break;
			}
			switch (channelCount)
			{
			case 1:
				p[0]=cols[k].GetGrey();
				p++;
				break;
			case 2:
				p[0]=cols[k].GetGrey();
				p[1]=cols[k].GetAlpha();
				p+=2;
				break;
			case 3:
				p[0]=cols[k].GetRed();
				p[1]=cols[k].GetGreen();
				p[2]=cols[k].GetBlue();
				p+=3;
				break;
			default:
				p[0]=cols[k].GetRed();
				p[1]=cols[k].GetGreen();
				p[2]=cols[k].GetBlue();
				p[3]=cols[k].GetAlpha();
				p+=4;
				break;
			}
		}
	}
	delete [] cols;
	delete [] syms;
	return;

L_Error:
	if (cols) delete [] cols;
	if (syms) delete [] syms;
	Empty();
	throw emString("Unsupported XPM format");
}


void emImage::TryParseTga(
	const unsigned char * tgaData, int tgaSize, int channelCount
) throw(emString)
{
	const unsigned char * tgaEnd, * p;
	emColor * palette;
	emColor runCol;
	int runLen,i,x,y,c,w,h,idLen,palType,palBitsPP,bitsPP,desc,palSize;
	int alphaMask,imgType;

	palette=NULL;

	tgaEnd=tgaData+tgaSize;

	if (tgaData+17>=tgaEnd) goto L_Error;
	idLen=tgaData[0];
	palType=tgaData[1];
	imgType=tgaData[2];
	palSize=tgaData[5];
	palSize|=((int)tgaData[6])<<8;
	palBitsPP=tgaData[7];
	w=tgaData[12];
	w|=((int)tgaData[13])<<8;
	h=tgaData[14];
	h|=((int)tgaData[15])<<8;
	bitsPP=tgaData[16];
	desc=tgaData[17];
	tgaData+=18+idLen;

	alphaMask = (desc&0x0f)==0 ? 255 : 0;

	if ((imgType&~8)==1) {
		if (palType!=1) goto L_Error;
		if (bitsPP!=8 && bitsPP!=16) goto L_Error;
		palette=new emColor[palSize];
		for (i=0; i<palSize; i++) {
			if (palBitsPP==16) {
				if (tgaData+1>=tgaEnd) goto L_Error;
				c=*tgaData++;
				c|=((int)*tgaData++)<<8;
				palette[i].SetRed((emByte)((((c>>10)&31)*255)/31));
				palette[i].SetGreen((emByte)((((c>>5)&31)*255)/31));
				palette[i].SetBlue((emByte)(((c&31)*255)/31));
				palette[i].SetAlpha((emByte)((c&0x8000)?255:alphaMask));
			}
			else if (palBitsPP==24) {
				if (tgaData+2>=tgaEnd) goto L_Error;
				palette[i].SetBlue(*tgaData++);
				palette[i].SetGreen(*tgaData++);
				palette[i].SetRed(*tgaData++);
				palette[i].SetAlpha(255);
			}
			else if (palBitsPP==32) {
				if (tgaData+3>=tgaEnd) goto L_Error;
				palette[i].SetBlue(*tgaData++);
				palette[i].SetGreen(*tgaData++);
				palette[i].SetRed(*tgaData++);
				palette[i].SetAlpha((emByte)((*tgaData++)|alphaMask));
			}
			else goto L_Error;
		}
	}
	else if ((imgType&~8)==2) {
		if (palType!=0) goto L_Error;
		if (bitsPP!=16 && bitsPP!=24 && bitsPP!=32) goto L_Error;
	}
	else if ((imgType&~8)==3) {
		if (palType!=0) goto L_Error;
		if (bitsPP!=8 && bitsPP!=16) goto L_Error;
	}
	else goto L_Error;

	if (channelCount<1 || channelCount>4) {
		if (palette) {
			channelCount=1;
			for (i=0; i<palSize; i++) {
				if (channelCount<3 && !palette[i].IsGrey()) channelCount+=2;
				if ((channelCount&1)!=0 && palette[i].GetAlpha()!=255) channelCount+=1;
			}
		}
		else if ((imgType&~8)==3 && bitsPP==8) channelCount=1;
		else {
			channelCount=1;
			runCol=0;
			runLen=0;
			p=tgaData;
			for (x=h*w; x>0; x--) {
				if ((imgType&8)!=0 && runLen<0) runLen++;
				else {
					if ((imgType&8)!=0 && runLen==0) {
						if (p>=tgaEnd) goto L_Error;
						runLen=*p++;
						if (runLen&0x80) runLen=-(runLen&0x7f)+1;
						else runLen++;
					}
					runLen--;
					for (c=0, i=0; i<bitsPP; i+=8) {
						if (p>=tgaEnd) goto L_Error;
						c|=((int)*p++)<<i;
					}
					if ((imgType&~8)==2) {
						if (bitsPP==16) {
							runCol.SetRed((emByte)((((c>>10)&31)*255)/31));
							runCol.SetGreen((emByte)((((c>>5)&31)*255)/31));
							runCol.SetBlue((emByte)(((c&31)*255)/31));
							runCol.SetAlpha((emByte)((c&0x8000)?255:alphaMask));
						}
						else {
							runCol.SetRed((emByte)(c>>16));
							runCol.SetGreen((emByte)(c>>8));
							runCol.SetBlue((emByte)c);
							runCol.SetAlpha((emByte)(bitsPP==24 ? 255 : ((c>>24)|alphaMask)));
						}
					}
					else {
						runCol.SetRed((emByte)c);
						runCol.SetGreen((emByte)c);
						runCol.SetBlue((emByte)c);
						runCol.SetAlpha((emByte)(bitsPP==8 ? 255 : ((c>>8)|alphaMask)));
					}
					if (channelCount<3 && !runCol.IsGrey()) channelCount+=2;
					if ((channelCount&1)!=0 && runCol.GetAlpha()!=255) channelCount+=1;
				}
			}
		}
	}

	Setup(w,h,channelCount);
	runCol=0;
	runLen=0;
	p=tgaData;
	for (y=0; y<h; y++) {
		for (x=0; x<w; x++) {
			if ((imgType&8)!=0 && runLen<0) runLen++;
			else {
				if ((imgType&8)!=0 && runLen==0) {
					if (p>=tgaEnd) goto L_Error;
					runLen=*p++;
					if (runLen&0x80) runLen=-(runLen&0x7f)+1;
					else runLen++;
				}
				runLen--;
				for (c=0, i=0; i<bitsPP; i+=8) {
					if (p>=tgaEnd) goto L_Error;
					c|=((int)*p++)<<i;
				}
				if ((imgType&~8)==1) {
					if (c<0 || c>=palSize) goto L_Error;
					runCol=palette[c];
				}
				else if ((imgType&~8)==2) {
					if (bitsPP==16) {
						runCol.SetRed((emByte)((((c>>10)&31)*255)/31));
						runCol.SetGreen((emByte)((((c>>5)&31)*255)/31));
						runCol.SetBlue((emByte)(((c&31)*255)/31));
						runCol.SetAlpha((emByte)((c&0x8000)?255:alphaMask));
					}
					else {
						runCol.SetRed((emByte)(c>>16));
						runCol.SetGreen((emByte)(c>>8));
						runCol.SetBlue((emByte)c);
						runCol.SetAlpha((emByte)(bitsPP==24 ? 255 : ((c>>24)|alphaMask)));
					}
				}
				else {
					runCol.SetRed((emByte)c);
					runCol.SetGreen((emByte)c);
					runCol.SetBlue((emByte)c);
					runCol.SetAlpha((emByte)(bitsPP==8 ? 255 : ((c>>8)|alphaMask)));
				}
			}
			SetPixel(x,(desc&0x20)?y:h-y-1,runCol);
		}
	}
	if (palette) delete [] palette;
	return;

L_Error:
	if (palette) delete [] palette;
	Empty();
	throw emString("Unsupported TGA format");
}


#ifdef EM_NO_DATA_EXPORT
void emImage::Empty()
{
	if (!--Data->RefCount) FreeData();
	Data=&EmptyData;
}
#endif


emColor emImage::GetPixel(int x, int y) const
{
	const emByte * p;

	if (
		(unsigned)x<(unsigned)Data->Width &&
		(unsigned)y<(unsigned)Data->Height
	) {
		switch (Data->ChannelCount)
		{
		case 1:
			p=Data->Map+y*Data->Width+x;
			return emColor(p[0],p[0],p[0]);
		case 2:
			p=Data->Map+(y*Data->Width+x)*2;
			return emColor(p[0],p[0],p[0],p[1]);
		case 3:
			p=Data->Map+(y*Data->Width+x)*3;
			return emColor(p[0],p[1],p[2]);
		default:
			p=Data->Map+(y*Data->Width+x)*4;
			return emColor(p[0],p[1],p[2],p[3]);
		}
	}
	else {
		return emColor(0);
	}
}


void emImage::SetPixel(int x, int y, emColor color)
{
	emByte * p;

	if (
		(unsigned)x<(unsigned)Data->Width &&
		(unsigned)y<(unsigned)Data->Height
	) {
		if (Data->RefCount>1) MakeWritable();
		switch (Data->ChannelCount)
		{
		case 1:
			p=Data->Map+y*Data->Width+x;
			p[0]=color.GetGrey();
			break;
		case 2:
			p=Data->Map+(y*Data->Width+x)*2;
			p[0]=color.GetGrey();
			p[1]=color.GetAlpha();
			break;
		case 3:
			p=Data->Map+(y*Data->Width+x)*3;
			p[0]=color.GetRed();
			p[1]=color.GetGreen();
			p[2]=color.GetBlue();
			break;
		default:
			p=Data->Map+(y*Data->Width+x)*4;
			p[0]=color.GetRed();
			p[1]=color.GetGreen();
			p[2]=color.GetBlue();
			p[3]=color.GetAlpha();
		}
	}
}


emByte emImage::GetPixelChannel(int x, int y, int channel) const
{
	if (
		(unsigned)x<(unsigned)Data->Width &&
		(unsigned)y<(unsigned)Data->Height &&
		(unsigned)channel<(unsigned)Data->ChannelCount
	) {
		return Data->Map[(y*Data->Width+x)*Data->ChannelCount+channel];
	}
	else {
		return 0;
	}
}


void emImage::SetPixelChannel(int x, int y, int channel, emByte value)
{
	if (
		(unsigned)x<(unsigned)Data->Width &&
		(unsigned)y<(unsigned)Data->Height &&
		(unsigned)channel<(unsigned)Data->ChannelCount
	) {
		if (Data->RefCount>1) MakeWritable();
		Data->Map[(y*Data->Width+x)*Data->ChannelCount+channel]=value;
	}
}


emColor emImage::GetPixelInterpolated(
	double x, double y, double w, double h, emColor bgColor
) const
{
	const emByte * py, * p;
	double x2,y2,xm,ym,xn,yn,rh,rw,fy;
	int irw,ifx,r,g,b,a;

	if (h<1.0) {
		y=(y*2+h-1.0)*0.5;
		h=1.0;
	}
	if (w<1.0) {
		x=(x*2+w-1.0)*0.5;
		w=1.0;
	}
	x2=x+w;
	y2=y+h;
	r=0x8000;
	g=0x8000;
	b=0x8000;
	a=0x8000;
	ym=floor(y);
	yn=ym+1.0;
	rh=65536.0/h;
	fy=(yn-y)*rh;
	for (;;) {
		if (ym<0.0 || ym>=Data->Height) {
			ifx=(int)fy;
			r+=bgColor.GetRed  ()*ifx;
			g+=bgColor.GetGreen()*ifx;
			b+=bgColor.GetBlue ()*ifx;
			a+=bgColor.GetAlpha()*ifx;
		}
		else {
			py=Data->Map+((int)ym)*Data->Width*Data->ChannelCount;
			xm=floor(x);
			xn=xm+1.0;
			rw=fy/w;
			ifx=(int)((xn-x)*rw);
			irw=(int)rw;
			for (;;) {
				if (xm<0.0 || xm>=Data->Width) {
					r+=bgColor.GetRed  ()*ifx;
					g+=bgColor.GetGreen()*ifx;
					b+=bgColor.GetBlue ()*ifx;
					a+=bgColor.GetAlpha()*ifx;
				}
				else {
					switch (Data->ChannelCount)
					{
					case 1:
						p=py+((int)xm);
						r+=p[0]*ifx;
						g+=p[0]*ifx;
						b+=p[0]*ifx;
						a+=255*ifx;
						break;
					case 2:
						p=py+((int)xm)*2;
						r+=p[0]*ifx;
						g+=p[0]*ifx;
						b+=p[0]*ifx;
						a+=p[1]*ifx;
						break;
					case 3:
						p=py+((int)xm)*3;
						r+=p[0]*ifx;
						g+=p[1]*ifx;
						b+=p[2]*ifx;
						a+=255*ifx;
						break;
					default:
						p=py+((int)xm)*4;
						r+=p[0]*ifx;
						g+=p[1]*ifx;
						b+=p[2]*ifx;
						a+=p[3]*ifx;
						break;
					}
				}
				xm=xn;
				xn+=1.0;
				ifx=irw;
				if (xn<=x2) continue;
				if (xm>=x2) break;
				ifx=(int)((x2-xm)*rw);
			}
		}
		ym=yn;
		yn+=1.0;
		fy=rh;
		if (yn<=y2) continue;
		if (ym>=y2) break;
		fy*=y2-ym;
	}
	return emColor(
		(emByte)(r>>16),
		(emByte)(g>>16),
		(emByte)(b>>16),
		(emByte)(a>>16)
	);
}


void emImage::Fill(int x, int y, int w, int h, emColor color)
{
	emByte * p, * pxe, * pye;
	union {
		emByte b[2];
		emUInt16 v;
	} v16buf;
	union {
		emByte b[4];
		emUInt32 v;
	} v32buf;
	emByte v8,r,g,b;
	emUInt16 v16;
	emUInt32 v32;
	int d1,d2;

	if (x<0) { w+=x; x=0; }
	if (w>Data->Width-x) w=Data->Width-x;
	if (w<=0) return;
	if (y<0) { h+=y; y=0; }
	if (h>Data->Height-y) h=Data->Height-y;
	if (h<=0) return;

	if (Data->RefCount>1) MakeWritable();

	switch (Data->ChannelCount)
	{
	case 1:
		d1=w;
		d2=Data->Width-w;
		p=Data->Map+y*Data->Width+x;
		pye=p+h*Data->Width;
		v8=color.GetGrey();
		do {
			pxe=p+d1;
			do {
				*p=v8;
				p++;
			} while (p<pxe);
			p+=d2;
		} while(p<pye);
		break;
	case 2:
		d1=w*2;
		d2=(Data->Width-w)*2;
		p=Data->Map+(y*Data->Width+x)*2;
		pye=p+h*Data->Width*2;
		v16buf.b[0]=color.GetGrey();
		v16buf.b[1]=color.GetAlpha();
		v16=v16buf.v;
		do {
			pxe=p+d1;
			do {
				(*(emUInt16*)p)=v16;
				p+=2;
			} while (p<pxe);
			p+=d2;
		} while(p<pye);
		break;
	case 3:
		d1=w*3;
		d2=(Data->Width-w)*3;
		p=Data->Map+(y*Data->Width+x)*3;
		pye=p+h*Data->Width*3;
		r=color.GetRed();
		g=color.GetGreen();
		b=color.GetBlue();
		do {
			pxe=p+d1;
			do {
				p[0]=r;
				p[1]=g;
				p[2]=b;
				p+=3;
			} while (p<pxe);
			p+=d2;
		} while(p<pye);
		break;
	default:
		d1=w*4;
		d2=(Data->Width-w)*4;
		p=Data->Map+(y*Data->Width+x)*4;
		pye=p+h*Data->Width*4;
		v32buf.b[0]=color.GetRed();
		v32buf.b[1]=color.GetGreen();
		v32buf.b[2]=color.GetBlue();
		v32buf.b[3]=color.GetAlpha();
		v32=v32buf.v;
		do {
			pxe=p+d1;
			do {
				(*(emUInt32*)p)=v32;
				p+=4;
			} while (p<pxe);
			p+=d2;
		} while(p<pye);
	}
}


void emImage::FillChannel(
	int x, int y, int w, int h, int channel, emByte value
)
{
	emByte * p, * pxe, * pye;
	int d0, d1, d2;

	if ((unsigned)channel>=(unsigned)Data->ChannelCount) return;

	if (x<0) { w+=x; x=0; }
	if (w>Data->Width-x) w=Data->Width-x;
	if (w<=0) return;
	if (y<0) { h+=y; y=0; }
	if (h>Data->Height-y) h=Data->Height-y;
	if (h<=0) return;

	if (Data->RefCount>1) MakeWritable();

	d0=Data->ChannelCount;
	d1=w*d0;
	d2=(Data->Width-w)*d0;
	p=Data->Map+(y*Data->Width+x)*d0+channel;
	pye=p+h*Data->Width*d0;
	do {
		pxe=p+d1;
		do {
			*p=value;
			p+=d0;
		} while (p<pxe);
		p+=d2;
	} while(p<pye);
}


void emImage::Copy(
	int x, int y, const emImage & img, int srcX, int srcY,
	int w, int h
)
{
	emByte * t;
	const emByte * s, * sxe, * sye;
	int td2,sd1,sd2;

	if (x<0) { w+=x; srcX-=x; x=0; }
	if (srcX<0) { w+=srcX; x-=srcX; srcX=0; }
	if (w>img.Data->Width-srcX) w=img.Data->Width-srcX;
	if (w>Data->Width-x) w=Data->Width-x;
	if (w<=0) return;

	if (y<0) { h+=y; srcY-=y; y=0; }
	if (srcY<0) { h+=srcY; y-=srcY; srcY=0; }
	if (h>img.Data->Height-srcY) h=img.Data->Height-srcY;
	if (h>Data->Height-y) h=Data->Height-y;
	if (h<=0) return;

	if (
		Data->Width==w &&
		Data->Height==h &&
		img.Data->Width==w &&
		img.Data->Height==h &&
		Data->ChannelCount==img.Data->ChannelCount
	) {
		if (Data!=img.Data) {
			img.Data->RefCount++;
			if (!--Data->RefCount) FreeData();
			Data=img.Data;
			if (Data->IsUsersMap) MakeWritable();
		}
		return;
	}

	if (Data->RefCount>1) MakeWritable();

	t=Data->Map+(y*Data->Width+x)*Data->ChannelCount;
	td2=(Data->Width-w)*Data->ChannelCount;

	s=img.Data->Map+(srcY*img.Data->Width+srcX)*img.Data->ChannelCount;
	sye=s+h*img.Data->Width*img.Data->ChannelCount;
	sd1=w*img.Data->ChannelCount;
	sd2=(img.Data->Width-w)*img.Data->ChannelCount;

#	define EM_IMG_COPY_LOOP(TCC,SCC,OPERATION) \
		do { \
			sxe=s+sd1; \
			do { \
				OPERATION \
				t+=TCC; s+=SCC; \
			} while (s!=sxe); \
			t+=td2; s+=sd2; \
		} while(s!=sye);

	if (t>s && t<sye && Data->ChannelCount==img.Data->ChannelCount) {
		s+=((h-1)*img.Data->Width+w-1)*img.Data->ChannelCount;
		t+=((h-1)*Data->Width+w-1)*Data->ChannelCount;
		td2=-td2;
		sd1=-sd1;
		sd2=-sd2;
		sye=s+h*(sd1+sd2);
		switch (Data->ChannelCount) {
		case 1:
			EM_IMG_COPY_LOOP(-1,-1,
				*t=*s;
			)
			break;
		case 2:
			EM_IMG_COPY_LOOP(-2,-2,
				*((emUInt16*)t)=*((emUInt16*)s);
			)
			break;
		case 3:
			EM_IMG_COPY_LOOP(-3,-3,
				t[0]=s[0];
				t[1]=s[1];
				t[2]=s[2];
			)
			break;
		default:
			EM_IMG_COPY_LOOP(-4,-4,
				*((emUInt32*)t)=*((emUInt32*)s);
			)
		}
	}
	else {
		switch (Data->ChannelCount) {
		case 1:
			switch (img.Data->ChannelCount) {
			case 1:
				EM_IMG_COPY_LOOP(1,1,
					*t=*s;
				)
				break;
			case 2:
				EM_IMG_COPY_LOOP(1,2,
					*t=*s;
				)
				break;
			case 3:
				EM_IMG_COPY_LOOP(1,3,
					*t=(emByte)((((int)s[0])+s[1]+s[2]+1)/3);
				)
				break;
			default:
				EM_IMG_COPY_LOOP(1,4,
					*t=(emByte)((((int)s[0])+s[1]+s[2]+1)/3);
				)
			}
			break;
		case 2:
			switch (img.Data->ChannelCount) {
			case 1:
				EM_IMG_COPY_LOOP(2,1,
					t[0]=*s;
					t[1]=255;
				)
				break;
			case 2:
				EM_IMG_COPY_LOOP(2,2,
					*((emUInt16*)t)=*((emUInt16*)s);
				)
				break;
			case 3:
				EM_IMG_COPY_LOOP(2,3,
					t[0]=(emByte)((((int)s[0])+s[1]+s[2]+1)/3);
					t[1]=255;
				)
				break;
			default:
				EM_IMG_COPY_LOOP(2,4,
					t[0]=(emByte)((((int)s[0])+s[1]+s[2]+1)/3);
					t[1]=s[3];
				)
			}
			break;
		case 3:
			switch (img.Data->ChannelCount) {
			case 1:
				EM_IMG_COPY_LOOP(3,1,
					t[0]=t[1]=t[2]=*s;
				)
				break;
			case 2:
				EM_IMG_COPY_LOOP(3,2,
					t[0]=t[1]=t[2]=*s;
				)
				break;
			case 3:
				EM_IMG_COPY_LOOP(3,3,
					t[0]=s[0];
					t[1]=s[1];
					t[2]=s[2];
				)
				break;
			default:
				EM_IMG_COPY_LOOP(3,4,
					t[0]=s[0];
					t[1]=s[1];
					t[2]=s[2];
				)
			}
			break;
		default:
			switch (img.Data->ChannelCount) {
			case 1:
				EM_IMG_COPY_LOOP(4,1,
					t[0]=t[1]=t[2]=*s;
					t[3]=255;
				)
				break;
			case 2:
				EM_IMG_COPY_LOOP(4,2,
					t[0]=t[1]=t[2]=*s;
					t[3]=s[1];
				)
				break;
			case 3:
				EM_IMG_COPY_LOOP(4,3,
					t[0]=s[0];
					t[1]=s[1];
					t[2]=s[2];
					t[3]=255;
				)
				break;
			default:
				EM_IMG_COPY_LOOP(4,4,
					*((emUInt32*)t)=*((emUInt32*)s);
				)
			}
		}
	}
}


void emImage::CopyChannel(
	int x, int y, int channel, const emImage & img, int srcX, int srcY,
	int w, int h, int srcChannel
)
{
	emByte * t;
	const emByte * s, * sxe, * sye;
	int td0, td2, sd0, sd1, sd2;

	if ((unsigned)channel>=(unsigned)Data->ChannelCount) return;
	if ((unsigned)srcChannel>=(unsigned)img.Data->ChannelCount) return;

	if (x<0) { w+=x; srcX-=x; x=0; }
	if (srcX<0) { w+=srcX; x-=srcX; srcX=0; }
	if (w>img.Data->Width-srcX) w=img.Data->Width-srcX;
	if (w>Data->Width-x) w=Data->Width-x;
	if (w<=0) return;

	if (y<0) { h+=y; srcY-=y; y=0; }
	if (srcY<0) { h+=srcY; y-=srcY; srcY=0; }
	if (h>img.Data->Height-srcY) h=img.Data->Height-srcY;
	if (h>Data->Height-y) h=Data->Height-y;
	if (h<=0) return;

	if (Data->RefCount>1) MakeWritable();

	sd0=img.Data->ChannelCount;
	sd1=w*sd0;
	sd2=(img.Data->Width-w)*sd0;
	s=img.Data->Map+(srcY*img.Data->Width+srcX)*sd0+srcChannel;
	sye=s+h*(sd1+sd2);

	td0=Data->ChannelCount;
	td2=(Data->Width-w)*td0;
	t=Data->Map+(y*Data->Width+x)*td0+channel;

	if (t>s && t<sye) {
		s+=((h-1)*img.Data->Width+w-1)*sd0;
		t+=((h-1)*Data->Width+w-1)*td0;
		td0=-td0;
		td2=-td2;
		sd0=-sd0;
		sd1=-sd1;
		sd2=-sd2;
		sye=s+h*(sd1+sd2);
	}

	do {
		sxe=s+sd1;
		do {
			*t=*s;
			t+=td0;
			s+=sd0;
		} while (s!=sxe);
		t+=td2;
		s+=sd2;
	} while(s!=sye);
}


void emImage::CopyTransformed(
	int x, int y, int w, int h, const emATMatrix & atm,
	const emImage & img, bool interpolate, emColor bgColor
)
{
	emByte * p, * pe;
	const emByte * imap, * sp;
	emATMatrix invAtm;
	emColor color;
	int y2,cc,icc,iw,ih;
	double d,sw,sh,sx,sy,sxd,syd;

	if (x<0) { w+=x; x=0; }
	if (w>=Data->Width-x) w=Data->Width-x;
	if (w<=0) return;

	y2=y+h;
	if (y<0) y=0;
	if (y2>=Data->Height) y2=Data->Height;
	if (y>=y2) return;

	if (Data->RefCount>1) MakeWritable();

	invAtm=emInvertATM(atm);
	d=invAtm.TransX(0.0,0.0);
	sw=emMax(fabs(invAtm.TransX(1.0,0.0)-d),fabs(invAtm.TransX(0.0,1.0)-d));
	d=invAtm.TransY(0.0,0.0);
	sh=emMax(fabs(invAtm.TransY(1.0,0.0)-d),fabs(invAtm.TransY(0.0,1.0)-d));
	cc=Data->ChannelCount;
	do {
		sx=invAtm.TransX(x+0.5,y+0.5);
		sy=invAtm.TransY(x+0.5,y+0.5);
		sxd=invAtm.Get(0,0);
		syd=invAtm.Get(0,1);
		p=Data->Map+(y*Data->Width+x)*cc;
		pe=p+w*cc;
		if (interpolate) {
			sx-=sw*0.5;
			sy-=sh*0.5;
			do {
				color=img.GetPixelInterpolated(sx,sy,sw,sh,bgColor);
				switch (cc)
				{
				case 1:
					p[0]=color.GetGrey();
					p++;
					break;
				case 2:
					p[0]=color.GetGrey();
					p[1]=color.GetAlpha();
					p+=2;
					break;
				case 3:
					p[0]=color.GetRed();
					p[1]=color.GetGreen();
					p[2]=color.GetBlue();
					p+=3;
					break;
				default:
					p[0]=color.GetRed();
					p[1]=color.GetGreen();
					p[2]=color.GetBlue();
					p[3]=color.GetAlpha();
					p+=4;
				}
				sx+=sxd;
				sy+=syd;
			} while (p<pe);
		}
		else if (
			w>3 && cc==img.Data->ChannelCount &&
			syd==0.0 && sy>=0.0 && sy<img.Data->Height &&
			sx>=0.0 && sx<img.Data->Width &&
			sx+sxd*(w-1)>=0.0 && sx+sxd*(w-1)<img.Data->Width
		) {
			imap=img.Data->Map+((int)sy)*img.Data->Width*cc;
			switch (cc)
			{
			case 1:
				do {
					sp=imap+((int)sx);
					p[0]=sp[0];
					p++;
					sx+=sxd;
				} while (p<pe);
				break;
			case 2:
				do {
					sp=imap+((int)sx)*2;
					*((emUInt16*)p)=*((emUInt16*)sp);
					p+=2;
					sx+=sxd;
				} while (p<pe);
				break;
			case 3:
				do {
					sp=imap+((int)sx)*3;
					p[0]=sp[0];
					p[1]=sp[1];
					p[2]=sp[2];
					p+=3;
					sx+=sxd;
				} while (p<pe);
				break;
			default:
				do {
					sp=imap+((int)sx)*4;
					*((emUInt32*)p)=*((emUInt32*)sp);
					p+=4;
					sx+=sxd;
				} while (p<pe);
			}
		}
		else {
			iw=img.Data->Width;
			ih=img.Data->Height;
			icc=img.Data->ChannelCount;
			imap=img.Data->Map;
			do {
				if (sx>=0.0 && sx<iw && sy>=0.0 && sy<ih) {
					switch (icc)
					{
					case 1:
						sp=imap+((int)sy)*iw+((int)sx);
						switch (cc)
						{
						case 1:
							p[0]=sp[0];
							p++;
							break;
						case 2:
							p[0]=sp[0];
							p[1]=255;
							p+=2;
							break;
						case 3:
							p[0]=sp[0];
							p[1]=sp[0];
							p[2]=sp[0];
							p+=3;
							break;
						default:
							p[0]=sp[0];
							p[1]=sp[0];
							p[2]=sp[0];
							p[3]=255;
							p+=4;
						}
						break;
					case 2:
						sp=imap+(((int)sy)*iw+((int)sx))*2;
						switch (cc)
						{
						case 1:
							p[0]=sp[0];
							p++;
							break;
						case 2:
							*((emUInt16*)p)=*((emUInt16*)sp);
							p+=2;
							break;
						case 3:
							p[0]=sp[0];
							p[1]=sp[0];
							p[2]=sp[0];
							p+=3;
							break;
						default:
							p[0]=sp[0];
							p[1]=sp[0];
							p[2]=sp[0];
							p[3]=sp[1];
							p+=4;
						}
						break;
					case 3:
						sp=imap+(((int)sy)*iw+((int)sx))*3;
						switch (cc)
						{
						case 1:
							p[0]=(emByte)((((int)sp[0])+sp[1]+sp[2]+1)/3);
							p++;
							break;
						case 2:
							p[0]=(emByte)((((int)sp[0])+sp[1]+sp[2]+1)/3);
							p[1]=255;
							p+=2;
							break;
						case 3:
							p[0]=sp[0];
							p[1]=sp[1];
							p[2]=sp[2];
							p+=3;
							break;
						default:
							p[0]=sp[0];
							p[1]=sp[1];
							p[2]=sp[2];
							p[3]=255;
							p+=4;
						}
						break;
					default:
						sp=imap+(((int)sy)*iw+((int)sx))*4;
						switch (cc)
						{
						case 1:
							p[0]=(emByte)((((int)sp[0])+sp[1]+sp[2]+1)/3);
							p++;
							break;
						case 2:
							p[0]=(emByte)((((int)sp[0])+sp[1]+sp[2]+1)/3);
							p[1]=sp[4];
							p+=2;
							break;
						case 3:
							p[0]=sp[0];
							p[1]=sp[1];
							p[2]=sp[2];
							p+=3;
							break;
						default:
							*((emUInt32*)p)=*((emUInt32*)sp);
							p+=4;
						}
					}
				}
				else {
					switch (cc)
					{
					case 1:
						p[0]=bgColor.GetGrey();
						p++;
						break;
					case 2:
						p[0]=bgColor.GetGrey();
						p[1]=bgColor.GetAlpha();
						p+=2;
						break;
					case 3:
						p[0]=bgColor.GetRed();
						p[1]=bgColor.GetGreen();
						p[2]=bgColor.GetBlue();
						p+=3;
						break;
					default:
						p[0]=bgColor.GetRed();
						p[1]=bgColor.GetGreen();
						p[2]=bgColor.GetBlue();
						p[3]=bgColor.GetAlpha();
						p+=4;
					}
				}
				sx+=sxd;
				sy+=syd;
			} while (p<pe);
		}
		y++;
	} while (y<y2);
}


emImage emImage::GetTransformed(
	const emATMatrix & atm, bool interpolate, emColor bgColor,
	int channelCount
) const
{
	double x,y,x1,y1,x2,y2;
	emImage img;
	int w,h;

	x1=x2=atm.TransX(0.0,0.0);
	y1=y2=atm.TransY(0.0,0.0);
	x=atm.TransX(Data->Width,0.0);
	y=atm.TransY(Data->Width,0.0);
	if (x1>x) x1=x; else if (x2<x) x2=x;
	if (y1>y) y1=y; else if (y2<y) y2=y;
	x=atm.TransX(0.0,Data->Height);
	y=atm.TransY(0.0,Data->Height);
	if (x1>x) x1=x; else if (x2<x) x2=x;
	if (y1>y) y1=y; else if (y2<y) y2=y;
	x=atm.TransX(Data->Width,Data->Height);
	y=atm.TransY(Data->Width,Data->Height);
	if (x1>x) x1=x; else if (x2<x) x2=x;
	if (y1>y) y1=y; else if (y2<y) y2=y;
	w=(int)(x2-x1+0.5);
	h=(int)(y2-y1+0.5);
	if (channelCount<0) channelCount=Data->ChannelCount;
	img.Setup(w,h,channelCount);
	img.CopyTransformed(
		0,0,w,h,
		emTranslateATM((w-x1-x2)*0.5,(h-y1-y2)*0.5,atm),
		*this,
		interpolate,
		bgColor
	);
	return img;
}


void emImage::CalcMinMaxRect(int * pX, int * pY, int * pW, int * pH,
                              emColor bgColor) const
{
	emByte bgValue[4];
	int cc,i,x1,y1,x2,y2,x,y,w,h;

	cc=Data->ChannelCount;
	if (cc<3) {
		bgValue[0]=bgColor.GetGrey();
		bgValue[1]=bgColor.GetAlpha();
	}
	else {
		bgValue[0]=bgColor.GetRed();
		bgValue[1]=bgColor.GetGreen();
		bgValue[2]=bgColor.GetBlue();
		bgValue[3]=bgColor.GetAlpha();
	}
	x1=0;
	y1=0;
	x2=0;
	y2=0;
	for (i=0; i<cc; i++) {
		CalcChannelMinMaxRect(&x,&y,&w,&h,i,bgValue[i]);
		if (w>0 && h>0) {
			if (x1<x2 && y1<y2) {
				if (x1>x) x1=x;
				if (y1>y) y1=y;
				if (x2<x+w) x2=x+w;
				if (y2<y+h) y2=y+h;
			}
			else {
				x1=x;
				y1=y;
				x2=x+w;
				y2=y+h;
			}
		}
	}
	*pX=x1;
	*pY=y1;
	*pW=x2-x1;
	*pH=y2-y1;
}


void emImage::CalcChannelMinMaxRect(
	int * pX, int * pY, int * pW, int * pH, int channel, emByte bgValue
) const
{
	const emByte * p0, * p, * pe;
	int cc,rs,x1,y1,x2,y2;

	cc=Data->ChannelCount;
	if ((unsigned)channel>=(unsigned)cc) goto L_Empty;
	p0=Data->Map+channel;
	rs=Data->Width*cc;
	x1=0;
	y1=0;
	x2=Data->Width;
	y2=Data->Height;
	if (x2<=0) goto L_Empty;
	for (;;) {
		if (y1>=y2) goto L_Empty;
		p=p0+y1*rs;
		pe=p+rs;
		do {
			if (p[0]!=bgValue) break;
			p+=cc;
		} while (p<pe);
		if (p<pe) break;
		y1++;
	}
	while (y1<y2-1) {
		p=p0+(y2-1)*rs;
		pe=p+rs;
		do {
			if (p[0]!=bgValue) break;
			p+=cc;
		} while (p<pe);
		if (p<pe) break;
		y2--;
	}
	for (;;) {
		p=p0+y1*rs+x1*cc;
		pe=p+rs*(y2-y1);
		do {
			if (p[0]!=bgValue) break;
			p+=rs;
		} while (p<pe);
		if (p<pe) break;
		x1++;
	}
	while (x1<x2-1) {
		p=p0+y1*rs+(x2-1)*cc;
		pe=p+rs*(y2-y1);
		do {
			if (p[0]!=bgValue) break;
			p+=rs;
		} while (p<pe);
		if (p<pe) break;
		x2--;
	}
	*pX=x1; *pY=y1; *pW=x2-x1; *pH=y2-y1;
	return;
L_Empty:
	*pX=0; *pY=0; *pW=0; *pH=0;
}


void emImage::CalcAlphaMinMaxRect(
	int * pX, int * pY, int * pW, int * pH
) const
{
	int cc;

	cc=Data->ChannelCount;
	if (cc==2 || cc==4) {
		CalcChannelMinMaxRect(pX,pY,pW,pH,cc-1,0);
	}
	else {
		*pX=0; *pY=0; *pW=Data->Width; *pH=Data->Height;
	}
}


emImage emImage::GetCropped(
	int x, int y, int w, int h, int channelCount
) const
{
	emImage img;
	int cc;

	cc=Data->ChannelCount;
	if (x<0) { w+=x; x=0; }
	if (y<0) { h+=y; y=0; }
	if (w>Data->Width-x) w=Data->Width-x;
	if (h>Data->Height-y) h=Data->Height-y;
	if (channelCount<0) channelCount=cc;
	if (w==Data->Width && h==Data->Height && channelCount==cc) {
		img=*this;
	}
	else {
		img.Setup(w,h,channelCount);
		img.Copy(0,0,*this,x,y,w,h);
	}
	return img;
}


emImage emImage::GetCroppedByAlpha(int channelCount) const
{
	int x,y,w,h;

	CalcAlphaMinMaxRect(&x,&y,&w,&h);
	return GetCropped(x,y,w,h,channelCount);
}


bool emImage::PreparePainter(
	emPainter * painter, emRootContext & rootContext, double clipX1,
	double clipY1, double clipX2, double clipY2, double originX,
	double originY, double scaleX, double scaleY
)
{
	if (Data->ChannelCount!=4) {
		*painter=emPainter();
		return false;
	}
	if (Data->RefCount>1) MakeWritable();
	if (clipX1<0.0) clipX1=0.0;
	if (clipY1<0.0) clipY1=0.0;
	if (clipX2>Data->Width) clipX2=Data->Width;
	if (clipY2>Data->Height) clipX2=Data->Height;
	*painter=emPainter(
		rootContext,
		Data->Map,
		Data->Width*4,
		4,
#		if EM_BYTE_ORDER==4321
			0xff000000,0x00ff0000,0x0000ff00,
#		elif EM_BYTE_ORDER==1234
			0x000000ff,0x0000ff00,0x00ff0000,
#		elif EM_BYTE_ORDER==3412
			0x00ff0000,0xff000000,0x000000ff,
#		else
#			error unexpected value for EM_BYTE_ORDER
#		endif
		clipX1,clipY1,clipX2,clipY2,
		originX,originY,scaleX,scaleY
	);
	return true;
}


bool emImage::IsChannelCountPaintable(int channelCount)
{
	return channelCount==4;
}


unsigned int emImage::GetDataRefCount() const
{
	return Data==&EmptyData ? UINT_MAX/2 : Data->RefCount;
}


void emImage::MakeWritable()
{
	SharedData * d;
	size_t mapSize;

	if (Data->RefCount>1 && Data!=&EmptyData) {
		mapSize=Data->Width*Data->Height*Data->ChannelCount;
		d=(SharedData*)malloc(sizeof(SharedData)+mapSize);
		d->RefCount=1;
		d->Width=Data->Width;
		d->Height=Data->Height;
		d->ChannelCount=Data->ChannelCount;
		d->IsUsersMap=0;
		d->Map=((emByte*)d)+sizeof(SharedData);
		if (mapSize) memcpy(d->Map,Data->Map,mapSize);
		if (!--Data->RefCount) FreeData();
		Data=d;
	}
}


void emImage::FreeData()
{
	EmptyData.RefCount=UINT_MAX/2;
	if (Data!=&EmptyData) free(Data);
}


emImage::SharedData emImage::EmptyData = {UINT_MAX/2,0,0,1,0,NULL};
