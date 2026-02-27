/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableState.h

#ifndef POSABLESTATE_H
#define POSABLESTATE_H

#include <gz4d/Point.h>
#include "SDV_types.h"


	typedef float			ScaleType;

	//constants for a rotation of x degrees around unit vector u or other
// 	const short typeQuaternion		= 0; //(cos(x/2), u*sin(x/2)
// 	const short typePseudoQuaternion	= 1; //(x, u)
// 	const short typeYawPitchRoll		= 2; ///degrees yaw (about screen Y), then degrees Pitch (about screen X), then degrees roll

    struct OrientationInfo
    {
        enum Type {typeInvalid, typeQuaternion, typePseudoQuaternion, typeYawPitchRoll};
        Type type;
        double angle; //in degrees for pseudoQuaternion, not used for yawpitchroll
        gz4d::Point<double> axis;  //contains all rotatoin info if typeYawPitchRoll.

        OrientationInfo():type(typeYawPitchRoll),angle(0.0),axis(0.0,0.0,0.0){}
        void SetInvalid(){type=typeInvalid;}
        operator bool(){return type!=typeInvalid;}
    };

    struct PosableStateInfo
    {
        gz4d::Point<double> position;
        OrientationInfo orientation;
        double scale;
    };

	typedef Point3f* Contour;

	/*---Pose Change flags---*/
	/*	  |     Rotate    |   Translate   |     Scale
	 *	+-|---------------|---------------|---------------+
	 *	| | | | | | | | | | | | | | | | | | | | | | | | | |
	 *	+-|-|-|---|---|---|-|-|---|---|---|-|-|---|---|---+ bit 0
	 *	 I|c|h| Z | Y | X | |p| Z | Y | X |M|m| Z | Y | X
	 *
	 *	  M = 1 ==> Max changed (m = 0)
	 *	  m = 1 ==> min Changed (M = 0)
	 *    p = 1 ==> Preferred value changed
	 *	  h = 1 ==> Home value changed
	 *	  c = 1 ==> Current (regular) value changed
	 *	  I = 1 ==> Index changed (for wrappers)
	 */
	const long ChangeNone			= 0x0000000;
	const long ChangeAnyMask		= 0x03F3F3F;
	const long ChangeScaleMask		= 0x000003F;
	const long ChangeTranslateMask	= 0x0003F00;
	const long ChangeTranslateXMask	= 0x0000300;
	const long ChangeTranslateYMask	= 0x0000C00;
	const long ChangeTranslateZMask	= 0x0003000;
	const long ChangeRotateMask		= 0x03F0000;
	const long ChangeRotateXMask	= 0x0030000;
	const long ChangeRotatePitchMask= 0x0030000;
	const long ChangeRotateYMask	= 0x00C0000;
	const long ChangeRotateYawMask	= 0x00C0000;
	const long ChangeRotateZMask	= 0x0300000;
	const long ChangeRotateRollMask	= 0x0300000;
	const long ChangeMinMask		= 0x0000040;
	const long ChangeMaxMask		= 0x0000080;
	const long ChangePreferredMask	= 0x0004000;
	const long ChangeHomeMask		= 0x0400000;
	const long ChangeCurrentMask	= 0x0800000;
	const long ChangeTypeMask		= 0x0C0C0C0;
	const long ChangeScale			= 0x080003F;
	const long ChangeTranslate		= 0x0803F00;
	const long ChangeTranslateX		= 0x0800300;
	const long ChangeTranslateY		= 0x0800C00;
	const long ChangeTranslateZ		= 0x0803000;
	const long ChangeRotate			= 0x0BF0000;
	const long ChangeRotateX		= 0x0830000;
	const long ChangeRotatePitch	= 0x0830000;
	const long ChangeRotateY		= 0x08C0000;
	const long ChangeRotateYaw		= 0x08C0000;
	const long ChangeRotateZ		= 0x0B00000;
	const long ChangeRotateRoll		= 0x0B00000;
	const long ChangePose			= 0x0BF3F3F;
	const long ChangeMinScale		= 0x000007F;
	const long ChangeMinTranslate	= 0x0003F40;
	const long ChangeMinTranslateX	= 0x0000340;
	const long ChangeMinTranslateY	= 0x0000C40;
	const long ChangeMinTranslateZ	= 0x0003040;
	const long ChangeMinRotate		= 0x03F0040;
	const long ChangeMinRotateX		= 0x0030040;
	const long ChangeMinRotatePitch	= 0x0030040;
	const long ChangeMinRotateY		= 0x00C0040;
	const long ChangeMinRotateYaw	= 0x00C0040;
	const long ChangeMinRotateZ		= 0x0300040;
	const long ChangeMinRotateRoll	= 0x0300040;
	const long ChangeMinPose		= 0x03F3F7F;
	const long ChangeMaxScale		= 0x00000BF;
	const long ChangeMaxTranslate	= 0x0003F80;
	const long ChangeMaxTranslateX	= 0x0000380;
	const long ChangeMaxTranslateY	= 0x0000C80;
	const long ChangeMaxTranslateZ	= 0x0003080;
	const long ChangeMaxRotate		= 0x03F0080;
	const long ChangeMaxRotateX		= 0x0030080;
	const long ChangeMaxRotatePitch	= 0x0030080;
	const long ChangeMaxRotateY		= 0x00C0080;
	const long ChangeMaxRotateYaw	= 0x00C0080;
	const long ChangeMaxRotateZ		= 0x0300080;
	const long ChangeMaxRotateRoll	= 0x0300080;
	const long ChangeMaxPose		= 0x03F3FBF;
	const long ChangeHomePose		= 0x07F3F3F;
	const long ChangeHomeTranslate	= 0x0403F00;
	const long ChangeHomeScale		= 0x040003F;
	const long ChangeAll			= 0x0FFFFFF;
	const long ChangeIndex			= 0x1000000;

