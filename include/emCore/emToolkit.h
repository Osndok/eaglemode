//------------------------------------------------------------------------------
// emToolkit.h
//
// Copyright (C) 2005-2010 Oliver Hamann.
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

#ifndef emToolkit_h
#define emToolkit_h

#ifndef emVarModel_h
#include <emCore/emVarModel.h>
#endif

#ifndef emClipboard_h
#include <emCore/emClipboard.h>
#endif

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif


//==============================================================================
//================================== emTkLook ==================================
//==============================================================================

class emTkLook {

public:

	// Class for the look of toolkit panels. Currently, the look consists of
	// a set of colors only. Objects of this class have copy-on-write
	// behavior.

	emTkLook();
		// Construct a default look.

	emTkLook(const emTkLook & look);
		// Construct a copied look.

	~emTkLook();
		// Destructor.

	emTkLook & operator = (const emTkLook & look);
		// Copy a look.

	bool operator == (const emTkLook & look) const;
	bool operator != (const emTkLook & look) const;
		// Compare two looks.

	void Apply(emPanel * panel, bool recursively) const;
		// Apply this look to a panel or to all panels in a sub-tree.
		// Applying actually works for panels of class emTkBorder and
		// its derivatives only, but the recursion is not stopped by
		// other panel classes. However, the recursion can be stopped by
		// an overloaded implementation of emTkBorder::SetLook.
		// Arguments:
		//   panel       - The panel.
		//   recursively - Whether to recurse ancestor panels.

	emColor GetBgColor() const;
	emColor GetFgColor() const;
	void SetBgColor(emColor bgColor);
	void SetFgColor(emColor fgColor);
		// Get/set back- and foreground colors of borders, labels,
		// groups and similar things.

	emColor GetButtonBgColor() const;
	emColor GetButtonFgColor() const;
	void SetButtonBgColor(emColor buttonBgColor);
	void SetButtonFgColor(emColor buttonFgColor);
		// Get/set back- and foreground colors of button faces.

	emColor GetInputBgColor() const;
	emColor GetInputFgColor() const;
	emColor GetInputHlColor() const;
	void SetInputBgColor(emColor inputBgColor);
	void SetInputFgColor(emColor inputFgColor);
	void SetInputHlColor(emColor inputHlColor);
		// Get/set background, foreground and highlight (=selection)
		// colors of editable data fields.

	emColor GetOutputBgColor() const;
	emColor GetOutputFgColor() const;
	emColor GetOutputHlColor() const;
	void SetOutputBgColor(emColor outputBgColor);
	void SetOutputFgColor(emColor outputFgColor);
	void SetOutputHlColor(emColor outputHlColor);
		// Get/set background, foreground and highlight (=selection)
		// colors of read-only data fields.

	unsigned int GetDataRefCount() const;
		// Get number of references to the internal data of this object.

	void MakeNonShared();
		// This must be called before handing the look to another
		// thread.

private:

	void DeleteData();
	void MakeWritable();

	struct SharedData {
		SharedData();
		SharedData(const SharedData & sd);
		unsigned int RefCount;
		emColor BgColor;
		emColor FgColor;
		emColor ButtonBgColor;
		emColor ButtonFgColor;
		emColor InputBgColor;
		emColor InputFgColor;
		emColor InputHlColor;
		emColor OutputBgColor;
		emColor OutputFgColor;
		emColor OutputHlColor;
	};

	SharedData * Data;

	static SharedData DefaultData;
};

#ifndef EM_NO_DATA_EXPORT
inline emTkLook::emTkLook()
{
	Data=&DefaultData;
}
#endif

inline emTkLook::emTkLook(const emTkLook & look)
{
	Data=look.Data;
	Data->RefCount++;
}

inline emTkLook::~emTkLook()
{
	if (!--Data->RefCount) DeleteData();
}

inline emTkLook & emTkLook::operator = (const emTkLook & look)
{
	look.Data->RefCount++;
	if (!--Data->RefCount) DeleteData();
	Data=look.Data;
	return *this;
}

inline bool emTkLook::operator != (const emTkLook & look) const
{
	return !(*this==look);
}

inline emColor emTkLook::GetBgColor() const
{
	return Data->BgColor;
}

inline emColor emTkLook::GetFgColor() const
{
	return Data->FgColor;
}

inline emColor emTkLook::GetButtonBgColor() const
{
	return Data->ButtonBgColor;
}

inline emColor emTkLook::GetButtonFgColor() const
{
	return Data->ButtonFgColor;
}

inline emColor emTkLook::GetInputBgColor() const
{
	return Data->InputBgColor;
}

inline emColor emTkLook::GetInputFgColor() const
{
	return Data->InputFgColor;
}

inline emColor emTkLook::GetInputHlColor() const
{
	return Data->InputHlColor;
}

inline emColor emTkLook::GetOutputBgColor() const
{
	return Data->OutputBgColor;
}

inline emColor emTkLook::GetOutputFgColor() const
{
	return Data->OutputFgColor;
}

inline emColor emTkLook::GetOutputHlColor() const
{
	return Data->OutputHlColor;
}

inline void emTkLook::MakeNonShared()
{
	MakeWritable();
}


//==============================================================================
//================================= emTkBorder =================================
//==============================================================================

class emTkBorder : public emPanel {

public:

	// This is the base class of all toolkit panels. A panel of this class
	// can have a border, a label, a how-to text and an auxiliary area. And
	// it has a content area. The label can consist of a caption, a
	// description and an icon. The how-to text describes how to use the
	// type of panel in general, and maybe something about its state. The
	// auxiliary area is for showing a custom panel with additional things
	// like a configuration or an extended help text. The label, the how-to
	// text and the auxiliary area are shown in the border. Alternatively, a
	// derived class can manage to move the label into the content area.

