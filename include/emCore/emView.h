//------------------------------------------------------------------------------
// emView.h
//
// Copyright (C) 2004-2012,2014,2016-2017 Oliver Hamann.
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

#ifndef emView_h
#define emView_h

#ifndef emCursor_h
#include <emCore/emCursor.h>
#endif

#ifndef emInput_h
#include <emCore/emInput.h>
#endif

#ifndef emPainter_h
#include <emCore/emPainter.h>
#endif

#ifndef emCoreConfig_h
#include <emCore/emCoreConfig.h>
#endif

class emPanel;
class emViewPort;
class emWindow;
class emScreen;
class emViewAnimator;
class emMagneticViewAnimator;
class emVisitingViewAnimator;
class emViewInputFilter;
class emCheatVIF;


//==============================================================================
//=================================== emView ===================================
//==============================================================================

class emView : public emContext {

public:

	// An emView is a rectangle on the screen in which the user can see a
	// tree of panels (see class emPanel). If not disabled by a feature
	// flag, the user can navigate through the panels by zooming, scrolling
	// and moving the focus.
	//
	// After constructing an emView, an appropriate emViewPort should be
	// created on it. Otherwise the view is not connected to any screen or
	// input device (it uses an internal dummy emViewPort then). See the
	// derived class emWindow - it solves the problem of connecting the
	// view. Another solution is the class emSubViewPanel.
	//
	// Even, emView does not create any panels. This should also be done by
	// the caller.
	//
	// emView has been derived from emContext so that it is easy to define
	// custom view settings as models of that context.

	typedef int ViewFlags;
		// Data type for the feature flags of a view. Possible flags
		// are:
	enum {
		VF_POPUP_ZOOM         =(1<<0),
			// The view pops up whenever the view is zoomed. For
			// this feature to work, an emScreen must be in the path
			// of contexts, so that the view is able to create a
			// private emWindow as the popup.
		VF_ROOT_SAME_TALLNESS =(1<<1),
			// The root panel always has the same tallness as the
			// view. This is highly recommended when using
			// VF_POPUP_ZOOM.
		VF_NO_ZOOM            =(1<<2),
			// The view is always zoomed out. This implies having
			// VF_NO_USER_NAVIGATION and not having VF_POPUP_ZOOM
			// and not having VF_EGO_MODE.
		VF_NO_USER_NAVIGATION =(1<<3),
			// The user cannot navigate in the view.
		VF_NO_FOCUS_HIGHLIGHT =(1<<4),
			// The view does not show any focus highlight.
		VF_NO_ACTIVE_HIGHLIGHT=(1<<5),
			// The view does not show any active highlight.
		VF_EGO_MODE           =(1<<6),
			// This is a special mode of user interaction. The mouse
			// pointer is degenerated to a cross-hair in the center
			// of the view, and each mouse movement scrolls the
			// view. This mode should not be used when there are
			// multiple windows or views on the screen, because the
			// mouse pointer cannot be moved. The name "ego mode"
			// has been chosen because it is somehow similar to the
			// mouse control in ego shooter games.
		VF_STRESS_TEST        =(1<<7)
			// This repaints the view on every time slice and shows
			// the frame rate in the upper-left corner. This feature
			// exists for use by developers.
	};

	emView(emContext & parentContext, ViewFlags viewFlags=0);
		// Constructor.
		// Arguments:
		//   parentContext - Parent context for this new context.
		//   viewFlags     - Initial feature flags.

	virtual ~emView();
		// Destructor. If a real view port has been created for this
		// view, it must be deleted before destructing the view. In
		// addition, it is always a good idea to delete the root panel
		// by destructors of derived classes, just because a panel could
		// depend on the properties of the derivative. For example,
		// emPanel::GetWindow() must not be used after destructing the
		// emWindow, and therefore the destructor of emWindow deletes
		// the panels itself.

	void LinkCrossPtr(emCrossPtrPrivate & crossPtr);
		// This means emCrossPtr<emView> is possible.

