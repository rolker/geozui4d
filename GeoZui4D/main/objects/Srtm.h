// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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
