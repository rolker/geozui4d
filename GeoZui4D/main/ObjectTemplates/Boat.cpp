// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Boat.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation that shows off several common
 *	techniques used to derive a GeoZui object.
 *
 *	Created by mdp 5/4/2005
 */
/*---------------------------------------------------------------------------*/
#include <gz4d/glo.h>
#include <GL/glut.h>
#include "MainWindow.h"
#include "engine/ObjectPoseState.h"
#include "Boat.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "utilities/DottedAddress.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include <string.h>
#include <stdlib.h>
#include "engine/Scene3D.h"
#include <filesystem>
#ifndef WIN32
#include <unistd.h>
#endif

extern MainWindow *mainWindow;

REGISTER_OBJECT(Boat, "boat");

static const char *BoatNames[] = {BOAT_NAMES, ""};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Boat::Boat()
: Object(GL_FALSE),
  tail(60, 0.5f)
{
	SetTypeAndVersion("BOAT", 1, 0); //Sets the type to BOAT and version to 1.0
	SetKeywordTable(BoatNames);
	pose = new ObjectPoseState(this); //because we want more than just translation

	courseMadeGood = new ObjectPoseState(this, "courseMadeGood");
	lockFromCMG = new PosableStateLock(this, pose);

	//Set up default boat dimensions
	width[BOW] = 2.0f;	width[BRIDGE] = 20.0f;
	width[HULL] = 25.0f; width[STERN] = 5.0f;

	length[BOW] = 30.0f; length[HULL] = 75.0f;
	length[STERN] = 20.0f; length[BRIDGE] = 40.0f;

	height[BOW] = 3.0f; height[STERN] = 15.0f;
	height[HULL] = height[BRIDGE] = 18.0f;

	//Modify Object defaults
	hideOutsideTimeBounds = false;
	interpolatePositions = 1;

	//Internal drawing parameters
	drawTail = false;
	drawCMG = false;
}

/*--------------------------------------------------------------------------*/
Boat::~Boat()
{
	delete courseMadeGood;
//	delete lockFromCMG;
}

/*--------------------------------------------------------------------------*/
void Boat::initializingLabel()
{
	pose->getAddress()->addChild(courseMadeGood->getAddress());
	lockFromCMG->publishName("CMGlock");
}

