//------------------------------------------------------------------------------
// emPanel.h
//
// Copyright (C) 2004-2008,2010-2012,2014-2017 Oliver Hamann.
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

#ifndef emPanel_h
#define emPanel_h

#ifndef emWindow_h
#include <emCore/emWindow.h>
#endif


//==============================================================================
//================================== emPanel ===================================
//==============================================================================

class emPanel : public emEngine {

public:

	// emPanel is the base class for all our panels. An emPanel is a
	// rectangular piece of a user interface which can be zoomed and
	// scrolled in an emView. Each panel can have child panels that are
	// making up more details. Child panels are always clipped by their
	// ancestors. That means, a panel can never be seen outside its parent.
	//
	// You should also read the comments on the class emView. A panel is
	// always a member of a tree of panels that is owned by a view. There is
	// no possibility to show a panel in multiple views. In addition, panels
	// could be created and destroyed very dynamically while the user
	// navigates around. Therefore, when talking about the classic
	// model-view concept, an emPanel should not be classified like a model.
	// It belongs the view. Practically this means that a panel object
	// should not contain any extensive model data. See class emModel for a
	// better place.
	//
	// Each panel has its own coordinate system. The origin is in the
	// upper-left corner of the panel, the X axis points to the right and
	// the Y-axis points to the bottom. The width of the panel is always 1.0
	// and the height depends on the layout.
	//
	// Note that an emView can have such a deep tree of panels, that the
	// precision of the data type "double" is not sufficient for converting
	// coordinates to a far ancestor or descendant panel. It is even not
	// always possible to calculate the view coordinates of a panel.
	// Therefore the view coordinates got with GetViewedX() and so on are
	// valid only if the panel is actually viewed (IsViewed()==true).
	//
	// There are many more topics around emPanel. Just sit back and read the
	// comments on all the methods.
	//
	// *********************************************************************
	// IMPORTANT HINT FOR THE ADVANCED PROGRAMMER: You should never create a
	// path of many panels where every panel has IsOpaque()==false and
	// GetCanvasColor().IsOpaque()==false simultaneously, because it would
	// mean that the view must paint all the panels on that path when the
	// last is viewed, and this could make the overall painting too slow.
	// *********************************************************************

	class ParentArgClass {
	public:
		// This is just a small helper class for the first argument of
		// the constructor of emPanel. It helps to avoid having multiple
		// versions of the panel constructor.
		ParentArgClass(emPanel & panel);
		ParentArgClass(emPanel * panel);
		ParentArgClass(emView & view);
		ParentArgClass(emView * view);
		emRootContext & GetRootContext() const;
		emView & GetView() const;
		emPanel * GetPanel() const;
	private:
		emView * View;
		emPanel * Panel;
	};

	typedef const ParentArgClass & ParentArg;
		// Type of the first argument of the constructor of emPanel.

	emPanel(ParentArg parent, const emString & name);
		// Construct a panel. Note that if this is not a root panel, the
		// panel will be initially hidden by being placed outside its
		// parent panel (call Layout to make visible).
		// Arguments:
		//   parent - The parent for this panel. This can be a parent
		//            panel or a view. If it is a view, this panel will
		//            be the root panel of that view. Otherwise it will
		//            be the last child of the parent panel. Note that
		//            the type ParentArg can be cast implicitly from:
		//            emPanel&, emPanel*, emView& and emView*. But the
		//            pointer must never be NULL.
		//   name   - The name for this panel. There must not be any
		//            sister panel with the same name.

	virtual ~emPanel();
		// Destruct this panel. Any child panels are deleted.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// This means emCrossPtr<emPanel> is possible.

	const emString & GetName() const;
		// Get the name of this panel.

	emString GetIdentity() const;
	static emString EncodeIdentity(const emArray<emString> & names);
	static emArray<emString> DecodeIdentity(const char * identity);
		// The identity of a panel consists of all the panel names on
		// the path from the root panel down to the identified panel.
		// They are delimited by colons, while colons and backslashes in
		// the names are quoted by backslashes.

	virtual emString GetTitle() const;
		// Get the title of this panel. Normally, the title of the
		// active panel is shown as the title of the view. The default
		// implementation of this method asks the parent panel. See
		// also: InvalidateTitle()

	virtual emString GetIconFileName() const;
		// Get the file name of an icon for this panel. For example,
		// this is used in bookmarks. The returned string can be the
		// name of a file in the default icon directory, or an absolute
		// file path. An empty string means to have no icon. The default
		// implementation of this method asks the parent panel.

	emView & GetView() const;
		// Get the view.

	emContext & GetViewContext() const;
		// Get the context of the view, this is just like GetView()
		// because emView is an emContext. (Yes, this method is
		// unnecessary)

	emRootContext & GetRootContext() const;
		// Get the root context (don't confuse with root panel).

