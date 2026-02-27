/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// VesselPredictor.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class displays a visual representation of a vessel prediction,
 *	involving rudder angle, heading, course over ground, and speed.
 *
 *	Created by mdp 6/15/2005
 */
/*---------------------------------------------------------------------------*/
#include "VesselPredictor.h"
#include "BasicVessel.h"
#include "engine/PosableStateBase.h"
#include <iostream>
using namespace std;

#define LEFT_RUDDER -30
#define RIGHT_RUDDER 30
#define RUDDER_STEP 15

REGISTER_OBJECT(VesselPredictor, "");

/*---------------------------------------------------------------------------*/
VesselPredictor::VesselPredictor(BasicVessel *v, int historyLength)
{
	parentVessel = v;
	history = new dynamics_snapshot[historyLength];
	endIndex = tMinusOneIndex = -1;
	histLen = historyLength;
	indexWrap = false;
	renderingPhantom = false;

	fudge = 1.0f; //0.55f;
	lookAhead = VESSEL_PREDICTOR_MAX_LOOKAHEAD;
	showRegPredictors = 0;

	//Values from recitified (final_analysis) 7/6/2005 run (sog between 4.5 and 7.0 kts, no EstCOGv's of 0)
	//Rsquare = .896
	predictionParameters[0][HeadingDelta][RudderMinusTwo] =  0.077f;
	predictionParameters[0][HeadingDelta][RudderMinusOne] = -0.038f;
	predictionParameters[0][HeadingDelta][Rudder] =			 0.042f;
	predictionParameters[0][HeadingDelta][Inst_HAV] =		 0.736f;
	predictionParameters[0][HeadingDelta][Const] =			-0.155f;
	//Rsquare = .937
	predictionParameters[1][HeadingDelta][RudderMinusTwo] =  0.143f;
	predictionParameters[1][HeadingDelta][RudderMinusOne] = -0.072f;
	predictionParameters[1][HeadingDelta][Rudder] =			 0.132f;
	predictionParameters[1][HeadingDelta][Inst_HAV] =		 1.373f;
	predictionParameters[1][HeadingDelta][Const] =			-0.469f;
	//Rsquare = .943
	predictionParameters[2][HeadingDelta][RudderMinusTwo] =  0.175f;
	predictionParameters[2][HeadingDelta][RudderMinusOne] = -0.081f;
	predictionParameters[2][HeadingDelta][Rudder] =			 0.247f;
	predictionParameters[2][HeadingDelta][Inst_HAV] =		 1.932f;
	predictionParameters[2][HeadingDelta][Const] =			-0.759f;
	//Rsquare = .942
	predictionParameters[3][HeadingDelta][RudderMinusTwo] =  0.211f;
	predictionParameters[3][HeadingDelta][RudderMinusOne] = -0.131f;
	predictionParameters[3][HeadingDelta][Rudder] =			 0.431f;
	predictionParameters[3][HeadingDelta][Inst_HAV] =		 2.387f;
	predictionParameters[3][HeadingDelta][Const] =			-1.183f;
	//Rsquare = .937
	predictionParameters[4][HeadingDelta][RudderMinusTwo] =  0.276f;
	predictionParameters[4][HeadingDelta][RudderMinusOne] = -0.245f;
	predictionParameters[4][HeadingDelta][Rudder] =			 0.662f;
	predictionParameters[4][HeadingDelta][Inst_HAV] =		 2.777f;
	predictionParameters[4][HeadingDelta][Const] =			-1.574f;
	//Rsquare = .927
	predictionParameters[5][HeadingDelta][RudderMinusTwo] =  0.350f;
	predictionParameters[5][HeadingDelta][RudderMinusOne] = -0.414f;
	predictionParameters[5][HeadingDelta][Rudder] =			 0.951f;
	predictionParameters[5][HeadingDelta][Inst_HAV] =		 3.089f;
	predictionParameters[5][HeadingDelta][Const] =			-2.048f;
	//Rsquare = .915
	predictionParameters[6][HeadingDelta][RudderMinusTwo] =  0.420f;
	predictionParameters[6][HeadingDelta][RudderMinusOne] = -0.571f;
	predictionParameters[6][HeadingDelta][Rudder] =			 1.225f;
	predictionParameters[6][HeadingDelta][Inst_HAV] =		 3.375f;
	predictionParameters[6][HeadingDelta][Const] =			-2.454f;
	//Rsquare = .900
	predictionParameters[7][HeadingDelta][RudderMinusTwo] =  0.492f;
	predictionParameters[7][HeadingDelta][RudderMinusOne] = -0.717f;
	predictionParameters[7][HeadingDelta][Rudder] =			 1.491f;
	predictionParameters[7][HeadingDelta][Inst_HAV] =		 3.595f;
	predictionParameters[7][HeadingDelta][Const] =			-2.923f;
	//Rsquare = .884
	predictionParameters[8][HeadingDelta][RudderMinusTwo] =  0.585f;
	predictionParameters[8][HeadingDelta][RudderMinusOne] = -0.893f;
	predictionParameters[8][HeadingDelta][Rudder] =			 1.763f;
	predictionParameters[8][HeadingDelta][Inst_HAV] =		 3.782f;
	predictionParameters[8][HeadingDelta][Const] =			-3.345f;
	//Rsquare = .864
	predictionParameters[9][HeadingDelta][RudderMinusTwo] =  0.664f;
	predictionParameters[9][HeadingDelta][RudderMinusOne] = -1.030f;
	predictionParameters[9][HeadingDelta][Rudder] =			 2.006f;
	predictionParameters[9][HeadingDelta][Inst_HAV] =		 3.914f;
	predictionParameters[9][HeadingDelta][Const] =			-3.824f;

	//Values from recitified (final_analysis) 7/6/2005 run (sog between 4.5 and 7.0 kts, no EstCOGv's of 0)
	//Rsquare = 0.931
	predictionParameters[0][cogDelta][RudderMinusTwo] = -0.031f;
	predictionParameters[0][cogDelta][RudderMinusOne] =  0.065f;
	predictionParameters[0][cogDelta][Rudder] =			-0.051f;
	predictionParameters[0][cogDelta][Inst_HAV] =		 0.410f;
	predictionParameters[0][cogDelta][Est_COGv] =		 0.293f;
	predictionParameters[0][cogDelta][Const] =			 0.039f;
	//Rsquare = .946
	predictionParameters[1][cogDelta][RudderMinusTwo] = -0.035f;
	predictionParameters[1][cogDelta][RudderMinusOne] =  0.119f;
	predictionParameters[1][cogDelta][Rudder] =			-0.086f;
	predictionParameters[1][cogDelta][Inst_HAV] =		 0.790f;
	predictionParameters[1][cogDelta][Est_COGv] =		 0.565f;
	predictionParameters[1][cogDelta][Const] =			 0.000f;
	//Rsquare = .957
	predictionParameters[2][cogDelta][RudderMinusTwo] = -0.010f;
	predictionParameters[2][cogDelta][RudderMinusOne] =  0.141f;
	predictionParameters[2][cogDelta][Rudder] =			-0.089f;
	predictionParameters[2][cogDelta][Inst_HAV] =		 1.173f;
	predictionParameters[2][cogDelta][Est_COGv] =		 0.799f;
	predictionParameters[2][cogDelta][Const] =			-0.111f;
	//Rsquare = .964
	predictionParameters[3][cogDelta][RudderMinusTwo] =  0.042f;
	predictionParameters[3][cogDelta][RudderMinusOne] =  0.129f;
	predictionParameters[3][cogDelta][Rudder] =			-0.052f;
	predictionParameters[3][cogDelta][Inst_HAV] =		 1.548f;
	predictionParameters[3][cogDelta][Est_COGv] =		 0.992f;
	predictionParameters[3][cogDelta][Const] =			-0.300f;
	//Rsquare = .969
	predictionParameters[4][cogDelta][RudderMinusTwo] =  0.118f;
	predictionParameters[4][cogDelta][RudderMinusOne] =  0.081f;
	predictionParameters[4][cogDelta][Rudder] =			 0.025f;
	predictionParameters[4][cogDelta][Inst_HAV] =		 1.922f;
	predictionParameters[4][cogDelta][Est_COGv] =		 1.139f;
	predictionParameters[4][cogDelta][Const] =			-0.556f;
	//Rsquare = .970
	predictionParameters[5][cogDelta][RudderMinusTwo] =  0.214f;
	predictionParameters[5][cogDelta][RudderMinusOne] =  0.001f;
	predictionParameters[5][cogDelta][Rudder] =			 0.142f;
	predictionParameters[5][cogDelta][Inst_HAV] =		 2.281f;
	predictionParameters[5][cogDelta][Est_COGv] =		 1.243f;
	predictionParameters[5][cogDelta][Const] =			-0.885f;
	//Rsquare = .968
	predictionParameters[6][cogDelta][RudderMinusTwo] =  0.321f;
	predictionParameters[6][cogDelta][RudderMinusOne] = -0.098f;
	predictionParameters[6][cogDelta][Rudder] =			 0.292f;
	predictionParameters[6][cogDelta][Inst_HAV] =		 2.631f;
	predictionParameters[6][cogDelta][Est_COGv] =		 1.305f;
	predictionParameters[6][cogDelta][Const] =			-1.273f;
	//Rsquare = .964
	predictionParameters[7][cogDelta][RudderMinusTwo] =  0.428f;
	predictionParameters[7][cogDelta][RudderMinusOne] = -0.200f;
	predictionParameters[7][cogDelta][Rudder] =			 0.467f;
	predictionParameters[7][cogDelta][Inst_HAV] =		 2.937f;
	predictionParameters[7][cogDelta][Est_COGv] =		 1.340f;
	predictionParameters[7][cogDelta][Const] =			-1.718f;
	//Rsquare = .958
	predictionParameters[8][cogDelta][RudderMinusTwo] =  0.523f;
	predictionParameters[8][cogDelta][RudderMinusOne] = -0.298f;
	predictionParameters[8][cogDelta][Rudder] =			 0.667f;
	predictionParameters[8][cogDelta][Inst_HAV] =		 3.203f;
	predictionParameters[8][cogDelta][Est_COGv] =		 1.358f;
	predictionParameters[8][cogDelta][Const] =			-2.200f;
	//Rsquare = .950
	predictionParameters[9][cogDelta][RudderMinusTwo] =  0.610f;
	predictionParameters[9][cogDelta][RudderMinusOne] = -0.409f;
	predictionParameters[9][cogDelta][Rudder] =			 0.896f;
	predictionParameters[9][cogDelta][Inst_HAV] =		 3.417f;
	predictionParameters[9][cogDelta][Est_COGv] =		 1.367f;
	predictionParameters[9][cogDelta][Const] =			-2.708f;
	/**/

	/*
	//Values from 7/6/2005 run (sog between 4.5 and 7.0 kts)
	//Rsquare = .875
	predictionParameters[0][HeadingDelta][RudderMinusTwo] =  0.072f;
	predictionParameters[0][HeadingDelta][RudderMinusOne] = -0.014f;
	predictionParameters[0][HeadingDelta][Rudder] =			 0.031f;
	predictionParameters[0][HeadingDelta][Inst_HAV] =		 0.710f;
	predictionParameters[0][HeadingDelta][Const] =			-0.218f;
	//Rsquare = .924
	predictionParameters[1][HeadingDelta][RudderMinusTwo] =  0.115f;
	predictionParameters[1][HeadingDelta][RudderMinusOne] = -0.002f;
	predictionParameters[1][HeadingDelta][Rudder] =			 0.089f;
	predictionParameters[1][HeadingDelta][Inst_HAV] =		 1.356f;
	predictionParameters[1][HeadingDelta][Const] =			-0.491f;
	//Rsquare = .932
	predictionParameters[2][HeadingDelta][RudderMinusTwo] =  0.157f;
	predictionParameters[2][HeadingDelta][RudderMinusOne] = -0.014f;
	predictionParameters[2][HeadingDelta][Rudder] =			 0.206f;
	predictionParameters[2][HeadingDelta][Inst_HAV] =		 1.892f;
	predictionParameters[2][HeadingDelta][Const] =			-0.842f;
	//Rsquare = .931
	predictionParameters[3][HeadingDelta][RudderMinusTwo] =  1.182f;
	predictionParameters[3][HeadingDelta][RudderMinusOne] = -0.041f;
	predictionParameters[3][HeadingDelta][Rudder] =			 0.371f;
	predictionParameters[3][HeadingDelta][Inst_HAV] =		 2.359f;
	predictionParameters[3][HeadingDelta][Const] =			-1.234f;
	//Rsquare = .925
	predictionParameters[4][HeadingDelta][RudderMinusTwo] =  0.229f;
	predictionParameters[4][HeadingDelta][RudderMinusOne] = -0.137f;
	predictionParameters[4][HeadingDelta][Rudder] =			 0.601f;
	predictionParameters[4][HeadingDelta][Inst_HAV] =		 2.751f;
	predictionParameters[4][HeadingDelta][Const] =			-1.667f;
	//Rsquare = .914
	predictionParameters[5][HeadingDelta][RudderMinusTwo] =  0.277f;
	predictionParameters[5][HeadingDelta][RudderMinusOne] = -0.260f;
	predictionParameters[5][HeadingDelta][Rudder] =			 0.861f;
	predictionParameters[5][HeadingDelta][Inst_HAV] =		 3.089f;
	predictionParameters[5][HeadingDelta][Const] =			-2.112f;
	//Rsquare = .900
	predictionParameters[6][HeadingDelta][RudderMinusTwo] =  0.353f;
	predictionParameters[6][HeadingDelta][RudderMinusOne] = -0.435f;
	predictionParameters[6][HeadingDelta][Rudder] =			 1.151f;
	predictionParameters[6][HeadingDelta][Inst_HAV] =		 3.366f;
	predictionParameters[6][HeadingDelta][Const] =			-2.579f;
	//Rsquare = .883
	predictionParameters[7][HeadingDelta][RudderMinusTwo] =  0.420f;
	predictionParameters[7][HeadingDelta][RudderMinusOne] = -0.587f;
	predictionParameters[7][HeadingDelta][Rudder] =			 1.426f;
	predictionParameters[7][HeadingDelta][Inst_HAV] =		 3.605f;
	predictionParameters[7][HeadingDelta][Const] =			-3.041f;
	//Rsquare = .864
	predictionParameters[8][HeadingDelta][RudderMinusTwo] =  0.508f;
	predictionParameters[8][HeadingDelta][RudderMinusOne] = -0.768f;
	predictionParameters[8][HeadingDelta][Rudder] =			 1.711f;
	predictionParameters[8][HeadingDelta][Inst_HAV] =		 3.789f;
	predictionParameters[8][HeadingDelta][Const] =			-3.521f;
	//Rsquare = .842
	predictionParameters[9][HeadingDelta][RudderMinusTwo] =  0.596f;
	predictionParameters[9][HeadingDelta][RudderMinusOne] = -0.943f;
	predictionParameters[9][HeadingDelta][Rudder] =			 1.986f;
	predictionParameters[9][HeadingDelta][Inst_HAV] =		 3.931f;
	predictionParameters[9][HeadingDelta][Const] =			-4.002f;
	*/

	/*
	//Values from 8/2/2005 run (generally at survey speed, sog >= 3.0kts, without Est_COGa)
	//Rsquare = 0.680
	predictionParameters[0][cogDelta][RudderMinusTwo] = -0.280f;
	predictionParameters[0][cogDelta][RudderMinusOne] =  0.108f;
	predictionParameters[0][cogDelta][Rudder] =			 0.141f;
	predictionParameters[0][cogDelta][Inst_HAV] =		-0.546f;
	predictionParameters[0][cogDelta][Est_COGv] =		 1.506f;
	predictionParameters[0][cogDelta][Const] =			 0.082f;
	//Rsquare = .791
	predictionParameters[1][cogDelta][RudderMinusTwo] = -0.523f;
	predictionParameters[1][cogDelta][RudderMinusOne] =  0.180f;
	predictionParameters[1][cogDelta][Rudder] =			 0.287f;
	predictionParameters[1][cogDelta][Inst_HAV] =		-0.926f;
	predictionParameters[1][cogDelta][Est_COGv] =		 2.867f;
	predictionParameters[1][cogDelta][Const] =			 0.153f;
	//Rsquare = .838
	predictionParameters[2][cogDelta][RudderMinusTwo] = -0.739f;
	predictionParameters[2][cogDelta][RudderMinusOne] =  0.243f;
	predictionParameters[2][cogDelta][Rudder] =			 0.434f;
	predictionParameters[2][cogDelta][Inst_HAV] =		-1.221f;
	predictionParameters[2][cogDelta][Est_COGv] =		 4.120f;
	predictionParameters[2][cogDelta][Const] =			 0.166f;
	//Rsquare = .870
	predictionParameters[3][cogDelta][RudderMinusTwo] = -0.993f;
	predictionParameters[3][cogDelta][RudderMinusOne] =  0.286f;
	predictionParameters[3][cogDelta][Rudder] =			 0.588f;
	predictionParameters[3][cogDelta][Inst_HAV] =		-1.356f;
	predictionParameters[3][cogDelta][Est_COGv] =		 5.207f;
	predictionParameters[3][cogDelta][Const] =			 0.152f;
	//Rsquare = .896
	predictionParameters[4][cogDelta][RudderMinusTwo] = -1.078f;
	predictionParameters[4][cogDelta][RudderMinusOne] =  0.280f;
	predictionParameters[4][cogDelta][Rudder] =			 0.778f;
	predictionParameters[4][cogDelta][Inst_HAV] =		-1.431f;
	predictionParameters[4][cogDelta][Est_COGv] =		 6.165f;
	predictionParameters[4][cogDelta][Const] =			 0.044f;
	//Rsquare = .915
	predictionParameters[5][cogDelta][RudderMinusTwo] = -1.185f;
	predictionParameters[5][cogDelta][RudderMinusOne] =  0.235f;
	predictionParameters[5][cogDelta][Rudder] =			 0.986f;
	predictionParameters[5][cogDelta][Inst_HAV] =		-1.397f;
	predictionParameters[5][cogDelta][Est_COGv] =		 6.959f;
	predictionParameters[5][cogDelta][Const] =			-0.129f;
	//Rsquare = .929
	predictionParameters[6][cogDelta][RudderMinusTwo] = -1.249f;
	predictionParameters[6][cogDelta][RudderMinusOne] =  0.158f;
	predictionParameters[6][cogDelta][Rudder] =			 1.212f;
	predictionParameters[6][cogDelta][Inst_HAV] =		-1.302f;
	predictionParameters[6][cogDelta][Est_COGv] =		 7.594f;
	predictionParameters[6][cogDelta][Const] =			-0.400f;
	//Rsquare = .938
	predictionParameters[7][cogDelta][RudderMinusTwo] = -1.292f;
	predictionParameters[7][cogDelta][RudderMinusOne] =  0.062f;
	predictionParameters[7][cogDelta][Rudder] =			 1.459f;
	predictionParameters[7][cogDelta][Inst_HAV] =		-1.155f;
	predictionParameters[7][cogDelta][Est_COGv] =		 8.099f;
	predictionParameters[7][cogDelta][Const] =			-0.743f;
	//Rsquare = .942
	predictionParameters[8][cogDelta][RudderMinusTwo] = -1.322f;
	predictionParameters[8][cogDelta][RudderMinusOne] = -0.059f;
	predictionParameters[8][cogDelta][Rudder] =			 1.724f;
	predictionParameters[8][cogDelta][Inst_HAV] =		-0.910f;
	predictionParameters[8][cogDelta][Est_COGv] =		 8.450f;
	predictionParameters[8][cogDelta][Const] =			-1.116f;
	//Rsquare = .940
	predictionParameters[9][cogDelta][RudderMinusTwo] = -1.326f;
	predictionParameters[9][cogDelta][RudderMinusOne] = -0.201f;
	predictionParameters[9][cogDelta][Rudder] =			 2.024f;
	predictionParameters[9][cogDelta][Inst_HAV] =		-0.817f;
	predictionParameters[9][cogDelta][Est_COGv] =		 8.807f;
	predictionParameters[9][cogDelta][Const] =			-1.631f;
*/
	/*
	//Values from 8/2/2005 run (generally at survey speed, sog >= 3.0kts, not as good as 7/6 run)
	//Rsquare = 0.118
	predictionParameters[0][cogDelta][RudderMinusTwo] = -0.230f;
	predictionParameters[0][cogDelta][RudderMinusOne] =  0.328f;
	predictionParameters[0][cogDelta][Rudder] =			-0.015f;
	predictionParameters[0][cogDelta][Inst_HAV] =		-0.543f;
	predictionParameters[0][cogDelta][Est_COGv] =		 1.257f;
	predictionParameters[0][cogDelta][Est_COGa] =		-0.998f;
	predictionParameters[0][cogDelta][Const] =			 0.224f;
	//Rsquare = .334
	predictionParameters[1][cogDelta][RudderMinusTwo] = -0.426f;
	predictionParameters[1][cogDelta][RudderMinusOne] =  0.481f;
	predictionParameters[1][cogDelta][Rudder] =			 0.065f;
	predictionParameters[1][cogDelta][Inst_HAV] =		-0.820f;
	predictionParameters[1][cogDelta][Est_COGv] =		 2.396f;
	predictionParameters[1][cogDelta][Est_COGa] =		-1.632f;
	predictionParameters[1][cogDelta][Const] =			-0.307f;
	//Rsquare = .513
	predictionParameters[2][cogDelta][RudderMinusTwo] = -0.626f;
	predictionParameters[2][cogDelta][RudderMinusOne] =  0.670f;
	predictionParameters[2][cogDelta][Rudder] =			 0.099f;
	predictionParameters[2][cogDelta][Inst_HAV] =		-0.889f;
	predictionParameters[2][cogDelta][Est_COGv] =		 3.364f;
	predictionParameters[2][cogDelta][Est_COGa] =		-2.324f;
	predictionParameters[2][cogDelta][Const] =			-0.379f;
	//Rsquare = .638
	predictionParameters[3][cogDelta][RudderMinusTwo] = -0.718f;
	predictionParameters[3][cogDelta][RudderMinusOne] =  0.728f;
	predictionParameters[3][cogDelta][Rudder] =			 0.222f;
	predictionParameters[3][cogDelta][Inst_HAV] =		-1.041f;
	predictionParameters[3][cogDelta][Est_COGv] =		 4.285f;
	predictionParameters[3][cogDelta][Est_COGa] =		-2.896f;
	predictionParameters[3][cogDelta][Const] =			-0.623f;
	//Rsquare = .725
	predictionParameters[4][cogDelta][RudderMinusTwo] = -0.912f;
	predictionParameters[4][cogDelta][RudderMinusOne] =  0.893f;
	predictionParameters[4][cogDelta][Rudder] =			 0.299f;
	predictionParameters[4][cogDelta][Inst_HAV] =		-0.997f;
	predictionParameters[4][cogDelta][Est_COGv] =		 5.107f;
	predictionParameters[4][cogDelta][Est_COGa] =		-3.321f;
	predictionParameters[4][cogDelta][Const] =			-0.758f;
	//Rsquare = .786
	predictionParameters[5][cogDelta][RudderMinusTwo] = -0.979f;
	predictionParameters[5][cogDelta][RudderMinusOne] =  0.826f;
	predictionParameters[5][cogDelta][Rudder] =			 0.491f;
	predictionParameters[5][cogDelta][Inst_HAV] =		-0.798f;
	predictionParameters[5][cogDelta][Est_COGv] =		 5.714f;
	predictionParameters[5][cogDelta][Est_COGa] =		-3.607f;
	predictionParameters[5][cogDelta][Const] =			-0.943f;
	//Rsquare = .826
	predictionParameters[6][cogDelta][RudderMinusTwo] = -1.031f;
	predictionParameters[6][cogDelta][RudderMinusOne] =  0.782f;
	predictionParameters[6][cogDelta][Rudder] =			 0.693f;
	predictionParameters[6][cogDelta][Inst_HAV] =		-0.732f;
	predictionParameters[6][cogDelta][Est_COGv] =		 6.332f;
	predictionParameters[6][cogDelta][Est_COGa] =		-3.769f;
	predictionParameters[6][cogDelta][Const] =			-1.272f;
	//Rsquare = .854
	predictionParameters[7][cogDelta][RudderMinusTwo] = -1.116f;
	predictionParameters[7][cogDelta][RudderMinusOne] =  0.731f;
	predictionParameters[7][cogDelta][Rudder] =			 0.914f;
	predictionParameters[7][cogDelta][Inst_HAV] =		-0.506f;
	predictionParameters[7][cogDelta][Est_COGv] =		 6.815f;
	predictionParameters[7][cogDelta][Est_COGa] =		-3.794f;
	predictionParameters[7][cogDelta][Const] =			-1.545f;
	//Rsquare = .870
	predictionParameters[8][cogDelta][RudderMinusTwo] = -1.127f;
	predictionParameters[8][cogDelta][RudderMinusOne] =  0.527f;
	predictionParameters[8][cogDelta][Rudder] =			 1.208f;
	predictionParameters[8][cogDelta][Inst_HAV] =		-0.029f;
	predictionParameters[8][cogDelta][Est_COGv] =		 7.022f;
	predictionParameters[8][cogDelta][Est_COGa] =		-3.724f;
	predictionParameters[8][cogDelta][Const] =			-1.817f;
	//Rsquare = .871
	predictionParameters[9][cogDelta][RudderMinusTwo] = -1.091f;
	predictionParameters[9][cogDelta][RudderMinusOne] =  0.359f;
	predictionParameters[9][cogDelta][Rudder] =			 1.514f;
	predictionParameters[9][cogDelta][Inst_HAV] =		 0.019f;
	predictionParameters[9][cogDelta][Est_COGv] =		 7.367f;
	predictionParameters[9][cogDelta][Est_COGa] =		-3.775f;
	predictionParameters[9][cogDelta][Const] =			-2.364f;
	*/

	/*
	//Values from 7/6/2005 run (sog between 4.5 and 7.0 kts)
	//Rsquare = 0.936
	predictionParameters[0][cogDelta][RudderMinusTwo] =  0.008f;
	predictionParameters[0][cogDelta][RudderMinusOne] =  0.019f;
	predictionParameters[0][cogDelta][Rudder] =			-0.029f;
	predictionParameters[0][cogDelta][Inst_HAV] =		 0.377f;
	predictionParameters[0][cogDelta][Est_COGv] =		 0.588f;
	predictionParameters[0][cogDelta][Est_COGa] =		-0.294f;
	predictionParameters[0][cogDelta][Const] =			 0.000f;
	//Rsquare = .949
	predictionParameters[1][cogDelta][RudderMinusTwo] =  0.034f;
	predictionParameters[1][cogDelta][RudderMinusOne] =  0.033f;
	predictionParameters[1][cogDelta][Rudder] =			-0.046f;
	predictionParameters[1][cogDelta][Inst_HAV] =		 0.764f;
	predictionParameters[1][cogDelta][Est_COGv] =		 1.109f;
	predictionParameters[1][cogDelta][Est_COGa] =		-0.553f;
	predictionParameters[1][cogDelta][Const] =			-0.062f;
	//Rsquare = .957
	predictionParameters[2][cogDelta][RudderMinusTwo] =  0.077f;
	predictionParameters[2][cogDelta][RudderMinusOne] =  0.033f;
	predictionParameters[2][cogDelta][Rudder] =			-0.039f;
	predictionParameters[2][cogDelta][Inst_HAV] =		 1.172f;
	predictionParameters[2][cogDelta][Est_COGv] =		 1.542f;
	predictionParameters[2][cogDelta][Est_COGa] =		-0.766f;
	predictionParameters[2][cogDelta][Const] =			-0.189f;
	//Rsquare = .962
	predictionParameters[3][cogDelta][RudderMinusTwo] =  0.136f;
	predictionParameters[3][cogDelta][RudderMinusOne] =  0.016f;
	predictionParameters[3][cogDelta][Rudder] =			-0.004f;
	predictionParameters[3][cogDelta][Inst_HAV] =		 1.596f;
	predictionParameters[3][cogDelta][Est_COGv] =		 1.880f;
	predictionParameters[3][cogDelta][Est_COGa] =		-0.932f;
	predictionParameters[3][cogDelta][Const] =			-0.379f;
	//Rsquare = .964
	predictionParameters[4][cogDelta][RudderMinusTwo] =  0.212f;
	predictionParameters[4][cogDelta][RudderMinusOne] = -0.025f;
	predictionParameters[4][cogDelta][Rudder] =			 0.065f;
	predictionParameters[4][cogDelta][Inst_HAV] =		 2.024f;
	predictionParameters[4][cogDelta][Est_COGv] =		 2.124f;
	predictionParameters[4][cogDelta][Est_COGa] =		-1.049f;
	predictionParameters[4][cogDelta][Const] =			-0.636f;
	//Rsquare = .963
	predictionParameters[5][cogDelta][RudderMinusTwo] =  0.299f;
	predictionParameters[5][cogDelta][RudderMinusOne] = -0.090f;
	predictionParameters[5][cogDelta][Rudder] =			 0.171f;
	predictionParameters[5][cogDelta][Inst_HAV] =		 2.451f;
	predictionParameters[5][cogDelta][Est_COGv] =		 2.276f;
	predictionParameters[5][cogDelta][Est_COGa] =		-1.120f;
	predictionParameters[5][cogDelta][Const] =			-0.955f;
	//Rsquare = .959
	predictionParameters[6][cogDelta][RudderMinusTwo] =  0.395f;
	predictionParameters[6][cogDelta][RudderMinusOne] = -0.171f;
	predictionParameters[6][cogDelta][Rudder] =			 0.309f;
	predictionParameters[6][cogDelta][Inst_HAV] =		 2.859f;
	predictionParameters[6][cogDelta][Est_COGv] =		 2.352f;
	predictionParameters[6][cogDelta][Est_COGa] =		-1.155f;
	predictionParameters[6][cogDelta][Const] =			-1.335f;
	//Rsquare = .954
	predictionParameters[7][cogDelta][RudderMinusTwo] =  0.492f;
	predictionParameters[7][cogDelta][RudderMinusOne] = -0.259f;
	predictionParameters[7][cogDelta][Rudder] =			 0.475f;
	predictionParameters[7][cogDelta][Inst_HAV] =		 3.216f;
	predictionParameters[7][cogDelta][Est_COGv] =		 2.385f;
	predictionParameters[7][cogDelta][Est_COGa] =		-1.168f;
	predictionParameters[7][cogDelta][Const] =			-1.770f;
	//Rsquare = .946
	predictionParameters[8][cogDelta][RudderMinusTwo] =  0.576f;
	predictionParameters[8][cogDelta][RudderMinusOne] = -0.346f;
	predictionParameters[8][cogDelta][Rudder] =			 0.669f;
	predictionParameters[8][cogDelta][Inst_HAV] =		 3.529f;
	predictionParameters[8][cogDelta][Est_COGv] =		 2.385f;
	predictionParameters[8][cogDelta][Est_COGa] =		-1.165f;
	predictionParameters[8][cogDelta][Const] =			-2.245f;
	//Rsquare = .936
	predictionParameters[9][cogDelta][RudderMinusTwo] =  0.654f;
	predictionParameters[9][cogDelta][RudderMinusOne] = -0.451f;
	predictionParameters[9][cogDelta][Rudder] =			 0.898f;
	predictionParameters[9][cogDelta][Inst_HAV] =		 3.790f;
	predictionParameters[9][cogDelta][Est_COGv] =		 2.369f;
	predictionParameters[9][cogDelta][Est_COGa] =		-1.155f;
	predictionParameters[9][cogDelta][Const] =			-2.750f;
	*/

	/* Older values (early July 2005)
	predictionParameters[0][HeadingDelta][RudderMinusTwo] =  0.183f;
	predictionParameters[0][HeadingDelta][RudderMinusOne] = -0.028f;
	predictionParameters[0][HeadingDelta][Rudder] =			 0.074f;
	predictionParameters[0][HeadingDelta][Inst_HAV] =		 0.502f;
	predictionParameters[0][HeadingDelta][Const] =			-0.774f;
	predictionParameters[1][HeadingDelta][RudderMinusTwo] =  0.525f;
	predictionParameters[1][HeadingDelta][RudderMinusOne] = -0.133f;
	predictionParameters[1][HeadingDelta][Rudder] =			 0.265f;
	predictionParameters[1][HeadingDelta][Inst_HAV] =		 0.569f;
	predictionParameters[1][HeadingDelta][Const] =			-2.221f;
	predictionParameters[2][HeadingDelta][RudderMinusTwo] =  0.835f;
	predictionParameters[2][HeadingDelta][RudderMinusOne] = -0.281f;
	predictionParameters[2][HeadingDelta][Rudder] =			 0.534f;
	predictionParameters[2][HeadingDelta][Inst_HAV] =		 0.636f;
	predictionParameters[2][HeadingDelta][Const] =			-3.678f;
	predictionParameters[3][HeadingDelta][RudderMinusTwo] =  1.120f;
	predictionParameters[3][HeadingDelta][RudderMinusOne] = -0.451f;
	predictionParameters[3][HeadingDelta][Rudder] =			 0.859f;
	predictionParameters[3][HeadingDelta][Inst_HAV] =		 0.682f;
	predictionParameters[3][HeadingDelta][Const] =			-5.170f;
	predictionParameters[4][HeadingDelta][RudderMinusTwo] =  1.386f;
	predictionParameters[4][HeadingDelta][RudderMinusOne] = -0.665f;
	predictionParameters[4][HeadingDelta][Rudder] =			 1.246f;
	predictionParameters[4][HeadingDelta][Inst_HAV] =		 0.734f;
	predictionParameters[4][HeadingDelta][Const] =			-6.649f;
	predictionParameters[5][HeadingDelta][RudderMinusTwo] =  1.635f;
	predictionParameters[5][HeadingDelta][RudderMinusOne] = -0.882f;
	predictionParameters[5][HeadingDelta][Rudder] =			 1.662f;
	predictionParameters[5][HeadingDelta][Inst_HAV] =		 0.758f;
	predictionParameters[5][HeadingDelta][Const] =			-8.160f;
	predictionParameters[6][HeadingDelta][RudderMinusTwo] =  1.874f;
	predictionParameters[6][HeadingDelta][RudderMinusOne] = -1.134f;
	predictionParameters[6][HeadingDelta][Rudder] =			 2.123f;
	predictionParameters[6][HeadingDelta][Inst_HAV] =		 0.780f;
	predictionParameters[6][HeadingDelta][Const] =			-9.663f;
	predictionParameters[7][HeadingDelta][RudderMinusTwo] =  2.088f;
	predictionParameters[7][HeadingDelta][RudderMinusOne] = -1.391f;
	predictionParameters[7][HeadingDelta][Rudder] =			 2.603f;
	predictionParameters[7][HeadingDelta][Inst_HAV] =		 0.821f;
	predictionParameters[7][HeadingDelta][Const] =			-11.121f;
	predictionParameters[8][HeadingDelta][RudderMinusTwo] =  2.293f;
	predictionParameters[8][HeadingDelta][RudderMinusOne] = -1.667f;
	predictionParameters[8][HeadingDelta][Rudder] =			 3.105f;
	predictionParameters[8][HeadingDelta][Inst_HAV] =		 0.866f;
	predictionParameters[8][HeadingDelta][Const] =			-12.556f;
	predictionParameters[9][HeadingDelta][RudderMinusTwo] =  2.482f;
	predictionParameters[9][HeadingDelta][RudderMinusOne] = -1.933f;
	predictionParameters[9][HeadingDelta][Rudder] =			 3.614f;
	predictionParameters[9][HeadingDelta][Inst_HAV] =		 0.900f;
	predictionParameters[9][HeadingDelta][Const] =			-13.989f;

	predictionParameters[0][cogDelta][RudderMinusTwo] =  0.332f;
	predictionParameters[0][cogDelta][RudderMinusOne] = -0.211f;
	predictionParameters[0][cogDelta][Rudder] =			 0.091f;
	predictionParameters[0][cogDelta][Inst_HAV] =		 0.021f;
	predictionParameters[0][cogDelta][Est_COGv] =		 0.567f;
	predictionParameters[0][cogDelta][Est_COGa] =		-0.748f;
	predictionParameters[0][cogDelta][Const] =			-0.672f;
	predictionParameters[1][cogDelta][RudderMinusTwo] =  0.555f;
	predictionParameters[1][cogDelta][RudderMinusOne] = -0.255f;
	predictionParameters[1][cogDelta][Rudder] =			 0.097f;
	predictionParameters[1][cogDelta][Inst_HAV] =		 0.031f;
	predictionParameters[1][cogDelta][Est_COGv] =		 1.201f;
	predictionParameters[1][cogDelta][Est_COGa] =		-0.613f;
	predictionParameters[1][cogDelta][Const] =			-1.268f;
	predictionParameters[2][cogDelta][RudderMinusTwo] =  0.857f;
	predictionParameters[2][cogDelta][RudderMinusOne] = -0.309f;
	predictionParameters[2][cogDelta][Rudder] =			 0.129f;
	predictionParameters[2][cogDelta][Inst_HAV] =		 0.043f;
	predictionParameters[2][cogDelta][Est_COGv] =		 1.667f;
	predictionParameters[2][cogDelta][Est_COGa] =		-1.293f;
	predictionParameters[2][cogDelta][Const] =			-2.168f;
	predictionParameters[3][cogDelta][RudderMinusTwo] =  1.001f;
	predictionParameters[3][cogDelta][RudderMinusOne] = -0.221f;
	predictionParameters[3][cogDelta][Rudder] =			 0.179f;
	predictionParameters[3][cogDelta][Inst_HAV] =		 0.053f;
	predictionParameters[3][cogDelta][Est_COGv] =		 2.126f;
	predictionParameters[3][cogDelta][Est_COGa] =		-1.076f;
	predictionParameters[3][cogDelta][Const] =			-3.079f;
	predictionParameters[4][cogDelta][RudderMinusTwo] =  1.174f;
	predictionParameters[4][cogDelta][RudderMinusOne] = -0.205f;
	predictionParameters[4][cogDelta][Rudder] =			 0.359f;
	predictionParameters[4][cogDelta][Inst_HAV] =		 0.067f;
	predictionParameters[4][cogDelta][Est_COGv] =		 2.425f;
	predictionParameters[4][cogDelta][Est_COGa] =		-1.664f;
	predictionParameters[4][cogDelta][Const] =			-4.261f;
	predictionParameters[5][cogDelta][RudderMinusTwo] =  1.207f;
	predictionParameters[5][cogDelta][RudderMinusOne] = -0.214f;
	predictionParameters[5][cogDelta][Rudder] =			 0.645f;
	predictionParameters[5][cogDelta][Inst_HAV] =		 0.073f;
	predictionParameters[5][cogDelta][Est_COGv] =		 2.836f;
	predictionParameters[5][cogDelta][Est_COGa] =		-1.440f;
	predictionParameters[5][cogDelta][Const] =			-5.264f;
	predictionParameters[6][cogDelta][RudderMinusTwo] =  1.359f;
	predictionParameters[6][cogDelta][RudderMinusOne] = -0.371f;
	predictionParameters[6][cogDelta][Rudder] =			 1.051f;
	predictionParameters[6][cogDelta][Inst_HAV] =		 0.087f;
	predictionParameters[6][cogDelta][Est_COGv] =		 3.061f;
	predictionParameters[6][cogDelta][Est_COGa] =		-1.968f;
	predictionParameters[6][cogDelta][Const] =			-6.543f;
	predictionParameters[7][cogDelta][RudderMinusTwo] =  1.427f;
	predictionParameters[7][cogDelta][RudderMinusOne] = -0.547f;
	predictionParameters[7][cogDelta][Rudder] =			 1.522f;
	predictionParameters[7][cogDelta][Inst_HAV] =		 0.101f;
	predictionParameters[7][cogDelta][Est_COGv] =		 3.347f;
	predictionParameters[7][cogDelta][Est_COGa] =		-1.697f;
	predictionParameters[7][cogDelta][Const] =			-7.696f;
	predictionParameters[8][cogDelta][RudderMinusTwo] =  1.520f;
	predictionParameters[8][cogDelta][RudderMinusOne] = -0.720f;
	predictionParameters[8][cogDelta][Rudder] =			 1.982f;
	predictionParameters[8][cogDelta][Inst_HAV] =		 0.114f;
	predictionParameters[8][cogDelta][Est_COGv] =		 3.593f;
	predictionParameters[8][cogDelta][Est_COGa] =		-2.234f;
	predictionParameters[8][cogDelta][Const] =			-8.910f;
	predictionParameters[9][cogDelta][RudderMinusTwo] =  1.564f;
	predictionParameters[9][cogDelta][RudderMinusOne] = -0.956f;
	predictionParameters[9][cogDelta][Rudder] =			 2.511f;
	predictionParameters[9][cogDelta][Inst_HAV] =		 0.121f;
	predictionParameters[9][cogDelta][Est_COGv] =		 3.917f;
	predictionParameters[9][cogDelta][Est_COGa] =		-1.986f;
	predictionParameters[9][cogDelta][Const] =			-9.977f;
	*/
}

