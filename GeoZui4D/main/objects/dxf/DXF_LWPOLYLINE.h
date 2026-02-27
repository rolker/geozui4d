/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
