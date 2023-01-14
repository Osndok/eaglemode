//------------------------------------------------------------------------------
// emListBox.cpp
//
// Copyright (C) 2015-2016,2021-2022 Oliver Hamann.
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
	Items.SetTuningLevel(4);
	AvlTree=NULL;
	SelectedItemIndices.SetTuningLevel(4);
	TriggeredItem=NULL;
	PrevInputItem=NULL;
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
		if (SelType==READ_ONLY_SELECTION) {
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


void emListBox::AddItem(
	const emString & name, const emString & text, const emAnything & data
)
{
	InsertItem(Items.GetCount(),name,text,data);
}


void emListBox::InsertItem(
	int index, const emString & name, const emString & text,
	const emAnything & data
)
{
	EM_AVL_INSERT_VARS(Item)
	emPanel * p1, * p2;
	Item * item;
	bool selectionChanged;
	int i,j,d;

	if (index<0) index=0;
	if (index>Items.GetCount()) index=Items.GetCount();
	item=new Item;
	Items.Insert(index,item);
	for (i=index; i<Items.GetCount(); i++) Items[i]->Index=i;
	item->Name=name;
	item->Text=text;
	item->Data=data;
	item->Interface=NULL;
	item->Selected=false;

	EM_AVL_INSERT_BEGIN_SEARCH(Item,AvlNode,AvlTree)
		d=strcmp(name.Get(),element->Name.Get());
		if (d<0) EM_AVL_INSERT_GO_LEFT
		else if (d>0) EM_AVL_INSERT_GO_RIGHT
		else {
			emFatalError(
				"emListBox: Item name \"%s\" not unique within \"%s\".",
				name.Get(),GetIdentity().Get()
			);
		}
	EM_AVL_INSERT_END_SEARCH
		element=item;
	EM_AVL_INSERT_NOW(AvlNode)

	selectionChanged=false;
	for (i=SelectedItemIndices.GetCount()-1; i>=0; i--) {
		j=SelectedItemIndices[i];
		if (j<index) break;
		SelectedItemIndices.Set(i,j+1);
		selectionChanged=true;
	}
	KeyWalkChars.Clear();
	if (selectionChanged) Signal(SelectionSignal);

	if (IsAutoExpanded()) {
		CreateItemPanel(GetItemName(index), index);
		if (index<Items.GetCount()-1) {
			p1=GetItemPanel(index);
			p2=GetItemPanel(index+1);
			if (p1 && p2) p1->BePrevOf(p2);
		}
	}
}


void emListBox::MoveItem(int fromIndex, int toIndex)
{
	Item * item;
	emPanel * p1, * p2;
	bool selectionChanged;
	int i,j,k,d;

	if (fromIndex<0 || fromIndex>=Items.GetCount()) return;
	if (toIndex<0) toIndex=0;
	if (toIndex>Items.GetCount()-1) toIndex=Items.GetCount()-1;
	if (fromIndex==toIndex) return;

	if (IsAutoExpanded()) {
		p1=GetItemPanel(fromIndex);
		p2=GetItemPanel(toIndex);
		if (p1 && p2) {
			if (fromIndex<toIndex) p1->BeNextOf(p2);
			else                   p1->BePrevOf(p2);
		}
	}

	item=Items[fromIndex];
	d=(fromIndex<toIndex ? 1 : -1);
	for (i=fromIndex; i!=toIndex; i+=d) {
		Items.Set(i,Items[i+d]);
		Items[i]->Index=i;
	}
	Items.Set(toIndex,item);
	Items[toIndex]->Index=toIndex;

	selectionChanged=false;
	if (fromIndex<toIndex) { j=fromIndex; k=toIndex; }
	else                   { j=toIndex; k=fromIndex; }
	for (i=0; i<SelectedItemIndices.GetCount(); i++) {
		if (SelectedItemIndices[i]>=j) break;
	}
	for (; j<=k; j++) {
		if (Items[j]->Selected && i<SelectedItemIndices.GetCount()) {
			if (SelectedItemIndices[i]!=j) {
				SelectedItemIndices.Set(i,j);
				selectionChanged=true;
			}
			i++;
		}
	}

	KeyWalkChars.Clear();
	if (selectionChanged) Signal(SelectionSignal);
}


bool emListBox::SortItems(
	int(*compare)(
		const emString & item1name, const emString & item1text,
		const emAnything & item1data,
		const emString & item2name, const emString & item2text,
		const emAnything & item2data,
		void * context
	),
	void * context
)
{
	CompareContext cc;
	emPanel * p1, * p2;
	bool selectionChanged;
	int i,j;

	cc.origCompare=compare;
	cc.origContext=context;
	if (!Items.Sort(CompareItems,&cc)) return false;

	for (i=0; i<Items.GetCount(); i++) Items[i]->Index=i;

	selectionChanged=false;
	for (i=0, j=0; j<Items.GetCount(); j++) {
		if (Items[j]->Selected && i<SelectedItemIndices.GetCount()) {
			if (SelectedItemIndices[i]!=j) {
				SelectedItemIndices.Set(i,j);
				selectionChanged=true;
			}
			i++;
		}
	}

	KeyWalkChars.Clear();
	if (selectionChanged) Signal(SelectionSignal);

	if (IsAutoExpanded()) {
		p1=GetItemPanel(0);
		for (i=1; i<Items.GetCount(); i++) {
			p2=GetItemPanel(i);
			if (p2) {
				if (p1) p2->BeNextOf(p1);
				p1=p2;
			}
		}
	}

	return true;
}


void emListBox::RemoveItem(int index)
{
	EM_AVL_REMOVE_VARS(Item)
	bool selectionChanged;
	int i,j,d;

	if (index>=0 && index<Items.GetCount()) {
		if (Items[index]->Interface) delete Items[index]->Interface;

		EM_AVL_REMOVE_BEGIN(Item,AvlNode,AvlTree)
			d=strcmp(Items[index]->Name.Get(),element->Name.Get());
			if (d<0) EM_AVL_REMOVE_GO_LEFT
			else if (d>0) EM_AVL_REMOVE_GO_RIGHT
			else EM_AVL_REMOVE_NOW
		EM_AVL_REMOVE_END

		if (TriggeredItem==Items[index]) TriggeredItem=NULL;
		if (PrevInputItem==Items[index]) PrevInputItem=NULL;

		delete Items[index];
		Items.Remove(index);
		for (i=index; i<Items.GetCount(); i++) Items[i]->Index=i;

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
		KeyWalkChars.Clear();
		if (selectionChanged) Signal(SelectionSignal);
	}
}


void emListBox::ClearItems()
{
	int i;

	if (!Items.IsEmpty()) {
		for (i=Items.GetCount()-1; i>=0; i--) {
			if (Items[i]->Interface) {
				delete Items[i]->Interface;
			}
		}
		for (i=Items.GetCount()-1; i>=0; i--) delete Items[i];
		Items.Clear();
		AvlTree=NULL;
		TriggeredItem=NULL;
		PrevInputItem=NULL;
		if (!SelectedItemIndices.IsEmpty()) {
			SelectedItemIndices.Clear();
			Signal(SelectionSignal);
		}
		KeyWalkChars.Clear();
	}
}


const emString & emListBox::GetItemName(int index) const
{
	static const emString emptyString;

	if (index>=0 && index<Items.GetCount()) return Items[index]->Name;
	else return emptyString;
}


int emListBox::GetItemIndex(const char * name) const
{
	EM_AVL_SEARCH_VARS(Item)
	int d;

	EM_AVL_SEARCH_BEGIN(Item,AvlNode,AvlTree)
		d=strcmp(name,element->Name.Get());
		if (d<0) EM_AVL_SEARCH_GO_LEFT
		else if (d>0) EM_AVL_SEARCH_GO_RIGHT
		else return element->Index;
	EM_AVL_SEARCH_END
	return -1;
}


const emString & emListBox::GetItemText(int index) const
{
	static const emString emptyString;

	if (index>=0 && index<Items.GetCount()) return Items[index]->Text;
	else return emptyString;
}


void emListBox::SetItemText(int index, const emString & text)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount()) {
		if (Items[index]->Text != text) {
			Items[index]->Text=text;
			KeyWalkChars.Clear();
			ipf=GetItemPanelInterface(index);
			if (ipf) ipf->ItemTextChanged();
		}
	}
}


