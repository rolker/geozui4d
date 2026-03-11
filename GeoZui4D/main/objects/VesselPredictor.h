// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// VesselPredictor.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class displays a visual representation of a vessel prediction,
 *	involving rudder angle, heading, course over ground, and speed.
 *
 *	Created by mdp 6/15/2005
 */
/*---------------------------------------------------------------------------*/

#ifndef VESSELPREDICTOR_H
#define VESSELPREDICTOR_H

#include "engine/Object.h"
class BasicVessel;

#define VESSEL_PREDICTOR_MAX_LOOKAHEAD 10

class VesselPredictor : public Object
{
  public:
    VesselPredictor(BasicVessel *v = NULL, int historyLength = 40);
	DERIVE_OBJECT(VesselPredictor);

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);

	void setInfo(double t, float rudder, float heading, float cog, float sog, float curdir = 0, float curstr = 0);
	void setParentVessel(BasicVessel *v) {parentVessel = v;}

	int setLookahead(int newLA = -1);
	int showRegularPredictors(int showThem = -1) {if (showThem > -1) showRegPredictors = showThem; return showRegPredictors;}

	float fudge;

  protected:
	struct dynamics_snapshot {
		float rudder;
		float rudder_minus_one;
		float heading;
		float heading_ang_vel;
		float ctw;
		float ctw_ang_vel;
		float ctw_ang_acc;
		float stw;
		float currentDir;
		float currentStr;
		double t;
	} *history;
	int endIndex, tMinusOneIndex, histLen;
	bool indexWrap, renderingPhantom;

	BasicVessel *parentVessel;
	int lookAhead;
	int showRegPredictors;

	enum {HeadingDelta = 0, cogDelta};
	enum {FirstParam = 0, RudderMinusTwo = 0, RudderMinusOne, Rudder, Inst_HAV, Const, Est_COGv, /*Est_COGa,*/ nParams};
	float predictionParameters[VESSEL_PREDICTOR_MAX_LOOKAHEAD][2][7]; //1-VESSEL_PREDICTOR_MAX_LOOKAHEAD seconds out, {Heading and COG}, 7 parameters
};

#endif
