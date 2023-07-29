//------------------------------------------------------------------------------
// emTextFilePanel.cpp
//
// Copyright (C) 2004-2010,2014-2019,2021-2023 Oliver Hamann.
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
#include <emText/emTextFileControlPanel.h>


emTextFilePanel::emTextFilePanel(
	ParentArg parent, const emString & name, emTextFileModel * fileModel,
	bool updateFileModel, bool alternativeView
)
	: emFilePanel(parent,name)
{
	AlternativeView=alternativeView;
	Model=NULL;
	Clipboard=emClipboard::LookupInherited(GetView());
	if (!Clipboard) {
		emFatalError("emTextFilePanel: No emClipboard available.");
	}
	SelectionStartIndex=0;
	SelectionEndIndex=0;
	SelectionId=-1;
	DragMode=DM_NONE;
	DragIndex=0;
	AddWakeUpSignal(GetVirFileStateSignal());
	SetFileModel(fileModel,updateFileModel);
	UpdateTextLayout();
}


emTextFilePanel::~emTextFilePanel()
{
}


void emTextFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (Model) RemoveWakeUpSignal(Model->GetChangeSignal());
	SelectionId=-1;
	EmptySelection();
	Model=dynamic_cast<emTextFileModel*>(fileModel);
	emFilePanel::SetFileModel(Model,updateFileModel);
	if (Model) AddWakeUpSignal(Model->GetChangeSignal());
	InvalidateControlPanel();
}


emString emTextFilePanel::GetIconFileName() const
{
	if (IsVFSGood()) {
		if (Model->GetCharEncoding()!=emTextFileModel::CE_BINARY) {
			return "plain_text.tga";
		}
	}
	return emFilePanel::GetIconFileName();
}


bool emTextFilePanel::IsHexView() const
{
	return AlternativeView || Model->GetCharEncoding()==emTextFileModel::CE_BINARY;
}


void emTextFilePanel::Select(int startIndex, int endIndex, bool publish)
{
	int textLen;

	textLen=0;
	if (IsVFSGood() && !IsHexView()) textLen=Model->GetContent().GetCount();
	if (startIndex<0) startIndex=0;
	if (endIndex>textLen) endIndex=textLen;
	if (startIndex>=endIndex) {
		startIndex=0;
		endIndex=0;
		publish=false;
	}
	if (
		SelectionStartIndex==startIndex && SelectionEndIndex==endIndex &&
		publish==(SelectionId!=-1)
	) return;
	if (SelectionId!=-1) {
		Clipboard->Clear(true,SelectionId);
		SelectionId=-1;
	}
	SelectionStartIndex=startIndex;
	SelectionEndIndex=endIndex;
	InvalidatePainting();
	if (publish) PublishSelection();
	Signal(SelectionSignal);
}


void emTextFilePanel::SelectAll(bool publish)
{
	if (IsVFSGood() && !IsHexView()) {
		Select(0,Model->GetContent().GetCount(),publish);
	}
}


void emTextFilePanel::EmptySelection()
{
	Select(0,0,false);
}


void emTextFilePanel::PublishSelection()
{
	emString str;

	if (SelectionId==-1) {
		str=ConvertSelectedTextToCurrentLocale();
		if (!str.IsEmpty()) SelectionId=Clipboard->PutText(str,true);
	}
}


void emTextFilePanel::CopySelectedTextToClipboard()
{
	emString str;

	str=ConvertSelectedTextToCurrentLocale();
	if (!str.IsEmpty()) Clipboard->PutText(str);
}


bool emTextFilePanel::Cycle()
{
	static const char * const ALT_ERROR="Hex display is not an alternative.";

	if (IsSignaled(GetVirFileStateSignal())) {
		UpdateTextLayout();
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
		if (!IsVFSGood() || IsHexView()) {
			SelectionId=-1;
			EmptySelection();
		}
	}
	if (Model && IsSignaled(Model->GetChangeSignal())) {
		SelectionId=-1;
		EmptySelection();
	}

	return emFilePanel::Cycle();
}


void emTextFilePanel::Notice(NoticeFlags flags)
{
	emFilePanel::Notice(flags);

	if (flags&NF_LAYOUT_CHANGED) {
		UpdateTextLayout();
	}
}


void emTextFilePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double mc,mr;
	int i,i1,i2,j1,j2;
	bool inArea;
	emString str;

	if (!IsVFSGood() || IsHexView()) {
		SetDragMode(DM_NONE);
		emFilePanel::Input(event,state,mx,my);
		return;
	}

	inArea=CheckMouse(mx,my,&mc,&mr);
	if (inArea && DragMode==DM_NONE) SetDragMode(DM_OVER_TEXT);
	if (!inArea && DragMode==DM_OVER_TEXT) SetDragMode(DM_NONE);

	switch (DragMode) {
	case DM_NONE:
		break;
	case DM_OVER_TEXT:
		if (
			event.IsKey(EM_KEY_LEFT_BUTTON) &&
			!state.GetCtrl() && !state.GetAlt() && !state.GetMeta()
		) {
			if (event.GetRepeat()==0) {
				i=Model->ColRow2Index(mc,mr,true);
				if (state.GetShift()) ModifySelection(i,i,false);
				else EmptySelection();
				DragIndex=i;
				SetDragMode(DM_SELECT);
			}
			else if (event.GetRepeat()==1) {
				i2=Model->GetNextWordBoundaryIndex(Model->ColRow2Index(mc,mr,false));
				i1=Model->GetPrevWordBoundaryIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					DragIndex=i2;
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					DragIndex=i2;
				}
				else {
					ModifySelection(i1,i1,false);
					DragIndex=i1;
				}
				SetDragMode(DM_SELECT_BY_WORDS);
			}
			else if (event.GetRepeat()==2) {
				i2=Model->GetNextRowIndex(Model->ColRow2Index(mc,mr,false));
				i1=Model->GetPrevRowIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					DragIndex=i2;
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					DragIndex=i2;
				}
				else {
					ModifySelection(i1,i1,false);
					DragIndex=i1;
				}
				SetDragMode(DM_SELECT_BY_ROWS);
			}
			else {
				SelectAll(true);
				DragIndex=Model->GetContent().GetCount();
			}
			Focus();
			event.Eat();
		}
		break;
	case DM_SELECT:
		i=Model->ColRow2Index(mc,mr,true);
		if (i!=DragIndex) {
			ModifySelection(DragIndex,i,false);
			DragIndex=i;
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(inArea?DM_OVER_TEXT:DM_NONE);
		}
		break;
	case DM_SELECT_BY_WORDS:
		i2=Model->GetNextWordBoundaryIndex(Model->ColRow2Index(mc,mr,false));
		i1=Model->GetPrevWordBoundaryIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			DragIndex=i2;
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (DragIndex<=j1) j1=Model->GetPrevWordBoundaryIndex(j2);
			else j2=Model->GetNextWordBoundaryIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				DragIndex=i2;
			}
			else {
				Select(i1,j2,false);
				DragIndex=i1;
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(inArea?DM_OVER_TEXT:DM_NONE);
		}
		break;
	case DM_SELECT_BY_ROWS:
		i2=Model->GetNextRowIndex(Model->ColRow2Index(mc,mr,false));
		i1=Model->GetPrevRowIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			DragIndex=i2;
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (DragIndex<=j1) j1=Model->GetPrevRowIndex(j2);
			else j2=Model->GetNextRowIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				DragIndex=i2;
			}
			else {
				Select(i1,j2,false);
				DragIndex=i1;
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(inArea?DM_OVER_TEXT:DM_NONE);
		}
		break;
	}

	if (
		!event.IsEmpty() &&
		(DragMode==DM_NONE || DragMode==DM_OVER_TEXT)
	) {
		if (event.IsKey(EM_KEY_A) && state.IsCtrlMod()) {
			SelectAll(true);
			event.Eat();
		}
		if (event.IsKey(EM_KEY_A) && state.IsShiftCtrlMod()) {
			EmptySelection();
			event.Eat();
		}
		if (
			(event.IsKey(EM_KEY_INSERT) && state.IsCtrlMod()) ||
			(event.IsKey(EM_KEY_C) && state.IsCtrlMod())
		) {
			CopySelectedTextToClipboard();
			event.Eat();
		}
	}

	emFilePanel::Input(event,state,mx,my);
}


emCursor emTextFilePanel::GetCursor() const
{
	if (DragMode==DM_NONE) return emFilePanel::GetCursor();
	return emCursor::TEXT;
}


