// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_ENTITY_H
#define DXF_ENTITY_H

#include"engine/SimpleObject.h"
#include"DXF_thing.h"
#include"DXF_VERTEX.h"
#include <gz4d/Color.h>

///\ingroup DXFGroup
class DXF_entity: public DXF_thing, public SimpleObject
{
    public:
        DXF_entity();

        virtual int readFrom(FILE *file=NULL)=0;

    protected:
        int processEntry();
        void processVerticies();
        void gotoOCS();
        int getFirstFace();

        gz4d::Color color;
        bool hasColor;
        gz4d::Vector<float,3> OCSnormal;
        std::vector<DXF_VERTEX> vertex;
        int firstFace;

        std::vector<float> OCSmatrix;
};


#endif
