//------------------------------------------------------------------------------
// emTextField.cpp
//
// Copyright (C) 2005-2011,2014-2016 Oliver Hamann.
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

#include <emCore/emTextField.h>


emTextField::emTextField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon,
	const emString & text, bool editable
)
	: emBorder(parent,name,caption,description,icon)
{
	Clipboard=emClipboard::LookupInherited(GetView());
	if (!Clipboard) {
		emFatalError("emTextField: No emClipboard available.");
	}
	Editable=editable;
	MultiLineMode=false;
	PasswordMode=false;
	OverwriteMode=false;
	Text=text;
	TextLen=Text.GetLen();
	CursorIndex=TextLen;
	SelectionStartIndex=0;
	SelectionEndIndex=0;
	MagicCursorColumn=-1;
	SelectionId=-1;
	CursorBlinkTime=emGetClockMS();
	CursorBlinkOn=true;
	DragMode=DM_NONE;
	DragPosC=0.0;
	DragPosR=0.0;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
}


emTextField::~emTextField()
{
}


void emTextField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		if (editable) {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
		InvalidatePainting();
	}
}


void emTextField::SetMultiLineMode(bool multiLineMode)
{
	if (MultiLineMode!=multiLineMode) {
		MultiLineMode=multiLineMode;
		InvalidatePainting();
	}
}


void emTextField::SetPasswordMode(bool passwordMode)
{
	if (PasswordMode!=passwordMode) {
		PasswordMode=passwordMode;
		InvalidatePainting();
	}
}


void emTextField::SetOverwriteMode(bool overwriteMode)
{
	if (OverwriteMode!=overwriteMode) {
		OverwriteMode=overwriteMode;
		InvalidatePainting();
	}
}


