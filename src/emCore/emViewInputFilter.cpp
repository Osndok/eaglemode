//------------------------------------------------------------------------------
// emViewInputFilter.cpp
//
// Copyright (C) 2011-2012,2014-2016 Oliver Hamann.
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

#include <emCore/emViewInputFilter.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emScreen.h>


//==============================================================================
//============================= emViewInputFilter ==============================
//==============================================================================

emViewInputFilter::emViewInputFilter(emView & view, emViewInputFilter * next)
	: emEngine(view.GetScheduler()),
	View(view)
{
	Next=next;
	if (Next) {
		Prev=Next->Prev;
		Next->Prev=this;
	}
	else {
		Prev=View.LastVIF;
		View.LastVIF=this;
	}
	if (Prev) Prev->Next=this;
	else View.FirstVIF=this;
}


emViewInputFilter::~emViewInputFilter()
{
	if (Next) Next->Prev=Prev;
	else View.LastVIF=Prev;
	if (Prev) Prev->Next=Next;
	else View.FirstVIF=Next;
}


double emViewInputFilter::GetTouchEventPriority(double touchX, double touchY) const
{
	return GetForwardTouchEventPriority(touchX,touchY);
}


void emViewInputFilter::Input(emInputEvent & event, const emInputState & state)
{
	ForwardInput(event,state);
}


bool emViewInputFilter::Cycle()
{
	return false;
}


//==============================================================================
//============================ emMouseZoomScrollVIF ============================
//==============================================================================

emMouseZoomScrollVIF::emMouseZoomScrollVIF(emView & view, emViewInputFilter * next)
	: emViewInputFilter(view,next),
	MouseAnim(view),
	WheelAnim(view)
{
	CoreConfig=emCoreConfig::Acquire(view.GetRootContext());
	LastMouseX=0.0;
	LastMouseY=0.0;
	ZoomFixX=0.0;
	ZoomFixY=0.0;
	EmuMidButtonTime=0;
	EmuMidButtonRepeat=0;
	WheelZoomSpeed=0;
	WheelZoomTime=0;
	MagnetismAvoidance=false;
	MagAvMouseMoveX=0.0;
	MagAvMouseMoveY=0.0;
	MagAvTime=0;
}


emMouseZoomScrollVIF::~emMouseZoomScrollVIF()
{
}


void emMouseZoomScrollVIF::Input(emInputEvent & event, const emInputState & state)
{
	double my,mx,dmx,dmy,f;
	emInputState rwstate;
	emPanel * p;

	rwstate=state;

	if (CoreConfig->EmulateMiddleButton) {
		EmulateMiddleButton(event,rwstate);
	}

	if ((GetView().GetViewFlags()&emView::VF_NO_USER_NAVIGATION)!=0) {
		if (MouseAnim.IsActive()) MouseAnim.Deactivate();
		if (WheelAnim.IsActive()) WheelAnim.Deactivate();
		ForwardInput(event,rwstate);
		return;
	}

	mx=rwstate.GetMouseX();
	my=rwstate.GetMouseY();

	if (MouseAnim.IsActive() && MouseAnim.IsGripped()) {
		dmx=mx-LastMouseX;
		dmy=my-LastMouseY;
		UpdateMagnetismAvoidance(dmx,dmy);
		if (!rwstate.GetMiddleButton() || !GetView().IsFocused()) {
			MouseAnim.SetGripped(false);
			MouseAnim.SetDeactivateWhenIdle(true);
			if (!MagnetismAvoidance) {
				GetView().ActivateMagneticViewAnimator();
			}
		}
		else if (fabs(dmx)>0.1 || fabs(dmy)>0.1) {
			if (rwstate.GetCtrl()) {
				f=GetMouseZoomSpeed(rwstate.GetShift());
				MouseAnim.MoveGrip(2,-dmy*f);
				if (CoreConfig->StickMouseWhenNavigating) {
					MoveMousePointer(-dmx,-dmy);
					mx-=dmx;
					my-=dmy;
					rwstate.SetMouse(mx,my);
				}
				ZoomFixX=mx;
			}
			else {
				f=GetMouseScrollSpeed(rwstate.GetShift());
				MouseAnim.MoveGrip(0,dmx*f);
				MouseAnim.MoveGrip(1,dmy*f);
				if (
					CoreConfig->StickMouseWhenNavigating &&
					!CoreConfig->PanFunction
				) {
					MoveMousePointer(-dmx,-dmy);
					mx-=dmx;
					my-=dmy;
					rwstate.SetMouse(mx,my);
				}
				ZoomFixX=mx;
				ZoomFixY=my;
			}
			MouseAnim.SetZoomFixPoint(ZoomFixX,ZoomFixY);
			SetMouseAnimParams();
		}
	}

	switch (event.GetKey()) {
	case EM_KEY_MIDDLE_BUTTON:
		if (!rwstate.GetAlt() && !rwstate.GetMeta()) {
			if (event.GetRepeat()) {
				p=GetView().GetFocusablePanelAt(mx,my,true);
				if (!p) p=GetView().GetRootPanel();
				if (p) GetView().VisitFullsized(p,true,((event.GetRepeat()&1)==0)!=rwstate.GetShift());
			}
			else {
				ZoomFixX=mx;
				ZoomFixY=my;
				InitMagnetismAvoidance();
				MouseAnim.Activate();
				SetMouseAnimParams();
				MouseAnim.SetZoomFixPoint(ZoomFixX,ZoomFixY);
				MouseAnim.SetDeactivateWhenIdle(false);
				MouseAnim.SetGripped(true);
			}
			event.Eat();
		}
		break;
	case EM_KEY_WHEEL_UP:
	case EM_KEY_WHEEL_DOWN:
		if (rwstate.IsNoMod() || rwstate.IsShiftMod()) {
			UpdateWheelZoomSpeed(
				event.GetKey()==EM_KEY_WHEEL_DOWN,
				rwstate.GetShift() || rwstate.Get(EM_KEY_MIDDLE_BUTTON)
			);
			ZoomFixX=mx;
			ZoomFixY=my;
			if (MouseAnim.IsActive() && MouseAnim.IsGripped()) {
				MouseAnim.MoveGrip(2,WheelZoomSpeed/GetView().GetZoomFactorLogarithmPerPixel());
				MouseAnim.SetZoomFixPoint(ZoomFixX,ZoomFixY);
				SetMouseAnimParams();
			}
			else {
				WheelAnim.Activate();
				SetWheelAnimParams();
				WheelAnim.SetDeactivateWhenIdle(false);
				WakeUp();
				WheelAnim.SetGripped(true);
				WheelAnim.SetZoomFixPoint(ZoomFixX,ZoomFixY);
				WheelAnim.MoveGrip(2,WheelZoomSpeed/GetView().GetZoomFactorLogarithmPerPixel());
			}
			event.Eat();
		}
		break;
	default:
		break;
	}

	LastMouseX=mx;
	LastMouseY=my;

	ForwardInput(event,rwstate);
}