	emTkBorder(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.

	virtual ~emTkBorder();
		// Destructor.

	const emString & GetCaption() const;
	void SetCaption(const emString & caption);
		// The caption to be shown in the label.

	const emString & GetDescription() const;
	void SetDescription(const emString & description);
		// The description to be shown in the label.

	const emImage & GetIcon() const;
	void SetIcon(const emImage & icon);
		// The icon to be shown in the label.

	void SetLabel(
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Set all three things which are making up the label.

	emAlignment GetLabelAlignment() const;
	void SetLabelAlignment(emAlignment labelAlignment);
		// Alignment of the label as a whole within its available space.

	emAlignment GetCaptionAlignment() const;
	void SetCaptionAlignment(emAlignment captionAlignment);
		// Horizontal alignment of lines within the caption text of the
		// label. The top and bottom flags are ignored.

	emAlignment GetDescriptionAlignment() const;
	void SetDescriptionAlignment(emAlignment descriptionAlignment);
		// Horizontal alignment of lines within the description text of
		// the label. The top and bottom flags are ignored.

	enum OuterBorderType {
		// Possibles types for the outer border line. This even
		// specifies whether the background of the panel should be
		// filled or not.
		OBT_NONE,
			// Do not have an outer border line, do not have a
			// margin and do not fill the background.
		OBT_FILLED,
			// Like OBT_NONE, but fill the whole background with
			// background color.
		OBT_MARGIN,
			// Like OBT_NONE, but have a small margin (for example,
			// this is used by emTkLabel and emTkCheckBox). Larger
			// margins should be solved through the parent panel,
			// e.g. see emTkTiling::SetSpace.
		OBT_MARGIN_FILLED,
			// Like OBT_MARGIN, but fill the whole background.
		OBT_RECT,
			// Have a rectangular outer border line and fill the
			// rectangle with background color. Have a small margin.
		OBT_ROUND_RECT,
			// Like OBT_RECT but with round corners.
		OBT_GROUP,
			// Have a small special outer border for groups (used by
			// emTkGroup).
		OBT_INSTRUMENT,
			// Like OBT_GROUP, but the border line is thicker (for
			// example, this is used by emTkTextField).
		OBT_INSTRUMENT_MORE_ROUND,
			// Like OBT_INSTRUMENT, but with a larger corner radius
			// (this is used by emTkButton).
		OBT_POPUP_ROOT
			// Have a special border for root panels of views which
			// have the VF_POPUP_ZOOM flag set (should not be used
			// for something else).
	};

	enum InnerBorderType {
		// Possibles types for the inner border line.
		IBT_NONE,
			// Do not have an inner border line.
		IBT_GROUP,
			// Have a special round inner border line for groups.
		IBT_INPUT_FIELD,
			// Have a special round inner border and background for
			// editable fields.
		IBT_OUTPUT_FIELD,
			// Have a special round inner border and background for
			// non-editable fields.
		IBT_CUSTOM_RECT
			// Have a special rectangular inner border for custom
			// stuff. Herewith, the content rectangle never has
			// round corners.
	};

	OuterBorderType GetOuterBorderType() const;
	InnerBorderType GetInnerBorderType() const;
	void SetOuterBorderType(OuterBorderType obt);
	void SetInnerBorderType(InnerBorderType ibt);
	void SetBorderType(OuterBorderType obt, InnerBorderType ibt);
		// Outer and inner border types. The default is OBT_NONE and
		// IBT_NONE.

	double GetBorderScaling() const;
	void SetBorderScaling(double borderScaling);
		// Scale factor for the size of the border. The default is 1.0.

	const emTkLook & GetLook() const;
	virtual void SetLook(const emTkLook & look, bool recursively=false);
		// Look of this toolkit panel. At construction of a panel, the
		// look is copied from the parent panel (if the parent is not
		// emTkBorder, the grand parent is asked, and so on). When
		// setting the look with the argument recursively=true, all
		// descendant panels of class emTkBorder are even set through
		// calling emTkLook::Apply for every child panel.

	void HaveAux(const emString & panelName, double tallness);
		// Make this border having a rectangular area for auxiliary
		// stuff. It could be a user interface for configuring this
		// panel, or an extended function, or some additional help or
		// what ever you want. Either you could show the things through
		// a child panel or through custom painting, but doing it with a
		// child panel is easier. Whenever you create that panel. It is
		// laid out automatically into the auxiliary area.
		// Arguments:
		//   panelName - Name of the child panel to be laid out in the
		//               auxiliary area.
		//   tallness  - Height/width ratio of the auxiliary area.

	void RemoveAux();
		// Inversion of HaveAux (does not delete the auxiliary panel).

	bool HasAux() const;
		// Whether this border has an area for auxiliary stuff.

	const emString & GetAuxPanelName() const;
	double GetAuxTallness() const;
		// Properties of the auxiliary area set with HaveAux. Valid only
		// if HasAux()==true.

	emPanel * GetAuxPanel();
		// Returns the auxiliary child panel, or NULL if not present.

	void GetAuxRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	);
		// Get the coordinates and canvas color of the auxiliary area.
		// Valid only if HasAux()==true.

	virtual void GetContentRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	);
		// Get the coordinates and canvas color of the content area as a
		// rectangle. If the inner border has round corners, the
		// rectangle returned here is smaller than with
		// GetContentRoundRect, so that it fits completely into the
		// content area.

	virtual void GetContentRoundRect(
		double * pX, double * pY, double * pW, double * pH, double * pR,
		emColor * pCanvasColor=NULL
	);
		// Get the coordinates and canvas color of the content area as a
		// round rectangle (argument pR is for returning the radius of
		// the corners).

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual void LayoutChildren();
		// See emPanel. Hint: For painting the content area, please
		// overload PaintContent instead of Paint, because with certain
		// border types, a shadow is painted over the content area.

	virtual bool HasHowTo();
	virtual emString GetHowTo();
		// This is about a text describing how to use this panel. If
		// HasHowTo()==true, the text returned by GetHowTo() is shown
		// very small in the center of the left edge of the border. When
		// overloading GetHowTo(), please do not forget to call the
		// original version and to include that text at the beginning.
		// The default implementation of GetHowTo() returns a preface,
		// and optionally a description of the disable state (if
		// disabled) and optionally a description of the keyboard focus
		// (if focusable). The default implementation of HasHowTo()
		// return false, because the default text alone is not so
		// helpful, and because the text would disturb some panel types.

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);
		// This can be overloaded for painting the content area. The
		// default implementation does nothing. The coordinates x,y,w,h
		// are like from GetContentRect, but you could even use the
		// coordinates returned by GetContentRoundRect.

	virtual bool HasLabel();
		// Whether this panel has a label. The default implementation
		// checks whether at least one of caption, description and icon
		// is not empty.

	virtual double GetBestLabelTallness();
		// Get the ideal tallness for the label area. The default
		// implementation calculates this for the default implementation
		// of PaintLabel.

	virtual void PaintLabel(
		const emPainter & painter, double x, double y, double w,
		double h, emColor color, emColor canvasColor
	);
		// Paint the label. The default implementation paints the
		// caption, description and icon. This could be overloaded to
		// paint something else for the label.

	bool IsLabelInBorder() const;
	void SetLabelInBorder(bool labelInBorder);
		// Whether to show the label as part of the border. The default
		// is true. If a derived class wants to have the label as part
		// of the content, it should set false here and call PaintLabel
		// itself.

	struct TkResources {
		TkResources();
		~TkResources();
		emImage ImgButton;
		emImage ImgButtonBorder;
		emImage ImgButtonChecked;
		emImage ImgButtonPressed;
		emImage ImgCheckBox;
		emImage ImgCheckBoxPressed;
		emImage ImgCustomRectBorder;
		emImage ImgGroupBorder;
		emImage ImgGroupInnerBorder;
		emImage ImgIOField;
		emImage ImgPopupBorder;
		emImage ImgRadioBox;
		emImage ImgRadioBoxPressed;
		emImage ImgSplitter;
		emImage ImgSplitterPressed;
		emImage ImgTunnel;
	};
	const TkResources & GetTkResources() const;
		// Shared resources used by the toolkit panel implementations.
		// This is more or less private stuff - do not use in custom
		// classes.