void emTextField::SetText(const emString & text)
{
	if (Text==text) return;
	EmptySelection();
	Text=text;
	TextLen=Text.GetLen();
	CursorIndex=TextLen;
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTextField::SetCursorIndex(int index)
{
	if (index<0) index=0;
	if (index>TextLen) index=TextLen;
	if (CursorIndex!=index) {
		index=GetNormalizedIndex(index);
		if (CursorIndex!=index) {
			CursorIndex=index;
			InvalidatePainting();
		}
	}
}


void emTextField::Select(int startIndex, int endIndex, bool publish)
{
	if (startIndex<0) startIndex=0;
	if (endIndex>TextLen) endIndex=TextLen;
	if (startIndex>=endIndex) {
		startIndex=0;
		endIndex=0;
	}
	if (SelectionStartIndex==startIndex && SelectionEndIndex==endIndex) return;
	startIndex=GetNormalizedIndex(startIndex);
	endIndex=GetNormalizedIndex(endIndex);
	if (SelectionStartIndex==startIndex && SelectionEndIndex==endIndex) return;
	if (SelectionId!=-1) {
		Clipboard->Clear(true,SelectionId);
		SelectionId=-1;
	}
	SelectionStartIndex=startIndex;
	SelectionEndIndex=endIndex;
	InvalidatePainting();
	if (publish) PublishSelection();
	Signal(SelectionSignal);
	SelectionChanged();
}


void emTextField::EmptySelection()
{
	Select(0,0,false);
}


void emTextField::SelectAll(bool publish)
{
	Select(0,TextLen,publish);
}


void emTextField::PublishSelection()
{
	emString str;
	int len;

	len=SelectionEndIndex-SelectionStartIndex;
	if (len>0 && SelectionId==-1) {
		if (PasswordMode) str=emString('*',len);
		else str=Text.GetSubString(SelectionStartIndex,len);
		SelectionId=Clipboard->PutText(str,true);
	}
}


void emTextField::CutSelectedTextToClipboard()
{
	CopySelectedTextToClipboard();
	DeleteSelectedText();
}


void emTextField::CopySelectedTextToClipboard()
{
	emString str;
	int len;

	len=SelectionEndIndex-SelectionStartIndex;
	if (len>0) {
		if (PasswordMode) str=emString('*',len);
		else str=Text.GetSubString(SelectionStartIndex,len);
		Clipboard->PutText(str);
	}
}


void emTextField::PasteSelectedTextFromClipboard()
{
	PasteSelectedText(Clipboard->GetText());
}


void emTextField::PasteSelectedText(const emString & text)
{
	int i,l,d;

	if (text.IsEmpty()) return;
	if (SelectionStartIndex<SelectionEndIndex) {
		i=SelectionStartIndex;
		l=SelectionEndIndex-SelectionStartIndex;
		d=TextLen-SelectionEndIndex;
		EmptySelection();
	}
	else {
		i=CursorIndex;
		l=0;
		d=TextLen-CursorIndex;
	}
	Text.Replace(i,l,text);
	TextLen=Text.GetLen();
	CursorIndex=TextLen-d;
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTextField::DeleteSelectedText()
{
	int i,l;

	i=SelectionStartIndex;
	l=SelectionEndIndex-SelectionStartIndex;
	if (l<=0) return;
	CursorIndex=i;
	EmptySelection();
	Text.Remove(i,l);
	TextLen=Text.GetLen();
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTextField::TextChanged()
{
}


void emTextField::SelectionChanged()
{
}


bool emTextField::Cycle()
{
	emUInt64 clk;
	bool busy;

	clk=emGetClockMS();
	busy=false;

	if (IsInFocusedPath()) {
		if (clk>=CursorBlinkTime+1000) {
			CursorBlinkTime=clk;
			if (!CursorBlinkOn) {
				CursorBlinkOn=true;
				InvalidatePainting();
			}
		}
		else if (clk>=CursorBlinkTime+500) {
			if (CursorBlinkOn) {
				CursorBlinkOn=false;
				InvalidatePainting();
			}
		}
		busy=true;
	}
	else {
		CursorBlinkTime=clk;
		if (!CursorBlinkOn) {
			CursorBlinkOn=true;
			InvalidatePainting();
		}
	}

	if (emBorder::Cycle()) busy=true;
	return busy;
}


void emTextField::Notice(NoticeFlags flags)
{
	if ((flags&(NF_FOCUS_CHANGED))!=0 && IsInFocusedPath()) {
		RestartCursorBlinking();
		WakeUp();
	}
	emBorder::Notice(flags);
}


void emTextField::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	static const double minExt=4.5;
	double mc,mr;
	int col,row,i,i1,i2,j1,j2;
	bool inArea;
	emString str;

	inArea=CheckMouse(mx,my,&mc,&mr);

	switch (DragMode) {
	case DM_NONE:
		if (
			inArea && event.IsKey(EM_KEY_LEFT_BUTTON) &&
			!state.GetAlt() && !state.GetMeta() &&
			GetViewCondition(VCT_MIN_EXT)>=minExt
		) {
			MagicCursorColumn=-1;
			if (state.GetCtrl()) {
				if (IsEditable() && IsEnabled()) {
					i=ColRow2Index(mc,mr,false);
					if (i<SelectionStartIndex || i>=SelectionEndIndex) {
						SetCursorIndex(ColRow2Index(mc,mr,true));
						SetDragMode(DM_INSERT);
					}
					else {
						SetCursorIndex(SelectionEndIndex);
						Index2ColRow(SelectionStartIndex,&col,&row);
						DragPosC=mc-col;
						DragPosR=mr-row;
						SetDragMode(DM_MOVE);
					}
				}
			}
			else if (event.GetRepeat()==0) {
				i=ColRow2Index(mc,mr,true);
				if (state.GetShift()) ModifySelection(i,i,false);
				else EmptySelection();
				SetCursorIndex(i);
				SetDragMode(DM_SELECT);
			}
			else if (event.GetRepeat()==1) {
				i2=GetNextWordBoundaryIndex(ColRow2Index(mc,mr,false));
				i1=GetPrevWordBoundaryIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					SetCursorIndex(i2);
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					SetCursorIndex(i2);
				}
				else {
					ModifySelection(i1,i1,false);
					SetCursorIndex(i1);
				}
				SetDragMode(DM_SELECT_BY_WORDS);
			}
			else if (event.GetRepeat()==2) {
				i2=GetNextRowIndex(ColRow2Index(mc,mr,false));
				i1=GetPrevRowIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					SetCursorIndex(i2);
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					SetCursorIndex(i2);
				}
				else {
					ModifySelection(i1,i1,false);
					SetCursorIndex(i1);
				}
				SetDragMode(DM_SELECT_BY_ROWS);
			}
			else {
				SelectAll(true);
				SetCursorIndex(TextLen);
			}
			RestartCursorBlinking();
			Focus();
			event.Eat();
		}
		break;
	case DM_SELECT:
		i=ColRow2Index(mc,mr,true);
		if (i!=CursorIndex) {
			MagicCursorColumn=-1;
			ModifySelection(CursorIndex,i,false);
			SetCursorIndex(i);
			RestartCursorBlinking();
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_SELECT_BY_WORDS:
		i2=GetNextWordBoundaryIndex(ColRow2Index(mc,mr,false));
		i1=GetPrevWordBoundaryIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			SetCursorIndex(i2);
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (CursorIndex<=j1) j1=GetPrevWordBoundaryIndex(j2);
			else j2=GetNextWordBoundaryIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				SetCursorIndex(i2);
			}
			else {
				Select(i1,j2,false);
				SetCursorIndex(i1);
			}
		}
		MagicCursorColumn=-1;
		RestartCursorBlinking();
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_SELECT_BY_ROWS:
		i2=GetNextRowIndex(ColRow2Index(mc,mr,false));
		i1=GetPrevRowIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			SetCursorIndex(i2);
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (CursorIndex<=j1) j1=GetPrevRowIndex(j2);
			else j2=GetNextRowIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				SetCursorIndex(i2);
			}
			else {
				Select(i1,j2,false);
				SetCursorIndex(i1);
			}
		}
		MagicCursorColumn=-1;
		RestartCursorBlinking();
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_INSERT:
		i=ColRow2Index(mc,mr,true);
		if (i!=CursorIndex) {
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			if (inArea && IsEditable() && IsEnabled()) {
				SelectionId=-1;
				EmptySelection();
				PasteSelectedText(Clipboard->GetText(true));
			}
			SetDragMode(DM_NONE);
		}
		break;
	case DM_MOVE:
		// When extending this for moving the text to somewhere else,
		// don't forget to disable that in password mode.
		i1=SelectionStartIndex;
		i2=SelectionEndIndex;
		if (i1<i2 && IsEditable() && IsEnabled()) {
			str=Text.Extract(i1,i2-i1);
			TextLen=Text.GetLen();
			i=ColRow2Index(mc-DragPosC,mr-DragPosR+0.5,true);
			Text.Insert(i,str);
			TextLen=Text.GetLen();
			if (i!=i1) {
				SelectionStartIndex+=i-i1;
				SelectionEndIndex+=i-i1;
				CursorIndex=SelectionEndIndex;
				MagicCursorColumn=-1;
				RestartCursorBlinking();
				InvalidatePainting();
				Signal(TextSignal);
				TextChanged();
				Signal(SelectionSignal);
				SelectionChanged();
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			SetDragMode(DM_NONE);
		}
		break;
	}

	if (
		!event.IsEmpty() && DragMode==DM_NONE &&
		GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		if (event.IsKey(EM_KEY_CURSOR_LEFT) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=GetPrevWordIndex(CursorIndex);
			else i=GetPrevIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_CURSOR_RIGHT) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=GetNextWordIndex(CursorIndex);
			else i=GetNextIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (MultiLineMode) {
			if (event.IsKey(EM_KEY_CURSOR_UP) && !state.GetAlt() && !state.GetMeta()) {
				if (state.GetCtrl()) {
					i=GetPrevParagraphIndex(CursorIndex);
					MagicCursorColumn=-1;
				}
				else {
					Index2ColRow(CursorIndex,&col,&row);
					if (MagicCursorColumn<0) MagicCursorColumn=col;
					i=ColRow2Index(MagicCursorColumn,row-1,true);
				}
				if (state.GetShift()) ModifySelection(CursorIndex,i,true);
				else EmptySelection();
				SetCursorIndex(i);
				RestartCursorBlinking();
				ScrollToCursor();
				event.Eat();
			}
			if (event.IsKey(EM_KEY_CURSOR_DOWN) && !state.GetAlt() && !state.GetMeta()) {
				if (state.GetCtrl()) {
					i=GetNextParagraphIndex(CursorIndex);
					MagicCursorColumn=-1;
				}
				else {
					Index2ColRow(CursorIndex,&col,&row);
					if (MagicCursorColumn<0) MagicCursorColumn=col;
					i=ColRow2Index(MagicCursorColumn,row+1,true);
				}
				if (state.GetShift()) ModifySelection(CursorIndex,i,true);
				else EmptySelection();
				SetCursorIndex(i);
				RestartCursorBlinking();
				ScrollToCursor();
				event.Eat();
			}
		}
		if (event.IsKey(EM_KEY_HOME) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=0;
			else i=GetRowStartIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_END) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=TextLen;
			else i=GetRowEndIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_A) && state.IsCtrlMod()) {
			SelectAll(true);
			SetCursorIndex(TextLen);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_INSERT) && state.IsNoMod()) {
			SetOverwriteMode(!GetOverwriteMode());
			RestartCursorBlinking();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_INSERT) && state.IsCtrlMod()) ||
		    (event.IsKey(EM_KEY_C) && state.IsCtrlMod())) {
			CopySelectedTextToClipboard();
			RestartCursorBlinking();
			event.Eat();
		}
	}

	if (
		!event.IsEmpty() && DragMode==DM_NONE &&
		IsEditable() && IsEnabled() &&
		GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsNoMod()) {
			if (IsSelectionEmpty()) {
				Select(GetPrevIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsNoMod()) {
			if (IsSelectionEmpty()) {
				Select(CursorIndex,GetNextIndex(CursorIndex),false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_DELETE) && state.IsShiftMod()) ||
		    (event.IsKey(EM_KEY_X) && state.IsCtrlMod())) {
			CutSelectedTextToClipboard();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_INSERT) && state.IsShiftMod()) ||
		    (event.IsKey(EM_KEY_V) && state.IsCtrlMod())) {
			PasteSelectedTextFromClipboard();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(GetPrevWordIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(
					CursorIndex,
					GetNextWordIndex(CursorIndex),
					false
				);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsShiftCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(GetRowStartIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsShiftCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(CursorIndex,GetRowEndIndex(CursorIndex),false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (MultiLineMode && event.IsKey(EM_KEY_ENTER) && state.IsNoMod()) {
			PasteSelectedText("\n");
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (
			!event.GetChars().IsEmpty() &&
			(unsigned char)(event.GetChars()[0])>=32 &&
			event.GetChars()[0]!=127
		) {
			if (
				OverwriteMode && IsSelectionEmpty() &&
				CursorIndex<GetRowEndIndex(CursorIndex)
			) {
				Select(CursorIndex,GetNextIndex(CursorIndex),false);
			}
			PasteSelectedText(event.GetChars());
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
	}

	emBorder::Input(event,state,mx,my);
}


bool emTextField::HasHowTo() const
{
	return true;
}


emString emTextField::GetHowTo() const
{
	emString h;

	h=emBorder::GetHowTo();
	h+=HowToTextField;
	if (MultiLineMode) h+=HowToMultiLineOn; else h+=HowToMultiLineOff;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emTextField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
	DoTextField(TEXT_FIELD_FUNC_PAINT,&painter,canvasColor,0.0,0.0,NULL,NULL,NULL);
}


bool emTextField::CheckMouse(
	double mx, double my, double * pCol, double * pRow
) const
{
	bool b;

	DoTextField(TEXT_FIELD_FUNC_XY2CR,NULL,0,mx,my,pCol,pRow,&b);
	return b;
}


bool emTextField::CheckMouse(
	double mx, double my, double * pCol, double * pRow
)
{
	return ((const emTextField*)this)->CheckMouse(mx,my,pCol,pRow);
}


void emTextField::DoTextField(
	DoTextFieldFunc func, const emPainter * painter, emColor canvasColor,
	double xIn, double yIn, double * pXOut, double * pYOut, bool * pHit
) const
{
	emColor bgColor,fgColor,hlColor,selColor,curColor;
	emString txt;
	double xy[10*2];
	double x,y,w,h,r,ws,d,d1,d2,cx,cy,cw,ch,dx,dy,tx,ty,tw,th;
	int i,i0,j,n,c,rows,cols,col,row,col0,row0,selIdx,selEnd;
	bool selected,selected0;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	d=emMin(h,w)*0.1+r*0.5;
	tx=x+d;
	ty=y+d;
	tw=w-2*d;
	th=h-2*d;

	CalcTotalColsRows(&cols,&rows);
	if (OverwriteMode && IsInFocusedPath()) {
		Index2ColRow(CursorIndex,&col,&row);
		if (col==cols) cols++;
	}
	ch=th/rows;
	cw=emPainter::GetTextSize("X",ch,false);

	ws=1.0;
	if (cw*cols>tw) {
		ws=tw/(cw*cols);
		cw=tw/cols;
		d=0.66;
		if (ws<d) {
			ty+=(ch-ch*ws/d)*0.5;
			ch-=ch-ch*ws/d;
			ws=d;
		}
	}

	if (func!=TEXT_FIELD_FUNC_PAINT) {
		if (func==TEXT_FIELD_FUNC_XY2CR) {
			*pXOut=(xIn-tx)/cw;
			*pYOut=(yIn-ty)/ch;
		}
		else {
			xIn=tx+xIn*cw;
			yIn=ty+yIn*ch;
			*pXOut=xIn;
			*pYOut=yIn;
		}
		dx=emMax(emMax(x-xIn,xIn-x-w)+r,0.0);
		dy=emMax(emMax(y-yIn,yIn-y-h)+r,0.0);
		*pHit = dx*dx+dy*dy <= r*r;
		return;
	}

	if (IsEditable()) {
		bgColor=GetLook().GetInputBgColor();
		fgColor=GetLook().GetInputFgColor();
		hlColor=GetLook().GetInputHlColor();
	}
	else {
		bgColor=GetLook().GetOutputBgColor();
		fgColor=GetLook().GetOutputFgColor();
		hlColor=GetLook().GetOutputHlColor();
	}

	if (!IsEnabled()) {
		bgColor=bgColor.GetBlended(GetLook().GetBgColor(),80.0F);
		fgColor=fgColor.GetBlended(GetLook().GetBgColor(),80.0F);
		hlColor=hlColor.GetBlended(GetLook().GetBgColor(),80.0F);
	}

	selColor=hlColor;
	selIdx=GetSelectionStartIndex();
	selEnd=GetSelectionEndIndex();
	if (selIdx<selEnd) {
		if (!IsInFocusedPath()) {
			selColor=bgColor.GetBlended(fgColor,40.0);
		}
		Index2ColRow(selIdx,&col0,&row0);
		Index2ColRow(selEnd,&col,&row);
		xy[ 0]=tx+col0*cw; xy[ 1]=ty+row0*ch;
		xy[ 2]=tx+tw;      xy[ 3]=ty+row0*ch;
		xy[ 4]=tx+tw;      xy[ 5]=ty+row*ch;
		xy[ 6]=tx+col*cw;  xy[ 7]=ty+row*ch;
		xy[ 8]=tx+col*cw;  xy[ 9]=ty+row*ch+ch;
		xy[10]=tx;         xy[11]=ty+row*ch+ch;
		xy[12]=tx;         xy[13]=ty+row0*ch+ch;
		xy[14]=tx+col0*cw; xy[15]=ty+row0*ch+ch;
		painter->PaintPolygon(xy,8,selColor,canvasColor);
	}

	row0=0;
	col0=0;
	i0=0;
	selected0=(i0>=selIdx && i0<selEnd);
	row=0;
	col=0;
	for (i=0;;) {
		n=emDecodeChar(&c,Text.Get()+i);
		selected=(i>=selIdx && i<selEnd);
		if (
			(
				c<=0x0d &&
				(c==0 || (MultiLineMode && (c==0x09 || c==0x0d || c==0x0a)))
			) ||
			selected0!=selected
		) {
			if (i0<i) {
				if (PasswordMode) {
					for (j=0; j<col-col0; j++) {
						painter->PaintText(
							tx+(col0+j)*cw,ty+row0*ch,"*",ch,ws,
							selected0 ? bgColor : fgColor,
							selected0 ? selColor : canvasColor
						);
					}
				}
				else {
					painter->PaintText(
						tx+col0*cw,ty+row0*ch,Text.Get()+i0,ch,ws,
						selected0 ? bgColor : fgColor,
						selected0 ? selColor : canvasColor,
						i-i0
					);
				}
			}
			if (c==0) break;
			row0=row;
			col0=col;
			i0=i;
			selected0=selected;
		}
		i+=n;
		col++;
		if (c<=0x0d && MultiLineMode) {
			if (c==0x09) {
				col=(col+7)&~7;
				col0=col;
				i0=i;
				selected0=(i0>=selIdx && i0<selEnd);
			}
			else if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[i]==0x0a) i++;
				col=0;
				row++;
				row0=row;
				col0=col;
				i0=i;
				selected0=(i0>=selIdx && i0<selEnd);
			}
		}
	}

	if (IsInFocusedPath()) {
		curColor=fgColor;
		if (!IsEditable()) curColor=curColor.GetTransparented(75.0F);
		else if (!IsCursorBlinkOn()) curColor=curColor.GetTransparented(88.0F);
		Index2ColRow(CursorIndex,&col,&row);
		cx=tx+cw*col;
		cy=ty+ch*row;
		if (GetOverwriteMode()) {
			d=ch*0.07;
			xy[ 0]=cx-d;    xy[ 1]=cy-d;
			xy[ 2]=cx+cw+d; xy[ 3]=cy-d;
			xy[ 4]=cx+cw+d; xy[ 5]=cy+ch+d;
			xy[ 6]=cx-d;    xy[ 7]=cy+ch+d;
			xy[ 8]=cx-d;    xy[ 9]=cy-d;
			xy[10]=cx;      xy[11]=cy;
			xy[12]=cx;      xy[13]=cy+ch;
			xy[14]=cx+cw;   xy[15]=cy+ch;
			xy[16]=cx+cw;   xy[17]=cy;
			xy[18]=cx;      xy[19]=cy;
			painter->PaintPolygon(xy,10,curColor);
		}
		else {
			d=ch*0.07;
			d1=d*0.5;
			d2=d*2.2;
			xy[ 0]=cx-d2; xy[ 1]=cy-d;
			xy[ 2]=cx+d2; xy[ 3]=cy-d;
			xy[ 4]=cx+d1; xy[ 5]=cy;
			xy[ 6]=cx+d1; xy[ 7]=cy+ch;
			xy[ 8]=cx+d2; xy[ 9]=cy+ch+d;
			xy[10]=cx-d2; xy[11]=cy+ch+d;
			xy[12]=cx-d1; xy[13]=cy+ch;
			xy[14]=cx-d1; xy[15]=cy;
			painter->PaintPolygon(xy,8,curColor);
		}
	}
}


void emTextField::SetDragMode(DragModeType dragMode)
{
	if (DragMode!=dragMode) {
		DragMode=dragMode;
		InvalidatePainting();
		RestartCursorBlinking();
	}
}


void emTextField::RestartCursorBlinking()
{
	CursorBlinkTime=emGetClockMS();
	if (!CursorBlinkOn) {
		CursorBlinkOn=true;
		InvalidatePainting();
	}
}


void emTextField::ScrollToCursor()
{
	int col,row;
	double x1,y1,x2,y2,dx,dy;
	bool b;

	if (!IsViewed() || !IsActive()) return;
	Index2ColRow(CursorIndex,&col,&row);
	DoTextField(TEXT_FIELD_FUNC_CR2XY,NULL,0,col-0.5,row-0.2,&x1,&y1,&b);
	DoTextField(TEXT_FIELD_FUNC_CR2XY,NULL,0,col+0.5,row+1.2,&x2,&y2,&b);
	b=false;
	dx=PanelToViewX(x1)-GetView().GetCurrentX();
	if (dx<0.0) b=true;
	else {
		dx=PanelToViewX(x2)-GetView().GetCurrentX()-GetView().GetCurrentWidth();
		if (dx>0.0) b=true; else dx=0.0;
	}
	dy=PanelToViewY(y1)-GetView().GetCurrentY();
	if (dy<0.0) b=true;
	else {
		dy=PanelToViewY(y2)-GetView().GetCurrentY()-GetView().GetCurrentHeight();
		if (dy>0.0) b=true; else dy=0.0;
	}
	if (b) {
		GetView().Scroll(dx,dy);
		if (!IsActive()) Activate();
	}
}


int emTextField::ColRow2Index(double column, double row, bool forCursor) const
{
	int i,j,k,n,c;

	if (!MultiLineMode) {
		for (i=0; ; i+=n, column-=1.0) {
			if (forCursor) { if (column<0.5) break; }
			else           { if (column<1.0) break; }
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0) break;
		}
	}
	else {
		for (j=0, i=0; row>=1.0; ) {
			j+=emDecodeChar(&c,Text.Get()+j);
			if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[j]==0x0a) j++;
				i=j;
				row-=1.0;
			}
			if (c==0) break;
		}
		for (j=0; ; i+=n, j=k) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x0a || c==0x0d || c==0) break;
			k=j+1;
			if (c==0x09) k=(k+7)&~7;
			if (column<=k) {
				if (forCursor) { if (k-column<column-j) i+=n; }
				else           { if (column==k)         i+=n; }
				break;
			}
		}
	}
	return i;
}


