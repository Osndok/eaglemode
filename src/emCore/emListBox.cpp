//------------------------------------------------------------------------------
// emListBox.cpp
//
// Copyright (C) 2015-2016 Oliver Hamann.
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

#include <emCore/emListBox.h>
#include <ctype.h>


emListBox::emListBox(
	ParentArg parent, const emString & name,
	const emString & caption,
	const emString & description,
	const emImage & icon,
	SelectionType selType
) :
	emRasterGroup(parent,name,caption,description,icon)
{
	SelType=selType;
	Items.SetTuningLevel(1);
	SelectedItemIndices.SetTuningLevel(4);
	TriggeredItemIndex=-1;
	PrevInputItemIndex=-1;
	KeyWalkClock=0;

	SetBorderType(OBT_INSTRUMENT,IBT_INPUT_FIELD);
}


emListBox::~emListBox()
{
}


void emListBox::SetSelectionType(SelectionType selType)
{
	if (SelType!=selType) {
		SelType=selType;
		if (SelType==READY_ONLY_SELECTION) {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
	}
}


void emListBox::AddItem(const emString & text, const emAnything & data)
{
	InsertItem(Items.GetCount(),text,data);
}


void emListBox::InsertItem(
	int index, const emString & text, const emAnything & data
)
{
	Item * item;
	bool selectionChanged;
	int i,j;

	if (index<0) index=0;
	if (index>Items.GetCount()) index=Items.GetCount();
	Items.InsertNew(index);
	item=&Items.GetWritable(index);
	item->Text=text;
	item->Data=data;
	item->Selected=false;
	selectionChanged=false;
	for (i=SelectedItemIndices.GetCount()-1; i>=0; i--) {
		j=SelectedItemIndices[i];
		if (j<index) break;
		SelectedItemIndices.Set(i,j+1);
		selectionChanged=true;
	}
	if (index<=TriggeredItemIndex) TriggeredItemIndex++;
	if (index<=PrevInputItemIndex) PrevInputItemIndex++;
	KeyWalkChars.Clear();
	if (selectionChanged) Signal(SelectionSignal);
	InvalidateAutoExpansion();
}


void emListBox::RemoveItem(int index)
{
	bool selectionChanged;
	int i,j;

	if (index>=0 && index<Items.GetCount()) {
		Items.Remove(index);
		selectionChanged=false;
		for (i=SelectedItemIndices.GetCount()-1; i>=0; i--) {
			j=SelectedItemIndices[i];
			if (j<=index) {
				if (j==index) {
					SelectedItemIndices.Remove(i);
					selectionChanged=true;
				}
				break;
			}
			SelectedItemIndices.Set(i,j-1);
			selectionChanged=true;
		}
		if (index<=TriggeredItemIndex) {
			if (index==TriggeredItemIndex) TriggeredItemIndex=-1;
			else TriggeredItemIndex--;
		}
		if (index<=PrevInputItemIndex) {
			if (index==PrevInputItemIndex) PrevInputItemIndex=-1;
			else PrevInputItemIndex--;
		}
		KeyWalkChars.Clear();
		if (selectionChanged) Signal(SelectionSignal);
		InvalidateAutoExpansion();
	}
}


void emListBox::ClearItems()
{
	if (!Items.IsEmpty()) {
		Items.Clear();
		TriggeredItemIndex=-1;
		PrevInputItemIndex=-1;
		if (!SelectedItemIndices.IsEmpty()) {
			SelectedItemIndices.Clear();
			Signal(SelectionSignal);
		}
		KeyWalkChars.Clear();
		InvalidateAutoExpansion();
	}
}


emString emListBox::GetItemText(int index) const
{
	if (index>=0 && index<Items.GetCount()) return Items[index].Text;
	else return emString();
}


void emListBox::SetItemText(int index, const emString & text)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount()) {
		if (Items[index].Text != text) {
			Items.GetWritable(index).Text=text;
			KeyWalkChars.Clear();
			ipf=GetItemPanelInterface(index);
			if (ipf) ipf->ItemTextChanged();
		}
	}
}


emAnything emListBox::GetItemData(int index) const
{
	if (index>=0 && index<Items.GetCount()) return Items[index].Data;
	else return emAnything();
}


void emListBox::SetItemData(int index, const emAnything & data)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount()) {
		Items.GetWritable(index).Data=data;
		ipf=GetItemPanelInterface(index);
		if (ipf) ipf->ItemDataChanged();
	}
}