	emWindow * GetWindow() const;
	emScreen * GetScreen() const;
		// These are short cuts for GetView().GetWindow() and
		// GetView().GetScreen().

	emPanel * GetParent() const;
		// Get the parent panel. Returns NULL if this is the root panel.

	emPanel * GetChild(const char * name) const;
		// Search a child panel by name. Returns NULL if there is no
		// such panel.

	emPanel * GetFirstChild() const;
	emPanel * GetLastChild() const;
		// Get the first or last child panel. The first child panel is
		// painted at first, and the last one is painted at last (first
		// means bottom, last means top). Returns NULL if this panel has
		// no children.

	emPanel * GetPrev() const;
	emPanel * GetNext() const;
		// Get the previous or next panel within the list of the
		// parent's list of children. It is painted before or after this
		// panel, respectively. Returns NULL if this is the first or
		// last child panel, respectively.

	void BeFirst();
	void BeLast();
	void BePrevOf(emPanel * sister);
	void BeNextOf(emPanel * sister);
		// Move this panel within the stacking order.

	void SortChildren(
		int(*compare)(emPanel * c1, emPanel * c2, void * context),
		void * context=NULL
	);
		// Sort all child panels.
		// Arguments:
		//   compare - A function for comparing two child panels. The
		//             result is:
		//               < 0   -  c1 is "less" than c2
		//               > 0   -  c1 is "greater" than c2
		//               == 0  -  c1 is "equal" to c2 (no change in
		//                        the order)
		//   context - Any pointer to be forwarded to the compare
		//             function.

	void DeleteAllChildren();
		// Delete all child panels.

	virtual void Layout(double layoutX, double layoutY,
	                    double layoutWidth, double layoutHeight,
	                    emColor canvasColor=0);
		// Move and/or resize this panel, and set the canvas color.
		// IMPORTANT: For best performance, please set the canvas color
		// whenever possible. It accelerates the paint algorithms, and
		// it helps the view to choose a better supreme viewed panel.
		// Arguments:
		//   layoutX, layoutX          - Position of this panel (upper-
		//                               left corner) in the coordinate
		//                               system of the parent panel. But
		//                               for the root panel, this is
		//                               ignored.
		//   layoutWidth, layoutHeight - Size of this panel in the
		//                               coordinate system of the parent
		//                               panel. But for the root panel,
		//                               only the quotient is of
		//                               interest.
		//   canvasColor               - Color of the canvas when this
		//                               panel is painted. Please study
		//                               the class emPainter for
		//                               understanding this argument.

	double GetLayoutX() const;
	double GetLayoutY() const;
	double GetLayoutWidth() const;
	double GetLayoutHeight() const;
		// Get the upper-left corner and size of this panel, in the
		// coordinate system of the parent panel.

	emColor GetCanvasColor() const;
		// Get the canvas color, non-opaque if unspecified.

	double GetWidth() const; // Always 1.0
	double GetHeight() const;
		// Get the size of this panel in its own coordinate system. The
		// width is always 1.0, and therefore the height is equal to
		// LayoutHeight/LayoutWidth.

	double GetTallness() const;
		// Get the height/width ratio of this panel. This is equal to
		// GetHeight() here. In classic computer graphics, the
		// reciprocal value is called "aspect ratio". But with emPanel
		// and emView it seems to be more practical working with
		// height/width ratios instead of width/height ratios. Let's
		// call it "tallness".

	virtual void GetSubstanceRect(double * pX, double * pY,
	                              double * pW, double * pH,
	                              double * pR) const;
		// Get the substance rectangle of this panel. This should
		// surround everything except empty margins and shadows. It is
		// used by emView to paint the focus rectangle, and it is used
		// to filter mouse and touch events (the panel will not get such
		// events outside of the rectangle). The returned rectangle is
		// in the coordinate system of this panel. It can have round
		// corners (*pR is set to the radius). The default
		// implementation returns the whole panel rectangle with zero
		// radius.