void emTextField::Index2ColRow(int index, int * pColumn, int * pRow) const
{
	int i,col,row,n,c;

	if (!MultiLineMode) {
		col=emGetDecodedCharCount(Text,index);
		row=0;
	}
	else {
		col=0;
		row=0;
		for (i=0; i<index; i+=n) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x09) {
				col=(col+8)&~7;
			}
			else if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[i+1]==0x0a) n++;
				col=0;
				row++;
			}
			else if (c==0) {
				break;
			}
			else {
				col++;
			}
		}
	}
	*pColumn=col;
	*pRow=row;
}


void emTextField::CalcTotalColsRows(int * pCols, int * pRows) const
{
	int i,cols,rows,rowcols,n,c;

	if (!MultiLineMode) {
		cols=emGetDecodedCharCount(Text);
		rows=1;
	}
	else {
		cols=0;
		rows=1;
		rowcols=0;
		for (i=0; ; i+=n) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x09) {
				rowcols=(rowcols+8)&~7;
			}
			else if (c==0x0a || c==0x0d) {
				if (cols<rowcols) cols=rowcols;
				if (c==0x0d && Text[i+1]==0x0a) n++;
				rowcols=0;
				rows++;
			}
			else if (c==0) {
				break;
			}
			else {
				rowcols++;
			}
		}
		if (cols<rowcols) cols=rowcols;
	}

	if (cols<1) cols=1;
	if (rows<1) rows=1;
	*pCols=cols;
	*pRows=rows;
}