void emListBox::SetSelectedIndices(const emArray<int> & itemIndices)
{
	emArray<int> sortedArray;
	int i,idx;

	sortedArray=itemIndices;
	if (sortedArray.GetCount()>1) {
		sortedArray.Sort(emStdComparer<int>::Compare);
	}
	for (i=0; i<SelectedItemIndices.GetCount();) {
		idx=SelectedItemIndices[i];
		if (sortedArray.BinarySearch(idx,emStdComparer<int>::Compare)>=0) {
			i++;
		}
		else {
			Deselect(idx);
		}
	}
	for (i=0; i<sortedArray.GetCount(); i++) {
		Select(sortedArray[i]);
	}
}


int emListBox::GetSelectedIndex() const
{
	if (SelectedItemIndices.IsEmpty()) return -1;
	else return SelectedItemIndices[0];
}


void emListBox::SetSelectedIndex(int index)
{
	Select(index,true);
}


bool emListBox::IsSelected(int index) const
{
	if (index>=0 && index<Items.GetCount()) {
		return Items[index].Selected;
	}
	return false;
}


void emListBox::Select(int index, bool solely)
{
	ItemPanelInterface * ipf;
	int i;

	if (index>=0 && index<Items.GetCount()) {
		if (solely) {
			while (!SelectedItemIndices.IsEmpty()) {
				i=SelectedItemIndices[0];
				if (i==index) {
					if (SelectedItemIndices.GetCount()==1) break;
					i=SelectedItemIndices[1];
				}
				Deselect(i);
			}
		}
		if (!Items[index].Selected) {
			Items.GetWritable(index).Selected=true;
			SelectedItemIndices.BinaryInsert(index,emStdComparer<int>::Compare);
			Signal(SelectionSignal);
			ipf=GetItemPanelInterface(index);
			if (ipf) ipf->ItemSelectionChanged();
		}
	}
	else {
		if (solely) {
			ClearSelection();
		}
	}
	PrevInputItemIndex=-1;
}


void emListBox::Deselect(int index)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount() && Items[index].Selected) {
		Items.GetWritable(index).Selected=false;
		SelectedItemIndices.BinaryRemove(index,emStdComparer<int>::Compare);
		Signal(SelectionSignal);
		ipf=GetItemPanelInterface(index);
		if (ipf) ipf->ItemSelectionChanged();
	}
	PrevInputItemIndex=-1;
}


void emListBox::ToggleSelection(int index)
{
	if (IsSelected(index)) Deselect(index);
	else Select(index);
}


void emListBox::SelectAll()
{
	int i;

	for (i=0; i<Items.GetCount(); i++) {
		Select(i);
	}
}


void emListBox::ClearSelection()
{
	while (!SelectedItemIndices.IsEmpty()) {
		Deselect(SelectedItemIndices[0]);
	}
}


void emListBox::TriggerItem(int index)
{
	TriggeredItemIndex=index;
	Signal(ItemTriggerSignal);
}


emListBox::ItemPanelInterface::ItemPanelInterface(
	emListBox & listBox, int itemIndex
) :
	ListBox(listBox),
	ItemIndex(itemIndex)
{
}


emListBox::ItemPanelInterface::~ItemPanelInterface()
{
}


void emListBox::ItemPanelInterface::ProcessItemInput(
	emPanel * panel, emInputEvent & event, const emInputState & state
)
{
	ListBox.ProcessItemInput(ItemIndex,panel,event,state);
}


emListBox::DefaultItemPanel::DefaultItemPanel(
	emListBox & listBox, const emString & name, int itemIndex
) :
	emPanel(listBox,name),
	ItemPanelInterface(listBox,itemIndex)
{
}


emListBox::DefaultItemPanel::~DefaultItemPanel()
{
}


void emListBox::DefaultItemPanel::Input(emInputEvent & event, const emInputState & state, double mx, double my)
{
	ProcessItemInput(this,event,state);
	emPanel::Input(event,state,mx,my);
}


bool emListBox::DefaultItemPanel::IsOpaque() const
{
	return false;
}


