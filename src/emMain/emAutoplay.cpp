//------------------------------------------------------------------------------
// emAutoplay.cpp
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

#include <emMain/emAutoplay.h>
#include <emCore/emRes.h>


//==============================================================================
//============================== emAutoplayConfig ==============================
//==============================================================================

emRef<emAutoplayConfig> emAutoplayConfig::Acquire(
	emRootContext & rootContext, const emString & filePath
)
{
	EM_IMPL_ACQUIRE_COMMON(emAutoplayConfig,rootContext,filePath)
}


const char * emAutoplayConfig::GetFormatName() const
{
	return "emAutoplay";
}


emAutoplayConfig::emAutoplayConfig(
	emContext & context, const emString & filePath
)
	: emConfigModel(context,filePath),
	emStructRec(),
	DurationMS(this,"DurationMS",5000,0),
	Recursive(this,"Recursive",false),
	Loop(this,"Loop",false),
	LastLocationValid(this,"LastLocationValid",false),
	LastLocation(this,"LastLocation","")
{
	PostConstruct(*this,filePath);
	LoadOrInstall();
}


emAutoplayConfig::~emAutoplayConfig()
{
}


//==============================================================================
//=========================== emAutoplayViewAnimator ===========================
//==============================================================================

emAutoplayViewAnimator::emAutoplayViewAnimator(emView & view)
	: emViewAnimator(view),
	LowPriEngine(*this),
	VisitingVA(view),
	CoreConfig(emCoreConfig::Acquire(view.GetRootContext())),
	Recursive(false),
	Loop(false),
	State(ST_NO_GOAL),
	OneMoreWakeUp(false),
	Backwards(false),
	SkipItemCount(0),
	SkipCurrent(false),
	NextLoopEndless(false),
	CameFrom(CAME_FROM_NONE),
	CurrentPanelState(CP_NOT_VISITED)
{
	VisitingVA.SetMaster(this);
	SetDeactivateWhenIdle();
}


emAutoplayViewAnimator::~emAutoplayViewAnimator()
{
}


void emAutoplayViewAnimator::SetRecursive(bool recursive)
{
	Recursive=recursive;
}


void emAutoplayViewAnimator::SetLoop(bool loop)
{
	Loop=loop;
}


void emAutoplayViewAnimator::SetGoalToItemAt(emPanel * panel)
{
	ClearGoal();
	if (panel) {
		State=ST_UNFINISHED;
		CurrentPanelIdentity=panel->GetIdentity();
		CurrentPanel=panel;
	}
}


void emAutoplayViewAnimator::SetGoalToItemAt(const emString & panelIdentity)
{
	ClearGoal();
	State=ST_UNFINISHED;
	CurrentPanelIdentity=panelIdentity;
}


void emAutoplayViewAnimator::SetGoalToPreviousItemOf(emPanel * panel)
{
	ClearGoal();
	if (panel) {
		State=ST_UNFINISHED;
		Backwards=true;
		SkipCurrent=true;
		CurrentPanelIdentity=panel->GetIdentity();
		CurrentPanel=panel;
	}
}


void emAutoplayViewAnimator::SetGoalToNextItemOf(emPanel * panel)
{
	ClearGoal();
	if (panel) {
		State=ST_UNFINISHED;
		SkipCurrent=true;
		CurrentPanelIdentity=panel->GetIdentity();
		CurrentPanel=panel;
	}
}


void emAutoplayViewAnimator::SkipToPreviousItem()
{
	if (State!=ST_NO_GOAL) {
		State=ST_UNFINISHED;
		if (Backwards) {
			SkipItemCount++;
		}
		else if (SkipItemCount>0) {
			SkipItemCount--;
		}
		else {
			InvertDirection();
		}
	}
}


void emAutoplayViewAnimator::SkipToNextItem()
{
	if (State!=ST_NO_GOAL) {
		State=ST_UNFINISHED;
		if (!Backwards) {
			SkipItemCount++;
		}
		else if (SkipItemCount>0) {
			SkipItemCount--;
		}
		else {
			InvertDirection();
		}
	}
}


void emAutoplayViewAnimator::ClearGoal()
{
	if (State!=ST_NO_GOAL) {
		VisitingVA.Deactivate();
		VisitingVA.ClearGoal();
		State=ST_NO_GOAL;
		OneMoreWakeUp=false;
		Backwards=false;
		SkipItemCount=0;
		SkipCurrent=false;
		NextLoopEndless=false;
		CameFrom=CAME_FROM_NONE;
		CameFromChildName.Clear();
		CurrentPanelIdentity.Clear();
		CurrentPanel=NULL;
		CurrentPanelState=CP_NOT_VISITED;
	}
}


void emAutoplayViewAnimator::Activate()
{
	if (!IsActive()) {
		emViewAnimator::Activate();
	}
}


void emAutoplayViewAnimator::Deactivate()
{
	if (IsActive()) {
		emViewAnimator::Deactivate();
		CurrentPanelState=CP_NOT_VISITED;
		VisitingVA.Deactivate();
		VisitingVA.ClearGoal();
	}
}


