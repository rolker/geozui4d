/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"DXF_layer.h"
#include "engine/ObjectPoseState.h"
#include "engine/SimpleObject.h"
#include "DXF_thing.h"
#include "DXF_entity.h"
#include "engine/ObjectCollection.h"
#include "engine/Window3D.h"
#include <gz4d/glo/PushAttrib.h>

REGISTER_OBJECT(DXF_layer, "");

DXF_layer::DXF_layer()
:SimpleObjectCollection(GL_FALSE)
{
	handle = 0;
	r = g = b = 1.0;
	
	ObjectPoseState * op = new ObjectPoseState(this);
	pose = op;
	
	op->ignoreHeightExaggeration();
	
}


void DXF_layer::setRGB(float red, float green, float blue)
{
	r = red;
	g = green;
	b = blue;
}

void DXF_layer::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    glPushMatrix();
	glLoadIdentity();
	parent->getPosable()->applyTransforms(win->getViewInfo());
	
	glColor3f(r,g,b);
	
	glPointSize(10);
	glBegin(GL_POINTS);
	glVertex3f(0.0,0.0,0.0);
	glEnd();
	
	SimpleObjectCollection::DrawLowRes(degradation,win);
	glPopMatrix();
}

int DXF_layer::DrawHighRes(int eye, Window3D *win)
{
	glColor3f(r,g,b);
	return SimpleObjectCollection::DrawHighRes(eye,win);
}

// documented in Object
int DXF_layer::readFrom(FILE * f)
{
	int i;
	
	while(groupCode!=0)
	{
		if(!processEntry())
		{
			switch(groupCode)
			{
			case 62:
				i = atoi(valueString);
				if(i>=0)
				{
					setRGB(red[i-1],green[i-1],blue[i-1]);
				}
				else
				{
					SetInvisible();
				}
				break;
			default:
				break;
			}
		}
		readNext();
	}
	if(!name.empty())
		Object::SetLabel(name);
	return 0;
}