/*---------------------------------------------------------------------------*/
int VesselPredictor::setLookahead(int newLA)
{
	if (newLA > VESSEL_PREDICTOR_MAX_LOOKAHEAD)
		lookAhead = VESSEL_PREDICTOR_MAX_LOOKAHEAD;
	else if (newLA >= 0)
		lookAhead = newLA;

	return lookAhead;
}

/*---------------------------------------------------------------------------*/
void VesselPredictor::setInfo(double t, float rudder, float heading, float cog, float sog, float curdir, float curstr)
{
	dynamics_snapshot *current, *tMinusOne = NULL;
	float vel, origX, origY, curX, curY, newX, newY, theta;
	double prev_t;

	if (endIndex <= -1)
		endIndex = 0;
	else if (history[endIndex].t < t)
	{
		endIndex++;

		if (endIndex >= histLen)
		{
			endIndex = 0;
			indexWrap = true;
		}
	}

	current = &history[endIndex];
	current->rudder = rudder;
	current->heading = heading;

	//determine how current affects course and speed through water
	if (curstr == 0.0)
	{
		current->ctw = cog;
		current->stw = sog;
	}
	else
	{
		theta = (float) (cog * M_PI / 180.0);
		origX = sog * sin(theta);
		origY = sog * cos(theta);
		theta = (float) (curdir * M_PI / 180.0);
		curX = curstr * sin(theta);
		curY = curstr * cos(theta);
		newX = origX - curX;
		newY = origY - curY;
		current->ctw = (float)(atan2(newX, newY) * 180.0 / M_PI);
		current->stw = (float)sqrt(newX * newX + newY * newY);
	}

	current->t = t;
	current->currentDir = curdir;
	current->currentStr = curstr;
	if (indexWrap || (endIndex > 1))
	{
		//Figure out which index is one second back
		if (tMinusOneIndex < 0)
			tMinusOneIndex = 0;
		prev_t = history[tMinusOneIndex].t; //this is done in case we don't go through the loop at all
		while (history[tMinusOneIndex].t < t - 1)
		{
			prev_t = history[tMinusOneIndex].t;
			tMinusOneIndex++;
			if (tMinusOneIndex >= histLen)
				tMinusOneIndex = 0;
		}
		if ((prev_t < t) && (t - prev_t < 3)) //if the previous index was in the ballpark, use it.
		{
			tMinusOneIndex--;
			if (tMinusOneIndex < 0)
				tMinusOneIndex = histLen - 1;
		}
		tMinusOne = &history[tMinusOneIndex];
	}
	if (tMinusOne)//calculate velocities and accelerations
	{
		vel = heading - tMinusOne->heading;
		if (vel > 180)
			vel -= 360;
		else if (vel < -180)
			vel += 360;
		current->heading_ang_vel = vel;
		vel = current->ctw - tMinusOne->ctw;
		if (vel > 180)
			vel -= 360;
		else if (vel < -180)
			vel += 360;
		current->ctw_ang_vel = vel;
		vel = vel - tMinusOne->ctw_ang_vel;
		if (vel > 180)
			vel -= 360;
		else if (vel < -180)
			vel += 360;
		current->ctw_ang_acc = vel;
		current->rudder_minus_one = tMinusOne->rudder;
	}
}

