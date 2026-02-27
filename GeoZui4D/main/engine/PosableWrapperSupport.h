/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableWrapperSupport.h

class PosableWrapperSupport;

#ifndef POSABLEWRAPPERSUPPORT_H
#define POSABLEWRAPPERSUPPORT_H

#include "widgets/Sv_Standards.h"

class PosableStateWrapper;
class PosableStateBase;
struct PosableWrapperList;
class SmartValue;

/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for classes that support PosableStateWrapper
 *	attachments (usually these are non-scalar entities for which it would
 *	make just as much sense to have a posable for each position).
 *
 *	\author Matt Plumlee (mdp)
 *  \date 2/11/2002
 *  \image html wrapperDef.jpg
 *  \image html wrapperUse.jpg
 *  \image html wrapperSupport.jpg
 *  \image html wrapperEx.jpg
 */
/*---------------------------------------------------------------------------*/
class PosableWrapperSupport
{
  public:
	/*---Constructors/Destructors---*/
	PosableWrapperSupport();
	virtual ~PosableWrapperSupport();

	/*---Accessors---*/
	virtual GLboolean indexContentsModifiable(PosableStateWrapper *wrapper) = 0;
	virtual PosableStateBase *getPosable();

	int getNumAttributes(PosableStateWrapper *newWrapper);
        const char *getAttributeName(PosableStateWrapper *newWrapper, unsigned int attributeIndex);
        virtual int getAttributeIndex(PosableStateWrapper *newWrapper, std::string attributeName);
        virtual int getAttribute(PosableStateWrapper *newWrapper, SmartValue &val, unsigned int attributeIndex);

	/*---Mutators---*/
	virtual GLboolean attachWrapper(PosableStateWrapper *newWrapper);
	virtual GLboolean detachWrapper(PosableStateWrapper *wrapper);
	virtual GLboolean indexUpdated(PosableStateWrapper *wrapper);
	virtual GLboolean insertValue(PosableStateWrapper *wrapper, GLboolean beforeCurrent = GL_TRUE, int dim = 0);
	virtual GLboolean deleteValue(PosableStateWrapper *wrapper, int dim = 0);
	virtual int setAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);

	/*---Change facilitiation---*/
	virtual void wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask) = 0; //wrapper changed something
	virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask) = 0; //we're updating the wrapper

	/*---Associative Access---*/
	virtual GLboolean requestAssociativeAttribute(int attributeIndex) {return GL_FALSE;}
	virtual void releaseAssociativeAttribute(int attributeIndex) {}
	virtual GLboolean setWrapperIndexFromAttribute(PosableStateWrapper *wrapper, float val) {return GL_FALSE;}
	int getAssociativeAttribute() {return associativeAttribute;}
	virtual void resetTimeBoundsOnAttribute(PosableStateWrapper *wrapper, int indexingAttribute, double offset = 0.0, double scale = 1.0) {}

  protected:
	virtual PosableWrapperList *findWrapperNode(PosableStateWrapper *wrapper,
												GLboolean mustExist = GL_TRUE,
												GLboolean getParentNode = GL_FALSE) = 0;
        unsigned int numWrapableAttributes;
	const char **wrapableAttributeNames;
	int associativeAttribute;
};

struct PosableWrapperList
{
	PosableStateWrapper *wrapper;
	PosableWrapperList *next;

	PosableWrapperList();
	void addWrapper(PosableStateWrapper *newWrapper);
	GLboolean removeWrapper(PosableStateWrapper *wrapper, PosableWrapperList *parent = NULL);
	PosableWrapperList *findWrapper(PosableStateWrapper *wrapper);

	void notify(long changeMask = ChangeTranslate);
};

#endif