	bool IsPointInSubstanceRect(double x, double y) const;
		// Ask if a point lies within the substance rectangle. The point
		// must be given in the coordinate system of this panel.

	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH) const;
		// Get the essence rectangle of this panel. When the panel is to
		// be shown full-sized in the view, this rectangular part of the
		// panel is actually shown full-sized. The returned rectangle is
		// in the coordinate system of this panel. The default
		// implementation calls GetSubstanceRect(...).

	bool IsViewed() const;
	bool IsInViewedPath() const;
		// A panel is viewed if it is painted to the view. And a panel
		// is in viewed path if itself or a descendant is viewed. There
		// is always exactly one viewed panel within the whole tree of
		// panels, whose parent is not viewed (or which does not have a
		// parent). It is called the supreme viewed panel. The viewed
		// panels are making up a tree.

	double GetViewedPixelTallness() const;
		// Same as GetView().GetCurrentPixelTallness()

	double GetViewedX() const;
	double GetViewedY() const;
	double GetViewedWidth() const;
	double GetViewedHeight() const;
		// Get the upper-left corner and size of this panel, in the
		// coordinate system of the view (which should be the coordinate
		// system of the screen, measured in pixels). Note the equation:
		// ViewedHeight / ViewedWidth * ViewedPixelTallness == Height
		// CAUTION: These methods are valid only if IsViewed()==true.

	double GetClipX1() const;
	double GetClipY1() const;
	double GetClipX2() const;
	double GetClipY2() const;
		// Get the upper-left and lower-right corners of the clipping
		// rectangle, in the coordinate system of the view. This
		// clipping respects the ancestor panels and the view, bot not
		// any overlapping panels like descendants, sisters and aunts.
		// CAUTION: These methods are valid only if IsViewed()==true.

	double PanelToViewX(double panelX) const;
	double PanelToViewY(double panelY) const;
	double ViewToPanelX(double viewX) const;
	double ViewToPanelY(double viewY) const;
		// Transform panel coordinates to view coordinates and vice
		// versa.
		// CAUTION: These methods are valid only if IsViewed()==true.

	double PanelToViewDeltaX(double panelDeltaX) const;
	double PanelToViewDeltaY(double panelDeltaY) const;
	double ViewToPanelDeltaX(double viewDeltaX) const;
	double ViewToPanelDeltaY(double viewDeltaY) const;
		// Transform panel deltas (widths and heights) to view deltas
		// and vice versa.
		// CAUTION: These methods are valid only if IsViewed()==true.

	enum ViewConditionType {
		VCT_AREA,
		VCT_WIDTH,
		VCT_HEIGHT,
		VCT_MIN_EXT,
		VCT_MAX_EXT
	};
	double GetViewCondition(ViewConditionType vcType=VCT_AREA) const;
		// This can be used to decide whether the panel should show a
		// detail or not (through painting or through existence of a
		// child panel). The larger the panel is shown, the larger is
		// the result of the method. The result should be compared
		// against a threshold value, and if the threshold is less or
		// equal, the detail should be shown. In particular, this method
		// works as follows: If IsInViewedPath()==false, the result is
		// always zero (=> don't show any details). Otherwise, if
		// IsViewed()==false, the view has zoomed into a child panel and
		// therefore the result is very very large (=> keep details,
		// don't destroy child panels). Otherwise, the panel is a viewed
		// one and the result is calculated from the viewed size
		// depending on the argument vcType:
		//   VCT_AREA:    GetViewedWidth() * GetViewedHeight()
		//   VCT_WIDTH:   GetViewedWidth()
		//   VCT_HEIGHT:  GetViewedHeight()
		//   VCT_MIN_EXT: emMin(GetViewedWidth(), GetViewedHeight())
		//   VCT_MAX_EXT: emMax(GetViewedWidth(), GetViewedHeight())
		// vcType should be chosen so that the threshold value can be
		// independent from the height/width ratio of this panel.
		// Therefore, please consider, in the coordinate system of this
		// panel, how the size of the detail is depending on the height
		// (GetHeight()) of this panel. For example, if the detail has a
		// fixed layout without any dependency on GetHeight(), say
		// VCT_WIDTH. The default of VCT_AREA assumes that the layout
		// scales the detail in one extend by GetHeight(), which is
		// often the case.

	double GetAutoExpansionThresholdValue() const;
	ViewConditionType GetAutoExpansionThresholdType() const;
	virtual void SetAutoExpansionThreshold(
		double thresholdValue, ViewConditionType vcType=VCT_AREA
	);
		// Threshold value and type for the auto-expansion mechanism
		// (see AutoExpand()). The default should be okay for normal
		// cases.

	bool IsAutoExpanded() const;
		// Whether this panel is currently auto-expanded (see
		// AutoExpand()).

	virtual void SetEnableSwitch(bool enableSwitch);
	bool GetEnableSwitch() const;
	bool IsEnabled() const;
		// Set/get the enable switch and get the enable state. A panel
		// is enabled if itself and all its ancestor panels have the
		// enable switch set to true. Thereby it is possible to disable
		// a whole sub-tree by setting the enable switch of its root to
		// false. The enable state has no influence on the arguments to
		// the Input method. Programmers of derived panel classes should
		// care about the enable state in methods like Input and Paint
		// when it is appropriate.

	virtual void SetFocusable(bool focusable);
	bool IsFocusable() const;
		// Whether this panel can be focused. The default is true. The
		// root panel must always be focusable. Thus, it has no effect
		// to call SetFocusable(false) on the root panel.

	emPanel * GetFocusableParent() const;
	emPanel * GetFocusableFirstChild() const;
	emPanel * GetFocusableLastChild() const;
	emPanel * GetFocusablePrev() const;
	emPanel * GetFocusableNext() const;
		// Like GetParent, GetFirstChild and so on, but these methods
		// behave like if all non-focusable panels would have been
		// removed from the panel tree. At such a thought removal, the
		// children of a removed panel are given to the parent of the
		// removed panel. It is allowed to call the methods on
		// non-focusable panels. They are just additional possible start
		// points for the search.

	bool IsActive() const;
	bool IsInActivePath() const;
		// There is always exactly one panel, which is called the active
		// panel. The application behind the view usually shows the
		// title and the control panel for the active panel, and the
		// view normally paints a highlight around it. And it can be the
		// focused panel (see IsFocused()). A panel cannot be active if
		// it is not focusable (see SetFocusable()). A panel is in
		// active path if itself or any descendant is the active panel.

	bool IsActivatedAdherent() const;
		// Ask whether this panel is activated adherent. "Adherent"
		// usually means that the activation has been made by the user.
		// An adherent activation is not so easy to change by zooming
		// and scrolling like a non-adherent activation.

	void Activate(bool adherent=true);
		// Make this the active panel, or if this panel is not
		// focusable, make the nearest focusable ancestor active.

	bool IsFocused() const;
	bool IsInFocusedPath() const;
		// When the view is focused, the active panel is even the
		// focused panel. Otherwise there is no focused panel. A panel
		// is in focused path if itself or any descendant is the focused
		// panel.

	bool IsViewFocused() const;
		// Ask whether the view has the input focus.

	void Focus(bool adherent=true);
		// Make this the focused panel, or if this panel is not
		// focusable, make the nearest focusable ancestor focused. This
		// is like calling Activate() and GetView().Focus().

	double GetUpdatePriority() const;
		// Get the priority for updating this panel. For example, this
		// could be used when working with emPriSchedAgent. The result
		// is in the range of 0.0 (minimum priority) to 1.0 (maximum
		// priority).

	emUInt64 GetMemoryLimit() const;
		// Get the maximum number of memory bytes this panel is allowed
		// to allocate, including all descendant panels and all mostly
		// non-shared referred models, but not including any descendant
		// panels which are also supporting this blurred concept.
		// Mainly, this method has been invented for panels which are
		// showing file contents, because files or their models often
		// can have any size, and we have to make sure that the overall
		// process does not consume too much memory.

	emUInt64 GetInputClockMS() const;
		// Get the time of the currently handled input event and input
		// state, or the current time if all events are handled. The
		// time is measured in milliseconds and starts anywhere, but it
		// should never overflow.

	virtual double GetTouchEventPriority(double touchX, double touchY) const;
		// Get the priority of this panel for receiving touch events.
		// This is used by certain view input filters to decide whether
		// to eat touch events for their purpose. Remember the
		// possibility of an emSubViewPanel. Currently, following
		// priorities are defined:
		//  0.0 - No touch event processing.
		//  1.0 - Set focus by touches.
		//  2.0 - Emulate mouse functions by touches.
		//  3.0 - Emulate mouse functions, and zoom/scroll by touches.
		// The default implementation returns 0.0 when not focusable, or
		// 1.0 when focusable, according to the default implementation
		// of the Input method.
		// Arguments:
		//   touchX, touchY - Position of a first touch in view
		//                    coordinates.

	typedef int AutoplayHandlingFlags;
	enum {
		APH_ITEM               = (1<<0),
		APH_DIRECTORY          = (1<<1),
		APH_CUTOFF             = (1<<2),
		APH_CUTOFF_AT_SUBITEMS = (1<<3)
	};
	virtual void SetAutoplayHandling(AutoplayHandlingFlags flags);
	AutoplayHandlingFlags GetAutoplayHandling() const;
		// How this panel shall be handled by an autoplay function:
		//  APH_ITEM               - This panel is worth to be shown or
		//                           played by autoplay. This flag is set
		//                           by default, but it is ignored if
		//                           the panel is not focusable.
		//  APH_DIRECTORY          - Autoplay shall enter or leave this
		//                           sub-tree only when acting recursively.
		//                           This flag is ignored if the panel is
		//                           not focusable.
		//  APH_CUTOFF             - Autoplay shall never enter or leave
		//                           this sub-tree. If APH_ITEM is also
		//                           set, the panel may be shown as part
		//                           of the upper tree.
		//  APH_CUTOFF_AT_SUBITEMS - Force any sub-items to act like
		//                           APH_CUTOFF.

	virtual bool IsContentReady(bool * pReadying=NULL) const;
		// Ask if this panel has created all its child panels and if it
		// is ready for showing or playback. If not, *pReadying is set
		// to whether the panel will be ready later. Otherwise the panel
		// may not be ready because it is not viewed completely or large
		// enough. This method may be polled by autoplay. It should only
		// be called through a low-priority engine. The default
		// implementation returns IsAutoExpanded() and readying false.

	virtual bool GetPlaybackState(bool * pPlaying, double * pPos=NULL) const;
		// Get the playback state of this panel.
		// Arguments:
		//   pPlaying - Pointer for returning whether playpack is active.
		//   pPos     - Optional pointer for returning the playback
		//              position in the range of 0.0 to 1.0.
		//              If *pPlaying is set to false, *pPos==0.0 means
		//              stopped by user or never started, and *Pos==1.0
		//              means stopped by playing to the end, and
		//              0.0<*pPos<1.0 means paused by user at that
		//              position.
		// Returns:
		//   Whether this panel supports playback. The default
		//   implementation returns false.

	virtual bool SetPlaybackState(bool playing, double pos=-1.0);
		// Set the playback state of this panel.
		// Arguments:
		//   playing - Whether playpack shall be active or not.
		//   pos     - Desired playback position in the range of 0.0 to
		//             1.0. -1.0 means not to change the position.
		// Returns:
		//   Whether this panel supports playback. The default
		//   implementation returns false.

