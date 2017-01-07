//------------------------------------------------------------------------------
// emViewInputFilter.h
//
// Copyright (C) 2011-2012,2014,2016 Oliver Hamann.
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

#ifndef emViewInputFilter_h
#define emViewInputFilter_h

#ifndef emViewAnimator_h
#include <emCore/emViewAnimator.h>
#endif


//==============================================================================
//============================= emViewInputFilter ==============================
//==============================================================================

class emViewInputFilter : public emEngine {

public:

	// Base class for an input event filter of an emView. Each view can have
	// a list of filters. Such a filter typically eats certain events for
	// zooming and scrolling the view.

	emViewInputFilter(emView & view, emViewInputFilter * next=NULL);
		// Construct a view input filter and insert it to the list of
		// filters of a view.
		// Arguments:
		//   view - The view.
		//   next - The next filter in the list. This filter is
		//          inserted before that filter. If next is NULL, this
		//          filter is appended to the end of the list.

	virtual ~emViewInputFilter();
		// Destruct this view input filter. This also removes the filter
		// from the list.

	emView & GetView() const;
		// Get the view.

	emViewInputFilter * GetPrev() const;
	emViewInputFilter * GetNext() const;
		// Get the previous or next filter in the list. NULL means this
		// is the first or last filter.

	virtual double GetTouchEventPriority(double touchX, double touchY) const;
		// Get the maximum touch event priority of this view input
		// filter and all its successors and all the panels of the view
		// for a certain touch position. The default implementation
		// calls GetForwardTouchEventPriority. This should usually also
		// be made by overloaded implementations. See the comments on
		// emPanel::GetTouchEventPriority for more.
		// Arguments:
		//   touchX, touchY - Position of a first touch in view
		//                    coordinates.

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);
		// Process input form keyboard, mouse, and touch. The default
		// implementation calls ForwardInput. This should usually also
		// be made by overloaded implementations.
		// Arguments:
		//   event  - An input event. It may be eaten by calling
		//            event.Eat(). The event reference in non-const only
		//            for that.
		//   state  - The current input state.

	virtual bool Cycle();
		// emViewInputFilter has been derived from emEngine for
		// convenience. This default implementation does nothing and
		// returns false.

	void ForwardInput(emInputEvent & event, const emInputState & state);
		// Forward input to succeeding filters and the panels. Actually
		// this calls Input on the next filter, or on the view if this
		// is the last filter.
		// Arguments:
		//   event  - An input event. It may be eaten by calling
		//            event.Eat(). The event reference in non-const only
		//            for that.
		//   state  - The current input state.

	double GetForwardTouchEventPriority(double touchX, double touchY) const;
		// Get the maximum touch event priority of succeeding filters
		// and the panels. Actually this calls GetTouchEventPriority on
		// the next filter, or on the view if this is the last filter.
		// Arguments:
		//   touchX, touchY - Position of a first touch in view
		//                    coordinates.

private:
	friend class emViewPort;
	emView & View;
	emViewInputFilter * Prev;
	emViewInputFilter * Next;
};

inline emView & emViewInputFilter::GetView() const
{
	return View;
}

inline emViewInputFilter * emViewInputFilter::GetPrev() const
{
	return Prev;
}

inline emViewInputFilter * emViewInputFilter::GetNext() const
{
	return Next;
}

inline void emViewInputFilter::ForwardInput(
	emInputEvent & event, const emInputState & state
)
{
	if (!Next) View.Input(event,state);
	else Next->Input(event,state);
}

inline double emViewInputFilter::GetForwardTouchEventPriority(
	double touchX, double touchY
) const
{
	if (!Next) return View.GetTouchEventPriority(touchX,touchY,true);
	else return Next->GetTouchEventPriority(touchX,touchY);
}


//==============================================================================
//============================ emMouseZoomScrollVIF ============================
//==============================================================================

class emMouseZoomScrollVIF : public emViewInputFilter {

public:

	// This view input filter eats some mouse events for zooming and
	// scrolling.

