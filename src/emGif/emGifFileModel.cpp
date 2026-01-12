//------------------------------------------------------------------------------
// emGifFileModel.cpp
//
// Copyright (C) 2004-2009,2014,2018-2019,2025 Oliver Hamann.
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

#include <emGif/emGifFileModel.h>


emRef<emGifFileModel> emGifFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emGifFileModel,context,name,common)
}


double emGifFileModel::GetTallness() const
{
	return Width>0 && Height>0 ? Height*PixelTallness/Width : 1.0;
}


void emGifFileModel::RenderImage(int index, emImage * image) const
{
	struct {
		emInt16 prev;
		emUInt16 len;
		emByte firstData;
		emByte data;
		emUInt16 padding;
	} table[4096];
	emByte buf[4096];
	const emColor * colors;
	emByte * t, * t0, * txe, * s, * se, * b, * be;
	Render * r;
	int i,dy,y,ny,td1,colorCount,transp,pix,code,codeSize,codeMask;
	int prevCode,clearCode,maxCode,sbits,ccnt;

	if (
		!image ||
		image->GetWidth()<Width ||
		image->GetHeight()<Height ||
		File ||
		index<0 ||
		index>=RenderCount
	) return;

	r=RenderArray[index];
	if (r->ColorCount) {
		colors=r->Colors;
		colorCount=r->ColorCount;
	}
	else {
		colors=Colors;
		colorCount=ColorCount;
	}
	clearCode=1<<r->MinCodeSize;
	maxCode=clearCode+2;
	for (i=0; i<maxCode; i++) {
		table[i].prev=-1;
		table[i].len=1;
		table[i].data=(emByte)i;
		table[i].firstData=(emByte)i;
	}
	codeSize=r->MinCodeSize+1;
	codeMask=(1<<codeSize)-1;
	prevCode=-1;
	sbits=8;
	ccnt=image->GetChannelCount();
	transp=r->Transparent;
	t0=image->GetWritableMap()+(image->GetWidth()*(size_t)r->Y+r->X)*ccnt;
	td1=r->Width*ccnt;
	s=(emByte*)r->Data;
	se=s+r->DataFill;
	dy=r->Interlaced ? 8 : 1;
	y=0;
	ny=r->Height;
	b=be=NULL;
	do {
		if (y>=r->Height) {
			if ((y&7)==0) y=4;
			else {
				dy>>=1;
				y=dy>>1;
			}
		}
		t=t0+image->GetWidth()*(size_t)y*ccnt;
		txe=t+td1;
		do {
			if (b>=be) {
				for (;;) {
					code=*s>>(8-sbits);
					if (sbits<codeSize) {
						s++;
						if (s>=se) goto L_END;
						code|=*s<<sbits;
						sbits+=8;
						if (sbits<codeSize) {
							s++;
							if (s>=se) goto L_END;
							code|=*s<<sbits;
							sbits+=8;
						}
					}
					sbits-=codeSize;
					code&=codeMask;
					if (code<clearCode || (code>clearCode+1 && code<=maxCode)) break;
					if (code!=clearCode) goto L_END;
					maxCode=clearCode+2;
					codeSize=r->MinCodeSize+1;
					codeMask=(1<<codeSize)-1;
					prevCode=-1;
				}
				if (maxCode<4096 && prevCode>=0) {
					table[maxCode].prev=(emInt16)prevCode;
					table[maxCode].firstData=table[prevCode].firstData;
					table[maxCode].len=(emUInt16)(table[prevCode].len+1);
					table[maxCode].data=table[code].firstData;
					maxCode++;
					if (maxCode>codeMask && maxCode<4096) {
						codeSize++;
						codeMask=(1<<codeSize)-1;
					}
				}
				prevCode=code;
				b=be=buf+table[code].len;
				do {
					b--;
					*b=table[code].data;
					code=table[code].prev;
				} while(b>buf);
			}
			pix=*b;
			b++;
			switch (ccnt) {
				case 1:
					if (pix!=transp && pix<colorCount) {
						t[0]=colors[pix].GetRed();
					}
					t++;
					break;
				case 2:
					if (pix!=transp && pix<colorCount) {
						t[0]=colors[pix].GetRed();
						t[1]=255;
					}
					t+=2;
					break;
				case 3:
					if (pix!=transp && pix<colorCount) {
						t[0]=colors[pix].GetRed();
						t[1]=colors[pix].GetGreen();
						t[2]=colors[pix].GetBlue();
					}
					t+=3;
					break;
				default:
					if (pix!=transp && pix<colorCount) {
						t[0]=colors[pix].GetRed();
						t[1]=colors[pix].GetGreen();
						t[2]=colors[pix].GetBlue();
						t[3]=255;
					}
					t+=4;
					break;
			}
		} while (t<txe);
		y+=dy;
		ny--;
	} while (ny>0);
L_END:
	return;
}