/*--------------------------------------------------------------------------*/
int Boat::processField(int option, char **fields)
{
    int i = 1;
    int sizeType = -1;

    switch(option)
    {
		case BOW_SIZE:
		case HULL_SIZE:
		case STERN_SIZE:
		case BRIDGE_SIZE:
			sizeType = option - BOW_SIZE + BOW;
			width[sizeType] = ConvertFloat(i);
			length[sizeType] = ConvertFloat(i);
			height[sizeType] = ConvertFloat(i);
			break;
		case SHADOW:
			shadow = Lookup(fields[i++]);
			break;
		default:
			return Object::processField(option, fields);
			break;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
Object * Boat::LoadXML(bpt::ptree const &xml, char * path, gz4d::netcdf::File ncfile)
{
	int i;
	const char *partNames[] = {"bow_size", "hull_size", "stern_size", "bridge_size"};
	Point3d gr;
	gr[2] = 0.0;
	double t;
	OrientationInfo orient;
	Point3d o;
	float bridgeNudge = 0.0;

	orient.type=typeYawPitchRoll;

	Object::LoadXML(xml,path, ncfile);

	for (i = BOW; i <= BRIDGE; i++)
		xml.getFloatTriple(width[i], length[i], height[i], partNames[i]);
	xml.getFloat(bridgeNudge, "bridge_nudge");
	
	xml.getBool(drawTail,"draw_tail");
	xml.getBool(drawCMG,"draw_cmg");
	calculateOffsets(bridgeNudge);
	
	xml.childNode();
	
	while(xml.hasCurrentNode())
	{
		if(xml.nodeName() =="Position")
		{	
			hideOutsideTimeBounds = true;
			xml.getDoubleTriple(gr,"position");
			xml.getDouble(t,"time");
			if(xml.getFloatTriple(o,"orientation"))
			{
				orient.axis[Roll] = o[0];
				orient.axis[Pitch] = o[1];
				orient.axis[Yaw] = o[2];
				SetPositionOrientation(gr[0],gr[1],gr[2],orient,t);
				calculateCOG=0;
			}
			else
			{
				SetPosition(gr[0],gr[1],gr[2],t);
			}
		}
		xml.nextNode();
	}

	return this;
}

/*---------------------------------------------------------------------------*/
// documented in Object
int Boat::readFrom(FILE *infile)
{
    //parse header
    if (Object::readFrom(infile))
        return 1;

	calculateOffsets();

	return 0; //returns 0 if ok.
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *	parameter informs the object to render itself more or less quickly.  A
 *	degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *	can be used to determine information such as pixel-size-in-meters, etc.
 */
void Boat::DrawLowRes(float degradation, Window3D *win)
{
	float scale;

	scale = 0.002f / win->getPoseInfo()->getScale(); //make the boat bigger in x & y if it would show up too small
	glPushMatrix(); //outer scale
	if (scale > 1.0 && MainWindow::getInterfaceType() == GeoNav3D)
		glScalef(scale, scale, 1.0);

	float heightBow = height[HULL] - height[BOW];
	float heightStern = height[HULL] - height[STERN];
	float dWidthBowHull = (width[HULL] - width[BOW]) / width[HULL];
	float dHeightBowHull = (height[HULL] - heightBow) / height[HULL];
//	float dWidthSternHull = (width[HULL] - width[STERN]) / width[HULL];
	float dHeightSternHull = (height[HULL] - heightStern) / height[HULL];

	glShadeModel(GL_FLAT);

	//draw the ship
	glEnable(GL_LIGHTING);        
    glColor3f(0.6f, 0.6f, 0.8f);
	glPushMatrix();
	  glScalef(0.5, 1.0, 1.0);
	  glTranslatef(0.0f, -offset[HULL], 0.0f);
    //  scale = 10 / sqrt(winScale);
	  //create the bow
	  glPushMatrix();
		glTranslatef(0.0, offset[BOW], 0.0);
		glBegin(GL_QUAD_STRIP); //my graphics card has trouble with GL_QUADS in line-mode (wireframe)
			//right-hand panel
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-width[HULL], 0.0, height[HULL]);
			glVertex3f(-width[BOW], length[BOW], height[HULL]);
			glNormal3f(dWidthBowHull - 1.0, dWidthBowHull, 0.0);
			glVertex3f(-width[HULL], 0.0, 0.0);
			glVertex3f(-width[BOW], length[BOW], heightBow);
			//bottom panel
			glNormal3f(0.0, dHeightBowHull, dHeightBowHull - 1.0);
			glVertex3f(width[HULL], 0.0, 0.0);
			glVertex3f(width[BOW], length[BOW], heightBow);
			//left-hand panel
			glNormal3f(1.0 - dWidthBowHull, dWidthBowHull, 0.0);
			glVertex3f(width[HULL], 0.0, height[HULL]);
			glVertex3f(width[BOW], length[BOW], height[HULL]);
			//top panel
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-width[HULL], 0.0, height[HULL]);
			glVertex3f(-width[BOW], length[BOW], height[HULL]);
		glEnd();
			//front panel
		glBegin(GL_QUAD_STRIP);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(-width[BOW], length[BOW], heightBow);
			glVertex3f(width[BOW], length[BOW], heightBow);
			glVertex3f(-width[BOW], length[BOW], height[HULL]);
			glVertex3f(width[BOW], length[BOW], height[HULL]);
		glEnd();
			//(no back panel)
	  glPopMatrix();
	  //create the hull
	  glPushMatrix();
		glTranslatef(0.0, offset[HULL], 0.0);
		glBegin(GL_QUAD_STRIP);
			//right-hand panel
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(-width[HULL], 0.0, 0.0);
			glVertex3f(-width[HULL], length[HULL], 0.0);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-width[HULL], 0.0, height[HULL]);
			glVertex3f(-width[HULL], length[HULL], height[HULL]);
			//top panel
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(width[HULL], 0.0, height[HULL]);
			glVertex3f(width[HULL], length[HULL], height[HULL]);
			//left-hand panel
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(width[HULL], 0.0, 0.0);
			glVertex3f(width[HULL], length[HULL], 0.0);
			//bottom panel
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(-width[HULL], 0.0, 0.0);
			glVertex3f(-width[HULL], length[HULL], 0.0);
		glEnd();
			//(no front or back panel)
	  glPopMatrix();
	  //create the stern
	  glPushMatrix();
		glTranslatef(0.0, offset[STERN], 0.0);
		glBegin(GL_QUAD_STRIP);
			//right-hand panel
			glNormal3f(dWidthBowHull - 1.0, -dWidthBowHull, 0.0);
			glVertex3f(-width[HULL], length[STERN], height[HULL]);
			glVertex3f(-width[STERN], 0.0, height[HULL]);
			glVertex3f(-width[HULL], length[STERN], 0.0);
			glVertex3f(-width[STERN], 0.0, heightStern);
			//bottom panel
			glNormal3f(0.0, -dHeightSternHull, dHeightBowHull - 1.0);
			glVertex3f(width[HULL], length[STERN], 0.0);
			glVertex3f(width[STERN], 0.0, heightStern);
			//left-hand panel
			glNormal3f(1.0 - dWidthBowHull, -dWidthBowHull, 0.0);
			glVertex3f(width[HULL], length[STERN], height[HULL]);
			glVertex3f(width[STERN], 0.0, height[HULL]);
			//top panel
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-width[HULL], length[STERN], height[HULL]);
			glVertex3f(-width[STERN], 0.0, height[HULL]);
		glEnd();
		glBegin(GL_QUAD_STRIP);
			//back panel
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-width[STERN], 0.0, height[HULL]);
			glVertex3f(width[STERN], 0.0, height[HULL]);
			glVertex3f(-width[STERN], 0.0, heightStern);
			glVertex3f(width[STERN], 0.0, heightStern);
		glEnd();
			//(no front panel)
	  glPopMatrix();
	  //create the bridge
	  glPushMatrix();
		glTranslatef(0.0, offset[BRIDGE], height[HULL]);
		glBegin(GL_QUAD_STRIP);
			//right-hand panel
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-width[BRIDGE], 0.0, 0.0);
			glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-width[BRIDGE], length[BRIDGE], 0.0);
			glVertex3f(-width[BRIDGE], length[BRIDGE], height[BRIDGE]);
			//front panel
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(width[BRIDGE], length[BRIDGE], 0.0);
			glVertex3f(width[BRIDGE], length[BRIDGE], height[BRIDGE]);
			//left-hand panel
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(width[BRIDGE], 0.0, 0.0);
			glVertex3f(width[BRIDGE], 0.0, height[BRIDGE]);
			//back panel
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-width[BRIDGE], 0.0, 0.0);
			glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
		glEnd();
			//top panel
		glBegin(GL_QUAD_STRIP);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-width[BRIDGE], length[BRIDGE], height[BRIDGE]);
			glVertex3f(width[BRIDGE], length[BRIDGE], height[BRIDGE]);
			glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
			glVertex3f(width[BRIDGE], 0.0, height[BRIDGE]);
		glEnd();
		//(no bottom panel)
	  glPopMatrix();
    glPopMatrix();
	glPopMatrix(); //outer scale

	//glDisable(GL_LINE_SMOOTH);
	glPushMatrix();
	glLoadIdentity();
		//draw the tail
		if (drawTail)
			tail.DrawIn(win);

		if (drawCMG)
		{
			//draw the CMG line
			glDisable(GL_LIGHTING);
			glLineWidth(2);
			glPushMatrix();
			courseMadeGood->applyTransforms();
			glColor3f(0.2f, 0.2f, 0.2f);
			glBegin(GL_LINES);
				glVertex3f(0.0, 0.0, 0.0);
				glVertex3f(0.0, 360.0, 0.0);
			glEnd();
			glLineWidth(4);
			glColor3f(0.8f, 0.8f, 0.8f);
			glBegin(GL_LINES);
				glVertex3f(0.0, 0.0, 0.0);
				glVertex3f(0.0, 360.0, 0.0);
			glEnd();
			glPopMatrix();
			glLineWidth(1);
		}
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