int emTextField::GetNormalizedIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextIndex(i);
		if (j>index || j==i) return i;
	}
}


void emTextField::ModifySelection(int oldIndex, int newIndex, bool publish)
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


int emTextField::GetNextIndex(int index) const
{
	int c;

	index+=emDecodeChar(&c,Text.Get()+index);
	if (c==0x0d && Text[index]==0x0a && MultiLineMode) index++;
	return index;
}


int emTextField::GetPrevIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTextField::GetNextWordBoundaryIndex(
	int index, bool * pIsDelimiter
) const
{
	const char * p;
	int i,n,c;
	bool prevDelim,delim,first;

	p=Text.Get();
	i=index;
	delim=false;
	first=true;
	for (;;) {
		n=emDecodeChar(&c,p+i);
		if (n<=0) {
			delim=true;
			break;
		}
		prevDelim=delim;
		if (
			PasswordMode ||
			(c>='0' && c<='9') ||
			(c>='A' && c<='Z') ||
			(c>='a' && c<='z') ||
			c=='_' ||
			c>127
		) {
			delim=false;
		}
		else {
			delim=true;
		}
		if (!first && delim!=prevDelim) break;
		i+=n;
		first=false;
	}
	if (pIsDelimiter) *pIsDelimiter=delim;
	return i;
}


