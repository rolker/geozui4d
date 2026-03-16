// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GDALRASTER_H
#define GDALRASTER_H

#include "engine/Object.h"
#include <gz4d/data/Raster.h>
#include <gz4d/Point.h>
#include <gz4d/glo/BufferObject.h>

class GdalRaster : Object
{
    public:
        GdalRaster();
        ~GdalRaster();
        DERIVE_OBJECT(GdalRaster);

        void DrawLowRes(float degradation, Window3D *win);
        int Load();

    private:
        bool firstDraw;
        gz4d::Raster<gz4d::Point<float> > grid;

        // VBO rendering
        std::shared_ptr<gz4d::glo::Buffer> vertexBuffer;
        std::vector<std::shared_ptr<gz4d::glo::Buffer>> lodIndexBuffers;
        std::vector<std::vector<GLsizei>> lodCounts;
        std::vector<std::vector<const GLvoid*>> lodOffsets;
        int numLodLevels;

        double cellsizeX;
        double cellsizeY;
        double noDataValue;
        bool hasNoData;

        void buildVertexBuffer(Window3D *win);
        void buildLodIndexBuffers(Window3D *win);
        int selectLodLevel(float pixelSize);
};

#endif