emImage emGifFileModel::RenderAll() const
{
	emImage image, undoImage;
	Render * r;
	int i;

	image.Setup(Width,Height,ChannelCount);
	if (RenderCount<=0) {
		image.Fill(BGColor);
		return image;
	}
	r=RenderArray[0];
	if (r->Transparent>=0 || r->X!=0 || r->Y!=0 ||
	    r->Width!=Width || r->Height!=Height) {
		image.Fill(0,0,Width,Height,BGColor);
	}
	RenderImage(0,&image);
	for (i=1; i<RenderCount; i++) {
		if (r->Disposal==2) {
			image.Fill(
				r->X,
				r->Y,
				r->Width,
				r->Height,
				BGColor
			);
		}
		else if (r->Disposal==3) {
			if (!undoImage.IsEmpty()) {
				image.Copy(r->X,r->Y,undoImage);
			}
			else {
				image.Fill(
					r->X,
					r->Y,
					r->Width,
					r->Height,
					BGColor
				);
			}
		}
		r=RenderArray[i];
		if (r->Disposal==3) {
			undoImage.Setup(r->Width,r->Height,image.GetChannelCount());
			undoImage.Copy(-r->X,-r->X,image);
		}
		else {
			undoImage.Clear();
		}
		RenderImage(i,&image);
	}
	return image;
}


emGifFileModel::emGifFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	Width=0;
	Height=0;
	ChannelCount=0;
	ColorCount=0;
	RenderCount=0;
	RenderArraySize=0;
	Animated=false;
	BGColor=emColor(0,0,0,0);
	FileSize=0;
	PixelTallness=1.0;
	Comment.Clear();
	Colors=NULL;
	RenderArray=NULL;
	InLoadingRenderData=false;
	NextDisposal=0;
	NextUserInput=false;
	NextDelay=0;
	NextTransparent=-1;
}


emGifFileModel::~emGifFileModel()
{
	emGifFileModel::QuitLoading();
	emGifFileModel::ResetData();
}


void emGifFileModel::ResetData()
{
	Render * r;
	int i;

	if (Colors) {
		delete [] Colors;
		Colors=NULL;
	}
	if (RenderArray) {
		for (i=0; i<RenderCount; i++) {
			r=RenderArray[i];
			if (r->Colors) delete [] r->Colors;
			if (r->Data) delete [] r->Data;
			delete r;
		}
		delete [] RenderArray;
		RenderArray=NULL;
		RenderCount=0;
		RenderArraySize=0;
	}
	Width=0;
	Height=0;
	ChannelCount=0;
	ColorCount=0;
	Animated=false;
	BGColor=emColor(0,0,0,0);
	FileSize=0;
	PixelTallness=1.0;
	Comment.Clear();
}