/*---------------------------------------------------------------------------*/
/** 
 *	This file contains the types and interfaces for a scene and objects
 *	locatable and/or movable within in the scene.
 *  \par PosableState is the base interface for all position-related information.
 *  \author Matt Plumlee (mdp)
 *	\date 12/8/2000
 *  \image html posableState.jpg
 *  \image html posableStateUse.jpg
 */
/*---------------------------------------------------------------------------*/
	class PosableState
	{
	  public:
		//Generic positioning information (note: if supported, minimums,
		//maximums and preferred positions can also be changed by these
		//functions through the changeMask.)
		PosableStateInfo getPoseInfo(long changeMask);
		void setPoseInfo(PosableStateInfo state,long changeMask);
		void setPoseInfoFromTo(long fromMask, long toMask);
		void offsetPoseInfoBy(PosableStateInfo state, long changeMask);
		void unsetPoseInfo( long changeMask);

		//Focus+context positioning information
		void getFocusInfoUsing(PosableStateInfo stateInfo);
		void getContextInfoUsing(PosableStateInfo stateInfo);

		//Continuous change indicators (high-res rendering hint)
		void setContinuousChange(unsigned char changing);
		unsigned char isInContinuousChange();

/*
		//Turn highlighting on/off
		void highlight(in boolean state);

		//Is the position information editable?
		boolean isEditable(in long changeMask);
*/
	};
/*
	//-------------------------------------------------------------------------
	//MultiPosableState is the base interface for all data that has multiple
	//points that can be edited.  This interface provides a way to shift the
	//PosableState interface among points (so you would select the point
	//through this interface, and use the inherited interface to make
	//changes to the point.
	interface MultiPosableState : PosableState
	{
		void addPoint(in Point3F location);
		void addPoints(in Contour locations);
		void removePoint();

		PosableStateInfo getPoseInfoAt(in long index);
		void setPoseInfoAt(in long index, in PosableStateInfo state, in long changeMask);
		void offsetPoseInfoAt(in long index, in PosableStateInfo state, in long changeMask);

		void setCurrentPoint(in long index);
		long getCurrentPoint();
	};

	//-------------------------------------------------------------------------
	//CompositePosableState is the base interface for all compound posables,
	//that is, Posables that themselves are composed of other Posables.
	typedef sequence<PosableState> PosableList;
	interface CompositePosableState : PosableState
	{
		void addPosableState(in PosableStateInfo state);
		void removePosableState();

		PosableState getPoseableAt(in long index);
		PosableList getPoseables();

		void setCurrentPosableIndex(in long index);
		long getCurrentPosableIndex();
	};
*/

#endif
