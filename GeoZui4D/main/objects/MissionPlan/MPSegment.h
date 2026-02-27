/* Copyright (c) 2006 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPSEGMENT_H 
#define MPSEGMENT_H

#include "MPItemBase.h"
#include "objects/SegmentUser.h"

class MPSegmentUser;

/// \ingroup MissionPlanGroup
/// Generic segment
class MPSegment: public MPItemBase
{
public:
    MPSegment();
    MPSegment(int cid, gz4d::xml::Node cmd);
    DERIVE_OBJECT(MPSegment);

    virtual void DrawLowRes(float degradation, Window3D *win);
    void command(gz4d::xml::Node c, std::string const &cmd);
protected:
    
    MPSegmentUser *segmentUser;
    Point3d startPos,endPos;
private:
};

class MPSegmentUser: public SegmentUser
{
public:
    MPSegmentUser();
    DERIVE_OBJECT(MPSegmentUser);
    void setColour(float r, float g, float b, float er = -1.0, float eg = -1.0, float eb = -1.0);
    void setSize(float s);
    void setPositions(gz4d::Point<double> const &s, gz4d::Point<double> const &e);
private:
    Segment* segment;
};

#endif
