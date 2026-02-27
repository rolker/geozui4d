/* Copyright (c) 2006 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "MPSegment.h"
#include "engine/Window3D.h"
#include "objects/Segment.h"


using namespace std;

REGISTER_OBJECT(MPSegment, "");

MPSegment::MPSegment()
{
}

MPSegment::MPSegment(int cid, gz4d::xml::Node cmd)
:MPItemBase(cid, cmd)
{
    segmentUser = new MPSegmentUser();
    AddObject(segmentUser,false);
}

void MPSegment::DrawLowRes(float degradation, Window3D *win)
{
    ObjectCollection::DrawLowRes(degradation, win);
}


void MPSegment::command(gz4d::xml::Node c, string const &cmd)
{
    if(!inConstructor)
        MPItemBase::command( c,cmd);

    Point3d p[2];
/*    c.childNode();
    while(c.hasCurrentNode())
    {
        if(c.nodeName() == "position")
        {
            int id;
            if (c.getInt( id,"id"))
            {
                c.getDouble(p[id][X],"x");
                c.getDouble(p[id][Y],"y");
                c.getDouble(p[id][Z],"z");
            }
        }
        c.nextNode();
    }
    c.parentNode();*/
    SetPosition( p[0]);
    segmentUser->setPositions( p[0],p[1]);

}

REGISTER_OBJECT(MPSegmentUser, "");

MPSegmentUser::MPSegmentUser() : SegmentUser()
{
    setColour(0.8,0.8,0.8);
    setSize(0.15);
    segment = newSegment();
}

void MPSegmentUser::setSize(float s)
{
    startHeight = startWidth = endHeight = endWidth = s;
}

void MPSegmentUser::setPositions(gz4d::Point<double> const &s, gz4d::Point<double> const &e)
{
    segment->setPositions(s - GetPosition(), e-GetPosition());
}

void MPSegmentUser::setColour(float r, float g, float b, float er, float eg, float eb)
{
    *startColor = gz4d::Color(r,g,b);
    
    if (er>=0.0)
        (*endColor)[0]=er;
    else
        (*endColor)[0]=r;
        
    if (eg>=0.0)
        (*endColor)[0]=eg;
    else
        (*endColor)[0]=g;
        
    if (eb>=0.0)
        (*endColor)[0]=eb;
    else
        (*endColor)[0]=b;
}