bool emMouseZoomScrollVIF::Cycle()
{
	bool busy;

	busy=false;

	if (WheelAnim.IsActive() && WheelAnim.IsGripped()) {
		if (
			WheelAnim.GetAbsVelocity()<10.0 &&
			WheelAnim.GetAbsSpringExtension()<0.5
		) {
			GetView().ActivateMagneticViewAnimator();
		}
		else {
			busy=true;
		}
	}

	return busy;
}


void emMouseZoomScrollVIF::EmulateMiddleButton(emInputEvent & event, emInputState & state)
{
	emUInt64 d;

	// Remember that we have to make sure that the event is not emulated
	// multiple times by nested views. Therefore this condition:
	if (!state.Get(EM_KEY_MIDDLE_BUTTON)) {
		if (
			(event.GetKey()==EM_KEY_ALT || event.GetKey()==EM_KEY_ALT_GR) &&
			event.GetRepeat()==0
		) {
			state.Set(EM_KEY_MIDDLE_BUTTON,true);
			emInputState tmpState(state);
			tmpState.Set(EM_KEY_ALT,false);
			tmpState.Set(EM_KEY_ALT_GR,false);
			d=GetView().GetInputClockMS()-EmuMidButtonTime;
			if (d<330) EmuMidButtonRepeat++;
			else EmuMidButtonRepeat=0;
			EmuMidButtonTime+=d;
			emInputEvent tmpEvent;
			tmpEvent.Setup(EM_KEY_MIDDLE_BUTTON,emString(),EmuMidButtonRepeat,0);
			emMouseZoomScrollVIF::Input(tmpEvent,tmpState);
		}
		else if (state.Get(EM_KEY_ALT) || state.Get(EM_KEY_ALT_GR)) {
			state.Set(EM_KEY_MIDDLE_BUTTON,true);
		}
	}
}


bool emMouseZoomScrollVIF::MoveMousePointerBackIntoView(double * pmx, double * pmy)
{
	//??? This is no longer called :-( (mouse wheel zoom-out on pop-up)
	double cx,cy,cw,ch,mx,my,safety,s;
	bool doMove;

	safety=3.0;
	cx=GetView().GetCurrentX();
	cy=GetView().GetCurrentY();
	cw=GetView().GetCurrentWidth();
	ch=GetView().GetCurrentHeight();
	mx=*pmx;
	my=*pmy;
	s=safety;
	if (s>cw*0.5) s=cw*0.5;
	doMove=false;
	if (mx<cx+s) {
		mx=cx+s;
		doMove=true;
	}
	else if (mx>cx+cw-s) {
		mx=cx+cw-s;
		doMove=true;
	}
	s=safety;
	if (s>ch*0.5) s=ch*0.5;
	if (my<cy+s) {
		my=cy+s;
		doMove=true;
	}
	else if (my>cy+ch-s) {
		my=cy+ch-s;
		doMove=true;
	}
	if (doMove) {
		MoveMousePointer(mx-(*pmx),my-(*pmy));
		*pmx=mx;
		*pmy=my;
	}
	return doMove;
}


void emMouseZoomScrollVIF::MoveMousePointer(double dx, double dy)
{
	emScreen * screen;

	screen=GetView().GetScreen();
	if (!screen) {
		emFatalError(
			"emMouseZoomScrollVIF::MoveMousePointer: No screen interface found."
		);
	}
	screen->MoveMousePointer(dx,dy);
}


double emMouseZoomScrollVIF::GetMouseZoomSpeed(bool fine) const
{
	double f;

	f=CoreConfig->MouseZoomSpeed;
	if (fine) f*=0.1;
	return f*6.0;
}


