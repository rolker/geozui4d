// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// RulerPost.cpp
// 
// Display a verical line with hash-marks on it.
//
// Created by mdp 3/3/2000
///////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "GZincludes.h"
//#include "MainWindow.h"
#include "engine/PosableStateBase.h"
#include "objects/RulerPost.h"
#include "engine/ViewManager.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ObjectCollection.h"
//#include "engine/Scene3D.h"
#include <string.h>

#define POSITION_STRING_LEN 42
#define POSITION_FORMAT " (%8.2f, %8.2f, %8.2f)"
#define LOCATION_CROSS_SIZE 4.0f
//#define LOCATION_POST_TOP 1000.0
#define LOCATION_POST_TOP (z < -1000.0? 0.0 : z + 1000.0)
#define LOCATION_POST_BOTTOM z

extern int dominantEye;
extern MainWindow * mainWindow;

REGISTER_OBJECT(RulerPost, "ruler");

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
RulerPost::RulerPost()
{
	position_label = NULL;
}

/*---------------------------------------------------------------------------*/
RulerPost::RulerPost(ObjectCollection * p, GLdouble xPos, GLdouble yPos, GLdouble zPos)
: x(xPos), y(yPos), z(zPos)
{
  char label[POSITION_STRING_LEN + 7];
  Point3d gr;

  position_label = new char[POSITION_STRING_LEN];
  gr[0] = (float) (xPos + p->GetPosition(0));
  gr[1] = (float) (yPos + p->GetPosition(1));
  gr[2] = p->GetPosition(2);
  sprintf(position_label, POSITION_FORMAT, gr[0], gr[1], zPos + gr[2]);
  SetPosition(gr);

  strcpy(label,"ruler ");
  strcat(label,position_label);
  SetLabel(label);

  //SetLocalOffset();
  
  std::cout << "Position " << label << std::endl;
}

/*---------------------------------------------------------------------------*/
RulerPost::~RulerPost()
{
	if (position_label)
	{
        delete position_label;
		position_label = NULL;
	}
}

/*---------------------------------------------------------------------------*/
/**	Display info about this object in the info window on the GUI.
 */
void RulerPost::ShowInfo(GLuint *names, int numNames)
{
	char infoLine[512];
	//	char *tmp = NULL;
	Scene3D *scene;

	scene = mainWindow->getScene3D();

	sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
	scene->appendInfo(infoLine);
	sprintf(infoLine, "GeoRef:\t%f, %f, %f\n", GetPosition(X), GetPosition(Y), GetPosition(Z));
	scene->appendInfo(infoLine);
	scene->appendInfo("--------------------\n");
}

/*----------------------------File Functions---------------------------------*/
/*---------------------------------------------------------------------------*/
//int Load(Point3f georef_p) {cout << "RulerPost::Load not implemented yet!\n";};