	ViewFlags GetViewFlags() const;
	void SetViewFlags(ViewFlags viewFlags);
		// Get or set the features of this view.

	const emSignal & GetViewFlagsSignal() const;
		// This signal is signaled when the features of this view have
		// changed.

	emColor GetBackgroundColor() const;
	void SetBackgroundColor(emColor c);
		// Get or set the background color of this view. This color is
		// used for areas which are not covered by the panels, or where
		// the panels are transparent. The default is some grey.

	virtual emString GetTitle() const;
		// Get the title to be shown for this view. The default
		// implementation returns the title of the active panel (not
		// directly, but updated through an engine). See also:
		// InvalidateTitle()

	const emSignal & GetTitleSignal() const;
		// This signal is signaled when the title has changed.

	emPanel * CreateControlPanel(emPanel & parent, const emString & name);
		// If this is a content view, this method may be called to
		// create a control panel in a control view. The call is
		// forwarded to the active panel. The result can be NULL which
		// means to have no control panel. The caller should delete the
		// control panel before calling CreateControlPanel again, and
		// before destructing this view.

	const emSignal & GetControlPanelSignal() const;
		// This signal is signaled when the control panel should be
		// recreated.

	bool IsFocused() const;
		// Whether this view has the focus.

	const emSignal & GetFocusSignal() const;
		// This signal is signaled when the focus of this view has
		// changed.

	void Focus();
		// Make that this view has the focus, if possible by the
		// emViewPort implementation. Hint: If this is an emWindow, you
		// may want to call Raise() before.

	emWindow * GetWindow() const;
		// Get the window of this view. It returns the nearest window
		// within the path of contexts to this view. If this view itself
		// is a window, that window is returned. If no window can be
		// found, NULL is returned. The result is cached internally.

	emScreen * GetScreen() const;
		// Get the screen of this view. If GetWindow() returns non-NULL,
		// the screen of that window is returned. Otherwise the call is
		// forwarded to emScreen::LookupInherited(*this). The result is
		// cached internally.

	double GetHomeX() const;
	double GetHomeY() const;
	double GetHomeWidth() const;
	double GetHomeHeight() const;
	double GetHomePixelTallness() const;
	double GetHomeTallness() const;
		// Get the home geometry of this view on the screen. "Home"
		// means the state where the view is not popped up by the
		// popup-zoom feature. X, Y, Width and Height are the
		// coordinates of the view rectangle, measured in pixels on the
		// screen. PixelTallness is the height/width ratio of the pixels
		// on the monitor (as far as known by the implementation of
		// emViewPort). And Tallness is the height/width ration of the
		// view on the monitor. It is equal to
		// Height/Width*PixelTallness.

	double GetCurrentX() const;
	double GetCurrentY() const;
	double GetCurrentWidth() const;
	double GetCurrentHeight() const;
	double GetCurrentPixelTallness() const;
	double GetCurrentTallness() const;
		// Get the current geometry of this view on the screen. This is
		// like GetHomeX(), GetHomeY() and so on, but if the view is
		// currently popped up by the popup-zoom feature, the geometry
		// of the popup is returned (should not make any difference for
		// PixelTallness).

	const emSignal & GetGeometrySignal() const;
		// This signal is signaled when the home geometry or the current
		// geometry of this view has changed.

	void GetMaxPopupViewRect(double * pX, double * pY,
	                         double * pW, double * pH) const;
		// Get the pixel coordinates of the maximum visible rectangle of
		// a popup. This is the bounding rectangle of all display
		// monitors, which intersect the home rectangle of the view. The
		// given pointers are for returning the rectangle coordinates
		// (x,y,w,h). NULL pointers are allowed.