void emListBox::DefaultItemPanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	double x,y,w,h,r,dx,dy;
	emColor bgColor,fgColor,hlColor;
	bool selected;

	if (GetListBox().GetSelectionType()!=READY_ONLY_SELECTION) {
		bgColor=GetListBox().GetLook().GetInputBgColor();
		fgColor=GetListBox().GetLook().GetInputFgColor();
		hlColor=GetListBox().GetLook().GetInputHlColor();
	}
	else {
		bgColor=GetListBox().GetLook().GetOutputBgColor();
		fgColor=GetListBox().GetLook().GetOutputFgColor();
		hlColor=GetListBox().GetLook().GetOutputHlColor();
	}

	if (!IsEnabled()) {
		bgColor=bgColor.GetBlended(GetListBox().GetLook().GetBgColor(),80.0F);
		fgColor=fgColor.GetBlended(GetListBox().GetLook().GetBgColor(),80.0F);
		hlColor=hlColor.GetBlended(GetListBox().GetLook().GetBgColor(),80.0F);
	}

	x=0.0;
	y=0.0;
	w=1.0;
	h=GetHeight();

	selected=IsItemSelected();

	if (selected) {
		dx=emMin(w,h)*0.015;
		dy=emMin(w,h)*0.015;
		r=emMin(w,h)*0.15;
		painter.PaintRoundRect(
			x+dx,y+dy,w-2*dx,h-2*dy,r,r,
			hlColor,canvasColor
		);
		canvasColor=hlColor;
	}

	dx=emMin(w,h)*0.15;
	dy=emMin(w,h)*0.03;
	painter.PaintTextBoxed(
		x+dx,y+dy,w-2*dx,h-2*dy,
		GetItemText(),
		GetHeight(),
		selected ? bgColor : fgColor,
		canvasColor,
		EM_ALIGN_LEFT,
		EM_ALIGN_LEFT
	);
}


void emListBox::DefaultItemPanel::ItemTextChanged()
{
	InvalidatePainting();
}


void emListBox::DefaultItemPanel::ItemDataChanged()
{
}


void emListBox::DefaultItemPanel::ItemSelectionChanged()
{
	InvalidatePainting();
}


void emListBox::CreateItemPanel(const emString & name, int itemIndex)
{
	new DefaultItemPanel(*this,name,itemIndex);
}


emString emListBox::GetItemPanelName(int index) const
{
	return emString::Format("%d",index);
}


emPanel * emListBox::GetItemPanel(int index) const
{
	return GetChild(GetItemPanelName(index));
}


emListBox::ItemPanelInterface * emListBox::GetItemPanelInterface(int index) const
{
	emPanel * panel;
	ItemPanelInterface * ipf;

	panel = GetItemPanel(index);
	if (!panel) return NULL;
	ipf = dynamic_cast<ItemPanelInterface *>(panel);
	if (!ipf) {
		emFatalError(
			"emListBox::GetItemPanelInterface: An item panel does"
			" not implement emListBox::ItemPanelInterface (list box"
			" type name is %s, item panel type name is %s)",
			typeid(*this).name(), typeid(*panel).name()
		);
	}
	return ipf;
}


void emListBox::Notice(NoticeFlags flags)
{
	emRasterGroup::Notice(flags);

	if ((flags&NF_FOCUS_CHANGED)!=0) {
		if (!IsInFocusedPath()) {
			KeyWalkChars.Clear();
		}
	}
}


void emListBox::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emRasterGroup::Input(event,state,mx,my);

	switch (event.GetKey()) {
	case EM_KEY_A:
		if (state.IsCtrlMod()) {
			if (
				IsEnabled() &&
				(SelType==MULTI_SELECTION || SelType==TOGGLE_SELECTION)
			) {
				SelectAll();
			}
			event.Eat();
		}
		break;
	default:
		break;
	}

	KeyWalk(event,state);
}


void emListBox::AutoExpand()
{
	int i;

	emRasterGroup::AutoExpand();

	for (i=0; i<Items.GetCount(); i++) {
		CreateItemPanel(GetItemPanelName(i), i);
	}
}


emPanel * emListBox::GetItemPanel(int index)
{
	return ((const emListBox*)this)->GetItemPanel(index);
}


emListBox::ItemPanelInterface * emListBox::GetItemPanelInterface(int index)
{
	return ((const emListBox*)this)->GetItemPanelInterface(index);
}


void emListBox::ProcessItemInput(
	int itemIndex, emPanel * panel, emInputEvent & event,
	const emInputState & state
)
{
	switch (event.GetKey()) {
	case EM_KEY_LEFT_BUTTON:
		if (!state.GetAlt() && !state.GetMeta()) {
			SelectByInput(
				itemIndex,
				state.GetShift(),state.GetCtrl(),
				event.GetRepeat()!=0
			);
			panel->Focus();
			event.Eat();
		}
		break;
	case EM_KEY_SPACE:
		if (!state.GetAlt() && !state.GetMeta()) {
			SelectByInput(itemIndex,state.GetShift(),state.GetCtrl(),false);
			event.Eat();
		}
		break;
	case EM_KEY_ENTER:
		if (!state.GetAlt() && !state.GetMeta()) {
			SelectByInput(itemIndex,state.GetShift(),state.GetCtrl(),true);
			event.Eat();
		}
		break;
	default:
		break;
	}
}


