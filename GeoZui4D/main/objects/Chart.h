/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef CHART_H
#define CHART_H

#include "engine/Object.h"
#include <gz4d/glo/Texture.h>
#include <gz4d/Image.h>

//#define MAX_TILE_SIZE 1024

class Chart : public Object
{
    public: 
        Chart();
        ~Chart();

        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

        DERIVE_OBJECT(Chart);

        //int DrawHighRes(int eye, Window3D * win);
        void DrawLowRes(float degradation, Window3D * win);

        void Fuse(Object * other);
        double getResolution();
    private:

        std::shared_ptr<gz4d::glo::Texture> texture;
        gz4d::ImageRGBA8 image;

        unsigned int rows, columns;

        double xstep,ystep;
        
};

#endif
