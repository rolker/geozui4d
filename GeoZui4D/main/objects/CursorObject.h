/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// CursorObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to display a cursor location in various ways.
 *	
 *
 *	Created by mdp 1/31/2003
 */
/*---------------------------------------------------------------------------*/
class CursorObject;

#ifndef CURSOROBJECT_H
#define CURSOROBJECT_H

#include "engine/Object.h"
#include "engine/InputInfo.h"
class Window3D;
class PosableStateLock;
class PosableStateBase;

class CursorObject : public Object
{
  public:
    CursorObject();
	DERIVE_OBJECT(CursorObject);

	//File-related functions
//    int readFrom(FILE *file); //read from a file
//	int processField(int option, char **fields);
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);

	void updateUsing(InputInfo *updateInfo);
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

	GLboolean toggleShowDistance() {showDistance ^= 1; return showDistance;}

	//Picking
        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

	//Reference-frame coupling
	void coupleTo(PosableStateBase *windowOrObject);

	//Scripting interface
//	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
//	virtual int list(Tcl_Interp *interp);

  protected:
	void computeEdges();

	//private data members go here
	  enum {FRONT_EDGE, BACK_EDGE, LEFT_EDGE, RIGHT_EDGE};
	Point3d edges[4];
	InputInfo info;
	GLboolean edgesComputed, pointValid, crosshairsValid, showDistance;

	PosableStateBase *currentWindowOrObject;
	PosableStateLock *lockToWindowOrObject;

//	enum CursorObjectParams {? = FirstObjectKeyword};
};

#endif
