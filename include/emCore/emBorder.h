//------------------------------------------------------------------------------
// emBorder.h
//
// Copyright (C) 2005-2010,2014-2016 Oliver Hamann.
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

#ifndef emBorder_h
#define emBorder_h

#ifndef emVarModel_h
#include <emCore/emVarModel.h>
#endif

#ifndef emLook_h
#include <emCore/emLook.h>
#endif


//==============================================================================
//================================== emBorder ==================================
//==============================================================================

class emBorder : public emPanel {

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

	emBorder(
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

	virtual ~emBorder();
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

	bool IsIconAboveCaption() const;
	void SetIconAboveCaption(bool iconAboveCaption=true);
		// Whether the icon is shown above the caption (true), or if it
		// is shown to the left of the caption (false, the default).

	double GetMaxIconAreaTallness() const;
	void SetMaxIconAreaTallness(double maxIconAreaTallness);
		// Maximum tallness (height/width ratio) of the area in the
		// label preserved for the icon (if the label has an icon). The
		// default is 1.0. If you have a group of elements (e.g.
		// buttons) which show icons and captions, and if the icon
		// images have different tallnesses, then it is a good idea to
		// set this parameter to the minimum(!) tallness of all the icon
		// images. Thereby, the icons and captions of all the elements
		// will be aligned pretty equal.

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
			// this is used by emLabel and emCheckBox). Larger
			// margins should be solved through the parent panel,
			// e.g. see emLinearLayout::SetSpace.
		OBT_MARGIN_FILLED,
			// Like OBT_MARGIN, but fill the whole background.
		OBT_RECT,
			// Have a rectangular outer border line and fill the
			// rectangle with background color. Have a small margin.
		OBT_ROUND_RECT,
			// Like OBT_RECT but with round corners.
		OBT_GROUP,
			// Have a small special outer border for groups (used by
			// group panels like emLinearGroup and so on).
		OBT_INSTRUMENT,
			// Like OBT_GROUP, but the border line is thicker (for
			// example, this is used by emTextField).
		OBT_INSTRUMENT_MORE_ROUND,
			// Like OBT_INSTRUMENT, but with a larger corner radius
			// (this is used by emButton).
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

	const emLook & GetLook() const;
	virtual void SetLook(const emLook & look, bool recursively=false);
		// Look of this toolkit panel. At construction of a panel, the
		// look is copied from the parent panel (if the parent is not
		// emBorder, the grand parent is asked, and so on). When
		// setting the look with the argument recursively=true, all
		// descendant panels of class emBorder are even set through
		// calling emLook::Apply for every child panel.

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
	) const;
		// Get the coordinates and canvas color of the auxiliary area.
		// Valid only if HasAux()==true.

	virtual void GetSubstanceRect(double * pX, double * pY,
	                              double * pW, double * pH,
	                              double * pR) const;
		// Overloaded from emPanel (read there).

	virtual void GetContentRoundRect(
		double * pX, double * pY, double * pW, double * pH, double * pR,
		emColor * pCanvasColor=NULL
	) const;
		// Get the coordinates and canvas color of the content area as a
		// round rectangle (argument pR is for returning the radius of
		// the corners).

	virtual void GetContentRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) const;
		// Get the coordinates and canvas color of the content area as a
		// rectangle. If the inner border has round corners, the
		// rectangle returned here is smaller than with
		// GetContentRoundRect, so that it fits completely into the
		// content area.

	virtual void GetContentRectUnobscured(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) const;
		// Get the coordinates and canvas color of the unobscured part
		// of the content area. Some border types are painting an
		// overlay like a shadow at the edges of the content area, after
		// PaintContent is called. This does not work for child panels,
		// because they are painted after the overlay. Therefore child
		// panels should be laid out in the rectangle returned by
		// GetContentRectUnobscured. It returns the inner part, which is
		// not painted over.

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool IsOpaque() const;
	virtual void Paint(const emPainter & painter, emColor canvasColor) const;
	virtual void LayoutChildren();
		// See emPanel. Hint: For painting the content area, please
		// overload PaintContent instead of Paint, because with certain
		// border types, a shadow is painted over the content area.

	virtual bool HasHowTo() const;
	virtual emString GetHowTo() const;
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
	) const;
		// This can be overloaded for painting the content area. The
		// default implementation does nothing. The coordinates x,y,w,h
		// are like from GetContentRect, but you could even use the
		// coordinates returned by GetContentRoundRect.

	virtual bool HasLabel() const;
		// Whether this panel has a label. The default implementation
		// checks whether at least one of caption, description and icon
		// is not empty.

	virtual double GetBestLabelTallness() const;
		// Get the ideal tallness for the label area. The default
		// implementation calculates this for the default implementation
		// of PaintLabel.

	virtual void PaintLabel(
		const emPainter & painter, double x, double y, double w,
		double h, emColor color, emColor canvasColor
	) const;
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
		emImage ImgDir;
		emImage ImgDirUp;
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

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
public:
	virtual void GetContentRoundRect(
		double * pX, double * pY, double * pW, double * pH, double * pR,
		emColor * pCanvasColor=NULL
	) final;
	virtual void GetContentRect(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) final;
	virtual void GetContentRectUnobscured(
		double * pX, double * pY, double * pW, double * pH,
		emColor * pCanvasColor=NULL
	) final;