bool emAutoplayViewAnimator::CycleAnimation(double dt)
{
	if (State==ST_UNFINISHED) {
		LowPriEngine.WakeUp();
		return true;
	}
	return false;
}


bool emAutoplayViewAnimator::LowPriCycle()
{
	bool readying,reached,oneMoreWakeUpNow;
	AdvanceResult ar;
	emPanel * p;

	oneMoreWakeUpNow=OneMoreWakeUp;
	OneMoreWakeUp=false;

	if (!IsActive() || State!=ST_UNFINISHED) return false;

	if (VisitingVA.HasGoal() && !VisitingVA.IsActive()) {
		reached=VisitingVA.HasReachedGoal();
		VisitingVA.ClearGoal();
		if (CurrentPanelState==CP_VISITING) {
			if (reached) {
				CurrentPanelState=CP_VISITED;
			}
			else {
				CurrentPanelState=CP_NOT_VISITED;
				State=ST_GIVEN_UP;
				return false;
			}
		}
	}

	for (;;) {

		if (!CurrentPanel) {
			CurrentPanel=GetView().GetPanelByIdentity(CurrentPanelIdentity);
		}

		if (!CurrentPanel || !CurrentPanel->IsInViewedPath()) {
			if (CurrentPanelState!=CP_VISITED) break;
			State=ST_GIVEN_UP;
			return false;
		}

		readying=false;
		if (!CurrentPanel->IsContentReady(&readying)) {
			if (CurrentPanelState!=CP_VISITED) break;
			if (readying) {
				if (oneMoreWakeUpNow && !IsTimeSliceAtEnd()) {
					LowPriEngine.WakeUp();
				}
				return true;
			}
		}

		ar=AdvanceCurrentPanel();

		if (ar==AR_FAILED) {
			if (CurrentPanelState!=CP_VISITED) break;
			if (CurrentPanel) GetView().RawVisitFullsized(CurrentPanel);
			State=ST_GIVEN_UP;
			return false;
		}

		if (ar==AR_FINISHED) {
			if (CurrentPanelState!=CP_VISITED) break;
			if (CurrentPanel) GetView().RawVisitFullsized(CurrentPanel);
			State=ST_GOAL_REACHED;
			return false;
		}
	}

	if (CurrentPanelState==CP_NOT_VISITED) {
		VisitingVA.SetAnimParamsByCoreConfig(*CoreConfig);
		if (VisitingVA.IsActive() || VisitingVA.IsAnimated() || !CurrentPanel) {
			VisitingVA.SetGoalFullsized(CurrentPanelIdentity,false);
			VisitingVA.Activate();
			CurrentPanelState=CP_VISITING;
		}
		else {
			if (CurrentPanel->IsFocusable()) {
				CurrentPanel->Activate();
			}
			else {
				p=CurrentPanel;
				while (p->GetParent() && !p->IsFocusable()) p=p->GetParent();
				if (!p->IsInActivePath()) p->Activate();
			}
			GetView().RawVisitFullsized(CurrentPanel);
			CurrentPanelState=CP_VISITED;
			if (!IsTimeSliceAtEnd()) {
				LowPriEngine.WakeUp();
				OneMoreWakeUp=true;
			}
		}
	}

	return true;
}


