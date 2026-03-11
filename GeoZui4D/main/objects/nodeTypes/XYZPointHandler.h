// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef XYZPOINTHANDLER_H
#define XYZPOINTHANDLER_H

#include "objects/nodeTypes/SDV_DataHandler.h"
#include "objects/nodeTypes/XYZPoint.h"
#include "SDV_types.h"
#include <list>


class XYZPointHandler : public SDV_DataHandler
{
public:
    XYZPointHandler();

    void Add(SDV_Data &s);

//	void AddNode(SoundingNode * s);
//    float GetAverage();
    void Draw();
    void Draw(double minx, double maxx, double miny, double maxy);
	void writeTo(FILE * outfile);
protected:
//    SoundingNode * Pop();
private:
    //SoundingNode * list;
    std::list<float> pointList;
    std::list<float> altList;
};
#endif