/*----------------------------Rendering Functions----------------------------*/
/*---------------------------------------------------------------------------*/
void RulerPost::DrawLowRes(float degradation, Window3D *win)
{   
	ViewManager *currentView = win->getViewInfo();
	float PixSize;
        gz4d::Point<double> centerPoint; //center of the widgets/workspace
	const GLint *viewport;

	PixSize = (float)currentView->getPixelSize();
	centerPoint = currentView->getReferencePoint();
	viewport = currentView->getViewport();

	glShadeModel(GL_FLAT);
	glBegin(GL_LINES);
	glNormal3f(0.0, 0.0, 1.0);
	//glColor3f(10.0, 10.0, 10.0);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(0.0, 0.0, LOCATION_POST_BOTTOM);
	glVertex3f(0.0, 0.0, LOCATION_POST_TOP);
	glVertex3f(LOCATION_CROSS_SIZE, 0.0, z + LOCATION_CROSS_SIZE);
	glVertex3f(-LOCATION_CROSS_SIZE, 0.0, z - LOCATION_CROSS_SIZE);
	glVertex3f(LOCATION_CROSS_SIZE, 0.0, z - LOCATION_CROSS_SIZE);
	glVertex3f(-LOCATION_CROSS_SIZE, 0.0, z + LOCATION_CROSS_SIZE);
	glVertex3f(0.0, LOCATION_CROSS_SIZE, z + LOCATION_CROSS_SIZE);
	glVertex3f(0.0, -LOCATION_CROSS_SIZE, z - LOCATION_CROSS_SIZE);
	glVertex3f(0.0, LOCATION_CROSS_SIZE, z - LOCATION_CROSS_SIZE);
	glVertex3f(0.0, -LOCATION_CROSS_SIZE, z + LOCATION_CROSS_SIZE);

	//hash marks
	int i, step, min, max;
	GLdouble viewSize, size;
	viewSize = PixSize * viewport[3] / 2; //vertical direction
	glNormal3f(0.0, 0.0, 1.0);
	for (step = 10; step >= 1; step /= 10)
	{
		if (step < PixSize)
			break;
		min = (int)centerPoint[Z] - (int)viewSize;
		max = (int)centerPoint[Z] + (int)viewSize;
		min -= min % step;
		max += step - (step % 10);
		if (min < z)
			min = (int)z;
		if (max > LOCATION_POST_TOP)
			max = (int)LOCATION_POST_TOP;
		for (i = min; i < max; i += step)
		{   
			if (i % 50 == 0)
				size = 10.0;
			else if (i % 10 == 0)
				size = 5.0;
			else if (i % 5 == 0)
				size = 1.0;
			else
				size = 0.5;

			glVertex3f(-size, 0.0, i);
			glVertex3f(size, 0.0, i);
			glVertex3f(0.0, -size, i);
			glVertex3f(0.0, size, i);
		}
		viewSize /= 2;
	}
	glEnd();
	doneFlag = 0;
}

/*---------------------------------------------------------------------------*/
int RulerPost::DrawHighRes(int eye, Window3D *win)
{   int i;
    float size;

	if (!doneFlag)
    {		
		glShadeModel(GL_FLAT);

		glBegin(GL_LINES);
		glColor3f(10.0, 10.0, 10.0);
		glVertex3f(0.0, 0.0, LOCATION_POST_BOTTOM);
		glVertex3f(0.0, 0.0, LOCATION_POST_TOP);
		glVertex3f(LOCATION_CROSS_SIZE, 0.0, z + LOCATION_CROSS_SIZE);
		glVertex3f(-LOCATION_CROSS_SIZE, 0.0, z - LOCATION_CROSS_SIZE);
		glVertex3f(LOCATION_CROSS_SIZE, 0.0, z - LOCATION_CROSS_SIZE);
		glVertex3f(-LOCATION_CROSS_SIZE, 0.0, z + LOCATION_CROSS_SIZE);
		glVertex3f(0.0, LOCATION_CROSS_SIZE, z + LOCATION_CROSS_SIZE);
		glVertex3f(0.0, -LOCATION_CROSS_SIZE, z - LOCATION_CROSS_SIZE);
		glVertex3f(0.0, LOCATION_CROSS_SIZE, z - LOCATION_CROSS_SIZE);
		glVertex3f(0.0, -LOCATION_CROSS_SIZE, z + LOCATION_CROSS_SIZE);

		//hash marks
		for (i = (int)LOCATION_POST_BOTTOM; i < (int)LOCATION_POST_TOP; i ++)
		{   if (i % 50 == 0)
			    size = 10.0;
			else if (i % 10 == 0)
				size = 5.0;
			else if (i % 5 == 0)
				size = 1.0;
			else
				size = 0.5;
		    glVertex3f(-size, 0.0, i);
		    glVertex3f(size, 0.0, i);
			glVertex3f(0.0, -size, i);
			glVertex3f(0.0, size, i);
		}
		glEnd();

		//display the location in big letters at the top or just above selected position.
		glColor3f(10.0, 10.0, 10.0);
		if (!win->printAt(0, 0, LOCATION_POST_TOP, position_label))
			win->printAt(0, 0, z + 10.0 * win->getPoseInfo()->getScale(), position_label);
	}

	if (eye != dominantEye && eye != MONO_EYE)
		return doneFlag;
	else
		return doneFlag = 1;
}

