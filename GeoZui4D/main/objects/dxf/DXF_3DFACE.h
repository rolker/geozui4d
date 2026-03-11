// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_3DFACE_H
#define DXF_3DFACE_H

#include "DXF_entity.h"
///\ingroup DXFGroup
class DXF_3DFACE:public DXF_entity
{
    public:
        DXF_3DFACE();

        int readFrom(FILE *file);
        void Draw(float degradation, Window3D *win);
    private:
        std::vector<gz4d::Point<float> > points;
        gz4d::Vector<float,3> normal;
        bool fourthFlag;
};

#endif



