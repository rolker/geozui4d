/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// SampleObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation, for use as a template in creating
 *	new objects
 *
 *	Created by mdp 12/13/2000
 */
/*---------------------------------------------------------------------------*/
#include "SampleObject.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h" 

extern int dominantEye;

REGISTER_OBJECT(SampleObject, "sample");

/*---------------------------------------------------------------------------*/
SampleObject::SampleObject()
: Object(GL_FALSE),
  tail(30, 2.0)
{
	int i;

	for (i = 0; i < SMOOTH_AMOUNT; i++)
	{
		oldPosition[i][X] = 0.0;
		oldPosition[i][Y] = 0.0;
		oldPosition[i][Z] = 0.0;
	}
	pose = new ObjectPoseState(this);
}

/*---------------------------------------------------------------------------*/
// documented in Object
int SampleObject::readFrom(FILE *infile)
{
	char string[160];
	Point3d Position;

	fgets(string, 160, infile); // get comment string, good to use for
							    //identifying the object type,
							    //i.e. [SAMPLEOBJECT]

	//get the geographic X,Y,Z position of the object (mandatory)
	fscanf(infile, "%lf %lf %lf", &Position[0], &Position[1], &Position[2] ); 

	//set the geoOffset for use in data fusion (mandatory)
	SetPosition(Position);

	return 0; //returns 0 if ok.
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *	parameter informs the object to render itself more or less quickly.  A
 *	degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *	can be used to determine information such as pixel-size-in-meters, etc.
 */
void SampleObject::DrawLowRes(float degradation, Window3D *win)
{
    doneFlag = 0;

    glEnable(GL_LINE_SMOOTH); // Anti-alias lines...
    glDisable(GL_LIGHTING);

	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINES);
	glVertex3f(5.0, 0.0, 0.0);
	glVertex3f(-5.0, 0.0, 0.0);
	glVertex3f(0.0, 10.0, 0.0);
	glVertex3f(0.0, -10.0, 0.0);
	glVertex3f(-2.0, 8.0, 0.0); //arrowHead
	glVertex3f(0.0, 10.0, 0.0); //arrowHead
	glVertex3f(2.0, 8.0, 0.0); //arrowHead
	glVertex3f(0.0, 10.0, 0.0); //arrowHead
	glVertex3f(0.0, -8.0, 2.0); //arrow tail
	glVertex3f(0.0, -8.0, -2.0); //arrow tail
	glEnd();

    glEnable(GL_LIGHTING);        
    glDisable(GL_LINE_SMOOTH);

	if (win)
	{
	    glPopMatrix(); //draw tail in scene coordinates.
		tail.DrawIn(win);
		glPushMatrix();
	}
}

/*---------------------------------------------------------------------------*/
int SampleObject::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0, NULL);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
	    doneFlag = 1;

	return 1;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void SampleObject::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
	const double *newPosition;
//	Point3f diff, composite;
//	float length, hypsqr;
//	int dim, i;

	if ((whatChanged & ChangeTranslateMask) && pose)
	{
		newPosition = pose->getPosition();
		tail.newPosition(newPosition);
		/*
		hypsqr = 0.0;
		for (dim = X; dim <= Z; dim++)
		{
			composite[dim] = 0.0;
			for (i = 0; i < SMOOTH_AMOUNT; i++)
				composite[dim] += oldPosition[i][dim];
			composite[dim] /= SMOOTH_AMOUNT;
			diff[dim] = newPosition[dim] - composite[dim];
			hypsqr += diff[dim] * diff[dim];
		}
		length = sqrt(hypsqr);
		pose->setOrientation(-asin(diff[Z]/length) * 180 / M_PI, atan2(diff[X], diff[Y]) * 180 / M_PI);
		for (i = 1; i < SMOOTH_AMOUNT; i++)
		{
			COPY_POINT(oldPosition[i - 1], oldPosition[i]);
		}
		COPY_POINT(oldPosition[i - 1], newPosition);
		*/
	}
	Object::PoseChanged(whichPose, whatChanged);
}
