/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// CompositeObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides a way to represent aggregate reference frames built
 *	from multiple PosableStateBases.  This is an abstract class.
 *	This class leaves the following responsibilities up to derived classes.
 *		* Managing PosableStateBases by passing change event along to them
 *		* Handling pose-change notifications.
 *		* Rendering to the screen (this class implements an empty function)
 *		* Loading and saving from/to disk (this class only reads/stores
 *		  posableStateBase addresses).
 *	This class actually implements the following.
 *		* Keeping track of PosableStateBases for management
 *		* Removing a PosableStateBase from change notifications.
 *
 *	Created by mdp 6/21/2001
 */
/*---------------------------------------------------------------------------*/
class CompositeObject;

#ifndef COMPOSITEOBJECT_H
#define COMPOSITEOBJECT_H

#include "PosableStateRelationship.h"
#include "engine/Object.h"

class CompositeObject: public PosableStateRelationship, public Object
{
  public:
	CompositeObject(void *creator = NULL, int pickingTypes = PICK_CLICK1, GLboolean allocatePosable = GL_TRUE);
	virtual ~CompositeObject();

	/*---File-related---*/
	virtual int readFrom(FILE *file);
	virtual int processField(int option, char **fields);
	virtual int writeTo(FILE *file);

	/*---Interaction facilitation---*/
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged) = 0;

	virtual long manage(PosableStateBase *pose); //assigns operands
	virtual void relinquish(PosableStateBase *pose = NULL); //relinquish all operands if NULL
	virtual void replace(PosableStateBase *oldPose, PosableStateBase *newPose);
	virtual long numManaging();

	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---Accessor functions---*/
	virtual long getType(long fromWhom = FromBoth, PosableStateBase *associatedPose = NULL);
	virtual PosableStateBase *getPoseOfOwner(int index = -1);
	virtual PosableStateBase *getPoseOfTarget(int index = -1);

	/*---Rendering---*/
	virtual void DrawLowRes(float degradation, Window3D *win);
	virtual int DrawHighRes(int eye, Window3D *win); //return 1 if done, 0 if not done.
	virtual void ShowInfo(unsigned int *names = NULL, int numNames = 0);

  protected:
	  enum CompositeObjectParams {N_RECORDS = FirstObjectKeyword, FirstChildKeyword};

	PosableStateBase **poseOfOperands;
	int numOperands, numOperandsAllocated;
	long numManaged;

	long relationshipFromOperands;
	long relationshipToResult;

	void prepareListForMoreOperands();
};

#define COMPOSITE_OBJECT_NAMES \
	OBJECT_NAMES, "N_RECORDS"

#endif
