/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SDV_DATAHANDLER_H
#define SDV_DATAHANDLER_H
/*
SDV_DataHandler is used as an abstract data type for the SDV_Quadtree structure. 
Since the SDV_Quadtree structure is designed to contain datapoints, such
as soundings, at the leaf nodes, which have a preset resolution, it is
possible to have more than one datapoint assigned to a leaf node. This 
class is responsible for managing multiple datapoints assigned to a node,
which can also be seen as multiple soundings in a bin.
*/

#include<stdio.h>
#include"objects/nodeTypes/SDV_Data.h"

//class TimeSequence;
#include "utilities/TimeSequence.h"

class SDV_DataHandler
{
public:
    SDV_DataHandler();
    virtual ~SDV_DataHandler(){}
    virtual float GetAverage();
    virtual float GetAlternateData();
    virtual void Add(SDV_Data &d) = 0;
    virtual float GetAverage(double t);
    virtual float GetAlternateData(double t);
    virtual void Add(SDV_Data &d, double t);
    virtual void Draw() = 0;
    virtual void Draw(double minx, double maxx, double miny, double maxy) = 0;
    virtual void writeTo(FILE * outfile) = 0;
//    int IsNull() {return isnull;};
     double GetStartValidTime();
     double GetEndValidTime();
protected:
//    int isnull;
    float value;
    float alternateData;
     
     TimeSequence times;
};

#endif 