protected:
	virtual bool HasHowTo() final;
	virtual emString GetHowTo() final;
	virtual void PaintContent(
		const emPainter & painter, double x, double y, double w,
		double h, emColor canvasColor
	) final;
	virtual bool HasLabel() final;
	virtual double GetBestLabelTallness() final;
	virtual void PaintLabel(
		const emPainter & painter, double x, double y, double w,
		double h, emColor color, emColor canvasColor
	) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	enum DoBorderFunc {
		BORDER_FUNC_PAINT,
		BORDER_FUNC_SUBSTANCE_ROUND_RECT,
		BORDER_FUNC_CONTENT_ROUND_RECT,
		BORDER_FUNC_CONTENT_RECT,
		BORDER_FUNC_CONTENT_RECT_UNOBSCURED,
		BORDER_FUNC_AUX_RECT
	};
	void DoBorder(
		DoBorderFunc func, const emPainter * painter,
		emColor canvasColor, double * pX, double * pY, double * pW,
		double * pH, double * pR, emColor * pCanvasColor
	) const;

	enum DoLabelFunc {
		LABEL_FUNC_PAINT,
		LABEL_FUNC_GET_BEST_TALLNESS
	};
	void DoLabel(
		DoLabelFunc func, const emPainter * painter, double x, double y,
		double w, double h, emColor color, emColor canvasColor,
		double * pBestTallness
	) const;

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
	emLook Look;
	double MaxIconAreaTallness;
	double BorderScaling;
	emAlignment LabelAlignment;
	emAlignment CaptionAlignment;
	emAlignment DescriptionAlignment;
	emByte OuterBorder;
	emByte InnerBorder;
	bool IconAboveCaption;
	bool LabelInBorder;

	static const char * HowToPreface;
	static const char * HowToDisabled;
	static const char * HowToFocus;
};

inline const emString & emBorder::GetCaption() const
{
	return Caption;
}

inline const emString & emBorder::GetDescription() const
{
	return Description;
}

inline const emImage & emBorder::GetIcon() const
{
	return Icon;
}

inline emAlignment emBorder::GetLabelAlignment() const
{
	return LabelAlignment;
}

inline emAlignment emBorder::GetCaptionAlignment() const
{
	return CaptionAlignment;
}

inline emAlignment emBorder::GetDescriptionAlignment() const
{
	return DescriptionAlignment;
}

inline bool emBorder::IsIconAboveCaption() const
{
	return IconAboveCaption;
}

inline double emBorder::GetMaxIconAreaTallness() const
{
	return MaxIconAreaTallness;
}

inline emBorder::OuterBorderType emBorder::GetOuterBorderType() const
{
	return (OuterBorderType)OuterBorder;
}

inline emBorder::InnerBorderType emBorder::GetInnerBorderType() const
{
	return (InnerBorderType)InnerBorder;
}

inline double emBorder::GetBorderScaling() const
{
	return BorderScaling;
}

inline const emLook & emBorder::GetLook() const
{
	return Look;
}

inline bool emBorder::HasAux() const
{
	return Aux!=NULL;
}

inline bool emBorder::IsLabelInBorder() const
{
	return LabelInBorder;
}

inline const emBorder::TkResources & emBorder::GetTkResources() const
{
	return TkResVarModel->Var;
}


#endif