int emTextField::GetPrevWordBoundaryIndex(
	int index, bool * pIsDelimiter
) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextWordBoundaryIndex(i,pIsDelimiter);
		if (j>=index || j==i) return i;
	}
}


int emTextField::GetNextWordIndex(int index) const
{
	bool isDelim;

	for (;;) {
		index=GetNextWordBoundaryIndex(index,&isDelim);
		if (!isDelim || index>=TextLen) break;
	}
	return index;
}


int emTextField::GetPrevWordIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextWordIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTextField::GetRowStartIndex(int index) const
{
	int i,j,c;

	if (!MultiLineMode) return 0;
	for (i=0, j=0; ; ) {
		i+=emDecodeChar(&c,Text.Get()+i);
		if (c==0x0d && Text[i]==0x0a) i++;
		if (c==0 || i>index) return j;
		if (c==0x0a || c==0x0d) j=i;
	}
}


int emTextField::GetRowEndIndex(int index) const
{
	int n,c;

	if (!MultiLineMode) return TextLen;
	for (;; index+=n) {
		n=emDecodeChar(&c,Text.Get()+index);
		if (c==0 || c==0x0a || c==0x0d) return index;
	}
}


int emTextField::GetNextRowIndex(int index) const
{
	int c;

	if (!MultiLineMode) return TextLen;
	for (;;) {
		index+=emDecodeChar(&c,Text.Get()+index);
		if (c==0 || c==0x0a || c==0x0d) {
			if (c==0x0d && Text[index]==0x0a) index++;
			return index;
		}
	}
}