emAutoplayViewAnimator::AdvanceResult emAutoplayViewAnimator::AdvanceCurrentPanel()
{
	emPanel * p, * c;

	p=CurrentPanel;

	if (!Backwards) {
		if (CameFrom==CAME_FROM_CHILD) {
			c=p->GetChild(CameFromChildName);
			if (!c) return AR_FAILED;
			do {
				c=c->GetNext();
			} while (c && !IsItem(c) && IsCutoff(c));
			if (c) {
				GoChild(c);
				return AR_AGAIN;
			}

			if (!IsCutoff(p)) {
				if (p->GetParent()) {
					GoParent();
					return AR_AGAIN;
				}
				if (Loop && IsItem(p)) {
					NextLoopEndless=false;
					if (!SkipCurrent) {
						if (SkipItemCount<=0) return AR_FINISHED;
						SkipItemCount--;
					}
				}

			}

			if (Loop && !NextLoopEndless) {
				NextLoopEndless=true;

				c=p->GetFirstChild();
				while (c && !IsItem(c) && IsCutoff(c)) {
					c=c->GetNext();
				}
				if (c) {
					GoChild(c);
					return AR_AGAIN;
				}

				GoSame();
				return AR_AGAIN;
			}
		}
		else {
			if (IsItem(p)) {
				NextLoopEndless=false;
				if (!SkipCurrent) {
					if (SkipItemCount<=0) return AR_FINISHED;
					SkipItemCount--;
				}
			}

			if (!IsCutoff(p) || (CameFrom==CAME_FROM_NONE && !IsItem(p))) {
				c=p->GetFirstChild();
				while (c && !IsItem(c) && IsCutoff(c)) {
					c=c->GetNext();
				}
				if (c) {
					GoChild(c);
					return AR_AGAIN;
				}
			}

			if (p->GetParent() && (IsItem(p) || !IsCutoff(p))) {
				GoParent();
				return AR_AGAIN;
			}

			if (Loop && !NextLoopEndless) {
				NextLoopEndless=true;
				GoSame();
				return AR_AGAIN;
			}
		}
	}
	else {
		if (CameFrom==CAME_FROM_CHILD) {
			c=p->GetChild(CameFromChildName);
			if (!c) return AR_FAILED;
			do {
				c=c->GetPrev();
			} while (c && !IsItem(c) && IsCutoff(c));
			if (c) {
				GoChild(c);
				return AR_AGAIN;
			}

			if (!IsCutoff(p)) {
				if (IsItem(p)) {
					NextLoopEndless=false;
					if (!SkipCurrent) {
						if (SkipItemCount<=0) return AR_FINISHED;
						SkipItemCount--;
					}
				}
				if (p->GetParent()) {
					GoParent();
					return AR_AGAIN;
				}
			}

			if (Loop && !NextLoopEndless) {
				NextLoopEndless=true;

				c=p->GetLastChild();
				while (c && !IsItem(c) && IsCutoff(c)) {
					c=c->GetPrev();
				}
				if (c) {
					GoChild(c);
					return AR_AGAIN;
				}

				GoSame();
				return AR_AGAIN;
			}
		}
		else if (CameFrom==CAME_FROM_NONE) {
			if (IsItem(p)) {
				NextLoopEndless=false;
				if (!SkipCurrent) {
					if (SkipItemCount<=0) return AR_FINISHED;
					SkipItemCount--;
				}
			}

			if (p->GetParent() && (IsItem(p) || !IsCutoff(p))) {
				GoParent();
				return AR_AGAIN;
			}

			if (!IsCutoff(p) || !IsItem(p)) {
				c=p->GetLastChild();
				while (c && !IsItem(c) && IsCutoff(c)) {
					c=c->GetPrev();
				}
				if (c) {
					GoChild(c);
					return AR_AGAIN;
				}
			}

			if (Loop && !NextLoopEndless) {
				NextLoopEndless=true;
				GoSame();
				return AR_AGAIN;
			}
		}
		else {
			if (!IsCutoff(p)) {
				c=p->GetLastChild();
				while (c && !IsItem(c) && IsCutoff(c)) {
					c=c->GetPrev();
				}
				if (c) {
					GoChild(c);
					return AR_AGAIN;
				}
			}

			if (IsItem(p)) {
				NextLoopEndless=false;
				if (!SkipCurrent) {
					if (SkipItemCount<=0) return AR_FINISHED;
					SkipItemCount--;
				}
			}

			if (p->GetParent()) {
				GoParent();
				return AR_AGAIN;
			}

			if (Loop && !NextLoopEndless) {
				NextLoopEndless=true;
				GoSame();
				return AR_AGAIN;
			}
		}
	}

	return AR_FAILED;
}


bool emAutoplayViewAnimator::IsItem(const emPanel * p) const
{
	return
		p->IsFocusable() &&
		(p->GetAutoplayHandling()&emPanel::APH_ITEM)!=0
	;
}


bool emAutoplayViewAnimator::IsCutoff(const emPanel * p) const
{
	emPanel::AutoplayHandlingFlags f;
	const emPanel * q;

	f=p->GetAutoplayHandling();
	if ((f&emPanel::APH_CUTOFF)!=0) return true;
	if (p->IsFocusable()) {
		if ((f&emPanel::APH_DIRECTORY)!=0) {
			 if (!Recursive) return true;
		}
		if ((f&emPanel::APH_ITEM)!=0) {
			if (!Recursive) return true;
			for (q=p->GetParent(); q; q=q->GetParent()) {
				f=q->GetAutoplayHandling();
				if ((f&emPanel::APH_CUTOFF_AT_SUBITEMS)!=0) return true;
				if (
					q->IsFocusable() &&
					(f&(emPanel::APH_ITEM|emPanel::APH_DIRECTORY))!=0
				) break;
			}
		}
	}
	return false;
}


void emAutoplayViewAnimator::GoParent()
{
	emPanel * p, * c;

	c=CurrentPanel;
	p=c->GetParent();

	SkipCurrent=false;
	CameFrom=CAME_FROM_CHILD;
	CameFromChildName=c->GetName();
	CurrentPanelIdentity=p->GetIdentity();
	CurrentPanel=p;
	CurrentPanelState=CP_NOT_VISITED;
}


void emAutoplayViewAnimator::GoChild(emPanel * c)
{
	SkipCurrent=false;
	CameFrom=CAME_FROM_PARENT;
	CameFromChildName.Clear();
	CurrentPanelIdentity=c->GetIdentity();
	CurrentPanel=c;
	CurrentPanelState=CP_NOT_VISITED;
}


void emAutoplayViewAnimator::GoSame()
{
	SkipCurrent=false;
}


