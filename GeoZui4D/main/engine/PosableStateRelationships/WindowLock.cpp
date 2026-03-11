// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// WindowLock.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides the mechanisms for locking two WindowPoseStates
 *	together in one or more aspects (rotation, translation, scale).
 *	The big thing that WindowLocks add to PosableStateLocks is the
 *	way delta-locking is handled:  delta-locked scale and delta-locked
 *	rotation are applied to	delta-locked translation such that the
 *	all delta-locked relationships remain fixed between two views
 *	(this has the effect of keeping a delta-locked window in the same
 *	exact location and the same size with respect to the other no
 *	matter which window moves).
 *	This class is responsible for implementing the following.
 *		* Managing new WindowPoseStates by passing change event along to them
 *		* Removing a WindowPoseState from change notifications.
 *
 *	Created by mdp 2/20/2001
 */
/*---------------------------------------------------------------------------*/

#include "engine/PosableStateRelationships/WindowLock.h"
#include "widgets/Sv_Standards.h"
#include "widgets/Sv_Matrix4.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
WindowLock::WindowLock(void *creator,
					   PosableStateBase *ownerPose,
					   PosableStateBase *targetPose,
					   long changeMask)
:PosableStateLock(creator, ownerPose, targetPose, changeMask)
{
}

/*---------------------------------------------------------------------------*/
WindowLock::~WindowLock()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function adds a lock (of the passed type) on the PosableStateBase.
 *	Locks and delta-locks are mutually exclusive, so this function removes the
 *	one if the other is set.  This function goes ahead and enforces the lock
 *	between the two views if it is a true lock (not a delta-lock).
 *
 *	@param	changeMask		LockType enum value or composite to be added
 *	@param	targetPose		PosableStateBase to lock to (replaces current value
 *							if non-NULL);
 */
void WindowLock::lock(long changeMask, PosableStateBase *targetPose)
{
	long removalMask;
        gz4d::Point<double> rot;
	double scale = poseOfOwner->getScale();
        gz4d::Point<double> translate = poseOfOwner->getPosition();
	gz4d::Point<double> angle = poseOfOwner->getOrientationAxis();

	if (targetPose && targetPose != poseOfTarget)
		manage(targetPose);

	removalMask = ((changeMask & LockOnlyMask) << 1) + ((changeMask & LockDeltaMask) >> 1);
	lockMechanism &= ~removalMask;

	lockMechanism |= changeMask;

	//Set up deltas
	if ((changeMask & LockDeltaScale)
		|| ((changeMask & LockDeltaTranslate)
			&& (lockMechanism & LockDeltaScale)))
	{
		scaleDeltaFactor = scale / poseOfTarget->getScale();
		scaleDeltaStart = scale;
	}
	else if (!(lockMechanism & LockDeltaScale))
		scaleDeltaStart = scaleDeltaFactor = 1.0;

	if ((changeMask & LockDeltaTranslate)
		|| ((changeMask & LockDeltaScale)
			&& (lockMechanism & LockDeltaTranslate)))
	{
            translateDelta = translate - poseOfTarget->getPosition();
	}

	if (changeMask & LockDeltaRotate)
	{
		Sv_Matrix4 transmatrix;
		float	tempVector[4], resultVector[4];
		COPY_POINT(tempVector, translateDelta);
		tempVector[3] = 1.0;

		rot = poseOfTarget->getOrientationAxis();
		glPushMatrix();
		glLoadIdentity();
		if (lockMechanism & LockDeltaRotateY)
		{
			angleDelta[Y] = angle[Y] - rot[Y];
			glRotatef(-rot[Y], 0.0, 0.0, 1.0);
		}
		if (lockMechanism & LockDeltaRotateX)
		{
			angleDelta[X] = angle[X] - rot[X];
			glRotatef(-rot[X], 1.0, 0.0, 0.0);
		}
		if (lockMechanism & LockDeltaRotateZ)
		{
			angleDelta[Z] = angle[Z] - rot[Z];
			glRotatef(-rot[Z], 0.0, 1.0, 0.0);
		}
		transmatrix.GetGLMatrix();
		glPopMatrix();
		transmatrix.Multiply(tempVector, resultVector);
		COPY_POINT(translateDelta, resultVector);
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
/**	This function subtracts a lock (of the passed type) on the passed
 *	PosableStateBase.
 *
 *	@param	changeMask
 *	@param	targetPose		PosableStateBase to remove lock from (replaces
 *							current value if non-NULL);
 */
void WindowLock::unlock(long changeMask, PosableStateBase *targetPose)
{
	lockMechanism &= ~changeMask;
	lock(lockMechanism); //reset our variables--necessary for delta locks.
}

/*---------------------------------------------------------------------------*/
/**	This function uses the information found in the passed PosableStateBase
 *	to alter the associated information in the other PosableStateBase(s)
 *	known to this PosableStateLock (as specified by the passed lock type).
*/
void WindowLock::handleChangeIn(PosableStateBase *lockSource, long lockType,
								GLboolean OverrideLockMechanism)
{
	const OrientationInfo *orient;
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

	if (!lockType)
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
		if (lockType & LockDeltaScale)
		{
			if (sourceFactor == 1)
				scale *= scaleDeltaFactor;
			else
				scale /= scaleDeltaFactor;
		}
		lockTarget->setScale(scale);
	}

	//Handle rotation locking
	if (lockType & LockRotateMask)
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
		|| ((lockMechanism & LockDeltaTranslate) && (lockMechanism & LockDeltaRotate)))
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
			|| ((lockMechanism & LockDeltaTranslate) && (lockMechanism & LockDeltaRotate)))
		{
			float	translationVector[4];

			scale = lockSource->getScale();
			if (lockMechanism & LockScaleMask) //handle special coupling
				scaleFactor = sourceFactor * scaleDeltaStart / scale;
			else
				scaleFactor = sourceFactor;

			if (lockMechanism & LockDeltaRotate)
			{
				Sv_Matrix4 transmatrix;
				float	tempVector[4];

				COPY_POINT(tempVector, translateDelta);
				tempVector[3] = 1.0;

				glPushMatrix();
				glLoadIdentity();
				transmatrix.SetIdentity();
				orient = poseOfTarget->getOrientation();
				if (lockMechanism & LockRotateZMask)
					glRotatef(orient->axis[Z], 0.0, 1.0, 0.0);
				if (lockMechanism & LockRotateXMask)
					glRotatef(orient->axis[X], 1.0, 0.0, 0.0);
				if (lockMechanism & LockRotateYMask)
					glRotatef(orient->axis[Y], 0.0, 0.0, 1.0);
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
