//------------------------------------------------------------------------------
// emView.h
//
// Copyright (C) 2004-2011 Oliver Hamann.
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

	virtual emString GetTitle();
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

	const emSignal & GetFocusSignal();
		// This signal is signaled when the focus of this view has
		// changed.

	void Focus();
		// Make that this view has the focus, if possible by the
		// emViewPort implementation. Hint: If this is an emWindow, you
		// may want to call Raise() before.

	emWindow * GetWindow();
		// Get the window of this view. It returns the nearest window
		// within the path of contexts to this view. If this view itself
		// is a window, that window is returned. If no window can be
		// found, NULL is returned. The result is cached internally.

	emScreen * GetScreen();
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

	emViewInputFilter * GetFirstVIF();
	emViewInputFilter * GetLastVIF();
		// Get the first or last view input filter (VIF) in the chain of
		// the VIFs of this view. The default chain (after contructing
		// the view) contains emDefaultTouchVIF, emCheatVIF,
		// emKeyboardZoomScrollVIF, and emMouseZoomScrollVIF in that
		// order. This could change in future versions. You can modify
		// the chain simply by deleting and creating VIFs as you like.
		// See emViewInputFilter for more details. Note that the
		// zoom/scroll VIFs have no effect when VF_NO_USER_NAVIGATION is
		// set in the view flags. Also, never forget about the
		// possibility of parent and child views (=>emSubViewPanel).
		// Parent views should always have VF_NO_ZOOM.

	emPanel * GetRootPanel();
		// Get the root panel of this view. Returns NULL when this view
		// has no panels.

	emPanel * GetSupremeViewedPanel();
		// Get the supreme viewed panel. It is the upper-most panel in
		// the panel tree which has IsViewed()==true. Returns NULL when
		// this view has no panels. The supreme viewed panel is always
		// chosen automatically by the view, depending on the visit
		// state.

	emPanel * GetActivePanel();
		// Get the active panel. The active panel is the panel which is
		// focused when the view is focused. Returns NULL when this view
		// has no panels.

	emPanel * GetPanelByIdentity(const char * identity);
		// Search for a panel by identity (see emPanel::GetIdentity()).
		// Returns NULL if not found.

	emPanel * GetPanelAt(double x, double y);
		// Get the uppermost panel at the given point.

	emPanel * GetFocusablePanelAt(double x, double y);
		// Get the uppermost focusable panel at the given point.

	emPanel * GetVisitedPanel(double * pRelX=NULL, double * pRelY=NULL,
	                          double * pRelA=NULL, bool * pAdherent=NULL);
		// Get the visited panel and optionally relative coordinates and
		// the visit type. The visited panel is the one on which the
		// view is currently anchored. That means, if any panel layout
		// is changed, the visited panel keeps its position within the
		// view.
		// Arguments:
		//   pRelX, pRelY - Pointers for returning the distance vector
		//                  between the center of the view and the
		//                  center of the panel, measured in view widths
		//                  and heights.
		//   pRelA        - Pointer for returning the area size of view
		//                  relative to the area size of the panel.
		//   pAdherent    - Pointer for returning IsVisitAdherent().
		// Returns:
		//   The visited panel, or NULL if this view has no panels.

	bool IsVisitAdherent() const;
		// Whether the visit is adherent. It usually means that the
		// visit has been set by the user.

	void Visit(emPanel * panel, bool adherent);
		// Visit a panel. This makes sure that the given panel is
		// visible and that is is shown not too large and not too small,
		// and it makes the panel the visited panel.
		// Arguments:
		//   panel    - The panel to be visited.
		//   adherent - Whether the visit shall be adherent.

	void Visit(emPanel * panel, double relX, double relY, double relA,
	           bool adherent);
		// Visit a panel and position the view relative to it. The
		// arguments are exactly the things which you can get with
		// GetVisitedPanel(...) (read there). But if relA is less or
		// equal 0.0, VisitFullsized(panel,adherent) is performed.

	void VisitBy(emPanel * panel, double relX, double relY, double relA);
		// Position the view relative to a panel and give the visit to
		// any panel which is somehow shown centered and well-sized by
		// the new position. The arguments are like with the Visit
		// method above.

	void VisitLazy(emPanel * panel, bool adherent);
		// Visit a panel while being lazy in positioning the view.

	void VisitFullsized(emPanel * panel, bool adherent,
	                    bool utilizeView=false);
		// Visit a panel and position the view so that the panel is
		// shown full-sized.

	void VisitByFullsized(emPanel * panel);
		// Position the view so that the given panel is shown full-sized
		// and give the visit to any panel which is somehow shown
		// centered and well-sized by the new position.

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
		// Visit a sister, parent or child of the currently visited
		// panel. This walks the tree of focusable panels only.
		// VisitNeighbour(0) is like VisitRight(), 1 like Down, 2 like
		// Left and 3 like Up.

	void Seek(const char * identity, bool adherent,
	          const char * subject=NULL);
	void Seek(const char * identity, double relX, double relY, double relA,
	          bool adherent, const char * subject=NULL);
	void SeekBy(const char * identity, double relX, double relY,
	            double relA, const char * subject=NULL);
	void SeekLazy(const char * identity, bool adherent,
	              const char * subject=NULL);
	void SeekFullsized(const char * identity, bool adherent,
	                   const char * subject=NULL);
	void SeekByFullsized(const char * identity, const char * subject=NULL);
		// These methods are like Visit, VisitBy, VisitLazy and so on,
		// but: The panel is referred by its identity instead of a
		// pointer. And, if the panel cannot be found immediately, a
		// seek algorithm is started which runs in a private engine
		// possibly for long time. The algorithm visits panels on the
		// path to the desired panel step by step, in the hope they are
		// expanding the path accordingly. For the whole time, the view
		// shows an info box and allows the user to abort the seeking by
		// any input. The additional argument 'subject' can be set to
		// give a nice name for the sought panel in the info box.

	bool IsSeeking() const;
		// Whether the view is currently seeking for a panel.

	void AbortSeeking();
		// Abort any seeking.

	void Zoom(double fixX, double fixY, double factor);
		// Zoom the view.
		// Arguments:
		//   fixX, fixY - Fix point for the zooming.
		//   factor     - Zoom factor. A value less than 1.0 means to
		//                zoom out, and a value greater than 1.0 means
		//                to zoom in.

	void Scroll(double deltaX, double deltaY);
		// Scroll the view.
		// Arguments:
		//   deltaX - How much to scroll in X direction (positive value
		//            means to scroll right).
		//   deltaY - How much to scroll in Y direction (positive value
		//            means to scroll down).

	void ZoomOut();
		// Zoom out the view completely.

	bool IsZoomedOut();
		// Whether the view is currently zoomed out completely.

	bool IsPoppedUp() const;
		// Whether the view is in popped up state.

	void SignalEOIDelayed();
		// After calling this, the End-Of-Interaction signal will be
		// signaled, but with a delay of some time slices.

	const emSignal & GetEOISignal() const;
		// Get the End-Of-Interaction signal. This signal indicates an
		// end of a temporary user interaction. It has been invented for
		// emTkButton (see emTkButton::IsNoEOI()). If the view has
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
	);
		// Get the touch event priority of this view for a certain touch
		// position. (See emPanel::GetTouchEventPriority).
		// Arguments:
		//   touchX, touchY - Position of a first touch in view
		//                    coordinates.
		//   afterVIFs      - Whether to leave out the view input filters
		//                    in the calculation.

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

	virtual emCursor GetCursor();
		// Get the mouse cursor to be shown for this view. The default
		// implementation returns the cursor of the panel where the
		// mouse is. See also: InvalidateCursor()

	virtual void Paint(const emPainter & painter, emColor canvasColor);
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
		// implemented by emView itself, see the implementation of
		// emView::DoCheats.