bool emTextFilePanel::IsOpaque() const
{
	if (IsVFSGood()) {
		return false;
	}
	else {
		return emFilePanel::IsOpaque();
	}
}


void emTextFilePanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	if (IsVFSGood()) {

		emPainter::UserSpaceLeaveGuard userSpaceLeaveGuard(painter); //!!!

		if (IsHexView()) {
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
	return new emTextFileControlPanel(parent,name,*this);
}


void emTextFilePanel::SetDragMode(DragModeType dragMode)
{
	if (DragMode!=dragMode) {
		DragMode=dragMode;
		InvalidateCursor();
	}
}


void emTextFilePanel::UpdateTextLayout()
{
	int count,rows;
	double h,f,t;

	if (!IsVFSGood()) {
		PageCount=PageRows=PageCols=0;
		PageWidth=PageGap=CharWidth=CharHeight=0.0;
	}
	else if (IsHexView()) {
		h=GetHeight();
		count=Model->GetContent().GetCount();
		rows=(int)(((unsigned)count+15)/16);
		PageCols=73;
		f=emPainter::GetTextSize("X",1.0,false);
		PageGap=2.0;
		t=0.5*PageGap/(PageCols+PageGap);
		PageCount=(int)floor(t+sqrt((2*rows/(h*f*PageGap)+t)*t));
		// PageCount*h/rows*f*(PageCols*PageCount+PageGap*(PageCount-1)) <= 1.0
		if (PageCount<1) {
			PageCount=1;
			PageRows=rows;
			CharWidth=1.0/PageCols;
			CharHeight=CharWidth/f;
		}
		else {
			PageRows=(rows+PageCount-1)/PageCount;
			CharHeight=h/PageRows;
			CharWidth=CharHeight*f;
		}
		PageGap*=CharWidth;
		PageWidth=PageCols*CharWidth;
	}
	else {
		h=GetHeight();
		rows=Model->GetLineCount();
		PageCols=Model->GetColumnCount();
		if (PageCols<8) PageCols=8;
		f=emPainter::GetTextSize("X",1.0,false);
		PageGap=1.0;
		t=0.5*PageGap/(PageCols+PageGap);
		PageCount=(int)floor(t+sqrt((2*rows/(h*f*PageGap)+t)*t));
		// PageCount*h/rows*f*(PageCols*PageCount+PageGap*(PageCount-1)) <= 1.0
		if (PageCount<1) {
			PageCount=1;
			PageRows=rows;
			CharWidth=1.0/PageCols;
			CharHeight=CharWidth/f;
			PageWidth=1.0;
			PageGap*=CharWidth;
		}
		else {
			PageRows=(rows+PageCount-1)/PageCount;
			CharHeight=h/PageRows;
			CharWidth=CharHeight*f;
			PageGap*=CharWidth;
			PageWidth=(1.0-(PageCount-1)*PageGap)/PageCount;
		}
	}
}


bool emTextFilePanel::CheckMouse(
	double mx, double my, double * pCol, double * pRow
) const
{
	bool inArea;
	double t;
	int pg;

	*pCol=0.0;
	*pRow=0.0;
	if (!IsVFSGood() || IsHexView()) return false;

	inArea=true;
	t=mx/(PageWidth+PageGap);
	if (t<0.0) {
		pg=0;
		inArea=false;
	}
	else if (t>=PageCount) {
		pg=PageCount-1;
		inArea=false;
	}
	else {
		pg=(int)t;
	}

	t=mx-pg*(PageWidth+PageGap);
	if (t>PageWidth+PageGap*0.5 && pg+1<PageCount) {
		t-=PageWidth+PageGap;
		pg++;
	}
	if (t<0.0) {
		*pCol=0.0;
		inArea=false;
	}
	else if (t>=PageCols*CharWidth) {
		*pCol=PageCols;
		if (t>=PageWidth) inArea=false;
	}
	else {
		*pCol=t/CharWidth;
	}

	t=my;
	if (t<0.0) {
		t=0.0;
		inArea=false;
	}
	else if (t>=GetHeight()) {
		t=GetHeight();
		inArea=false;
	}
	*pRow=emMin(pg*PageRows+t/CharHeight,(double)Model->GetLineCount());

	return inArea;
}


void emTextFilePanel::ModifySelection(int oldIndex, int newIndex, bool publish)
{
	int d1,d2;

	if (SelectionStartIndex<SelectionEndIndex) {
		d1=oldIndex-SelectionStartIndex; if (d1<0) d1=-d1;
		d2=oldIndex-SelectionEndIndex; if (d2<0) d2=-d2;
		if (d1<d2) oldIndex=SelectionEndIndex;
		else oldIndex=SelectionStartIndex;
	}
	if (oldIndex<newIndex) Select(oldIndex,newIndex,publish);
	else Select(newIndex,oldIndex,publish);
}


emString emTextFilePanel::ConvertSelectedTextToCurrentLocale() const
{
	const char * data;
	int len,i1,i2;

	if (!IsVFSGood() || IsHexView()) return emString();
	data=Model->GetContent().Get();
	len=Model->GetContent().GetCount();
	i1=SelectionStartIndex;
	i2=SelectionEndIndex;
	if (i2>len) i2=len;
	if (i1<0) i1=0;
	if (i1>=i2) return emString();
	return Model->ConvertToCurrentLocale(data+i1,data+i2);
}


void emTextFilePanel::PaintAsText(
	const emPainter & painter, emColor canvasColor
) const
{
	double h,x,y,clipx1,clipy1,clipx2,clipy2;
	int row,endRow,rows,page;

	h=GetHeight();

	clipx1=painter.GetUserClipX1();
	clipy1=painter.GetUserClipY1();
	clipx2=painter.GetUserClipX2();
	clipy2=painter.GetUserClipY2();

	rows=Model->GetLineCount();

	page=(int)(clipx1/(PageWidth+PageGap));
	if (page<0) page=0;
	x=page*(PageWidth+PageGap);
	for (; page<PageCount && x<clipx2; page++, x+=PageWidth+PageGap) {

		painter.PaintRect(
			x,
			0.0,
			PageWidth,
			h,
			TextBgColor,
			canvasColor
		);

		row=(int)(clipy1/CharHeight);
		if (row<0) row=0;
		y=row*CharHeight;
		row+=page*PageRows;
		endRow=(int)ceil(clipy2/CharHeight);
		if (endRow>PageRows) endRow=PageRows;
		endRow+=page*PageRows;
		if (endRow>rows) endRow=rows;

		if (CharHeight*GetViewedWidth()<0.5) {
			PaintTextRowsSilhouette(painter,x,y,row,endRow);
		}
		else {
			PaintTextRows(painter,x,y,row,endRow);
		}
	}
}


void emTextFilePanel::PaintTextRowsSilhouette(
	const emPainter & painter, double x, double y, int row, int endRow
) const
{
	emColor fg,bg;
	double xfac;
	int step,selRow1,selRow2;

	step=(int)(0.5/(CharHeight*GetViewedWidth()));
	if (step<1) step=1;
	row=((row-1)/step+1)*step;

	selRow1=selRow2=0;
	if (SelectionStartIndex<SelectionEndIndex) {
		selRow1=Model->Index2Row(SelectionStartIndex);
		selRow2=Model->Index2Row(SelectionEndIndex);
		if (selRow1<selRow2) {
			selRow2=emMax(selRow2,selRow1+step);
		}
		else {
			selRow1=selRow2=0;
		}
	}

	xfac=PageCols*CharWidth/255.0;
	while (row<endRow) {
		if (row<selRow2 && row>=selRow1) {
			painter.PaintRect(
				x,
				y,
				PageWidth,
				CharHeight*step,
				TextSelBgColor,
				TextBgColor
			);
			fg=TextSelFg96Color;
			bg=TextSelBgColor;
		}
		else {
			fg=TextFg96Color;
			bg=TextBgColor;
		}
		painter.PaintRect(
			x+Model->GetRelativeLineIndent(row)*xfac,
			y+CharHeight*0.1,
			Model->GetRelativeLineWidth(row)*xfac,
			CharHeight*step*0.8,
			fg,
			bg
		);
		y+=CharHeight*step;
		row+=step;
	}
}


void emTextFilePanel::PaintTextRows(
	const emPainter & painter, double x, double y, int row, int endRow
) const
{
	const char * pContent;
	int i1,i2,i3,col;

	pContent=Model->GetContent();
	for (; row<endRow; row++, y+=CharHeight) {
		i1=Model->GetLineStart(row);
		i2=Model->GetLineEnd(row);
		emMBState mbState;
		if (
			SelectionStartIndex>=SelectionEndIndex ||
			SelectionStartIndex>=i2 ||
			SelectionEndIndex<=i1
		) {
			if (i1<i2) {
				PaintTextRowPart(
					painter,x,y,0,pContent+i1,pContent+i2,&mbState,
					TextFgColor,TextBgColor,TextBgColor
				);
			}
		}
		else {
			col=0;
			if (i1<SelectionStartIndex) {
				col=PaintTextRowPart(
					painter,x,y,col,pContent+i1,pContent+SelectionStartIndex,
					&mbState,TextFgColor,TextBgColor,TextBgColor
				);
				i1=SelectionStartIndex;
			}
			if (i1<SelectionEndIndex) {
				i3=emMin(SelectionEndIndex,i2);
				col=PaintTextRowPart(
					painter,x,y,col,pContent+i1,pContent+i3,
					&mbState,TextSelFgColor,TextSelBgColor,TextBgColor
				);
				i1=i3;
			}
			if (i1<i2) {
				col=PaintTextRowPart(
					painter,x,y,col,pContent+i1,pContent+i2,
					&mbState,TextFgColor,TextBgColor,TextBgColor
				);
			}
			if (i2<SelectionEndIndex) {
				painter.PaintRect(
					x+col*CharWidth,
					y,
					PageWidth-col*CharWidth,
					CharHeight,
					TextSelBgColor,
					TextBgColor
				);
			}
		}
	}
}


int emTextFilePanel::PaintTextRowPart(
	const emPainter & painter, double rowX, double rowY, int column,
	const char * src, const char * srcEnd, emMBState * mbState,
	emColor fgColor, emColor bgColor, emColor canvasColor
) const
{
	char buf[512+EM_MB_LEN_MAX];
	const char * p, * q, * e;
	int len,c1,c2;
	bool sameCharEncoding;

	sameCharEncoding=Model->IsSameCharEncoding();

	while (src<srcEnd) {
		if (sameCharEncoding) {
			p=src;
			e=srcEnd;
			src=srcEnd;
		}
		else {
			len=Model->ConvertToCurrentLocale(
				buf,sizeof(buf),&src,srcEnd,mbState
			);
			if (len<=0) break;
			p=buf;
			e=buf+len;
		}
		for (;;) {
			c1=column;
			while (p<e && *p==0x09) {
				column=(column+8)&~7;
				p++;
			}
			q=p;
			while (p<e && *p!=0x09) p++;
			c2=column;
			column+=emGetDecodedCharCount(q,p-q);
			if (bgColor!=canvasColor) {
				painter.PaintRect(
					rowX+c1*CharWidth,
					rowY,
					(column-c1)*CharWidth,
					CharHeight,
					bgColor,
					canvasColor
				);
			}
			if (q>=p) break;
			painter.PaintText(
				rowX+c2*CharWidth,
				rowY,
				q,
				CharHeight,
				1.0,
				fgColor,
				bgColor,
				p-q
			);
		}
	}
	return column;
}


void emTextFilePanel::PaintAsHex(
	const emPainter & painter, emColor canvasColor
) const
{
	char buf[256];
	char buf2[32];
	const char * pStart, * pEnd, * p;
	int i,j,k,row,page;
	double h,f,pagex,bx,rowy,clipx1,clipy1,clipx2,clipy2;

	pStart=Model->GetContent();
	pEnd=pStart+Model->GetContent().GetCount();

	h=GetHeight();
	clipx1=painter.GetUserClipX1();
	clipy1=painter.GetUserClipY1();
	clipx2=painter.GetUserClipX2();
	clipy2=painter.GetUserClipY2();

	painter.PaintRect(0,0,1,h,HexBgColor,canvasColor);

	p=pStart;
	page=0;
	pagex=0;
	if (pagex+PageWidth+PageGap<=clipx1) {
		page=(int)((clipx1-pagex)/(PageWidth+PageGap));
		pagex+=page*(PageWidth+PageGap);
		p+=page*PageRows*16;
	}
	if (CharHeight*GetViewedWidth()<1.0) {
		for (; page<PageCount && pagex<clipx2; page++, pagex+=PageWidth+PageGap) {
			f=(pEnd-p+15)/16*CharHeight;
			if (f>h) f=h;
			painter.PaintRect(
				pagex,
				0,
				CharWidth*8,
				f,
				HexAddr64Color,
				HexBgColor
			);
			painter.PaintRect(
				pagex+CharWidth*9,
				0,
				CharWidth*47,
				f,
				HexData48Color,
				HexBgColor
			);
			painter.PaintRect(
				pagex+CharWidth*(9+48),
				0,
				CharWidth*16,
				f,
				HexAsc64Color,
				HexBgColor
			);
			p+=16*PageRows;
		}
	}
	else if (CharHeight*GetViewedWidth()<3.0) {
		for (; page<PageCount && pagex<clipx2; page++, pagex+=PageWidth+PageGap) {
			row=0;
			rowy=0;
			if (rowy+CharHeight<=clipy1) {
				row=(int)((clipy1-rowy)/CharHeight);
				rowy+=row*CharHeight;
				p+=row*16;
			}
			while (row<PageRows && rowy<clipy2 && p<pEnd) {
				bx=pagex;
				painter.PaintRect(
					bx,
					rowy+CharHeight*0.1,
					CharWidth*8,
					CharHeight*0.8,
					HexAddr96Color,
					HexBgColor
				);
				bx+=9*CharWidth;
				for (i=0, j=0; i<16 && p<pEnd; i++, p++) {
					k=(unsigned char)*p;
					if (((unsigned)(k-0x20))<0x60) j++;
					painter.PaintRect(
						bx+3*i*CharWidth,
						rowy+CharHeight*0.1,
						CharWidth*2,
						CharHeight*0.8,
						HexData96Color,
						HexBgColor
					);
				}
				painter.PaintRect(
					bx+48*CharWidth,
					rowy+CharHeight*0.1,
					i*CharWidth,
					CharHeight*0.8,
					emColor(HexAscColor,(emByte)(32+j*64/i)),
					HexBgColor
				);
				row++;
				rowy+=CharHeight;
			}
			p+=16*(PageRows-row);
		}
	}
	else {
		for (; page<PageCount && pagex<clipx2; page++, pagex+=PageWidth+PageGap) {
			row=0;
			rowy=0;
			if (rowy+CharHeight<=clipy1) {
				row=(int)((clipy1-rowy)/CharHeight);
				rowy+=row*CharHeight;
				p+=row*16;
			}
			while (row<PageRows && rowy<clipy2 && p<pEnd) {
				sprintf(buf,"%08X",(unsigned int)(p-pStart));
				bx=pagex;
				painter.PaintText(
					bx,rowy,buf,CharHeight,1.0,HexAddrColor,HexBgColor
				);
				bx+=9*CharWidth;
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
					painter.PaintText(
						bx+3*i*CharWidth,rowy,buf,CharHeight,1.0,
						HexDataColor,HexBgColor,2
					);
				}
				painter.PaintText(
					bx+48*CharWidth,rowy,buf2,CharHeight,1.0,
					HexAscColor,HexBgColor,i
				);
				row++;
				rowy+=CharHeight;
			}
			p+=16*(PageRows-row);
		}
	}
}


const emColor emTextFilePanel::TextBgColor(255,255,255);
const emColor emTextFilePanel::TextFgColor(0,0,0);
const emColor emTextFilePanel::TextFg96Color(TextFgColor,96);
const emColor emTextFilePanel::TextSelFgColor(TextBgColor);
const emColor emTextFilePanel::TextSelFg96Color(TextSelFgColor,96);
const emColor emTextFilePanel::TextSelBgColor(16,56,192);
const emColor emTextFilePanel::HexBgColor(0,0,0);
const emColor emTextFilePanel::HexAddrColor(64,128,64);
const emColor emTextFilePanel::HexDataColor(128,128,64);
const emColor emTextFilePanel::HexAscColor(64,96,128);
const emColor emTextFilePanel::HexAddr64Color(HexAddrColor,64);
const emColor emTextFilePanel::HexData48Color(HexDataColor,48);
const emColor emTextFilePanel::HexAsc64Color(HexAscColor,64);
const emColor emTextFilePanel::HexAddr96Color(HexAddrColor,96);
const emColor emTextFilePanel::HexData96Color(HexDataColor,96);
