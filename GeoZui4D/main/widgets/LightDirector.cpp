/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// LightDirector.cpp
///////////////////////////////////////////////////////////////////////////////
/**
 *	This class implements a light direction interactor that displays light
 *	direction and allows the user to modify this direction interactively.
 *	This class implements the following responsibilities:
 *	*	Implement a light direction interactor as described above
 *
 *	Created by mdp 7/16/2001
 */
///////////////////////////////////////////////////////////////////////////////

#include<gz4d/glo.h>
#include <GL/glut.h>
#include "MainWindow.h"
#include "engine/PosableStateBase.h"
#include "widgets/Sv_Standards.h"
#include "LightDirector.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/SDV_Light.h"
#include "engine/Scene3D.h"

#define VSPHERE_DENOMINATOR 3.0
#define SCALE_OFFSET 0.00032f
#define PI 3.1415926535
extern MainWindow *mainWindow;

/*---------------------------------------------------------------------------*/
/**	Constructor
 */
LightDirector::LightDirector(Window3D *myParent, Point3d loc)
: Interactor("lightDirector"),
  Pickable(PICK_PRESS1)
{
	parent = myParent;
	light = myParent->getLight();
	state = Visible;
	targetRadius = 30.0;
	sourceRadius = 15.0;
	distanceBetween = 80.0;
	COPY_POINT(location, loc);
}

/*---------------------------------------------------------------------------*/
/**	Destructor
 */
LightDirector::~LightDirector()
{
}