	emViewInputFilter * GetFirstVIF() const;
	emViewInputFilter * GetLastVIF() const;
		// Get the first or last view input filter (VIF) in the chain of
		// the VIFs of this view. The default chain (after constructing
		// the view) contains emDefaultTouchVIF, emCheatVIF,
		// emKeyboardZoomScrollVIF, and emMouseZoomScrollVIF in that
		// order. This could change in future versions. You can modify
		// the chain simply by deleting and creating VIFs as you like.
		// See emViewInputFilter for more details. Note that the
		// zoom/scroll VIFs have no effect when VF_NO_USER_NAVIGATION is
		// set in the view flags. Also, never forget about the
		// possibility of parent and child views (=>emSubViewPanel).
		// Parent views should always have VF_NO_ZOOM.

	emPanel * GetRootPanel() const;
		// Get the root panel of this view. Returns NULL when this view
		// has no panels.

	emPanel * GetSupremeViewedPanel() const;
		// Get the supreme viewed panel. It is the upper-most panel in
		// the panel tree which has IsViewed()==true. Returns NULL when
		// this view has no panels. The supreme viewed panel is always
		// chosen automatically by the view, depending on the visit
		// state.

	emPanel * GetActivePanel() const;
		// Get the active panel. The active panel is the panel which is
		// focused when the view is focused. Returns NULL when this view
		// has no panels.

	bool IsActivationAdherent() const;
		// Whether the panel activation is adherent. It usually means
		// that the activation has been made by the user.

	void SetActivePanel(emPanel * panel, bool adherent=true);
		// Make the given Panel the active panel, or if it is not
		// focusable, make the nearest focusable ancestor active.
		// The location of the view is not changed.

	void SetActivePanelBestPossible();
		// Search for a viewed panel whose position in the view is best
		// for being the active panel, and activate it. But if the old
		// activation is adherent, the activation is changed only if it
		// is really too bad.

	emPanel * GetPanelByIdentity(const char * identity) const;
		// Search for a panel by identity (see emPanel::GetIdentity()).
		// Returns NULL if not found.

	emPanel * GetPanelAt(double x, double y) const;
		// Get the uppermost panel at the given point.

	emPanel * GetFocusablePanelAt(double x, double y,
	                              bool checkSubstance) const;
		// Get the uppermost focusable panel at the given point. If
		// checkSubstance is true, the substance rectangles of the
		// panels are checked. Otherwise the whole panel rectangles are
		// checked.

	emPanel * GetVisitedPanel(double * pRelX=NULL, double * pRelY=NULL,
	                          double * pRelA=NULL) const;
		// Get the visited panel and optionally relative coordinates.
		// The visited panel is the one on which the view is currently
		// anchored. That means, if any panel layout is changed, the
		// visited panel keeps its position and size in the view.
		// Normally, the visited panel is the active panel. But if the
		// active panel is not viewed, the visited panel is the viewed
		// panel which is (in the tree of panels) nearest to the active
		// panel.
		// Arguments:
		//   pRelX, pRelY - Pointers for returning the distance vector
		//                  between the center of the view and the
		//                  center of the panel, measured in view widths
		//                  and heights.
		//   pRelA        - Pointer for returning the area size of the
		//                  view relative to the area size of the panel.
		// Returns:
		//   The visited panel, or NULL if this view has no panels.

	void Visit(emPanel * panel, double relX, double relY, double relA,
	           bool adherent);
	void Visit(const char * identity, double relX, double relY, double relA,
	           bool adherent, const char * subject=NULL);
		// Start to visit a panel. This means to position the view
		// relative to the given panel and make that panel the active
		// panel. The operation is animated and may take a long time.
		// Arguments:
		//   panel     - The panel to be visited.
		//   identity  - Identity of the panel to be visited.
		//   relX,relY - Desired distance vector between the center of
		//               the view and the center of the panel, measured
		//               in view widths and heights.
		//   relA      - Desired area size of the view relative to the
		//               area size of the panel.
		//   adherent  - Whether the activation shall be adherent.
		//   subject   - A subject text to be shown in an info box
		//               for the case the operation takes much time.
		//               Best is to give the title of the panel.

	void Visit(emPanel * panel, bool adherent);
	void Visit(const char * identity, bool adherent,
	           const char * subject=NULL);
		// Like the Visit methods above, but automatically
		// choose a good position for the panel in the view.

