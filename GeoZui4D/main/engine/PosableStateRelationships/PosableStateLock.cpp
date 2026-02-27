/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableStateLock.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides the mechanisms for locking two PosableStateBases
 *  together in one or more aspects (rotation, translation, scale).
 *  This class is responsible for implementing the following.
 *      * Inserting itself between one or more PosableStateBase(s) and their
 *        User(s).
 *      * Modifying 
 *      * Removing itself, and restoring the link between one or more
 *        PosableStateBase(s) and their User(s).
 *      * Registering and unregistering itself with the lockManager when
 *        constructed and destroyed, respectively.
 *
 *  Created by mdp 1/5/2001
 */
/*---------------------------------------------------------------------------*/

#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "widgets/Sv_Standards.h"
#include "widgets/Sv_Matrix4.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "utilities/DottedAddress.h"
#include "engine/WindowPoseState.h"
#include <gz4d/util/Utilities.h>

Addressable CouplingNeighborhoodAnchor;
DottedAddress *CouplingNeighborhood = new DottedAddress(&CouplingNeighborhoodAnchor, "Coupling");

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
Creates a lock from ownerPose to targetPose. 
*/
PosableStateLock::PosableStateLock(void *creator,
                                   PosableStateBase *ownerPose,
                                   PosableStateBase *targetPose,
                                   long changeMask)
: PosableStateRelationship(creator)
{
    relationshipManager->registerRelationship(this); //make sure to do this before managing anyone ourselves
    poseOfOwner = NULL;
    poseOfTarget = NULL;
    lockMechanism = LockNone;
    lockMechanismBuffer = LockNone;
    ignoreLockEvent = GL_FALSE;
    targetIsWindow = GL_FALSE;
    ownerIsWindow = GL_FALSE;
    if (ownerPose)
    {
        manage(ownerPose);
        lock(changeMask, targetPose);
    }
}

