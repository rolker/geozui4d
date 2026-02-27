#include "engine/Window3D.h"
#include"S57_sounding.h"
#include"S57.h"
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/glo/Vertex.h>

#include<iostream>


using namespace std;

REGISTER_OBJECT(S57_sounding,"");

/*---------------------------------------------------------------------------*/
S57_sounding::S57_sounding()
{
    pointSize = 3;
}

/*---------------------------------------------------------------------------*/
S57_sounding::~S57_sounding()
{
}

/*---------------------------------------------------------------------------*/
void S57_sounding::DrawLowRes(float deg,Window3D * win)
{

        gz4d::glo::PushAttrib attrib(GL_POINT_BIT|GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
        glColor3f(S57::dimFactor()*colour[0],S57::dimFactor()*colour[1],S57::dimFactor()*colour[2]);
        for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
        {
            win->printOver((*p)[X], (*p)[Y], (*p)[Z], sounding[p-exteriorRing.begin()], GL_TRUE, GLUT_BITMAP_HELVETICA_12, -8);
            win->printOver((*p)[X], (*p)[Y], (*p)[Z], decimal[p-exteriorRing.begin()], GL_TRUE, GLUT_BITMAP_HELVETICA_10, xOffset[p-exteriorRing.begin()] - 8, -8);
	}
	glPointSize(pointSize);
	glBegin(GL_POINTS);
        for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
        {
            gz4d::glo::Vertex(*p);
        }
	glEnd();
}

/*---------------------------------------------------------------------------*/
void S57_sounding::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
	S57_object::SetAttributes(item, info, objectNumber);
        for(std::vector<gz4d::Point<float> >::iterator p = exteriorRing.begin(); p != exteriorRing.end(); ++p)
	{
		int depth = (int) -((*p)[Z] * 10.0);
                sounding.push_back(boost::lexical_cast<std::string>(depth/10));
                decimal.push_back(boost::lexical_cast<std::string>(depth % 10));
		xOffset.push_back(ToglWindow::printingWidth(sounding.back(), GLUT_BITMAP_HELVETICA_12));
	}
}
