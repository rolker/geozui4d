/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// VectorAnimator.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class VectorAnimator;

#ifndef VECTORANIMATOR_H
#define VECTORANIMATOR_H

#include "interactors/Animator.h"

class Object;
/** 
 *	Instances of this class animate a PosableStateBase along a given vector.
 *	This class is responsible for the following.
 *	\li Managing operand and resultant PosableStateBases
 *  \li Process input events
 *	\li Recognize a "cancel" event
 *
 *	\author Matt Plumlee (mdp)
 *  \date 9/13/2002
 */
class VectorAnimator: public Animator
{
  public:
	DERIVE_ANIMATOR(VectorAnimator);
	enum {DirectionOperand, SpeedOperand};
	VectorAnimator(PosableStateBase *resultant,
				 PosableStateBase *DirectionOperand = NULL);
        VectorAnimator(std::string name = "", PosableStateBase *resultant = NULL,
					  PosableStateBase *DirectionOperand = NULL);
	void init(PosableStateBase *DirectionOperand = NULL);
	virtual ~VectorAnimator();

	/*---Accessor functions---*/
	long getChangeMask() {return changeMask;}

	/*---Interaction functions---*/
	virtual int HandleInput(unsigned int inputType, InputInfo *info);
	virtual int animate();

	/*---Scripting functions---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

  protected:
};

#endif
