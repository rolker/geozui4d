/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableStateLock.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class PosableStateLock;

#ifndef POSABLESTATELOCK_H
#define POSABLESTATELOCK_H

#include<sstream>
#include "engine/PosableStateBase.h"
#include "PosableStateRelationship.h"
#include "utilities/Addressable.h"
/** 
 *	This class provides the mechanisms for locking two PosableStateBases
 *	together in one or more aspects (rotation, translation, scale).
 *	This class is responsible for implementing the following.
 *	\li Managing new PosableStateBases by passing change event along to them
 *	\li Removing a PosableStateBase from change notifications.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/5/2001
 */
class PosableStateLock: public PosableStateRelationship, public Addressable
{
  public:
	PosableStateLock(void *creator,
					 PosableStateBase *ownerPose = NULL,
					 PosableStateBase *targetPose = NULL, long changeMask = LockNone);
	virtual ~PosableStateLock();

	/*---Interaction facilitation---*/
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
	void handleChangeIn(PosableStateBase *lockSource, long lockType,
						GLboolean OverrideLockMechanism = GL_FALSE);

	virtual long manage(PosableStateBase *pose); //assigns ownerPose first, then targetPose(s)
	virtual void relinquish(PosableStateBase *pose = NULL); //relinquish both if NULL
	virtual void replace(PosableStateBase *oldPose, PosableStateBase *newPose);
	virtual long numManaging();

	virtual long getRelationshipType(PosableStateBase *targetPose = NULL);
	virtual void lock(long changeMask, PosableStateBase *targetPose = NULL);
	virtual void unlock(long changeMask, PosableStateBase *targetPose = NULL);
	virtual void reset(); //remove all locks

	virtual void restorePreviousLock(); //"undo" last unlock operation

	/*---Accessor functions---*/
	virtual long getType(long fromWhom = FromBoth, PosableStateBase *associatedPose = NULL);
	virtual PosableStateBase *getPoseOfOwner(int index = -1);
	virtual PosableStateBase *getPoseOfTarget(int index = -1);
	
        virtual void getHtmlInfo(std::stringstream &html, short level);

	 /** List of lock types
\verbatim
	 *---Locking types---
	 *	  Other |    Rotate     |   Translate   |     Scale
	 *	+-------|---------------|---------------|---------------+
	 *	|L|S|O|T| | |d|l|d|l|d|l| | |d|l|d|l|d|l| | |d|l|d|l|d|l|
	 *	+---|---|---|---|---|---|---|---|---|---|---|---|---|---+ bit 0
	 *	    |   |   | Z | Y | X |   | Z | Y | X |   | Z | Y | X
	 *	|8|4|2|1|8|4|2|1|8|4|2|1|8|4|2|1|8|4|2|1|8|4|2|1|8|4|2|1|
	 *
	 *	d = delta-locked (relative)
	 *	l = locked (absolute)
	 *	T = lock from target
	 *	O = lock from owner
	 *	S = set (only used with O)
	 *	L = localized-relative lock (rotation/scale apply to translate)
\endverbatim
	 */
	enum LockType {                     //oorrttss
					LockNone              = 0x00000000, ///< no locks
					LockScaleMask         = 0x0000003F,
					LockTranslateMask     = 0x00003F00,
					LockRotateMask        = 0x003F0000,
					LockRotateXMask       = 0x00030000,
					LockRotateYMask       = 0x000C0000,
					LockRotateZMask       = 0x00300000,
					LockAnyMask           = 0x0F3F3F3F,
					LockScale             = 0x00000015,
					LockTranslate         = 0x00001500,
					LockRotate            = 0x00150000,
					LockRotateX           = 0x00010000,
					LockRotateY           = 0x00040000,
					LockRotateZ           = 0x00100000,
					LockOnlyMask          = 0x00151515,
					LockDeltaScale        = 0x0000002A,
					LockDeltaTranslate    = 0x00002A00,
					LockDeltaRotate       = 0x002A0000,
					LockDeltaRotateX      = 0x00020000,
					LockDeltaRotateY      = 0x00080000,
					LockDeltaRotateZ      = 0x00200000,
					LockDeltaMask         = 0x002A2A2A,
					LockFromTarget        = 0x01000000,
					LockFromOwner         = 0x02000000,
					LockBoth              = 0x03000000,
					LockSetFromOwner      = 0x06000000,
					LockLocalized         = 0x08000000,
					LockDynamic           = 0x20000000,
					LockLocalOwnerDynamic = 0x2A2A2A2A}; ///< Dynamic, Localized, FromOwner, Delta

  protected:
	PosableStateBase *poseOfOwner, *poseOfTarget;

	long			lockMechanism;
	long			lockMechanismBuffer;
	GLboolean		ignoreLockEvent, ownerIsWindow, targetIsWindow;
        gz4d::Point<double> translateDelta;
	GLfloat			angleDelta[3];
	double	scaleDeltaFactor, scaleDeltaStart;
};

extern DottedAddress *CouplingNeighborhood;

#endif
