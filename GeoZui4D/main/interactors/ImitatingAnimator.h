// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ImitatingAnimator.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class ImitatingAnimator;

#ifndef IMITATINGANIMATOR_H
#define IMITATINGANIMATOR_H

#include "interactors/Animator.h"
/** 
 *	Instances of this class animate a PosableStateBase toward the state of
 *	two others summed together, with some potential input from various input
 *	streams.  This class is responsible for the following.
 *	\li Managing operand and resultant PosableStateBases
 *  \li Process input events
 *	\li Recognize a "cancel" event
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/18/2002
 */
class ImitatingAnimator: public Animator
{
  public:
	DERIVE_ANIMATOR(ImitatingAnimator);
	enum {BaseOperand, OffsetOperand};
	ImitatingAnimator(PosableStateBase *resultant,
					  PosableStateBase *OffsetOperand = NULL);
        ImitatingAnimator(std::string name = "",
					  PosableStateBase *resultant = NULL,
					  PosableStateBase *OffsetOperand = NULL);
	void init(PosableStateBase *OffsetOperand = NULL);
	virtual ~ImitatingAnimator();

	/*---Accessor functions---*/
	float getSpeed() {return speed;}
	long getChangeMask() {return changeMask;}

	/*---Mutator functions---*/
	void setSpeed(float newSpeed) {speed = newSpeed;}

	/*---Interaction functions---*/
	virtual int HandleInput(unsigned int inputType, InputInfo *info);
	virtual int animate();

	/*---Scripting funcitons---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---Event notification---*/
	static unsigned int imitationStabilizedEvent() { return imitationStabilizedInputType->value;}

  protected:
	float speed;
	GLboolean changedLastCycle, scaleTempersSpeed;

	static const EventType *imitationStabilizedInputType;
};

#endif