/*---------------------------------------------------------------------------*/
int Boat::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];
	char retString[20];

	if (argc >= 1)
	{
		if (!strcmp(message, "truncateTail"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "usage: truncateTail <index>", NULL);
				return TCL_ERROR;
			}
			tail.truncateTailAt(atoi(argv[1]));
			return TCL_OK;
		}
		if (!strcmp(message, "drawCMG"))
		{
			if (argc >= 2)
				drawCMG = atoi(argv[1]);
			sprintf(retString, "%d", drawCMG);
			Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
			return TCL_OK;
		}
		if (!strcmp(message, "drawTail"))
		{
			if (argc >= 2)
				drawTail = isTrue(argv[1]);
			sprintf(retString, "%d", drawTail);
			Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
			return TCL_OK;
		}
	}
	return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Boat::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "truncateTail drawTail drawCMG ", NULL);
	return Object::list(interp);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Boat::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
	const double *newPosition;
	Point3d newPos = {0.0f, 0.0f, 0.0f};
	double scale;

	if ((whatChanged & ChangeTranslateMask) && pose == whichPose)
	{
		newPosition = pose->getPosition();
		newPos[X] = newPosition[X];
		newPos[Y] = newPosition[Y];
		tail.newPosition(newPos);
		scale = pose->getScale();
		courseMadeGood->AccumulateNotifications();
		courseMadeGood->setScale(scale * scale * tail.getAverageCourse(3, courseMadeGood));
		courseMadeGood->ResumeSingularNotification();
	}
	Object::PoseChanged(whichPose, whatChanged);
}

