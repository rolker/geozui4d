// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ObjectPoseState.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, but don't have any focus or context info.
 *	GL transformations are specific to objects (vs. views)
 *
 *	Created by mdp 12/27/2000
 */
/*---------------------------------------------------------------------------*/
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "PosableStateUser.h"
#include "engine/ViewManager.h"
#include <assert.h>
#include <gz4d/glo/Translate.h>

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
ObjectPoseState::ObjectPoseState(PosableStateUser *initialUser, std::string name, OrientationInfo::Type orientationType)
:PoseState(initialUser, name, orientationType)
{
	dontHeightExaggerate = false;
        
}

/*---------------------------------------------------------------------------*/
ObjectPoseState::ObjectPoseState(PosableStateBase *toBeReplaced,
								 GLboolean userCall)
:PoseState(toBeReplaced)
{
	if (userCall)
		replace(toBeReplaced);
}

/*-----------------------------GL Transformations----------------------------*/
/*---------------------------------------------------------------------------*/
void ObjectPoseState::applyTransforms(ViewManager * view)
{
    if(view)
    {
        gz4d::glo::Translate(pose.position-view->getReferencePoint());
    }
    else
        gz4d::glo::Translate(pose.position);

    double scale = pose.scale;
    if(minPixelsPerMeter > 0.0)
    {
        double pixelSize = view->getPixelSizeAt(pose.position);
        double maxPixelSize = 1.0/minPixelsPerMeter;
        if (maxPixelSize<pixelSize)
            scale = pose.scale*pixelSize/maxPixelSize;
    }


    if(dontHeightExaggerate)
            glScaled(scale, scale, scale/view->getHeightExaggeration());
    else
            glScaled(scale, scale, scale);

    if (pose.orientation.type == OrientationInfo::typeYawPitchRoll)
    {
            glRotated(-pose.orientation.axis[Yaw], 0.0, 0.0, 1.0);
            glRotated(-pose.orientation.axis[Pitch], 1.0, 0.0, 0.0);
            glRotated(pose.orientation.axis[Roll], 0.0, 1.0, 0.0);
    }
    else if (pose.orientation.type == OrientationInfo::typePseudoQuaternion)
            glRotated(pose.orientation.angle,
                                    pose.orientation.axis[X],
                                    pose.orientation.axis[Y],
                                    pose.orientation.axis[Z]);
    else //typeQuaternion
            assert(0);//not implemented yet
}