private:

	enum DoBorderFunc {
		BORDER_FUNC_PAINT,
		BORDER_FUNC_CONTENT_RECT,
		BORDER_FUNC_CONTENT_ROUND_RECT,
		BORDER_FUNC_AUX_RECT
	};
	void DoBorder(
		DoBorderFunc func, const emPainter * painter,
		emColor canvasColor, double * pX, double * pY, double * pW,
		double * pH, double * pR, emColor * pCanvasColor
	);

	enum DoLabelFunc {
		LABEL_FUNC_PAINT,
		LABEL_FUNC_GET_BEST_TALLNESS
	};
	void DoLabel(
		DoLabelFunc func, const emPainter * painter, double x, double y,
		double w, double h, emColor color, emColor canvasColor,
		double * pBestTallness
	);

	struct AuxData {
		emString PanelName;
		double Tallness;
		emCrossPtr<emPanel> PanelPointerCache;
	};

	emRef<emVarModel<TkResources> > TkResVarModel;
	emString Caption;
	emString Description;
	emImage Icon;
	AuxData * Aux;
	emTkLook Look;
	double BorderScaling;
	emAlignment LabelAlignment;
	emAlignment CaptionAlignment;
	emAlignment DescriptionAlignment;
	emByte OuterBorder;
	emByte InnerBorder;
	bool LabelInBorder;

	static const char * HowToPreface;
	static const char * HowToDisabled;
	static const char * HowToFocus;
};

inline const emString & emTkBorder::GetCaption() const
{
	return Caption;
}

inline const emString & emTkBorder::GetDescription() const
{
	return Description;
}

inline const emImage & emTkBorder::GetIcon() const
{
	return Icon;
}

inline emAlignment emTkBorder::GetLabelAlignment() const
{
	return LabelAlignment;
}

inline emAlignment emTkBorder::GetCaptionAlignment() const
{
	return CaptionAlignment;
}

inline emAlignment emTkBorder::GetDescriptionAlignment() const
{
	return DescriptionAlignment;
}

inline emTkBorder::OuterBorderType emTkBorder::GetOuterBorderType() const
{
	return (OuterBorderType)OuterBorder;
}

inline emTkBorder::InnerBorderType emTkBorder::GetInnerBorderType() const
{
	return (InnerBorderType)InnerBorder;
}

inline double emTkBorder::GetBorderScaling() const
{
	return BorderScaling;
}

inline const emTkLook & emTkBorder::GetLook() const
{
	return Look;
}

inline bool emTkBorder::HasAux() const
{
	return Aux!=NULL;
}

inline bool emTkBorder::IsLabelInBorder() const
{
	return LabelInBorder;
}

inline const emTkBorder::TkResources & emTkBorder::GetTkResources() const
{
	return TkResVarModel->Var;
}


//==============================================================================
//================================= emTkLabel ==================================
//==============================================================================

class emTkLabel : public emTkBorder {

public:

	// A panel of this class simply shows the label as the content, and it
	// is not focusable by default.

	emTkLabel(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emTkBorder, but it performs:
		//  SetOuterBorderType(OBT_MARGIN);
		//  SetLabelInBorder(false);
		//  SetFocusable(false);

protected:

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);
		// Paints the label.
};


//==============================================================================
//================================= emTkTiling =================================
//==============================================================================

class emTkTiling : public emTkBorder {

public:

	// A panel of this class automatically lays out any child panels within
	// the content area, just like in a rectangular tiling. By default, the
	// panel itself is not focusable and has no border, because it is meant
	// as a pure layout programming tool. For other use, see the derived
	// class emTkGroup.

