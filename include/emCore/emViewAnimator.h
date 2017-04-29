//------------------------------------------------------------------------------
// emViewAnimator.h
//
// Copyright (C) 2014-2017 Oliver Hamann.
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

#ifndef emViewAnimator_h
#define emViewAnimator_h

#ifndef emView_h
#include <emCore/emView.h>
#endif


//==============================================================================
//=============================== emViewAnimator ===============================
//==============================================================================

class emViewAnimator : public emEngine {

public:

	emViewAnimator(emView & view);
	virtual ~emViewAnimator();

	emView & GetView() const;

	void SetMaster(emViewAnimator * master);
	emViewAnimator * GetMaster() const;
	emViewAnimator * GetActiveSlave() const;

	bool IsActive() const;
	virtual void Activate();
	virtual void Deactivate();

	bool IsDeactivatingWhenIdle() const;
	void SetDeactivateWhenIdle(bool deactivateWhenIdle=true);

	virtual void Input(emInputEvent & event, const emInputState & state);
	virtual void Paint(const emPainter & painter) const;

protected:

	virtual bool Cycle();

	virtual bool CycleAnimation(double dt) = 0;

	void InvalidatePainting();

private:

	emView & View;
	emViewAnimator * Master;
	emViewAnimator * ActiveSlave;
	emViewAnimator * * UpperActivePtr;
	emUInt64 LastTSC;
	emUInt64 LastClk;
	bool DeactivateWhenIdle;
};

inline emView & emViewAnimator::GetView() const
{
	return View;
}

inline emViewAnimator * emViewAnimator::GetMaster() const
{
	return Master;
}

inline emViewAnimator * emViewAnimator::GetActiveSlave() const
{
	return ActiveSlave;
}

inline bool emViewAnimator::IsActive() const
{
	return *UpperActivePtr==this;
}

inline bool emViewAnimator::IsDeactivatingWhenIdle() const
{
	return DeactivateWhenIdle;
}

inline void emViewAnimator::InvalidatePainting()
{
	View.InvalidatePainting();
}


//==============================================================================
//=========================== emKineticViewAnimator ============================
//==============================================================================

class emKineticViewAnimator : public emViewAnimator {

public:

	emKineticViewAnimator(emView & view);
	virtual ~emKineticViewAnimator();

	virtual void Activate();
	virtual void Deactivate();

	double GetVelocity(int dimension) const;
	double GetAbsVelocity() const;
	void SetVelocity(int dimension, double velocity);

	void CenterZoomFixPoint();
	void SetZoomFixPoint(double zoomFixX, double zoomFixY);

	void SetFrictionEnabled(bool enabled);
	bool IsFrictionEnabled() const;
	double GetFriction() const;
	void SetFriction(double friction);

protected:

	virtual bool CycleAnimation(double dt);

private:

	void UpdateBusyState();
	void UpdateZoomFixPoint();

	double Velocity[3];
	bool ZoomFixPointCentered;
	double ZoomFixX,ZoomFixY;
	bool FrictionEnabled;
	double Friction;
	bool Busy;
};

inline double emKineticViewAnimator::GetVelocity(int dimension) const
{
	return Velocity[dimension];
}

inline bool emKineticViewAnimator::IsFrictionEnabled() const
{
	return FrictionEnabled;
}

inline double emKineticViewAnimator::GetFriction() const
{
	return Friction;
}




//==============================================================================
//=========================== emSpeedingViewAnimator ===========================
//==============================================================================

class emSpeedingViewAnimator : public emKineticViewAnimator {

public:

	emSpeedingViewAnimator(emView & view);

	virtual ~emSpeedingViewAnimator();

	virtual void Activate();
	virtual void Deactivate();

	double GetTargetVelocity(int dimension) const;
	double GetAbsTargetVelocity() const;
	void SetTargetVelocity(int dimension, double targetVelocity);

	double GetAcceleration() const;
	void SetAcceleration(double acceleration);

	double GetReverseAcceleration() const;
	void SetReverseAcceleration(double reverseAcceleration);

protected:

	virtual bool CycleAnimation(double dt);

private:

	void UpdateBusyState();

	double TargetVelocity[3];
	double Acceleration;
	double ReverseAcceleration;
	bool Busy;
};

inline double emSpeedingViewAnimator::GetTargetVelocity(int dimension) const
{
	return TargetVelocity[dimension];
}

inline double emSpeedingViewAnimator::GetAcceleration() const
{
	return Acceleration;
}

inline double emSpeedingViewAnimator::GetReverseAcceleration() const
{
	return ReverseAcceleration;
}


//==============================================================================
//=========================== emSwipingViewAnimator ============================
//==============================================================================

class emSwipingViewAnimator : public emKineticViewAnimator {

public:

	emSwipingViewAnimator(emView & view);
	virtual ~emSwipingViewAnimator();

	virtual void Activate();
	virtual void Deactivate();

	bool IsGripped() const;
	void SetGripped(bool gripped);

	void MoveGrip(int dimension, double distance);

	double GetSpringConstant() const;
	void SetSpringConstant(double springConstant);

	double GetAbsSpringExtension() const;

protected:

	virtual bool CycleAnimation(double dt);

private:

	void UpdateBusyState();

	bool Gripped;
	double SpringExtension[3];
	double InstantaneousVelocity[3];
	double SpringConstant;
	bool Busy;
};