double emMouseZoomScrollVIF::GetMouseScrollSpeed(bool fine) const
{
	double f;

	f=CoreConfig->MouseScrollSpeed;
	if (fine) f*=0.1;
	if (CoreConfig->PanFunction) f=-f; else f=6.0*f;
	return f;
}


void emMouseZoomScrollVIF::UpdateWheelZoomSpeed(bool down, bool fine)
{
	double dt,newSpeed,a,aMin,t1,t2,f1,f2;
	emUInt64 clk;

	clk=GetView().GetInputClockMS();
	dt=(clk-WheelZoomTime)*0.001;
	WheelZoomTime=clk;

	newSpeed=CoreConfig->MouseWheelZoomSpeed * log(sqrt(2.0));
	if (fine) newSpeed*=0.1;
	if (down) newSpeed=-newSpeed;

	a = CoreConfig->MouseWheelZoomAcceleration;
	aMin = CoreConfig->MouseWheelZoomAcceleration.GetMinValue();
	if (a>aMin*1.0001) {
		t1 = 0.03;
		t2 = 0.35;
		f1 = pow(2.2,a);
		f2 = pow(0.4,a);
		if (newSpeed*WheelZoomSpeed<0.0) dt=t2;
		if (dt<t1) dt=t1;
		if (dt>t2) dt=t2;
		newSpeed *= exp(log(f1) + (log(f2)-log(f1))*(dt-t1)/(t2-t1));
	}

	WheelZoomSpeed=newSpeed;
}


void emMouseZoomScrollVIF::SetMouseAnimParams()
{
	double k,kMin,zflpp;

	k=CoreConfig->KineticZoomingAndScrolling;
	kMin=CoreConfig->KineticZoomingAndScrolling.GetMinValue();
	if (k<kMin*1.0001) {
		k=0.001;
	}
	zflpp=GetView().GetZoomFactorLogarithmPerPixel();
	MouseAnim.SetSpringConstant(2500.0/(k*k));
	MouseAnim.SetFriction(2.0/zflpp/(k*k));
	MouseAnim.SetFrictionEnabled(true);
}


void emMouseZoomScrollVIF::SetWheelAnimParams()
{
	double k,kMin,zflpp;

	k=CoreConfig->KineticZoomingAndScrolling;
	kMin=CoreConfig->KineticZoomingAndScrolling.GetMinValue();
	if (k<kMin*1.0001) {
		k=0.001;
	}
	zflpp=GetView().GetZoomFactorLogarithmPerPixel();
	WheelAnim.SetSpringConstant(480.0/(k*k));
	WheelAnim.SetFriction(2.0/zflpp/(k*k));
	WheelAnim.SetFrictionEnabled(true);
}


void emMouseZoomScrollVIF::InitMagnetismAvoidance()
{
	MagAvMouseMoveX=0.0;
	MagAvMouseMoveY=0.0;
	MagAvTime=GetView().GetInputClockMS();
	MagnetismAvoidance=false;
}


void emMouseZoomScrollVIF::UpdateMagnetismAvoidance(double dmx, double dmy)
{
	static const double MOUSE_HOLD_MAX_MOVE = 2.0;
	static const emUInt64 MOUSE_HOLD_TIME = 750;
	emUInt64 clk;
	double r;

	clk=GetView().GetInputClockMS();
	MagAvMouseMoveX += dmx;
	MagAvMouseMoveY += dmy;
	r=sqrt(MagAvMouseMoveX*MagAvMouseMoveX+MagAvMouseMoveY*MagAvMouseMoveY);
	if (r>MOUSE_HOLD_MAX_MOVE) {
		MagAvMouseMoveX=0.0;
		MagAvMouseMoveY=0.0;
		MagAvTime=clk;
	}
	MagnetismAvoidance = (clk-MagAvTime>=MOUSE_HOLD_TIME);
}


//==============================================================================
//========================== emKeyboardZoomScrollVIF ===========================
//==============================================================================

emKeyboardZoomScrollVIF::emKeyboardZoomScrollVIF(emView & view, emViewInputFilter * next)
	: emViewInputFilter(view,next),
	Animator(view)
{
	CoreConfig=emCoreConfig::Acquire(view.GetRootContext());
	Active=false;
	NavByProgState=0;
}


emKeyboardZoomScrollVIF::~emKeyboardZoomScrollVIF()
{
}


