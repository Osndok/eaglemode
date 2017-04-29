//------------------------------------------------------------------------------
// emAutoplay.h
//
// Copyright (C) 2017 Oliver Hamann.
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

#ifndef emAutoplay_h
#define emAutoplay_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emViewAnimator_h
#include <emCore/emViewAnimator.h>
#endif


//==============================================================================
//============================== emAutoplayConfig ==============================
//==============================================================================

class emAutoplayConfig : public emConfigModel, public emStructRec {

public:

	static emRef<emAutoplayConfig> Acquire(
		emRootContext & rootContext, const emString & filePath
	);

	emIntRec DurationMS;
	emBoolRec Recursive;
	emBoolRec Loop;
	emBoolRec LastLocationValid;
	emStringRec LastLocation;

	virtual const char * GetFormatName() const;

protected:

	emAutoplayConfig(emContext & context, const emString & filePath);
	virtual ~emAutoplayConfig();
};


//==============================================================================
//=========================== emAutoplayViewAnimator ===========================
//==============================================================================

class emAutoplayViewAnimator : public emViewAnimator {

public:

	emAutoplayViewAnimator(emView & view);
	virtual ~emAutoplayViewAnimator();

	bool IsRecursive() const;
	void SetRecursive(bool recursive);
	bool IsLoop() const;
	void SetLoop(bool loop);

	void SetGoalToItemAt(emPanel * panel);
	void SetGoalToItemAt(const emString & panelIdentity);
	void SetGoalToPreviousItemOf(emPanel * panel);
	void SetGoalToNextItemOf(emPanel * panel);
	void SkipToPreviousItem();
	void SkipToNextItem();
	void ClearGoal();

	bool HasGoal() const;
	bool HasReachedGoal() const;
	bool HasGivenUp() const;

	emPanel * GetCurrentPanel() const;
	const emString & GetCurrentPanelIdentity() const;

	virtual void Activate();
	virtual void Deactivate();

protected:

	virtual bool CycleAnimation(double dt);

private:

	bool LowPriCycle();

	enum AdvanceResult {
		AR_AGAIN,
		AR_FAILED,
		AR_FINISHED
	};

	AdvanceResult AdvanceCurrentPanel();
	bool IsItem(const emPanel * p) const;
	bool IsCutoff(const emPanel * p) const;
	void GoParent();
	void GoChild(emPanel * c);
	void GoSame();
	void InvertDirection();

	class LowPriEngineClass : public emEngine {
	public:
		LowPriEngineClass(emAutoplayViewAnimator & owner);
		virtual ~LowPriEngineClass();
	protected:
		virtual bool Cycle();
	private:
		emAutoplayViewAnimator & Owner;
	};

	friend class LowPriEngineClass;

	LowPriEngineClass LowPriEngine;

	emVisitingViewAnimator VisitingVA;

	emRef<emCoreConfig> CoreConfig;

	bool Recursive;
	bool Loop;

	enum StateEnum {
		ST_NO_GOAL,
		ST_UNFINISHED,
		ST_GIVEN_UP,
		ST_GOAL_REACHED
	};
	StateEnum State;

	bool OneMoreWakeUp;

	bool Backwards;

	int SkipItemCount;
	bool SkipCurrent;

	bool NextLoopEndless;

	enum CameFromType {
		CAME_FROM_NONE,
		CAME_FROM_PARENT,
		CAME_FROM_CHILD
	};
	CameFromType CameFrom;
	emString CameFromChildName;

	emString CurrentPanelIdentity;
	emCrossPtr<emPanel> CurrentPanel;

	enum CurrentPanelStateEnum {
		CP_NOT_VISITED,
		CP_VISITING,
		CP_VISITED,
	};
	CurrentPanelStateEnum CurrentPanelState;
};

inline bool emAutoplayViewAnimator::IsRecursive() const
{
	return Recursive;
}

inline bool emAutoplayViewAnimator::IsLoop() const
{
	return Loop;
}

inline bool emAutoplayViewAnimator::HasGoal() const
{
	return State!=ST_NO_GOAL;
}

inline bool emAutoplayViewAnimator::HasReachedGoal() const
{
	return State==ST_GOAL_REACHED;
}

inline bool emAutoplayViewAnimator::HasGivenUp() const
{
	return State==ST_GIVEN_UP;
}

inline emPanel * emAutoplayViewAnimator::GetCurrentPanel() const
{
	return CurrentPanel;
}