	emTkTiling(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emTkBorder, but sets non-focusable.

	virtual ~emTkTiling();
		// Destructor.

	bool IsRowByRow() const;
	void SetRowByRow(bool rowByRow=true);
		// Whether to layout the child panels column-by-column (false,
		// the default) or row-by-row (true). Here is an example for the
		// order of 10 panels in a 4x3 grid:
		//   column-by-column      row-by-row
		//      1  4  7 10         1  2  3  4
		//      2  5  8            5  6  7  8
		//      3  6  9            9 10

	int GetFixedColumnCount() const;
	int GetFixedRowCount() const;
	void SetFixedColumnCount(int fixedColumnCount);
	void SetFixedRowCount(int fixedRowCount);
		// Whether to have a fixed number of columns or rows, and how
		// many. A value less or equal zero means to have no fixed
		// number of columns or rows and to let the layout algorithm
		// decide it. If both are fixed and if there are more child
		// panels than cells, the row count is increased accordingly.
		// The default is zero for both.

	int GetMinCellCount() const;
	void SetMinCellCount(int minCellCount);
		// Minimum number of cells to be generated. The layout algorithm
		// behaves like if there were at least this number of child
		// panels. The additional cells are simply making up unused
		// space. The default is zero.

	void SetChildTallness(double ct);
		// Set the demanded tallness of child panels (height/width
		// ratio). This method is a short cut for:
		//   SetPrefChildTallness(ct);
		//   SetChildTallnessForced();

	void SetPrefChildTallness(double pct);
		// Set the preferred tallness of child panels (height/width
		// ratio). The child panels may get another tallness, for
		// filling the available space best possible. This method is a
		// short cut for: SetPrefChildTallness(pct,0,true);

	double GetPrefChildTallness(int idx) const;
	void SetPrefChildTallness(double pct, int idx, bool allFurther=true);
		// Preferred tallness of child panels in the first row and in
		// the first column. The index idx denotes a cell. It is a cell
		// in the first row (idx>0) or in the first column (idx<0), or
		// both (idx==0). Other cells cannot be set, because they are
		// sized implicitly by the first row and column. Here is a chart
		// showing the meaning of idx, and the effect of different
		// tallnesses:
		//
		//   +--------+-------+-------------+-----+----
		//   | idx  0 | idx 1 |    idx 2    |idx 3| ...
		//   +--------+-------+-------------+-----+----
		//   |        |       |             |     |
		//   | idx -1 |       |             |     |
		//   |        |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   | idx -2 |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   |        |       |             |     |
		//   |        |       |             |     |
		//   | idx -3 |       |             |     |
		//   |        |       |             |     |
		//   |        |       |             |     |
		//   +--------+-------+-------------+-----+----
		//   | ...    |       |             |     |
		//
		// Now to the important trick argument "allFurther": If true
		// (the default), all further cells in the row and/or column are
		// even set. For example, SetPrefChildTallness(1.0,3) means to
		// set the cells 3, 4, 5, ... INT_MAX. And
		// SetPrefChildTallness(1.0,-2) means to set the cells -2, -3,
		// -4, ... INT_MIN. And SetPrefChildTallness(1.0,-3,false) means
		// to set cell -3 only. And SetPrefChildTallness(1.0,0) means to
		// set all cells at once.
		//
		// There is no limit for idx except through the memory required
		// by internal arrays.
		//
		// The default preferred child tallness is: 0.2

	void SetChildTallnessForced(bool forced=true);
		// Whether the preferred child tallness has to be applied in any
		// case, instead of deviating for filling the whole available
		// space. This method is a short cut for:
		//   SetForcedChildTallnessColumn(forced ? 0 : -1);
		//   SetForcedChildTallnessRow(forced ? 0 : -1);

	int GetForcedChildTallnessColumn() const;
	int GetForcedChildTallnessRow() const;
	void SetForcedChildTallnessColumn(int column);
	void SetForcedChildTallnessRow(int row);
		// Index of a single column or row, in which the preferred child
		// tallness has to be applied in any case. -1 means to have no
		// such column or row (this is the default). Understand that if
		// both a row and a column are forced, all cells of the whole
		// grid are forced implicitly.

	emAlignment GetAlignment() const;
	void SetAlignment(emAlignment alignment);
		// Alignment of the child panels as a whole within the available
		// space. This applies only if the available space cannot be
		// filled completely, with respect to forced child tallness. The
		// default is EM_ALIGN_CENTER.

	double GetSpaceL() const;
	double GetSpaceT() const;
	double GetSpaceH() const;
	double GetSpaceV() const;
	double GetSpaceR() const;
	double GetSpaceB() const;
	void SetSpaceL(double l);
	void SetSpaceT(double t);
	void SetSpaceH(double h);
	void SetSpaceV(double v);
	void SetSpaceR(double r);
	void SetSpaceB(double b);
	void SetSpace(double l, double t, double h, double v, double r,
	              double b);
	void SetSpace(double lr, double tb, double h, double v);
	void SetInnerSpace(double h, double v);
	void SetOuterSpace(double l, double t, double r, double b);
	void SetOuterSpace(double lr, double tb);
		// Left, top, horizontal, vertical, right and bottom space (l,
		// t, h, v, r, b). This defines space between child panels, and
		// between child panels and borders. Here is a chart showing the
		// meaning of the six parameters:
		//
		//   +-------------------- border -------------------+
		//   |         t             t             t         |
		//   |     +-------+     +-------+     +-------+     |
		//   |  l  + panel +  h  + panel +  h  + panel +  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         v             v             v         |
		//   |     +-------+     +-------+     +-------+     |
		//   |  l  + panel +  h  + panel +  h  + panel +  r  |
		//   |     +-------+     +-------+     +-------+     |
		//   |         b             b             b         |
		//   +-----------------------------------------------+
		//
		// The method argument lr means to set l and r to the same
		// value, same with tb for t and b.
		//
		// The values are relative to the average size of the child
		// panels. For example, v=0.5 means that the vertical space
		// between child panels gets half as tall as an average child
		// panel. The default is zero for all parameters.

protected:

	virtual void LayoutChildren();
		// Tiles all child panels in the content area (except for an
		// auxiliary panel, which is laid out in the border).

private:

	double SpaceL,SpaceT,SpaceH,SpaceV,SpaceR,SpaceB;
	double PCT;
	emArray<double> PCTPos, PCTNeg;
	int FixedColumnCount,FixedRowCount,MinCellCount,FCTColumn,FCTRow;
	emAlignment Alignment;
	bool RowByRow;
};

inline bool emTkTiling::IsRowByRow() const
{
	return RowByRow;
}

inline int emTkTiling::GetFixedColumnCount() const
{
	return FixedColumnCount;
}

inline int emTkTiling::GetFixedRowCount() const
{
	return FixedRowCount;
}

inline int emTkTiling::GetMinCellCount() const
{
	return MinCellCount;
}

inline int emTkTiling::GetForcedChildTallnessColumn() const
{
	return FCTColumn;
}

inline int emTkTiling::GetForcedChildTallnessRow() const
{
	return FCTRow;
}

inline emAlignment emTkTiling::GetAlignment() const
{
	return Alignment;
}

inline double emTkTiling::GetSpaceL() const
{
	return SpaceL;
}

inline double emTkTiling::GetSpaceT() const
{
	return SpaceT;
}

inline double emTkTiling::GetSpaceH() const
{
	return SpaceH;
}

inline double emTkTiling::GetSpaceV() const
{
	return SpaceV;
}

inline double emTkTiling::GetSpaceR() const
{
	return SpaceR;
}

inline double emTkTiling::GetSpaceB() const
{
	return SpaceB;
}


//==============================================================================
//================================= emTkGroup ==================================
//==============================================================================

class emTkGroup : public emTkTiling {

public:

	// Class for a group of panels. Any user-created child panels are laid
	// out automatically. This is just like emTkTiling, but it has a group
	// border and it is focusable.

	emTkGroup(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
};


//==============================================================================
//================================= emTkTunnel =================================
//==============================================================================

class emTkTunnel : public emTkBorder {

public:

	// This panel shows a single child panel very small. Around that, a
	// decoration is painted which looks like a tunnel. Therefore the name
	// of this class. The single child panel is laid out automatically
	// whenever it is created by the user of this class.

	emTkTunnel(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.

	double GetChildTallness() const;
	void SetChildTallness(double childTallness);
		// Tallness for the child panel (end of tunnel). A value <=0.0
		// means to take the tallness of the content rectangle. That is
		// the default.

	double GetDepth() const;
	void SetDepth(double depth);
		// Depth of the tunnel. The formula is more or less:
		//   AreaOfEnd = AreaOfEntrance/((Depth+1)*(Depth+1))
		// The default is 10.0.

	virtual void GetChildRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	);
		// Get coordinates and canvas color of the end of the tunnel.

protected:

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

	virtual void LayoutChildren();

private:

	enum DoTunnelFunc {
		TUNNEL_FUNC_PAINT,
		TUNNEL_FUNC_CHILD_RECT
	};
	void DoTunnel(
		DoTunnelFunc func, const emPainter * painter,
		emColor canvasColor, double * pX, double * pY, double * pW,
		double * pH, emColor * pCanvasColor
	);

	double ChildTallness,Depth;
};

inline double emTkTunnel::GetChildTallness() const
{
	return ChildTallness;
}

inline double emTkTunnel::GetDepth() const
{
	return Depth;
}


//==============================================================================
//================================= emTkButton =================================
//==============================================================================

class emTkButton : public emTkBorder {

public:

	// Class for a button. Buttons can be triggered (clicked) by the user to
	// perform a function. The label is shown in the button face.

	emTkButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.

	virtual ~emTkButton();
		// Destructor.

	bool IsNoEOI() const;
	void SetNoEOI(bool noEOI=true);
		// Whether clicking this button is not an "End Of Interaction".
		// If false (the default), GetView().SignalEOIDelayed() is
		// called on every click.

	const emSignal & GetClickSignal() const;
		// This signal is signaled when the button has been clicked.

	const emSignal & GetPressStateSignal() const;
		// This signal is signaled when the press state has changed (see
		// IsPressed()).

	bool IsPressed() const;
		// Whether the button is currently pressed or not.

	void Click(bool shift=false);
		// Perform a button click programmatically.

protected:

	virtual void Clicked();
		// Called when the button has been clicked.

	virtual void PressStateChanged();
		// Called when the press state has changed.

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo();
	virtual emString GetHowTo();

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

