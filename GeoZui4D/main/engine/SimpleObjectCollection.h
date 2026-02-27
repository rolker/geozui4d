/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////

// This class is a base for an object composed of SimpleObjects. 

#ifndef SIMPLEOBJECTCOLLECTION_H
#define SIMPLEOBJECTCOLLECTION_H

#include "engine/Object.h"

class SimpleObject;

class SimpleObjectCollection : public Object
{
public:
	SimpleObjectCollection(GLboolean allocatePosable = GL_TRUE, int pickingTypes = PICK_CLICK1 | PICK_PRESS3);
	~SimpleObjectCollection();
	
        virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits) {if (pickType == PICK_PRESS2) return GL_FALSE; else return Object::pickAction(pickType, inputInfo, names, hits);}

	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	void ResetHighRes();
	
	void addObject(SimpleObject * object);
	bool IsEmpty();
protected:

	struct SimpleObjectNodeType
	{
		SimpleObject * object;
		SimpleObjectNodeType * next;
	};
	
	SimpleObjectNodeType * objects;
	SimpleObjectNodeType * hiresCursor;
	
};


#endif
