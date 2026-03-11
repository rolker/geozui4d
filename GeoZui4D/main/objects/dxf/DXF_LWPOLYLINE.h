// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_LWPOLYLINE_H
#define DXF_LWPOLYLINE_H

#include "DXF_entity.h"

///\ingroup DXFGroup
class DXF_LWPOLYLINE:public DXF_entity
{
    public:
        DXF_LWPOLYLINE();

        int readFrom(FILE *file);
        void Draw(float degradation, Window3D *win);

    private:
        std::vector<gz4d::Point<float> > points;
        int flags;
        float elevation;
};

#endif