void emKeyboardZoomScrollVIF::Input(emInputEvent & event, const emInputState & state)
{
	double vs,vz,tx,ty,tz;

	if ((GetView().GetViewFlags()&emView::VF_NO_USER_NAVIGATION)!=0) {
		if (Animator.IsActive()) Animator.Deactivate();
		Active=false;
		NavByProgState=0;
		ForwardInput(event,state);
		return;
	}

	NavigateByProgram(event,state);

	if (
		(
			state.IsAltMod() ||
			state.IsShiftAltMod()
		) && (
			event.GetKey()==EM_KEY_CURSOR_LEFT ||
			event.GetKey()==EM_KEY_CURSOR_RIGHT ||
			event.GetKey()==EM_KEY_CURSOR_UP ||
			event.GetKey()==EM_KEY_CURSOR_DOWN ||
			event.GetKey()==EM_KEY_PAGE_DOWN ||
			event.GetKey()==EM_KEY_PAGE_UP
		)
	) {
		Active=true;
		if (!Animator.IsActive()) {
			Animator.Activate();
			Animator.SetDeactivateWhenIdle(false);
		}
	}

	if (!Animator.IsActive()) {
		Active=false;
	}

	if (Active) {
		tx=0.0;
		ty=0.0;
		tz=0.0;
		if (GetView().IsFocused() && state.Get(EM_KEY_ALT)) {
			vs=GetScrollSpeed(state.Get(EM_KEY_SHIFT));
			vz=GetZoomSpeed(state.Get(EM_KEY_SHIFT));
			if (state.Get(EM_KEY_CURSOR_LEFT )) tx-=vs;
			if (state.Get(EM_KEY_CURSOR_RIGHT)) tx+=vs;
			if (state.Get(EM_KEY_CURSOR_UP   )) ty-=vs;
			if (state.Get(EM_KEY_CURSOR_DOWN )) ty+=vs;
			if (state.Get(EM_KEY_PAGE_DOWN   )) tz-=vz;
			if (state.Get(EM_KEY_PAGE_UP     )) tz+=vz;
		}
		SetAnimatorParameters();
		Animator.SetTargetVelocity(0,tx);
		Animator.SetTargetVelocity(1,ty);
		Animator.SetTargetVelocity(2,tz);
		if (Animator.GetAbsTargetVelocity()<0.01) {
			Animator.SetDeactivateWhenIdle(true);
			Active=false;
		}
	}

	ForwardInput(event,state);
}


void emKeyboardZoomScrollVIF::NavigateByProgram(
	emInputEvent & event, const emInputState & state
)
{
	static const double scrollDelta=0.3;
	static const double zoomFac=1.0015;
	double cx,cy,cw,ch,cpt;
	int step;

	// This implements a special key sequence for scrolling and zooming. The
	// sequence is meant to be generated by other programs. It is not useful
	// for control by human. The key sequence consists of three key
	// combinations:
	// 1.) Shift+Alt+End
	// 2.) Shift+Alt+A or Shift+Alt+B or Shift+Alt+C ... or Shift+Alt+Z
	//     This is the strength of the move (A = weakest, Z = strongest).
	// 3.) Shift+Alt+CursorUp|Down|Left|Right or Shift+Alt+PageUp|Down
	//     This is the direction of the operation (scrolling or zooming).

	if (NavByProgState==0) {
		if (event.GetKey()==EM_KEY_END && state.IsShiftAltMod()) {
			NavByProgState=1;
			event.Eat();
		}
	}
	else if (NavByProgState==1) {
		if (event.GetKey()!=EM_KEY_NONE) {
			NavByProgState=0;
			if (state.IsShiftAltMod()) {
				step=((int)event.GetKey())-EM_KEY_A+1;
				if (step>=1 && step<=26) {
					NavByProgState=1+step;
					event.Eat();
				}
			}
		}
	}
	else if (NavByProgState>=2) {
		if (event.GetKey()!=EM_KEY_NONE) {
			step=NavByProgState-1;
			NavByProgState=0;
			if (state.IsShiftAltMod()) {
				cx=GetView().GetCurrentX();
				cy=GetView().GetCurrentY();
				cw=GetView().GetCurrentWidth();
				ch=GetView().GetCurrentHeight();
				cpt=GetView().GetCurrentPixelTallness();
				switch (event.GetKey()) {
				case EM_KEY_CURSOR_LEFT:
					GetView().Scroll(-scrollDelta*step,0.0);
					event.Eat();
					break;
				case EM_KEY_CURSOR_RIGHT:
					GetView().Scroll(scrollDelta*step,0.0);
					event.Eat();
					break;
				case EM_KEY_CURSOR_UP:
					GetView().Scroll(0.0,-scrollDelta*step/cpt);
					event.Eat();
					break;
				case EM_KEY_CURSOR_DOWN:
					GetView().Scroll(0.0,scrollDelta*step/cpt);
					event.Eat();
					break;
				case EM_KEY_PAGE_UP:
					GetView().Zoom(cx+cw*0.5,cy+ch*0.5,pow(zoomFac,step));
					event.Eat();
					break;
				case EM_KEY_PAGE_DOWN:
					GetView().Zoom(cx+cw*0.5,cy+ch*0.5,1.0/pow(zoomFac,step));
					event.Eat();
					break;
				default:
					break;
				}
			}
		}
	}
}


double emKeyboardZoomScrollVIF::GetZoomSpeed(bool fine) const
{
	double f;

	f=CoreConfig->KeyboardZoomSpeed;
	if (fine) f*=0.1;
	f/=GetView().GetZoomFactorLogarithmPerPixel();
	f*=2.0;
	return f;
}


double emKeyboardZoomScrollVIF::GetScrollSpeed(bool fine) const
{
	double f;

	f=CoreConfig->KeyboardScrollSpeed;
	if (fine) f*=0.1;
	f/=GetView().GetZoomFactorLogarithmPerPixel();
	f*=2.0;
	return f;
}


