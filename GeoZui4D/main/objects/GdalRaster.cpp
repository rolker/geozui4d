// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "GdalRaster.h"
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/operations.h>
#include <gz4d/glo/VertexArray.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <cmath>
#include <iostream>

extern MainWindow * mainWindow;

REGISTER_OBJECT(GdalRaster, "tif");
REGISTER_ALIAS(GdalRasterTiff, GdalRaster, "tiff");
REGISTER_ALIAS(GdalRasterVrt, GdalRaster, "vrt");

GdalRaster::GdalRaster()
    :firstDraw(true)
    ,numLodLevels(0)
    ,cellsizeX(0.0)
    ,cellsizeY(0.0)
    ,noDataValue(0.0)
    ,hasNoData(false)
{
}

GdalRaster::~GdalRaster()
{
}

int GdalRaster::Load()
{
    if(GetFilename().empty())
        return 1;

    GDALAllRegister();
    GDALDataset *ds = static_cast<GDALDataset*>(GDALOpen(GetFilename().c_str(), GA_ReadOnly));
    if(!ds)
    {
        std::cerr << "GdalRaster: could not open " << GetFilename() << std::endl;
        return 1;
    }

    int bandCount = ds->GetRasterCount();
    int cols = ds->GetRasterXSize();
    int rows = ds->GetRasterYSize();

    if(bandCount < 1 || cols < 2 || rows < 2)
    {
        GDALClose(ds);
        return 1;
    }

    // Reject byte imagery (RGB/RGBA, palette, grayscale) — elevation data
    // uses wider types; let the texture loader handle images
    GDALDataType bandType = ds->GetRasterBand(1)->GetRasterDataType();
    if(bandType == GDT_Byte)
    {
        GDALClose(ds);
        return 1;
    }

    // Get geotransform
    double gt[6];
    if(ds->GetGeoTransform(gt) != CE_None)
    {
        std::cerr << "GdalRaster: no geotransform in " << GetFilename() << std::endl;
        GDALClose(ds);
        return 1;
    }

    // Set spatial reference from dataset projection
    const char *projWkt = ds->GetProjectionRef();
    if(projWkt && projWkt[0] != '\0')
        spatialReference.SetFromUserInput(projWkt);

    // Compute center in geographic coordinates for autoProjection
    double centerCol = cols / 2.0;
    double centerRow = rows / 2.0;
    double centerX = gt[0] + centerCol * gt[1] + centerRow * gt[2];
    double centerY = gt[3] + centerCol * gt[4] + centerRow * gt[5];

    // If the data is in a projected CRS, convert center to geographic
    double centerLon = centerX;
    double centerLat = centerY;
    if(spatialReference && !spatialReference.IsGeographic())
    {
        gz4d::ogr::SpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");
        gz4d::geo::Projection toGeo(spatialReference, wgs84);
        // Projection::operator()(lat, lon) returns Point(x, y, alt),
        // so for a geographic destination [0]=lon and [1]=lat
        gz4d::Point<double> geoCenter = toGeo(centerY, centerX);
        centerLon = geoCenter[0];
        centerLat = geoCenter[1];
    }

    autoProjection(centerLon, centerLat);

    // Read elevation band as float
    GDALRasterBand *band = ds->GetRasterBand(1);
    int hasNodataVal = 0;
    noDataValue = band->GetNoDataValue(&hasNodataVal);
    hasNoData = (hasNodataVal != 0);

    std::vector<float> elevation(cols * rows);
    if(band->RasterIO(GF_Read, 0, 0, cols, rows,
                       elevation.data(), cols, rows,
                       GDT_Float32, 0, 0) != CE_None)
    {
        std::cerr << "GdalRaster: error reading raster data" << std::endl;
        GDALClose(ds);
        return 1;
    }

    GDALClose(ds);

    // Project all grid points to scene coordinates
    gz4d::geo::Projection proj(spatialReference, mainWindow->getScene3D()->getSpatialReference());

    // Compute origin (top-left pixel center) in source CRS
    double originX = gt[0] + 0.5 * gt[1] + 0.5 * gt[2];
    double originY = gt[3] + 0.5 * gt[4] + 0.5 * gt[5];

    // Project origin to get object position
    gz4d::Point<double> geo = proj(originY, originX);
    SetPosition(geo[0], geo[1], 0.0);

    grid.Resize(cols, rows);

    // Project each grid point and store relative to origin
    // Also compute average cell size in projected coordinates
    double cellsizeSum = 0.0;
    int cellsizeCount = 0;

    for(int r = 0; r < rows; ++r)
    {
        for(int c = 0; c < cols; ++c)
        {
            double srcX = gt[0] + (c + 0.5) * gt[1] + (r + 0.5) * gt[2];
            double srcY = gt[3] + (c + 0.5) * gt[4] + (r + 0.5) * gt[5];

            gz4d::Point<double> p = proj(srcY, srcX);

            float z = elevation[r * cols + c];
            if(hasNoData && z == static_cast<float>(noDataValue))
                z = std::numeric_limits<float>::quiet_NaN();

            grid.GetRow(r)[c] = gz4d::Point<float>(
                static_cast<float>(p[0] - geo[0]),
                static_cast<float>(p[1] - geo[1]),
                z
            );

            // Sample cell sizes from a few points
            if(c > 0 && r == 0)
            {
                float dx = grid.GetRow(r)[c][0] - grid.GetRow(r)[c-1][0];
                float dy = grid.GetRow(r)[c][1] - grid.GetRow(r)[c-1][1];
                cellsizeSum += std::sqrt(dx*dx + dy*dy);
                cellsizeCount++;
            }
        }
        if(r > 0)
        {
            float dx = grid.GetRow(r)[0][0] - grid.GetRow(r-1)[0][0];
            float dy = grid.GetRow(r)[0][1] - grid.GetRow(r-1)[0][1];
            cellsizeSum += std::sqrt(dx*dx + dy*dy);
            cellsizeCount++;
        }
    }

    if(cellsizeCount > 0)
    {
        double avgCellsize = cellsizeSum / cellsizeCount;
        cellsizeX = avgCellsize;
        cellsizeY = avgCellsize;
    }
    else
    {
        cellsizeX = 1.0;
        cellsizeY = 1.0;
    }

    std::cerr << "GdalRaster: loaded " << cols << "x" << rows
              << " cellsize=" << cellsizeX << std::endl;

    return 0;
}