	virtual void PaintBoxSymbol(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

	virtual bool CheckMouse(double mx, double my);

	bool IsShownChecked() const;
	bool IsShownBoxed() const;
	bool IsShownRadioed() const;
	void SetShownChecked(bool shownChecked);
	void SetShownBoxed(bool shownBoxed);
	void SetShownRadioed(bool shownRadioed);
		// Yes, this class has the ability to paint all our button
		// types.

private:

	enum DoButtonFunc {
		BUTTON_FUNC_PAINT,
		BUTTON_FUNC_CHECK_MOUSE
	};
	void DoButton(
		DoButtonFunc func, const emPainter * painter,
		emColor canvasColor,
		double mx, double my, bool * pHit
	);

	emSignal ClickSignal;
	emSignal PressStateSignal;
	unsigned Pressed : 1;
	unsigned NoEOI : 1;
	unsigned ShownChecked : 1;
	unsigned ShownBoxed : 1;
	unsigned ShownRadioed : 1;

	static const char * HowToButton;
	static const char * HowToEOIButton;
};

inline bool emTkButton::IsNoEOI() const
{
	return NoEOI;
}

inline const emSignal & emTkButton::GetClickSignal() const
{
	return ClickSignal;
}

inline const emSignal & emTkButton::GetPressStateSignal() const
{
	return PressStateSignal;
}

inline bool emTkButton::IsPressed() const
{
	return Pressed;
}

inline bool emTkButton::IsShownChecked() const
{
	return ShownChecked;
}

inline bool emTkButton::IsShownBoxed() const
{
	return ShownBoxed;
}

inline bool emTkButton::IsShownRadioed() const
{
	return ShownRadioed;
}


//==============================================================================
//============================== emTkCheckButton ===============================
//==============================================================================

class emTkCheckButton : public emTkButton {

public:

	// Class for a check button. This is like emTkButton, but a check state
	// is managed and shown. The check state toggles on every click of the
	// button, for switching something on and off.

	emTkCheckButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emTkButton. The initial check state is false.

	virtual ~emTkCheckButton();
		// Destructor.

	const emSignal & GetCheckSignal() const;
		// This signal is signaled when the check state has changed.

	bool IsChecked() const;
	void SetChecked(bool checked=true);
		// Get/set the check state of this button.

protected:

	virtual void Clicked();
		// See emTkButton. This implements the toggling of the check
		// state.

	virtual void CheckChanged();
		// Called when the check state has changed.

	virtual emString GetHowTo();

private:

	emSignal CheckSignal;
	bool Checked;

	static const char * HowToCheckButton;
	static const char * HowToChecked;
	static const char * HowToNotChecked;
};

inline const emSignal & emTkCheckButton::GetCheckSignal() const
{
	return CheckSignal;
}

inline bool emTkCheckButton::IsChecked() const
{
	return Checked;
}


//==============================================================================
//============================== emTkRadioButton ===============================
//==============================================================================

class emTkRadioButton : public emTkCheckButton {

public:

	// Class for a radio button. This is similar to a check button, but in a
	// set of radio buttons, only one button can have checked state, and the
	// user can unchecked a button only by checking another. That is the
	// usual behavior. Actually an emTkRadioButton does not modify its check
	// state on any click, as long as it is not a member of an
	// emTkRadioButton::Mechanism or emTkRadioButton::Group (it's not a must
	// to use these helper classes).

	emTkRadioButton(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Like emTkCheckButton.

	virtual ~emTkRadioButton();
		// Destructor. Removes the button from any Mechanism.

	class Mechanism : public emUncopyable {

	public:

		// Class for the mechanism of a set of radio buttons.

		Mechanism();
		virtual ~Mechanism();

		void Add(emTkRadioButton * radioButton);
			// Add a radio button to this mechanism. If the button
			// is already a member of another mechanism, it is
			// removed from that mechanism automatically.

		void AddAll(emPanel * parent);
			// Add all radio buttons which are children of the given
			// panel.

		void Remove(emTkRadioButton * radioButton);
		void RemoveByIndex(int index);
			// Remove a radio button from this mechanism.

		void RemoveAll();
			// Remove all radio buttons from this mechanism.

		const emSignal & GetCheckSignal() const;
			// This signal is signaled whenever there was a change
			// in the result of GetChecked().

		emTkRadioButton * GetChecked();
		const emTkRadioButton * GetChecked() const;
		void SetChecked(emTkRadioButton * radioButton);
			// Get/set the member button which is currently checked.
			// NULL means to have no member button checked.

		int GetCheckIndex();
		void SetCheckIndex(int index);
			// Get/set the index of the member button which is
			// currently checked. -1 means to have no member button
			// checked.

		int GetCount();
			// Get number of member buttons.

		int GetIndexOf(const emTkRadioButton * button) const;
			// Get the index of a member button, or -1 if not found.

	protected:

		virtual void CheckChanged();
			// Called whenever there was a change in the result of
			// GetChecked().

	private:

		emArray<emTkRadioButton *> Array;
		emSignal CheckSignal;
		int CheckIndex;
	};

	class Group : public emTkGroup, public Mechanism {

	public:

		// Combination of emTkGroup and Mechanism. Any radio buttons
		// created as children of such a group are added automatically
		// to the mechanism (this magic happens in the constructor of
		// emTkRadioButton).

		Group(
			ParentArg parent, const emString & name,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		);
			// Like the constructor of emTkGroup.

		virtual ~Group();
			// Destructor.
	};

protected:

	virtual void Clicked();

	virtual void CheckChanged();

	virtual emString GetHowTo();

private:

	friend class Mechanism;

	Mechanism * Mech;
	int MechIndex;

	static const char * HowToRadioButton;
};

inline const emSignal & emTkRadioButton::Mechanism::GetCheckSignal() const
{
	return CheckSignal;
}

inline emTkRadioButton * emTkRadioButton::Mechanism::GetChecked()
{
	return CheckIndex>=0 ? Array[CheckIndex] : NULL;
}

inline const emTkRadioButton * emTkRadioButton::Mechanism::GetChecked() const
{
	return CheckIndex>=0 ? Array[CheckIndex] : NULL;
}

inline int emTkRadioButton::Mechanism::GetCheckIndex()
{
	return CheckIndex;
}

inline int emTkRadioButton::Mechanism::GetCount()
{
	return Array.GetCount();
}

inline int emTkRadioButton::Mechanism::GetIndexOf(
	const emTkRadioButton * button
) const
{
	return button && button->Mech==this ? button->MechIndex : -1;
}


//==============================================================================
//================================ emTkCheckBox ================================
//==============================================================================

class emTkCheckBox : public emTkCheckButton {

public:

	// This is like emTkCheckButton, but with a different visualization:
	// Instead of a push button, a small check box is shown with the label
	// on the right.

	emTkCheckBox(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
};


//==============================================================================
//================================ emTkRadioBox ================================
//==============================================================================

class emTkRadioBox : public emTkRadioButton {

public:

	// This is like emTkRadioButton, but with a different visualization:
	// Instead of a push button, a small check box is shown with the label
	// on the right.

	emTkRadioBox(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
};


//==============================================================================
//=============================== emTkTextField ================================
//==============================================================================

class emTkTextField : public emTkBorder {

public:

	// Class for a data field panel showing a single line of text which can
	// optionally be edited by the user. An optional multi-line mode is also
	// provided. Selection and clipboard functions are supported.