	void VisitFullsized(emPanel * panel, bool adherent,
	                    bool utilizeView=false);
	void VisitFullsized(const char * identity, bool adherent,
	                    bool utilizeView=false, const char * subject=NULL);
		// Like the Visit methods above, but position the view so that
		// the panel is shown full-sized.

	void RawVisit(emPanel * panel, double relX, double relY, double relA);
	void RawVisit(emPanel * panel);
	void RawVisitFullsized(emPanel * panel, bool utilizeView=false);
		// Like the Visit methods above, but without animation (these
		// methods act immediately), without aborting any active
		// animation, and without changing the active panel.

	void VisitNext();
	void VisitPrev();
	void VisitFirst();
	void VisitLast();
	void VisitLeft();
	void VisitRight();
	void VisitUp();
	void VisitDown();
	void VisitNeighbour(int direction);
	void VisitIn();
	void VisitOut();
		// Start to visit a sister, parent or child of the active panel.
		// This walks the tree of focusable panels only.
		// VisitNeighbour(0) is like VisitRight(), 1 like Down, 2 like
		// Left and 3 like Up.

	void Scroll(double deltaX, double deltaY);
		// Scroll the view. This aborts any active animation and
		// performs immediately without animation. The active panel is
		// adapted.
		// Arguments:
		//   deltaX     - How many pixels to scroll in X direction.
		//                A positive value means to scroll right.
		//   deltaY     - How many pixels to scroll in Y direction.
		//                A positive value means to scroll down.

	void Zoom(double fixX, double fixY, double factor);
		// Zoom the view. This aborts any active animation and performs
		// immediately without animation. The active panel is adapted.
		// Arguments:
		//   fixX, fixY - Fix point for the zooming.
		//   factor     - Zoom factor. A value less than 1.0 means to
		//                zoom out, and a value greater than 1.0 means
		//                to zoom in.

	void RawScrollAndZoom(
		double fixX, double fixY,
		double deltaX, double deltaY, double deltaZ,
		emPanel * panel=NULL, double * pDeltaXDone=NULL,
		double * pDeltaYDone=NULL, double * pDeltaZDone=NULL
	);
		// Scroll and zoom the view without aborting animations and
		// without changing the active panel. The scrolling is performed
		// before the zooming.
		// Arguments:
		//   fixX, fixY  - Fix point for the zooming.
		//   deltaX      - How many pixels to scroll in X direction.
		//                 A positive value means to scroll right.
		//   deltaY      - How many pixels to scroll in Y direction.
		//                 A positive value means to scroll down.
		//   deltaZ      - How many pixels to zoom. The zoom factor is:
		//                 exp(deltaZ * GetZoomFactorLogarithmPerPixel())
		//   panel       - A panel to be used for the calculations.
		//                 It should be a panel which is also viewed
		//                 after the operation. If NULL or not viewed,
		//                 the visited panel is used.
		//   pDeltaXDone - Pointer for returning the number of pixels by
		//                 which the view actually has been scrolled in
		//                 X.
		//   pDeltaYDone - Pointer for returning the number of pixels by
		//                 which the view actually has been scrolled in
		//                 Y.
		//   pDeltaZDone - Pointer for returning the number of pixels by
		//                 which the view actually has been zoomed.

	double GetZoomFactorLogarithmPerPixel() const;
		// How much zooming feels the same amount of motion to the user
		// as scrolling one pixel, returned as natural logarithm of zoom
		// factor.

	void ZoomOut();
		// Zoom out the view completely. This aborts any active
		// animation and performs immediately without animation. The
		// active panel is adapted.

	void RawZoomOut();
		// Like ZoomOut(), but without aborting animations and without
		// changing the active panel.

	bool IsZoomedOut() const;
		// Whether the view is currently zoomed out completely.

	bool IsPoppedUp() const;
		// Whether the view is in popped up state.

	void SignalEOIDelayed();
		// After calling this, the End-Of-Interaction signal will be
		// signaled, but with a delay of some time slices.

