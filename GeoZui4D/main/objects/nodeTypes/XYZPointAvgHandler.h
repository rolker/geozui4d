// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef XYZPOINTAVGHANDLER_H
#define XYZPOINTAVGHANDLER_H

#include "objects/nodeTypes/SDV_DataHandler.h"
#include "objects/nodeTypes/XYZPointAvg.h"
#include "SDV_types.h"

class XYZPointAvgHandler : public SDV_DataHandler
{
public:
    XYZPointAvgHandler();

    void Add(SDV_Data &s);
    void Draw();
    void Draw(double minx, double maxx, double miny, double maxy);
    void writeTo(FILE * outfile);
protected:
private:
    double total;
    double altTotal;
    int count;
};
#endif
