// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// PosableStateRelationship.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class PosableStateRelationship;

#ifndef POSABLESTATERELATIONSHIP_H
#define POSABLESTATERELATIONSHIP_H

#include "SDV_types.h"

class PosableStateBase;
/** 
 *	This is the interface for classes that represent constraints or other
 *	relationships between PosableStateBases.
 *	Implementations of this interface are responsible for the following.
 *	\li Managing new PosableStateBases by collecting and/or passing along
 *		change events
 *	\li Removing a PosableStateBase from management.
 *  \li Provide information about the relationship.
 *	\note Transformed by mdp 6/20/2001
 *	\author Matt Plumlee (mdp)
 *  \date 1/5/2001 as PosableStateBroker
 */
class PosableStateRelationship
{

  public:
	PosableStateRelationship(void *Creator, int delThresh = 2) {creator = Creator; deleteThreshold = delThresh;}
	virtual ~PosableStateRelationship() {;}

	/*---Interaction facilitation---*/
	virtual long manage(PosableStateBase *pose) = 0;
	virtual void relinquish(PosableStateBase *pose = 0) = 0;
	virtual void replace(PosableStateBase *oldPose, PosableStateBase *newPose) = 0;
	virtual long numManaging() = 0;
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged) = 0;

	/*---Relationship types---*/
	/*	      Other |    Rotate     |   Translate   |     Scale
	 *	+-----------|---------------|---------------|---------------+
	 *	|D|C|L|S|O|T| | |r|l|r|l|r|l| | |r|l|r|l|r|l| | |r|l|r|l|r|l|
	 *	+---|---|---|---|---|---|---|---|---|---|---|---|---|---|---+ bit 0
	 *	|   |   |   |   | Z | Y | X |   | Z | Y | X |   | Z | Y | X
	 *
	 *	r = relative relationship
	 *	l = absolute relationship
	 *	T = one-way relationship from target
	 *	O = one-way relationship from owner
	 *	S = set (only used with O)
	 *	L = localized-relative relationship (rotation/scale apply to translate, essentially owner's reference frame)
	 *	C = "complicated" relationship flag
	 *	D = "dynamic" relationship flag (automatically "rachets"--unlocks and relocks when the target end of a one-way localized lock changes)
	 */
	enum RelationshipType {
					NoRelationship		= 0x0,
					ScaleMask			= 0x0000003F,
					TranslateMask		= 0x00003F00,
					RotateMask			= 0x003F0000,
					RotateXMask			= 0x00030000,
					RotateYMask			= 0x000C0000,
					RotateZMask			= 0x00300000,
					AnyMask				= 0x0F3F3F3F,
					Scale				= 0x00000015,
					ScaleComplex		= 0x10000015,
					Translate			= 0x00001500,
					TranslateX			= 0x00000100,
					TranslateY			= 0x00000500,
					TranslateZ			= 0x00001000,
					TranslateComplex	= 0x10001500,
					Rotate				= 0x00150000,
					RotateX				= 0x00010000,
					RotateY				= 0x00040000,
					RotateZ				= 0x00100000,
					RotateComplex		= 0x10150000,
					AbsoluteMask		= 0x00151515,
					ComplexMask			= 0x10151515,
					RelativeScale		= 0x0000002A,
					RelativeTranslate	= 0x00002A00,
					RelativeRotate		= 0x002A0000,
					RelativeRotateX		= 0x00020000,
					RelativeRotateY		= 0x00080000,
					RelativeRotateZ		= 0x00200000,
					RelativeMask		= 0x002A2A2A,
					FromTarget			= 0x01000000,
					FromOwner			= 0x02000000,
					FromBoth			= 0x03000000,
					SetFromOwner		= 0x06000000,
					Localized			= 0x08000000,
					Dynamic				= 0x20000000};

	/*---Accessor functions---*/
	virtual long getType(long fromWhom = FromBoth, PosableStateBase *associatedPose = NULL) = 0;
	virtual void *getCreator() {return creator;}
	virtual PosableStateBase *getPoseOfOwner(int index = -1) = 0;
	virtual PosableStateBase *getPoseOfTarget(int index = -1) = 0;
	int getDeleteThreshold() {return deleteThreshold;}
	void setDeleteThreshold(int newThresh) {deleteThreshold = newThresh;}

  protected:
	void *creator;
	int deleteThreshold;
};

#endif