void emAutoplayViewAnimator::InvertDirection()
{
	emArray<emString> names;
	int cnt;

	Backwards = !Backwards;

	NextLoopEndless=false;

	if (CameFrom==CAME_FROM_PARENT) {
		names=emPanel::DecodeIdentity(CurrentPanelIdentity);
		cnt=names.GetCount();
		if (cnt>0) {
			CameFrom=CAME_FROM_CHILD;
			CameFromChildName=names[cnt-1];
			names.Remove(cnt-1);
			CurrentPanelIdentity=emPanel::EncodeIdentity(names);
			CurrentPanel=GetView().GetPanelByIdentity(CurrentPanelIdentity);
			CurrentPanelState=CP_NOT_VISITED;
		}
	}
	else if (CameFrom==CAME_FROM_CHILD) {
		names=emPanel::DecodeIdentity(CurrentPanelIdentity);
		names.Add(CameFromChildName);
		CameFrom=CAME_FROM_PARENT;
		CameFromChildName.Clear();
		CurrentPanelIdentity=emPanel::EncodeIdentity(names);
		CurrentPanel=GetView().GetPanelByIdentity(CurrentPanelIdentity);
		CurrentPanelState=CP_NOT_VISITED;
	}
}


emAutoplayViewAnimator::LowPriEngineClass::LowPriEngineClass(
	emAutoplayViewAnimator & owner
) :
	emEngine(owner.GetScheduler()),
	Owner(owner)
{
	SetEnginePriority(LOW_PRIORITY);
}


emAutoplayViewAnimator::LowPriEngineClass::~LowPriEngineClass()
{
}


bool emAutoplayViewAnimator::LowPriEngineClass::Cycle()
{
	return Owner.LowPriCycle();
}


//==============================================================================
//============================ emAutoplayViewModel =============================
//==============================================================================

emRef<emAutoplayViewModel> emAutoplayViewModel::Acquire(emView & view)
{
	EM_IMPL_ACQUIRE_COMMON(emAutoplayViewModel,view,"")
}


void emAutoplayViewModel::SetConfigFilePath(const emString & configFilePath)
{
	if (!Config || Config->GetInstallPath()!=configFilePath) {
		Config=emAutoplayConfig::Acquire(GetRootContext(), configFilePath);
		DurationMS=Config->DurationMS;
		ViewAnimator.SetRecursive(Config->Recursive);
		ViewAnimator.SetLoop(Config->Loop);
		LastLocationValid=Config->LastLocationValid;
		LastLocation=Config->LastLocation;
		Signal(ChangeSignal);
	}
}


void emAutoplayViewModel::SetDurationMS(int durationMS)
{
	if (durationMS<0) durationMS=0;
	if (Config && Config->DurationMS!=durationMS) {
		Config->DurationMS=durationMS;
		Config->Save();
	}
	if (DurationMS!=durationMS) {
		DurationMS=durationMS;
		Signal(ChangeSignal);
	}
}


void emAutoplayViewModel::SetRecursive(bool recursive)
{
	if (Config && Config->Recursive!=recursive) {
		Config->Recursive=recursive;
		Config->Save();
	}
	if (ViewAnimator.IsRecursive()!=recursive) {
		ViewAnimator.SetRecursive(recursive);
		Signal(ChangeSignal);
	}
}


void emAutoplayViewModel::SetLoop(bool loop)
{
	if (Config && Config->Loop!=loop) {
		Config->Loop=loop;
		Config->Save();
	}
	if (ViewAnimator.IsLoop()!=loop) {
		ViewAnimator.SetLoop(loop);
		Signal(ChangeSignal);
	}
}


void emAutoplayViewModel::SetAutoplaying(bool autoPlaying)
{
	if (Autoplaying!=autoPlaying) {
		Autoplaying=autoPlaying;
		Signal(ChangeSignal);

		if (Autoplaying) {
			if (!ViewAnimator.HasGoal() || !ViewAnimator.IsActive()) {
				ViewAnimator.SetGoalToItemAt(View.GetActivePanel());
				ViewAnimator.Activate();
				ViewAnimatorStartTime=emGetClockMS();
			}
			WakeUp();
		}
		else {
			StopItemPlaying(false);
			ViewAnimator.ClearGoal();
			ViewAnimator.Deactivate();
		}

		SetScreensaverInhibited(Autoplaying);

		PlayedAnyInCurrentSession=false;
	}
}


bool emAutoplayViewModel::CanContinueLastAutoplay() const
{
	return !Autoplaying && LastLocationValid;
}


void emAutoplayViewModel::ContinueLastAutoplay()
{
	if (CanContinueLastAutoplay()) {
		StopItemPlaying(true);
		ViewAnimator.SetGoalToItemAt(LastLocation);
		ViewAnimator.Activate();
		ViewAnimatorStartTime=emGetClockMS();
		SetAutoplaying(true);
	}
}


void emAutoplayViewModel::SkipToPreviousItem()
{
	emPanel * p;

	if (ViewAnimator.IsActive()) {
		ViewAnimator.SkipToPreviousItem();
	}
	else {
		p=NULL;
		if (Autoplaying) {
			if (PlayingItem) p=PlayingPanel;
			else p=ViewAnimator.GetCurrentPanel();
		}
		if (!p) p=View.GetActivePanel();
		StopItemPlaying(true);
		ViewAnimator.SetGoalToPreviousItemOf(p);
		ViewAnimator.Activate();
		ViewAnimatorStartTime=emGetClockMS();
		WakeUp();
	}
}