	const emSignal & GetEOISignal() const;
		// Get the End-Of-Interaction signal. This signal indicates an
		// end of a temporary user interaction. It has been invented for
		// emButton (see emButton::IsNoEOI()). If the view has
		// VF_POPUP_ZOOM set, the application should call ZoomOut() when
		// an EOI has been signaled.

	bool IsSoftKeyboardShown() const;
	void ShowSoftKeyboard(bool show);
		// Get and set whether to show a software keyboard (if there is
		// no hardware keyboard).

	emUInt64 GetInputClockMS() const;
		// Get the time of the currently handled input event and input
		// state, or the current time if all events are handled. The
		// time is measured in milliseconds and starts anywhere, but it
		// should never overflow.

	virtual double GetTouchEventPriority(
		double touchX, double touchY, bool afterVIFs=false
	) const;
		// Get the touch event priority of this view for a certain touch
		// position. (See emPanel::GetTouchEventPriority).
		// Arguments:
		//   touchX, touchY - Position of a first touch in view
		//                    coordinates.
		//   afterVIFs      - Whether to leave out the view input filters
		//                    in the calculation.

	emViewAnimator * GetActiveAnimator() const;
		// Get the active view animator, or NULL if none is active.

	void ActivateMagneticViewAnimator();
		// Activate the magnetic view animator.

	void AbortActiveAnimator();
		// Abort any activate view animator.

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);
		// Process input form keyboard, mouse, and touch after filtering
		// it by the view input filters. The default implementation
		// forwards to the panels.
		// Arguments:
		//   event  - An input event. It may be eaten by calling
		//            event.Eat(). The event reference in non-const only
		//            for that.
		//   state  - The current input state.

	virtual emCursor GetCursor() const;
		// Get the mouse cursor to be shown for this view. The default
		// implementation returns the cursor of the panel where the
		// mouse is. See also: InvalidateCursor()

	virtual void Paint(
		const emPainter & painter, emColor canvasColor
	) const;
		// Paint this view. The default implementation paints the
		// panels, the focus and an info box when seeking.
		// Arguments:
		//   painter     - A painter for painting the view to the
		//                 screen. Origin and scaling of this painter
		//                 are prepared for having the coordinate system
		//                 of the screen.
		//   canvasColor - Color of the canvas.
		// See also: InvalidatePainting()

	virtual void InvalidateTitle();
		// Indicate a change in the results of GetTitle(). The default
		// implementation simply signals the title signal (see
		// GetTitleSignal()).

	void InvalidateCursor();
		// Indicate a change in the results of GetCursor(). After
		// calling this, showings of the cursor will be updated.

	void InvalidatePainting();
	void InvalidatePainting(double x, double y, double w, double h);
		// Indicate a change in the results of Paint(). After calling
		// this, the view will be re-painted. The second version of the
		// method allows to invalidate just a rectangular area instead
		// of the whole view.
		// Arguments:
		//   x,y,w,h - Upper-left corner and size of the rectangle.

	virtual void DoCustomCheat(const char * func);
		// This method could be overloaded for implementing custom cheat
		// codes. For example, if the user enters "chEat:abc!", this
		// method is called with func="abc". The default implementation
		// calls the ancestor view, if there. For the cheat codes
		// implemented by default, see the implementation of
		// emCheatVIF::Input.

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
public:
	virtual emString GetTitle() final;
	virtual double GetTouchEventPriority(
		double touchX, double touchY, bool afterVIFs=false
	) final;
