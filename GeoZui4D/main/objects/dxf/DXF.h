/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef DXF_H
#define DXF_H

#include"DXF_reader.h"
#include"engine/ObjectCollection.h"
#include"DXF_BLOCK.h"


/** \defgroup DXFGroup DXF Models
* Classes relating to loading and displaying 3D DXF models.
* @{
*/

class DXF_layer;

class DXF:public DXF_reader,public ObjectCollection
{
    public:
        DXF();
        DERIVE_OBJECT(DXF);

        int readFrom(FILE *file); //read from a file
        void DrawLowRes(float degradation, Window3D *win);
        int DrawHighRes(int eye, Window3D *win);
        DXF_BLOCK * getBlock(std::string const &n);
        void setColor(float red, float green, float blue);
        void dontUseColor();
        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile){return Object::LoadXML(xml,path,ncfile);}

    private:
        void processHeader();
        void processClasses();
        void processTables();
        void processBlocks();
        void processEntities();
        
        
        struct BlockNode
        {
            DXF_BLOCK * block;
            BlockNode * next;
        };
        
        BlockNode * blocks;
        
        float r,g,b;
        int useColor;
        DXF_layer * nullLayer;
};

///@}

#endif
