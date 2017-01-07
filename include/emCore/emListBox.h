//------------------------------------------------------------------------------
// emListBox.h
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

#ifndef emListBox_h
#define emListBox_h

#ifndef emAnything_h
#include <emCore/emAnything.h>
#endif

#ifndef emRasterGroup_h
#include <emCore/emRasterGroup.h>
#endif


//==============================================================================
//================================= emListBox ==================================
//==============================================================================

class emListBox : public emRasterGroup {

public:

	// Class for list box. It is an emRasterGroup which shows a group of
	// text items, from which the user can select one item. Optionally,
	// multi-selection can be enabled. Each item just consists of a text,
	// but it is possible to derive from emListBox in order to create custom
	// item panels, which show more than just a text.

	enum SelectionType {
		// Type of selection a user can make in an emListBox.
		READY_ONLY_SELECTION,
		SINGLE_SELECTION,
		MULTI_SELECTION,
		TOGGLE_SELECTION
	};

	emListBox(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		SelectionType selType=SINGLE_SELECTION
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   selType     - Type of selection.

	virtual ~emListBox();
		// Destructor.

	SelectionType GetSelectionType() const;
	void SetSelectionType(SelectionType selType);
		// Get or set the type of selection, which the user can make in
		// this list box.

	int GetItemCount() const;
		// Get number of Items.

	void AddItem(
		const emString & text,
		const emAnything & data = emAnything()
	);
		// Add an item to the end of the list.
		// Arguments:
		//   text - The text to be shown in the item.
		//   data - Any custom data to be stored in the item.

	void InsertItem(
		int index, const emString & text,
		const emAnything & data = emAnything()
	);
		// Add an item at any position in the list.
		// Arguments:
		//   index - Index of the new item.
		//   text  - The text to be shown in the item.
		//   data  - Any custom data to be stored in the item.

	void RemoveItem(int index);
		// Remove an item from the list.
		// Arguments:
		//   index - Index of the item to be removed.

	void ClearItems();
		// Remove all items.

	emString GetItemText(int index) const;
		// Get the text of an item.
		// Arguments:
		//   index - Index of the item.
		// Returns: The text of the item, or an empty string if the
		// index is out of range.

	void SetItemText(int index, const emString & text);
		// Set the text of an item.
		// Arguments:
		//   index - Index of the item.
		//   text  - The text to be shown in the item.

	emAnything GetItemData(int index) const;
		// Get the data of an item.
		// Arguments:
		//   index - Index of the item.
		// Returns: The data of the item, or an invalid emAnything if
		// the index is out of range.

	void SetItemData(int index, const emAnything & data);
		// Set the data of an item.
		// Arguments:
		//   index - Index of the item.
		//   data  - Any custom data to be stored in the item.

	int GetSelectionCount() const;
		// Get number of selected items.

	const emArray<int> & GetSelectedIndices() const;
		// Get the indices of the selected items. The returned array is
		// always sorted.

	void SetSelectedIndices(const emArray<int> & itemIndices);
		// Set the indices of the selected items.

	int GetSelectedIndex() const;
		// Get the index of the first (or solely) selected item. If no
		// item is selected, -1 is returned.

	void SetSelectedIndex(int index);
		// Select a certain item solely.
		// Arguments:
		//   index - Index of the item.

	bool IsSelected(int index) const;
		// Ask whether a certain item is selected.
		// Arguments:
		//   index - Index of the item.

	void Select(int index, bool solely=false);
		// Select an item.
		// Arguments:
		//   index  - Index of the item.
		//   solely - Whether all other items should be deselected.

	void Deselect(int index);
		// Deselect an item.
		// Arguments:
		//   index - Index of the item.

	void ToggleSelection(int index);
		// Invert the selection of an item.
		// Arguments:
		//   index - Index of the item.

	void SelectAll();
		// Select all items.

	void ClearSelection();
		// Deselect all items.

	const emSignal & GetSelectionSignal() const;
		// This signal is signaled after each change of the selection.

	const emSignal & GetItemTriggerSignal() const;
		// This signal is signaled when an item is triggered by a double
		// click or by pressing the enter key. The triggered item index
		// can be get with GetTriggeredItemIndex().

	int GetTriggeredItemIndex() const;
		// Get the index of the item which was triggered by a double
		// click or enter key press.

	void TriggerItem(int index);
		// Trigger an item programmatically.

	class ItemPanelInterface {

	public:

		// Class for an interface to an item panel. This must be derived
		// by custom item panel classes. Also see
		// emListBox::CreateItemPanel(...).

		ItemPanelInterface(emListBox & listBox, int itemIndex);
			// Constructor.
			// Arguments:
			//   listBox   - The list box.
			//   itemIndex - The index of the item.

		virtual ~ItemPanelInterface();
			// Destructor.

		emListBox & GetListBox() const;
			// Get the list box.

		int GetItemIndex() const;
			// Get the item index.

		emString GetItemText() const;
			// The the text of the item.

		emAnything GetItemData() const;
			// The the data of the item.

		bool IsItemSelected() const;
			// Whether the item is selected.

	protected:

		void ProcessItemInput(
			emPanel * panel, emInputEvent & event,
			const emInputState & state
		);
			// Process mouse an keyboard events which select,
			// deselect, or trigger an item. This method must be
			// called from the Input method of the item panel.

		virtual void ItemTextChanged() = 0;
			// Called when the text of the item has changed.

		virtual void ItemDataChanged() = 0;
			// Called when the data of the item has changed.

		virtual void ItemSelectionChanged() = 0;
			// Called when the selection of the item has changed.

	private:
		friend class emListBox;
		emListBox & ListBox;
		int ItemIndex;
	};

	class DefaultItemPanel : public emPanel, public ItemPanelInterface {

	public:

		// Default class for an item panel.

		DefaultItemPanel(emListBox & listBox, const emString & name,
		                 int itemIndex);

		virtual ~DefaultItemPanel();

	protected:

		virtual void Input(emInputEvent & event,
		                   const emInputState & state,
		                   double mx, double my);

		virtual bool IsOpaque() const;

		virtual void Paint(const emPainter & painter,
		                   emColor canvasColor) const;

		virtual void ItemTextChanged();

		virtual void ItemDataChanged();

		virtual void ItemSelectionChanged();
	};

protected:

	virtual void CreateItemPanel(const emString & name, int itemIndex);
		// Create the panel for an item. This can be overloaded in order
		// to have custom item panels. The default implementation
		// creates an instance of emListBox::DefaultItemPanel. A derived
		// class may create a panel of an other class. It just has to be
		// a derivative of emPanel and mListBox::ItemPanelInterface.

	virtual emString GetItemPanelName(int index) const;
		// Get the name for an item panel. The default implementation
		// simply converts the index to a decimal string.

	virtual emPanel * GetItemPanel(int index) const;
		// Get an item panel. The default implementation calls
		// GetChild(..) with the name of the item panel.

	virtual ItemPanelInterface * GetItemPanelInterface(int index) const;
		// Get the ItemPanelInterface for an item panel. The default
		// implementation calls GetItemPanel and does a dynamic cast.

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual void AutoExpand();

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
	virtual emPanel * GetItemPanel(int index) final;
	virtual ItemPanelInterface * GetItemPanelInterface(int index) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	friend class ItemPanelInterface;

	void ProcessItemInput(
		int itemIndex, emPanel * panel, emInputEvent & event,
		const emInputState & state
	);

	void SelectByInput(int itemIndex, bool shift, bool ctrl, bool trigger);

	void KeyWalk(emInputEvent & event, const emInputState & state);

	struct Item {
		emString Text;
		emAnything Data;
		bool Selected;
	};

	SelectionType SelType;
	emArray<Item> Items;
	emArray<int> SelectedItemIndices;
	int TriggeredItemIndex;
	int PrevInputItemIndex;
	emSignal SelectionSignal;
	emSignal ItemTriggerSignal;
	emString KeyWalkChars;
	emUInt64 KeyWalkClock;
};

inline emListBox::SelectionType emListBox::GetSelectionType() const
{
	return SelType;
}

inline int emListBox::GetItemCount() const
{
	return Items.GetCount();
}

inline int emListBox::GetSelectionCount() const
{
	return SelectedItemIndices.GetCount();
}

inline const emArray<int> & emListBox::GetSelectedIndices() const
{
	return SelectedItemIndices;
}

inline const emSignal & emListBox::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline const emSignal & emListBox::GetItemTriggerSignal() const
{
	return ItemTriggerSignal;
}

inline int emListBox::GetTriggeredItemIndex() const
{
	return TriggeredItemIndex;
}

inline emListBox & emListBox::ItemPanelInterface::GetListBox() const
{
	return ListBox;
}

inline int emListBox::ItemPanelInterface::GetItemIndex() const
{
	return ItemIndex;
}

inline emString emListBox::ItemPanelInterface::GetItemText() const
{
	return ListBox.GetItemText(ItemIndex);
}

inline emAnything emListBox::ItemPanelInterface::GetItemData() const
{
	return ListBox.GetItemData(ItemIndex);
}

inline bool emListBox::ItemPanelInterface::IsItemSelected() const
{
	return ListBox.IsSelected(ItemIndex);
}


#endif