/*---------------------------------------------------------------------------*/
/** Draw the predictor lines relative to data collection center
 */
void VesselPredictor::DrawLowRes(float degradation, Window3D *win)
{
	float speedFactor, *params;
	dynamics_snapshot *current, *tMinusOne = NULL;
	float val[7], theta, ctheta, finalX = 0, finalY = 0, strength, heading, ctw, courseDelta;
	int finalIndex = lookAhead - 1;
	int i, j, rud, colorFlag = 0;
//	GLboolean tailState, CMGstate;
	float currentX, currentY; //vector components for direction of current

	if (endIndex < 0 || renderingPhantom)
		return;
	current = &history[endIndex];
	if (tMinusOneIndex >= 0)
		tMinusOne = &history[tMinusOneIndex];

	//grab necessary info
	heading = current->heading;
	ctw = current->ctw;
	courseDelta = ctw - heading;
	if (courseDelta > 180)
		courseDelta -= 360;
	if (courseDelta < -180)
		courseDelta += 360;
	val[Rudder] = current->rudder;
	val[Inst_HAV] = current->heading_ang_vel;
	val[Est_COGv] = current->ctw_ang_vel;
/*	val[Est_COGa] = current->ctw_ang_acc;*/
	if (tMinusOne)
	{
		val[RudderMinusTwo] = tMinusOne->rudder_minus_one;
		val[RudderMinusOne] = tMinusOne->rudder;
	}
	else
	{
		val[RudderMinusTwo] = 0;
		val[RudderMinusOne] = 0;
	}
	speedFactor = current->stw;
	if (!speedFactor)
		return;


	ctheta = (float)((current->currentDir - heading) * M_PI / 180.0); //since we're already rotated by vessel heading, have to subtract it back out.
	strength = current->currentStr;
	currentX = (float)(strength * sin(ctheta));
	currentY = (float)(strength * cos(ctheta));

	//Draw predictor line
    glDisable(GL_LIGHTING);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(0.9f, 0.9f, 0.5f);
	glLineWidth(5.0);

	if(lookAhead)
	{
	glBegin(GL_LINE_STRIP);
		glVertex3f(0.0, 0.0, 0.0);
		for (i = 0; i < lookAhead; i++)
		{
			params = predictionParameters[i][cogDelta];
			theta = params[Const];
			for (j = FirstParam; j < nParams; j++)
			{
				if (j == Const)
					j++;
				theta += params[j] * val[j];
			}
			theta *= fudge; //fudge factor
			theta += courseDelta;
			theta *= (float)(M_PI / 180.0);

			//contributions below: model for vessel and contribution of current
			//glVertex3f(finalX = (float)((i + 1) * (speedFactor * sin(theta) + currentX)),
			//		   finalY = (float)((i + 1) * (speedFactor * cos(theta) + currentY)), 0.0);
			glVertex3f(finalX = (float)(finalX + (speedFactor * sin(theta) + currentX)),
					   finalY = (float)(finalY + (speedFactor * cos(theta) + currentY)), 0.0);

		}
	glEnd();
	}

	if (showRegPredictors && lookAhead)
	{
		//Draw alternate predictor lines
		glLineWidth(1.0);
		for (rud = LEFT_RUDDER; rud <= RIGHT_RUDDER; rud += RUDDER_STEP)
		{
			colorFlag = 1 - colorFlag;
			glColor3f(colorFlag? 0.7f : 0.9f, 0.7f, colorFlag? 0.9f : 0.7f);
			val[Rudder] = rud;
			val[RudderMinusTwo] = rud;
			val[RudderMinusOne] = rud;
			glBegin(GL_LINE_STRIP);
				glVertex3f(0.0, 0.0, 0.0);
				for (i = 0; i < lookAhead; i++)
				{
					params = predictionParameters[i][cogDelta];
					theta = params[Const];
					for (j = FirstParam; j < nParams; j++)
					{
						if (j == Const)
							j++;
						theta += params[j] * val[j];
					}
					theta *= fudge; //fudge factor
					theta += courseDelta;
					theta *= (float)(M_PI / 180.0);
					//contributions below: model for vessel and contribution of current
					glVertex3f((float)((i + 1) * (speedFactor * sin(theta) + currentX)),
							   (float)((i + 1) * (speedFactor * cos(theta) + currentY)), 0.0);
				}
			glEnd();
		}
	}

    glEnable(GL_LIGHTING);

	glLineWidth(1.0);
	val[Rudder] = current->rudder;
	if (tMinusOne)
	{
		val[RudderMinusTwo] = tMinusOne->rudder_minus_one;
		val[RudderMinusOne] = tMinusOne->rudder;
	}
	else
	{
		val[RudderMinusTwo] = 0;
		val[RudderMinusOne] = 0;
	}

	
	//Draw boat with new heading
	if (parentVessel)
	{
		renderingPhantom = true;
		glPushMatrix();
		if(lookAhead)
		{
			glTranslatef(finalX, finalY, 0.0);

			params = predictionParameters[finalIndex][HeadingDelta];
			theta = params[Const];
			for (j = FirstParam; j < Const; j++)
				theta += params[j] * val[j];
			//theta *= (float)(M_PI / 180.0);
			glRotatef(-theta, 0.0, 0.0, 1.0);
		}
			//CMGstate = parentVessel->hideCMG();
			//tailState = parentVessel->hideTail();
			//parentVessel->hideChildren();
			parentVessel->DrawAsFlatIcon(win, 0.6f, GL_FALSE);
			//if (CMGstate)
			//	parentVessel->showCMG();
			//if (tailState)
			//	parentVessel->showTail();
			//parentVessel->showChildren();

		glPopMatrix();
		renderingPhantom = false;
	}
}