void GdalRaster::buildVertexBuffer(Window3D *win)
{
    int cols = grid.GetColumns();
    int rows = grid.GetRows();

    vertexBuffer = std::shared_ptr<gz4d::glo::Buffer>(
        new gz4d::glo::Buffer(win->getGloContext(), GL_ARRAY_BUFFER));

    std::vector<gz4d::glo::VertexArray::N3F_V3F> v(rows * cols);

    for(int y = 0; y < rows; ++y)
    {
        for(int x = 0; x < cols; ++x)
        {
            int idx = y * cols + x;
            gz4d::Point<float> const &pt = grid.GetRow(y)[x];

            v[idx].x = pt[0];
            v[idx].y = pt[1];
            v[idx].z = pt[2];

            // Compute normal via cross product of adjacent faces
            gz4d::Vector<double,3> normal(0.0, 0.0, 1.0);
            if(x < cols - 1 && y < rows - 1)
            {
                normal = cross(
                    grid.GetRow(y)[x+1] - grid.GetRow(y)[x],
                    grid.GetRow(y+1)[x] - grid.GetRow(y)[x]
                );
                if(!gz4d::IsNan(normal[0]) && !gz4d::IsNan(normal[1]))
                    normal = normalize(normal);
                else
                    normal = gz4d::Vector<double,3>(0.0, 0.0, 1.0);
            }
            else if(x == cols - 1 && x > 0)
            {
                // Copy from left neighbor
                int prevIdx = y * cols + x - 1;
                normal = gz4d::Vector<double,3>(v[prevIdx].nx, v[prevIdx].ny, v[prevIdx].nz);
            }
            else if(y == rows - 1 && y > 0)
            {
                // Copy from above neighbor
                int prevIdx = (y-1) * cols + x;
                normal = gz4d::Vector<double,3>(v[prevIdx].nx, v[prevIdx].ny, v[prevIdx].nz);
            }

            v[idx].nx = static_cast<float>(normal[0]);
            v[idx].ny = static_cast<float>(normal[1]);
            v[idx].nz = static_cast<float>(normal[2]);
        }
    }

    vertexBuffer->Bind();
    vertexBuffer->Data(v.size() * sizeof(gz4d::glo::VertexArray::N3F_V3F), &v.front());
    vertexBuffer->Unbind();
}

