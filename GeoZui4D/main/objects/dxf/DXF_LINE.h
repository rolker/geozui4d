// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_LINE_H
#define DXF_LINE_H

#include "DXF_entity.h"

///\ingroup DXFGroup
class DXF_LINE:public DXF_entity
{
    public:
        int readFrom(FILE *file);
        void Draw(float degradation, Window3D *win);

    private:
        gz4d::Point<float> start, end;
};

#endif