int emTextField::GetPrevRowIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextRowIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTextField::GetNextParagraphIndex(int index) const
{
	bool e;

	if (!MultiLineMode) return TextLen;
	for (e=false; index<TextLen; ) {
		index=GetNextRowIndex(index);
		if (Text[index]==0x0a || Text[index]==0x0d) e=true;
		else if (e) break;
	}
	return index;
}


int emTextField::GetPrevParagraphIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextParagraphIndex(i);
		if (j>=index || j==i) return i;
	}
}


const char * emTextField::HowToTextField=
	"\n"
	"\n"
	"TEXT FIELD\n"
	"\n"
	"This is a text field. In such a field, a text can be viewed and edited.\n"
	"\n"
	"Quick hint about an incompatibility against other user interfaces: For inserting\n"
	"selected text, press Ctrl + left mouse button instead of the middle mouse\n"
	"button.\n"
	"\n"
	"Mouse control:\n"
	"\n"
	"  Left-Button-Click        - Set cursor position, clear selection.\n"
	"\n"
	"  Left-Button-Double-Click - Select a word.\n"
	"\n"
	"  Left-Button-Triple-Click - Select a row.\n"
	"\n"
	"  Left-Button-Quad-Click   - Select all.\n"
	"\n"
	"  Left-Button-Drag         - Select passed characters.\n"
	"\n"
	"  Shift+Left-Button-Drag   - Extend or reduce selection by passed characters.\n"
	"\n"
	"  Ctrl+Left-Button-Click on non-selected area - Insert a copy of common selected\n"
	"                                                text.\n"
	"\n"
	"  Ctrl+Left-Button-Drag on selected area      - Move selected text.\n"
	"\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Normal key input inserts the corresponding character at the cursor position.\n"
	"  Any selected text is replaced by the character. Special key combinations are:\n"
	"\n"
	"  Cursor-Keys             - Move the cursor.\n"
	"\n"
	"  Ctrl+Cursor-Keys        - Move the cursor by words or paragraphs.\n"
	"\n"
	"  Home or End             - Move the cursor to beginning or end of row.\n"
	"\n"
	"  Ctrl+Home or Ctrl+End   - Move the cursor to beginning or end of all.\n"
	"\n"
	"  Shift+<Cursor Movement> - Select text: Hold the Shift key while moving the\n"
	"                            cursor with one of the above key combinations, to\n"
	"                            select the passed characters.\n"
	"\n"
	"  Ctrl+A                  - Select the whole text.\n"
	"\n"
	"  Insert                  - Switch between insert mode and replace mode.\n"
	"\n"
	"  Backspace               - Delete the selected text, or the character on the\n"
	"                            left side of the cursor.\n"
	"\n"
	"  Delete                  - Delete the selected text, or the character on the\n"
	"                            right side of the cursor.\n"
	"\n"
	"  Shift+Delete or Ctrl+X  - Cut operation: Copy the selected text to the\n"
	"                            clipboard and delete it.\n"
	"\n"
	"  Ctrl+Insert or Ctrl+C   - Copy operation: Copy the selected text to the\n"
	"                            clipboard.\n"
	"\n"
	"  Shift+Insert or Ctrl+V  - Paste operation: Insert text from the clipboard. Any\n"
	"                            selected text is replaced by the insertion.\n"
	"\n"
	"  Ctrl+Backspace          - Delete to the left until beginning of a word.\n"
	"\n"
	"  Ctrl+Delete             - Delete to the right until beginning of a word.\n"
	"\n"
	"  Shift+Ctrl+Backspace    - Delete all on the left side of the cursor.\n"
	"\n"
	"  Shift+Ctrl+Delete       - Delete all on the right side of the cursor.\n"
;


const char * emTextField::HowToMultiLineOff=
	"\n"
	"\n"
	"MULTI-LINE: DISABLED\n"
	"\n"
	"This text field has the multi-line mode disabled. You can view or edit only\n"
	"a single line.\n"
;


const char * emTextField::HowToMultiLineOn=
	"\n"
	"\n"
	"MULTI-LINE: ENABLED\n"
	"\n"
	"This text field has the multi-line mode enabled. You may view or edit multiple\n"
	"lines.\n"
;


const char * emTextField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This text field is read-only. You cannot edit the text.\n"
;
