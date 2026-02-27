#ifndef GZ4D_DISTORTIONGRID_H
#define GZ4D_DISTORTIONGRID_H

#include <gz4d/gz4d.h>
#include <gz4d/data/Raster.h>
#include <gz4d/glo/DisplayList.h>
#include <gz4d/glo/Vertex.h>

namespace gz4d
{
    /// Renders a Raster of 2D floats as a grid capable of distorting a texture map.
    /// \ingroup renderer
    class DistortionGrid: public glo::DisplayList
    {
        //Raster<Vector<float,2> >::Ptr data;
        public:
            DistortionGrid(){}
            //DistortionGrid(Raster<Vector<float,2> >::Ptr &raster):DisplayList(),data(raster){}
            //DistortionGrid(DistortionGrid const &d):DisplayList(),data(d.data){}

            //void SetData(Raster<Vector<float,2> >::Ptr &r){if(r!=data){data = r;Rebuild();}}
            template<typename T> void Draw(const Raster<T> &data, int draw_type=GL_QUAD_STRIP)
            {
                //if(data)
                {
                    if(!Call())
                    {
                        glNormal3f(0.0,0.0,1.0);
                        float rstep = 1.0/(data.GetRows()-1);
                        float sstep = 1.0/(data.GetColumns()-1);
                        for(int r = 0; r < data.GetRows()-1; ++r)
                        {
                            float t = r*rstep;
                            T const *row0 = data.GetRow(r);
                            T const *row1 = data.GetRow(r+1);
                            glBegin(draw_type);
                            for(int c = 0; c < data.GetColumns(); ++c)
                            {
                                glTexCoord2f(c*sstep,t);
                                glo::Vertex(row0[c]);
                                glTexCoord2f(c*sstep,t+rstep);
                                glo::Vertex(row1[c]);
                            }
                            glEnd();
                        }
                        End();
                    }
                }
            }

    };
}

#endif