void emGifFileModel::TryStartLoading()
{
	char sigver[6];
	int i,flags,bgIndex,aspect;

	FileSize=emTryGetFileSize(GetFilePath());

	File=new emFileStream();
	File->TryOpen(GetFilePath(),"rb");

	File->TryRead(sigver,6);
	Width=File->TryReadUInt16LE();
	Height=File->TryReadUInt16LE();
	flags=File->TryReadUInt8();
	bgIndex=File->TryReadUInt8();
	aspect=File->TryReadUInt8();

#if 0 // Too many buggy encoders...
	if (aspect) PixelTallness=64.0/(aspect+15.0);
	else PixelTallness=1.0;
#else
	PixelTallness=1.0;
#endif

	if (memcmp(sigver,"GIF87a",6)!=0 &&
	    memcmp(sigver,"GIF89a",6)!=0) goto Err;

	if ((flags&0x80)!=0) {
		ColorCount=2<<(flags&7);
		Colors=new emColor[ColorCount];
		for (i=0; i<ColorCount; i++) {
			Colors[i].SetRed(File->TryReadUInt8());
			Colors[i].SetGreen(File->TryReadUInt8());
			Colors[i].SetBlue(File->TryReadUInt8());
			Colors[i].SetAlpha(255);
		}
		if (bgIndex<ColorCount) {
			BGColor=Colors[bgIndex];
			BGColor.SetAlpha(0);
		}
	}

	return;
Err:
	throw emException("GIF format error");
}


bool emGifFileModel::TryContinueLoading()
{
	char * p;
	Render * r;
	Render * * pr;
	char tmp[256];
	int b,i;

	if (InLoadingRenderData) {
		r=RenderArray[RenderCount-1];
		b=File->TryReadUInt8();
		if (b) {
			if (r->DataFill+b>r->DataSize) {
				r->DataSize+=65536;
				p=new char[r->DataSize];
				if (r->Data) {
					memcpy(p,r->Data,r->DataFill);
					delete [] r->Data;
				}
				r->Data=p;
			}
			File->TryRead(r->Data+r->DataFill,b);
			r->DataFill+=b;
		}
		else {
			if (r->DataFill<=0) goto Err;
			if (r->DataFill<r->DataSize) {
				p=new char[r->DataFill];
				memcpy(p,r->Data,r->DataFill);
				delete [] r->Data;
				r->Data=p;
				r->DataSize=r->DataFill;
			}
			InLoadingRenderData=false;
		}
		return false;
	}

	b=File->TryReadUInt8();
	if (b==0x3B) {
		File.Reset();
		if (!PostProcess()) goto Err;
		return true;
	}
	else if (b==0x2c) {
		r=new Render;
		r->Disposal=NextDisposal;
		r->Delay=NextDelay;
		r->Transparent=NextTransparent;
		r->UserInput=NextUserInput;
		r->Interlaced=false;
		r->X=File->TryReadUInt16LE();
		r->Y=File->TryReadUInt16LE();
		r->Width=File->TryReadUInt16LE();
		r->Height=File->TryReadUInt16LE();
		r->ColorCount=0;
		r->DataSize=0;
		r->DataFill=0;
		r->Colors=NULL;
		r->Data=NULL;
		if (RenderCount>=RenderArraySize) {
			RenderArraySize+=64;
			pr=new Render*[RenderArraySize];
			if (RenderArray) {
				for (i=0; i<RenderCount; i++) pr[i]=RenderArray[i];
				delete [] RenderArray;
			}
			RenderArray=pr;
		}
		RenderArray[RenderCount++]=r;
		b=File->TryReadUInt8();
		if ((b&0x40)!=0) r->Interlaced=true;
		if ((b&0x80)!=0) {
			r->ColorCount=2<<(b&7);
			r->Colors=new emColor[r->ColorCount];
			for (i=0; i<r->ColorCount; i++) {
				r->Colors[i].SetRed(File->TryReadUInt8());
				r->Colors[i].SetGreen(File->TryReadUInt8());
				r->Colors[i].SetBlue(File->TryReadUInt8());
				r->Colors[i].SetAlpha(255);
			}
		}
		else {
			if (ColorCount<=0) goto Err;
		}
		r->MinCodeSize=File->TryReadUInt8();
		if (r->MinCodeSize<1 || r->MinCodeSize>8) goto Err;
		NextDisposal=0;
		NextUserInput=false;
		NextDelay=0;
		NextTransparent=-1;
		InLoadingRenderData=true;
	}
	else if (b==0x21) {
		b=File->TryReadUInt8();
		if (b==0xfe) {
			if (Comment.GetLen()>=65536) goto Err;
			for (i=0;;) {
				b=File->TryReadUInt8();
				if (!b) break;
				i+=b;
				if (i>=65536) goto Err;
				File->TryRead(tmp,b);
				tmp[b]=0;
				Comment+=tmp;
			}
		}
		else if (b==0xf9) {
			if (File->TryReadUInt8()!=4) goto Err;
			b=File->TryReadUInt8();
			NextDelay=File->TryReadUInt16LE();
			NextTransparent=File->TryReadUInt8();
			File->TryReadUInt8();
			NextDisposal=(b>>2)&7;
			if (NextDisposal==4) NextDisposal=3;
			if (NextDisposal>3) goto Err;
			NextUserInput=(b&0x02) ? true : false;
			if ((b&0x01)==0) NextTransparent=-1;
		}
		else if (b==0x01) {
			NextDisposal=0;
			NextUserInput=false;
			NextDelay=0;
			NextTransparent=-1;
			for (;;) {
				b=File->TryReadUInt8();
				if (!b) break;
				File->TrySkip(b);
			}
		}
		else {
			for (;;) {
				b=File->TryReadUInt8();
				if (!b) break;
				File->TrySkip(b);
			}
		}
	}
	else if (b) {
		goto Err;
	}
	return false;

Err:
	throw emException("GIF format error");
}


