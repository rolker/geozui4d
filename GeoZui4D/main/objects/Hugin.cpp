/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Hugin.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class displays the Hugin auv, alonf with a tail for it's position
 *	history.
 *
 *	Created by rja March 16, 2001
 */
/*---------------------------------------------------------------------------*/
#include "GZincludes.h"
//#include<GL/glew.h>
//#include <GL/glut.h>
//#include "MainWindow.h"
#include "engine/ObjectPoseState.h"
#include "Hugin.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h"

extern int dominantEye;

extern MainWindow *mainWindow;

REGISTER_OBJECT(Hugin, "");

/*---------------------------------------------------------------------------*/
Hugin::Hugin()
: Object(GL_FALSE, PICK_DOUBLE1 | PICK_CLICK1),
  tail(30, 0.5)
{
	char *tempname = new char[12];

//	oldPosition[X] = 0.0;
//	oldPosition[Y] = 0.0;
//	oldPosition[Z] = 0.0;
	sprintf(tempname, "Hugin_%d", id);
	pose = new ObjectPoseState(this, tempname);
	delete tempname;

	mainWindow->requestNewSubwindow(this, mainWindow);
}

/*---------------------------------------------------------------------------*/
// documented in Object
int Hugin::readFrom(FILE *infile)
{
	char string[160];
	Point3d gr;
	
	if(!fgets(string, 160, infile)) // get comment string, good to use for
        string[0] = 0;							    //identifying the object type,
							    //i.e. [SAMPLEOBJECT]

	//get the geographic X,Y,Z position of the object (mandatory)
	if(fscanf(infile, "%lf %lf %lf", &gr[X], &gr[Y], &gr[Z] ) == 3) 
	   SetPosition(gr);

	return 0; //returns 0 if ok.
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *	parameter informs the object to render itself more or less quickly.  A
 *	degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *	can be used to determine information such as pixel-size-in-meters, etc.
 */
void Hugin::DrawLowRes(float degradation, Window3D *win)
{
    doneFlag = 0;
    
//        glEnable(GL_LINE_SMOOTH); // Anti-alias lines...
//        glDisable(GL_LIGHTING);

	glColor3f(1.0f, 0.2f, 0.0f);
	glPushMatrix();
//		glTranslatef(0.0,5.0,0.0);
		glScalef(1.0,5.0,1.0);
		glutSolidSphere(1.0,10,10);
	glPopMatrix();
    glEnable(GL_LIGHTING);        
    glDisable(GL_LINE_SMOOTH);

    glPopMatrix();//get back to scene coordinates
	tail.DrawIn(win);
	glPushMatrix();
}

/*---------------------------------------------------------------------------*/
int Hugin::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0, NULL);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
	    doneFlag = 1;

	return 1;
}

/*--------------------------Picking Functions--------------------------------*/
/*---------------------------------------------------------------------------*/
int Hugin::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
	Window3D *window;
	RelationshipQueryResult *locks;

	if (pickType != (PICK_DOUBLE1))
		return Object::pickAction(pickType, inputInfo, names, hits);

	window = Window3D::cast(inputInfo->window);
	if (window)
	{
		locks = relationshipManager->getLocksBetween(pose, window->getPoseInfo());
		if (locks->numResults() == 0)
			new PosableStateLock(this, pose, window->getPoseInfo(),
								 PosableStateLock::LockDeltaRotate |
								 PosableStateLock::LockDeltaTranslate |
								 PosableStateLock::LockFromOwner);
		else //assume first one is good enough for us.
			delete locks->first();

		delete locks;

		return GL_TRUE;
	}
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Hugin::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    if ((whatChanged & ChangeTranslateMask) && pose)
        tail.newPosition(pose->getPosition());
    Object::PoseChanged(whichPose, whatChanged);
}
