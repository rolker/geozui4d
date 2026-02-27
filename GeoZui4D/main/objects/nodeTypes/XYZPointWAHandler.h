/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef XYZPOINTWAHANDLER_H
#define XYZPOINTWAHANDLER_H

#include "objects/nodeTypes/SDV_DataHandler.h"
#include "objects/nodeTypes/XYZPointWA.h"
#include "SDV_types.h"
#include "objects/Distance.h"

class XYZPointWAHandler : public SDV_DataHandler
{
public:
    XYZPointWAHandler(double X, double Y);

    void Add(SDV_Data &s);

//    double GetAverage();
    void Draw();
    void Draw(double minx, double maxx, double miny, double maxy){};
    void writeTo(FILE * outfile);
private:
    double sum;
    double altSum;
    double weight;
    double x,y;
};
#endif
