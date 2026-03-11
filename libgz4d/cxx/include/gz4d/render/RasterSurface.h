// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RASTERSURFACE_H
#define GZ4D_RASTERSURFACE_H

#include <gz4d/gz4d.h>
#include <gz4d/glo/DisplayList.h>
#include <gz4d/data/RasterMeta.h>
#include <gz4d/Point.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/Normal.h>
#include <gz4d/vector/operations.h>

namespace gz4d
{
    /// Renders data from a Raster as a surface.
    template<typename RasterMetaType>
    class RasterSurface: public glo::DisplayList
    {
        RasterMetaType raster;
        public:
            RasterSurface(){}
            RasterSurface(RasterSurface const &rs):raster(rs.raster){}
            RasterSurface(RasterMetaType const &rm):raster(rm){}

            void SetRaster(RasterMetaType rm){raster = rm; Rebuild();}
            void Draw()
            {
                if(raster.GetRaster())
                {
                    int rows = (int)raster.GetRaster()->GetRows();
                    int cols = (int)raster.GetRaster()->GetColumns();
                    if((rows > 1) && (cols > 1))
                    {
                        if(!Call())
                        {
                            glBegin(GL_QUADS);
                            for(int r = 0; r < rows-1; ++r)
                            {
                                
                                for(int c = 0; c < cols-1; ++c)
                                {
                                    Point<float> p00(raster.GetPoint(c,r));
                                    Point<float> p01(raster.GetPoint(c,r+1));
                                    Point<float> p10(raster.GetPoint(c+1,r));
                                    Point<float> p11(raster.GetPoint(c+1,r+1));
                                    
                                    Point<float> nx = (p10 - p00)+(p11 - p01);
                                    Point<float> ny = (p01 - p00)+(p11 - p10);
                                    Point<float> n = normalize(cross(nx,ny));
                                    glo::Normal(n);
                                    
                                    glTexCoord1f(raster.GetValue(c,r));
                                    glo::Vertex(p00);
                                    glTexCoord1f(raster.GetValue(c+1,r));
                                    glo::Vertex(p10);
                                    glTexCoord1f(raster.GetValue(c+1,r+1));
                                    glo::Vertex(p11);
                                    glTexCoord1f(raster.GetValue(c,r+1));
                                    glo::Vertex(p01);
                                    
                                }
                            }
                            glEnd();
                            End();
                        }
                    }
                }
            }

    };
}

#endif
