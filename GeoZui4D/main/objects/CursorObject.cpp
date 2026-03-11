// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// CursorObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to display a cursor location in various ways.
 *	
 *
 *	Created by mdp 1/31/2003
 */
/*---------------------------------------------------------------------------*/
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "objects/CursorObject.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "utilities/DottedAddress.h"
#include "engine/PosableStateBase.h"
#include "engine/Scene3D.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "interactors/PickInteractor.h"
#include <string.h>

REGISTER_OBJECT(CursorObject, "cursor");

//static const char *CursorObjectNames[] = {OBJECT_NAMES, ""};

/*---------------------------------------------------------------------------*/
CursorObject::CursorObject()
{
	SetTypeAndVersion("CURSOR_OBJECT", 1, 0);
//	SetKeywordTable(CursorObjectNames);
	edgesComputed = GL_FALSE;
	info.window = NULL;
	pointValid = GL_FALSE;
	visible = GL_FALSE;
	currentWindowOrObject = NULL;
	crosshairsValid = GL_FALSE;
	lockToWindowOrObject = new PosableStateLock(this, pose);
	lockToWindowOrObject->publishName("lockToWindowOrObject");
	lockToWindowOrObject->setDeleteThreshold(1);
	showDistance = 1;
}

/*--------------------------------------------------------------------------*/
/*
int CursorObject::processField(int option, char **fields)
{
    int i = 1;
    int sizeType = -1;

    switch(option)
    {
      default:
            return Object::processField(option, fields);
            break;
    }
    return 0;
}
*/

/*---------------------------------------------------------------------------*/
// documented in Object
/*
int CursorObject::readFrom(FILE *infile)
{
    //parse header
    if (Object::readFrom(infile))
        return 1;

	return 0; //returns 0 if ok.
}
*/

/*---------------------------------------------------------------------------*/
/** This function renders the cursor in the passed window.
 */
void CursorObject::DrawLowRes(float degradation, Window3D *inWin)
{
	doneFlag = 0;
	Point3f winPos;
	
	float adder, dist;
	char distanceString[32];
	int i;

	if (!visible)
		return;

	if (!edgesComputed)
	{
		computeEdges();
		ToglWindow::SetTogl(inWin->GetTogl());
	}
	
	int boo;
	if(renderingForPicking)
		boo = 2;

        glPushAttrib(GL_POINT_BIT);

	glDisable(GL_LIGHTING);        
	glLineWidth(1.0);
	if (pointValid)
	{
		//draw a little point
		glPointSize(4.0);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POINTS);
			glVertex3f(0.0,0.0,0.0);
		glEnd();
		//now its outline
		glPointSize(6.0);
		glColor3f(0.3f, 0.4f, 0.5f);
		glBegin(GL_POINTS);
			glVertex3f(0.0,0.0,0.0);
		glEnd();
		glPointSize(1.0);
		//draw vertical line
		glColor3f(1.0f, 1.0f, 1.0f);
		adder = 10000.0 / inWin->getPoseInfo()->getScale();
		glBegin(GL_LINES);
			glVertex3f(0.0,0.0,-adder);
			glVertex3f(0.0,0.0, adder);
		glEnd();
		glColor4f(0.5f, 0.5f, 0.2f, 0.5f);
		glLineWidth(3.0);
		glBegin(GL_LINES);
			glVertex3f(0.0,0.0,-adder);
			glVertex3f(0.0,0.0, adder);
		glEnd();
		glLineWidth(1.0);

		if (showDistance)
		{
			//calculate and print the distance from workspace center to cursor (added by mdp 12/21/2004)
			DIFF_POINT(inWin->getPoseInfo()->getPosition(),pose->getPosition(),winPos);
                        std::cerr << CERR_POINT(winPos) << std::endl;
			dist = 0.0;
			for (i = X; i <= Z; i++)
				dist += winPos[i] * winPos[i];
			dist = (float)sqrt(dist);
			if (dist < 100.0)
				sprintf(distanceString, "Cursor Dist.: %.2f m", dist);
			else
				sprintf(distanceString, "Cursor Dist.: %.0f m", dist);
			//glPopMatrix(); //get back to world coordinates;
			glColor3f(0.3f, 0.9f, 0.5f);
			if(!inWin->printAt(0.0, 0.0, 0.0, distanceString, GL_TRUE, GLUT_BITMAP_HELVETICA_10, 5, 5))
				inWin->printAt(winPos[X], winPos[Y], winPos[Z], distanceString, GL_TRUE, GLUT_BITMAP_HELVETICA_10, 10, 10);
			glBegin(GL_LINES);
				glVertex3f(0.0,0.0,0.0);
				glVertex3f(winPos[X], winPos[Y], winPos[Z]);
			glEnd();
			//glPushMatrix();
		}
	}
	
	Point3f lEdges[4];
	for(i=0;i<4;i++)
		DIFF_POINT(edges[i],pose->getPosition(),lEdges[i]);
	
	//get back to world coordinates and draw crosshairs
	if (crosshairsValid)
	{
		glColor3f(0.6f, 0.8f, 1.0f);
		glBegin(GL_LINES);
			glVertex3fv(lEdges[FRONT_EDGE]);
			glVertex3fv(lEdges[BACK_EDGE]);
		glEnd();
		glColor3f(0.3f, 0.4f, 0.5f);
		glBegin(GL_LINES);
			glVertex3fv(lEdges[LEFT_EDGE]);
			glVertex3fv(lEdges[RIGHT_EDGE]);
		glEnd();
		glColor4f(0.2f, 0.5f, 0.5f, 0.5f);
		glLineWidth(3.0);
		glBegin(GL_LINES);
			glVertex3fv(lEdges[FRONT_EDGE]);
			glVertex3fv(lEdges[BACK_EDGE]);
		glEnd();
		glBegin(GL_LINES);
			glVertex3fv(lEdges[LEFT_EDGE]);
			glVertex3fv(lEdges[RIGHT_EDGE]);
		glEnd();
		glLineWidth(1.0);
	}
	glEnable(GL_LIGHTING);
        glPopAttrib();
}