/*---------------------------------------------------------------------------*/
PosableStateLock::~PosableStateLock()
{
    relinquish();
    relationshipManager->unregisterRelationship(this); //make sure to do this after unmanaging
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** Reset lock to empty
 */
void PosableStateLock::reset()
{
    lockMechanism = LockNone;
    ignoreLockEvent = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** The manage procedure assigns poseOfOwner first (if it is NULL), then
 *  poseOfTarget (virtual to allow child classes to have multiple targets).
 *  The return value is the "index" of the managed pose for future reference
 *  (if this implementation pays any attention to it).
 */
long PosableStateLock::manage(PosableStateBase *pose)
{
    PosableStateBase *test;

    if (pose == poseOfOwner)
        return 0;

    DYNAMIC_CAST(test, WindowPoseState *, pose);
    if (!poseOfOwner)
    {
        poseOfOwner = pose;
        relationshipManager->attach(this, pose);
        ownerIsWindow = (test)? GL_TRUE : GL_FALSE;
        return 0;
    }
    else
    {
        if (poseOfTarget != pose)
        {
            if (poseOfTarget)
                relinquish(poseOfTarget);
            relationshipManager->attach(this, pose);
            poseOfTarget = pose;
            targetIsWindow = (test)? GL_TRUE : GL_FALSE;
        }
        return 1;
    }
}

/*---------------------------------------------------------------------------*/
/** The relinquish procedure removes this PosableStateLock from the
 *  notification path of the passed PosableStateBase, or of both if NULL
 *  (virtual to allow child classes to relinquish all if NULL).  The original
 *  PosableStateUser is reinstated (i.e., don't stack these things until some
 *  changes are made to allow it to work properly!).
 */
void PosableStateLock::relinquish(PosableStateBase *pose)
{
    if (!pose || pose == poseOfOwner)
    {
        relationshipManager->detach(this, poseOfOwner);
        poseOfOwner = NULL;
    }
    if (!pose || pose == poseOfTarget)
    {
        relationshipManager->detach(this, poseOfTarget);
        poseOfTarget = NULL;
    }
}

/*---------------------------------------------------------------------------*/
/** The replace procedure replaces one pose with another, with no side
 *  effects.
 */
void PosableStateLock::replace(PosableStateBase *oldPose, PosableStateBase *newPose)
{
    if (oldPose == poseOfOwner)
        poseOfOwner = newPose;
    else if (oldPose == poseOfTarget)
        poseOfTarget = newPose;
}

/*---------------------------------------------------------------------------*/
/** The numManaging function returns how many PosableStateBases this instance
 *  is currently managing (minus the number it has relinquished).
 */
long PosableStateLock::numManaging()
{
    long answer = 0;

    if (poseOfOwner)
        answer++;
    if (poseOfTarget)
        answer++;

    return answer;
}

/*---------------------------------------------------------------------------*/
long PosableStateLock::getRelationshipType(PosableStateBase *targetPose)
{
    return lockMechanism;
}

/*---------------------------------------------------------------------------*/
/** This function adds a lock (of the passed type) on the PosableStateBase.
 *  Locks and delta-locks are mutually exclusive, so this function removes the
 *  one if the other is set.  This function goes ahead and enforces the lock
 *  between the two views if it is a true lock (not a delta-lock).
 *
 *  @param  changeMask      LockType enum value or composite to be added
 *  @param  targetPose      PosableStateBase to lock to (replaces current value
 *                          if non-NULL);
 */
void PosableStateLock::lock(long changeMask, PosableStateBase *targetPose)
{
    long removalMask;
    double scale;
    gz4d::Point<double> translate;
    gz4d::Point<double> angle,rot;

    if (!poseOfOwner)
    {
        if (targetPose != NULL && poseOfTarget != NULL)
            poseOfOwner = targetPose;
        targetPose = NULL;
        return;
    }

    scale = poseOfOwner->getScale();
    if (scale == 0.0)
        scale = 1.0;
    if (targetIsWindow != ownerIsWindow)
        scale = 1.0f / scale;

    translate = poseOfOwner->getPosition();
    angle = poseOfOwner->getOrientationAxis();

    if (targetPose && targetPose != poseOfTarget)
        manage(targetPose);

    removalMask = ((changeMask & LockOnlyMask) << 1) + ((changeMask & LockDeltaMask) >> 1);
    lockMechanism &= ~removalMask;

    lockMechanism |= changeMask;

    //Set up deltas
    if ((changeMask & LockDeltaScale)
        || (changeMask & LockLocalized))
    {
        scaleDeltaFactor = scale / poseOfTarget->getScale();
        scaleDeltaStart = scale;
    }
    else if (!(lockMechanism & LockDeltaScale))
        scaleDeltaStart = scaleDeltaFactor = 1.0;

    if ((changeMask & LockDeltaTranslate)
        || (changeMask & LockLocalized))
    {
        translateDelta = translate - poseOfTarget->getPosition();
    }

    if ((changeMask & LockDeltaRotate)
        || (lockMechanism & LockLocalized))
    {
        rot = poseOfTarget->getOrientationAxis();
        if (gz4d::IsNan(rot[0]))
            rot = gz4d::Point<double>(0.0,0.0,0.0);

        if (!gz4d::IsNan(angle[0]))
        {
            if (lockMechanism & LockDeltaRotateY)
                angleDelta[Y] = angle[Y] - rot[Y];
            if (lockMechanism & LockDeltaRotateX)
                angleDelta[X] = angle[X] - rot[X];
            if (lockMechanism & LockDeltaRotateZ)
                angleDelta[Z] = angle[Z] - rot[Z];
        }

        if (lockMechanism & LockLocalized)
        {
            Sv_Matrix4 transmatrix;
            float   tempVector[4], resultVector[4];

            if (!gz4d::IsNan(angle[0]))
            {
                angleDelta[Y] = angle[Y] - rot[Y];
                angleDelta[X] = angle[X] - rot[X];
                angleDelta[Z] = angle[Z] - rot[Z];
            }
            else
                angleDelta[X] = angleDelta[Y] = angleDelta[Z] = 0.0;
            COPY_POINT(tempVector, translateDelta);
            tempVector[3] = 1.0;
            glPushMatrix();
            glLoadIdentity();
            if (!gz4d::IsNan(angle[0]))
            {
                glRotatef(-angle[Y], 0.0, 0.0, 1.0);
                glRotatef(-angle[X], 1.0, 0.0, 0.0);
                glRotatef(-angle[Z], 0.0, 1.0, 0.0);
            }
            else
            {
                glRotatef(-rot[Y], 0.0, 0.0, 1.0);
                glRotatef(-rot[X], 1.0, 0.0, 0.0);
                glRotatef(-rot[Z], 0.0, 1.0, 0.0);
            }
            transmatrix.GetGLMatrix();
            glPopMatrix();
            transmatrix.Multiply(tempVector, resultVector);
            COPY_POINT(translateDelta, resultVector);
        }
    }

    //enforce any true locks
    if (changeMask & LockOnlyMask)
    {
        if (lockMechanism & LockFromTarget)
            handleChangeIn(poseOfTarget, changeMask/*, GL_TRUE?*/);
        else if (lockMechanism & LockFromOwner)
            handleChangeIn(poseOfOwner, changeMask);
    }
}

/*---------------------------------------------------------------------------*/
/** This function subtracts a lock (of the passed type) on the passed
 *  PosableStateBase.
 *
 *  @param  changeMask  ?
 *  @param  targetPose      PosableStateBase to remove lock from (replaces
 *                          current value if non-NULL);
 */
void PosableStateLock::unlock(long changeMask, PosableStateBase *targetPose)
{
    lockMechanismBuffer = lockMechanism;
    lockMechanism &= ~changeMask;
    if (lockMechanism & LockLocalized)
        lock(lockMechanism); //reset our variables.
}

/*---------------------------------------------------------------------------*/
/** This function restores the lock mechanism to the one in place
 *  just prior to the last unlock operation.
 */
void PosableStateLock::restorePreviousLock()
{
    lock(lockMechanismBuffer);
}

/*---------------------------------------------------------------------------*/
void PosableStateLock::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    long action;

    if (!(whichPose == poseOfTarget || whichPose == poseOfOwner))
        return;

    if (!ignoreLockEvent)
    {
        action = whatChanged & lockMechanism;
        if (action || (lockMechanism & LockLocalized))
        {
            //If it's a dynamic lock, just update the offsets, otherwise, handle the change
            if ((lockMechanism & Dynamic) &&
                (((lockMechanism & LockFromOwner) && whichPose == poseOfTarget) ||
                ((lockMechanism & LockFromTarget) && whichPose == poseOfOwner)))
                    lock(lockMechanism);
            else
                handleChangeIn(whichPose, action);
        }
    }
}

/*---------------------------------------------------------------------------*/
/** This function uses the information found in the passed PosableStateBase
 *  to alter the associated information in the other PosableStateBase(s)
 *  known to this PosableStateLock (as specified by the passed lock type).
*/
void PosableStateLock::handleChangeIn(PosableStateBase *lockSource, long lockType,
                                      GLboolean OverrideLockMechanism)
{
    const OrientationInfo *orient;
    gz4d::Point<double> angle;
    double scaleFactor = 1.0, scale;
    PosableStateBase *lockTarget;
    int sourceFactor;

    //Make sure we only play with the bits we're set up to deal with.
    if (!OverrideLockMechanism)
    {
        if (ignoreLockEvent)
            return;
        if (lockSource == poseOfOwner &&
            ((lockMechanism & LockFromOwner) == 0))
            return;
        if (lockSource == poseOfTarget &&
            ((lockMechanism & LockFromTarget) == 0))
            return;
        lockType &= lockMechanism;
    }

    if (!lockType && !(lockMechanism & LockLocalized))
        return;

    //Set up the pointer and multiplier for shortening the subsequent code
    if (lockSource == poseOfOwner)
    {
        lockTarget = poseOfTarget;
        sourceFactor = -1;
    }
    else
    {
        lockTarget = poseOfOwner;
        sourceFactor = 1;
    }

    if (!lockTarget)
        return;

    lockTarget->AccumulateNotifications();
    //Handle scale locking
    if (lockType & LockScaleMask)
    {
        scale = lockSource->getScale();
        if (targetIsWindow != ownerIsWindow)
            scale = 1.0f / scale;

        if ((lockType & LockDeltaScale) ||
            (lockMechanism & LockLocalized))
        {
            if (sourceFactor == 1)
                scale *= scaleDeltaFactor;
            else
                scale /= scaleDeltaFactor;
        }
        lockTarget->setScale(scale);
    }

    //Handle rotation locking
    if ((lockType & LockRotateMask) ||
        (lockMechanism & LockLocalized))
    {
        orient = lockSource->getOrientation();
        lockTarget->setOrientation(*orient, lockType | ChangeCurrentMask);

        if (lockType & LockDeltaRotate)
        {
            lockTarget->rotateBy(sourceFactor * angleDelta[X],
                                 sourceFactor * angleDelta[Y],
                                 sourceFactor * angleDelta[Z],
                                 lockType | ChangeCurrentMask);
        }
    }

    //Handle Translation locking
    if (lockType & LockTranslateMask
        || (lockMechanism & LockLocalized))
    {
        lockTarget->setPosition(lockSource->getPosition());
        if ((lockMechanism & LockDeltaTranslate) &&
            (lockType & LockTranslate))
        {
            lock(LockTranslate);
            lockTarget->ResumeSingularNotification();
            return;
        }

        if ((lockType & LockDeltaTranslate)
            || (lockMechanism & LockLocalized))
        {
            float   translationVector[4];

            scale = lockSource->getScale();
            if (scale == 0.0)
                scale = 1.0;
            if ((lockMechanism & LockScaleMask) || lockMechanism & LockLocalized) //handle special coupling
            {
                    scaleFactor = sourceFactor * scale / scaleDeltaStart;
            }
            else
                scaleFactor = sourceFactor;

            if (lockMechanism & LockLocalized)
            {
                Sv_Matrix4 transmatrix;
                float   tempVector[4];

                COPY_POINT(tempVector, translateDelta);
                tempVector[3] = 1.0;

                glPushMatrix();
                glLoadIdentity();
                transmatrix.SetIdentity();
                orient = lockSource->getOrientation();
                if (orient)
                    angle = orient->axis;
                else
                    angle = gz4d::Point<double>(0.0,0.0,0.0);
                glRotatef(angle[Z], 0.0, 1.0, 0.0);
                glRotatef(angle[X], 1.0, 0.0, 0.0);
                glRotatef(angle[Y], 0.0, 0.0, 1.0);
                transmatrix.GetGLMatrix();
                glPopMatrix();
                transmatrix.Multiply(tempVector, translationVector);
            }
            else
                COPY_POINT(translationVector, translateDelta);

            lockTarget->moveBy(translationVector[X] * scaleFactor,
                               translationVector[Y] * scaleFactor,
                               translationVector[Z] * scaleFactor);

        }
    }

    ignoreLockEvent = GL_TRUE;
    lockTarget->ResumeSingularNotification();
    ignoreLockEvent = GL_FALSE;
}

/*---------------------------Accessor Functions------------------------------*/
/*---------------------------------------------------------------------------*/
/** This function returns what type of lock we have from the given direction.
 *  We don't care about the passed pose since we only have one of each.
 */
long PosableStateLock::getType(long fromWhom, PosableStateBase *associatedPose)
{
    if (fromWhom & lockMechanism)
        return lockMechanism;
    else
        return fromWhom;
}

/*---------------------------------------------------------------------------*/
/** We don't care about the index since we only have one of owner.
 */
PosableStateBase *PosableStateLock::getPoseOfOwner(int index)
{
    return poseOfOwner;
}

/*---------------------------------------------------------------------------*/
/** We don't care about the index since we only have one of owner.
 */
PosableStateBase *PosableStateLock::getPoseOfTarget(int index)
{
    return poseOfTarget;
}

void PosableStateLock::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">PosableStateLock:</h" << level << ">";
    html << "<h" << level+1 << ">Owner</h" << level+1 << ">";
    if(getPoseOfOwner())
        getPoseOfOwner()->getHtmlInfo(html,level);
    else
        html << "None.";
        
    html << "<h" << level+1 << ">Target</h" << level+1 << ">";
    if(getPoseOfTarget())
        getPoseOfTarget()->getHtmlInfo(html,level);
    else
        html << "None.";
}
