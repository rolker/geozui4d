// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// PathAnimator.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class PathAnimator;

#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H

#include "interactors/Animator.h"

class Object;
class PosableStateWrapper;
/** 
 *	Instances of this class animate a PosableStateBase toward along a path
 *	using a PosableStateWrapper.  It determines direction from a simple
 *	calculation based on the current and previous locations.  Input is
 *	also accepted from various input streams.  This class is responsible for
 *	the following.
 *	\li Managing operand and resultant PosableStateBases
 *	\li Process input events
 *	\li Recognize a "cancel" event
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/8/2002
 */
class PathAnimator: public Animator
{
  public:
	DERIVE_ANIMATOR(PathAnimator);
	enum {WrapperOperand, OffsetOperand};
	PathAnimator(PosableStateBase *resultant,
				 PosableStateBase *OffsetOperand = NULL);
        PathAnimator(std::string name = "", PosableStateBase *resultant = NULL,
					  PosableStateBase *OffsetOperand = NULL);
	void init(PosableStateBase *OffsetOperand = NULL);
	virtual ~PathAnimator();

	/*---Accessor functions---*/
	float getSpeed() {return speed;}
	float getIdleSpeed() {return idleSpeed;}
	float getNegativeSpeedFactor() {return negativeFactor;}
	long getChangeMask() {return changeMask;}

	/*---Mutator functions---*/
	void setSpeed(float newSpeed) {speed = newSpeed;}
	void setIdleSpeed(float newSpeed) {idleSpeed = newSpeed;}
	void setNegativeSpeedFactor(float newFactor) {negativeFactor = newFactor;}
        GLboolean setTimeAttribute(PosableStateWrapper *wrapper, std::string name);
        GLboolean setHeadingAttribute(PosableStateWrapper *wrapper, std::string name) {return setAttribute(wrapper, name, headingAttribute, headingAttributeIndex);}
        GLboolean setPitchAttribute(PosableStateWrapper *wrapper, std::string name) {return setAttribute(wrapper, name, pitchAttribute, pitchAttributeIndex);}
        GLboolean setRollAttribute(PosableStateWrapper *wrapper, std::string name) {return setAttribute(wrapper, name, rollAttribute, rollAttributeIndex);}
        GLboolean setScaleAttribute(PosableStateWrapper *wrapper, std::string name) {return setAttribute(wrapper, name, scaleAttribute, scaleAttributeIndex);}
        GLboolean setAttribute(PosableStateWrapper *wrapper, std::string name, std::string&attName, int &attInd);
        GLboolean setTimeOffset(std::string offset);

	/*---Interaction functions---*/
	virtual int HandleInput(unsigned int inputType, InputInfo *info);
	virtual int animate();

	/*---Scripting functions---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---File Related functions---*/
    virtual Animator * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

  protected:
	float speed, idleSpeed, negativeFactor;
	double lastHeading, headingOffset;
	GLboolean respondToArrowKeys, allowSpeedChange, allowHeadingChange;
	
        int reportAttributeError(Tcl_Interp *interp, PosableStateWrapper *wrapper, std::string attribute);
	
	Object *stayAbove;
	float aboveAmount;
	double timeOffset, timeScale;
        std::string timeAttribute, headingAttribute, pitchAttribute, rollAttribute, scaleAttribute;
	int headingAttributeIndex, pitchAttributeIndex, rollAttributeIndex, scaleAttributeIndex;
	GLboolean invertScale, logScale, timeBoundsNeedsReset;
};

#endif
