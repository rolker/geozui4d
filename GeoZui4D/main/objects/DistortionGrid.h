#ifndef DISTORTIONGRID_H
#define DISTORTIONGRID_H

#include "engine/Object.h"
#include <gz4d/render/DistortionGrid.h>
#include "RasterStream.h"
#include <gz4d/Vector.h>
#include <gz4d/data/Raster.h>

class DistortionGrid: public RasterStream<gz4d::Raster<gz4d::Vector<float,2> > >, public Object
{
    public:
        DERIVE_OBJECT(DistortionGrid);
        void Idle(double time);
        void DrawLowRes(float degradation, Window3D *win);
        virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
    private:
        gz4d::DistortionGrid dg;
        Object *texture;
        gz4d::Raster<gz4d::Vector<float,2> >::Ptr currentRaster;
};

#endif
