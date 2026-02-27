/* Copyright (c) 2007 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef SRTM_H
#define SRTM_H


#include "engine/Object.h"

class Srtm : Object
{
    public:
        Srtm();
        ~Srtm();
        DERIVE_OBJECT(Srtm);

        void DrawLowRes(float degradation, Window3D *win);
        int DrawHighRes(int eye, Window3D *win);
        int Load();
    private:
        short * data;
        short * normals;
        float * positions;
        long dataCount;
        int hiresRow;
        float north,west;
        float cellsize;
};

#endif