void GdalRaster::buildLodIndexBuffers(Window3D *win)
{
    int cols = grid.GetColumns();
    int rows = grid.GetRows();
    int minDim = std::min(cols, rows);

    // Generate LOD levels with step sizes 1, 2, 4, 8, ...
    // Level i always uses step 2^i; the full-resolution level is always built
    // so small rasters still render.
    for(int step = 1; step == 1 || step <= minDim / 4; step *= 2)
    {
        std::vector<uint32_t> indices;
        std::vector<GLsizei> counts;
        std::vector<const GLvoid*> offsets;

        for(int y = 0; y + step < rows; y += step)
        {
            // Build strip segments for this row, breaking at nodata
            int stripStart = -1;
            int stripCount = 0;

            for(int x = 0; x < cols; x += step)
            {
                int i0 = y * cols + x;
                int i1 = (y + step) * cols + x;

                // Check for nodata (NaN)
                bool valid = !std::isnan(grid.GetRow(y)[x][2]) &&
                             !std::isnan(grid.GetRow(y + step)[x][2]);

                if(valid)
                {
                    if(stripStart < 0)
                        stripStart = indices.size();

                    indices.push_back(i0);
                    indices.push_back(i1);
                    stripCount += 2;
                }
                else
                {
                    // End current strip if one is active
                    if(stripCount > 0)
                    {
                        offsets.push_back(reinterpret_cast<const GLvoid*>(
                            static_cast<uintptr_t>(stripStart * sizeof(uint32_t))));
                        counts.push_back(stripCount);
                        stripStart = -1;
                        stripCount = 0;
                    }
                }
            }

            // Finalize last strip segment in this row
            if(stripCount > 0)
            {
                offsets.push_back(reinterpret_cast<const GLvoid*>(
                    static_cast<uintptr_t>(stripStart * sizeof(uint32_t))));
                counts.push_back(stripCount);
            }
        }

        // Push a (possibly empty) entry for every level so buffer index
        // always matches LOD level; DrawLowRes skips levels with no strips
        auto ebo = std::shared_ptr<gz4d::glo::Buffer>(
            new gz4d::glo::Buffer(win->getGloContext(), GL_ELEMENT_ARRAY_BUFFER));
        if(!indices.empty())
        {
            ebo->Bind();
            ebo->Data(indices.size() * sizeof(uint32_t), indices.data());
            ebo->Unbind();
        }

        lodIndexBuffers.push_back(ebo);
        lodCounts.push_back(counts);
        lodOffsets.push_back(offsets);
    }

    numLodLevels = static_cast<int>(lodIndexBuffers.size());
}

int GdalRaster::selectLodLevel(float pixelSize)
{
    if(numLodLevels <= 1)
        return 0;

    double cellsPerPixel = pixelSize / cellsizeX;
    if(cellsPerPixel <= 1.0)
        return 0;

    int level = static_cast<int>(std::floor(std::log2(cellsPerPixel)));
    if(level < 0)
        level = 0;
    if(level >= numLodLevels)
        level = numLodLevels - 1;
    return level;
}

void GdalRaster::DrawLowRes(float degradation, Window3D *win)
{
    if(grid.GetColumns() < 2 || grid.GetRows() < 2)
        return;

    if(firstDraw)
    {
        buildVertexBuffer(win);
        buildLodIndexBuffers(win);
        firstDraw = false;
    }

    // Select LOD level
    float pixelSize = static_cast<float>(win->getViewInfo()->getPixelSize());
    int level = selectLodLevel(pixelSize);

    // Bias toward coarser LOD during interaction
    if(degradation > 0.0f && level < numLodLevels - 1)
        level += static_cast<int>(degradation * 2.0f);
    if(level >= numLodLevels)
        level = numLodLevels - 1;

    if(level >= static_cast<int>(lodIndexBuffers.size()) || lodCounts[level].empty())
        return;

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    EnableTexture();
    glColor3f(1.0, 1.0, 1.0);

    vertexBuffer->Bind();
    lodIndexBuffers[level]->Bind();
    glInterleavedArrays(GL_N3F_V3F, 0, NULL);
    glMultiDrawElements(GL_QUAD_STRIP,
                        lodCounts[level].data(),
                        GL_UNSIGNED_INT,
                        lodOffsets[level].data(),
                        lodCounts[level].size());
    glDisableClientState(GL_VERTEX_ARRAY);
    vertexBuffer->Unbind();
    lodIndexBuffers[level]->Unbind();
    DisableTexture();
}
