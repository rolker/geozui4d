/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////

#include "engine/SimpleObjectCollection.h"
#include "engine/SimpleObject.h"
#include "engine/ObjectCollection.h"

SimpleObjectCollection::SimpleObjectCollection(GLboolean allocatePosable, int pickingTypes)
:Object(allocatePosable, pickingTypes)
{
	objects = NULL;
}

SimpleObjectCollection::~SimpleObjectCollection()
{
	SimpleObjectNodeType * c;
	while(objects)
	{
		c = objects->next;
		delete objects->object;
		delete objects;
		objects = c;
	}
}

void SimpleObjectCollection::DrawLowRes(float degradation, Window3D *win)
{
	SimpleObjectNodeType * c;
	
	c = objects;
	while(c)
	{
		c->object->Draw(degradation,win);
		c = c->next;
	}
}

int SimpleObjectCollection::DrawHighRes(int eye, Window3D *win)
{
	int i = 0;
	
	while(hiresCursor && i<10)
	{
		hiresCursor->object->Draw(0.0,win);
		hiresCursor = hiresCursor->next;
		i++;
	}
	
	if(hiresCursor)
		return 0;
	return 1;
}

void SimpleObjectCollection::ResetHighRes()
{
  hiresCursor=objects;
}

void SimpleObjectCollection::addObject(SimpleObject * object)
{
	SimpleObjectNodeType * newobj;
	
	bbox.merge(object->getBBox());
	if(parent)
		parent->setBBoxStale();
	
	newobj = new SimpleObjectNodeType;
	
	newobj->object = object;
	newobj->next = objects;
	objects = newobj;
}

bool SimpleObjectCollection::IsEmpty()
{
	if(objects)
		return false;
	return true;
}