void emKeyboardZoomScrollVIF::SetAnimatorParameters()
{
	static const double tFriction=1.6;
	static const double tAccel=0.6;
	static const double tReverse=0.2;
	double v,k,kMin;

	v=(GetScrollSpeed()+GetZoomSpeed())*0.5;
	k=CoreConfig->KineticZoomingAndScrolling;
	kMin=CoreConfig->KineticZoomingAndScrolling.GetMinValue();
	if (k<kMin*1.0001) {
		k=0.001;
	}
	Animator.CenterZoomFixPoint();
	Animator.SetAcceleration(v/(k*tAccel));
	Animator.SetReverseAcceleration(v/(k*tReverse));
	Animator.SetFriction(v/(k*tFriction));
	Animator.SetFrictionEnabled(true);
}


//==============================================================================
//================================= emCheatVIF =================================
//==============================================================================

emCheatVIF::emCheatVIF(emView & view, emViewInputFilter * next)
	: emViewInputFilter(view,next)
{
	CoreConfig=emCoreConfig::Acquire(view.GetRootContext());
	memset(CheatBuffer,0,sizeof(CheatBuffer));
}


emCheatVIF::~emCheatVIF()
{
}


void emCheatVIF::Input(emInputEvent & event, const emInputState & state)
{
	const char * p, * func;
	emLibHandle lib;
	emString str;
	void * sym;
	size_t sz;

	if ((GetView().GetViewFlags()&emView::VF_NO_USER_NAVIGATION)!=0) {
		ForwardInput(event,state);
		return;
	}

	p=event.GetChars();
	if (!*p) goto L_DONE;
	sz=strlen(p);
	if (sz>sizeof(CheatBuffer)) sz=sizeof(CheatBuffer);
	memmove(CheatBuffer,CheatBuffer+sz,sizeof(CheatBuffer)-sz);
	memcpy(CheatBuffer+sizeof(CheatBuffer)-sz,p,sz);
	p=CheatBuffer+sizeof(CheatBuffer)-1;
	if (*p!='!') goto L_DONE;
	(*(char*)p)=0;
	do { p--; if (p<CheatBuffer || !*p) goto L_DONE; } while (*p!=':');
	func=p+1;
	p=getenv("EM_EASY_CHEATS");
	if (!p || strcasecmp(p,"enabled")!=0) {
		if (func-6<CheatBuffer) goto L_DONE;
		if (memcmp(func-6,"chEat",5)!=0) goto L_DONE;
	}

	// Enable easy cheats for the whole process and even for child processes
	// (no need to type chEat): chEat:easy!
	if (strcmp(func,"easy")==0) {
		putenv((char*)"EM_EASY_CHEATS=enabled");
	}

	// Stress test on/off: chEat:st!
	else if (strcmp(func,"st")==0) {
		GetView().SetViewFlags(GetView().GetViewFlags()^emView::VF_STRESS_TEST);
	}

	// Popup-zoom on/off: chEat:pz!
	else if (strcmp(func,"pz")==0) {
		GetView().SetViewFlags(GetView().GetViewFlags()^emView::VF_POPUP_ZOOM);
	}

	// Ego mode on/off: chEat:egomode!
	else if (strcmp(func,"egomode")==0) {
		GetView().SetViewFlags(GetView().GetViewFlags()^emView::VF_EGO_MODE);
	}

	// StickMouseWhenNavigating on/off: chEat:smwn!
	else if (strcmp(func,"smwn")==0) {
		CoreConfig->StickMouseWhenNavigating.Invert();
		CoreConfig->Save();
	}

	// EmulateMiddleButton on/off: chEat:emb!
	else if (strcmp(func,"emb")==0) {
		CoreConfig->EmulateMiddleButton.Invert();
		CoreConfig->Save();
	}

	// PanFunction on/off: chEat:pan!
	else if (strcmp(func,"pan")==0) {
		CoreConfig->PanFunction.Invert();
		CoreConfig->Save();
	}

	// Tree dump: chEat:td!
	else if (strcmp(func,"td")==0) {
		lib=NULL;
		try {
			lib=emTryOpenLib("emTreeDump",false);
			sym=emTryResolveSymbolFromLib(lib,"emTreeDumpFileFromRootContext");
			if (
				!((bool(*)(emRootContext*,const char *,emString*))sym)(
					&GetView().GetRootContext(),
					emGetInstallPath(EM_IDT_TMP,"emCore","debug.emTreeDump"),
					&str
				)
			) {
				throw emException("%s",str.Get());
			}
		}
		catch (emException & exception) {
			emWarning("%s",exception.GetText());
		}
		if (lib) emCloseLib(lib);
	}

	// Debug log on/off: chEat:dlog!
	else if (strcmp(func,"dlog")==0) {
		emEnableDLog(!emIsDLogEnabled());
	}

#if defined(_WIN32)
	// On Windows, simply press the Print key and find the screenshot in the
	// clipboard.
#else
	// Screenshot: chEat:ss!
	else if (strcmp(func,"ss")==0) {
		emString scPath;
		for (int scNum=0; ; scNum++) {
			scPath=emGetChildPath(
				emGetInstallPath(EM_IDT_TMP,"emCore"),
				emString::Format("emScreenshot%03d.xwd",scNum)
			);
			if (!emIsExistingPath(scPath)) break;
		}
		if (system(emString::Format("xwd -root > %s",scPath.Get()).Get())==-1) {
			emWarning("Could not run xwd: %s",emGetErrorText(errno).Get());
		}
		// Note: Sometimes xwdtopnm produces a black image (seen with
		// Netpbm 10.18.18). Better convert with gimp.
	}
#endif

	// Crash by a segmentation fault: chEat:segfault!
	else if (strcmp(func,"segfault")==0) {
		*(char*)NULL=0;
	}

	// Crash by an arithmetic exception: chEat:divzero!
	else if (strcmp(func,"divzero")==0) {
		emSleepMS(255/func[strlen(func)]);
	}

	// Call emFatalError: chEat:fatal!
	else if (strcmp(func,"fatal")==0) {
		emFatalError("You entered that cheat code!");
	}

	// For application defined cheat codes.
	else GetView().DoCustomCheat(func);


L_DONE:
	ForwardInput(event,state);
}