	emTkTextField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		const emString & text=emString(),
		bool editable=false
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   editable    - Whether the text can be edited by the user.

	virtual ~emTkTextField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the text can be edited by the user.

	bool GetMultiLineMode() const;
	void SetMultiLineMode(bool multiLineMode=true);
		// Whether the text may have multiple lines.

	bool GetPasswordMode() const;
	void SetPasswordMode(bool passwordMode=true);
		// Whether the text is a password that should not really be
		// shown.

	bool GetOverwriteMode() const;
	void SetOverwriteMode(bool overwriteMode=true);
		// Current mode of overwriting or inserting (can be changed with
		// the insert key).

	const emSignal & GetTextSignal() const;
		// Signaled whenever the text has changed.

	const emString & GetText() const;
	void SetText(const emString & text);
		// The text.

	int GetTextLen() const;
	int GetCursorIndex() const;
	void SetCursorIndex(int index);
	const emSignal & GetSelectionSignal() const;
	int GetSelectionStartIndex() const;
	int GetSelectionEndIndex() const;
	void Select(int startIndex, int endIndex, bool publish);
	bool IsSelectionEmpty() const;
	void EmptySelection();
	void SelectAll(bool publish);
	void PublishSelection();
	void CutSelectedTextToClipboard();
	void CopySelectedTextToClipboard();
	void PasteSelectedTextFromClipboard();
	void PasteSelectedText(const emString & text);
	void DeleteSelectedText();
	bool IsCursorBlinkOn() const;
		// Advanced stuff - still undocumented.

protected:

	virtual void TextChanged();
		// Called when the text has changed.

	virtual void SelectionChanged();
		// Called when the selection has changed.

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo();
	virtual emString GetHowTo();

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

	virtual bool CheckMouse(double mx, double my, double * pCol, double * pRow);

private:

	enum DoTextFieldFunc {
		TEXT_FIELD_FUNC_PAINT,
		TEXT_FIELD_FUNC_XY2CR,
		TEXT_FIELD_FUNC_CR2XY
	};
	void DoTextField(
		DoTextFieldFunc func, const emPainter * painter,
		emColor canvasColor,
		double xIn, double yIn, double * pXOut, double * pYOut, bool * pHit
	);

	enum DragModeType {
		DM_NONE,
		DM_SELECT,
		DM_SELECT_BY_WORDS,
		DM_SELECT_BY_ROWS,
		DM_INSERT,
		DM_MOVE
	};

	void SetDragMode(DragModeType dragMode);
	void RestartCursorBlinking();
	void ScrollToCursor();
	int ColRow2Index(double column, double row, bool forCursor) const;
	void Index2ColRow(int index, int * pColumn, int * pRow) const;
	void CalcTotalColsRows(int * pCols, int * pRows) const;
	int GetNormalizedIndex(int index) const;
	void ModifySelection(int oldColumn, int newColumn, bool publish);
	int GetNextIndex(int index) const;
	int GetPrevIndex(int index) const;
	int GetNextWordBoundaryIndex(int index, bool * pIsDelimiter=NULL) const;
	int GetPrevWordBoundaryIndex(int index, bool * pIsDelimiter=NULL) const;
	int GetNextWordIndex(int index) const;
	int GetPrevWordIndex(int index) const;
	int GetRowStartIndex(int index) const;
	int GetRowEndIndex(int index) const;
	int GetNextRowIndex(int index) const;
	int GetPrevRowIndex(int index) const;
	int GetNextParagraphIndex(int index) const;
	int GetPrevParagraphIndex(int index) const;

	emRef<emClipboard> Clipboard;
	emSignal TextSignal;
	emSignal SelectionSignal;
	bool Editable;
	bool MultiLineMode;
	bool PasswordMode;
	bool OverwriteMode;
	emString Text;
	int TextLen,CursorIndex,SelectionStartIndex,SelectionEndIndex;
	int MagicCursorColumn;
	emInt64 SelectionId;
	emUInt64 CursorBlinkTime;
	bool CursorBlinkOn;
	DragModeType DragMode;
	double DragPosC,DragPosR;

	static const char * HowToTextField;
	static const char * HowToMultiLineOff;
	static const char * HowToMultiLineOn;
	static const char * HowToReadOnly;
};

inline bool emTkTextField::IsEditable() const
{
	return Editable;
}

inline bool emTkTextField::GetMultiLineMode() const
{
	return MultiLineMode;
}

inline bool emTkTextField::GetPasswordMode() const
{
	return PasswordMode;
}

inline bool emTkTextField::GetOverwriteMode() const
{
	return OverwriteMode;
}

inline const emSignal & emTkTextField::GetTextSignal() const
{
	return TextSignal;
}

inline const emString & emTkTextField::GetText() const
{
	return Text;
}

inline int emTkTextField::GetTextLen() const
{
	return TextLen;
}

inline int emTkTextField::GetCursorIndex() const
{
	return CursorIndex;
}

inline const emSignal & emTkTextField::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline int emTkTextField::GetSelectionStartIndex() const
{
	return SelectionStartIndex;
}

inline int emTkTextField::GetSelectionEndIndex() const
{
	return SelectionEndIndex;
}

inline bool emTkTextField::IsSelectionEmpty() const
{
	return SelectionStartIndex>=SelectionEndIndex;
}

inline bool emTkTextField::IsCursorBlinkOn() const
{
	return CursorBlinkOn;
}


//==============================================================================
//============================== emTkScalarField ===============================
//==============================================================================

class emTkScalarField : public emTkBorder {

public:

	// Class for a data field panel showing a scalar value which can
	// optionally be edited by the user. The scalar value is a 64-bit signed
	// integer number, but on the shown scale the values can be translated
	// to any texts (e.g. rational numbers, names ,...).

	emTkScalarField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		emInt64 minValue=0, emInt64 maxValue=10, emInt64 value=0,
		bool editable=false
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   minValue    - Minimum allowed value.
		//   maxValue    - Maximum allowed value.
		//   value       - Initial value.
		//   editable    - Whether the value can be edited by the user.

	virtual ~emTkScalarField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the value can be edited by the user.

	emInt64 GetMinValue() const;
	emInt64 GetMaxValue() const;
	void SetMinValue(emInt64 minValue);
	void SetMaxValue(emInt64 maxValue);
	void SetMinMaxValues(emInt64 minValue, emInt64 maxValue);
		// Get/set the range of possible values.

	const emSignal & GetValueSignal() const;
		// This signal is signaled after each change of the value.

	emInt64 GetValue() const;
	void SetValue(emInt64 value);
		// Get/set the value.

	const emArray<emUInt64> & GetScaleMarkIntervals() const;
	void SetScaleMarkIntervals(const emArray<emUInt64> & intervals);
	void SetScaleMarkIntervals(unsigned interval1, unsigned interval2, ...);
		// Get/set the layout of scale marks. It is an array of
		// intervals between the scale marks of different size levels.
		// The first interval is for the largest scale marks, the second
		// is for the second-largest scale marks and so on. Thus, the
		// array must be sorted from large to small. For example, a
		// classic centimeter rule would have {10,5,1}, with the values
		// in millimeters. The default is {1}. The arguments to the
		// ellipse version method must be terminated by a 0. Note that
		// the ellipse version can take only 32-bit intervals (I am not
		// sure whether an ellipse on emUInt64 would be portable).