private:
	friend class emViewPort;
	friend class emViewInputFilter;
	friend class emCheatVIF;
	friend class emPanel;
	friend class emSubViewPanel;

	struct PanelRingNode {
		PanelRingNode * Prev;
		PanelRingNode * Next;
	};

	void SetFocused(bool focused);
	void SetGeometry(double x, double y, double width, double height,
	                 double pixelTallness);

	void AddToNoticeList(PanelRingNode * node);

	void Update();

	void CalcVisitFullsizedCoords(emPanel * panel, double * pRelX,
	                              double * pRelY, double * pRelA,
	                              bool utilizeView=false);

	void VisitRelBy(emPanel * panel, double relX, double relY, double relA,
	                bool forceViewingUpdate);
	void VisitRel(emPanel * panel, double relX, double relY, double relA,
	              bool adherent, bool forceViewingUpdate);
	void VisitAbs(emPanel * panel, double vx, double vy, double vw,
	              bool adherent, bool forceViewingUpdate);
	void VisitImmobile(emPanel * panel, bool adherent);

	void FindBestSVP(emPanel * * pPanel, double * pVx, double * pVy,
	                 double * pVw);
	bool FindBestSVPInTree(emPanel * * pPanel, double * pVx, double * pVy,
	                       double * pVw, bool covering);

	void SwapViewPorts(bool swapFocus);

	void RecurseInput(emInputEvent & event,
	                  const emInputState & state);
	void RecurseChildrenInput(emPanel * parent, double mx, double my,
	                          double tx, double ty,
	                          emInputEvent & event,
	                          const emInputState & state);

	void InvalidateHighlight();
	void PaintHighlight(const emPainter & painter);

	void SetSeekPos(emPanel * panel, const char * childName);
	bool IsHopeForSeeking();

	void SetActivationCandidate(emPanel * panel);

	class UpdateEngineClass : public emEngine {
	public:
		UpdateEngineClass(emView & view);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
	};
	friend class UpdateEngineClass;

	class ActivationEngineClass : public emEngine {
	public:
		ActivationEngineClass(emView & view);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
	};
	friend class ActivationEngineClass;

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

	class SeekEngineClass : public emEngine {
	public:
		SeekEngineClass(
			emView & view, int seekType, const emString & identity,
			double relX, double relY, double relA, bool adherent,
			const emString & subject
		);
		virtual ~SeekEngineClass();
		void Paint(const emPainter & painter);
	protected:
		virtual bool Cycle();
	private:
		emView & View;
		int SeekType;
		emString Identity;
		double RelX,RelY,RelA;
		bool Adherent;
		emString Subject;
		emArray<emString> Names;
		int TimeSlicesWithoutHope;
		bool GiveUp;
		emUInt64 GiveUpClock;
	};
	friend class SeekEngineClass;

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
	emWindow * WindowPtrCache;
	emRef<emModel> ScreenRefCache;
	bool WindowPtrValid;
	bool ScreenRefValid;
	emWindow * PopupWindow;
	emViewInputFilter * FirstVIF;
	emViewInputFilter * LastVIF;
	emPanel * RootPanel;
	emPanel * SupremeViewedPanel;
	emPanel * MinSVP, * MaxSVP;
	emPanel * ActivePanel;
	emPanel * ActivationCandidate;
	emPanel * VisitedPanel;
	emSignal ViewFlagsSignal;
	emSignal TitleSignal;
	emSignal ControlPanelSignal;
	emSignal FocusSignal;
	emSignal GeometrySignal;
	emSignal EOISignal;
	emUInt64 PanelCreationNumber;
	double HomeX,HomeY,HomeWidth,HomeHeight,HomePixelTallness;
	double CurrentX,CurrentY,CurrentWidth,CurrentHeight,
	       CurrentPixelTallness;
	double LastMouseX,LastMouseY;
	emString Title;
	emCursor Cursor;
	emColor BackgroundColor;
	ViewFlags VFlags;
	bool Focused;
	bool VisitAdherent;
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
	ActivationEngineClass * ActivationEngine;
	EOIEngineClass * EOIEngine;
	SeekEngineClass * SeekEngine;
	int ProtectSeeking;
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