/*---------------------------------------------------------------------------*/
void Boat::calculateOffsets(float bridge_nudge)
{
	const float *pos;

	offset[STERN] = 0.0f;
	offset[HULL] = offset[STERN] + length[STERN];
	offset[BOW] = length[HULL] + offset[HULL];
	offset[BRIDGE] = offset[BOW] / 4.0f + bridge_nudge;
	pos = pose->getPosition();
	courseMadeGood->setPosition(pos[X], pos[Y] + length[HULL] + length[BOW], pos[Z]);
	lockFromCMG->lock(PosableStateLock::LockLocalized | PosableStateLock::LockDeltaTranslate | PosableStateLock::LockRotateY | PosableStateLock::LockFromOwner, courseMadeGood);
}

/*---------------------------------------------------------------------------*/
/**	Display info about this object in the info window on the GUI.
 */
void Boat::ShowInfo(unsigned int *names, int numNames)
{
	char infoLine[512];
	Scene3D *scene;

	scene = mainWindow->getScene3D();

	sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className);
	scene->appendInfo(infoLine);
	sprintf(infoLine, "Name:\t%s\n", GetLabel());
	scene->appendInfo(infoLine);

	sprintf(infoLine, "SOG:\t%f\n", courseMadeGood->getScale());
	scene->appendInfo(infoLine);
	sprintf(infoLine, "COG:\t%f\n", courseMadeGood->getOrientationAxis()[Yaw]);
	scene->appendInfo(infoLine);

	if (GetFilename())
	{
		sprintf(infoLine, "File:\t%s\n", GetFilename().string());
		scene->appendInfo(infoLine);
	}

	sprintf(infoLine, "Last Pos:%f, %f, %f\n", GetPosition(X), GetPosition(Y), GetPosition(Z));
	scene->appendInfo(infoLine);
	scene->appendInfo("--------------------\n");
}