protected:
	virtual emCursor GetCursor() final;
	virtual void Paint(const emPainter & painter, emColor canvasColor) final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
	friend class emViewPort;
	friend class emViewAnimator;
	friend class emVisitingViewAnimator;
	friend class emViewInputFilter;
	friend class emCheatVIF;
	friend class emPanel;
	friend class emSubViewPanel;
	friend class emWindow;

	struct PanelRingNode {
		PanelRingNode * Prev;
		PanelRingNode * Next;
	};

	void SetWindowAndScreen(emWindow * window=NULL);

	void SetFocused(bool focused);
	void SetGeometry(double x, double y, double width, double height,
	                 double pixelTallness);

	void AddToNoticeList(PanelRingNode * node);

	void Update();

	void CalcVisitCoords(const emPanel * panel, double * pRelX,
	                     double * pRelY, double * pRelA) const;

	void CalcVisitFullsizedCoords(const emPanel * panel, double * pRelX,
	                              double * pRelY, double * pRelA,
	                              bool utilizeView=false) const;

	void RawVisit(emPanel * panel, double relX, double relY, double relA,
	              bool forceViewingUpdate);
	void RawVisitAbs(emPanel * panel, double vx, double vy, double vw,
	                 bool forceViewingUpdate);
	void RawZoomOut(bool forceViewingUpdate);

	void FindBestSVP(emPanel * * pPanel, double * pVx, double * pVy,
	                 double * pVw) const;
	bool FindBestSVPInTree(emPanel * * pPanel, double * pVx, double * pVy,
	                       double * pVw, bool covering) const;

	void SwapViewPorts(bool swapFocus);

	void RecurseInput(emInputEvent & event,
	                  const emInputState & state);
	void RecurseInput(emPanel * parent, emInputEvent & event,
	                  const emInputState & state);

	void InvalidateHighlight();
	void PaintHighlight(const emPainter & painter) const;
	void PaintHighlightArrowsOnLine(
		const emPainter & painter, double x, double y,
		double dx, double dy, double pos, double delta,
		int count, double goalX, double goalY, double arrowSize,
		emColor shadowColor, emColor arrowColor
	) const;
	void PaintHighlightArrowsOnBow(
		const emPainter & painter, double x, double y, double radius,
		int quadrant, double pos, double delta, int count,
		double goalX, double goalY, double arrowSize,
		emColor shadowColor, emColor arrowColor
	) const;
	void PaintHighlightArrow(
		const emPainter & painter, double x, double y,
		double goalX, double goalY, double arrowSize,
		emColor shadowColor, emColor arrowColor
	) const;

	void SetSeekPos(emPanel * panel, const char * childName);
	bool IsHopeForSeeking() const;

	class UpdateEngineClass : public emEngine {
	public:
		UpdateEngineClass(emView & view);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
	};
	friend class UpdateEngineClass;

	class EOIEngineClass : public emEngine {
	public:
		EOIEngineClass(emView & view);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
		int CountDown;
	};
	friend class EOIEngineClass;

	class StressTestClass : public emEngine {
	public:
		StressTestClass(emView & view);
		virtual ~StressTestClass();
		void PaintInfo(const emPainter & painter);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
		int TCnt,TPos,TValid;
		emUInt64 * T;
		double FrameRate;
		emUInt64 FRUpdate;
	};
	friend class StressTestClass;

	emCrossPtrList CrossPtrList;
	emRef<emCoreConfig> CoreConfig;
	emViewPort * DummyViewPort;
	emViewPort * HomeViewPort;
	emViewPort * CurrentViewPort;
	emWindow * Window;
	emRef<emModel> ScreenRef;
	emWindow * PopupWindow;
	emViewInputFilter * FirstVIF;
	emViewInputFilter * LastVIF;
	emViewAnimator * ActiveAnimator;
	emMagneticViewAnimator * MagneticVA;
	emVisitingViewAnimator * VisitingVA;
	emPanel * RootPanel;
	emPanel * SupremeViewedPanel;
	emPanel * MinSVP, * MaxSVP;
	emPanel * ActivePanel;
	emSignal ViewFlagsSignal;
	emSignal TitleSignal;
	emSignal ControlPanelSignal;
	emSignal FocusSignal;
	emSignal GeometrySignal;
	emSignal EOISignal;
	double HomeX,HomeY,HomeWidth,HomeHeight,HomePixelTallness;
	double CurrentX,CurrentY,CurrentWidth,CurrentHeight,
	       CurrentPixelTallness;
	double LastMouseX,LastMouseY;
	emString Title;
	emCursor Cursor;
	emColor BackgroundColor;
	ViewFlags VFlags;
	bool Focused;
	bool ActivationAdherent;
	bool ZoomScrollInAction;
	bool TitleInvalid;
	bool CursorInvalid;
	bool SVPChoiceInvalid;
	bool SVPChoiceByOpacityInvalid;
	bool GotPopupWindowCloseSignal;
	bool RestartInputRecursion;
	bool ZoomedOutBeforeSG;
	int SettingGeometry;
	int SVPUpdCount;
	emUInt64 SVPUpdSlice;
	emInputEvent NoEvent;
	PanelRingNode NoticeList;
	UpdateEngineClass * UpdateEngine;
	EOIEngineClass * EOIEngine;
	emPanel * SeekPosPanel;
	emString SeekPosChildName;
	StressTestClass * StressTest;

	static const double MaxSVPSize;
	static const double MaxSVPSearchSize;
};


