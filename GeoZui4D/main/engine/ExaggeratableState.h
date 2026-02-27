/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// ExaggeratableState.h
/*---------------------------------------------------------------------------*/

class ExaggeratableState;

#ifndef EXAGGERATABLE_STATE_H
#define EXAGGERATABLE_STATE_H


#include "engine/TranslatableState.h"
/** 
 *	This is the implementation for classes that have position and scale, but
 *	scale is an exaggeration in only the z direction.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/10/2002
 */
/*---------------------------------------------------------------------------*/
class ExaggeratableState : public virtual TranslatableState
{
  public:
	/*---Constructors/Destructors---*/
	ExaggeratableState(PosableStateUser *initialUser = NULL, const char *name = NULL);
	ExaggeratableState(PosableStateBase *toBeReplaced, GLboolean userCall = GL_TRUE);
	virtual ~ExaggeratableState();

	/*---Accessors---*/
	//Use the changeMask arguments to get at min, max, and preferred data
        virtual void getPoseInfo(gz4d::Point<double> &position, OrientationInfo &rot, double &scale, int changeMask = ChangeAll);
	virtual int supportMask(int changeMask = ChangeAll);

	virtual double getScale(int changeMask = ChangeScale);

	/*---Mutators---*/
	virtual void setScale(double sval, int changeMask = ChangeScale);

	virtual void scaleBy(double sval = 1.0, int changeMask = ChangeScale);

	/*---GL Transformations---*/
	virtual void applyTransforms(ViewManager * view = NULL);

  protected:
	float _scale, _homeScale;
};

#endif
