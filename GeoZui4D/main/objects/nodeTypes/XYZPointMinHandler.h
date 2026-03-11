// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