void emAutoplayViewModel::SkipToNextItem()
{
	emPanel * p;

	if (ViewAnimator.IsActive()) {
		ViewAnimator.SkipToNextItem();
	}
	else {
		p=NULL;
		if (Autoplaying) {
			if (PlayingItem) p=PlayingPanel;
			else p=ViewAnimator.GetCurrentPanel();
		}
		if (!p) p=View.GetActivePanel();
		StopItemPlaying(true);
		ViewAnimator.SetGoalToNextItemOf(p);
		ViewAnimator.Activate();
		ViewAnimatorStartTime=emGetClockMS();
		WakeUp();
	}
}


void emAutoplayViewModel::Input(emInputEvent & event, const emInputState & state)
{
	switch (event.GetKey()) {
	case EM_KEY_F12:
		if (state.IsNoMod()) {
			SkipToNextItem();
			event.Eat();
		}
		else if (state.IsShiftMod()) {
			SkipToPreviousItem();
			event.Eat();
		}
		else if (state.IsCtrlMod()) {
			SetAutoplaying(!IsAutoplaying());
			event.Eat();
		}
		else if (state.IsShiftCtrlMod()) {
			ContinueLastAutoplay();
			event.Eat();
		}
		break;
	case EM_KEY_BACK_BUTTON:
		if (state.IsNoMod()) {
			View.Focus();
			SkipToPreviousItem();
			event.Eat();
		}
		break;
	case EM_KEY_FORWARD_BUTTON:
		if (state.IsNoMod()) {
			View.Focus();
			SkipToNextItem();
			event.Eat();
		}
		break;
	default:
		break;
	}
}


emAutoplayViewModel::emAutoplayViewModel(emView & view, const emString & name)
	: emModel(view,name),
	View(view),
	DurationMS(5000),
	ViewAnimator(view),
	ViewAnimatorStartTime(0),
	Autoplaying(false),
	LastLocationValid(false),
	ItemProgress(0.0),
	ScreensaverInhibited(false),
	PlayedAnyInCurrentSession(false),
	PlayingItem(false),
	PlaybackActive(false),
	ItemPlayStartTime(0),
	ERectX(0.0),
	ERectY(0.0),
	ERectW(0.0),
	ERectH(0.0)
{
	SetMinCommonLifetime(UINT_MAX);
}


emAutoplayViewModel::~emAutoplayViewModel()
{
	if (ScreensaverInhibited && View.GetWindow()) {
		ScreensaverInhibited=false;
		View.GetWindow()->AllowScreensaver();
	}
}


bool emAutoplayViewModel::Cycle()
{
	UpdateItemPlaying();

	if (ViewAnimator.HasGoal() && !ViewAnimator.IsActive()) {
		if (!Autoplaying) {
			ViewAnimator.ClearGoal();
		}
		else if (ViewAnimator.HasReachedGoal()) {
			if (!ViewAnimator.GetCurrentPanel()) {
				SetAutoplaying(false);
			}
			else {
				SaveLocation(ViewAnimator.GetCurrentPanel());
				StartItemPlaying(ViewAnimator.GetCurrentPanel());
				ViewAnimator.ClearGoal();
			}
		}
		else if (ViewAnimator.HasGivenUp()) {
			if (PlayedAnyInCurrentSession) SaveLocation(NULL);
			SetAutoplaying(false);
		}
		else if (emGetClockMS()-ViewAnimatorStartTime < 1500) {
			ViewAnimator.Activate();
		}
		else {
			SetAutoplaying(false);
		}
	}

	return Autoplaying || ViewAnimator.IsActive();
}


void emAutoplayViewModel::SetItemProgress(double itemProgress)
{
	if (itemProgress<0.0) itemProgress=0.0;
	if (itemProgress>1.0) itemProgress=1.0;

	if (ItemProgress!=itemProgress) {
		ItemProgress=itemProgress;
		Signal(ProgressSignal);
	}
}


void emAutoplayViewModel::StartItemPlaying(emPanel * panel)
{
	double pos,progress;
	bool playing;

	StopItemPlaying(true);

	PlayingItem=true;
	PlayingPanel=panel;
	PlaybackActive=false;
	ItemPlayStartTime=emGetClockMS();

	progress=0.0;

	playing=false;
	pos=0.0;
	if (panel->GetPlaybackState(&playing, &pos)) {
		if (playing) {
			PlaybackActive=true;
			progress=pos;
		}
		else if (PlayedAnyInCurrentSession || pos>=0.999999) {
			if (panel->SetPlaybackState(true, 0.0)) {
				PlaybackActive=true;
				progress=0.0;
			}
		}
		else {
			if (panel->SetPlaybackState(true)) {
				PlaybackActive=true;
				progress=pos;
			}
		}
	}

	SetItemProgress(progress);

	UpdateFullsized(true);

	PlayedAnyInCurrentSession=true;
}