protected:

	virtual bool Cycle();
		// emPanel has been derived from emEngine for convenience. This
		// default implementation does nothing and returns false.

	typedef emUInt16 NoticeFlags;
	enum {
		NF_CHILD_LIST_CHANGED       = (1<<0),
		NF_LAYOUT_CHANGED           = (1<<1),
		NF_VIEWING_CHANGED          = (1<<2),
		NF_ENABLE_CHANGED           = (1<<3),
		NF_ACTIVE_CHANGED           = (1<<4),
		NF_FOCUS_CHANGED            = (1<<5),
		NF_VIEW_FOCUS_CHANGED       = (1<<6),
		NF_UPDATE_PRIORITY_CHANGED  = (1<<7),
		NF_MEMORY_LIMIT_CHANGED     = (1<<8),
		NF_SOUGHT_NAME_CHANGED      = (1<<9)
	};
	virtual void Notice(NoticeFlags flags);
		// Called some time after this panel has possibly changed in
		// some states. Each flag in the argument indicates a certain
		// state which may have changed:
		//   NF_CHILD_LIST_CHANGED      - List of children
		//   NF_LAYOUT_CHANGED          - GetLayout...(), GetHeight(),
		//                                GetCanvasColor()
		//   NF_VIEWING_CHANGED         - IsViewed(), IsInViewedPath(),
		//                                GetViewed...(), GetClip...(),
		//                                GetViewCondition(...)
		//   NF_ENABLE_CHANGED          - IsEnabled()
		//   NF_ACTIVE_CHANGED          - IsActive(), IsInActivePath()
		//   NF_FOCUS_CHANGED           - IsFocused(), IsInFocusedPath()
		//   NF_VIEW_FOCUS_CHANGED      - IsViewFocused()
		//   NF_UPDATE_PRIORITY_CHANGED - GetUpdatePriority()
		//   NF_MEMORY_LIMIT_CHANGED    - GetMemoryLimit()
		//   NF_SOUGHT_NAME_CHANGED     - GetSoughtName()
		// The default implementation does nothing.

	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
		// Process input form keyboard, mouse, and touch. This method is
		// called on every panel whenever there is a change in the input
		// state or when there is an input event. The order of callings
		// is from children to parents and from top to bottom (=last to
		// first). The default implementation does this: First, if it is
		// a mouse or touch event and if this panel is focusable, the
		// focus is set to this panel and the event is eaten. And
		// secondly, if this is the active panel, certain keyboard
		// events are processed and eaten for switching the focus to
		// another panel. Also see the methods GetInputClockMS and
		// GetTouchEventPriority.
		// Arguments:
		//   event  - An input event. This is non-empty only if:
		//            * It is a mouse button event, and the mouse
		//              position lies within the substance rectangles of
		//              the panel and of all of its ancestors, and the
		//              event has not been eaten by a descendant panel
		//              or by an overlapping panel in front or by a view
		//              input filter.
		//            * It is a touch event, and the first touch
		//              position lies within the substance rectangles of
		//              the panel and of all of its ancestors, and the
		//              event has not been eaten by a descendant panel
		//              or by an overlapping panel in front or by a view
		//              input filter. Normally, touch events are
		//              converted to mouse events by a view input
		//              filter.
		//            * It is a keyboard key event, and this panel is in
		//              focused path, and the event has not been eaten
		//              by a descendant panel or by a view input filter.
		//            The event can be eaten by calling event.Eat(). The
		//            event reference is non-const only for that. Please
		//            do not modify the event in any other way.
		//   state  - The current input state. The values of
		//            state.GetMouseX/Y are from the coordinate system
		//            of the view (thus, they are in pixels).
		//   mx, my - Position of the mouse in the coordinate system of
		//            this panel. Valid only if IsInViewedPath().

	virtual emCursor GetCursor() const;
		// Get the mouse cursor to be shown for this panel. The default
		// implementation asks the parent panel. See also:
		// InvalidateCursor()

	virtual bool IsOpaque() const;
		// Whether this panel is completely opaque by its painting or by
		// its child panels. If true, the background may not be
		// initialized when the panel is painted. It even helps the view
		// to choose a better supreme viewed panel. The default
		// implementation returns false. See also: InvalidatePainting()

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;
		// Paint this panel. The default implementation does nothing.
		// Note that for a single painting of the whole panel, this
		// method may be called multiple times with different clipping
		// rectangles in order to optimize cache usage.
		//
		// ***************** CAUTION: MULTI-THREADING ******************
		// * In order to improve the graphics performance on           *
		// * multi-core CPUs, Paint may be called by multiple threads  *
		// * in parallel, but in a way that there is always at most    *
		// * one thread at a time in user code, outside any call to    *
		// * emPainter. There is a shared mutex which is entered       *
		// * before a thread calls an emPanel::Paint. It is left only  *
		// * while the thread is in a call to a method of emPainter    *
		// * and re-entered before such a call returns.                *
		// * Please prepare your code for this. Ideally, do not modify *
		// * any shared data in an implementation of Paint.            *
		// *************************************************************
		//
		// Arguments:
		//   painter     - A painter for painting the panel to the
		//                 screen. Origin and scaling of this painter
		//                 are prepared for having the coordinate system
		//                 of the panel.
		//   canvasColor - Please study emPainter for understanding this
		//                 parameter. Normally this is equal to
		//                 GetCanvasColor(), but if this panel is the
		//                 supreme viewed panel, it could be a different
		//                 color.
		// See also: InvalidatePainting()

	virtual void AutoExpand();
		// Create child panels by auto-expansion. Often it is a good
		// idea to dynamically create and delete the children of a panel
		// depending on the view condition, instead of wasting resources
		// by having the child panels always existent. For solving
		// dynamic creation and deletion, AutoExpand could be overloaded
		// to create the child panels in it. The default implementation
		// does nothing. AutoExpand is called when the view condition
		// reaches a threshold value. As soon as the view condition
		// falls below again, the child panels are deleted through
		// calling AutoShrink(). See also:
		// SetAutoExpansionThreshold(...), AutoShrink(),
		// InvalidateAutoExpansion().

	virtual void AutoShrink();
		// Delete child panels by auto-shrinking. The default
		// implementation deletes exactly those child panels that have
		// been created within a call to AutoExpand (yes, there is some
		// internal magic for knowing whether a panel has bee created
		// inside or outside AutoExpand). So there is no need to
		// overload this method except if you want to do something like
		// setting panel pointer variables to NULL.

	virtual void LayoutChildren();
		// Lay out the child panels of this panel. The default
		// implementation does nothing. It is not a must to do the
		// layout herein, but it is a good idea. The method is called
		// after there was a change in the list of child panels (like
		// NF_CHILD_LIST_CHANGED) or in the layout of this panel (like
		// NF_LAYOUT_CHANGED), but only if there is at least one child
		// panel. See also: InvalidateChildrenLayout()

	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);
		// Create a control panel for this content panel. If this panel
		// is in a content view, and if it is the active panel, then
		// this method may be called by the view to create a control
		// panel. The default implementation asks the parent of this
		// panel. A result of NULL means to have no control panel.
		// Remember that the control panel is normally created in
		// another view and therefore in another view context than this
		// content panel. But it is okay to have links (pointers,
		// references) from a control panel to the content view context
		// or to its models. That means, per definition, the content
		// view context always has to live longer than the panels of the
		// control view. Hint: If a panel does not want to leave its
		// control panel to its descendants, it could check IsActive()
		// to see whether the control panel would be for itself or for a
		// descendant. See also: InvalidateControlPanel()

	const char * GetSoughtName() const;
		// While the view is seeking for a child of this panel, this
		// method returns the name of the sought child panel. Otherwise
		// it returns NULL.

	virtual bool IsHopeForSeeking() const;
		// While the view is seeking for a still non-existent child of
		// this panel, this method is called by the view on every time
		// slice for asking whether there is any hope that the desired
		// child panel will ever be created. If this returns false
		// continuously for at least 10 time slices, the seeking is
		// given up. Thus, the implementation can temporarily say false
		// even if there is hope. This often simplifies the
		// implementation regarding pending notices and signals. The
		// default implementation always returns false. Hint: While the
		// view is seeking for a child panel, the parent panel is shown
		// full-sized and its memory limit is set to a maximum. Thereby
		// it is often not needed to care about the seek problem when
		// programming a panel.

	void InvalidateTitle();
		// Indicate a change in the results of GetTitle(). After calling
		// this, showings of the title will be updated.

	void InvalidateCursor();
		// Indicate a change in the results of GetCursor(). After
		// calling this, showings of the cursor will be updated.

	void InvalidatePainting();
	void InvalidatePainting(double x, double y, double w, double h);
		// Indicate a change in the results of IsOpaque() and Paint().
		// After calling this, the panel will be re-painted if it is
		// shown. The second version of the method allows to invalidate
		// just a rectangular area instead of the whole panel.
		// Arguments:
		//   x,y,w,h - Upper-left corner and size of the rectangle, in
		//             the coordinate system of this panel.

	void InvalidateAutoExpansion();
		// Indicate a change in the results of AutoExpand(). After
		// calling this, and if in expanded state, AutoShrink() and
		// AutoExpand() will be called again.

	void InvalidateChildrenLayout();
		// Indicate a change in the results of LayoutChildren(). After
		// calling this, LayoutChildren() will be called again, but only
		// if there is at least one child.

	void InvalidateControlPanel();
		// Indicate a change in the results of CreateControlPanel().
		// After calling this, the control panel will be re-created, but
		// only if it is shown.

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding
	// "const". Besides, please read the new comments about multi-threading
	// in Paint more above.
