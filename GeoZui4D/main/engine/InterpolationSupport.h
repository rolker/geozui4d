// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// InterpolationSupport.h
/*---------------------------------------------------------------------------*/

class InterpolationSupport;

#ifndef INTERPOLATIONSUPPORT_H
#define INTERPOLATIONSUPPORT_H

#include "engine/PosableWrapperSupport.h"
#include "engine/Interactor.h"
class CubicSpline;
class InverseCubicSpline;
class ForwardCubicSpline;

/** 
 *	This is a PosableWrapperSupport that provides a way to index between
 *	integer indexed Point3d's.  
 *  \note Currently, only cubic splines and linear interpolation are supported.
 *	\author Matt Plumlee (mdp)
 *  \date 2/19/2002
 *  \image html wrapperSupport.jpg
 */
/*---------------------------------------------------------------------------*/
class InterpolationSupport : public PosableWrapperSupport
{
  public:
	/*---Constructors/Destructors---*/
	InterpolationSupport();
	virtual ~InterpolationSupport();

	/*---Accessors---*/
	virtual GLboolean indexContentsModifiable(PosableStateWrapper *wrapper);
	virtual int getAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);

	/*---Mutators---*/
        virtual void setNumLines(unsigned int nLines);
	virtual int setAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);
	virtual void initAttribSplineRepresentations();

	/*---Change facilitiation---*/
	virtual void wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask); //wrapper changed something
	virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask); //we're updating the wrapper
	virtual GLboolean insertValue(PosableStateWrapper *wrapper, GLboolean beforeCurrent = GL_TRUE, int dim = 0);
	virtual GLboolean deleteValue(PosableStateWrapper *wrapper, int dim = 0);

	/*---Associative Access---*/
        virtual GLboolean requestAssociativeAttribute(unsigned int attributeIndex);
        virtual void releaseAssociativeAttribute(unsigned int attributeIndex);
	virtual GLboolean setWrapperIndexFromAttribute(PosableStateWrapper *wrapper, float val);

	/*---Event notification---*/
	static unsigned int interpolationBoundsEvent() { return interpolationBoundsInputType->value;}
	enum whatType {PointType, LineType};

  protected:
	virtual PosableWrapperList *findWrapperNode(PosableStateWrapper *wrapper,
												GLboolean mustExist = GL_TRUE,
												GLboolean getParentNode = GL_FALSE);
        CubicSpline *getSplineRepresentation(unsigned int whichLine = 0);
        InverseCubicSpline *getInverseSplineRepresentation(unsigned int whichLine = 0);

	static const EventType *interpolationBoundsInputType;

	/*---Child class functions---*/
	virtual std::vector<gz4d::Point<double> > &getPoints(unsigned int whichLine = 0) = 0;
        virtual float *getAttributeValues(unsigned int whichLine = 0, int attributeIndex = -1) = 0;
	virtual int getNumPoints(unsigned int whichLine = 0) = 0;
	virtual GLboolean modifyPoint(unsigned int index, gz4d::Point<double> const &newValue, long &changeMask, unsigned int whichLine = 0);
        virtual GLboolean insertItem(int what, unsigned int index, unsigned int whichLine = 0) = 0;
        virtual GLboolean deleteItem(int what, unsigned int index, unsigned int whichLine = 0) = 0;

	PosableWrapperList **wrappers;
	CubicSpline **splineRepresentations;
	ForwardCubicSpline ***attribSplineRepresentations;
	InverseCubicSpline **invsplineRepresentations;
        unsigned int numLines, numAttachedToAttribute;
};

#endif