void emAutoplayViewModel::UpdateItemPlaying()
{
	bool playing,done;
	double pos;
	emUInt64 t;

	if (!PlayingItem) return;

	if (!CheckPlayingPanel()) {
		SetAutoplaying(false);
		return;
	}

	if (PlaybackActive) {
		playing=false;
		pos=0.0;
		if (!PlayingPanel->GetPlaybackState(&playing, &pos)) {
			playing=false;
			pos=0.0;
		}
		if (playing) {
			done=false;
		}
		else if (pos<0.00000001) {
			SetAutoplaying(false);
			return;
		}
		else if (pos>0.99999999) {
			done=true;
		}
		else {
			done=false;
		}
		SetScreensaverInhibited(playing || done);
	}
	else {
		t=emGetClockMS()-ItemPlayStartTime;
		done = (t>=(emUInt64)DurationMS);
		if (done || DurationMS<=0) pos=1.0;
		else pos = ((double)t)/DurationMS;
		SetScreensaverInhibited(true);
	}

	SetItemProgress(pos);

	if (!done) UpdateFullsized(false);

	if (
		done && (
			!View.GetActiveAnimator() ||
			dynamic_cast<emMagneticViewAnimator*>(View.GetActiveAnimator())
		)
	) {
		PlaybackActive=false;
		SkipToNextItem();
	}
}


void emAutoplayViewModel::StopItemPlaying(bool resetPos)
{
	if (PlayingItem) {
		PlayingItem=false;
		if (PlayingPanel) {
			if (PlaybackActive) {
				if (resetPos) {
					PlayingPanel->SetPlaybackState(false,0.0);
				}
				else {
					PlayingPanel->SetPlaybackState(false);
				}
			}
			PlayingPanel=NULL;
		}
		PlaybackActive=false;
		SetItemProgress(0.0);
	}
}


bool emAutoplayViewModel::CheckPlayingPanel() const
{
	double ph,vw,vh,vpt,ex,ey,ew,eh,fw,fh,fwe,fhe;
	const emPanel * p, * s;

	p=PlayingPanel;
	if (!p) return false;

	if (!p->IsInViewedPath()) return false;

	if (p->IsActive()) return true;

	ph=p->GetHeight();
	vw=View.GetCurrentWidth();
	vh=View.GetCurrentHeight();
	vpt=View.GetCurrentPixelTallness();

	if (p->IsViewed()) {
		fw=p->GetViewedWidth()/vw;
		fh=p->GetViewedHeight()/vh;
	}
	else {
		s=View.GetSupremeViewedPanel();
		fw=s->GetViewedWidth()/vw;
		do {
			fw/=s->GetLayoutWidth();
			if (fw>1E+9) return false;
			s=s->GetParent();
			if (!s) return false;
		} while (s!=p);
		fh=fw*ph*(vw/vh/vpt);
	}

	p->GetEssenceRect(&ex,&ey,&ew,&eh);
	fwe=fw*ew;
	fhe=fh*(eh/ph);

	if (emMin(emMin(fw,fh),emMin(fwe,fhe)) > 20.0) return false;
	if (emMax(emMax(fw,fh),emMax(fwe,fhe)) < 0.07) return false;

	return true;
}


void emAutoplayViewModel::UpdateFullsized(bool initially)
{
	double x,y,w,h;
	emPanel * p;

	p=PlayingPanel;
	if (!p) return;

	p->GetEssenceRect(&x,&y,&w,&h);
	if (
		initially ||
		fabs(ERectX-x)>0.1 || fabs(ERectY-y)>0.1 ||
		fabs(ERectW-w)>0.1 || fabs(ERectH-h)>0.1
	) {
		ERectX=x;
		ERectY=y;
		ERectW=w;
		ERectH=h;
		View.VisitFullsized(
			p,
			p->IsActive() && View.IsActivationAdherent()
		);
	}
}


void emAutoplayViewModel::SaveLocation(emPanel * panel)
{
	emString l;
	bool v;

	if (panel) {
		v=true;
		l=panel->GetIdentity();
	}
	else {
		v=false;
		l.Clear();
	}

	if (LastLocationValid!=v || LastLocation!=l) {
		LastLocationValid=v;
		LastLocation=l;
		Signal(ChangeSignal);
		if (Config) {
			Config->LastLocationValid=LastLocationValid;
			Config->LastLocation=LastLocation;
			Config->Save();
		}
	}
}


void emAutoplayViewModel::SetScreensaverInhibited(bool inhibited)
{
	if (inhibited) {
		if (!ScreensaverInhibited && View.GetWindow()) {
			ScreensaverInhibited=true;
			View.GetWindow()->InhibitScreensaver();
		}
	}
	else {
		if (ScreensaverInhibited && View.GetWindow()) {
			ScreensaverInhibited=false;
			View.GetWindow()->AllowScreensaver();
		}
	}
}


//==============================================================================
//=========================== emAutoplayControlPanel ===========================
//==============================================================================

emAutoplayControlPanel::emAutoplayControlPanel(
	ParentArg parent, const emString & name,
	emView & contentView
)
	: emPackGroup(parent,name),
	BtAutoplay(NULL),
	BtPrev(NULL),
	BtNext(NULL),
	BtContinueLast(NULL),
	SfDuration(NULL),
	CbRecursive(NULL),
	CbLoop(NULL)
{
	Model=emAutoplayViewModel::Acquire(contentView);
	AddWakeUpSignal(Model->GetChangeSignal());
	AddWakeUpSignal(Model->GetProgressSignal());
	SetCaption("Autoplay");
}


