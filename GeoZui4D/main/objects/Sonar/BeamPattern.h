#ifndef BEAMPATTERN_H
#define BEAMPATTERN_H

#include "engine/Object.h"
#include <gz4d/render/RasterSurface.h>
#include <gz4d/acoustic/UniformArray.h>
#include <gz4d/render/Grid.h>
#include "engine/TextureMaps/Clut.h"

class BeamPattern: public Object
{
    public:
        BeamPattern();
        DERIVE_OBJECT(BeamPattern);

        void DrawLowRes(float degradation, Window3D *win);
        int DrawHighRes(int eye, Window3D *win){hires = true; int ret = Object::DrawHighRes(eye,win); hires = false; return ret;}

        virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
        virtual Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

        virtual std::string GetHUDString();

    private:
        void Build();
        gz4d::acoustic::UniformArray array;
        float amplitude;

        std::vector<gz4d::Point<double> > footprint;
        gz4d::Point<double> fp_center;
        int fp_steps;

        gz4d::RasterSurface<gz4d::SphericalRaster<float,float> > plot;
        gz4d::SphericalRaster<float,float> raster;

        int thetaRes ,phiRes;
        gz4d::Interval<float> thetaInterval, phiInterval;

        double Lx, Ly;
        double Dx, Dy;
        int N,M;
        double minDB;

        double xSteering, ySteering;

        std::string elementType;
        gz4d::Grid<gz4d::SphericalPoint<float> > grid;

        bool hires;

        Clut pclut;
        bool buildClut;
};

#endif