public:
	virtual emString GetTitle() final;
	virtual emString GetIconFileName() final;
	virtual void GetSubstanceRect(double * pX, double * pY,
	                              double * pW, double * pH,
	                              double * pR) final;
	virtual void GetEssenceRect(double * pX, double * pY,
	                            double * pW, double * pH) final;
	virtual double GetTouchEventPriority(double touchX, double touchY) final;
protected:
	virtual emCursor GetCursor() final;
	virtual bool IsOpaque() final;
	virtual void Paint(const emPainter & painter, emColor canvasColor) final;
	virtual bool IsHopeForSeeking() final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	friend class emView;
	friend class ParentArgClass;

	void AddPendingNotice(NoticeFlags flags);
	void HandleNotice();
	void UpdateChildrenViewing();
	void AvlInsertChild(emPanel * child);
	void AvlRemoveChild(emPanel * child);

	emView & View;
	emCrossPtrList CrossPtrList;
	emString Name;
	emAvlNode AvlNode;
	emAvlTree AvlTree;
	emPanel * Parent;
	emPanel * FirstChild;
	emPanel * LastChild;
	emPanel * Prev;
	emPanel * Next;
	emView::PanelRingNode NoticeNode;
	double LayoutX, LayoutY, LayoutWidth, LayoutHeight;
	double ViewedX, ViewedY, ViewedWidth, ViewedHeight;
	double ClipX1, ClipY1, ClipX2, ClipY2;
	double AEThresholdValue;
	emColor CanvasColor;
	NoticeFlags PendingNoticeFlags;
	unsigned Viewed : 1;
	unsigned InViewedPath : 1;
	unsigned EnableSwitch : 1;
	unsigned Enabled : 1;
	unsigned Focusable : 1;
	unsigned Active : 1;
	unsigned InActivePath : 1;
	unsigned PendingInput : 1;
	unsigned ChildrenLayoutInvalid : 1;
	unsigned AEInvalid : 1;
	unsigned AEDecisionInvalid : 1;
	unsigned AECalling : 1;
	unsigned AEExpanded : 1;
	unsigned CreatedByAE : 1;
	unsigned AEThresholdType : 3;
	unsigned AutoplayHandling : 4;
};