inline const emString & emAutoplayViewAnimator::GetCurrentPanelIdentity() const
{
	return CurrentPanelIdentity;
}


//==============================================================================
//============================ emAutoplayViewModel =============================
//==============================================================================

class emAutoplayViewModel : public emModel {

public:

	static emRef<emAutoplayViewModel> Acquire(emView & view);

	void SetConfigFilePath(const emString & configFilePath);

	const emSignal & GetChangeSignal() const;

	int GetDurationMS() const;
	void SetDurationMS(int durationMS);

	bool IsRecursive() const;
	void SetRecursive(bool recursive);

	bool IsLoop() const;
	void SetLoop(bool loop);

	bool IsAutoplaying() const;
	void SetAutoplaying(bool autoPlaying);

	bool CanContinueLastAutoplay() const;
	void ContinueLastAutoplay();

	void SkipToPreviousItem();
	void SkipToNextItem();

	const emSignal & GetProgressSignal() const;
	double GetItemProgress() const;

	void Input(emInputEvent & event, const emInputState & state);

protected:

	emAutoplayViewModel(emView & view, const emString & name);
	virtual ~emAutoplayViewModel();

	virtual bool Cycle();

private:

	void SetItemProgress(double itemProgress);
	void StartItemPlaying(emPanel * panel);
	void UpdateItemPlaying();
	void StopItemPlaying(bool resetPos=false);
	bool CheckPlayingPanel() const;
	void UpdateFullsized(bool initially=false);
	void SaveLocation(emPanel * panel);
	void SetScreensaverInhibited(bool inhibited);

	emView & View;

	emRef<emAutoplayConfig> Config;

	emSignal ChangeSignal;

	int DurationMS;

	emAutoplayViewAnimator ViewAnimator;
	emUInt64 ViewAnimatorStartTime;

	bool Autoplaying;
	bool LastLocationValid;
	emString LastLocation;

	emSignal ProgressSignal;
	double ItemProgress;

	bool ScreensaverInhibited;
	bool PlayedAnyInCurrentSession;

	bool PlayingItem;
	emCrossPtr<emPanel> PlayingPanel;
	bool PlaybackActive;
	emUInt64 ItemPlayStartTime;

	double ERectX,ERectY,ERectW,ERectH;
};

inline const emSignal & emAutoplayViewModel::GetChangeSignal() const
{
	return ChangeSignal;
}

inline int emAutoplayViewModel::GetDurationMS() const
{
	return DurationMS;
}

inline bool emAutoplayViewModel::IsRecursive() const
{
	return ViewAnimator.IsRecursive();
}

inline bool emAutoplayViewModel::IsLoop() const
{
	return ViewAnimator.IsLoop();
}

inline bool emAutoplayViewModel::IsAutoplaying() const
{
	return Autoplaying;
}

inline const emSignal & emAutoplayViewModel::GetProgressSignal() const
{
	return ProgressSignal;
}

inline double emAutoplayViewModel::GetItemProgress() const
{
	return ItemProgress;
}


//==============================================================================
//=========================== emAutoplayControlPanel ===========================
//==============================================================================

class emAutoplayControlPanel : public emPackGroup {

public:

	emAutoplayControlPanel(
		ParentArg parent, const emString & name,
		emView & contentView
	);

	virtual ~emAutoplayControlPanel();

protected:

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	void UpdateControls();
	void UpdateProgress();

	static int DurationValueToMS(emInt64 value);
	static emInt64 DurationMSToValue(int ms);
	static void DurationTextOfValue(
		char * buf, int bufSize, emInt64 value,
		emUInt64 markInterval, void * context
	);

	class AutoplayButton : public emCheckButton {

	public:

		AutoplayButton(
			ParentArg parent, const emString & name,
			const emString & caption=emString(),
			const emString & description=emString(),
			const emImage & icon=emImage()
		);

		virtual ~AutoplayButton();

		void SetProgress(double progress);

	protected:

		virtual void PaintLabel(
			const emPainter & painter, double x, double y, double w,
			double h, emColor color, emColor canvasColor
		) const;

	private:
		double Progress;
	};

	emRef<emAutoplayViewModel> Model;

	AutoplayButton * BtAutoplay;
	emButton * BtPrev;
	emButton * BtNext;
	emButton * BtContinueLast;
	emScalarField * SfDuration;
	emCheckBox * CbRecursive;
	emCheckBox * CbLoop;
};

#endif
