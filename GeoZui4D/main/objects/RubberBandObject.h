/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// RubberBandObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to illustrate a box for rubber-banding effects,
 *	initially created for selecting regions in GUTMs.
 *	
 *	Created by mdp 10/15/2004
 */
/*---------------------------------------------------------------------------*/
class RubberBandObject;

#ifndef RUBBERBANDOBJECT_H
#define RUBBERBANDOBJECT_H

#include "engine/Object.h"
#include "engine/PosableWrapperSupport.h"
class Window3D;

class RubberBandObject : public Object, public PosableWrapperSupport
{
  public:
    RubberBandObject();
    ~RubberBandObject();
	DERIVE_OBJECT(RubberBandObject);

	//File-related functions
//    int readFrom(FILE *file); //read from a file
//	int processField(int option, char **fields);
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);

	//Picking
    virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

	//Scripting interface
//	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
//	virtual int list(Tcl_Interp *interp);

	//access
	void setCorner(int index, float x, float y, float z);
        gz4d::Point<double> getCorner(int index);

	void setValidRange(float mx, float Mx, float my, float My, float step_amount = -1.0);
	void setStep(float step_amount) {step = step_amount;}
	void unsetValidRange() {useMinMax = GL_FALSE;}
	void setKeepPointsOnTop(GLboolean b = GL_TRUE);
	void setZchangeable(GLboolean c = GL_TRUE) {zChangeable = c;}

  protected:
	//Wrapper Support
	virtual GLboolean indexContentsModifiable(PosableStateWrapper *wrapper);
	virtual void wrapperChangedValue(PosableStateWrapper *wrapper, long &changeMask); //wrapper changed something
	virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask); //we're updating the wrapper
	virtual PosableWrapperList *findWrapperNode(PosableStateWrapper *wrapper,
												GLboolean mustExist = GL_TRUE,
												GLboolean getParentNode = GL_FALSE);

	//data members
	PosableStateBase *oppositeCorner;
	PosableWrapperList wrappers;
	int selectedCorner;
	float minX, maxX, minY, maxY, step;
	float verticalBoost;
	GLboolean zChangeable, useMinMax, keepPointsOnTop;
};

#endif