inline bool emSwipingViewAnimator::IsGripped() const
{
	return Gripped;
}

inline double emSwipingViewAnimator::GetSpringConstant() const
{
	return SpringConstant;
}


//==============================================================================
//=========================== emMagneticViewAnimator ===========================
//==============================================================================

class emMagneticViewAnimator : public emKineticViewAnimator {

public:

	emMagneticViewAnimator(emView & view);
	virtual ~emMagneticViewAnimator();

	virtual void Activate();
	virtual void Deactivate();

protected:

	virtual bool CycleAnimation(double dt);

private:

	double CalculateDistance(double * pDX, double * pDY, double * pDZ) const;

	void GetViewRect(double * pX, double * pY,
	                 double * pW, double * pH) const;

	emRef<emCoreConfig> CoreConfig;
	bool MagnetismActive;
};


//==============================================================================
//=========================== emVisitingViewAnimator ===========================
//==============================================================================

class emVisitingViewAnimator : public emViewAnimator {

public:

	emVisitingViewAnimator(emView & view);
	virtual ~emVisitingViewAnimator();

	bool IsAnimated() const;
	void SetAnimated(bool animated=true);

	double GetAcceleration() const;
	void SetAcceleration(double acceleration);

	double GetMaxCuspSpeed() const;
	void SetMaxCuspSpeed(double maxCuspSpeed);

	double GetMaxAbsoluteSpeed() const;
	void SetMaxAbsoluteSpeed(double maxAbsoluteSpeed);

	void SetAnimParamsByCoreConfig(const emCoreConfig & coreConfig);

	void SetGoal(const char * identity, bool adherent,
	             const char * subject=NULL);
	void SetGoal(const char * identity, double relX, double relY, double relA,
	             bool adherent, const char * subject=NULL);
	void SetGoalFullsized(const char * identity, bool adherent,
	                      bool utilizeView=false, const char * subject=NULL);
	void ClearGoal();

	bool HasGoal() const;
	bool HasReachedGoal() const;
	bool HasGivenUp() const;

	virtual void Activate();
	virtual void Deactivate();
	virtual void Input(emInputEvent & event, const emInputState & state);
	virtual void Paint(const emPainter & painter) const;

protected:

	virtual bool CycleAnimation(double dt);

private:

	enum StateEnum {
		ST_NO_GOAL,
		ST_CURVE,
		ST_DIRECT,
		ST_SEEK,
		ST_GIVING_UP,
		ST_GIVEN_UP,
		ST_GOAL_REACHED
	};

	enum VisitTypeEnum {
		VT_VISIT,
		VT_VISIT_REL,
		VT_VISIT_FULLSIZED
	};

	struct CurvePoint {
		double X;
		double Z;
	};

	void SetGoal(
		VisitTypeEnum visitType, const char * identity, double relX, double relY,
		double relA, bool adherent, bool utilizeView, const char * subject
	);

	void UpdateSpeed(
		double pos, double dist, int panelsAfter, double distFinal, double dt
	);

	emPanel * GetNearestExistingPanel(
		double * pRelX, double * pRelY, double * pRelA, bool * pAdherent,
		int * pDepth, int * pPanelsAfter, double * pDistFinal
	) const;

	emPanel * GetNearestViewedPanel(emPanel * nearestExistingPanel) const;

	void GetDistanceTo(
		emPanel * panel, double relX, double relY, double relA,
		double * pDirX, double * pDirY, double * pDistXY, double * pDistZ
	) const;

	void GetViewRect(
		double * pX, double * pY, double * pW, double * pH
	) const;

	static double GetDirectDist(double x, double z);

	static void GetDirectPoint(
		double x, double z, double d,
		double * pX, double * pZ
	);

	static void GetCurvePosDist(
		double x, double z, double * pCurvePos, double * pCurveDist
	);

	static CurvePoint GetCurvePoint(double d);

	bool Animated;
	double Acceleration;
	double MaxCuspSpeed;
	double MaxAbsoluteSpeed;
	StateEnum State;
	VisitTypeEnum VisitType;
	emString Identity;
	double RelX,RelY,RelA;
	bool Adherent;
	bool UtilizeView;
	emString Subject;
	emArray<emString> Names;
	int MaxDepthSeen;
	double Speed;
	int TimeSlicesWithoutHope;
	emUInt64 GiveUpClock;

	static const double CurveDeltaDist;
	static const CurvePoint CurvePoints[];
	static const int CurveMaxIndex;
};

inline bool emVisitingViewAnimator::IsAnimated() const
{
	return Animated;
}

inline double emVisitingViewAnimator::GetAcceleration() const
{
	return Acceleration;
}

inline double emVisitingViewAnimator::GetMaxCuspSpeed() const
{
	return MaxCuspSpeed;
}

inline double emVisitingViewAnimator::GetMaxAbsoluteSpeed() const
{
	return MaxAbsoluteSpeed;
}

inline bool emVisitingViewAnimator::HasGoal() const
{
	return State!=ST_NO_GOAL;
}

inline bool emVisitingViewAnimator::HasReachedGoal() const
{
	return State==ST_GOAL_REACHED;
}

inline bool emVisitingViewAnimator::HasGivenUp() const
{
	return State==ST_GIVEN_UP;
}


#endif