	bool IsNeverHidingMarks() const;
	void SetNeverHideMarks(bool neverHide);
		// By default, marks with an interval greater than
		// (MaxValue - MinValue) are automatically not shown, so that
		// the other marks can be seen better. Setting true here
		// disables that automatism.

	virtual void TextOfValue(char * buf, int bufSize, emInt64 value,
	                         emUInt64 markInterval) const;
		// Convert a scale mark value to a null-terminated character
		// string for display on the scale. The default implementation
		// uses the callback function set with SetTextOfValueFunc.

	void SetTextOfValueFunc(
		void(*textOfValueFunc)(
			char * buf, int bufSize, emInt64 value,
			emUInt64 markInterval, void * context
		),
		void * context=NULL
	);
		// Set a function for converting a scale mark value to a
		// null-terminated character string for display on the scale.
		// The context argument is forwarded to the function for any
		// use. The default performs simple decimal conversion.

	static void DefaultTextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
		// This is the default text-of-value-function. It performs
		// decimal conversion.

	double GetTextBoxTallness() const;
	void SetTextBoxTallness(double textBoxTallness);
		// Tallness of the text box of a scale mark. The default is 0.5.

	emUInt64 GetKeyboardInterval() const;
	void SetKeyboardInterval(emUInt64 kbInterval);
		// How much to add and sub to the value by '+' and '-' keys. The
		// default is zero which means to choose a good interval
		// automatically.

protected:

	virtual void ValueChanged();
		// Called when the value has changed.

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);

	virtual bool HasHowTo();
	virtual emString GetHowTo();

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

	virtual bool CheckMouse(double mx, double my, emInt64 * pValue);

private:

	enum DoScalarFieldFunc {
		SCALAR_FIELD_FUNC_PAINT,
		SCALAR_FIELD_FUNC_CHECK_MOUSE
	};
	void DoScalarField(
		DoScalarFieldFunc func, const emPainter * painter,
		emColor canvasColor,
		double mx, double my, emInt64 * pValue, bool * pHit
	);

	void StepByKeyboard(int dir);

	bool Editable;
	emSignal ValueSignal;
	emInt64 MinValue,MaxValue;
	emInt64 Value;
	emArray<emUInt64> ScaleMarkIntervals;
	bool MarksNeverHidden;
	void(*TextOfValueFunc)(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	void * TextOfValueFuncContext;
	double TextBoxTallness;
	emUInt64 KBInterval;
	bool Pressed;

	static const char * HowToScalarField;
	static const char * HowToReadOnly;
};

inline bool emTkScalarField::IsEditable() const
{
	return Editable;
}

inline emInt64 emTkScalarField::GetMinValue() const
{
	return MinValue;
}

inline emInt64 emTkScalarField::GetMaxValue() const
{
	return MaxValue;
}

inline const emSignal & emTkScalarField::GetValueSignal() const
{
	return ValueSignal;
}

inline emInt64 emTkScalarField::GetValue() const
{
	return Value;
}

inline const emArray<emUInt64> & emTkScalarField::GetScaleMarkIntervals() const
{
	return ScaleMarkIntervals;
}

inline bool emTkScalarField::IsNeverHidingMarks() const
{
	return MarksNeverHidden;
}

inline double emTkScalarField::GetTextBoxTallness() const
{
	return TextBoxTallness;
}

inline emUInt64 emTkScalarField::GetKeyboardInterval() const
{
	return KBInterval;
}


//==============================================================================
//=============================== emTkColorField ===============================
//==============================================================================

class emTkColorField : public emTkBorder {

public:

	// Class for a data field panel showing a color which can optionally be
	// edited by the user.

	emTkColorField(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		emColor color=0x000000ff, bool editable=false,
		bool alphaEnabled=false
	);
		// Constructor.
		// Arguments:
		//   parent       - Parent for this panel (emPanel or emView).
		//   name         - The name for this panel.
		//   caption      - The label's caption, or empty.
		//   description  - The label's description, or empty.
		//   icon         - The label's icon, or empty.
		//   color        - Initial color.
		//   editable     - Whether the color can be edited by the user.
		//   alphaEnabled - Whether the alpha channel of the color is
		//                  relevant.

	virtual ~emTkColorField();
		// Destructor.

	bool IsEditable() const;
	void SetEditable(bool editable=true);
		// Whether the color can be edited by the user.

	bool IsAlphaEnabled() const;
	void SetAlphaEnabled(bool alphaEnabled=true);
		// Whether the alpha channel of the color is relevant.

	const emSignal & GetColorSignal() const;
		// This signal is signaled after each change of the color.

	emColor GetColor() const;
	void SetColor(emColor value);
		// Get/set the color.

protected:

	virtual void ColorChanged();
		// Called when the color has changed.

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();
	virtual void LayoutChildren();

	virtual bool HasHowTo();
	virtual emString GetHowTo();

	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);

private:

	void UpdateRGBAOutput();
	void UpdateHSVOutput(bool initial=false);
	void UpdateNameOutput();
	void UpdateExpAppearance();

	static void TextOfPercentValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfHueValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);

	struct Expansion {
		emTkTiling * Tiling;
		emTkScalarField * SfRed;
		emTkScalarField * SfGreen;
		emTkScalarField * SfBlue;
		emTkScalarField * SfAlpha;
		emTkScalarField * SfHue;
		emTkScalarField * SfSat;
		emTkScalarField * SfVal;
		emTkTextField * TfName;
		emInt64 RedOut,GreenOut,BlueOut,AlphaOut,HueOut,SatOut,ValOut;
		emString NameOut;
	};

	emSignal ColorSignal;
	Expansion * Exp;
	emColor Color;
	bool Editable;
	bool AlphaEnabled;
	bool Pressed;

	static const char * HowToColorField;
	static const char * HowToReadOnly;
};

inline bool emTkColorField::IsEditable() const
{
	return Editable;
}

inline bool emTkColorField::IsAlphaEnabled() const
{
	return AlphaEnabled;
}

inline const emSignal & emTkColorField::GetColorSignal() const
{
	return ColorSignal;
}

inline emColor emTkColorField::GetColor() const
{
	return Color;
}


//==============================================================================
//================================ emTkSplitter ================================
//==============================================================================

class emTkSplitter : public emTkBorder {

public:

	// Class for a splitter panel. Such a panel can get two child panels
	// which are laid out automatically, either horizontally or vertically,
	// filling the whole content area. Between the two child panels is a
	// grip which can be dragged by the user, for making one of the panels
	// larger while the other gets smaller.

