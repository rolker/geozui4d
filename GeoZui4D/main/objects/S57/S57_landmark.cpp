// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// S57_landmark.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to visualize S57 landmarks (LNDMRK)
 *	
 *	Created by mdp 3/8/2005
 */
/*---------------------------------------------------------------------------*/
#include "engine/Window3D.h"
#include "S57_landmark.h"
#include "engine/PosableStateBase.h"
#include "engine/Scene3D.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"

REGISTER_OBJECT(S57_landmark,"");

char const *S57_landmark::landmarkCategories[] = {
	"", "cairn", "cemetery", "chimney", "dish Aerial", "flagstaff (flagpole)",
		"flare Stack", "mast", "windsock", "monument", "column (pillar)",
		"memorial plaque", "obelisk", "statue", "cross", "dome", "radar scanner",
		"tower", "windmill", "windmotor", "spire/minaret", "large rock/boulder"};

char const *S57_landmark::functions[] = {
	"", "", "harbour-master's office", "custom office", "health office",
		"hospital", "post office", "hotel", "railway station", "police station",
		"water-police station", "pilot office", "pilot lookout", "bank office",
		"headquarters for district control", "transit shed/warehouse", "factory",
		"power station", "administrative", "educational facility", "church",
		"chapel", "temple", "pagoda", "shinto shrine", "buddhist temple",
		"mosque", "marabout", "lookout", "communication", "television", "radio",
		"radar", "light support", "microwave", "cooling", "observation",
		"timeball", "clock", "control", "airship mooring", "stadium",
		"bus station"};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
S57_landmark::S57_landmark()
{
	pointSize = 5;
	function = category = 0;
	displayLabel = NULL;
	printOffset = 0;
}

/*---------------------------------------------------------------------------*/
S57_landmark::~S57_landmark()
{
	delete displayLabel;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void S57_landmark::DrawLowRes(float deg,Window3D * win)
{
/*
	const float *pos, *winpos;
	float distX, distY;
*/

	S57_object::DrawLowRes(deg, win);

        if (win)
        {
            double scale = win->getViewInfo()->getPosable()->getScale();
            if (scale <= scamin) //use = because we already nudged scamax & scamin for roundoff
                return;
        }

	glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 75.0 + height);
	glEnd();

	if (!displayLabel)
		return;

	if (!printOffset)
		printOffset = (int)(-win->printingWidth(displayLabel) / 2.0);
/*
	pos = pose->getPosition();
	winpos = win->getPoseInfo()->getPosition();
	distX = pos[X] - winpos[X];
	distY = pos[Y] - winpos[Y];
	if (distX * distX + distY * distY > 1000000.0) //only show within 1.0 kilometers...for now
*/
		win->printAt(0, 0, 75.0 + height, displayLabel, GL_TRUE, GLUT_BITMAP_HELVETICA_10, printOffset);
/*	else
		win->printOver(0, 0, 75.0 + height, displayLabel, GL_TRUE, GLUT_BITMAP_HELVETICA_10, printOffset);
*/
}

/*---------------------------------------------------------------------------*/
void S57_landmark::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
	S57_object::SetAttributes(item, info, objectNumber);

	// function
        if(item["FUNCTN"])
        {
	   function = item["FUNCTN"].GetValue<int>();
        }
        if(item["CATLMK"])
            category = item["CATLMK"].GetValue<int>();

	if (function > 0 || category > 0)
	{
		if (function < 0)
			function = 0;
		if (category < 0)
			category = 0;
		displayLabel = new char[strlen(functions[function]) + strlen(landmarkCategories[category]) + 2];
		sprintf(displayLabel, "%s %s", functions[function], landmarkCategories[category]);
	}
}

/*---------------------------------------------------------------------------*/
void S57_landmark::ShowS57ShapeInfo(Scene3D *scene)
{
	char infoLine[512];

    if (generateSpecialPurposeString(infoLine))
        scene->appendInfo(infoLine);
    //landmarks have no shape characteristics, but does have Function and Category
	if (function > 0)
	{
		sprintf(infoLine, "Function:\t%s\n", functions[function]);
		scene->appendInfo(infoLine);
	}
	if (category > 0)
	{
		sprintf(infoLine, "Category:\t%s\n", landmarkCategories[category]);
		scene->appendInfo(infoLine);
	}
}