//==============================================================================
//================================= emViewPort =================================
//==============================================================================

class emViewPort : public emUncopyable {

public:

	// Base class for the connection between an emView and the operating
	// system or the hardware or whatever. The default implementation
	// connects the view to nothing. When a view is constructed, it creates
	// such a dummy view port just to have a view port at all. A real view
	// port should be created on the view afterwards. For example, this is
	// solved in emWindow by creating an emWindowPort, which is a derivative
	// of emViewPort.
	//
	// Note that emView implements some tricks for solving the popup-zoom
	// feature. When popping up, the view creates a private emWindow and
	// exchanges the view port of that window for its own view port
	// temporarily. A view port derivative must come clear with such an
	// exchange (do not store any references to the view in it).
	//
	// The title is not interfaced through emViewPort (but through
	// emWindowPort) because it must not be exchanged when the view pops up.
	//
	// The popup-zoom feature is even the reason why all views must share
	// the same coordinate system. It should be the coordinate system of
	// screen.

	emViewPort(emView & homeView);

	virtual ~emViewPort();

	double GetViewX() const;
	double GetViewY() const;
	double GetViewWidth() const;
	double GetViewHeight() const;

	emCursor GetViewCursor() const;

	void PaintView(const emPainter & painter, emColor canvasColor) const;

protected:

	void SetViewGeometry(double x, double y, double w, double h,
	                     double pixelTallness);

	void SetViewFocused(bool focused);
	virtual void RequestFocus();

	virtual bool IsSoftKeyboardShown() const;
	virtual void ShowSoftKeyboard(bool show);

	virtual emUInt64 GetInputClockMS() const;

	void InputToView(emInputEvent & event, const emInputState & state);

	virtual void InvalidateCursor();

	virtual void InvalidatePainting(double x, double y, double w, double h);

	// - - - - - - - - - - Depreciated methods - - - - - - - - - - - - - - -
	// The following virtual non-const methods have been replaced by const
	// methods (see above). The old versions still exist here with the
	// "final" keyword added, so that old overridings will fail to compile.
	// If you run into this, please adapt your overridings by adding "const".
	virtual bool IsSoftKeyboardShown() final;
	virtual emUInt64 GetInputClockMS() final;
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
	friend class emView;
	emViewPort();
	emView * HomeView;
	emView * CurrentView;
};


//==============================================================================
//============================== Implementations ===============================
//==============================================================================

//----------------------------------- emView -----------------------------------

inline void emView::LinkCrossPtr(emCrossPtrPrivate & crossPtr)
{
	CrossPtrList.LinkCrossPtr(crossPtr);
}

inline emView::ViewFlags emView::GetViewFlags() const
{
	return VFlags;
}

inline const emSignal & emView::GetViewFlagsSignal() const
{
	return ViewFlagsSignal;
}

inline emColor emView::GetBackgroundColor() const
{
	return BackgroundColor;
}

inline const emSignal & emView::GetTitleSignal() const
{
	return TitleSignal;
}

inline const emSignal & emView::GetControlPanelSignal() const
{
	return ControlPanelSignal;
}