protected:

	void SetViewGeometry(double x, double y, double w, double h,
	                     double pixelTallness);

	double GetViewX() const;
	double GetViewY() const;
	double GetViewWidth() const;
	double GetViewHeight() const;

	void SetViewFocused(bool focused);
	virtual void RequestFocus();

	virtual bool IsSoftKeyboardShown();
	virtual void ShowSoftKeyboard(bool show);

	virtual emUInt64 GetInputClockMS();

	void InputToView(emInputEvent & event, const emInputState & state);

	emCursor GetViewCursor();

	void PaintView(const emPainter & painter, emColor canvasColor);

	virtual void InvalidateCursor();

	virtual void InvalidatePainting(double x, double y, double w, double h);

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

inline const emSignal & emView::GetFocusSignal()
{
	return FocusSignal;
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

inline emViewInputFilter * emView::GetFirstVIF()
{
	return FirstVIF;
}

inline emViewInputFilter * emView::GetLastVIF()
{
	return LastVIF;
}

inline emPanel * emView::GetRootPanel()
{
	return RootPanel;
}

inline emPanel * emView::GetSupremeViewedPanel()
{
	return SupremeViewedPanel;
}

inline emPanel * emView::GetActivePanel()
{
	return ActivePanel;
}

inline bool emView::IsVisitAdherent() const
{
	return VisitAdherent;
}

inline bool emView::IsSeeking() const
{
	return SeekEngine!=NULL;
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

inline void emViewPort::SetViewGeometry(
	double x, double y, double w, double h, double pixelTallness
)
{
	CurrentView->SetGeometry(x,y,w,h,pixelTallness);
}

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

inline void emViewPort::SetViewFocused(bool focused)
{
	CurrentView->SetFocused(focused);
}

inline emCursor emViewPort::GetViewCursor()
{
	return CurrentView->GetCursor();
}

inline void emViewPort::PaintView(
	const emPainter & painter, emColor canvasColor
)
{
	CurrentView->Paint(painter,canvasColor);
}


#endif