inline emPanel::ParentArgClass::ParentArgClass(emPanel & panel)
	: View(&panel.View),Panel(&panel)
{
}

inline emPanel::ParentArgClass::ParentArgClass(emPanel * panel)
	: View(&panel->View),Panel(panel)
{
}

inline emPanel::ParentArgClass::ParentArgClass(emView & view)
	: View(&view),Panel(NULL)
{
}

inline emPanel::ParentArgClass::ParentArgClass(emView * view)
	: View(view),Panel(NULL)
{
}

inline emRootContext & emPanel::ParentArgClass::GetRootContext() const
{
	return View->GetRootContext();
}

inline emView & emPanel::ParentArgClass::GetView() const
{
	return *View;
}

inline emPanel * emPanel::ParentArgClass::GetPanel() const
{
	return Panel;
}

inline void emPanel::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline const emString & emPanel::GetName() const
{
	return Name;
}

inline emView & emPanel::GetView() const
{
	return View;
}

inline emContext & emPanel::GetViewContext() const
{
	return View;
}

inline emRootContext & emPanel::GetRootContext() const
{
	return View.GetRootContext();
}

inline emWindow * emPanel::GetWindow() const
{
	return View.GetWindow();
}

inline emScreen * emPanel::GetScreen() const
{
	return View.GetScreen();
}

