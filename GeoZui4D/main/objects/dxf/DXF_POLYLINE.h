// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_POLYLINE_H
#define DXF_POLYLINE_H

#include "DXF_entity.h"

///\ingroup DXFGroup
class DXF_POLYLINE:public DXF_entity
{
    public:
        DXF_POLYLINE();
    
        int readFrom(FILE *file);
        void Draw(float degradation, Window3D *win);
    private:
        float elevation;
        int flags;
        int m,n;
        static bool warn;
        std::vector<gz4d::Vector<float,3> > normals;
        int firstTime;
            
        GLuint displayList;
            
};


#endif