	emMouseZoomScrollVIF(emView & view, emViewInputFilter * next=NULL);
	virtual ~emMouseZoomScrollVIF();

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);

	virtual bool Cycle();

private:

	void EmulateMiddleButton(emInputEvent & event, emInputState & state);
	bool MoveMousePointerBackIntoView(double * pmx, double * pmy);
	void MoveMousePointer(double dx, double dy);
	double GetMouseZoomSpeed(bool fine=false) const;
	double GetMouseScrollSpeed(bool fine=false) const;
	void UpdateWheelZoomSpeed(bool down, bool fine);
	void SetMouseAnimParams();
	void SetWheelAnimParams();
	void InitMagnetismAvoidance();
	void UpdateMagnetismAvoidance(double dmx, double dmy);

	emSwipingViewAnimator MouseAnim;
	emSwipingViewAnimator WheelAnim;
	emRef<emCoreConfig> CoreConfig;
	double LastMouseX,LastMouseY,ZoomFixX,ZoomFixY;
	emUInt64 EmuMidButtonTime;
	int EmuMidButtonRepeat;
	double WheelZoomSpeed;
	emUInt64 WheelZoomTime;
	bool MagnetismAvoidance;
	double MagAvMouseMoveX,MagAvMouseMoveY;
	emUInt64 MagAvTime;
};


//==============================================================================
//========================== emKeyboardZoomScrollVIF ===========================
//==============================================================================

class emKeyboardZoomScrollVIF : public emViewInputFilter {

public:

	// This view input filter eats some keyboard events for zooming and
	// scrolling.

	emKeyboardZoomScrollVIF(emView & view, emViewInputFilter * next=NULL);
	virtual ~emKeyboardZoomScrollVIF();

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);

private:

	void NavigateByProgram(emInputEvent & event, const emInputState & state);
	double GetZoomSpeed(bool fine=false) const;
	double GetScrollSpeed(bool fine=false) const;
	void SetAnimatorParameters();

	emSpeedingViewAnimator Animator;
	emRef<emCoreConfig> CoreConfig;
	bool Active;
	int NavByProgState;
};


//==============================================================================
//================================= emCheatVIF =================================
//==============================================================================

class emCheatVIF : public emViewInputFilter {

public:

	// This view input filter implements some chat codes.

	emCheatVIF(emView & view, emViewInputFilter * next=NULL);
	virtual ~emCheatVIF();

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);

private:

	emRef<emCoreConfig> CoreConfig;
	char CheatBuffer[64];
};


//==============================================================================
//============================= emDefaultTouchVIF ==============================
//==============================================================================

class emDefaultTouchVIF : public emViewInputFilter {

public:

	// This view input filter eats touch events for zooming and scrolling
	// and for emulating mouse events.

	emDefaultTouchVIF(emView & view, emViewInputFilter * next=NULL);
	virtual ~emDefaultTouchVIF();

	virtual double GetTouchEventPriority(double touchX, double touchY) const;

protected:

	virtual void Input(emInputEvent & event, const emInputState & state);

	virtual bool Cycle();

private:

	void DoGesture();
	void ResetTouches();
	void NextTouches();
	void RemoveTouch(int index);
	bool IsAnyTouchDown() const;
	double GetTouchMoveX(int index) const;
	double GetTouchMoveY(int index) const;
	double GetTouchMove(int index) const;
	double GetTotalTouchMoveX(int index) const;
	double GetTotalTouchMoveY(int index) const;
	double GetTotalTouchMove(int index) const;

	struct Touch {
		emUInt64 Id;
		int MsTotal;
		int MsSincePrev;
		bool Down;
		double X;
		double Y;
		bool PrevDown;
		double PrevX;
		double PrevY;
		double DownX;
		double DownY;
	};

	enum { MAX_TOUCH_COUNT=16 };

	emInputState InputState;
	emInputEvent InputEvent;
	Touch Touches[MAX_TOUCH_COUNT];
	int TouchCount;
	emUInt64 TouchesTime;
	int GestureState;
};


#endif