/*---------------------------------------------------------------------------*/
int CursorObject::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0, win);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
	    doneFlag = 1;

	return 1;
}

/*---------------------------------------------------------------------------*/
int CursorObject::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
	if (pickType == PICK_CLICK1)
	{
		if (ToglWindow::getMouseTracking())
		{
			coupleTo(NULL);
			visible = GL_FALSE; //hide us and pick again
			PickInteractor::getInstance()->rePick(pickType, inputInfo);
			visible = GL_TRUE;
			if (!currentWindowOrObject)
				ToglWindow::toggleMouseTracking();
		}
		else
			ToglWindow::toggleMouseTracking();
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void CursorObject::updateUsing(InputInfo *updateInfo)
{
	if (updateInfo)
	{
		edgesComputed = GL_FALSE;
		info.window = updateInfo->window;
		info.x.i = updateInfo->x.i;
		info.y.i = updateInfo->y.i;

		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
	else
	{
		edgesComputed = GL_TRUE;
		info.window = NULL;
		crosshairsValid = GL_FALSE;
	}
}

/*---------------------------------------------------------------------------*/
void CursorObject::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
	edgesComputed = GL_FALSE;
	Object::PoseChanged(whichPose, whatChanged);
}

/*---------------------------------------------------------------------------*/
void CursorObject::computeEdges()
{
    gz4d::Point<double> worldPos, screenPos;
	Window3D *win;
	ViewManager *view;
	GLdouble depth;

	win = Window3D::cast(info.window);
	if (!win)
		return;
	view = win->getViewInfo();
	coupleTo(view->getPosable());

	//Find an intersection of the ray from the eye to a surface.
	ToglWindow::SetTogl(win->GetTogl());
	if (!win->getSelectedLocation(&info, worldPos, &depth, GL_FALSE))
		pointValid = GL_FALSE;
	else
	{
//		lockToWindowOrObject->unlock(PosableStateLock::LockDeltaTranslate);
		pose->setPosition(worldPos[X], worldPos[Y], worldPos[Z]);
//		lockToWindowOrObject->restorePreviousLock();
		pointValid = GL_TRUE;
	}

	//Find the ray from the eye through the cursor
	screenPos[X] = info.x.i;
	screenPos[Y] = info.y.i;
	screenPos[Z] = 0.0;
	view->getWorldPoint(screenPos, worldPos);
	COPY_POINT(edges[FRONT_EDGE], worldPos);
	screenPos[Z] = 1.0;
	view->getWorldPoint(screenPos, worldPos);
	COPY_POINT(edges[BACK_EDGE], worldPos);
	//Find the line that happens to be orthogonal to both.
	screenPos[Z] = depth;
	screenPos[X] = 0;
	view->getWorldPoint(screenPos, worldPos);
	COPY_POINT(edges[LEFT_EDGE], worldPos);
	screenPos[X] = view->getViewport()[2] - 1;
	view->getWorldPoint(screenPos, worldPos);
	COPY_POINT(edges[RIGHT_EDGE], worldPos);

	crosshairsValid = GL_TRUE;
	edgesComputed = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void CursorObject::coupleTo(PosableStateBase *windowOrObject)
{
	if (windowOrObject == currentWindowOrObject)
		return;

	if (windowOrObject == NULL)
	{
		lockToWindowOrObject->unlock(PosableStateLock::LockDeltaTranslate |
					   PosableStateLock::LockFromTarget | PosableStateLock::Dynamic |
					   PosableStateLock::LockLocalized);
		lockToWindowOrObject->relinquish(currentWindowOrObject);
		currentWindowOrObject = NULL;
	}
	else
	{
		if (currentWindowOrObject)
			coupleTo(NULL);

		currentWindowOrObject = windowOrObject;
		lockToWindowOrObject->lock(PosableStateLock::LockDeltaTranslate |
				   PosableStateLock::LockFromTarget | PosableStateLock::Dynamic |
				   PosableStateLock::LockLocalized, currentWindowOrObject);
	}
}


/*---------------------------------------------------------------------------*/
/*
int CursorObject::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];

	if (argc >= 1)
	{
		if (!strcmp(message, "whatever"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "usage: whatever", NULL);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return Object::perform(argc, argv, interp);
}
*/

/*---------------------------------------------------------------------------*/
/*
int CursorObject::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "whatever ", NULL);
	return Object::list(interp);
}
*/
