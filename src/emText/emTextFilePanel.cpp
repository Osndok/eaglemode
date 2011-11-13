//------------------------------------------------------------------------------
// emTextFilePanel.cpp
//
// Copyright (C) 2004-2010 Oliver Hamann.
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

#include <emText/emTextFilePanel.h>
#include <emCore/emToolkit.h>


emTextFilePanel::emTextFilePanel(
	ParentArg parent, const emString & name, emTextFileModel * fileModel,
	bool updateFileModel, bool alternativeView
)
	: emFilePanel(parent,name)
{
	AlternativeView=alternativeView;
	Model=NULL;
	AddWakeUpSignal(GetVirFileStateSignal());
	SetFileModel(fileModel,updateFileModel);
}


void emTextFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	Model=dynamic_cast<emTextFileModel*>(fileModel);
	emFilePanel::SetFileModel(Model,updateFileModel);
}


bool emTextFilePanel::Cycle()
{
	static const char * const ALT_ERROR="Hex display is not an alternative.";

	if (IsSignaled(GetVirFileStateSignal())) {
		InvalidateControlPanel(); //??? very cheap solution, but okay for now.
		InvalidatePainting();
		if (IsVFSGood()) {
			if (AlternativeView && Model->GetCharEncoding()==emTextFileModel::CE_BINARY) {
				SetCustomError(ALT_ERROR);
			}
		}
		else if (GetCustomError()==ALT_ERROR) {
			switch (Model->GetFileState()) {
			case emFileModel::FS_LOADED:
			case emFileModel::FS_UNSAVED:
			case emFileModel::FS_SAVING:
				if (AlternativeView && Model->GetCharEncoding()==emTextFileModel::CE_BINARY) {
					break;
				}
			default:
				ClearCustomError();
			}
		}
	}
	return emFilePanel::Cycle();
}


bool emTextFilePanel::IsOpaque()
{
	if (IsVFSGood()) {
		return false;
	}
	else {
		return emFilePanel::IsOpaque();
	}
}


void emTextFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (IsVFSGood()) {
		if (Model->GetCharEncoding()==emTextFileModel::CE_BINARY || AlternativeView) {
			PaintAsHex(painter,canvasColor);
		}
		else {
			PaintAsText(painter,canvasColor);
		}
	}
	else {
		emFilePanel::Paint(painter,canvasColor);
	}
}