emAnything emListBox::GetItemData(int index) const
{
	if (index>=0 && index<Items.GetCount()) return Items[index]->Data;
	else return emAnything();
}


void emListBox::SetItemData(int index, const emAnything & data)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount()) {
		Items[index]->Data=data;
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
		return Items[index]->Selected;
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
		if (!Items[index]->Selected) {
			Items[index]->Selected=true;
			SelectedItemIndices.BinaryInsert(index,emStdComparer<int>::Compare);
			Signal(SelectionSignal);
			ipf=GetItemPanelInterface(index);
			if (ipf) ipf->ItemSelectionChanged();
			PrevInputItem=NULL;
		}
	}
	else {
		if (solely) {
			ClearSelection();
		}
	}
}


void emListBox::Deselect(int index)
{
	ItemPanelInterface * ipf;

	if (index>=0 && index<Items.GetCount() && Items[index]->Selected) {
		Items[index]->Selected=false;
		SelectedItemIndices.BinaryRemove(index,emStdComparer<int>::Compare);
		Signal(SelectionSignal);
		ipf=GetItemPanelInterface(index);
		if (ipf) ipf->ItemSelectionChanged();
		PrevInputItem=NULL;
	}
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
	if (index>=0 && index<Items.GetCount()) {
		TriggeredItem=Items[index];
		Signal(ItemTriggerSignal);
	}
}


