/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef XYZPOINTMINHANDLER_H
#define XYZPOINTMINHANDLER_H

#include "objects/nodeTypes/SDV_DataHandler.h"
#include "objects/nodeTypes/XYZPointMin.h"
#include "SDV_types.h"

class XYZPointMinHandler : public SDV_DataHandler
{
public:
    XYZPointMinHandler();

    void Add(SDV_Data &s);
    void Draw();
    void Draw(double minx, double maxx, double miny, double maxy);
	void writeTo(FILE * outfile);
protected:
private:
};
#endif