emPanel * emTextFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	emTkGroup * grp;
	const char * p;

	if (
		IsVFSGood() &&
		Model->GetCharEncoding()!=emTextFileModel::CE_BINARY &&
		!AlternativeView
	) {

		grp=new emTkGroup(
			parent,
			name,
			"Text File Info"
		);

		grp->SetRowByRow();
		grp->SetPrefChildTallness(0.2);

		switch (Model->GetCharEncoding()) {
			case emTextFileModel::CE_7BIT   : p="7-Bit"    ; break;
			case emTextFileModel::CE_8BIT   : p="8-Bit"    ; break;
			case emTextFileModel::CE_UTF8   : p="UTF-8"    ; break;
			case emTextFileModel::CE_UTF16LE: p="UTF-16LE" ; break;
			case emTextFileModel::CE_UTF16BE: p="UTF-16BE" ; break;
			default                         : p="Binary"   ;
		}
		new emTkTextField(
			grp,
			"enc",
			"Character Encoding",
			emString(),
			emImage(),
			p
		);

		switch (Model->GetLineBreakEncoding()) {
			case emTextFileModel::LBE_MAC  : p="MAC (CR)"  ; break;
			case emTextFileModel::LBE_DOS  : p="DOS (CRLF)"; break;
			case emTextFileModel::LBE_UNIX : p="UNIX (LF)" ; break;
			case emTextFileModel::LBE_MIXED: p="Mixed"     ; break;
			default                        : p="None"      ;
		}
		new emTkTextField(
			grp,
			"lbenc",
			"Line Break Encoding",
			emString(),
			emImage(),
			p
		);

		new emTkTextField(
			grp,
			"lines",
			"Number of Lines",
			emString(),
			emImage(),
			emString::Format("%d",Model->GetLineCount())
		);

		new emTkTextField(
			grp,
			"columns",
			"Number of Columns",
			emString(),
			emImage(),
			emString::Format("%d",Model->GetColumnCount())
		);
		return grp;
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


void emTextFilePanel::PaintAsText(
	const emPainter & painter, emColor canvasColor
)
{
	static const emColor textBgColor(255,255,255);
	static const emColor textFgColor(0,0,0);
	static const emColor textFgColor96(textFgColor,96);
	const char * pContent, * pRow;
	int i1,i2,i3,row,row2,col,cols,rows,page,pages,pagerows,step;
	double h,ch,cw,f,t,pagew,gap,x,y,clipx1,clipy1,clipx2,clipy2;

	h=GetHeight();

	clipx1=painter.GetUserClipX1();
	clipy1=painter.GetUserClipY1();
	clipx2=painter.GetUserClipX2();
	clipy2=painter.GetUserClipY2();

	pContent=Model->GetContent();
	rows=Model->GetLineCount();
	cols=Model->GetColumnCount();
	if (cols<8) cols=8;

	f=painter.GetTextSize("X",1.0,false);
	gap=1.0;
	t=0.5*gap/(cols+gap);
	pages=(int)floor(t+sqrt((2*rows/(h*f*gap)+t)*t));
	// pages*h/rows*f*(cols*pages+gap*(pages-1)) <= 1.0
	if (pages<1) {
		pages=1;
		pagerows=rows;
		cw=1.0/cols;
		ch=cw/f;
		pagew=1.0;
		gap*=cw;
	}
	else {
		pagerows=(rows+pages-1)/pages;
		ch=h/pagerows;
		cw=ch*f;
		gap*=cw;
		pagew=(1.0-(pages-1)*gap)/pages;
	}

	page=(int)(clipx1/(pagew+gap));
	if (page<0) page=0;
	x=page*(pagew+gap);
	for (; page<pages && x<clipx2; page++, x+=pagew+gap) {

		painter.PaintRect(
			x,
			0.0,
			pagew,
			h,
			textBgColor,
			canvasColor
		);

		row=(int)(clipy1/ch);
		if (row<0) row=0;
		y=row*ch;
		row+=page*pagerows;
		row2=(int)ceil(clipy2/ch);
		if (row2>pagerows) row2=pagerows;
		row2+=page*pagerows;
		if (row2>rows) row2=rows;

		if (ch*GetViewedWidth()<0.5) {
			step=(int)(0.5/(ch*GetViewedWidth()));
			if (step<1) step=1;
			row=((row-1)/step+1)*step;
			while (row<row2) {
				f=cols*cw/255.0;
				painter.PaintRect(
					x+Model->GetRelativeLineIndent(row)*f,
					y+ch*0.1,
					Model->GetRelativeLineWidth(row)*f,
					ch*step*0.8,
					textFgColor96,
					textBgColor
				);
				y+=ch*step;
				row+=step;
			}
		}
		else {
			while (row<row2) {
				i1=Model->GetLineStart(row);
				pRow=pContent+i1;
				i3=Model->GetLineEnd(row)-i1;
				i2=0;
				col=0;
				for (;;) {
					switch (Model->GetCharEncoding()) {
					case emTextFileModel::CE_UTF16LE:
						while (i2<i3 && (((emByte)pRow[i2])|(((emByte)pRow[i2+1])<<8))==0x09) {
							col=(col+8)&~7;
							i2+=2;
						}
						i1=i2;
						while (i2<i3 && (((emByte)pRow[i2])|(((emByte)pRow[i2+1])<<8))!=0x09) {
							i2+=2;
						}
						break;
					case emTextFileModel::CE_UTF16BE:
						while (i2<i3 && ((((emByte)pRow[i2])<<8)|((emByte)pRow[i2+1]))==0x09) {
							col=(col+8)&~7;
							i2+=2;
						}
						i1=i2;
						while (i2<i3 && ((((emByte)pRow[i2])<<8)|((emByte)pRow[i2+1]))!=0x09) {
							i2+=2;
						}
						break;
					default:
						while (i2<i3 && pRow[i2]==0x09) { col=(col+8)&~7; i2++; }
						i1=i2;
						while (i2<i3 && pRow[i2]!=0x09) i2++;
						break;
					}
					if (i1>=i2) break;
					switch (Model->GetCharEncoding()) {
					case emTextFileModel::CE_UTF8:
						if (emIsUtf8System()) {
							col+=PaintTextUtf8ToUtf8(
								painter,
								x+col*cw,
								y,
								cw,
								ch,
								pRow+i1,
								i2-i1,
								textFgColor,
								textBgColor
							);
						}
						else {
							col+=PaintTextUtf8To8Bit(
								painter,
								x+col*cw,
								y,
								cw,
								ch,
								pRow+i1,
								i2-i1,
								textFgColor,
								textBgColor
							);
						}
						break;
					case emTextFileModel::CE_8BIT:
						if (emIsUtf8System()) {
							col+=PaintText8BitToUtf8(
								painter,
								x+col*cw,
								y,
								cw,
								ch,
								pRow+i1,
								i2-i1,
								textFgColor,
								textBgColor
							);
						}
						else {
							painter.PaintText(
								x+col*cw,
								y,
								pRow+i1,
								ch,
								1.0,
								textFgColor,
								textBgColor,
								i2-i1
							);
							col+=i2-i1;
						}
						break;
					case emTextFileModel::CE_UTF16LE:
					case emTextFileModel::CE_UTF16BE:
						col+=PaintTextUtf16(
							painter,
							x+col*cw,
							y,
							cw,
							ch,
							pRow+i1,
							i2-i1,
							textFgColor,
							textBgColor
						);
						break;
					default:
						painter.PaintText(
							x+col*cw,
							y,
							pRow+i1,
							ch,
							1.0,
							textFgColor,
							textBgColor,
							i2-i1
						);
						col+=i2-i1;
						break;
					}
				}
				y+=ch;
				row++;
			}
		}
	}
}


int emTextFilePanel::PaintTextUtf8ToUtf8(
	const emPainter & painter, double x, double y, double charWidth,
	double charHeight, const char * text, int textLen,
	emColor color, emColor canvasColor
) const
{
	int i,columns,c,n;

	painter.PaintText(x,y,text,charHeight,1.0,color,canvasColor,textLen);

	columns=textLen;
	for (i=0; i<textLen; i++) {
		if (((signed char)text[i])<0) {
			n=emDecodeUtf8Char(&c,text+i,textLen-i);
			n--;
			if (n>0) {
				columns-=n;
				i+=n;
			}
		}
	}
	return columns;
}


int emTextFilePanel::PaintText8BitToUtf8(
	const emPainter & painter, double x, double y, double charWidth,
	double charHeight, const char * text, int textLen,
	emColor color, emColor canvasColor
) const
{
	char buf[256];
	int i,l,c,bufPos;

	bufPos=0;
	l=0;
	for (i=0; i<textLen; i++) {
		if (l>=(int)sizeof(buf)-6) {
			painter.PaintText(
				x+bufPos*charWidth,
				y,
				buf,
				charHeight,
				1.0,
				color,
				canvasColor,
				l
			);
			bufPos=i;
			l=0;
		}
		c=(unsigned char)text[i];
		if (c>=128) {
			l+=emEncodeUtf8Char(buf+l,c);
		}
		else {
			buf[l++]=(char)c;
		}
	}
	if (l>0) {
		painter.PaintText(
			x+bufPos*charWidth,
			y,
			buf,
			charHeight,
			1.0,
			color,
			canvasColor,
			l
		);
	}
	return textLen;
}


int emTextFilePanel::PaintTextUtf8To8Bit(
	const emPainter & painter, double x, double y, double charWidth,
	double charHeight, const char * text, int textLen,
	emColor color, emColor canvasColor
) const
{
	char buf[256];
	int i,l,columns,c,n,bufPos;

	columns=textLen;
	bufPos=0;
	l=0;
	for (i=0; i<textLen; i++) {
		if (l>=(int)sizeof(buf)) {
			painter.PaintText(
				x+bufPos*charWidth,
				y,
				buf,
				charHeight,
				1.0,
				color,
				canvasColor,
				l
			);
			bufPos=columns;
			l=0;
		}
		c=(unsigned char)text[i];
		if (c>=128) {
			n=emDecodeUtf8Char(&c,text+i,textLen-i);
			n--;
			if (n>0) {
				columns-=n;
				i+=n;
			}
			if (c>255) c='?';
		}
		buf[l++]=(char)c;
	}
	if (l>0) {
		painter.PaintText(
			x+bufPos*charWidth,
			y,
			buf,
			charHeight,
			1.0,
			color,
			canvasColor,
			l
		);
	}
	return columns;
}


int emTextFilePanel::PaintTextUtf16(
	const emPainter & painter, double x, double y, double charWidth,
	double charHeight, const char * text, int textLen,
	emColor color, emColor canvasColor
) const
{
	char buf[256];
	int i,l,c,c2,pos,bufPos,sh1,sh2;
	bool isUtf8;

	if (Model->GetCharEncoding()==emTextFileModel::CE_UTF16LE) { sh1=0; sh2=8; }
	else { sh1=8; sh2=0; }
	isUtf8=emIsUtf8System();
	pos=0;
	bufPos=0;
	l=0;
	for (i=0; i<textLen; ) {
		if (l>=(int)sizeof(buf)-6) {
			painter.PaintText(
				x+bufPos*charWidth,
				y,
				buf,
				charHeight,
				1.0,
				color,
				canvasColor,
				l
			);
			bufPos=pos;
			l=0;
		}
		c=((((emByte)text[i])<<sh1)|(((emByte)text[i+1])<<sh2));
		i+=2;
		if (c<128) {
			buf[l++]=(char)c;
			pos++;
		}
		else if (c!=0xFEFF) {
			if (c>=0xD800 && c<=0xDBFF && i<textLen) {
				c2=((((emByte)text[i])<<sh1)|(((emByte)text[i+1])<<sh2));
				if (c2>=0xDC00 && c2<=0xDFFF) {
					i+=2;
					c=0x10000+((c&0x03FF)<<10)+(c2&0x03FF);
				}
			}
			if (isUtf8) l+=emEncodeUtf8Char(buf+l,c);
			else if (c<256) buf[l++]=(char)c;
			else buf[l++]='?';
			pos++;
		}
	}
	if (l>0) {
		painter.PaintText(
			x+bufPos*charWidth,
			y,
			buf,
			charHeight,
			1.0,
			color,
			canvasColor,
			l
		);
	}
	return pos;
}


void emTextFilePanel::PaintAsHex(
	const emPainter & painter, emColor canvasColor
)
{
	static const emColor colBg(0,0,0);
	static const emColor colAddr(64,128,64);
	static const emColor colHex(128,128,64);
	static const emColor colAsc(64,96,128);
	static const emColor colAddr64(colAddr,64);
	static const emColor colHex48(colHex,48);
	static const emColor colAsc64(colAsc,64);
	static const emColor colAddr96(colAddr,96);
	static const emColor colHex96(colHex,96);
	char buf[256];
	char buf2[32];
	const char * pStart, * pEnd, * p;
	int i,j,k,count,row,cols,rows,page,pages,pagerows;
	double h,cw,ch,f,t,pagex,gap,pagew,bx,rowy,clipx1,clipy1,clipx2,clipy2;

	count=Model->GetContent().GetCount();
	pStart=Model->GetContent();
	pEnd=pStart+count;

	h=GetHeight();
	clipx1=painter.GetUserClipX1();
	clipy1=painter.GetUserClipY1();
	clipx2=painter.GetUserClipX2();
	clipy2=painter.GetUserClipY2();

	painter.PaintRect(0,0,1,h,colBg,canvasColor);

	rows=(count+15)/16;
	if (!rows) return;
	cols=73;


	f=painter.GetTextSize("X",1.0,false);
	gap=2.0;
	t=0.5*gap/(cols+gap);
	pages=(int)floor(t+sqrt((2*rows/(h*f*gap)+t)*t));
	// pages*h/rows*f*(cols*pages+gap*(pages-1)) <= 1.0
	if (pages<1) {
		pages=1;
		pagerows=rows;
		cw=1.0/cols;
		ch=cw/f;
	}
	else {
		pagerows=(rows+pages-1)/pages;
		ch=h/pagerows;
		cw=ch*f;
	}
	gap*=cw;
	pagew=cols*cw+gap;

	p=pStart;
	page=0;
	pagex=0;
	if (pagex+pagew<=clipx1) {
		page=(int)((clipx1-pagex)/pagew);
		pagex+=page*pagew;
		p+=page*pagerows*16;
	}
	if (ch*GetViewedWidth()<1.0) {
		for (; page<pages && pagex<clipx2; page++, pagex+=pagew) {
			f=(pEnd-p+15)/16*ch;
			if (f>h) f=h;
			painter.PaintRect(
				pagex,
				0,
				cw*8,
				f,
				colAddr64,
				colBg
			);
			painter.PaintRect(
				pagex+cw*9,
				0,
				cw*47,
				f,
				colHex48,
				colBg
			);
			painter.PaintRect(
				pagex+cw*(9+48),
				0,
				cw*16,
				f,
				colAsc64,
				colBg
			);
			p+=16*pagerows;
		}
	}
	else if (ch*GetViewedWidth()<3.0) {
		for (; page<pages && pagex<clipx2; page++, pagex+=pagew) {
			row=0;
			rowy=0;
			if (rowy+ch<=clipy1) {
				row=(int)((clipy1-rowy)/ch);
				rowy+=row*ch;
				p+=row*16;
			}
			while (row<pagerows && rowy<clipy2 && p<pEnd) {
				bx=pagex;
				painter.PaintRect(
					bx,
					rowy+ch*0.1,
					cw*8,
					ch*0.8,
					colAddr96,
					colBg
				);
				bx+=9*cw;
				for (i=0, j=0; i<16 && p<pEnd; i++, p++) {
					k=(unsigned char)*p;
					if (((unsigned)(k-0x20))<0x60) j++;
					painter.PaintRect(
						bx+3*i*cw,
						rowy+ch*0.1,
						cw*2,
						ch*0.8,
						colHex96,
						colBg
					);
				}
				painter.PaintRect(
					bx+48*cw,
					rowy+ch*0.1,
					i*cw,
					ch*0.8,
					emColor(colAsc,(emByte)(32+j*64/i)),
					colBg
				);
				row++;
				rowy+=ch;
			}
			p+=16*(pagerows-row);
		}
	}
	else {
		for (; page<pages && pagex<clipx2; page++, pagex+=pagew) {
			row=0;
			rowy=0;
			if (rowy+ch<=clipy1) {
				row=(int)((clipy1-rowy)/ch);
				rowy+=row*ch;
				p+=row*16;
			}
			while (row<pagerows && rowy<clipy2 && p<pEnd) {
				sprintf(buf,"%08X",(unsigned int)(p-pStart));
				bx=pagex;
				painter.PaintText(bx,rowy,buf,ch,1.0,colAddr,colBg);
				bx+=9*cw;
				for (i=0; i<16 && p<pEnd; i++, p++) {
					k=(unsigned char)*p;
					j=(k>>4)+'0';
					if (j>'9') j+='A'-'9'-1;
					buf[0]=(char)j;
					j=(k&0xF)+'0';
					if (j>'9') j+='A'-'9'-1;
					buf[1]=(char)j;
					if (((unsigned)(k-0x20))>=0x60) k='.';
					buf2[i]=(char)k;
					painter.PaintText(bx+3*i*cw,rowy,buf,ch,1.0,colHex,colBg,2);
				}
				painter.PaintText(bx+48*cw,rowy,buf2,ch,1.0,colAsc,colBg,i);
				row++;
				rowy+=ch;
			}
			p+=16*(pagerows-row);
		}
	}
}
