/* Copyright (c) 2006 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "MPPoint.h"
#include <GL/glut.h>
#include "MPCommand.h"


using namespace std;

REGISTER_OBJECT(MPPoint, "");

MPPoint::MPPoint()
{
}

MPPoint::MPPoint(int cid, gz4d::xml::Node cmd, int pickType)
:MPItemBase(cid,cmd,pickType)
{
    inConstructor = true;
    MPPoint::command(cmd, string());
    inConstructor = false;
}

void MPPoint::DrawLowRes(float degradation, Window3D *win)
{
    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT);
    if (((MPCommand*)parent)->isSelectedCmd())
        glColor4fv(selectedColor);
    else
        glColor4fv(color);
    glDisable(GL_LIGHTING);
    glPointSize(12);
    glBegin(GL_POINTS);
    glVertex3d(0.0,0.0,0.0);
    glEnd();
    glutSolidSphere(size,10,10);
    glPopAttrib();
}

void MPPoint::command(gz4d::xml::Node c, string const &cmd)
{
    if(!inConstructor)
        MPItemBase::command(c, cmd);

/*    c.childNode();
    while(c.hasCurrentNode())
    {
        if(c.nodeName() == "position")
        {
            double x,y,z;
            c.getDouble(x,"x");
            c.getDouble(y,"y");
            c.getDouble(z,"z");
            SetPosition(x,y,z);
        }
        c.nextNode();
    }
    c.parentNode();*/
}


