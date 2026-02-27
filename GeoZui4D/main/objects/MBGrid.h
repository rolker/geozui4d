/* Copyright (c) 2010 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef MBGRID_H
#define MBGRID_H

#include "engine/Object.h"
#include <gz4d/data/Raster.h>
#include <gz4d/Point.h>
#include<gz4d/glo/BufferObject.h>

class MBGrid : Object
{
    public:
        MBGrid();
        ~MBGrid();
        DERIVE_OBJECT(MBGrid);

        void DrawLowRes(float degradation, Window3D *win);
        //int DrawHighRes(int eye, Window3D *win);
        int Load();

    private:
        bool firstDraw;
        gz4d::Raster<gz4d::Point<float> > grid;
        //gz4d::Raster<gz4d::Vector<float,3> > normals;

        std::shared_ptr<gz4d::glo::Buffer> abo;
        std::shared_ptr<gz4d::glo::Buffer> eabo;
        
        //std::vector<uint32_t> indecies;
        std::vector<GLsizei> counts;
        std::vector<GLvoid const*> indexOffsets;
};

#endif
