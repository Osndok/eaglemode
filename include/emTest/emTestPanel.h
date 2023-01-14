//------------------------------------------------------------------------------
// emTestPanel.h
//
// Copyright (C) 2005-2008,2014-2016,2020,2022 Oliver Hamann.
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

	class PolyDrawPanel : public emLinearGroup {
	public:
		PolyDrawPanel(ParentArg parent, const emString & name);
	protected:
		virtual bool Cycle();
		virtual void AutoExpand();
	private:
		class CanvasPanel : public emPanel {
		public:
			CanvasPanel(ParentArg parent, const emString & name);
			void Setup(
				int type,
				int vertexCount,
				bool withCanvasColor,
				const emTexture& texture,
				double strokeWidth,
				const emStroke& stroke,
				const emStrokeEnd& strokeStart,
				const emStrokeEnd& strokeEnd
			);
		protected:
			virtual void Input(emInputEvent & event, const emInputState & state,
			                   double mx, double my);
			virtual void Paint(const emPainter & painter, emColor canvasColor) const;
		private:
			int Type;
			emArray<double> XY;
			bool WithCanvasColor;
			emTexture Texture;
			double StrokeWidth;
			emStroke Stroke;
			emStrokeEnd StrokeStart;
			emStrokeEnd StrokeEnd;
			int DragIdx;
			double DragDX,DragDY;
			bool ShowHandles;
		};
		emCrossPtr<emRadioButton::RasterGroup> Type;
		emCrossPtr<emTextField> VertexCount;
		emCrossPtr<emCheckBox> WithCanvasColor;
		emCrossPtr<emColorField> FillColor;
		emCrossPtr<emTextField> StrokeWidth;
		emCrossPtr<emColorField> StrokeColor;
		emCrossPtr<emCheckBox> StrokeRounded;
		emCrossPtr<emRadioButton::RasterGroup> StrokeDashType;
		emCrossPtr<emTextField> DashLengthFactor;
		emCrossPtr<emTextField> GapLengthFactor;
		emCrossPtr<emRadioButton::RasterGroup> StrokeStartType;
		emCrossPtr<emColorField> StrokeStartInnerColor;
		emCrossPtr<emTextField> StrokeStartWidthFactor;
		emCrossPtr<emTextField> StrokeStartLengthFactor;
		emCrossPtr<emRadioButton::RasterGroup> StrokeEndType;
		emCrossPtr<emColorField> StrokeEndInnerColor;
		emCrossPtr<emTextField> StrokeEndWidthFactor;
		emCrossPtr<emTextField> StrokeEndLengthFactor;
		emCrossPtr<CanvasPanel> Canvas;
	};

	emList<emString> InputLog;
	emColor BgColor, DefaultBgColor;
	emCrossPtr<TkTestGrp> TkT;
	emCrossPtr<emTestPanel> TP1,TP2,TP3,TP4;
	emCrossPtr<emColorField> BgColorField;
	emCrossPtr<PolyDrawPanel> PolyDraw;
	emCrossPtr<emLabel> ControlPanel;
	emImage TestImage;
};


#endif