inline emPanel * emPanel::GetParent() const
{
	return Parent;
}

inline emPanel * emPanel::GetFirstChild() const
{
	return FirstChild;
}

inline emPanel * emPanel::GetLastChild() const
{
	return LastChild;
}

inline emPanel * emPanel::GetPrev() const
{
	return Prev;
}

inline emPanel * emPanel::GetNext() const
{
	return Next;
}

inline double emPanel::GetLayoutX() const
{
	return LayoutX;
}

inline double emPanel::GetLayoutY() const
{
	return LayoutY;
}

inline double emPanel::GetLayoutWidth() const
{
	return LayoutWidth;
}

inline double emPanel::GetLayoutHeight() const
{
	return LayoutHeight;
}

inline emColor emPanel::GetCanvasColor() const
{
	return CanvasColor;
}

inline double emPanel::GetWidth() const
{
	return 1.0;
}

inline double emPanel::GetHeight() const
{
	return LayoutHeight/LayoutWidth;
}

inline double emPanel::GetTallness() const
{
	return LayoutHeight/LayoutWidth;
}

inline bool emPanel::IsViewed() const
{
	return Viewed;
}

inline bool emPanel::IsInViewedPath() const
{
	return InViewedPath;
}

inline double emPanel::GetViewedPixelTallness() const
{
	return View.CurrentPixelTallness;
}