	emTkSplitter(
		ParentArg parent, const emString & name,
		const emString & caption=emString(),
		const emString & description=emString(),
		const emImage & icon=emImage(),
		bool vertical=false, double minPos=0.0, double maxPos=1.0,
		double pos=0.5
	);
		// Constructor.
		// Arguments:
		//   parent      - Parent for this panel (emPanel or emView).
		//   name        - The name for this panel.
		//   caption     - The label's caption, or empty.
		//   description - The label's description, or empty.
		//   icon        - The label's icon, or empty.
		//   vertical    - See SetVertical.
		//   minPos      - See SetMinMaxPos.
		//   maxPos      - See SetMinMaxPos.
		//   pos         - See SetPos.

	virtual ~emTkSplitter();
		// Destructor.

	bool IsVertical() const;
	void SetVertical(bool vertical=true);
		// Whether the child panels are laid out left-right (false) or
		// on top of each other (true).

	double GetMinPos() const;
	double GetMaxPos() const;
	double GetPos() const;
	void SetMinMaxPos(double minPos, double maxPos);
	void SetPos(double pos);
		// Get/set minimum, maximum and current position of the grip.
		// The position ranges from 0.0 to 1.0 (0.0 = first child panel
		// collapsed, 1.0 = second child panel collapsed).

	const emSignal & GetPosSignal() const;
		// This signal is signaled after each change of the grip
		// position.

protected:

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual emCursor GetCursor();
	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	);
	virtual void LayoutChildren();

private:

	void CalcGripRect(
		double contentX, double contentY, double contentW,
		double contentH, double * pX, double * pY, double * pW,
		double * pH
	);

	bool Vertical;
	double MinPos;
	double MaxPos;
	double Pos;
	emSignal PosSignal;
	bool Pressed;
	double MousePosInGrip;
	bool MouseInGrip;
};

inline bool emTkSplitter::IsVertical() const
{
	return Vertical;
}

inline double emTkSplitter::GetMinPos() const
{
	return MinPos;
}

inline double emTkSplitter::GetMaxPos() const
{
	return MaxPos;
}

inline double emTkSplitter::GetPos() const
{
	return Pos;
}

inline const emSignal & emTkSplitter::GetPosSignal() const
{
	return PosSignal;
}


//==============================================================================
//================================= emTkDialog =================================
//==============================================================================

class emTkDialog : public emWindow {

public:

	// Class for a dialog window. Such a dialog has a content area and a
	// button area. The content area is an emTkTiling which can be given
	// individual child panels. The button area can have buttons like "OK"
	// and "Cancel" for finishing the dialog.

	emTkDialog(
		emContext & parentContext,
		ViewFlags viewFlags=VF_POPUP_ZOOM|VF_ROOT_SAME_TALLNESS,
		WindowFlags windowFlags=WF_MODAL,
		const emString & wmResName="emTkDialog"
	);
		// Like the constructor of emWindow, but see that the default
		// argument values are different (it's a modal dialog with
		// popup-zoom by default).

	virtual ~emTkDialog();
		// Destructor.

	void SetRootTitle(const emString & title);
		// Set the title for this dialog. More precise, set the title
		// for the private root panel of this view. If you create some
		// content panel with another title, and if it gets focus, that
		// title is shown. The default title is an empty string.

	emTkTiling * GetContentTiling();
		// This panel makes up the content area of the dialog, not
		// including the buttons. For convenience, it is an emTkTiling
		// with default properties, except that the inner border is set
		// to emTkBorder::IBT_CUSTOM_RECT. You may change the properties
		// as you wish, and you should give it one or more child panels
		// as the content.

	void AddPositiveButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
	void AddNegativeButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
	void AddCustomButton(
		const emString & caption,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// Add a button to the button area. These buttons are finishing
		// the dialog. For the meaning of "Positive", "Negative" and
		// "Custom", please see GetResult().

	void AddOKButton();
	void AddCancelButton();
	void AddOKCancelButtons();
		// AddOKButton() is like AddPositiveButton("OK").
		// AddCancelButton() is like AddNegativeButton("Cancel").
		// AddOKCancelButtons() is like AddOKButton() plus
		// AddCancelButton().

	emTkButton * GetButton(int index);
		// Get a button. The index is: 0 for the first added button, 1
		// for the second added button, and so on.

	const emSignal & GetFinishSignal() const;
		// Signaled when any of the buttons has been triggered, or by
		// pressing the Enter key or the Escape key, or by the window
		// close signal. It is okay not to destruct the dialog and to
		// wait for another finish signal.

	enum {
		// Possible results:
		POSITIVE=1, // Positive button triggered or Enter key pressed.
		NEGATIVE=0, // Negative button triggered or Escape key pressed
		            // or window-closing commanded (see GetCloseSignal).
		CUSTOM1 =2, // First custom button triggered.
		CUSTOM2 =3, // Second custom button triggered.
		CUSTOM3 =4  // ...
		// Continued (customIndex=result+1-CUSTOM1)
	};
	int GetResult() const;
		// The result should be asked after the finish signal has been
		// signaled. Before that, the result is not valid.

	void Finish(int result);
		// Finish this dialog with the given result programmatically.

	void EnableAutoDeletion(bool autoDelete=true);
	bool IsAutoDeletionEnabled();
		// Whether to delete this object automatically a few time slices
		// after the dialog has finished.

	static void ShowMessage(
		emContext & parentContext,
		const emString & title,
		const emString & message,
		const emString & description=emString(),
		const emImage & icon=emImage()
	);
		// This function creates a modal dialog with an emTkLabel as the
		// content, and with an OK button. The dialog deletes itself
		// when finished. The argument 'message' is the caption of the
		// label.

protected:

	virtual void Finished(int result);
		// Like the finish signal. Default implementation does nothing.
		// It's allowed to delete (destruct) this dialog herein.

private:

	bool PrivateCycle();

	class DlgButton : public emTkButton {
	public:
		DlgButton(
			ParentArg parent, const emString & name,
			const emString & caption,
			const emString & description,
			const emImage & icon,
			int result
		);
	protected:
		virtual void Clicked();
	private:
		int Result;
	};

	class DlgPanel : public emTkBorder {
	public:
		DlgPanel(ParentArg parent, const emString & name);
		virtual ~DlgPanel();
		void SetTitle(const emString & title);
		virtual emString GetTitle();
		emString Title;
		emTkTiling * ContentTiling;
		emTkTiling * ButtonTiling;
	protected:
		virtual void Input(
			emInputEvent & event, const emInputState & state,
			double mx, double my
		);
		virtual void LayoutChildren();
	};

	class PrivateEngineClass : public emEngine {
	public:
		PrivateEngineClass(emTkDialog & dlg);
	protected:
		virtual bool Cycle();
		emTkDialog & Dlg;
	};
	friend class PrivateEngineClass;

	PrivateEngineClass PrivateEngine;
	emSignal FinishSignal;
	int Result;
	int ButtonNum,CustomRes;
	int FinishState;
	bool ADEnabled;
};

inline emTkTiling * emTkDialog::GetContentTiling()
{
	return ((DlgPanel*)GetRootPanel())->ContentTiling;
}

inline const emSignal & emTkDialog::GetFinishSignal() const
{
	return FinishSignal;
}

inline int emTkDialog::GetResult() const
{
	return Result;
}

inline bool emTkDialog::IsAutoDeletionEnabled()
{
	return ADEnabled;
}


#endif
