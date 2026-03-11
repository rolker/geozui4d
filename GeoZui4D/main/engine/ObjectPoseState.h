// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ObjectPoseState.h
/*---------------------------------------------------------------------------*/

class ObjectPoseState;

#ifndef OBJECT_POSE_STATE_H
#define OBJECT_POSE_STATE_H


#include "PoseState.h"
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, but don't have any focus or context info.
 *	GL transformations are specific to objects (vs. views)
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/27/2000
 */
class ObjectPoseState : public virtual PoseState
{
    public:
        /*---Constructors/Destructors---*/
        ObjectPoseState(PosableStateUser *initialUser = NULL, std::string name = "", OrientationInfo::Type orientationType = OrientationInfo::typeYawPitchRoll);
        ObjectPoseState(PosableStateBase *toBeReplaced, GLboolean userCall = GL_TRUE);

        /*---GL Transformations---*/
        virtual void applyTransforms(ViewManager * view = NULL);

        void ignoreHeightExaggeration(bool i = true){dontHeightExaggerate = i;};
    protected:
        bool dontHeightExaggerate;
};

#endif
