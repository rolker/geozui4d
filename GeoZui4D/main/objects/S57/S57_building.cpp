#include "engine/Window3D.h"
#include"S57_building.h"
#include "engine/Scene3D.h"
#include "S57_landmark.h"
#include "engine/ViewManager.h"
#include "engine/PosableStateBase.h"
#include<iostream>

using namespace std;

REGISTER_OBJECT(S57_building,"");

S57_building::S57_building()
{
	height = 0.0;
	pointSize = 3.0;
	printOffset = 0;
        function = 0;
}

S57_building::~S57_building()
{
}

/*---------------------------------------------------------------------------*/
void S57_building::DrawLowRes(float deg,Window3D * win)
{
/*
	const float *pos, *winpos;
	float distX, distY;
*/
	S57_object::DrawLowRes(deg,win);

        if (win)
        {
            
            double scale = win->getViewInfo()->getPosable()->getScale();
            if (scale <= scamin) //use = because we already nudged scamax & scamin for roundoff
                return;
        }

	if(height>0.0)
	{
		glPushMatrix();
		glTranslated(0.0,0.0,height);
		S57_object::DrawLowRes(deg,win);
		glPopMatrix();
	}

	if (function > 0)
	{
		if (!printOffset)
			printOffset = int(-win->printingWidth(S57_landmark::functions[function]) / 2.0);
/*
		pos = pose->getPosition();
		winpos = win->getPoseInfo()->getPosition();
		distX = pos[X] - winpos[X];
		distY = pos[Y] - winpos[Y];
		if (distX * distX + distY * distY > 1000000.0) //only show within a kilometer...for now
*/
			win->printAt(0, 0, 10.0 + height, S57_landmark::functions[function], GL_TRUE, GLUT_BITMAP_HELVETICA_10, printOffset);
/*		else
			win->printOver(0, 0, 10.0 + height, S57_landmark::functions[function], GL_TRUE, GLUT_BITMAP_HELVETICA_10, printOffset);
*/
	}
}

/*---------------------------------------------------------------------------*/
void S57_building::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
	Point3f c = {0.4f, 0.3f, 0.2f};
	SetColour(c);

	S57_object::SetAttributes(item, info, objectNumber);

        gz4d::ogr::Point p = item.GetGeometry();
        if(!p && height <= 0)
		height = 3.0; // make all non-point buildings at least 3 meters tall.

	// function
        if(item["FUNCTN"])
	   function = item["FUNCTN"].GetValue<int>();
}

/*---------------------------------------------------------------------------*/
void S57_building::ShowS57Info(Scene3D *scene)
{
	char infoLine[512];
	if (function > 0)
	{
		sprintf(infoLine, "Function:\t%s\n", S57_landmark::functions[function]);
		scene->appendInfo(infoLine);
	}
}