void emGifFileModel::QuitLoading()
{
	File.Reset();
	InLoadingRenderData=false;
	NextDisposal=0;
	NextUserInput=false;
	NextDelay=0;
	NextTransparent=-1;
}


void emGifFileModel::TryStartSaving()
{
	throw emException("emGifFileModel: Saving not implemented.");
}


bool emGifFileModel::TryContinueSaving()
{
	return true;
}


void emGifFileModel::QuitSaving()
{
}


emUInt64 emGifFileModel::CalcMemoryNeed()
{
	emUInt64 need;

	need=((emUInt64)Width)*Height;
	if (ChannelCount) need*=ChannelCount;
	else need*=3;
	need=need*3/2;
	need+=FileSize;
	return need;
}


double emGifFileModel::CalcFileProgress()
{
	double progress;
	emUInt64 sz;
	int i;

	if (!File) return 100.0;
	sz=ColorCount*3;
	for (i=0; i<RenderCount; i++) {
		sz+=RenderArray[i]->ColorCount*3;
		sz+=RenderArray[i]->DataFill;
	}
	progress=100.0*sz/FileSize;
	if (progress>100.0) progress=100.0;
	return progress;
}


bool emGifFileModel::PostProcess()
{
	Render * * ra;
	int i,j,cc,d;

	ra=RenderArray;

	for (i=0; i<RenderCount; i++) {
		if (ra[i]->Width <=0) ra[i]->Width =Width ;
		if (ra[i]->Height<=0) ra[i]->Height=Height;
	}
	for (i=0; i<RenderCount; i++) {
		if (Width <ra[i]->Width ) Width =ra[i]->Width ;
		if (Height<ra[i]->Height) Height=ra[i]->Height;
	}
	if (Width<=0 || Height<=0) return false;
	for (i=0; i<RenderCount; i++) {
		if (ra[i]->X>Width -ra[i]->Width ) ra[i]->X=Width -ra[i]->Width ;
		if (ra[i]->Y>Height-ra[i]->Height) ra[i]->Y=Height-ra[i]->Height;
	}

	for (i=0; i<RenderCount; i++) {
		cc=ra[i]->ColorCount;
		if (!cc) cc=ColorCount;
		if (ra[i]->Transparent>=cc) ra[i]->Transparent=-1;
	}

	if (RenderCount>1) {
		for (i=0; i<RenderCount; i++) {
			if (
				RenderArray[i]->Delay ||
				RenderArray[i]->UserInput ||
				RenderArray[i]->Disposal>=2
			) {
				Animated=true;
				break;
			}
		}
		if (!Animated) {
			// Not sure about this: be animated if there are overlappings:
			for (i=0; i<RenderCount; i++) {
				for (j=i; j<RenderCount; j++) {
					if (
						ra[j]->X<ra[i]->X+ra[i]->Width &&
						ra[j]->Y<ra[i]->Y+ra[i]->Height &&
						ra[j]->X+ra[j]->Width>ra[i]->X &&
						ra[j]->Y+ra[j]->Height>ra[i]->Y
					) {
						Animated=true;
						break;
					}
				}
			}
		}
		if (Animated) {
			d=1;
			for (i=0; i<RenderCount; i++) {
				if (!ra[i]->UserInput) {
					if (ra[i]->Delay) d=ra[i]->Delay;
					else ra[i]->Delay=d;
				}
			}
		}
	}
	else if (RenderCount==1) {
		ra[0]->Delay=0;
		ra[0]->UserInput=false;
	}

	if (RenderCount>0) ra[RenderCount-1]->Disposal=0;
	for (i=0; i<RenderCount-1; i++) {
		if (ra[i]->Disposal==3) {
			for (j=i-1; j>=0; j--) {
				if (
					ra[j]->Disposal!=2 &&
					ra[j]->X<ra[i]->X+ra[i]->Width &&
					ra[j]->Y<ra[i]->Y+ra[i]->Height &&
					ra[j]->X+ra[j]->Width>ra[i]->X &&
					ra[j]->Y+ra[j]->Height>ra[i]->Y
				) break;
			}
			if (j<0) ra[i]->Disposal=2;
		}
	}
	for (i=RenderCount-1; i>=0; i--) {
		if (ra[i]->Disposal==3) continue;
		if (ra[i]->Transparent>=0) continue;
		for (j=i-1; j>=0; j--) {
			if (
				(ra[j]->Disposal==2 || ra[j]->Disposal==3) &&
				ra[j]->X>=ra[i]->X &&
				ra[j]->Y>=ra[i]->Y &&
				ra[j]->X+ra[j]->Width<=ra[i]->X+ra[i]->Width &&
				ra[j]->Y+ra[j]->Height<=ra[i]->Y+ra[i]->Height
			) {
				ra[j]->Disposal=0;
			}
			if (ra[j]->UserInput || ra[j]->Delay) break;
		}
	}

	ChannelCount=2;
	for (i=0; i<RenderCount; i++) {
		j=ChannelCount;
		if (
			ra[i]->Transparent<0 &&
			ra[i]->X<=0 &&
			ra[i]->Y<=0 &&
			ra[i]->X+ra[i]->Width>=Width &&
			ra[i]->Y+ra[i]->Height>=Height
		) {
			ChannelCount=1;
		}
		if (ChannelCount==2) {
			if (ra[i]->UserInput || ra[i]->Delay) break;
		}
		if (ra[i]->Disposal==2) ChannelCount=2;
		if (ra[i]->Disposal==3) ChannelCount=j;
	}
	if (ChannelCount==2) {
		if (BGColor.IsOpaque()) ChannelCount-=1;
		if (!BGColor.IsTotallyTransparent() && !BGColor.IsGrey()) ChannelCount+=2;
	}
	for (i=0; ChannelCount<3 && i<RenderCount; i++) {
		if (ra[i]->ColorCount) {
			for (j=0; j<ra[i]->ColorCount; j++) {
				if (!ra[i]->Colors[j].IsGrey()) { ChannelCount+=2; break; }
			}
		}
		else {
			for (j=0; j<ColorCount; j++) {
				if (!Colors[j].IsGrey()) { ChannelCount+=2; break; }
			}
		}
	}

	return true;
}