inline double emPanel::GetViewedX() const
{
	return ViewedX;
}

inline double emPanel::GetViewedY() const
{
	return ViewedY;
}

inline double emPanel::GetViewedWidth() const
{
	return ViewedWidth;
}

inline double emPanel::GetViewedHeight() const
{
	return ViewedHeight;
}

inline double emPanel::GetClipX1() const
{
	return ClipX1;
}

inline double emPanel::GetClipY1() const
{
	return ClipY1;
}

inline double emPanel::GetClipX2() const
{
	return ClipX2;
}

inline double emPanel::GetClipY2() const
{
	return ClipY2;
}

inline double emPanel::PanelToViewX(double panelX) const
{
	return panelX*ViewedWidth+ViewedX;
}

inline double emPanel::PanelToViewY(double panelY) const
{
	return panelY*ViewedWidth/View.CurrentPixelTallness+ViewedY;
}

inline double emPanel::ViewToPanelX(double viewX) const
{
	return (viewX-ViewedX)/ViewedWidth;
}

inline double emPanel::ViewToPanelY(double viewY) const
{
	return (viewY-ViewedY)*View.CurrentPixelTallness/ViewedWidth;
}

inline double emPanel::PanelToViewDeltaX(double panelDeltaX) const
{
	return panelDeltaX*ViewedWidth;
}

inline double emPanel::PanelToViewDeltaY(double panelDeltaY) const
{
	return panelDeltaY*ViewedWidth/View.CurrentPixelTallness;
}

inline double emPanel::ViewToPanelDeltaX(double viewDeltaX) const
{
	return viewDeltaX/ViewedWidth;
}

inline double emPanel::ViewToPanelDeltaY(double viewDeltaY) const
{
	return viewDeltaY*View.CurrentPixelTallness/ViewedWidth;
}

inline double emPanel::GetAutoExpansionThresholdValue() const
{
	return AEThresholdValue;
}

inline emPanel::ViewConditionType emPanel::GetAutoExpansionThresholdType() const
{
	return (ViewConditionType)AEThresholdType;
}

inline bool emPanel::IsAutoExpanded() const
{
	return AEExpanded;
}

inline bool emPanel::GetEnableSwitch() const
{
	return EnableSwitch;
}

inline bool emPanel::IsEnabled() const
{
	return Enabled;
}

inline bool emPanel::IsFocusable() const
{
	return Focusable;
}

inline bool emPanel::IsActive() const
{
	return Active;
}

inline bool emPanel::IsInActivePath() const
{
	return InActivePath;
}

inline bool emPanel::IsActivatedAdherent() const
{
	return Active && View.IsActivationAdherent();
}

inline bool emPanel::IsFocused() const
{
	return Active && View.IsFocused();
}

inline bool emPanel::IsInFocusedPath() const
{
	return InActivePath && View.IsFocused();
}

inline bool emPanel::IsViewFocused() const
{
	return View.IsFocused();
}

inline emUInt64 emPanel::GetInputClockMS() const
{
	return View.GetInputClockMS();
}

inline emPanel::AutoplayHandlingFlags emPanel::GetAutoplayHandling() const
{
	return AutoplayHandling;
}

inline void emPanel::InvalidateChildrenLayout()
{
	ChildrenLayoutInvalid=1;
	if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
}

inline void emPanel::AddPendingNotice(NoticeFlags flags)
{
	PendingNoticeFlags|=flags;
	if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
}


#endif