/*---------------------------------------------------------------------------*/
void LightDirector::setVisible(GLboolean vis)
{
	if (vis)
		state |= Visible;
	else
		state &= ~Visible;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	HandleInput handles button 1 events.  It is assumed that 
 */
int LightDirector::HandleInput(unsigned int inputType, InputInfo *info)
{
    Window3D *currentWindow;
    ViewManager *currentView;
    PosableStateBase *currentPose, *lightPose;
    double centerX, centerY, centerZ;
    GLint viewport[4];
    GLdouble modelMatrix[16];
    Point3d newLightPos;
    float scaleFactor, sqrSum, sine, cosine, x, y, z;

    if (inputType & EnumeratedAction)
    {
        if (inputType == BreakAction)
            return REMOVE_FROM_STREAM;
        return PASS_EVENT_THROUGH;
    }

    /*---Handle idle event---*/
    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

    /*---Handle Mouse Button Events---*/
    if (inputType & MouseButton1)    //Is it a button1 event?
    {
        if (inputType & MouseButtonUp)
            return REMOVE_FROM_STREAM;
    }

    /*---Handle Mouse Motion Events---*/
    else if (inputType & MouseMotion)
    {
        currentWindow = Window3D::cast(info->window);
        if (!currentWindow)
            return PASS_EVENT_THROUGH;
        currentView = currentWindow->getViewInfo();
        currentPose = currentView->getPosable();
        lightPose = light->getPosable();
        glPushMatrix();
        gz4d::Point<double> orient = currentPose->getOrientationAxis();
        glRotatef(orient[X], 1.0, 0.0, 0.0);
        glScalef(SCALE_OFFSET *1.05f, SCALE_OFFSET*1.05f, SCALE_OFFSET*1.05f); //don't ask me why... (mdp 2/12/2004)
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
        gluProject(location[X], location[Y], location[Z], modelMatrix, projMatrix, viewport, &centerX, &centerY, &centerZ);
        glPopMatrix();
        //determine how far out to make the virtual sphere
        if (viewport[2] > viewport[3])
            scaleFactor = viewport[3] / VSPHERE_DENOMINATOR;
        else
            scaleFactor = viewport[2] / VSPHERE_DENOMINATOR;
        newLightPos[X] = (info->x.i - centerX) / scaleFactor;
        newLightPos[Y] = ((viewport[3] - info->y.i) - centerY) / scaleFactor;
        sqrSum = (newLightPos[X] * newLightPos[X] +
                newLightPos[Y] * newLightPos[Y]);
        if (sqrSum >= 1.0)
            newLightPos[Z] = 0.0;
        else
            newLightPos[Z] = 0.3f * sqrt(1.0 - sqrSum);
        sine = sin((90 - orient[X]) * PI / 180.0);
        cosine = cos((90 - orient[X]) * PI / 180.0);
        y = newLightPos[Y];
        z = newLightPos[Z];
        newLightPos[Y] = y * cosine - z * sine;
        newLightPos[Z] = y * sine + z * cosine;
        sine = sin(-orient[Y] * PI / 180.0);
        cosine = cos(-orient[Y] * PI / 180.0);
        x = newLightPos[X];
        y = newLightPos[Y];
        newLightPos[X] = x * cosine - y * sine;
        newLightPos[Y] = x * sine + y * cosine;

        lightPose->setPosition(newLightPos);
        return SINK_EVENT;
    }
    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	RemovalNoltify updates the internal state notion when removed
 *	from the interactor list.
 */
void LightDirector::RemovalNotify(InputMediator *fromList)
{
	 state &= ~Dragging;
}

/*---------------------------------------------------------------------------*/
int LightDirector::pickAction(Pickable::PickType pickType, InputInfo *info, GLuint *names, int hits)
{
	Window3D *currentWindow;

	currentWindow = Window3D::cast(info->window);
	if (!currentWindow)
		return GL_FALSE;

	state |= Dragging;
	currentWindow->inputMediator.PushInteractor(this);
	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void LightDirector::renderPickableObject(Window3D *inWindow)
{
	if (!mainWindow->getScene3D()->AreWidgetsVisible())
		return;

	if (parent != inWindow)
		return;

	glMatrixMode(GL_PROJECTION);
	glMultMatrixd(projMatrix);
	glMatrixMode(GL_MODELVIEW);
	DrawIn(inWindow);
}

/*---------------------------------------------------------------------------*/
void LightDirector::DrawIn(Window3D *inWindow)
{
	PosableStateBase *currentPose, *lightPose;
	gz4d::Point<double> lightPos;
	Point3d sourcePos;
	float len;
	
	if (!(state & Visible))
		return;

	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glColor3f(0.7f, 0.7f, 0.7f);
	currentPose = inWindow->getPoseInfo();
	lightPose = light->getPosable();
	lightPos = lightPose->getPosition();
	len = sqrt(lightPos[X] * lightPos[X] + lightPos[Y] * lightPos[Y] +
			   lightPos[Z] * lightPos[Z]);

	glLineWidth(2.0);
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	glScalef(SCALE_OFFSET, SCALE_OFFSET, SCALE_OFFSET);
	gz4d::Point<double> orient = currentPose->getOrientationAxis();
	glRotatef(orient[X], 1.0, 0.0, 0.0);
	glTranslatef(location[X], location[Y], location[Z]);
	glRotatef(orient[Y], 0.0, 1.0, 0.0);
	glPushMatrix();
	glRotatef(90.0, 1.0, 0.0, 0.0); // apply a rotation to the whole scene
	glScalef(1.0, -1.0, -1.0);     // invert y & z
	light->Draw();
	glPopMatrix();
	glScalef(1.0, 1.0, -1.0);
	glutSolidSphere(targetRadius, 8, 6);
	sourcePos[X] = distanceBetween * lightPos[X] / len;
	sourcePos[Y] = distanceBetween * lightPos[Z] / len;
	sourcePos[Z] = distanceBetween * lightPos[Y] / len;
	glColor3f(2.4f, 2.4f, 1.8f);
	glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3dv(sourcePos);
	glEnd();
	glTranslatef(sourcePos[X], sourcePos[Y], sourcePos[Z]);
	glutSolidSphere(sourceRadius, 8, 6);
	glPopMatrix();
	glLineWidth(1.0);
}