inline bool emView::IsFocused() const
{
	return Focused;
}

inline const emSignal & emView::GetFocusSignal() const
{
	return FocusSignal;
}

inline emWindow * emView::GetWindow() const
{
	return Window;
}

inline double emView::GetHomeX() const
{
	return HomeX;
}

inline double emView::GetHomeY() const
{
	return HomeY;
}

inline double emView::GetHomeWidth() const
{
	return HomeWidth;
}

inline double emView::GetHomeHeight() const
{
	return HomeHeight;
}

inline double emView::GetHomePixelTallness() const
{
	return HomePixelTallness;
}

inline double emView::GetHomeTallness() const
{
	return HomeHeight/HomeWidth*HomePixelTallness;
}

inline double emView::GetCurrentX() const
{
	return CurrentX;
}

inline double emView::GetCurrentY() const
{
	return CurrentY;
}

inline double emView::GetCurrentWidth() const
{
	return CurrentWidth;
}

inline double emView::GetCurrentHeight() const
{
	return CurrentHeight;
}

inline double emView::GetCurrentPixelTallness() const
{
	return CurrentPixelTallness;
}

inline double emView::GetCurrentTallness() const
{
	return CurrentHeight/CurrentWidth*CurrentPixelTallness;
}

inline const emSignal & emView::GetGeometrySignal() const
{
	return GeometrySignal;
}

inline emViewInputFilter * emView::GetFirstVIF() const
{
	return FirstVIF;
}

inline emViewInputFilter * emView::GetLastVIF() const
{
	return LastVIF;
}

inline emPanel * emView::GetRootPanel() const
{
	return RootPanel;
}

inline emPanel * emView::GetSupremeViewedPanel() const
{
	return SupremeViewedPanel;
}

inline emPanel * emView::GetActivePanel() const
{
	return ActivePanel;
}

inline bool emView::IsActivationAdherent() const
{
	return ActivationAdherent;
}

inline bool emView::IsPoppedUp() const
{
	return PopupWindow!=NULL;
}

inline const emSignal & emView::GetEOISignal() const
{
	return EOISignal;
}

inline bool emView::IsSoftKeyboardShown() const
{
	return CurrentViewPort->IsSoftKeyboardShown();
}

inline void emView::ShowSoftKeyboard(bool show)
{
	CurrentViewPort->ShowSoftKeyboard(show);
}

inline emUInt64 emView::GetInputClockMS() const
{
	return CurrentViewPort->GetInputClockMS();
}

inline emViewAnimator * emView::GetActiveAnimator() const
{
	return ActiveAnimator;
}

inline void emView::InvalidateCursor()
{
	CurrentViewPort->InvalidateCursor();
}

inline void emView::InvalidatePainting()
{
	CurrentViewPort->InvalidatePainting(
		CurrentX,CurrentY,CurrentWidth,CurrentHeight
	);
}

inline void emView::InvalidatePainting(double x, double y, double w, double h)
{
	CurrentViewPort->InvalidatePainting(x,y,w,h);
}


//--------------------------------- emViewPort ---------------------------------

inline double emViewPort::GetViewX() const
{
	return CurrentView->GetCurrentX();
}

inline double emViewPort::GetViewY() const
{
	return CurrentView->GetCurrentY();
}

inline double emViewPort::GetViewWidth() const
{
	return CurrentView->GetCurrentWidth();
}

inline double emViewPort::GetViewHeight() const
{
	return CurrentView->GetCurrentHeight();
}

inline emCursor emViewPort::GetViewCursor() const
{
	return CurrentView->GetCursor();
}

inline void emViewPort::PaintView(
	const emPainter & painter, emColor canvasColor
) const
{
	CurrentView->Paint(painter,canvasColor);
}

inline void emViewPort::SetViewGeometry(
	double x, double y, double w, double h, double pixelTallness
)
{
	CurrentView->SetGeometry(x,y,w,h,pixelTallness);
}

inline void emViewPort::SetViewFocused(bool focused)
{
	CurrentView->SetFocused(focused);
}


#endif
