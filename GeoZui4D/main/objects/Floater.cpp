/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"Floater.h"
#include<iostream>
#include"CurrentLayer.h"
#include"engine/PosableStateBase.h"
#include<math.h>
#include "engine/Scene3D.h"




#include "utilities/SmartValue.h"

#include "engine/InputMediator.h"

using namespace std;

REGISTER_OBJECT(Floater, "floater");

#define FLOATER_STEPS 1000    
#define FLOATER_TAIL_LENGTH 120 // in time steps

static const char *FloaterAttributeNames[] = {OBJECT_ATTRIBUTES,"tailLength"};

Floater::Floater() : Object(GL_TRUE,0)
{
	current = NULL;
    tailLength =  FLOATER_TAIL_LENGTH;
	
	numAttributes = NumFloaterAttributes;
	attributeNames = FloaterAttributeNames;
}

Floater::~Floater()
{
}


void Floater::setTailLength(int tl)
{
	tailLength = tl;
}

/*---------------------------------------------------------------------------*/
int Floater::getAttribute(SmartValue &val, int attributeIndex)
{
	switch (attributeIndex)
	{
		case Attr_TailLength:
			val.set(tailLength);
			break;
		default:
			return Object::getAttribute(val,attributeIndex);
			break;
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int Floater::setAttribute(SmartValue &val, int attributeIndex)
{

	switch (attributeIndex)
	{
		case Attr_TailLength:
			tailLength = val.asInteger();
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
			break;
		default:
			return Object::setAttribute(val,attributeIndex);
			break;
	}
	return TCL_OK;
}

void Floater::Idle(double time)
{
	Object::Idle(time);
	
	if(lastIdleTime >= bbox.start && lastIdleTime <= bbox.end)
	{
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}


void Floater::DrawLowRes(float degradation, Window3D *win)
{
	double t,it;
	int p1,p2,startp;
	int i;
	float p;

        glPushAttrib(GL_POINT_BIT|GL_LINE_BIT|GL_DEPTH_BUFFER_BIT);
	if(lastIdleTime >= bbox.start && lastIdleTime <= bbox.end)
	{
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);

		p1 = (int)floor((lastIdleTime-bbox.start)/timestep);
		p2 = (int)ceil((lastIdleTime-bbox.start)/timestep);
		startp = p1 - tailLength;
		if(startp < 0)
			startp = 0;
		t = (lastIdleTime - (bbox.start + p1*timestep))/timestep;
		it = 1.0-t;

		glPointSize(10);
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_POINTS);
			glVertex3d(positions[p1][0]*it+positions[p2][0]*t,
					   positions[p1][1]*it+positions[p2][1]*t,
					   positions[p1][2]*it+positions[p2][2]*t);
		glEnd();

		glDepthFunc(GL_LEQUAL);
		glPointSize(5);
		glColor3f(1.0,1.0,0.0);
		glBegin(GL_POINTS);
			glVertex3f(positions[p1][0]*it+positions[p2][0]*t,
			           positions[p1][1]*it+positions[p2][1]*t,
			           positions[p1][2]*it+positions[p2][2]*t);
		glEnd();

		glLineWidth(5.0);
		glColor3f(1.0,0.0,0.0);
		glBegin(GL_LINE_STRIP);
			for(i=startp;i<=p1;i++)
			{
				glVertex3d(positions[i][0]*it+positions[i+1][0]*t,
						positions[i][1]*it+positions[i+1][1]*t,
						positions[i][2]*it+positions[i+1][2]*t);
			}
		glEnd();
		glLineWidth(2.5);
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_LINE_STRIP);
			for(i=startp;i<=p1;i++)
			{
				p = (i-startp)/(float)(p1-startp);
				glColor3f(p,p,1.0-p);
				glVertex3d(positions[i][0]*it+positions[i+1][0]*t,
						positions[i][1]*it+positions[i+1][1]*t,
						positions[i][2]*it+positions[i+1][2]*t);
			}
		glEnd();

	}
        glPopAttrib();
}

void Floater::setCurrent(CurrentLayer * c, double time)
{
	double t;
	int p;
	float u,v;


	
	current = c;
	t = bbox.start = time;
	bbox.end = current->GetBoundingBox().end;


	timestep = (bbox.end-t)/FLOATER_STEPS;

	positions = new Point3d[FLOATER_STEPS];
	positions[0][0]=0.0;
	positions[0][1]=0.0;
	positions[0][2]=current->getZ(GetPosition(0),GetPosition(1),t);
		
	for(t = time+timestep, p=1; p < FLOATER_STEPS; t+=timestep,p++)
	{
		current->getInterpolatedCurrent(GetPosition(0)+positions[p-1][0],GetPosition(1)+positions[p-1][1],t,u,v);
		positions[p][0]=positions[p-1][0]+u*timestep;
		positions[p][1]=positions[p-1][1]+v*timestep;
		positions[p][2]=current->getZ(GetPosition(1)+positions[p][1],GetPosition(0)+positions[p][0],t);
	}
	
}