//==============================================================================
//============================= emDefaultTouchVIF ==============================
//==============================================================================

emDefaultTouchVIF::emDefaultTouchVIF(emView & view, emViewInputFilter * next)
	: emViewInputFilter(view,next)
{
	TouchCount=0;
	TouchesTime=GetView().GetInputClockMS();
	GestureState=0;
}


emDefaultTouchVIF::~emDefaultTouchVIF()
{
}


double emDefaultTouchVIF::GetTouchEventPriority(double touchX, double touchY) const
{
	double pri;

	if ((GetView().GetViewFlags()&emView::VF_NO_USER_NAVIGATION)!=0) {
		pri=2.0;
	}
	else {
		pri=3.0;
	}
	return emMax(pri,GetForwardTouchEventPriority(touchX,touchY));
}


void emDefaultTouchVIF::Input(emInputEvent & event, const emInputState & state)
{
	double pri,priForward;
	int i,j,oldState;

	if (GestureState==0) {
		if (!event.IsTouchEvent() || state.GetTouchCount()<=0) {
			ForwardInput(event,state);
			return;
		}
		if ((GetView().GetViewFlags()&emView::VF_NO_USER_NAVIGATION)!=0) {
			pri=2.0;
		}
		else {
			pri=3.0;
		}
		priForward=GetForwardTouchEventPriority(state.GetTouchX(0),state.GetTouchY(0));
		if (priForward>pri) {
			ForwardInput(event,state);
			return;
		}
		TouchCount=0;
		TouchesTime=GetView().GetInputClockMS();
		WakeUp();
	}

	//???:
	emDLog("emDefaultTouchVIF[%p]::Input:",this);
	for (i=0; i<state.GetTouchCount(); i++) {
		emDLog(
			"  touch: id=%ld x=%g y=%g",
			(long)state.GetTouchId(i),
			state.GetTouchX(i),
			state.GetTouchY(i)
		);
	}
	//:???

	if (event.IsTouchEvent()) event.Eat();

	InputState=state;

	NextTouches();

	for (j=0; j<TouchCount; j++) Touches[j].Down=false;
	for (i=0; i<state.GetTouchCount(); i++) {
		for (j=0; j<TouchCount; j++) {
			if (Touches[j].Id==state.GetTouchId(i)) {
				Touches[j].Down=true;
				Touches[j].X=state.GetTouchX(i);
				Touches[j].Y=state.GetTouchY(i);
				break;
			}
		}
		if (j==TouchCount && j<MAX_TOUCH_COUNT) {
			Touches[j].Id=state.GetTouchId(i);
			Touches[j].MsTotal=0;
			Touches[j].MsSincePrev=0;
			Touches[j].Down=true;
			Touches[j].X=state.GetTouchX(i);
			Touches[j].Y=state.GetTouchY(i);
			Touches[j].PrevDown=false;
			Touches[j].PrevX=state.GetTouchX(i);
			Touches[j].PrevY=state.GetTouchY(i);
			Touches[j].DownX=state.GetTouchX(i);
			Touches[j].DownY=state.GetTouchY(i);
			TouchCount++;
		}
	}

	for (;;) {
		oldState=GestureState;
		DoGesture();
		if (oldState==GestureState) break;
		NextTouches();
	}

	ForwardInput(event,InputState);
}


bool emDefaultTouchVIF::Cycle()
{
	int oldState;

	do {
		oldState=GestureState;
		NextTouches();
		DoGesture();
	} while (oldState!=GestureState);

	return GestureState!=0;
}