void emListBox::SelectByInput(int itemIndex, bool shift, bool ctrl, bool trigger)
{
	int i,i1,i2;

	if (!IsEnabled()) return;

	switch (SelType) {
		case READY_ONLY_SELECTION:
			break;
		case SINGLE_SELECTION:
			Select(itemIndex,true);
			if (trigger) TriggerItem(itemIndex);
			break;
		case MULTI_SELECTION:
			if (shift) {
				i1=itemIndex;
				i2=itemIndex;
				if (
					PrevInputItemIndex>=0 &&
					PrevInputItemIndex<Items.GetCount() &&
					PrevInputItemIndex!=itemIndex
				) {
					if (PrevInputItemIndex<itemIndex) {
						i1=PrevInputItemIndex+1;
					}
					else {
						i2=PrevInputItemIndex-1;
					}
				}
				for (i=i1; i<=i2; i++) {
					if (ctrl) ToggleSelection(i);
					else Select(i,false);
				}
			}
			else if (ctrl) {
				ToggleSelection(itemIndex);
			}
			else {
				Select(itemIndex,true);
			}
			if (trigger) TriggerItem(itemIndex);
			break;
		case TOGGLE_SELECTION:
			if (shift) {
				i1=itemIndex;
				i2=itemIndex;
				if (
					PrevInputItemIndex>=0 &&
					PrevInputItemIndex<Items.GetCount() &&
					PrevInputItemIndex!=itemIndex
				) {
					if (PrevInputItemIndex<itemIndex) {
						i1=PrevInputItemIndex+1;
					}
					else {
						i2=PrevInputItemIndex-1;
					}
				}
				for (i=i1; i<=i2; i++) {
					ToggleSelection(i);
				}
			}
			else {
				ToggleSelection(itemIndex);
			}
			if (trigger) TriggerItem(itemIndex);
			break;
	}

	PrevInputItemIndex=itemIndex;
}


void emListBox::KeyWalk(emInputEvent & event, const emInputState & state)
{
	const char * s1, * s2;
	emPanel * panel;
	emScreen * screen;
	emString str;
	int len, c1, c2, i;
	emUInt64 clk;

	if (event.GetChars().IsEmpty()) return;
	if (state.GetCtrl() || state.GetAlt() || state.GetMeta()) return;
	for (i=0; i<event.GetChars().GetLen(); i++) {
		c1=(unsigned char)event.GetChars()[i];
		if (c1<=32 || c1==127) return;
	}

	clk=GetInputClockMS();
	if (clk-KeyWalkClock>1000) KeyWalkChars.Clear();
	KeyWalkClock=clk;

	str=KeyWalkChars+event.GetChars();
	len=str.GetLen();

	if (str[0]=='*') {
		// ??? undocumented feature: e.g. type "*bar" to find "FooBar".
		for (i=0; i<Items.GetCount(); i++) {
			s1=str.Get()+1;
			s2=Items[i].Text;
			for (i=0;;) {
				c1=(unsigned char)s1[i];
				c2=(unsigned char)s2[i];
				if (!c1 || !c2) break;
				if (tolower(c1)==tolower(c2)) i++;
				else { i=0; s2++; }
			}
			if (!c1) break;
		}
	}
	else {
		for (i=0; i<Items.GetCount(); i++) {
			if (strncasecmp(str,Items[i].Text,len)==0) break;
		}
		if (i>=Items.GetCount()) {
			for (i=0; i<Items.GetCount(); i++) {
				s1=str;
				s2=Items[i].Text;
				for (;;) {
					c1=tolower((unsigned char)*s1++);
					if (!c1) break;
					do {
						c2=tolower((unsigned char)*s2++);
					} while (c2 && c2!=c1 && (c2==' ' || c2=='-' || c2=='_'));
					if (c2!=c1) break;
				}
				if (!c1) break;
			}
		}
	}

	if (i<Items.GetCount()) {
		KeyWalkChars=str;
		if (IsEnabled() && SelType != READY_ONLY_SELECTION) {
			Select(i,true);
		}
		panel=GetItemPanel(i);
		if (panel) {
			GetView().Visit(panel,true);
		}
	}
	else {
		KeyWalkChars.Clear();
		screen=GetScreen();
		if (screen) screen->Beep();
	}

	event.Eat();
}
