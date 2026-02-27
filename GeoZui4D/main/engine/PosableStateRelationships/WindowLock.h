/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// WindowLock.h
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
class WindowLock;

#ifndef WINDOWLOCK_H
#define WINDOWLOCK_H

#include "engine/PosableStateRelationships/PosableStateLock.h"

class WindowLock: public PosableStateLock
{
  public:
	WindowLock(void *creator,
			   PosableStateBase *ownerPose = NULL,
			   PosableStateBase *targetPose = NULL, long changeMask = LockNone);
	virtual ~WindowLock();

	/*---Interaction facilitation---*/
	void handleChangeIn(PosableStateBase *lockSource, long lockType,
						GLboolean OverrideLockMechanism = GL_FALSE);

	virtual void lock(long changeMask, PosableStateBase *targetPose = NULL);
	virtual void unlock(long changeMask, PosableStateBase *targetPose = NULL);
};

#endif