emListBox::ItemPanelInterface::ItemPanelInterface(
	emListBox & listBox, int itemIndex
) :
	ListBox(listBox)
{
	if (itemIndex<0 || itemIndex>=listBox.Items.GetCount()) {
		emFatalError("ItemPanelInterface: itemIndex out of range");
	}
	Item=listBox.Items[itemIndex];
	if (((emListBox::Item*)Item)->Interface) {
		emFatalError("ItemPanelInterface: Multiple instances for same item not allowed");
	}
	((emListBox::Item*)Item)->Interface=this;
}


emListBox::ItemPanelInterface::~ItemPanelInterface()
{
	((emListBox::Item*)Item)->Interface=NULL;
}


void emListBox::ItemPanelInterface::ProcessItemInput(
	emPanel * panel, emInputEvent & event, const emInputState & state
)
{
	ListBox.ProcessItemInput(((emListBox::Item*)Item)->Index,panel,event,state);
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

	if (GetListBox().GetSelectionType()!=READ_ONLY_SELECTION) {
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


emPanel * emListBox::GetItemPanel(int index) const
{
	return dynamic_cast<emPanel*>(GetItemPanelInterface(index));
}


emListBox::ItemPanelInterface * emListBox::GetItemPanelInterface(int index) const
{
	if (index>=0 && index<Items.GetCount()) return Items[index]->Interface;
	else return NULL;
}


void emListBox::CreateItemPanel(const emString & name, int itemIndex)
{
	new DefaultItemPanel(*this,name,itemIndex);
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
		if (state.IsCtrlMod() || state.IsShiftCtrlMod()) {
			if (
				IsEnabled() &&
				(SelType==MULTI_SELECTION || SelType==TOGGLE_SELECTION)
			) {
				if (state.IsCtrlMod()) SelectAll();
				else ClearSelection();
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
		CreateItemPanel(Items[i]->Name, i);
		if (!Items[i]->Interface) {
			emFatalError(
				"emListBox::AutoExpand: An item panel does not implement"
				" emListBox::ItemPanelInterface (list box type name is %s).",
				typeid(*this).name()
			);
		}
	}
}


void emListBox::AutoShrink()
{
	int i;

	// This is needed because item panels may also be created outside
	// AutoExpand().
	for (i=Items.GetCount()-1; i>=0; i--) {
		if (Items[i]->Interface) {
			delete Items[i]->Interface;
		}
	}

	emRasterGroup::AutoShrink();
}


bool emListBox::HasHowTo() const
{
	return true;
}


emString emListBox::GetHowTo() const
{
	emString h;

	h=emRasterGroup::GetHowTo();
	h+=HowToListBox;
	switch (SelType) {
		case READ_ONLY_SELECTION:
			h+=HowToReadOnlySelection;
			break;
		case SINGLE_SELECTION:
			h+=HowToSingleSelection;
			break;
		case MULTI_SELECTION:
			h+=HowToMultiSelection;
			break;
		case TOGGLE_SELECTION:
			h+=HowToToggleSelection;
			break;
	}
	return h;
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
		case READ_ONLY_SELECTION:
			break;
		case SINGLE_SELECTION:
			Select(itemIndex,true);
			if (trigger) TriggerItem(itemIndex);
			break;
		case MULTI_SELECTION:
			if (shift) {
				i1=itemIndex;
				i2=itemIndex;
				if (PrevInputItem && PrevInputItem->Index!=itemIndex) {
					if (PrevInputItem->Index<itemIndex) {
						i1=PrevInputItem->Index+1;
					}
					else {
						i2=PrevInputItem->Index-1;
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
				if (PrevInputItem && PrevInputItem->Index!=itemIndex) {
					if (PrevInputItem->Index<itemIndex) {
						i1=PrevInputItem->Index+1;
					}
					else {
						i2=PrevInputItem->Index-1;
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

	PrevInputItem=Items[itemIndex];
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
			s2=Items[i]->Text;
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
			if (strncasecmp(str,Items[i]->Text,len)==0) break;
		}
		if (i>=Items.GetCount()) {
			for (i=0; i<Items.GetCount(); i++) {
				s1=str;
				s2=Items[i]->Text;
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
		if (IsEnabled() && SelType != READ_ONLY_SELECTION) {
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


int emListBox::CompareItems(
	Item * const * item1, Item * const * item2, void * context
)
{
	CompareContext * cc = (CompareContext*)context;
	return cc->origCompare(
		(*item1)->Name,(*item1)->Text,(*item1)->Data,
		(*item2)->Name,(*item2)->Text,(*item2)->Data,
		cc->origContext
	);
}


const char * const emListBox::HowToListBox=
	"\n"
	"\n"
	"LIST BOX\n"
	"\n"
	"This is a list box. It may show any number of items from which one or more may\n"
	"be selected (by program or by user). Selected items are shown highlighted.\n"
;


const char * const emListBox::HowToReadOnlySelection=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This list box is read-only. You cannot modify the selection.\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Any normal key               - To find and focus an item, you can simply\n"
	"                                 enter the first characters of its caption.\n"
;


const char * const emListBox::HowToSingleSelection=
	"\n"
	"\n"
	"SINGLE-SELECTION\n"
	"\n"
	"This is a single-selection list box. You can select only one item.\n"
	"\n"
	"Mouse control:\n"
	"\n"
	"  Left-Button-Click            - Select the clicked item.\n"
	"\n"
	"  Left-Button-Double-Click     - Trigger the clicked item (application-defined\n"
	"                                 function).\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Space                        - Select the focused item.\n"
	"\n"
	"  Enter                        - Trigger the focused item (application-defined\n"
	"                                 function).\n"
	"\n"
	"  Any normal key               - To find, focus and select an item, you can simply\n"
	"                                 enter the first characters of its caption.\n"
;


const char * const emListBox::HowToMultiSelection=
	"\n"
	"\n"
	"MULTI-SELECTION\n"
	"\n"
	"This list box supports multi-selection. You can select one or more items.\n"
	"\n"
	"Mouse control:\n"
	"\n"
	"  Left-Button-Click            - Select the clicked item.\n"
	"\n"
	"  Shift+Left-Button-Click      - Select the range of items from the previously\n"
	"                                 clicked item to this clicked item.\n"
	"\n"
	"  Ctrl+Left-Button-Click       - Invert the selection of the clicked item.\n"
	"\n"
	"  Shift+Ctrl+Left-Button-Click - Invert the selection of a range of items or\n"
	"                                 select an additional range.\n"
	"\n"
	"  Left-Button-Double-Click     - Trigger the clicked item (application-defined\n"
	"                                 function).\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Space                        - Select the focused item.\n"
	"\n"
	"  Shift+Space                  - Select the range of items from the previously\n"
	"                                 selected item to the focused item.\n"
	"\n"
	"  Ctrl+Space                   - Invert the selection of the focused item.\n"
	"\n"
	"  Shift+Ctrl+Space             - Invert the selection of a range of items or\n"
	"                                 select an additional range.\n"
	"\n"
	"  Ctrl+A                       - Select all items.\n"
	"\n"
	"  Shift+Ctrl+A                 - Clear the selection.\n"
	"\n"
	"  Enter                        - Trigger the focused item (application-defined\n"
	"                                 function).\n"
	"\n"
	"  Any normal key               - To find, focus and select an item, you can simply\n"
	"                                 enter the first characters of its caption.\n"
;


const char * const emListBox::HowToToggleSelection=
	"\n"
	"\n"
	"TOGGLE-SELECTION\n"
	"\n"
	"This is a toggle-selection list box. You can select or deselect\n"
	"individual items independently from other items.\n"
	"\n"
	"Mouse control:\n"
	"\n"
	"  Left-Button-Click            - Invert the selection of the clicked item.\n"
	"\n"
	"  Shift+Left-Button-Click      - Invert the selection of the range of items from\n"
	"                                 the previously clicked item to this clicked\n"
	"                                 item.\n"
	"\n"
	"  Left-Button-Double-Click     - Trigger the clicked item (application-defined\n"
	"                                 function).\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Space                        - Invert the selection of the focused item.\n"
	"\n"
	"  Shift+Space                  - Invert the selection of the range of items from\n"
	"                                 the previously selected item to the focused\n"
	"                                 item.\n"
	"\n"
	"  Ctrl+A                       - Select all items.\n"
	"\n"
	"  Shift+Ctrl+A                 - Deselect all items.\n"
	"\n"
	"  Enter                        - Trigger the focused item (application-defined\n"
	"                                 function).\n"
	"\n"
	"  Any normal key               - To find, focus and select an item, you can simply\n"
	"                                 enter the first characters of its caption.\n"
;
