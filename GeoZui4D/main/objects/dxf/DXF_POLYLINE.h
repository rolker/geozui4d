/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