void emDefaultTouchVIF::DoGesture()
{
	enum {
		STATE_READY = 0,
		STATE_FIRST_DOWN,
		STATE_SCROLL,
		STATE_ZOOM_IN,
		STATE_ZOOM_OUT,
		STATE_FIRST_DOWN_UP,
		STATE_DOUBLE_DOWN,
		STATE_DOUBLE_DOWN_UP,
		STATE_TRIPLE_DOWN,
		STATE_TRIPLE_DOWN_UP,
		STATE_SECOND_DOWN,
		STATE_EMU_MOUSE_1,
		STATE_EMU_MOUSE_2,
		STATE_EMU_MOUSE_3,
		STATE_EMU_MOUSE_4,
		STATE_THIRD_DOWN,
		STATE_FOURTH_DOWN,
		STATE_FINISH
	};
	emPanel * p;
	double dx,dy;

	switch (GestureState) {
	case STATE_READY:
		if (TouchCount>0) {
			GestureState=STATE_FIRST_DOWN;
		}
		break;
	case STATE_FIRST_DOWN:
		if (TouchCount>1) {
			GestureState=STATE_SECOND_DOWN;
			break;
		}
		if (!Touches[0].Down) {
			GestureState=STATE_FIRST_DOWN_UP;
			break;
		}
		if (GetTotalTouchMove(0)>20.0) {
			GetView().Scroll(-GetTotalTouchMoveX(0),-GetTotalTouchMoveY(0));
			GestureState=STATE_SCROLL;
			break;
		}
		if (Touches[0].MsTotal>250) {
			GestureState=STATE_ZOOM_IN;
			break;
		}
		break;
	case STATE_SCROLL:
		if (!Touches[0].Down) {
			GestureState=STATE_FINISH;
			break;
		}
		GetView().Scroll(-GetTouchMoveX(0),-GetTouchMoveY(0));
		break;
	case STATE_ZOOM_IN:
		if (!Touches[0].Down) {
			GestureState=STATE_FINISH;
			break;
		}
		GetView().Scroll(-GetTouchMoveX(0),-GetTouchMoveY(0));
		GetView().Zoom(Touches[0].X,Touches[0].Y,exp(0.002*Touches[0].MsSincePrev));
		break;
	case STATE_ZOOM_OUT:
		if (!Touches[0].Down) {
			GestureState=STATE_FINISH;
			break;
		}
		GetView().Scroll(-GetTouchMoveX(0),-GetTouchMoveY(0));
		GetView().Zoom(Touches[0].X,Touches[0].Y,exp(-0.002*Touches[0].MsSincePrev));
		break;
	case STATE_FIRST_DOWN_UP:
		if (TouchCount>1) {
			RemoveTouch(0);
			GestureState=STATE_DOUBLE_DOWN;
			break;
		}
		if (Touches[0].MsTotal>250) {
			GestureState=STATE_FINISH;
			break;
		}
		break;
	case STATE_DOUBLE_DOWN:
		if (!Touches[0].Down) {
			GestureState=STATE_DOUBLE_DOWN_UP;
			break;
		}
		if (Touches[0].MsTotal>250) {
			GestureState=STATE_ZOOM_OUT;
			break;
		}
		break;
	case STATE_DOUBLE_DOWN_UP:
		if (TouchCount>1) {
			RemoveTouch(0);
			GestureState=STATE_TRIPLE_DOWN;
			break;
		}
		if (Touches[0].MsTotal>250) {
			p=GetView().GetFocusablePanelAt(Touches[0].X,Touches[0].Y,true);
			if (!p) p=GetView().GetRootPanel();
			if (p) GetView().VisitFullsized(p,true,false);
			GestureState=STATE_FINISH;
			break;
		}
		break;
	case STATE_TRIPLE_DOWN:
		if (!Touches[0].Down) {
			GestureState=STATE_TRIPLE_DOWN_UP;
			break;
		}
		if (Touches[0].MsTotal>250) {
			GestureState=STATE_ZOOM_IN;
			break;
		}
		break;
	case STATE_TRIPLE_DOWN_UP:
		if (TouchCount>1) {
			RemoveTouch(0);
			GestureState=STATE_DOUBLE_DOWN;
			break;
		}
		if (Touches[0].MsTotal>250) {
			p=GetView().GetFocusablePanelAt(Touches[0].X,Touches[0].Y,true);
			if (!p) p=GetView().GetRootPanel();
			if (p) GetView().VisitFullsized(p,true,true);
			GestureState=STATE_FINISH;
			break;
		}
		break;
	case STATE_SECOND_DOWN:
		if (TouchCount>2) {
			GestureState=STATE_THIRD_DOWN;
			break;
		}
		if (Touches[0].MsTotal>250 || !IsAnyTouchDown()) {
			dx=Touches[1].X-Touches[0].X;
			dy=Touches[1].Y-Touches[0].Y;
			if (fabs(dx)>=fabs(dy)) {
				if (dx>0) {
					InputState.SetMouse(Touches[0].X,Touches[0].Y);
					InputState.Set(EM_KEY_LEFT_BUTTON,true);
					InputEvent.Setup(EM_KEY_LEFT_BUTTON,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					GestureState=STATE_EMU_MOUSE_1;
					break;
				}
				else {
					InputState.SetMouse(Touches[0].X,Touches[0].Y);
					InputState.Set(EM_KEY_RIGHT_BUTTON,true);
					InputEvent.Setup(EM_KEY_RIGHT_BUTTON,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					GestureState=STATE_EMU_MOUSE_2;
					break;
				}
			}
			else {
				if (dy>0) {
					InputState.SetMouse(Touches[0].X,Touches[0].Y);
					InputState.Set(EM_KEY_SHIFT,true);
					InputEvent.Setup(EM_KEY_SHIFT,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					InputState.Set(EM_KEY_LEFT_BUTTON,true);
					InputEvent.Setup(EM_KEY_LEFT_BUTTON,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					GestureState=STATE_EMU_MOUSE_3;
					break;
				}
				else {
					InputState.SetMouse(Touches[0].X,Touches[0].Y);
					InputState.Set(EM_KEY_CTRL,true);
					InputEvent.Setup(EM_KEY_CTRL,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					InputState.Set(EM_KEY_LEFT_BUTTON,true);
					InputEvent.Setup(EM_KEY_LEFT_BUTTON,emString(),0,0);
					ForwardInput(InputEvent,InputState);
					GestureState=STATE_EMU_MOUSE_4;
					break;
				}
			}
		}
		break;
	case STATE_EMU_MOUSE_1:
		InputState.SetMouse(Touches[0].X,Touches[0].Y);
		if (!Touches[0].Down) {
			InputState.Set(EM_KEY_LEFT_BUTTON,false);
			InputEvent.Eat();
			ForwardInput(InputEvent,InputState);
			GestureState=STATE_FINISH;
			break;
		}
		InputState.Set(EM_KEY_LEFT_BUTTON,true);
		break;
	case STATE_EMU_MOUSE_2:
		InputState.SetMouse(Touches[0].X,Touches[0].Y);
		if (!Touches[0].Down) {
			InputState.Set(EM_KEY_RIGHT_BUTTON,false);
			InputEvent.Eat();
			ForwardInput(InputEvent,InputState);
			GestureState=STATE_FINISH;
			break;
		}
		InputState.Set(EM_KEY_RIGHT_BUTTON,true);
		break;
	case STATE_EMU_MOUSE_3:
		InputState.SetMouse(Touches[0].X,Touches[0].Y);
		if (!Touches[0].Down) {
			InputState.Set(EM_KEY_SHIFT,false);
			InputState.Set(EM_KEY_LEFT_BUTTON,false);
			InputEvent.Eat();
			ForwardInput(InputEvent,InputState);
			GestureState=STATE_FINISH;
			break;
		}
		InputState.Set(EM_KEY_SHIFT,true);
		InputState.Set(EM_KEY_LEFT_BUTTON,true);
		break;
	case STATE_EMU_MOUSE_4:
		InputState.SetMouse(Touches[0].X,Touches[0].Y);
		if (!Touches[0].Down) {
			InputState.Set(EM_KEY_CTRL,false);
			InputState.Set(EM_KEY_LEFT_BUTTON,false);
			InputEvent.Eat();
			ForwardInput(InputEvent,InputState);
			GestureState=STATE_FINISH;
			break;
		}
		InputState.Set(EM_KEY_CTRL,true);
		InputState.Set(EM_KEY_LEFT_BUTTON,true);
		break;
	case STATE_THIRD_DOWN:
		if (TouchCount>3) {
			GestureState=STATE_FOURTH_DOWN;
			break;
		}
		if (!IsAnyTouchDown()) {
			InputState.Set(EM_KEY_MENU,true);
			InputEvent.Setup(EM_KEY_MENU,emString(),0,0);
			ForwardInput(InputEvent,InputState);
			InputState.Set(EM_KEY_MENU,false);
			ForwardInput(InputEvent,InputState);
			GestureState=STATE_FINISH;
			break;
		}
		break;
	case STATE_FOURTH_DOWN:
		if (TouchCount>4) {
			GestureState=STATE_FINISH;
			break;
		}
		if (!IsAnyTouchDown()) {
			GetView().ShowSoftKeyboard(!GetView().IsSoftKeyboardShown());
			GestureState=STATE_FINISH;
			break;
		}
		break;
	case STATE_FINISH:
		if (!IsAnyTouchDown()) {
			ResetTouches();
			GestureState=STATE_READY;
		}
		break;
	}
}


void emDefaultTouchVIF::ResetTouches()
{
	TouchCount=0;
}


void emDefaultTouchVIF::NextTouches()
{
	emUInt64 t;
	int i,msSincePrev;

	t=GetView().GetInputClockMS();
	msSincePrev=(int)(t-TouchesTime);
	TouchesTime=t;
	for (i=TouchCount-1; i>=0; i--) {
		Touches[i].MsTotal+=msSincePrev;
		Touches[i].MsSincePrev=msSincePrev;
		Touches[i].PrevDown=Touches[i].Down;
		Touches[i].PrevX=Touches[i].X;
		Touches[i].PrevY=Touches[i].Y;
	}
}


void emDefaultTouchVIF::RemoveTouch(int index)
{
	if (index>=0 && index<TouchCount) {
		while (index<TouchCount-1) {
			Touches[index]=Touches[index+1];
			index++;
		}
		TouchCount--;
	}
}


bool emDefaultTouchVIF::IsAnyTouchDown() const
{
	int i;

	for (i=TouchCount-1; i>=0; i--) {
		if (Touches[i].Down) return true;
	}
	return false;
}


double emDefaultTouchVIF::GetTouchMoveX(int index) const
{
	return Touches[index].X-Touches[index].PrevX;
}


double emDefaultTouchVIF::GetTouchMoveY(int index) const
{
	return Touches[index].Y-Touches[index].PrevY;
}


double emDefaultTouchVIF::GetTouchMove(int index) const
{
	double dx,dy;

	dx=GetTouchMoveX(index);
	dy=GetTouchMoveY(index);
	return sqrt(dx*dx+dy*dy);
}


double emDefaultTouchVIF::GetTotalTouchMoveX(int index) const
{
	return Touches[index].X-Touches[index].DownX;
}


double emDefaultTouchVIF::GetTotalTouchMoveY(int index) const
{
	return Touches[index].Y-Touches[index].DownY;
}


double emDefaultTouchVIF::GetTotalTouchMove(int index) const
{
	double dx,dy;

	dx=GetTotalTouchMoveX(index);
	dy=GetTotalTouchMoveY(index);
	return sqrt(dx*dx+dy*dy);
}