emAutoplayControlPanel::~emAutoplayControlPanel()
{
}


bool emAutoplayControlPanel::Cycle()
{
	if (BtAutoplay && IsSignaled(BtAutoplay->GetCheckSignal())) {
		Model->SetAutoplaying(BtAutoplay->IsChecked());
	}

	if (BtPrev && IsSignaled(BtPrev->GetClickSignal())) {
		Model->SkipToPreviousItem();
	}

	if (BtNext && IsSignaled(BtNext->GetClickSignal())) {
		Model->SkipToNextItem();
	}

	if (BtContinueLast && IsSignaled(BtContinueLast->GetClickSignal())) {
		Model->ContinueLastAutoplay();
	}

	if (SfDuration && IsSignaled(SfDuration->GetValueSignal())) {
		Model->SetDurationMS(DurationValueToMS(SfDuration->GetValue()));
	}

	if (CbRecursive && IsSignaled(CbRecursive->GetCheckSignal())) {
		Model->SetRecursive(CbRecursive->IsChecked());
	}

	if (CbLoop && IsSignaled(CbLoop->GetCheckSignal())) {
		Model->SetLoop(CbLoop->IsChecked());
	}

	if (IsSignaled(Model->GetChangeSignal())) {
		UpdateControls();
	}

	if (IsSignaled(Model->GetProgressSignal())) {
		UpdateProgress();
	}

	return emPackGroup::Cycle();
}


void emAutoplayControlPanel::AutoExpand()
{
	emLinearLayout * lPrevNext;
	emPackLayout * lSettings;

	emPackGroup::AutoExpand();

	SetPrefChildTallness(0,0.7);
	SetPrefChildTallness(1,0.4);
	SetPrefChildTallness(2,0.7);
	SetPrefChildTallness(3,0.4);

	SetChildWeight(0,1.0);
	SetChildWeight(1,0.64);
	SetChildWeight(2,0.17);
	SetChildWeight(3,0.28);

	BtAutoplay=new AutoplayButton(
		this,"autoplay",
		"Autoplay",
		"Start or stop autoplay.\n\n"
		"The autoplay function shows or plays things one after the other. This\n"
		"is useful as a slideshow of picture files or for playing back multiple\n"
		"audio or video files. Autoplay always starts at the focused panel (i.e.\n"
		"the thing you have zoomed in) and follows the visual order.\n"
		"\n"
		"Hotkey: Ctrl+F12",
		emGetInsResImage(GetRootContext(),"emMain","Autoplay.tga")
	);
	BtAutoplay->SetIconAboveCaption();
	BtAutoplay->SetBorderScaling(0.5);
	AddWakeUpSignal(BtAutoplay->GetCheckSignal());

	lPrevNext=new emLinearLayout(this,"prev_next");
	lPrevNext->SetOrientationThresholdTallness(0.7);
		BtPrev=new emButton(
			lPrevNext,"prev",
			"Previous",
			"Skip to previous autoplay item. This also works\n"
			"when autoplay is off, for a manual show.\n"
			"\n"
			"Hotkey: Shift+F12, backward button of the mouse",
			emGetInsResImage(GetRootContext(),"emMain","SkipToPrev.tga")
		);
		BtPrev->SetIconAboveCaption();
		BtPrev->SetBorderScaling(0.5);
		AddWakeUpSignal(BtPrev->GetClickSignal());
		BtNext=new emButton(
			lPrevNext,"next",
			"Next",
			"Skip to next autoplay item. This also works\n"
			"when autoplay is off, for a manual show.\n"
			"\n"
			"Hotkeys: F12, forward button of the mouse",
			emGetInsResImage(GetRootContext(),"emMain","SkipToNext.tga")
		);
		BtNext->SetIconAboveCaption();
		BtNext->SetBorderScaling(0.5);
		AddWakeUpSignal(BtNext->GetClickSignal());

	BtContinueLast=new emButton(
		this,"cont",
		"Continue Last Autoplay",
		"Start autoplay where it has stopped for the last time.\n"
		"\n"
		"Hotkey: Shift+Ctrl+F12",
		emGetInsResImage(GetRootContext(),"emMain","ContinueLastAutoplay.tga")
	);
	BtContinueLast->SetIconAboveCaption();
	BtContinueLast->SetBorderScaling(0.5);
	AddWakeUpSignal(BtContinueLast->GetClickSignal());

	lSettings=new emPackGroup(this,"settings","Autoplay Settings");
	lSettings->SetBorderScaling(1.5);
	lSettings->SetPrefChildTallness(0,0.15);
	lSettings->SetPrefChildTallness(1,0.15);
	lSettings->SetPrefChildTallness(2,0.15);
	lSettings->SetChildWeight(0,1.0);
	lSettings->SetChildWeight(1,0.75);
	lSettings->SetChildWeight(2,0.75);
		SfDuration=new emScalarField(
			lSettings,"duration",
			"Duration",
			"Number of seconds autoplay shall show each\n"
			"item that has no playback function."
		);
		SfDuration->SetEditable();
		SfDuration->SetTextOfValueFunc(DurationTextOfValue,this);
		SfDuration->SetMinMaxValues(0,900);
		SfDuration->SetScaleMarkIntervals(100,20,5,0);
		AddWakeUpSignal(SfDuration->GetValueSignal());
		CbRecursive=new emCheckBox(
			lSettings,"recursive",
			"Recursive",
			"Whether autoplay shall play subdirectories recursively."
		);
		CbRecursive->SetNoEOI();
		AddWakeUpSignal(CbRecursive->GetCheckSignal());
		CbLoop=new emCheckBox(
			lSettings,"loop",
			"Loop",
			"Whether autoplay shall start from the beginning\n"
			"after reaching the end."
		);
		CbLoop->SetNoEOI();
		AddWakeUpSignal(CbLoop->GetCheckSignal());

	UpdateControls();
	UpdateProgress();
}


