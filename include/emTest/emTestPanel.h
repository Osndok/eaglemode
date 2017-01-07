//------------------------------------------------------------------------------
// emTestPanel.h
//
// Copyright (C) 2005-2008,2014-2016 Oliver Hamann.
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

#ifndef emTestPanel_h
#define emTestPanel_h

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emTestPanel : public emPanel {

public:

	emTestPanel(ParentArg parent, const emString & name);

	virtual ~emTestPanel();

	virtual emString GetTitle() const;

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

	virtual void AutoExpand();

	virtual void LayoutChildren();

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);

private:

	void UpdateControlPanel();

	class TkTest : public emRasterGroup {

	public:

		TkTest(ParentArg parent, const emString & name);
		virtual ~TkTest();

	protected:

		virtual bool Cycle();

	private:

		static void TextOfTimeValue(
			char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
			void * context
		);
		static void TextOfLevelValue(
			char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
			void * context
		);

		emScalarField * SFLen, * SFPos;
		emCheckBox * CbTopLev, * CbPZoom, * CbModal, * CbUndec, * CbPopup;
		emCheckBox * CbMax, * CbFull;
		emButton * BtCreateDlg, * BtOpenFile, * BtOpenFiles, * BtSaveFile;
		emFileDialog * FileDlg;
	};

	class TkTestGrp : public emRasterGroup {
	public:
		TkTestGrp(ParentArg parent, const emString & name);
	protected:
		virtual void AutoExpand();
	};

	class CustomItemPanel :
		public emLinearGroup,
		public emListBox::ItemPanelInterface
	{
	public:
		CustomItemPanel(emListBox & listBox, const emString & name, int itemIndex);
		virtual ~CustomItemPanel();
	protected:
		virtual void Input(emInputEvent & event, const emInputState & state,
		                   double mx, double my);
		virtual void AutoExpand();
		virtual void ItemTextChanged();
		virtual void ItemDataChanged();
		virtual void ItemSelectionChanged();
	};

	class CustomListBox : public emListBox {
	public:
		CustomListBox(
			ParentArg parent, const emString & name,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage(),
			SelectionType selType=SINGLE_SELECTION
		);
	protected:
		virtual void CreateItemPanel(const emString & name, int itemIndex);
	};

	emList<emString> InputLog;
	emColor BgColor, DefaultBgColor;
	emCrossPtr<TkTestGrp> TkT;
	emCrossPtr<emTestPanel> TP1,TP2,TP3,TP4;
	emCrossPtr<emColorField> BgColorField;
	emCrossPtr<emLabel> ControlPanel;
};


#endif