void emAutoplayControlPanel::AutoShrink()
{
	emPackGroup::AutoShrink();

	BtAutoplay=NULL;
	BtPrev=NULL;
	BtNext=NULL;
	BtContinueLast=NULL;
	SfDuration=NULL;
	CbRecursive=NULL;
	CbLoop=NULL;
}


void emAutoplayControlPanel::UpdateControls()
{
	if (BtAutoplay) {
		BtAutoplay->SetChecked(Model->IsAutoplaying());
	}

	if (BtContinueLast) {
		BtContinueLast->SetEnableSwitch(Model->CanContinueLastAutoplay());
	}

	if (SfDuration) {
		SfDuration->SetValue(DurationMSToValue(Model->GetDurationMS()));
	}

	if (CbRecursive) {
		CbRecursive->SetChecked(Model->IsRecursive());
	}

	if (CbLoop) {
		CbLoop->SetChecked(Model->IsLoop());
	}
}


void emAutoplayControlPanel::UpdateProgress()
{
	if (BtAutoplay) {
		BtAutoplay->SetProgress(Model->GetItemProgress());
	}
}


int emAutoplayControlPanel::DurationValueToMS(emInt64 value)
{
	static const int table[] = {
		500,
		1000,
		2000,
		3000,
		5000,
		10000,
		15000,
		30000,
		60000,
		120000
	};
	static const int size=(int)(sizeof(table)/sizeof(int));
	int i,j;

	i=(int)(value/100);
	j=(int)(value%100);
	if (i<0) return table[0];
	if (i>=size-1) return table[size-1];
	return table[i]+((table[i+1]-table[i])*j+50)/100;
}


emInt64 emAutoplayControlPanel::DurationMSToValue(int ms)
{
	int a,b,i;

	a=0;
	b=900;
	while (a<b) {
		i=(a+b)/2;
		if (DurationValueToMS(i)>=ms) b=i;
		else a=i+1;
	}
	return a;
}


void emAutoplayControlPanel::DurationTextOfValue(
	char * buf, int bufSize, emInt64 value,
	emUInt64 markInterval, void * context
)
{
	double seconds;

	seconds=DurationValueToMS(value)/1000.0;
	snprintf(buf,bufSize,"%g",seconds);
	buf[bufSize-1]=0;
}


emAutoplayControlPanel::AutoplayButton::AutoplayButton(
	ParentArg parent, const emString & name,
	const emString & caption,
	const emString & description,
	const emImage & icon
) :
	emCheckButton(parent,name,caption,description,icon),
	Progress(0.0)
{
}


emAutoplayControlPanel::AutoplayButton::~AutoplayButton()
{
}


void emAutoplayControlPanel::AutoplayButton::SetProgress(double progress)
{
	if (progress<0.0) progress=0.0;
	if (progress>1.0) progress=1.0;
	if (fabs(Progress-progress)>0.001) {
		Progress=progress;
		if (IsChecked()) {
			InvalidatePainting();
		}
	}
}


void emAutoplayControlPanel::AutoplayButton::PaintLabel(
	const emPainter & painter, double x, double y, double w,
	double h, emColor color, emColor canvasColor
) const
{
	double ex,ey,ew,eh,t;

	emCheckButton::PaintLabel(painter,x,y,w,h,color,canvasColor);

	if (IsChecked()) {
		ew=emMin(w,h)*0.72;
		eh=ew;
		ex=x+(w-ew)*0.5;
		ey=y+(h*3/4.257-eh)*0.5;
		t=ew*0.14;
		painter.PaintEllipseOutline(
			ex+ew*0.02,ey+ew*0.02,ew,eh,t,
			emColor(0,0,0,48)
		);
		painter.PaintEllipseOutline(
			ex,ey,ew,eh,
			-90.0+360.0*Progress,
			360.0*(1.0-Progress),t,
			emColor(64,0,64,80)
		);
		painter.PaintEllipseOutline(
			ex,ey,ew,eh,
			-90.0,
			360.0*Progress,t,
			emColor(255,92,255,176)
		);
		painter.PaintTextBoxed(
			ex+ew*0.5-t,ey-t*0.5,t*2,t,
			"Item Progress",t*0.1,
			emColor(32,0,32,64),0,
			EM_ALIGN_CENTER,EM_ALIGN_CENTER,1.0,false
		);
	}
}
